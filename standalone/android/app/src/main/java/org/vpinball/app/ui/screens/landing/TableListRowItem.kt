package org.vpinball.app.ui.screens.landing

import androidx.compose.animation.Crossfade
import androidx.compose.foundation.Image
import androidx.compose.foundation.gestures.detectTapGestures
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.aspectRatio
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
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
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import org.vpinball.app.R
import org.vpinball.app.Table
import org.vpinball.app.TableImageState
import org.vpinball.app.util.drawWithGradient
import org.vpinball.app.util.loadImage

@Composable
fun TableListRowItem(
    table: Table,
    onPlay: (table: Table) -> Unit,
    onRename: (table: Table) -> Unit,
    onTableImage: (table: Table) -> Unit,
    onViewScript: (table: Table) -> Unit,
    onShare: (table: Table) -> Unit,
    onReset: (table: Table) -> Unit,
    onDelete: (table: Table) -> Unit,
) {
    val focusManager = LocalFocusManager.current

    val contextMenuExpanded = remember { mutableStateOf(false) }
    var globalTouchOffset by remember { mutableStateOf(Offset.Zero) }

    val bitmap by
        produceState<ImageBitmap?>(null, table.uuid, table.image, table.modifiedAt) { value = withContext(Dispatchers.IO) { table.loadImage() } }

    val ratio = 2f / 3f
    val rowHeight = 120.dp

    val imageState by remember {
        derivedStateOf {
            if (bitmap != null) {
                TableImageState.IMAGE_LOADED
            } else {
                TableImageState.NO_IMAGE
            }
        }
    }

    Row(
        modifier = Modifier.fillMaxWidth().padding(8.dp).pointerInput(Unit) { detectTapGestures(onTap = { onPlay(table) }) },
        horizontalArrangement = Arrangement.spacedBy(15.dp),
        verticalAlignment = Alignment.CenterVertically,
    ) {
        Box {
            Box(
                modifier =
                    Modifier.height(rowHeight)
                        .aspectRatio(ratio)
                        .clip(RoundedCornerShape(8.dp))
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
                Crossfade(imageState, label = "table_grid_item_cross_fade") { imageState ->
                    when (imageState) {
                        TableImageState.IMAGE_LOADED -> {
                            bitmap?.let { bitmap ->
                                Box(modifier = Modifier.fillMaxSize()) {
                                    Image(
                                        bitmap = bitmap,
                                        contentDescription = null,
                                        modifier = Modifier.fillMaxSize(),
                                        contentScale = ContentScale.Fit,
                                        alignment = Alignment.Center,
                                    )
                                }
                            }
                        }

                        TableImageState.LOADING_IMAGE -> {
                            Box {}
                        }

                        else -> {
                            Box(modifier = Modifier.fillMaxSize()) {
                                Image(
                                    painter = painterResource(R.drawable.img_table_placeholder),
                                    contentDescription = null,
                                    modifier = Modifier.fillMaxSize().drawWithGradient(),
                                    contentScale = ContentScale.Fit,
                                    alignment = Alignment.Center,
                                )
                            }
                        }
                    }
                }
            }

            TableListItemDropdownMenu(
                table = table,
                expanded = contextMenuExpanded,
                onRename = { onRename(table) },
                onTableImage = { onTableImage(table) },
                onViewScript = { onViewScript(table) },
                onShare = { onShare(table) },
                onReset = { onReset(table) },
                onDelete = { onDelete(table) },
                offsetProvider = { globalTouchOffset },
            )
        }

        Box(modifier = Modifier.weight(1f).height(rowHeight), contentAlignment = Alignment.CenterStart) {
            Text(
                text = table.name,
                style = MaterialTheme.typography.titleMedium,
                color = Color.White,
                modifier = Modifier.padding(horizontal = 12.dp),
                maxLines = 3,
                overflow = TextOverflow.Ellipsis,
            )
        }
    }
}
