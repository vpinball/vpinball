package org.vpinball.app.ui.screens.landing

import androidx.compose.animation.Crossfade
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.gestures.detectTapGestures
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.aspectRatio
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.derivedStateOf
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.produceState
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.ImageBitmap
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.layout.LayoutCoordinates
import androidx.compose.ui.layout.onGloballyPositioned
import androidx.compose.ui.platform.LocalFocusManager
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import dev.chrisbanes.haze.HazeDefaults
import dev.chrisbanes.haze.HazeState
import dev.chrisbanes.haze.HazeTint
import dev.chrisbanes.haze.haze
import dev.chrisbanes.haze.hazeChild
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import org.vpinball.app.ArtworkState
import org.vpinball.app.R
import org.vpinball.app.VPinballManager
import org.vpinball.app.data.entity.PinTable
import org.vpinball.app.util.drawWithGradient
import org.vpinball.app.util.hasArtwork
import org.vpinball.app.util.loadArtwork

@Composable
fun TableListGridItem(
    table: PinTable,
    onPlay: (table: PinTable) -> Unit,
    onRename: (table: PinTable) -> Unit,
    onChangeArtwork: (table: PinTable) -> Unit,
    onViewScript: (table: PinTable) -> Unit,
    onShare: (table: PinTable) -> Unit,
    onReset: (table: PinTable) -> Unit,
    onDelete: (table: PinTable) -> Unit,
) {
    val focusManager = LocalFocusManager.current

    val contextMenuExpanded = remember { mutableStateOf(false) }
    var globalTouchOffset by remember { mutableStateOf(Offset.Zero) }
    val bitmap by produceState<ImageBitmap?>(null, table) { value = withContext(Dispatchers.IO) { table.loadArtwork() } }

    val artworkState by remember {
        derivedStateOf {
            if (bitmap != null) {
                ArtworkState.IMAGE_LOADED
            } else {
                if (table.hasArtwork()) {
                    ArtworkState.LOADING_IMAGE
                } else {
                    ArtworkState.NO_IMAGE
                }
            }
        }
    }

    val hazeState = remember { HazeState() }

    val ratio = VPinballManager.getDisplaySize().width.toFloat() / VPinballManager.getDisplaySize().height.toFloat()

    Box {
        Box(
            modifier =
                Modifier.fillMaxSize()
                    .aspectRatio(ratio)
                    .border(width = 2.dp, color = Color.Yellow, shape = RoundedCornerShape(8.dp))
                    .clip(RoundedCornerShape(8.dp))
                    .padding(all = 4.dp)
                    .pointerInput(Unit) {
                        detectTapGestures(
                            onTap = { onPlay(table) },
                            onLongPress = { offset ->
                                focusManager.clearFocus()
                                globalTouchOffset = offset
                                contextMenuExpanded.value = true
                            },
                        )
                    }
                    .onGloballyPositioned { layoutCoordinates: LayoutCoordinates ->
                        globalTouchOffset = layoutCoordinates.localToRoot(globalTouchOffset)
                    }
        ) {
            Column(modifier = Modifier.haze(state = hazeState)) {
                Crossfade(artworkState, label = "table_grid_item_cross_fade") { artworkState ->
                    when (artworkState) {
                        ArtworkState.IMAGE_LOADED -> {
                            bitmap?.let { bitmap ->
                                Image(bitmap, contentDescription = null, modifier = Modifier.fillMaxSize(), contentScale = ContentScale.Crop)
                            }
                        }

                        ArtworkState.LOADING_IMAGE -> {
                            Box(modifier = Modifier.background(Color.Black))
                        }

                        else -> {
                            Image(
                                painter = painterResource(R.drawable.img_table_placeholder),
                                contentDescription = null,
                                modifier = Modifier.fillMaxSize().drawWithGradient(),
                                contentScale = ContentScale.Crop,
                            )
                        }
                    }
                }
            }

            Box(
                modifier =
                    Modifier.align(Alignment.BottomCenter).fillMaxWidth().padding(all = 2.dp).clip(RoundedCornerShape(6.dp)).hazeChild(
                        state = hazeState
                    ) {
                        backgroundColor = Color.Black
                        tints = listOf(HazeTint(Color.White.copy(alpha = 0.1f)))
                        blurRadius = 40.dp
                        noiseFactor = HazeDefaults.noiseFactor
                    }
            ) {
                Column(
                    modifier = Modifier.fillMaxWidth().padding(2.dp),
                    verticalArrangement = Arrangement.spacedBy(2.dp),
                    horizontalAlignment = Alignment.CenterHorizontally,
                ) {
                    Text(text = table.name, color = Color.White, textAlign = TextAlign.Center, style = MaterialTheme.typography.titleSmall)
                }
            }
        }

        TableListItemDropdownMenu(
            table = table,
            expanded = contextMenuExpanded,
            onRename = { onRename(table) },
            onChangeArtwork = { onChangeArtwork(table) },
            onViewScript = { onViewScript(table) },
            onShare = { onShare(table) },
            onReset = { onReset(table) },
            onDelete = { onDelete(table) },
            offsetProvider = { globalTouchOffset },
        )
    }
}
