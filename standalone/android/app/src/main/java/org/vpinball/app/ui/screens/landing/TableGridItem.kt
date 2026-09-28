package org.vpinball.app.ui.screens.landing

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
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import dev.chrisbanes.haze.HazeInput
import dev.chrisbanes.haze.blur.HazeBlurStyle
import dev.chrisbanes.haze.blur.HazeColorEffect
import dev.chrisbanes.haze.blur.hazeBlur
import dev.chrisbanes.haze.hazeSource
import dev.chrisbanes.haze.rememberHazeState
import org.vpinball.app.Table

@Composable
fun TableGridItem(
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

    val hazeState = rememberHazeState()

    Box {
        Box(
            modifier =
                Modifier.fillMaxSize()
                    .aspectRatio(2f / 3f)
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
            Column(modifier = Modifier.hazeSource(hazeState)) { TableImageView(table = table) }

            Box(
                modifier =
                    Modifier.align(Alignment.BottomCenter)
                        .fillMaxWidth()
                        .clip(RoundedCornerShape(6.dp))
                        .hazeBlur(
                            input = HazeInput.Sources(hazeState),
                            style =
                                HazeBlurStyle {
                                    blurRadius(40.dp)
                                    backgroundColor(Color.Black)
                                    colorEffects(listOf(HazeColorEffect.tint(Color.White.copy(alpha = 0.1f))))
                                    noiseFactor(0.15f)
                                },
                        )
            ) {
                Column(
                    modifier = Modifier.fillMaxWidth().padding(horizontal = 7.dp, vertical = 5.dp),
                    verticalArrangement = Arrangement.spacedBy(2.dp),
                    horizontalAlignment = Alignment.CenterHorizontally,
                ) {
                    Text(text = table.name, color = Color.White, textAlign = TextAlign.Center, style = MaterialTheme.typography.titleSmall)
                }
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
