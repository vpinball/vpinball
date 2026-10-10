package org.vpinball.app.util

import androidx.compose.runtime.Composable
import androidx.compose.runtime.remember
import androidx.compose.ui.Modifier
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
