package org.androidaudioplugin.samples.aaphostdogfooding

import androidx.compose.foundation.Canvas
import androidx.compose.foundation.ScrollableColumn
import androidx.compose.foundation.clickable
import androidx.compose.foundation.contentColor
import androidx.compose.foundation.layout.*
import androidx.compose.runtime.State
import androidx.compose.foundation.lazy.LazyColumnFor
import androidx.compose.material.*
import androidx.compose.runtime.Composable
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.TextStyle
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.ui.tooling.preview.Preview
import org.androidaudioplugin.PluginInformation
import org.androidaudioplugin.PortInformation
import androidx.compose.runtime.livedata.observeAsState
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.geometry.Size
import androidx.compose.ui.geometry.toRect
import androidx.compose.ui.graphics.Paint
import androidx.compose.ui.graphics.PaintingStyle

@Composable
@ExperimentalMaterialApi
fun AAPHostSample2App() {
    MaterialTheme { AAPHostSample2AppContent() }
}

@Composable
@ExperimentalMaterialApi
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
        bodyContent()

        val currentState = aapHostSampleViewModel.modalState.observeAsState(ModalPanelState.None)
        val pluginGraph = aapHostSampleViewModel.pluginGraph.observeAsState()
        val panelContent = @Composable() {
            when (currentState.value) {
                ModalPanelState.AddPluginConnection -> {
                    AvailablePlugins(onItemClick = { plugin ->
                        pluginGraph.value!!.nodes.add(AAPPluginGraphNode(plugin))
                        aapHostSampleViewModel.onModalStateChanged(ModalPanelState.None)
                    })
                }
                ModalPanelState.ShowPluginDetails -> {
                    PluginDetails()
                }
            }
        }

        if (currentState.value != ModalPanelState.None) {
            Scrim()
            Box(Modifier.padding(PluginListPanelPadding)) {
                // remove Container when we will support multiply children
                Surface {
                    Box(Modifier.fillMaxSize(), Alignment.CenterStart) { panelContent() }
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
        val paint = remember { Paint().apply { style = PaintingStyle.Fill } }
        val color = MaterialTheme.colors.onSurface
        Canvas(Modifier.fillMaxSize()) {
            drawRect(color.copy(alpha = ScrimDefaultOpacity))
        }
    }
    Surface(Modifier.clickable(onClick = { aapHostSampleViewModel.onModalStateChanged(ModalPanelState.None) })) {
        scrimContent()
    }
}
//end

@ExperimentalMaterialApi
@Composable
fun HomeScreen(
) {
    Scaffold(
        topBar = {
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
                val (selectedTab, setSelectedTab) = remember { mutableStateOf(0) }
                TabRow(
                    selectedTab
                ) {
                    arrayOf("Rack", "Plugins", "Src/Dst").forEachIndexed {tabIndex, label ->
                        Tab(selectedTab == tabIndex,
                            { setSelectedTab(tabIndex) },
                            text = { Text(label) }
                        )
                    }
                }
                when (selectedTab) {
                    0 -> Rack()
                    1 -> AvailablePlugins(onItemClick = { p ->
                        aapHostSampleViewModel.onSelectedPluginDetailsChanged(p)
                        aapHostSampleViewModel.onModalStateChanged(ModalPanelState.ShowPluginDetails)
                    })
                }
            }
        }
    )
}

@Composable
fun Rack() {
    Box {
        ScrollableColumn {
            Column {
                Row {
                    Button(onClick = {
                        aapHostSampleViewModel.onModalStateChanged(ModalPanelState.AddPluginConnection)
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
    aapHostSampleViewModel.pluginGraph.value!!.nodes.forEach { n ->
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
fun AvailablePlugins(onItemClick: (PluginInformation) -> Unit = {}) {
    val small = TextStyle(fontSize = 12.sp)

    val pluginsState = aapHostSampleViewModel.availablePluginServices.observeAsState()
    LazyColumnFor(pluginsState.value!!.flatMap { s -> s.plugins }) { p ->
        Row(
            modifier = Modifier.padding(start = 16.dp, end = 16.dp)
                .then(Modifier.clickable { onItemClick(p) })
        ) {
            Column(modifier = Modifier.weight(1f)) {
                Text(p.displayName)
                Text(p.packageName, style = small)
            }
        }
    }
}

@Composable
fun PluginDetails() {
    val plugin = aapHostSampleViewModel.selectedPluginDetails!!
    ScrollableColumn(modifier = Modifier.padding(12.dp)) {
        Column {
            Row {
                /* We have no decision on its specification here yet...
                if (plugin.iconResource != 0) {
                    // FIXME: Android specific.
                    val image = loadImageResource(id = plugin.iconResource)
                    if (image.resource.resource != null)
                        image(image.resource.resource!!, modifier = Modifier.preferredHeightIn(max = 120.dp))
                }
                */
                Text(plugin.value!!.displayName)
            }
            Row {
                Column {
                    Row {
                        Text("package: ")
                        Text(plugin.value!!.packageName)
                    }
                    Row {
                        Text("classname: ")
                        Text(plugin.value!!.localName)
                    }
                    if (plugin.value!!.author != null) {
                        Row {
                            Text("author: ")
                            Text(plugin.value!!.author ?: "")
                        }
                    }
                    if (plugin.value!!.backend != null) {
                        Row {
                            Text("backend: ")
                            Text(plugin.value!!.backend ?: "")
                        }
                    }
                    if (plugin.value!!.manufacturer != null) {
                        Row {
                            Text("manfufacturer: ")
                            Text(plugin.value!!.manufacturer ?: "")
                        }
                    }
                }
            }
            val modifier = Modifier.padding(6.dp)
            Text("Ports")
            Column {
                for (port in plugin.value!!.ports) {
                    Row {
                        Text(text = port.name, modifier = modifier)
                        Text(text = when (port.content) {
                            PortInformation.PORT_CONTENT_TYPE_AUDIO-> "Audio"
                            PortInformation.PORT_CONTENT_TYPE_MIDI-> "MIDI"
                            else -> "-" }, modifier = modifier)
                        Text(text = when (port.direction) {
                            PortInformation.PORT_DIRECTION_INPUT -> "In"
                            else -> "Out"}, modifier = modifier)
                    }
                }
            }
        }
    }
}

@Preview("Default Preview")
@Composable
@ExperimentalMaterialApi
fun DefaultPreview() {
    AAPHostSample2App()
}
