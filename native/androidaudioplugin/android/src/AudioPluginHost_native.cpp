
#include <jni.h>
#include <android/log.h>
#include <android/sharedmem_jni.h>
#include <cstdlib>
#include <sys/mman.h>
#include "aidl/org/androidaudioplugin/BnAudioPluginInterface.h"
#include "aidl/org/androidaudioplugin/BpAudioPluginInterface.h"
#include "aap/android-audio-plugin-host.hpp"
#include "audio-plugin-host-android.hpp"
#include "AudioPluginInterfaceImpl.h"
#include "aap/android-context.h"

extern "C" {

const char *strdup_fromJava(JNIEnv *env, jstring s) {
	jboolean isCopy;
	if (!s)
		return "";
	const char *u8 = env->GetStringUTFChars(s, &isCopy);
	auto ret = strdup(u8);
	env->ReleaseStringUTFChars(s, u8);
	return ret;
}

const char *java_plugin_information_class_name = "org/androidaudioplugin/PluginInformation",
		*java_port_information_class_name = "org/androidaudioplugin/PortInformation",
		*java_audio_plugin_host_helper_class_name = "org/androidaudioplugin/AudioPluginHostHelper";

static jmethodID
		j_method_is_out_process,
		j_method_get_package_name,
		j_method_get_local_name,
		j_method_get_name,
		j_method_get_manufacturer,
		j_method_get_version,
		j_method_get_plugin_id,
		j_method_get_shared_library_filename,
		j_method_get_library_entrypoint,
		j_method_get_port_count,
		j_method_get_port,
		j_method_port_get_name,
		j_method_port_get_direction,
		j_method_port_get_content,
		j_method_port_has_value_range,
		j_method_port_get_default,
		j_method_port_get_minimum,
		j_method_port_get_maximum,
		j_method_query_audio_plugins;

void initializeJNIMetadata(JNIEnv *env)
{
	jclass java_plugin_information_class = env->FindClass(java_plugin_information_class_name),
			java_port_information_class = env->FindClass(java_port_information_class_name),
			java_audio_plugin_host_helper_class = env->FindClass(java_audio_plugin_host_helper_class_name);

	j_method_is_out_process = env->GetMethodID(java_plugin_information_class,
											   "isOutProcess", "()Z");
	j_method_get_package_name = env->GetMethodID(java_plugin_information_class, "getPackageName",
													   "()Ljava/lang/String;");
	j_method_get_local_name = env->GetMethodID(java_plugin_information_class, "getLocalName",
												 "()Ljava/lang/String;");
	j_method_get_name = env->GetMethodID(java_plugin_information_class, "getDisplayName",
										 "()Ljava/lang/String;");
	j_method_get_manufacturer = env->GetMethodID(java_plugin_information_class,
												 "getManufacturer", "()Ljava/lang/String;");
	j_method_get_version = env->GetMethodID(java_plugin_information_class, "getVersion",
											"()Ljava/lang/String;");
	j_method_get_plugin_id = env->GetMethodID(java_plugin_information_class, "getPluginId",
											  "()Ljava/lang/String;");
	j_method_get_shared_library_filename = env->GetMethodID(java_plugin_information_class,
															"getSharedLibraryName",
															"()Ljava/lang/String;");
	j_method_get_library_entrypoint = env->GetMethodID(java_plugin_information_class,
													   "getLibraryEntryPoint",
													   "()Ljava/lang/String;");
	j_method_get_port_count = env->GetMethodID(java_plugin_information_class,
											   "getPortCount", "()I");
	j_method_get_port = env->GetMethodID(java_plugin_information_class, "getPort",
										 "(I)Lorg/androidaudioplugin/PortInformation;");
	j_method_port_get_name = env->GetMethodID(java_port_information_class, "getName",
											  "()Ljava/lang/String;");
	j_method_port_get_direction = env->GetMethodID(java_port_information_class,
												   "getDirection", "()I");
	j_method_port_get_content = env->GetMethodID(java_port_information_class, "getContent",
												 "()I");
	j_method_port_has_value_range = env->GetMethodID(java_port_information_class, "getHasValueRange",
												 "()Z");
	j_method_port_get_default = env->GetMethodID(java_port_information_class, "getDefault",
													 "()F");
	j_method_port_get_minimum = env->GetMethodID(java_port_information_class, "getMinimum",
												 "()F");
	j_method_port_get_maximum = env->GetMethodID(java_port_information_class, "getMaximum",
												 "()F");
	j_method_query_audio_plugins = env->GetStaticMethodID(java_audio_plugin_host_helper_class, "queryAudioPlugins",
														  "(Landroid/content/Context;)[Lorg/androidaudioplugin/PluginInformation;");
}

const char* keepPointer(std::vector<const char*> freeList, const char* ptr) {
	freeList.emplace_back(ptr);
	return ptr;
}

aap::PluginInformation *
pluginInformation_fromJava(JNIEnv *env, jobject pluginInformation) {
	std::vector<const char*> freeList{};
	auto aapPI = new aap::PluginInformation(
			env->CallBooleanMethod(pluginInformation, j_method_is_out_process),
			keepPointer(freeList, strdup_fromJava(env, (jstring) env->CallObjectMethod(pluginInformation,
																 j_method_get_package_name))),
			keepPointer(freeList, strdup_fromJava(env, (jstring) env->CallObjectMethod(pluginInformation,
																 j_method_get_local_name))),
			keepPointer(freeList, strdup_fromJava(env, (jstring) env->CallObjectMethod(pluginInformation,
																 j_method_get_name))),
			keepPointer(freeList, strdup_fromJava(env, (jstring) env->CallObjectMethod(pluginInformation,
																 j_method_get_manufacturer))),
			keepPointer(freeList, strdup_fromJava(env, (jstring) env->CallObjectMethod(pluginInformation,
																 j_method_get_version))),
			keepPointer(freeList, strdup_fromJava(env, (jstring) env->CallObjectMethod(pluginInformation,
																 j_method_get_plugin_id))),
			keepPointer(freeList, strdup_fromJava(env, (jstring) env->CallObjectMethod(pluginInformation,
																 j_method_get_shared_library_filename))),
			keepPointer(freeList, strdup_fromJava(env, (jstring) env->CallObjectMethod(pluginInformation,
																 j_method_get_library_entrypoint))),
			"" // no use on Android
	);
	for (auto p : freeList)
		free((void*) p);

	int nPorts = env->CallIntMethod(pluginInformation, j_method_get_port_count);
	for (int i = 0; i < nPorts; i++) {
		jobject port = env->CallObjectMethod(pluginInformation, j_method_get_port, i);
		auto name = strdup_fromJava(env, (jstring) env->CallObjectMethod(port, j_method_port_get_name));
		auto content = (aap::ContentType) (int) env->CallIntMethod(port, j_method_port_get_content);
		auto direction = (aap::PortDirection) (int) env->CallIntMethod(port, j_method_port_get_direction);
		aapPI->addPort(
			env->CallBooleanMethod(port, j_method_port_has_value_range) ?
			new aap::PortInformation(name, content, direction,
					env->CallFloatMethod(port, j_method_port_get_default),
					env->CallFloatMethod(port, j_method_port_get_minimum),
					env->CallFloatMethod(port, j_method_port_get_maximum)) :
			new aap::PortInformation(name, content, direction));
		free((void*) name);
	}

	return aapPI;
}


// FIXME: I haven't figured out how it results in crashes yet. Plugins must be set in Kotlin layer so far.
jobjectArray queryInstalledPluginsJNI()
{
	auto apal = dynamic_cast<aap::AndroidPluginHostPAL*>(aap::getPluginHostPAL());
	auto env = apal->getJNIEnv();

	jclass java_audio_plugin_host_helper_class = env->FindClass(java_audio_plugin_host_helper_class_name);
	j_method_query_audio_plugins = env->GetStaticMethodID(java_audio_plugin_host_helper_class, "queryAudioPlugins",
														  "(Landroid/content/Context;)[Lorg/androidaudioplugin/PluginInformation;");
	return (jobjectArray) env->CallStaticObjectMethod(java_audio_plugin_host_helper_class, j_method_query_audio_plugins, aap::get_android_application_context());
}


std::shared_ptr<aidl::org::androidaudioplugin::BnAudioPluginInterface> sp_binder;

jobject
Java_org_androidaudioplugin_AudioPluginNatives_createBinderForService(JNIEnv *env, jclass clazz, jint sampleRate) {
    sp_binder = ndk::SharedRefBase::make<aap::AudioPluginInterfaceImpl>();
    auto ret = AIBinder_toJavaBinder(env, sp_binder->asBinder().get());
    return ret;
}

void
Java_org_androidaudioplugin_AudioPluginNatives_destroyBinderForService(JNIEnv *env, jclass clazz,
                                                                      jobject binder) {
    auto abinder = AIBinder_fromJavaBinder(env, binder);
    AIBinder_decStrong(abinder);
    sp_binder.reset();
}

void Java_org_androidaudioplugin_AudioPluginNatives_initializeLocalHost(JNIEnv *env, jclass cls, jobjectArray jPluginInfos)
{
    // FIXME: enable later code once queryInstalledPluginsJNI() is fixed.
    assert(jPluginInfos != nullptr);
	//if (jPluginInfos == nullptr)
	//	jPluginInfos = aap::queryInstalledPluginsJNI();
	auto apal = dynamic_cast<aap::AndroidPluginHostPAL*>(aap::getPluginHostPAL());
	apal->initializeKnownPlugins(jPluginInfos);
}

void Java_org_androidaudioplugin_AudioPluginNatives_cleanupLocalHostNatives(JNIEnv *env, jclass cls)
{
}

JNIEXPORT void JNICALL
Java_org_androidaudioplugin_AudioPluginNatives_setApplicationContext(JNIEnv *env, jclass clazz,
																  jobject applicationContext) {
	auto apal = dynamic_cast<aap::AndroidPluginHostPAL*>(aap::getPluginHostPAL());
	apal->initialize(env, applicationContext);
    initializeJNIMetadata(env);
}
JNIEXPORT void JNICALL
Java_org_androidaudioplugin_AudioPluginNatives_initialize(JNIEnv *env, jclass clazz,
                                                       jobjectArray jPluginInfos) {
	auto apal = dynamic_cast<aap::AndroidPluginHostPAL*>(aap::getPluginHostPAL());
    apal->initializeKnownPlugins(jPluginInfos);
}

JNIEXPORT void JNICALL
Java_org_androidaudioplugin_AudioPluginNatives_addBinderForHost(JNIEnv *env, jclass clazz,
                                                                jstring packageName, jstring className, jobject binder) {
	const char *packageNameDup = strdup_fromJava(env, packageName);
	const char *classNameDup = strdup_fromJava(env, className);
	auto binderRef = env->NewGlobalRef(binder);
	auto aiBinder = AIBinder_fromJavaBinder(env, binder);
	auto apal = dynamic_cast<aap::AndroidPluginHostPAL*>(aap::getPluginHostPAL());
    apal->serviceConnections.push_back(aap::AudioPluginServiceConnection(packageNameDup, classNameDup, binderRef, aiBinder));
	free((void*) packageNameDup);
	free((void*) classNameDup);
}

JNIEXPORT void JNICALL
Java_org_androidaudioplugin_AudioPluginNatives_removeBinderForHost(JNIEnv *env, jclass clazz,
                                                                   jstring packageName, jstring className) {
	const char *packageNameDup = strdup_fromJava(env, packageName);
	const char *classNameDup = strdup_fromJava(env, className);
	auto apal = dynamic_cast<aap::AndroidPluginHostPAL*>(aap::getPluginHostPAL());
	auto c = apal->serviceConnections.begin();
	while (c != apal->serviceConnections.end()) {
		c = std::next(c);
		if (c->packageName == packageNameDup && c->className == classNameDup)
			apal->serviceConnections.erase(c);
	}
	free((void*) packageNameDup);
	free((void*) classNameDup);
}

JNIEXPORT int JNICALL
Java_org_androidaudioplugin_AudioPluginNatives_getSharedMemoryFD(JNIEnv *env, jclass clazz, jobject shm) {
	return ASharedMemory_dupFromJava(env, shm);
}

JNIEXPORT void JNICALL
Java_org_androidaudioplugin_AudioPluginNatives_closeSharedMemoryFD(JNIEnv *env, jclass clazz, int fd) {
	close(fd);
}
} // extern "C"
