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

@Composable
fun Modifier.drawWithGradient(
    brush: Brush = remember {
        Brush.linearGradient(
            colors =
                listOf(
                    Color(0xFF555555),
                    Color(0xFF777777),
                    Color(0xFFBBBBBB),
                    Color(0xFFFFFFFF),
                    Color(0xFFBBBBBB),
                    Color(0xFF777777),
                    Color(0xFF555555),
                ),
            start = Offset(0f, 0f),
            end = Offset.Infinite,
        )
    }
): Modifier =
    then(
        Modifier.graphicsLayer(alpha = 0.99f).drawWithCache {
            onDrawWithContent {
                drawContent()
                drawRect(brush, blendMode = BlendMode.SrcAtop)
            }
        }
    )
