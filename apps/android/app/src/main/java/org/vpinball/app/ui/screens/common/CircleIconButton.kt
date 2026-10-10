package org.vpinball.app.ui.screens.common

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.Shape
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.unit.dp
import dev.chrisbanes.haze.HazeInput
import dev.chrisbanes.haze.HazeProgressive
import dev.chrisbanes.haze.HazeState
import dev.chrisbanes.haze.blur.HazeBlurStyle
import dev.chrisbanes.haze.blur.HazeColorEffect
import dev.chrisbanes.haze.blur.hazeBlur

val CircleButtonSize = 44.dp

@Composable
fun Modifier.glassPill(hazeState: HazeState?, shape: Shape = CircleShape): Modifier {
    val surface = MaterialTheme.colorScheme.surface
    val tint = MaterialTheme.colorScheme.surfaceContainerHigh.copy(alpha = 0.6f)
    val outline = MaterialTheme.colorScheme.onSurface.copy(alpha = 0.12f)
    val base =
        if (hazeState != null) {
            clip(shape)
                .hazeBlur(
                    input = HazeInput.Sources(hazeState),
                    style =
                        HazeBlurStyle {
                            blurRadius(24.dp)
                            backgroundColor(surface)
                            colorEffects(listOf(HazeColorEffect.tint(tint)))
                            noiseFactor(0.1f)
                        },
                )
        } else {
            clip(shape).background(MaterialTheme.colorScheme.surfaceContainerHigh)
        }
    return base.border(1.dp, outline, shape)
}

@Composable
fun Modifier.frostedEdge(hazeState: HazeState, alpha: Float, solidFraction: Float = 0.5f): Modifier {
    val surface = MaterialTheme.colorScheme.surface
    val tint = surface.copy(alpha = 0.55f)
    return hazeBlur(
        input = HazeInput.Sources(hazeState),
        style =
            HazeBlurStyle {
                blurRadius(30.dp)
                backgroundColor(surface)
                colorEffects(listOf(HazeColorEffect.tint(tint)))
                noiseFactor(0.05f)
                alpha(alpha)
                mask(Brush.verticalGradient(solidFraction to Color.Black, 1f to Color.Transparent))
                progressive(HazeProgressive.verticalGradient(startIntensity = 1f, endIntensity = 0f))
            },
    )
}

@Composable
fun CircleIconButton(iconRes: Int, contentDescription: String, hazeState: HazeState? = null, modifier: Modifier = Modifier, onClick: () -> Unit) {
    IconButton(onClick = onClick, modifier = modifier.size(CircleButtonSize).glassPill(hazeState)) {
        Icon(
            painter = painterResource(id = iconRes),
            contentDescription = contentDescription,
            tint = MaterialTheme.colorScheme.onSurface,
            modifier = Modifier.size(20.dp),
        )
    }
}
