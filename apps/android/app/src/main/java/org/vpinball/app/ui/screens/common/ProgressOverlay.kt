package org.vpinball.app.ui.screens.common

import androidx.compose.foundation.border
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.navigationBarsPadding
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.LinearProgressIndicator
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import dev.chrisbanes.haze.HazeInput
import dev.chrisbanes.haze.HazeState
import dev.chrisbanes.haze.blur.HazeBlurStyle
import dev.chrisbanes.haze.blur.HazeColorEffect
import dev.chrisbanes.haze.blur.hazeBlur
import org.vpinball.app.ui.theme.VpxDarkYellow

@Composable
fun ProgressOverlay(modifier: Modifier = Modifier, title: String? = null, progress: Int = 0, status: String? = null, hazeState: HazeState) {
    val shape = RoundedCornerShape(26.dp)
    val surface = Color(0xFF1C1C1E)
    val tint = Color(0xFF1C1C1E).copy(alpha = 0.55f)

    Box(modifier = modifier.fillMaxSize().navigationBarsPadding().padding(20.dp)) {
        Box(
            modifier =
                Modifier.align(Alignment.BottomCenter)
                    .fillMaxWidth()
                    .clip(shape)
                    .hazeBlur(
                        input = HazeInput.Sources(hazeState),
                        style =
                            HazeBlurStyle {
                                blurRadius(40.dp)
                                backgroundColor(surface)
                                colorEffects(listOf(HazeColorEffect.tint(tint)))
                                noiseFactor(0.15f)
                            },
                    )
                    .border(1.dp, Color.White.copy(alpha = 0.14f), shape)
        ) {
            Column(
                modifier = Modifier.fillMaxWidth().padding(22.dp),
                verticalArrangement = Arrangement.spacedBy(18.dp),
                horizontalAlignment = Alignment.CenterHorizontally,
            ) {
                Text(
                    text = title ?: "",
                    textAlign = TextAlign.Center,
                    color = Color.White,
                    style = MaterialTheme.typography.titleMedium,
                    fontWeight = FontWeight.Bold,
                )

                LinearProgressIndicator(
                    progress = { progress.toFloat() / 100f },
                    color = Color.VpxDarkYellow,
                    trackColor = Color.White.copy(alpha = 0.22f),
                    gapSize = 0.dp,
                    drawStopIndicator = {},
                    modifier = Modifier.fillMaxWidth(),
                )

                Text(
                    text = status ?: "",
                    textAlign = TextAlign.Center,
                    color = Color.White.copy(alpha = 0.75f),
                    style = MaterialTheme.typography.labelLarge,
                    fontWeight = FontWeight.SemiBold,
                )
            }
        }
    }
}
