#pragma once

#ifndef _ANDROID_AUDIO_PLUGIN_HOST_HPP_
#define _ANDROID_AUDIO_PLUGIN_HOST_HPP_

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <dlfcn.h>
#include <time.h>
#include <vector>
#include "android/asset_manager.h"
#include "android-audio-plugin.h"


namespace aap
{

class PluginInstance;
class AAPEditor;

enum ContentType {
	AAP_CONTENT_TYPE_UNDEFINED,
	AAP_CONTENT_TYPE_AUDIO,
	AAP_CONTENT_TYPE_MIDI,
};

enum PortDirection {
	AAP_PORT_DIRECTION_INPUT,
	AAP_PORT_DIRECTION_OUTPUT
};

enum PluginInstantiationState {
	PLUGIN_INSTANTIATION_STATE_UNPREPARED,
	PLUGIN_INSTANTIATION_STATE_INACTIVE,
	PLUGIN_INSTANTIATION_STATE_ACTIVE,
	PLUGIN_INSTANTIATION_STATE_TERMINATED,
};

class PortInformation
{
	const char *name;
	ContentType content_type;
	PortDirection direction;
	
public:
	const char* getName() const { return name; }
	ContentType getContentType() const { return content_type; }
	PortDirection getPortDirection() const { return direction; }
};

#define safe_strdup(s) ((const char*) s ? strdup(s) : NULL)

#define SAFE_FREE(s) if (s) { free((void*) s); s = NULL; }

class PluginInformation
{
	const char *name;
	const char *manufacturer_name;
	const char *version;
	const char *identifier_string;
	const char *shared_library_filename;
	const char *plugin_id;
	long last_info_updated_unixtime;

	/* NULL-terminated list of categories */
	const char *primary_category;
	/* NULL-terminated list of ports */
	std::vector<const PortInformation*> ports;
	/* NULL-terminated list of required extensions */
	std::vector<const AndroidAudioPluginExtension *> required_extensions;
	/* NULL-terminated list of optional extensions */
	std::vector<const AndroidAudioPluginExtension *> optional_extensions;

	// hosting information
	bool is_out_process;

public:
	/* In VST3 world, they are like "Effect", "Synth", "Instrument|Synth", "Fx|Delay" ... can be anything. Here we list typical-looking ones */
	const char * PRIMARY_CATEGORY_EFFECT = "Effect";
	const char * PRIMARY_CATEGORY_SYNTH = "Synth";
	
	PluginInformation(bool isOutProcess, char* pluginName, char* manufacturerName, char* versionString, char* pluginID, char* sharedLibraryFilename)
		: is_out_process(isOutProcess),
		  name(safe_strdup(pluginName)),
		  manufacturer_name(safe_strdup(manufacturerName)),
		  version(safe_strdup(versionString)),
		  plugin_id(safe_strdup(pluginID)),
		  shared_library_filename(safe_strdup(sharedLibraryFilename)),
		  last_info_updated_unixtime((long) time(NULL))
	{
		char *cp;
		int len = sprintf(NULL, "%s+%s+%s", name, plugin_id, version);
		cp = (char*) malloc(len);
		sprintf(cp, "%s+%s+%s", name, plugin_id, version);
		identifier_string = (const char*) cp;
	}
	
	~PluginInformation()
	{
		SAFE_FREE(name)
		SAFE_FREE(manufacturer_name)
		SAFE_FREE(version)
		SAFE_FREE(plugin_id)
		SAFE_FREE(shared_library_filename)
		SAFE_FREE(identifier_string)
	}
	
	const char* getName() const
	{
		return name;
	}
	
	const char* getManufacturerName() const
	{
		return manufacturer_name;
	}
	
	const char* getVersion() const
	{
		return version;
	}
	
	/* locally identifiable string.
	 * It is combination of name+unique_id+version, to support plugin debugging. */
	const char* getIdentifier() const
	{
		return identifier_string;
	}
	
	const char* getPrimaryCategory() const
	{
		return primary_category;
	}
	
