#!/bin/bash

ANDROID_HOME=~/Android/Sdk
BUILD_TOOLS_VERSION=29.0.1
THIS_DIR=`dirname $(readlink -f $0)`
AIDL_DIR=$THIS_DIR
AIDL_TOOL=$ANDROID_HOME/build-tools/$BUILD_TOOLS_VERSION/aidl

$AIDL_TOOL --lang=ndk \
	-o $THIS_DIR/../native/aap-android/src/gen \
	-h $THIS_DIR/../native/aap-android/src/gen/include \
	org/androidaudioplugin/AudioPluginInterface.aidl
$AIDL_TOOL --lang=java \
	-o $THIS_DIR/../java/androidaudioplugin/src/main/gen org/androidaudioplugin/AudioPluginInterface.aidl
