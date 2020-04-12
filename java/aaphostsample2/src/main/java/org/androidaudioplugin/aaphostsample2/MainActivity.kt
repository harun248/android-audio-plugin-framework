package org.androidaudioplugin.aaphostsample2

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.compose.Composable
import androidx.compose.remember
import androidx.compose.state
import androidx.ui.core.*
import androidx.ui.foundation.*
import androidx.ui.graphics.Paint
import androidx.ui.graphics.PaintingStyle
import androidx.ui.layout.*
import androidx.ui.material.*
import androidx.ui.text.TextStyle
import androidx.ui.tooling.preview.Preview
import androidx.ui.unit.dp
import androidx.ui.unit.sp
import androidx.ui.unit.toRect
import org.androidaudioplugin.PluginInformation

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
    Surface {
        ModalPanelLayout(
            bodyContent = { HomeScreen() }
        )
    }
}

enum class ModalPanelState {
    None,
    AddPluginConnection,
    ShowPluginDetails
}

@Composable
fun ModalPanelLayout(
    bodyContent: @Composable() () -> Unit
) {
    Box(Modifier.fillMaxSize()) {
        Stack {
            bodyContent()

            val panelContent = @Composable() {
                when (AAPHostSampleState.modalState) {
                    ModalPanelState.AddPluginConnection -> {
                        AvailablePlugins(onItemClick = { plugin ->
                            AAPHostSampleState.pluginGraph.nodes.add(AAPPluginGraphNode(plugin))
                            AAPHostSampleState.modalState = ModalPanelState.None
                        })
                    }
                    ModalPanelState.ShowPluginDetails -> {
                        PluginDetails()
                    }
                }
            }

            if (AAPHostSampleState.modalState != ModalPanelState.None) {
                Scrim()
                Box(
                    padding = PluginListPanelPadding
                ) {
                    // remove Container when we will support multiply children
                    Surface {
                        Box(Modifier.fillMaxSize(), children = panelContent)
                    }
                }
            }
        }
    }
}

// Taken from Drawer.kt in androidx.ui/ui-material and modified.  begin
private const val ScrimDefaultOpacity = 0.32f
private val PluginListPanelPadding = 36.dp

@Composable
private fun Scrim() {
    val scrimContent = @Composable {
        val paint = remember { Paint().apply { style = PaintingStyle.fill } }
        val color = MaterialTheme.colors.onSurface
        Canvas(Modifier.fillMaxSize()) {
            val scrimAlpha = ScrimDefaultOpacity
            paint.color = color.copy(alpha = scrimAlpha)
            drawRect(size.toRect(), paint)
        }
    }
    if (AAPHostSampleState.modalState != ModalPanelState.None) {
        Clickable(onClick = { AAPHostSampleState.modalState = ModalPanelState.None }, children = scrimContent)
    } else {
        scrimContent()
    }
}
// end

@Composable
fun HomeScreen(
    scaffoldState: ScaffoldState = remember { ScaffoldState() }
) {
    Scaffold(
        scaffoldState = scaffoldState,
        topAppBar = {
            TopAppBar(
                title = {
                    Text(
                        text = "AAPHostSample2",
                        style = MaterialTheme.typography.subtitle2.copy(color = contentColor())
                    )
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
                            0 -> Rack()
                            1 -> AvailablePlugins(onItemClick = { p ->
                                AAPHostSampleState.selectedPluginDetails = p
                                AAPHostSampleState.modalState = ModalPanelState.ShowPluginDetails
                            })
                        }
                    }
                }
            }
        }
    )
}

@Composable
fun Rack() {
    Stack {
        VerticalScroller {
            Column {
                Row {
                    Button(onClick = {
                        AAPHostSampleState.modalState = ModalPanelState.AddPluginConnection
                    }) {
                        Text("Add")
                    }
                }
                RackConnections()
            }
        }
    }
}

@Composable
fun RackConnections() {
    val small = TextStyle(fontSize = 12.sp)
    AAPHostSampleState.pluginGraph.nodes.forEach { n ->
        Row(
            modifier = Modifier.padding(start = 16.dp, end = 16.dp)
        ) {
            Clickable(onClick = {}) {
                Column(modifier = Modifier.weight(1f)) {
                    Text(n.displayName)
                }
            }
        }
    }
}

@Composable
fun AvailablePlugins(onItemClick: (PluginInformation) -> Unit = {}) {
    VerticalScroller {
        Column {
            val small = TextStyle(fontSize = 12.sp)

            AAPHostSampleState.availablePluginServices.forEach { s ->
                s.plugins.forEach { p ->
                    Row(
                        modifier = Modifier.padding(start = 16.dp, end = 16.dp)
                    ) {
                        Clickable(onClick = { onItemClick(p) }) {
                            Column(modifier = Modifier.weight(1f)) {
                                Text(p.displayName)
                                Text(s.packageName, style = small)
                            }
                        }
                    }
                }
            }
        }
    }
}

@Composable
fun PluginDetails() {
    val plugin = AAPHostSampleState.selectedPluginDetails!!
    VerticalScroller {
        Column {
            Row { Text(plugin.displayName) }
            Row {
                Text("package: ")
                Text(plugin.packageName)
            }
            Row {
                Text("classname: ")
                Text(plugin.localName)
            }
            if (plugin.author != null)
                Row {
                    Text("author: ")
                    Text(plugin.author!!)
                }
            if (plugin.backend != null)
                Row {
                    Text("backend: ")
                    Text(plugin.backend!!)
                }
            if (plugin.manufacturer != null)
                Row {
                    Text("manfufacturer: ")
                    Text(plugin.manufacturer!!)
                }
        }
    }
}

@Preview
@Composable
fun DefaultPreview() {
    AAPHostSample2App()
}