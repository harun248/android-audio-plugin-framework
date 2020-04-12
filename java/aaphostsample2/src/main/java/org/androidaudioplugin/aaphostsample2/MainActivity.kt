package org.androidaudioplugin.aaphostsample2

import android.os.Bundle
import androidx.animation.AnimatedFloat
import androidx.appcompat.app.AppCompatActivity
import androidx.compose.Composable
import androidx.compose.emptyContent
import androidx.compose.remember
import androidx.compose.state
import androidx.ui.core.*
import androidx.ui.foundation.*
import androidx.ui.foundation.shape.corner.RoundedCornerShape
import androidx.ui.graphics.Color
import androidx.ui.graphics.Paint
import androidx.ui.graphics.PaintingStyle
import androidx.ui.layout.*
import androidx.ui.material.*
import androidx.ui.material.icons.Icons
import androidx.ui.material.icons.filled.ArrowBack
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

    var (state, onStateChange) = state { PluginListPanelState.None }
    Surface {
        ModalPluginListPanelLayout(
            visibilityState = state,
            onStateChange = onStateChange,
            panelContent = {
                VerticalScroller {
                    Column {
                        AvailablePlugins(onItemClick = { plugin ->
                            AAPHostSampleState.pluginGraph.nodes.add(AAPPluginGraphNode(plugin))
                            onStateChange(PluginListPanelState.None)
                        })
                    }
                }
            },
            bodyContent = {
                HomeScreen(onStateChange)
            })
    }
}

enum class PluginListPanelState {
    None,
    Visible
}

@Composable
fun ModalPluginListPanelLayout(
    visibilityState: PluginListPanelState,
    onStateChange: (PluginListPanelState) -> Unit,
    panelContent: @Composable() () -> Unit,
    bodyContent: @Composable() () -> Unit
) {
    Box(Modifier.fillMaxSize()) {
        Stack {
            bodyContent()
            if (visibilityState == PluginListPanelState.Visible) {
                Scrim(visibilityState, onStateChange)
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
private fun Scrim(
    state: PluginListPanelState,
    onStateChange: (PluginListPanelState) -> Unit // FIXME: change from DrawerState to something else (PluginListState?)
) {
    val scrimContent = @Composable {
        val paint = remember { Paint().apply { style = PaintingStyle.fill } }
        val color = MaterialTheme.colors.onSurface
        Canvas(Modifier.fillMaxSize()) {
            val scrimAlpha = ScrimDefaultOpacity
            paint.color = color.copy(alpha = scrimAlpha)
            drawRect(size.toRect(), paint)
        }
    }
    if (state == PluginListPanelState.Visible) {
        Clickable(onClick = { onStateChange(PluginListPanelState.None) }, children = scrimContent)
    } else {
        scrimContent()
    }
}
// end

@Composable
fun HomeScreen(onStateChange: (PluginListPanelState) -> Unit, scaffoldState: ScaffoldState = remember { ScaffoldState() }) {
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
fun Rack(onPluginListDrawerStateChange: (PluginListPanelState) -> Unit) {
    Button(onClick = { onPluginListDrawerStateChange(PluginListPanelState.Visible) }) {
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
    val small = TextStyle(fontSize = 12.sp)

    AAPHostSampleState.availablePluginServices.forEach { s ->
        s.plugins.forEach { p ->
            Row(
                modifier = Modifier.padding(start = 16.dp, end = 16.dp)
            ) {
                Clickable(onClick = {onItemClick(p) }) {
                    Column(modifier = Modifier.weight(1f)) {
                        Text(p.displayName)
                        Text(s.packageName, style = small)
                    }
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