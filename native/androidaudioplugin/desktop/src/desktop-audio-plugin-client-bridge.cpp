
#include <memory>
#include <sys/mman.h>
#include <fcntl.h>
#include "aap/android-audio-plugin.h"
#include "aap/android-audio-plugin-host.hpp"
#include "audio-plugin-host-desktop.hpp"
#include <grpc++/grpc++.h>
// It should be automatically generated by cmake scripts
#include "AudioPluginService.grpc.pb.h"

#define AAP_SERVICE_URL "localhost:44100"

class DesktopClientContext {

public:
	const char *unique_id{nullptr};
	int32_t instance_id{0};
	////ndk::SpAIBinder spAIBinder{nullptr};
    aap::SharedMemoryExtension* shared_memory_extension{nullptr};
    std::unique_ptr<org::androidaudioplugin::AudioPluginService::Stub> proxy{nullptr};
	AndroidAudioPluginBuffer *previous_buffer{nullptr};
	AndroidAudioPluginState state{};
	int state_ashmem_fd{0};

    DesktopClientContext() {}

    int initialize(int sampleRate, const char *pluginUniqueId)
	{
		unique_id = pluginUniqueId;
    	auto channel = grpc::CreateChannel(AAP_SERVICE_URL, grpc::InsecureChannelCredentials());
    	proxy = std::move(org::androidaudioplugin::AudioPluginService::NewStub(channel));
		return 0;
    }

    ~DesktopClientContext()
    {
		////if (instance_id != 0)
	    ////	proxy->destroy(instance_id);
    }
};

void releaseStateBuffer(DesktopClientContext *ctx)
{
	if (ctx->state.raw_data)
		munmap((void*) ctx->state.raw_data, (size_t) ctx->state.data_size);
	if (ctx->state_ashmem_fd)
		shm_unlink(ctx->unique_id);
}

void ensureStateBuffer(DesktopClientContext *ctx, int bufferSize)
{
	if (ctx->state.raw_data && ctx->state.data_size >= bufferSize)
		return;
	releaseStateBuffer(ctx);
	ctx->state_ashmem_fd = shm_open(ctx->unique_id, O_CREAT | O_RDWR, bufferSize);
	ctx->state.data_size = bufferSize;
	ctx->state.raw_data = mmap(nullptr, bufferSize, PROT_READ | PROT_WRITE, MAP_SHARED, ctx->state_ashmem_fd, 0);
}

void resetBuffers(DesktopClientContext *ctx, AndroidAudioPluginBuffer* buffer)
{
	int n = buffer->num_buffers;

	auto prevBuf = ctx->previous_buffer;
	auto &fds = ctx->shared_memory_extension->getPortBufferFDs();

    // close extra shm FDs that are (1)insufficient in size, or (2)not needed anymore.
    if (prevBuf != nullptr) {
        int nPrev = prevBuf->num_buffers;
        for (int i = prevBuf->num_frames < buffer->num_frames ? 0 : n; i < nPrev; i++) {
            close(fds[i]);
            fds[i] = 0;
        }
    }
    fds.resize(n, 0);

    org::androidaudioplugin::PrepareRequest request;

    // allocate shm FDs, first locally, then remotely.
    for (int i = 0; i < n; i++) {
        assert(fds[i] != 0);
        request.set_shared_memory_fds(i, fds[i]);
    }

    request.set_instance_id(ctx->instance_id);
    request.set_frame_count(buffer->num_frames);
    request.set_port_count(n);
    org::androidaudioplugin::Unit response;
    grpc::ClientContext gctx;
	auto status = ctx->proxy->Prepare(&gctx, request, &response);
    assert(status.ok());

	ctx->previous_buffer = buffer;
}

void aap_bridge_plugin_prepare(AndroidAudioPlugin *plugin, AndroidAudioPluginBuffer* buffer)
{
	auto ctx = (DesktopClientContext*) plugin->plugin_specific;
	resetBuffers(ctx, buffer);
}

void aap_bridge_plugin_activate(AndroidAudioPlugin *plugin)
{
	auto ctx = (DesktopClientContext*) plugin->plugin_specific;
    org::androidaudioplugin::InstanceId request;
    request.set_instance_id(ctx->instance_id);
    org::androidaudioplugin::Unit response;
    grpc::ClientContext gctx;
    auto status = ctx->proxy->Activate(&gctx, request, &response);
    assert(status.ok());
}

void aap_bridge_plugin_process(AndroidAudioPlugin *plugin,
	AndroidAudioPluginBuffer* buffer,
	long timeoutInNanoseconds)
{
	auto ctx = (DesktopClientContext*) plugin->plugin_specific;
	if (ctx->previous_buffer != buffer)
		resetBuffers(ctx, buffer);
    org::androidaudioplugin::ProcessRequest request;
    request.set_instance_id(ctx->instance_id);
    request.set_timeout_nanoseconds(timeoutInNanoseconds);
    org::androidaudioplugin::Unit response;
    grpc::ClientContext gctx;
	auto status = ctx->proxy->Process(&gctx, request, &response);
    assert(status.ok());
}

