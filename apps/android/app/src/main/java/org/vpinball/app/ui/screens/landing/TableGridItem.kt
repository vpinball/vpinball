package org.vpinball.app.ui.screens.landing

import androidx.compose.animation.core.animateFloatAsState
import androidx.compose.foundation.gestures.detectTapGestures
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.draw.scale
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.layout.onSizeChanged
import androidx.compose.ui.platform.LocalFocusManager
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import org.vpinball.app.Table
import org.vpinball.app.TableManager

@Composable
fun TableGridItem(
    table: Table,
    onPlay: (table: Table) -> Unit,
    onRename: (table: Table) -> Unit,
    onSetImage: (table: Table) -> Unit,
    onViewScript: (table: Table) -> Unit,
    onShare: (table: Table) -> Unit,
    onResetImage: (table: Table) -> Unit,
    onResetSettings: (table: Table) -> Unit,
    onDelete: (table: Table) -> Unit,
    modifier: Modifier = Modifier,
    inRecentlyPlayed: Boolean = false,
) {
    val focusManager = LocalFocusManager.current

    val contextMenuExpanded = remember { mutableStateOf(false) }
    var touchOffset by remember { mutableStateOf(Offset.Zero) }
    var anchorHeight by remember { mutableIntStateOf(0) }
    var pressed by remember { mutableStateOf(false) }
    val scale by animateFloatAsState(if (pressed) 0.95f else 1f, label = "card_scale")

    Box(modifier = modifier.onSizeChanged { anchorHeight = it.height }) {
        Column(
            modifier =
                Modifier.fillMaxWidth().scale(scale).alpha(if (pressed) 0.8f else 1f).pointerInput(Unit) {
                    detectTapGestures(
                        onPress = {
                            pressed = true
                            tryAwaitRelease()
                            pressed = false
                        },
                        onTap = { onPlay(table) },
                        onLongPress = { offset ->
                            focusManager.clearFocus()
                            touchOffset = offset
                            contextMenuExpanded.value = true
                        },
                    )
                },
            verticalArrangement = Arrangement.spacedBy(8.dp),
        ) {
            TableImageView(table = table)

            Text(
                text = table.name,
                style = MaterialTheme.typography.bodyMedium,
                fontWeight = FontWeight.Medium,
                color = MaterialTheme.colorScheme.onBackground,
                maxLines = 3,
                overflow = TextOverflow.Ellipsis,
                modifier = Modifier.padding(horizontal = 2.dp),
            )
        }

        TableContextMenu(
            table = table,
            expanded = contextMenuExpanded,
            onToggleFavorite = { TableManager.getInstance().toggleFavorite(table) },
            onRemoveFromRecent = if (inRecentlyPlayed) ({ TableManager.getInstance().clearPlayed(table) }) else null,
            onRename = { onRename(table) },
            onSetImage = { onSetImage(table) },
            onViewScript = { onViewScript(table) },
            onShare = { onShare(table) },
            onResetImage = { onResetImage(table) },
            onResetSettings = { onResetSettings(table) },
            onDelete = { onDelete(table) },
            offsetProvider = { Offset(touchOffset.x, touchOffset.y - anchorHeight) },
        )
    }
}
