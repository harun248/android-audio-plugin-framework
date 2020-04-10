package org.androidaudioplugin.aaphostsample2

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.compose.Composable
import androidx.compose.emptyContent
import androidx.compose.remember
import androidx.compose.state
import androidx.ui.core.Modifier
import androidx.ui.core.setContent
import androidx.ui.foundation.*
import androidx.ui.foundation.shape.corner.RoundedCornerShape
import androidx.ui.graphics.Color
import androidx.ui.layout.*
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

    var (state, onStateChange) = state { DrawerState.Closed }
    Surface {
        ModalDrawerLayout(
            drawerState = state,
            onStateChange = onStateChange,
            drawerContent = {
                VerticalScroller {
                    Column {
                        AvailablePlugins()
                    }
                }
            },
            bodyContent = {
                HomeScreen(onStateChange)
            })
    }
}


@Composable
fun HomeScreen(onStateChange: (DrawerState) -> Unit, scaffoldState: ScaffoldState = remember { ScaffoldState() }) {
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
                    listOf("Rack", "Plugins", "Inputs", "Settings"),
                    AAPHostSampleState.currentMainTab
                ) { index, title ->
                    Tab(
                        text = { Text(title) },
                        selected = AAPHostSampleState.currentMainTab == index,
                        onSelected = { AAPHostSampleState.currentMainTab = index }
                    )
                }
                VerticalScroller {
                    Column {
                        when (AAPHostSampleState.currentMainTab) {
                            0 -> Rack(onStateChange)
                            1 -> AvailablePlugins()
                        }
                    }
                }
            }
        }
    )
}

@Composable
fun Rack(onStateChange: (DrawerState) -> Unit) {
    Button(onClick = { onStateChange(DrawerState.Opened) }) {
        Text("Add")
    }
    RackConnections()
}

@Composable
fun RackConnections() {
    val small = TextStyle(fontSize = 12.sp)
    AAPHostSampleState.pluginGraph.nodes.forEach { n ->
        Row(
            modifier = Modifier.padding(start = 16.dp, end = 16.dp)
        ) {
            Column(modifier = Modifier.weight(1f)) {
                Text(n.displayName)
            }
        }
    }
}

@Composable
fun AvailablePlugins() {
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