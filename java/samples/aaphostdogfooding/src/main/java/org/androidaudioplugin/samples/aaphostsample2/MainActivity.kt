package org.androidaudioplugin.samples.aaphostdogfooding

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.compose.material.ExperimentalMaterialApi
import androidx.compose.ui.platform.setContent
import org.androidaudioplugin.AudioPluginHostHelper

class MainActivity : AppCompatActivity() {

    @ExperimentalMaterialApi
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        val services = AudioPluginHostHelper.queryAudioPluginServices(applicationContext)
        aapHostSampleViewModel.availablePluginServices.value!!.addAll(services)

        setContent {
            AAPHostSample2App()
        }
    }
}
