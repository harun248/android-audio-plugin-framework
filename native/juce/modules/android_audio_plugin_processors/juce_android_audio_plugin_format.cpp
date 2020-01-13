
#include "juce_android_audio_plugin_format.h"
#include <android/sharedmem.h>
#include <sys/mman.h>
#include <unistd.h>

using namespace aap;
using namespace juce;

namespace juceaap {

AndroidAudioPluginEditor::AndroidAudioPluginEditor(AudioProcessor *processor, aap::EditorInstance *native)
    : juce::AudioProcessorEditor(processor), native(native)
{
}

double AndroidAudioPluginInstance::getTailLengthSeconds() const {
    return native->getTailTimeInMilliseconds() / 1000.0;
}

static void fillPluginDescriptionFromNative(PluginDescription &description,
                                            const aap::PluginInformation &src) {
    description.name = src.getName();
    description.pluginFormatName = "AAP";

    description.category.clear();
    description.category += juce::String(src.getPrimaryCategory());

    description.manufacturerName = src.getManufacturerName();
    description.version = src.getVersion();
    // JUCE plugin identifier is "PluginID" in AAP (not "identifier_string").
    description.fileOrIdentifier = src.getPluginID();
    // So far this is as hacky as AudioUnit implementation.
    description.lastFileModTime = Time();
    description.lastInfoUpdateTime = Time(src.getLastInfoUpdateTime());
    description.uid = String(src.getPluginID()).hashCode();
    description.isInstrument = src.isInstrument();
    for (int i = 0; i < src.getNumPorts(); i++) {
        auto port = src.getPort(i);
        auto dir = port->getPortDirection();
        if (dir == AAP_PORT_DIRECTION_INPUT)
            description.numInputChannels++;
        else if (dir == AAP_PORT_DIRECTION_OUTPUT)
            description.numOutputChannels++;
    }
    description.hasSharedContainer = src.hasSharedContainer();
}


void AndroidAudioPluginInstance::fillNativeAudioBuffers(AndroidAudioPluginBuffer *dst,
                                                        AudioBuffer<float> &buffer) {
    int n = buffer.getNumChannels();
    for (int i = 0; i < n; i++)
        memcpy(dst->buffers[i], (void *) buffer.getReadPointer(i), buffer.getNumSamples() * sizeof(float));
}

void AndroidAudioPluginInstance::fillNativeMidiBuffers(AndroidAudioPluginBuffer *dst,
                                                       MidiBuffer &buffer) {
    auto desc = native->getPluginDescriptor();
    int numPorts = desc->getNumPorts();
    for (int i = 0; i < numPorts; i++)
        if (desc->getPort(i)->getContentType() == AAP_CONTENT_TYPE_MIDI &&
            desc->getPort(i)->getPortDirection() == AAP_PORT_DIRECTION_INPUT)
            memcpy(dst->buffers[i], buffer.data.getRawDataPointer(), buffer.data.size());
}

int AndroidAudioPluginInstance::getNumBuffers(AndroidAudioPluginBuffer *buffer) {
    auto b = buffer->buffers;
    int n = 0;
    while (b) {
        n++;
        b++;
    }
    return n;
}

AndroidAudioPluginInstance::AndroidAudioPluginInstance(aap::PluginInstance *nativePlugin)
        : native(nativePlugin),
          sample_rate(-1) {
}

void
AndroidAudioPluginInstance::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) {
    sample_rate = sampleRate;

    // minimum setup, as the pointers are not passed by JUCE framework side.
    int n = native->getPluginDescriptor()->getNumPorts();
    buffer.shared_memory_fds = new int[n + 1];
    buffer.buffers = new void *[n + 1];
    buffer.num_frames = maximumExpectedSamplesPerBlock;
    for (int i = 0; i < n; i++) {
        int fd = ASharedMemory_create(nullptr, buffer.num_frames * sizeof(float));
        buffer.shared_memory_fds[i] = fd;
        buffer.buffers[i] = mmap(nullptr, buffer.num_frames * sizeof(float),
                PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    }
    buffer.shared_memory_fds[n] = 0;
    buffer.buffers[n] = nullptr;

    native->prepare(sampleRate, maximumExpectedSamplesPerBlock, &buffer);

    native->activate();
}

void AndroidAudioPluginInstance::releaseResources() {
    native->dispose();

    if (buffer.buffers) {
        for (int i = 0; buffer.buffers[i] != nullptr; i++) {
            munmap(buffer.buffers[i], buffer.num_frames * sizeof(float));
            close(buffer.shared_memory_fds[i]);
        }
        buffer.buffers = NULL;
    }
}

void AndroidAudioPluginInstance::processBlock(AudioBuffer<float> &audioBuffer,
                                              MidiBuffer &midiMessages) {
    fillNativeAudioBuffers(&buffer, audioBuffer);
    fillNativeMidiBuffers(&buffer, midiMessages);
    native->process(&buffer, 0);
}

bool AndroidAudioPluginInstance::hasMidiPort(bool isInput) const {
    auto d = native->getPluginDescriptor();
    for (int i = 0; i < d->getNumPorts(); i++) {
        auto p = d->getPort(i);
        if (p->getPortDirection() ==
            (isInput ? AAP_PORT_DIRECTION_INPUT : AAP_PORT_DIRECTION_OUTPUT) &&
            p->getContentType() == AAP_CONTENT_TYPE_MIDI)
            return true;
    }
    return false;
}

AudioProcessorEditor *AndroidAudioPluginInstance::createEditor() {
    if (!native->getPluginDescriptor()->hasEditor())
        return nullptr;
    auto ret = new AndroidAudioPluginEditor(this, native->createEditor());
    ret->startEditorUI();
    return ret;
}

void AndroidAudioPluginInstance::fillInPluginDescription(PluginDescription &description) const {
    auto src = native->getPluginDescriptor();
    fillPluginDescriptionFromNative(description, *src);
}

const aap::PluginInformation *
AndroidAudioPluginFormat::findDescriptorFrom(const PluginDescription &desc) {
    auto it = cached_descs.begin();
    while (it.next())
        if (it.getValue()->uid == desc.uid)
            return it.getKey();
    return NULL;
}

AndroidAudioPluginFormat::AndroidAudioPluginFormat()
        : android_host(aap::PluginHost()) {
    for (int i = 0; i < android_host.getNumPluginDescriptors(); i++) {
        auto d = android_host.getPluginDescriptorAt(i);
        auto dst = new PluginDescription();
        fillPluginDescriptionFromNative(*dst, *d);
        cached_descs.set(d, dst);
    }
}

AndroidAudioPluginFormat::~AndroidAudioPluginFormat() {
    auto it = cached_descs.begin();
    while (it.next())
        delete it.getValue();
    cached_descs.clear();
}

void AndroidAudioPluginFormat::findAllTypesForFile(OwnedArray <PluginDescription> &results,
                                                   const String &fileOrIdentifier) {
    auto id = fileOrIdentifier.toRawUTF8();
    // FIXME: everything is already cached, so this can be simplified.
    for (int i = 0; i < android_host.getNumPluginDescriptors(); i++) {
        auto d = android_host.getPluginDescriptorAt(i);
        if (strcmp(id, d->getName().data()) == 0 ||
            strcmp(id, d->getPluginID().data()) == 0) {
            auto dst = cached_descs[d];
            if (!dst)
                // doesn't JUCE handle invalid fileOrIdentifier?
                return;
            results.add(dst);
        }
    }
}

void AndroidAudioPluginFormat::createPluginInstance(const PluginDescription &description,
                                                    double initialSampleRate,
                                                    int initialBufferSize,
                                                    PluginCreationCallback callback) {
    auto descriptor = findDescriptorFrom(description);
    String error("");
    if (descriptor == nullptr) {
        error << "Android Audio Plugin " << description.name << "was not found.";
        callback(nullptr, error);
    } else {
        auto androidInstance = android_host.instantiatePlugin(
                descriptor->getPluginID().data());
        std::unique_ptr <AndroidAudioPluginInstance> instance{
                new AndroidAudioPluginInstance(androidInstance)};
        callback(std::move(instance), error);
    }
}
} // namespace
