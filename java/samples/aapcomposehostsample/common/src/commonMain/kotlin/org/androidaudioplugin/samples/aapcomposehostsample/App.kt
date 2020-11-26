package org.androidaudioplugin.samples.aapcomposehostsample

import androidx.compose.foundation.ScrollableColumn
import androidx.compose.foundation.clickable
import androidx.compose.foundation.contentColor
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumnFor
import androidx.compose.material.*
import androidx.compose.runtime.Composable
import androidx.compose.runtime.remember
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.TextStyle
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.ui.tooling.preview.Preview

@Composable
fun AAPHostSample2App() {
    MaterialTheme { AAPHostSample2AppContent() }
}

@ExperimentalMaterialApi
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
        bodyContent()

        val panelContent = @Composable() {
            when (aapHostSampleState.value.modalState) {
                ModalPanelState.AddPluginConnection -> {
                    AvailablePlugins(onItemClick = { plugin ->
                        aapHostSampleState.value.pluginGraph.nodes.add(AAPPluginGraphNode(plugin))
                        aapHostSampleState.value.modalState = ModalPanelState.None
                    })
                }
                ModalPanelState.ShowPluginDetails -> {
                    PluginDetails()
                }
            }
        }

        if (aapHostSampleState.value.modalState != ModalPanelState.None) {
            //Scrim()
            Box(Modifier.padding(PluginListPanelPadding)) {
                // remove Container when we will support multiply children
                Surface {
                    Box(Modifier.fillMaxSize(), Alignment.CenterStart) { panelContent }
                }
            }
        }
    }
}

// Taken from Drawer.kt in androidx.ui/ui-material and modified.  begin
private const val ScrimDefaultOpacity = 0.32f
private val PluginListPanelPadding = 36.dp

/*
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
    if (aapHostSampleState.value.modalState != ModalPanelState.None) {
        Clickable(onClick = { aapHostSampleState.value.modalState = ModalPanelState.None }, children = scrimContent)
    } else {
        scrimContent()
    }
}
// end
*/

@ExperimentalMaterialApi
@Composable
fun HomeScreen(
    scaffoldState: ScaffoldState = remember { ScaffoldState(drawerState = DrawerState(DrawerValue.Closed, DefaultAnimationClock()), SnackbarHostState()) }
) {
    Scaffold(
        scaffoldState = scaffoldState,
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
                TabRow(
                    aapHostSampleState.value.currentMainTab
                ) {
                    val index = aapHostSampleState.value.currentMainTab
                    Tab(aapHostSampleState.value.currentMainTab == index,
                        {},
                        Modifier.offset(0.dp), // Why is this parameter mandatory!?
                        { Text(arrayOf("Rack", "Plugins", "Src/Dst") [index]) },
                        { aapHostSampleState.value.currentMainTab = index }
                    )
                }
                    ScrollableColumn {
                        when (aapHostSampleState.value.currentMainTab) {
                            0 -> Rack()
                            1 -> AvailablePlugins(onItemClick = { p ->
                                aapHostSampleState.value.selectedPluginDetails = p
                                aapHostSampleState.value.modalState = ModalPanelState.ShowPluginDetails
                            })
                        }
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
                        aapHostSampleState.value.modalState = ModalPanelState.AddPluginConnection
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
    aapHostSampleState.value.pluginGraph.nodes.forEach { n ->
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
    ScrollableColumn {
        Column {
            val small = TextStyle(fontSize = 12.sp)

            LazyColumnFor(aapHostSampleState.value.availablePluginServices.flatMap { s -> s.plugins }) { p ->
                Row(
                    modifier = Modifier.padding(start = 16.dp, end = 16.dp)
                        .then(Modifier.clickable { onItemClick(p) })
                ) {
                    Column(modifier = Modifier.weight(1f)) {
                        Text(p.displayName)
                        Text(p.service.packageName, style = small)
                    }
                }
            }
        }
    }
}

@Composable
fun PluginDetails() {
    val plugin = aapHostSampleState.value.selectedPluginDetails!!
    ScrollableColumn(modifier = Modifier.padding(12.dp)) {
        Column {
            Row {
                if (plugin.iconResource != 0) {
                    // FIXME: Android specific.
                    val image = loadImageResource(id = plugin.iconResource)
                    if (image.resource.resource != null)
                        image(image.resource.resource!!, modifier = Modifier.preferredHeightIn(max = 120.dp))
                }
                Text(plugin.displayName)
            }
            LazyColumnFor(plugin) {
                Row {
                    Text("package: ")
                    Text(plugin.packageName)
                }
                Row {
                    Text("classname: ")
                    Text(plugin.localName)
                }
                if (plugin.author != null) {
                    Row {
                        Text("author: ")
                        Text(plugin.author)
                    }
                }
                if (plugin.backend != null) {
                    Row {
                        Text("backend: ")
                        Text(plugin.backend)
                    }
                }
                if (plugin.manufacturer != null) {
                    Row {
                        Text("manfufacturer: ")
                        Text(plugin.manufacturer)
                    }
                }
            }
            val modifier = Modifier.padding(6.dp)
            Text("Ports")
            LazyColumnFor(plugin) {
                for (port in plugin.ports) {
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
fun DefaultPreview() {
    AAPHostSample2App()
}
