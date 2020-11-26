import androidx.compose.desktop.Window
import androidx.compose.ui.unit.IntSize
import org.androidaudioplugin.samples.aapcomposehostsample.AAPHostSample2App
import org.androidaudioplugin.samples.aapcomposehostsample.AAPHostSampleState

fun updateAudioPluginServices(state: AAPHostSampleState) {
    state.availablePluginServices.clear()
    state.availablePluginServices.addAll(AudioPluginHostHelper.queryAudioPluginServices())
}

class Driver {
    fun main() = Window(title = "Compose for Desktop", size = IntSize(300, 300)) {
        var state = AAPHostSampleState()
        updateAudioPluginServices(state)
        AAPHostSample2App()
    }
}


