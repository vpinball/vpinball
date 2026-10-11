package org.vpinball.app.util

import androidx.compose.animation.core.animateFloatAsState
import androidx.compose.animation.core.tween
import androidx.compose.foundation.clickable
import androidx.compose.foundation.interaction.MutableInteractionSource
import androidx.compose.foundation.interaction.collectIsPressedAsState
import androidx.compose.material3.MaterialTheme
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.remember
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.drawBehind
import androidx.compose.ui.draw.drawWithCache
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.BlendMode
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.graphicsLayer
import org.vpinball.app.ui.theme.isDarkAppearance

@Composable
fun Modifier.drawWithGradient(dark: Boolean = isDarkAppearance()): Modifier {
    val brush =
        remember(dark) {
            val colors =
                if (dark) {
                    listOf(0xFF555555, 0xFF777777, 0xFFBBBBBB, 0xFFFFFFFF, 0xFFBBBBBB, 0xFF777777, 0xFF555555)
                } else {
                    listOf(0xFF8E8E93, 0xFF6E6E73, 0xFF48484C, 0xFF6E6E73, 0xFF8E8E93)
                }
            Brush.linearGradient(colors = colors.map { Color(it) }, start = Offset(0f, 0f), end = Offset.Infinite)
        }
    return then(
        Modifier.graphicsLayer(alpha = 0.99f).drawWithCache {
            onDrawWithContent {
                drawContent()
                drawRect(brush, blendMode = BlendMode.SrcAtop)
            }
        }
    )
}

@Composable
fun Modifier.cellClickable(enabled: Boolean = true, onClick: () -> Unit): Modifier {
    val interactionSource = remember { MutableInteractionSource() }
    val pressed by interactionSource.collectIsPressedAsState()
    val alpha = animateFloatAsState(if (pressed) 1f else 0f, tween(if (pressed) 0 else 150), label = "cell_press")
    val color = MaterialTheme.colorScheme.onSurface.copy(alpha = 0.1f)
    return drawBehind { drawRect(color.copy(alpha = color.alpha * alpha.value)) }
        .clickable(interactionSource = interactionSource, indication = null, enabled = enabled, onClick = onClick)
}
