package org.androidaudioplugin.aaphostsample2

import android.content.Context
import androidx.compose.Model
import org.androidaudioplugin.AudioPluginHostHelper
import org.androidaudioplugin.AudioPluginServiceInformation

@Model
object AAPHostSampleState {

    var currentMainTab = 0

    var availablePluginServices = mutableListOf<AudioPluginServiceInformation>()

    fun updateAudioPluginServices(context: Context) {
        availablePluginServices.clear()
        availablePluginServices.addAll(AudioPluginHostHelper.queryAudioPluginServices(context))
    }
}

/*

aaphostsample2

Model
	AvailablePlugins
	PluginGraph

PluginGraph
	Connections : listOf<PluginConnection>()
	Inputs : PluginGraphNode
	Outputs : PluginGraphNode

PluginConnection
	Input
	Output
	ChannelConnections : mapOf<PluginChannel, PluginChannel>()

PluginGraphNode
	AudioSources : listOf<AudioSource>()
	MidiSources : listOf<MidiSource>()
	AudioDestinations : listOf<AudioDestination>()
	MidiDestinations : listOf<MidiDestination>()

AudioSource
	>: AudioBuffer
	>: PluginInput
	>: AudioDeviceInput
MidiSource
	>: MidiBuffer
	>: MidiDeviceInput
AudioDestination
	>: PluginOutput

plugin list

connection list

sample inputs list

- audio in -> audio out
  - sample files
  - audio device inputs
- midi files


 */