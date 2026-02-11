package org.vpinball.app.ui.screens.landing

import androidx.compose.foundation.gestures.detectTapGestures
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.aspectRatio
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.layout.LayoutCoordinates
import androidx.compose.ui.layout.onGloballyPositioned
import androidx.compose.ui.platform.LocalFocusManager
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import org.vpinball.app.Table

@Composable
fun TableRowItem(
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

    val rowHeight = 120.dp

    Box {
        Row(
            modifier =
                Modifier.fillMaxWidth()
                    .padding(8.dp)
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
                    },
            horizontalArrangement = Arrangement.spacedBy(15.dp),
            verticalAlignment = Alignment.CenterVertically,
        ) {
            Box(modifier = Modifier.height(rowHeight).aspectRatio(2f / 3f).clip(RoundedCornerShape(8.dp))) { TableImageView(table = table) }

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

        TableContextMenu(
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
}