	int32_t getNumPorts() const
	{
		return ports.size();
	}
	
	const PortInformation *getPort(int32_t index) const
	{
		return ports[index];
	}
	
	int32_t getNumRequiredExtensions() const
	{
		return required_extensions.size();
	}
	
	const AndroidAudioPluginExtension *getRequiredExtension(int32_t index) const
	{
		return required_extensions[index];
	}
	
	int32_t getNumOptionalExtensions() const
	{
		return optional_extensions.size();
	}
	
	const AndroidAudioPluginExtension *getOptionalExtension(int32_t index) const
	{
		return optional_extensions[index];
	}
	
	long getLastInfoUpdateTime() const
	{
		return last_info_updated_unixtime;
	}
	
	/* unique identifier across various environment */
	const char* getPluginID() const
	{
		return plugin_id;
	}
	
	bool isInstrument() const
	{
		// The purpose of this function seems to be based on hacky premise. So do we.
		return strstr(getPrimaryCategory(), "Instrument") != NULL;
	}
	
	bool hasSharedContainer() const
	{
		// TODO: FUTURE (v0.6) It may be something AAP should support because
		// context switching over outprocess plugins can be quite annoying...
		return false;
	}
	
	bool hasEditor() const
	{
		// TODO: FUTURE (v0.4)
		return false;
	}

	const char* getLocalPluginSharedLibrary() const
	{
		// By metadata or inferred.
		// Since Android expects libraries stored in `lib` directory,
		// it will just return the name of the shared library.
		return shared_library_filename;
	}
	
	bool isOutProcess() const
	{
		return is_out_process;
	}
};

class EditorInstance
{
	const PluginInstance *owner;

public:

	EditorInstance(const PluginInstance *owner)
		: owner(owner)
	{
		// TODO: FUTURE (v0.4)
	}

	void startEditorUI()
	{
		// TODO: FUTURE (v0.4)
	}
};

class PluginHostBackend
{
};

class PluginHostBackendLV2 : public PluginHostBackend
{
};

class PluginHostBackendVST3 : public PluginHostBackend
{
};

class PluginHost
{
	PluginHostBackendLV2 backend_lv2;
	PluginHostBackendLV2 backend_vst3;

	AAssetManager *asset_manager;
	std::vector<const PluginHostBackend*> backends;
	
	std::vector<const PluginInformation*> plugin_descriptors;
	
	PluginInformation* loadDescriptorFromAssetBundleDirectory(const char *directory);
	PluginInstance* instantiateLocalPlugin(const PluginInformation *descriptor);
	PluginInstance* instantiateRemotePlugin(const PluginInformation *descriptor);

public:

	PluginHost(AAssetManager* assetManager, const PluginInformation* const* pluginDescriptors);

	~PluginHost()
	{
			for (int i = 0; i < getNumPluginDescriptors(); i++)
				free((void*) getPluginDescriptorAt(i));
	}
	
	bool isPluginAlive (const char *identifier);
	
	bool isPluginUpToDate (const char *identifier, long lastInfoUpdated);
	
	void addHostBackend(const PluginHostBackend *backend)
	{
		assert(backend != NULL);
		backends.push_back(backend);
	}
	
	int32_t getNumHostBackends()
	{
		return backends.size();
	}

	const PluginHostBackend* getHostBackend(int index)
	{
		return backends[index];
	}
	
	int32_t getNumPluginDescriptors()
	{
		return plugin_descriptors.size();
	}

	const PluginInformation* getPluginDescriptorAt(int index)
	{
		return plugin_descriptors[index];
	}
	
	const PluginInformation* getPluginDescriptor(const char *identifier)
	{
		for(int i = 0; i < getNumPluginDescriptors(); i++) {
			auto d = getPluginDescriptorAt(i);
			if (strcmp(d->getIdentifier(), identifier) == 0)
				return d;
		}
		return NULL;
	}
	
	PluginInstance* instantiatePlugin(const char* identifier);
	
