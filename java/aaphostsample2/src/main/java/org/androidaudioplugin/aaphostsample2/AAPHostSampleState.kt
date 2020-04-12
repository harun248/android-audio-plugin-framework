package org.androidaudioplugin.aaphostsample2

import android.content.Context
import androidx.compose.Model
import androidx.compose.frames.ModelList
import org.androidaudioplugin.AudioPluginHostHelper
import org.androidaudioplugin.AudioPluginServiceInformation
import org.androidaudioplugin.PluginInformation

@Model
object AAPHostSampleState {

    var currentMainTab = 0

    var pluginGraph = PluginGraph()

    var availableAudioDataSources = AudioDataSourceSet()

    var availablePluginServices = ModelList<AudioPluginServiceInformation>()

    fun updateAudioPluginServices(context: Context) {
        availablePluginServices.clear()
        availablePluginServices.addAll(AudioPluginHostHelper.queryAudioPluginServices(context))
    }
}

typealias PluginConnection = Pair<PluginPort, PluginPort>

class PluginGraph {
    var systemAudioIn = SystemAudioInNode()
    var systemAudioOut = SystemAudioOutNode()
    var systemMidiIn = SystemMidiInNode()
    var systemMidiOut = SystemMidiOutNode()
    var nodes = ModelList<PluginGraphNode>()
    var connections = ModelList<PluginConnection>()

    constructor() {
        nodes.add(systemAudioIn)
        nodes.add(systemMidiIn)
        nodes.add(systemAudioOut)
        nodes.add(systemMidiOut)
        connections.add(PluginConnection(PluginPort(systemAudioIn, 0), PluginPort(systemAudioOut, 0)))
        connections.add(PluginConnection(PluginPort(systemAudioIn, 1), PluginPort(systemAudioOut, 1)))
    }
}

abstract class PluginGraphNode {
    abstract val displayName : String
    abstract var inPorts : List<PluginPort>
    abstract var outPorts : List<PluginPort>

    val audioSources
        get() = inPorts.filter { p -> p.isAudio }
    val audioDestinations
        get() = outPorts.filter { p -> p.isAudio }
    val midiSources
        get() = inPorts.filter { p -> p.isMidi }
    val midiDestinations
        get() = outPorts.filter { p -> p.isMidi }
}

class AAPPluginGraphNode(var plugin: PluginInformation) : PluginGraphNode() {

    override val displayName = plugin.displayName
    override var inPorts = listOf<PluginPort>()
    override var outPorts = listOf<PluginPort>()
}

class SystemAudioInNode : PluginGraphNode() {
    override val displayName = "System Audio In"
    override var inPorts = listOf<PluginPort>() // empty
    override var outPorts = listOf<PluginPort>()
}

class SystemAudioOutNode : PluginGraphNode() {
    override val displayName = "System Audio Out"
    override var inPorts = listOf<PluginPort>()
    override var outPorts = listOf<PluginPort>() // empty
}

class SystemMidiInNode : PluginGraphNode() {
    override val displayName = "System MIDI In"
    override var inPorts = listOf<PluginPort>() // empty
    override var outPorts = listOf<PluginPort>()
}

class SystemMidiOutNode : PluginGraphNode() {
    override val displayName = "System MIDI Out"
    override var inPorts = listOf<PluginPort>()
    override var outPorts = listOf<PluginPort>() // empty
}

class PluginPort(var plugin: PluginGraphNode, var port: Int) {
    val isAudio : Boolean
        get() = throw NotImplementedError()
    val isMidi : Boolean
        get() = throw NotImplementedError()
}


class AudioDataSourceSet {
    var audioSources = listOf<AudioSource>()
    var midiSources = listOf<MidiSource>()
    var audioDestinations = listOf<AudioDestination>()
    var midiDestinations = listOf<MidiDestination>()
}

open class AudioSource {
}

class AudioBufferSource {
    constructor(buffer: ByteArray) {
    }
}

class AudioInput : AudioSource() {

}

class AudioDeviceInput : AudioSource() {

}

open class AudioDestination {

}

class AudioBufferDestination {
    constructor(buffer: ByteArray) {
    }
}

class AudioDeviceOutput : AudioDestination() {

}


open class MidiSource {

}

class MidiBufferInput {
    constructor(buffer: ByteArray) {
    }
}

class MidiDeviceInput : MidiSource() {

}

open class MidiDestination {

}

class MidiBufferOutput {
    constructor(buffer: ByteArray) {
    }
}

class MidiDeviceOutput : MidiDestination() {

}
