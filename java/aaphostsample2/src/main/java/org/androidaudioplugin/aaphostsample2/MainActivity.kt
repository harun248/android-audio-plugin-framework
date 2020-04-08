package org.androidaudioplugin.aaphostsample2

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.compose.Composable
import androidx.compose.emptyContent
import androidx.compose.remember
import androidx.ui.core.Modifier
import androidx.ui.core.setContent
import androidx.ui.foundation.*
import androidx.ui.layout.Column
import androidx.ui.layout.Row
import androidx.ui.layout.padding
import androidx.ui.material.*
import androidx.ui.material.icons.Icons
import androidx.ui.material.icons.filled.ArrowBack
import androidx.ui.text.TextStyle
import androidx.ui.tooling.preview.Preview
import androidx.ui.unit.dp
import androidx.ui.unit.sp

class MainActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        setContent {
            AAPHostSampleState.updateAudioPluginServices(applicationContext)
            AAPHostSample2App()
        }
    }

}

@Composable
fun AAPHostSample2App() {
    MaterialTheme { AAPHostSample2AppContent() }
}

@Composable
fun AAPHostSample2AppContent() {
    Surface { HomeScreen() }
}

@Composable
fun HomeScreen(scaffoldState: ScaffoldState = remember { ScaffoldState() }) {
    Scaffold(
        scaffoldState = scaffoldState,
        topAppBar = {
            TopAppBar(
                title = {
                    Text(
                        text = "AAPHostSample2",
                        style = MaterialTheme.typography.subtitle2.copy(color = contentColor())
                    )
                },
                navigationIcon = {
                    IconButton(onClick = { /*FIXME: implement*/ }) {
                        Icon(Icons.Filled.ArrowBack)
                    }
                }
            )
        },
        bodyContent = {
            Column {
                TabRow(
                    listOf("Connections", "Inputs", "Settings"),
                    AAPHostSampleState.currentMainTab
                ) { index, title ->
                    Tab(
                        text = { Text(title) },
                        selected = AAPHostSampleState.currentMainTab == index,
                        onSelected = { emptyContent() }
                    )
                }
                VerticalScroller {
                    Column {
                        when (AAPHostSampleState.currentMainTab) {
                            0 -> ListAvailablePlugins()
                        }
                    }
                }
            }
        }
    )
}

@Composable
fun ListAvailablePlugins() {
    val small = TextStyle(fontSize = 12.sp)
    AAPHostSampleState.availablePluginServices.forEach { s ->
        s.plugins.forEach { p ->
            Row(
                modifier = Modifier.padding(start = 16.dp, end = 16.dp)
            ) {
                Column(modifier = Modifier.weight(1f)) {
                    Text(p.displayName)
                    Text(s.packageName, style = small)
                }
            }
        }
    }
}

@Preview
@Composable
fun DefaultPreview() {
    AAPHostSample2App()
}