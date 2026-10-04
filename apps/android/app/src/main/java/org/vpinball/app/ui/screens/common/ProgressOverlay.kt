package org.vpinball.app.ui.screens.common

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
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
    Box(modifier = modifier.fillMaxSize().padding(20.dp)) {
        Box(
            modifier =
                Modifier.align(Alignment.BottomCenter)
                    .fillMaxWidth()
                    .clip(RoundedCornerShape(16.dp))
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
                modifier = Modifier.fillMaxWidth().padding(15.dp),
                verticalArrangement = Arrangement.spacedBy(10.dp),
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
                    trackColor = Color.Gray,
                    gapSize = 0.dp,
                    drawStopIndicator = {},
                    modifier = Modifier.fillMaxWidth(),
                )

                Text(
                    text = status ?: "",
                    textAlign = TextAlign.Center,
                    color = Color.White,
                    style = MaterialTheme.typography.titleSmall,
                    fontWeight = FontWeight.Bold,
                )
            }
        }
    }
}
