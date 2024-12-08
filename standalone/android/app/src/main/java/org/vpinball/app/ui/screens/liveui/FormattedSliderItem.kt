package org.vpinball.app.ui.screens.liveui

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Slider
import androidx.compose.material3.SliderDefaults
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.draw.clip
import androidx.compose.ui.draw.shadow
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import org.vpinball.app.ui.theme.VpxRed

@Composable
@OptIn(ExperimentalMaterial3Api::class)
fun FormattedSliderItem(
    label: String,
    value: Float,
    onValueChange: (Float) -> Unit,
    format: FormattedSliderItemFormat,
    minValue: Float = 0f,
    maxValue: Float = 100f,
    onActiveChange: (String?) -> Unit,
    visible: Boolean,
) {
    var isActive by remember { mutableStateOf(false) }

    Column(modifier = Modifier.alpha(if (visible) 1.0f else 0.0f)) {
        Text(text = label, style = MaterialTheme.typography.bodyLarge, color = Color.White)

        Row(verticalAlignment = Alignment.CenterVertically) {
            Slider(
                value = value,
                onValueChange = {
                    if (!isActive) {
                        isActive = true
                        onActiveChange(label)
                    }
                    onValueChange(it)
                },
                onValueChangeFinished = {
                    isActive = false
                    onActiveChange(null)
                },
                valueRange = minValue..maxValue,
                track = {
                    SliderDefaults.Track(
                        sliderState = it,
                        drawStopIndicator = null,
                        thumbTrackGapSize = 0.dp,
                        colors =
                            SliderDefaults.colors(
                                activeTrackColor = Color.VpxRed,
                                inactiveTrackColor = SliderDefaults.colors().disabledInactiveTrackColor,
                                disabledActiveTrackColor = Color.VpxRed.copy(alpha = 0.5f),
                            ),
                        modifier = Modifier.height(4.dp),
                    )
                },
                thumb = { Box(Modifier.size(26.dp).shadow(2.dp, CircleShape).clip(CircleShape).background(Color.White)) },
                modifier = Modifier.weight(1f),
            )

            Text(
                text = format.formatValue(value),
                style = MaterialTheme.typography.bodyMedium,
                color = Color.White,
                modifier = Modifier.padding(start = 8.dp),
            )
        }
    }
}
