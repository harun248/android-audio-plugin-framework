
#include "juce_android_audio_plugin_format.h"
#include <sys/mman.h>
#include <libgen.h>
#include <unistd.h>
#if ANDROID
#include <android/sharedmem.h>
#else
#endif

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


void AndroidAudioPluginInstance::fillNativeInputBuffers(AudioBuffer<float> &audioBuffer,
														MidiBuffer &midiBuffer) {
	// FIXME: there is some glitch between how JUCE AudioBuffer assigns a channel for each buffer item
	//  and how AAP expects them.
    int juceInputsAssigned = 0;
    auto aapDesc = this->native->getPluginDescriptor();
	int n = aapDesc->getNumPorts();
    for (int i = 0; i < n; i++) {
    	auto port = aapDesc->getPort(i);
    	if (port->getPortDirection() != AAP_PORT_DIRECTION_INPUT)
    		continue;
    	if (port->getContentType() == AAP_CONTENT_TYPE_AUDIO && juceInputsAssigned < audioBuffer.getNumChannels())
	        memcpy(buffer->buffers[i], (void *) audioBuffer.getReadPointer(juceInputsAssigned++), audioBuffer.getNumSamples() * sizeof(float));
		else if (port->getContentType() == AAP_CONTENT_TYPE_MIDI) {
			int di = 4; // fill length later
			MidiMessage msg{};
			int pos{0};
			// FIXME: time unit must be configurable.
			double oneTick = 1 / 480.0; // sec
			double secondsPerFrame = 1.0 / sample_rate; // sec
			MidiBuffer::Iterator iter{midiBuffer};
			*((int32_t*) buffer->buffers[i]) = 0; // reset to ensure that there is no message by default
			while (iter.getNextEvent(msg, pos)) {
				double timestamp = msg.getTimeStamp();
				double timestampSeconds = timestamp * secondsPerFrame;
				int32_t timestampTicks = (int32_t) (timestampSeconds / oneTick);
				do {
					*((uint8_t*) buffer->buffers[i] + di++) = (uint8_t) timestampTicks % 0x80;
					timestampTicks /= 0x80;
				} while (timestampTicks > 0x80);
				memcpy(((uint8_t*) buffer->buffers[i]) + di, msg.getRawData(), msg.getRawDataSize());
				di += msg.getRawDataSize();
			}
			// AAP MIDI buffer is length-prefixed raw MIDI data.
			*((int32_t*) buffer->buffers[i]) = di - 4;
		} else {
			// control parameters should assigned when parameter is assigned.
			// FIXME: at this state there is no callbacks for parameter changes, so
			// any dynamic parameter changes are ignored at this state.
		}
    }
}

void AndroidAudioPluginInstance::fillNativeOutputBuffers(AudioBuffer<float> &audioBuffer) {
	// FIXME: there is some glitch between how JUCE AudioBuffer assigns a channel for each buffer item
	//  and how AAP expects them.
	int juceOutputsAssigned = 0;
	auto aapDesc = this->native->getPluginDescriptor();
	int n = aapDesc->getNumPorts();
	for (int i = 0; i < n; i++) {
		auto port = aapDesc->getPort(i);
		if (port->getPortDirection() != AAP_PORT_DIRECTION_OUTPUT)
			continue;
		if (port->getContentType() == AAP_CONTENT_TYPE_AUDIO &&
			juceOutputsAssigned < audioBuffer.getNumChannels())
			memcpy((void *) audioBuffer.getWritePointer(juceOutputsAssigned++), buffer->buffers[i],
				   buffer->num_frames * sizeof(float));
	}
}

AndroidAudioPluginInstance::AndroidAudioPluginInstance(aap::PluginInstance *nativePlugin)
        : native(nativePlugin),
          sample_rate(-1) {
	buffer.reset(new AndroidAudioPluginBuffer());
    // It is super awkward, but plugin parameter definition does not exist in juce::PluginInformation.
    // Only AudioProcessor.addParameter() works. So we handle them here.
    auto desc = nativePlugin->getPluginDescriptor();
    for (int i = 0; i < desc->getNumPorts(); i++) {
    	auto port = desc->getPort(i);
    	if (port->getPortDirection() != AAP_PORT_DIRECTION_INPUT || port->getContentType() != AAP_CONTENT_TYPE_UNDEFINED)
    		continue;
		portMapAapToJuce[i] = getNumParameters();
        addParameter(new AndroidAudioPluginParameter(port));
	}
}