	EditorInstance* createEditor(PluginInstance* instance)
	{
		return new EditorInstance(instance);
	}
};

class PluginInstance
{
	friend class PluginHost;
	
	PluginHost *host;
	const PluginInformation *descriptor;
	AndroidAudioPlugin *plugin;
	AAPHandle *instance;
	const AndroidAudioPluginExtension * const *extensions;
	PluginInstantiationState plugin_state;

	PluginInstance(PluginHost* host, const PluginInformation* pluginDescriptor, AndroidAudioPlugin* loadedPlugin)
		: host(host),
		  descriptor(pluginDescriptor),
		  plugin(loadedPlugin),
		  instance(NULL),
		  extensions(NULL),
		  plugin_state(PLUGIN_INSTANTIATION_STATE_UNPREPARED)
	{
	}
	
public:

	const PluginInformation* getPluginDescriptor()
	{
		return descriptor;
	}

	void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock)
	{
		assert(plugin_state == PLUGIN_INSTANTIATION_STATE_UNPREPARED);
		
		instance = plugin->instantiate(plugin, sampleRate, extensions);
		plugin->prepare(instance);
		plugin_state = PLUGIN_INSTANTIATION_STATE_INACTIVE;
	}
	
	void activate()
	{
		if (plugin_state == PLUGIN_INSTANTIATION_STATE_ACTIVE)
			return;
		assert(plugin_state == PLUGIN_INSTANTIATION_STATE_INACTIVE);
		
		plugin->activate(instance);
		plugin_state = PLUGIN_INSTANTIATION_STATE_ACTIVE;
	}
	
	void deactivate()
	{
		if (plugin_state == PLUGIN_INSTANTIATION_STATE_INACTIVE)
			return;
		assert(plugin_state == PLUGIN_INSTANTIATION_STATE_ACTIVE);
		
		plugin->deactivate(instance);
		plugin_state = PLUGIN_INSTANTIATION_STATE_INACTIVE;
	}
	
	void dispose()
	{
		if (instance != NULL)
			plugin->terminate(instance);
		instance = NULL;
		plugin_state = PLUGIN_INSTANTIATION_STATE_TERMINATED;
	}
	
	void process(AndroidAudioPluginBuffer *buffer, int32_t timeoutInNanoseconds)
	{
		// It is not a TODO here, but if pointers have changed, we have to reconnect
		// LV2 ports.
		plugin->process(instance, buffer, timeoutInNanoseconds);
	}
	
	EditorInstance* createEditor()
	{
		return host->createEditor(this);
	}
	
	int getNumPrograms()
	{
		// TODO: FUTURE (v0.6). LADSPA does not support it either.
		return 0;
	}
	
	int getCurrentProgram()
	{
		// TODO: FUTURE (v0.6). LADSPA does not support it either.
		return 0;
	}
	
	void setCurrentProgram(int index)
	{
		// TODO: FUTURE (v0.6). LADSPA does not support it, but resets all parameters.
	}
	
	const char * getProgramName(int index)
	{
		// TODO: FUTURE (v0.6). LADSPA does not support it either.
		return NULL;
	}
	
	void changeProgramName(int index, const char * newName)
	{
		// TODO: FUTURE (v0.6). LADSPA does not support it either.
	}
	
	int32_t getStateSize()
	{
		return plugin->get_state(instance)->data_size;
	}
	
	void const* getState()
	{
		return plugin->get_state(instance)->raw_data;
	}
	
	void setState(const void* data, int32_t offset, int32_t sizeInBytes)
	{
		AndroidAudioPluginState state;
		state.data_size = sizeInBytes;
		state.raw_data = data;
		plugin->set_state(instance, &state);
	}
	
	uint32_t getTailTimeInMilliseconds()
	{
		// TODO: FUTURE (v0.6) - most likely just a matter of plugin property
		return 0;
	}
};

} // namespace

#endif // _ANDROID_AUDIO_PLUGIN_HOST_HPP_

