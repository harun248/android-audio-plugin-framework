package org.androidaudioplugin.samples.aapcomposehostsample

//import android.content.Context
import android.content.Context
import org.androidaudioplugin.AudioPluginHostHelper
import org.androidaudioplugin.AudioPluginServiceInformation
import org.androidaudioplugin.PluginInformation

fun updateAudioPluginServices(context: Context, state: AAPHostSampleState) {
    state.availablePluginServices.clear()
    state.availablePluginServices.addAll(AudioPluginHostHelper.queryAudioPluginServices(context))
}