void AndroidAudioPluginInstance::allocateSharedMemory(int bufferIndex, int32_t size)
{
#if ANDROID
    int fd = ASharedMemory_create(nullptr, size);
        buffer->shared_memory_fds[bufferIndex] = fd;
        buffer->buffers[bufferIndex] = mmap(nullptr, buffer->num_frames * sizeof(float),
                PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
#else
    buffer->buffers[bufferIndex] = mmap(nullptr, size,
                             PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	//buffer->buffers[bufferIndex] = calloc(size, 1);
    assert(buffer->buffers[bufferIndex]);
#endif
}

void
AndroidAudioPluginInstance::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) {
    sample_rate = sampleRate;

    // minimum setup, as the pointers are not passed by JUCE framework side.
    int n = native->getPluginDescriptor()->getNumPorts();
    buffer->shared_memory_fds = new int[n + 1];
    buffer->buffers = new void *[n + 1];
    buffer->num_frames = maximumExpectedSamplesPerBlock;
    for (int i = 0; i < n; i++)
        allocateSharedMemory(i, buffer->num_frames * sizeof(float));
    buffer->shared_memory_fds[n] = 0;
    buffer->buffers[n] = nullptr;

    native->prepare(sampleRate, maximumExpectedSamplesPerBlock, buffer.get());

	for (int i = 0; i < n; i++) {
		auto port = native->getPluginDescriptor()->getPort(i);
		if (port->getContentType() == AAP_CONTENT_TYPE_UNDEFINED && port->getPortDirection() == AAP_PORT_DIRECTION_INPUT) {
			float v = this->getParameters()[portMapAapToJuce[i]]->getValue();
			std::fill_n((float *) buffer->buffers[i], buffer->num_frames, v);
		}
	}
    native->activate();
}

void AndroidAudioPluginInstance::releaseResources() {
    native->dispose();

    if (buffer->buffers) {
        for (int i = 0; buffer->buffers[i] != nullptr; i++) {
#if ANDROID
            munmap(buffer->buffers[i], buffer->num_frames * sizeof(float));
            if (buffer->shared_memory_fds[i] != 0)
	            close(buffer->shared_memory_fds[i]);
#else
			munmap(buffer->buffers[i], buffer->num_frames * sizeof(float));
            //free(buffer->buffers[i]);
#endif
        }
        buffer->buffers = nullptr;
    }
}

void AndroidAudioPluginInstance::processBlock(AudioBuffer<float> &audioBuffer,
                                              MidiBuffer &midiMessages) {
	fillNativeInputBuffers(audioBuffer, midiMessages);
    native->process(buffer.get(), 0);
	fillNativeOutputBuffers(audioBuffer);
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
	for (auto p : getPluginHostPAL()->getInstalledPlugins())
		if (strcmp(p->getPluginID().c_str(), desc.fileOrIdentifier.toRawUTF8()) == 0)
			return p;
	return nullptr;
}

AndroidAudioPluginFormat::AndroidAudioPluginFormat()
        : android_host(aap::PluginHost()) {
#if ANDROID
    for (int i = 0; i < android_host.getNumPluginDescriptors(); i++) {
        auto d = android_host.getPluginDescriptorAt(i);
        auto dst = new PluginDescription();
        fillPluginDescriptionFromNative(*dst, *d);
        cached_descs.set(d, dst);
    }
#endif
}

AndroidAudioPluginFormat::~AndroidAudioPluginFormat() {}

void AndroidAudioPluginFormat::findAllTypesForFile(OwnedArray <PluginDescription> &results,
                                                   const String &fileOrIdentifier) {
	// For Android `fileOrIdentifier` is a service name, and for desktop it is a specific `aap_metadata.xml` file.
#if ANDROID
    auto id = fileOrIdentifier.toRawUTF8();
    // So far there is no way to perform query (without Java help) it is retrieved from cached list.
    for (aap::PluginInformation* p : getPluginHostPAL()->getPluginListCache()) {
        if (strcmp(p->getContainerIdentifier().c_str(), id) == 0) {
            auto d = new PluginDescription();
			juce_plugin_descs.add(d);
            fillPluginDescriptionFromNative(*d, *p);
            results.add(d);
        }
    }
#else
	auto metadataPath = fileOrIdentifier.toRawUTF8();
	for (auto p : PluginInformation::parsePluginDescriptor(metadataPath, metadataPath)) {
		auto dst = new PluginDescription();
		juce_plugin_descs.add(dst); // to automatically free when disposing `this`.
		fillPluginDescriptionFromNative(*dst, *p);
		results.add(dst);
		// FIXME: not sure if we should add it here. There is no ther timing to do this at desktop.
		cached_descs.set(p, dst);
	}
#endif
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

StringArray AndroidAudioPluginFormat::searchPathsForPlugins(const FileSearchPath &directoriesToSearch,
								  bool recursive,
								  bool allowPluginsWhichRequireAsynchronousInstantiation) {
	std::vector<std::string> paths{};
	for (int i = 0; i < directoriesToSearch.getNumPaths(); i++)
		getPluginHostPAL()->getAAPMetadataPaths(directoriesToSearch[i].getFullPathName().toRawUTF8(), paths);
	StringArray ret{};
	for (auto p : paths)
		ret.add(p);
	return ret;
}

// Unlike desktop system, it is not practical to either look into file systems
// on Android. And it is simply impossible to "enumerate" asset directories.
// Therefore we simply return empty list.
FileSearchPath AndroidAudioPluginFormat::getDefaultLocationsToSearch() {
	FileSearchPath ret{};
	for (auto path : getPluginHostPAL()->getPluginPaths()) {
		File dir{path};
		if (dir.exists())
			ret.add(dir);
	}
	return ret;
}

} // namespace
