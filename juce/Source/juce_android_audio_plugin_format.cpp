
#include "../JuceLibraryCode/JuceHeader.h"
#include "../../include/android-audio-plugin-host.hpp"

using namespace aap;
using namespace juce;

namespace juceaap
{

class JuceAndroidAudioPluginInstance;

class JuceAndroidAudioPluginEditor : public juce::AudioProcessorEditor
{
	AAPEditor *native;
	
public:

	JuceAndroidAudioPluginEditor(AudioProcessor *processor, AAPEditor *native)
		: AudioProcessorEditor(processor), native(native)
	{
	}
	
	void startEditorUI()
	{
		native->startEditorUI();
	}
	
	// TODO: override if we want to.
	/*
	virtual void setScaleFactor(float newScale)
	{
	}
	*/
};

static void fillPluginDescriptionFromNative(PluginDescription &description, AAPDescriptor &src)
{
	description.name = src.getName();
	description.pluginFormatName = "AAP";
	
	description.category.clear();
	description.category += src.getPrimaryCategory();
	
	description.manufacturerName = src.getManufacturerName();
	description.version = src.getVersion();
	description.fileOrIdentifier = src.getIdentifier();
	// TODO: fill it
	// description.lastFileModTime
	description.lastInfoUpdateTime = Time(src.getLastInfoUpdateTime());
	description.uid = src.getUid();
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

class JuceAndroidAudioPluginInstance : public juce::AudioPluginInstance
{
	AAPInstance *native;
	int sample_rate;

public:

	JuceAndroidAudioPluginInstance(AAPInstance *nativePlugin)
		: native(nativePlugin),
		  sample_rate(-1)
	{
	}

	const String getName() const override
	{
		return native->getPluginDescriptor()->getName();
	}
	
	void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override
	{
		sample_rate = sampleRate;
		native->prepareToPlay(sampleRate, maximumExpectedSamplesPerBlock);
		native->activate();
	}
	
	void releaseResources() override
	{
		native->dispose();
	}
	
	void fillNativeAudioBuffers(AndroidAudioPluginBuffer* dst, AudioBuffer<float>& buffer)
	{
		assert (dst->numBuffers == buffer.getNumChannels());
		for (int i = 0; i < dst->numBuffers; i++)
			dst->buffers[i] = (void*) buffer.getReadPointer(i);
	}
	
	void fillNativeMidiBuffers(AndroidAudioPluginBuffer* dst, MidiBuffer& buffer, int bufferIndex)
	{
		if (dst->numBuffers == 0)
			return;
		dst->buffers[bufferIndex] = (void*) buffer.data.getRawDataPointer();
	}
	
	void processBlock(AudioBuffer<float>& audioBuffer, MidiBuffer& midiMessages) override
	{
		AndroidAudioPluginBuffer buffer;
		void* buffers[audioBuffer.getNumChannels() + 1];
		fillNativeAudioBuffers(&buffer, audioBuffer);
		fillNativeMidiBuffers(&buffer, midiMessages, audioBuffer.getNumChannels());
		native->process(&buffer, 0);
	}
	
	double getTailLengthSeconds() const override
	{
		return native->getTailTimeInMilliseconds();
	}
	
	bool hasMidiPort(bool isInput) const
	{
		auto d = native->getPluginDescriptor();
		for (int i = 0; i < d->getNumPorts(); i++) {
			auto p = d->getPort(i);
			if (p->getPortDirection() == (isInput ? AAP_PORT_DIRECTION_INPUT : AAP_PORT_DIRECTION_OUTPUT) &&
			    p->getBufferType() == AAP_BUFFER_TYPE_CONTROL &&
			    p->isControlMidi())
				return true;
		}
		return false;
	}
	
	bool acceptsMidi() const override
	{
		return hasMidiPort(true);
	}
	
	bool producesMidi() const override
	{
		return hasMidiPort(false);
	}
	
	AudioProcessorEditor* createEditor() override
	{
		if (!native->getPluginDescriptor()->hasEditor())
			return nullptr;
		auto ret = new JuceAndroidAudioPluginEditor(this, native->createEditor());
		ret->startEditorUI();
		return ret;
	}
	
	bool hasEditor() const override
	{
		return native->getPluginDescriptor()->hasEditor();
	}
	
	int getNumPrograms() override
	{
		return native->getNumPrograms();
	}
	
	int getCurrentProgram() override
	{
		return native->getCurrentProgram();
	}
	
	void setCurrentProgram(int index) override
	{
		native->setCurrentProgram(index);
	}
	
	const String getProgramName(int index) override
	{
		return native->getProgramName(index);
	}
	
	void changeProgramName(int index, const String& newName) override
	{
		native->changeProgramName(index, newName.toUTF8());
	}
	
	void getStateInformation(juce::MemoryBlock& destData) override
	{
		int32_t size = native->getStateSize();
		destData.setSize(size);
		destData.copyFrom(native->getState(), 0, size);
	}
	
	void setStateInformation(const void* data, int sizeInBytes) override
	{
		native->setState(data, 0, sizeInBytes);
	}
	
	 void fillInPluginDescription(PluginDescription &description) const override
	 {
		auto src = native->getPluginDescriptor();
		fillPluginDescriptionFromNative(description, *src);
	 }
};

class JuceAndroidAudioPluginFormat : public juce::AudioPluginFormat
{
	AAPHost android_manager;
	HashMap<AAPDescriptor*,PluginDescription*> cached_descs;

	AAPDescriptor *findDescriptorFrom(const PluginDescription &desc)
	{
		auto it = cached_descs.begin();
		while (it.next())
			if (it.getValue()->uid == desc.uid)
				return it.getKey();
		return NULL;
	}


public:
	JuceAndroidAudioPluginFormat(AAssetManager *assetManager, const char* const* pluginAssetDirectories)
		: android_manager(AAPHost())
	{
		android_manager.initialize(assetManager, pluginAssetDirectories);
		
		for (int i = 0; i < android_manager.getNumPluginDescriptors(); i++) {
			auto d = android_manager.getPluginDescriptorAt(i);
			auto dst = new PluginDescription();
			fillPluginDescriptionFromNative(*dst, *d);
			cached_descs.set(d, dst);
		}
	}

	~JuceAndroidAudioPluginFormat()
	{
		auto it = cached_descs.begin();
		while (it.next())
			delete it.getValue();
		cached_descs.clear();
	}

	String getName() const
	{
		return "AAP";
	}
	
	void findAllTypesForFile(OwnedArray<PluginDescription>& results, const String& fileOrIdentifier)
	{
		auto id = fileOrIdentifier.toRawUTF8();
		// FIXME: everything is already cached, so this can be simplified.
		for (int i = 0; i < android_manager.getNumPluginDescriptors(); i++) {
			auto d = android_manager.getPluginDescriptorAt(i);
			if (strcmp(id, d->getName()) == 0 || strcmp(id, d->getIdentifier()) == 0) {
				auto dst = cached_descs [d];
				if (!dst)
					// doesn't JUCE handle invalid fileOrIdentifier?
					return;
				results.add(dst);
			}
		}
	}
	
	bool fileMightContainThisPluginType(const String &fileOrIdentifier)
	{
		auto f = File::createFileWithoutCheckingPath(fileOrIdentifier);
		return f.hasFileExtension(".aap");
	}
	
	String getNameOfPluginFromIdentifier(const String &fileOrIdentifier)
	{
		auto descriptor = android_manager.getPluginDescriptor(fileOrIdentifier.toRawUTF8());
		return descriptor != NULL ? String(descriptor->getName()) : String();
	}
	
	bool pluginNeedsRescanning(const PluginDescription &description)
	{
		return android_manager.isPluginUpToDate (description.fileOrIdentifier.toRawUTF8(), description.lastInfoUpdateTime.toMilliseconds());
	}
	
	bool doesPluginStillExist(const PluginDescription &description)
	{
		return android_manager.isPluginAlive (description.fileOrIdentifier.toRawUTF8());
	}
	
	bool canScanForPlugins() const
	{
		return true;
	}
	
	StringArray searchPathsForPlugins(const FileSearchPath &directoriesToSearch,
		bool recursive,
		bool allowPluginsWhichRequireAsynchronousInstantiation = false)
	{
		// regardless of whatever parameters this function is passed, it is
		// impossible to change the list of detected plugins.
		StringArray ret;
		for (int i = 0; i < android_manager.getNumPluginDescriptors(); i++)
			ret.add(android_manager.getPluginDescriptorAt(i)->getIdentifier());
		return ret;
	}
	
	FileSearchPath getDefaultLocationsToSearch()
	{
		const char **paths = android_manager.getDefaultPluginSearchPaths();
		StringArray arr(paths);
		String joined = arr.joinIntoString(":");
		FileSearchPath ret(joined);
		return ret;
	}

protected:
	void createPluginInstance(const PluginDescription &description,
		double initialSampleRate,
		int initialBufferSize,
		void *userData,
		PluginCreationCallback callback)
	{
		auto descriptor = findDescriptorFrom(description);
		String error("");
		if (descriptor == nullptr) {
			error << "Android Audio Plugin " << description.name << "was not found.";
			callback(userData, nullptr, error);
		} else {
			auto androidInstance = android_manager.instantiatePlugin(descriptor->getIdentifier());
			auto instance = new JuceAndroidAudioPluginInstance(androidInstance);
			callback(userData, instance, error);
		}
	}
	
	bool requiresUnblockedMessageThreadDuringCreation(const PluginDescription &description) const noexcept
	{
		// FIXME: implement correctly(?)
		return false;
	}
};


JuceAndroidAudioPluginInstance p(NULL);
JuceAndroidAudioPluginFormat f(NULL, NULL);

} // namespace