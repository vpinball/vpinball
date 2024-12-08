package org.vpinball.app.ui.screens.liveui

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Slider
import androidx.compose.material3.SliderDefaults
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableFloatStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.draw.clip
import androidx.compose.ui.draw.shadow
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.unit.dp
import kotlin.math.abs
import kotlin.math.roundToInt
import org.vpinball.app.R
import org.vpinball.app.VPinballManager
import org.vpinball.app.jni.VPinballCustomTableOption
import org.vpinball.app.ui.screens.common.ShadowBox
import org.vpinball.app.ui.theme.VpxRed

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun CustomTableOptionItem(customTableOption: VPinballCustomTableOption, activeSlider: String?, onActiveSliderChanged: (String?) -> Unit) {
    var optionsExpanded by remember { mutableStateOf(false) }
    var options by remember { mutableStateOf(listOf<String>()) }
    var isActive by remember { mutableStateOf(false) }
    var value by remember { mutableFloatStateOf(customTableOption.value) }

    LaunchedEffect(customTableOption.literals) {
        options =
            if (customTableOption.literals.isNotEmpty()) {
                customTableOption.literals.split("||").map { it.trim() }
            } else {
                emptyList()
            }
    }

    if (customTableOption.literals.isNotEmpty()) {
        ShadowBox(backgroundVisible = false) {
            Row(
                modifier = Modifier.padding(vertical = 8.dp).alpha(if (activeSlider == null) 1.0f else 0.0f),
                verticalAlignment = Alignment.CenterVertically,
            ) {
                Text(text = customTableOption.name, style = MaterialTheme.typography.bodyLarge, color = Color.White, modifier = Modifier.weight(1f))

                Box {
                    Row(verticalAlignment = Alignment.CenterVertically, modifier = Modifier.clickable { optionsExpanded = true }) {
                        Text(
                            text = options.getOrElse(customTableOption.value.toInt()) { "" },
                            style = MaterialTheme.typography.bodyLarge,
                            color = Color.White,
                        )

                        Icon(
                            painter = painterResource(id = R.drawable.img_sf_chevron_up_chevron_down),
                            contentDescription = null,
                            tint = Color.White,
                            modifier = Modifier.size(14.dp).padding(start = 4.dp),
                        )
                    }

                    DropdownMenu(expanded = optionsExpanded, onDismissRequest = { optionsExpanded = false }) {
                        options.forEachIndexed { index, option ->
                            DropdownMenuItem(
                                text = {
                                    Text(
                                        text = option,
                                        style = MaterialTheme.typography.bodyLarge,
                                        color = MaterialTheme.colorScheme.onSurfaceVariant,
                                    )
                                },
                                onClick = {
                                    optionsExpanded = false
                                    value = index.toFloat()
                                    customTableOption.value = value
                                    VPinballManager.setCustomTableOption(customTableOption)
                                },
                            )
                        }
                    }
                }
            }
        }
    } else {
        ShadowBox(backgroundVisible = (activeSlider == customTableOption.name)) {
            Column(modifier = Modifier.alpha(if (activeSlider == null || activeSlider == customTableOption.name) 1.0f else 0.0f)) {
                Text(text = customTableOption.name, style = MaterialTheme.typography.bodyLarge, color = Color.White)

                Row(verticalAlignment = Alignment.CenterVertically) {
                    Slider(
                        value = value,
                        onValueChange = {
                            if (!isActive) {
                                isActive = true
                                onActiveSliderChanged(customTableOption.name)
                            }
                            val roundedValue = (it * 1000).roundToInt() / 1000f
                            val finalValue =
                                if (abs(roundedValue - customTableOption.maxValue) < customTableOption.step) {
                                    customTableOption.maxValue
                                } else {
                                    roundedValue
                                }
                            value = finalValue
                            customTableOption.value = value
                            VPinballManager.setCustomTableOption(customTableOption)
                        },
                        onValueChangeFinished = {
                            isActive = false
                            onActiveSliderChanged(null)
                        },
                        valueRange = customTableOption.minValue..customTableOption.maxValue,
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
                        text = customTableOption.unit.formatValue(value),
                        style = MaterialTheme.typography.bodyMedium,
                        color = Color.White,
                        modifier = Modifier.padding(start = 8.dp),
                    )
                }
            }
        }
    }
}
