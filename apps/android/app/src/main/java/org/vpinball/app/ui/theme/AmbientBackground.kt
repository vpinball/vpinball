package org.vpinball.app.ui.theme

import androidx.compose.foundation.Canvas
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.MaterialTheme
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp

@Composable
fun AmbientBackground(modifier: Modifier = Modifier) {
    val dark = isDarkAppearance()
    val base = MaterialTheme.colorScheme.background

    if (!dark) {
        Canvas(modifier = modifier.fillMaxSize().background(base)) {}
        return
    }

    Canvas(modifier = modifier.fillMaxSize().background(base)) {
        val glows =
            listOf(
                Triple(Color.VpxRed.copy(alpha = 0.10f), Offset(size.width * 0.05f, size.height * 0.12f), 420.dp.toPx()),
                Triple(Color(0xFF3A2BFF).copy(alpha = 0.07f), Offset(size.width * 0.9f, size.height * 0.18f), 360.dp.toPx()),
                Triple(Color.VpxDarkYellow.copy(alpha = 0.05f), Offset(size.width * 0.9f, size.height * 0.95f), 480.dp.toPx()),
            )
        for ((color, center, radius) in glows) {
            drawCircle(
                brush = Brush.radialGradient(colors = listOf(color, Color.Transparent), center = center, radius = radius),
                radius = radius,
                center = center,
            )
        }
    }
}
