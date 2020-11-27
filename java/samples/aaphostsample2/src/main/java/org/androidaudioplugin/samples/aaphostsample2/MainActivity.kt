package org.androidaudioplugin.samples.aaphostsample2

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.compose.material.ExperimentalMaterialApi
import androidx.compose.ui.platform.setContent
import org.androidaudioplugin.samples.aapcomposehostsample.AAPHostSample2App

class MainActivity : AppCompatActivity() {

    @ExperimentalMaterialApi
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        setContent {
            AAPHostSample2App()
        }
    }
}