void aap_bridge_plugin_deactivate(AndroidAudioPlugin *plugin)
{
	auto ctx = (DesktopClientContext*) plugin->plugin_specific;
    org::androidaudioplugin::InstanceId request;
    request.set_instance_id(ctx->instance_id);
    org::androidaudioplugin::Unit response;
    grpc::ClientContext gctx;
	auto status = ctx->proxy->Deactivate(&gctx, request, &response);
    assert (status.ok());
}

void aap_bridge_plugin_get_state(AndroidAudioPlugin *plugin, AndroidAudioPluginState* result)
{
	auto ctx = (DesktopClientContext*) plugin->plugin_specific;
    org::androidaudioplugin::InstanceId sizeRequest;
    sizeRequest.set_instance_id(ctx->instance_id);
    org::androidaudioplugin::Size sizeResponse;
    grpc::ClientContext gctx;
	auto status = ctx->proxy->GetStateSize(&gctx, sizeRequest, &sizeResponse);
    assert(status.ok());
	ensureStateBuffer(ctx, sizeResponse.size());
	org::androidaudioplugin::GetStateRequest request;
	request.set_instance_id(ctx->instance_id);
	request.set_shared_memory_fd(ctx->state_ashmem_fd);
    org::androidaudioplugin::Unit response;
	auto status2 = ctx->proxy->GetState(&gctx, request, &response);
    assert(status2.ok());
    result->data_size = ctx->state.data_size;
    result->raw_data = ctx->state.raw_data;
}

void aap_bridge_plugin_set_state(AndroidAudioPlugin *plugin, AndroidAudioPluginState *input)
{
	auto ctx = (DesktopClientContext*) plugin->plugin_specific;
	// we have to ensure that the pointer is shared memory, so use state buffer inside ctx.
	ensureStateBuffer(ctx, input->data_size);
	memcpy((void*) ctx->state.raw_data, input->raw_data, (size_t) input->data_size);
    grpc::ClientContext gctx;
	org::androidaudioplugin::SetStateRequest request;
	request.set_instance_id(ctx->instance_id);
	request.set_size(input->data_size);
	request.set_shared_memory_fd(ctx->state_ashmem_fd);
    org::androidaudioplugin::Unit response;
	auto status = ctx->proxy->SetState(&gctx, request, &response);
    assert(status.ok());
}

AndroidAudioPlugin* aap_bridge_plugin_new(
	AndroidAudioPluginFactory *pluginFactory,	// unused
	const char* pluginUniqueId,
	int aapSampleRate,
	AndroidAudioPluginExtension** extensions	// unused
	)
{
	assert(pluginFactory != nullptr);
	assert(pluginUniqueId != nullptr);
	assert(extensions != nullptr);

	// TODO: initialize IPC stuff here.
    auto ctx = new DesktopClientContext();
    if(ctx->initialize(aapSampleRate, pluginUniqueId))
        return nullptr;

    for (int i = 0; extensions[i] != nullptr; i++) {
        auto ext = extensions[i];
        if (strcmp(ext->uri, aap::SharedMemoryExtension::URI) == 0) {
            ctx->shared_memory_extension = (aap::SharedMemoryExtension *) ext->data;
            break;
        }
    }
    assert(ctx->shared_memory_extension != nullptr);

    grpc::ClientContext gctx;
    org::androidaudioplugin::CreateRequest request;
    request.set_plugin_id(pluginUniqueId);
    request.set_sample_rate(aapSampleRate);
    org::androidaudioplugin::InstanceId response;
    auto status = ctx->proxy->Create(&gctx, request, &response);
    assert(status.ok());

	return new AndroidAudioPlugin {
		ctx,
		aap_bridge_plugin_prepare,
		aap_bridge_plugin_activate,
		aap_bridge_plugin_process,
		aap_bridge_plugin_deactivate,
		aap_bridge_plugin_get_state,
		aap_bridge_plugin_set_state
		};
}

void aap_bridge_plugin_delete(
		AndroidAudioPluginFactory *pluginFactory,	// unused
		AndroidAudioPlugin *instance)
{
	auto ctx = (DesktopClientContext*) instance->plugin_specific;
	releaseStateBuffer(ctx);

	delete ctx;
	delete instance;
}


extern "C" {

AndroidAudioPluginFactory* GetDesktopAudioPluginFactoryClientBridge() {
	return new AndroidAudioPluginFactory{aap_bridge_plugin_new, aap_bridge_plugin_delete};
}

}

