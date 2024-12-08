package org.vpinball.app.ui.screens.liveui

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableFloatStateOf
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.layout.boundsInWindow
import androidx.compose.ui.layout.onGloballyPositioned
import androidx.compose.ui.platform.LocalDensity
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.DpOffset
import androidx.compose.ui.unit.dp
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import org.vpinball.app.VPinballManager
import org.vpinball.app.jni.VPinballCustomTableOption
import org.vpinball.app.jni.VPinballTableOptions
import org.vpinball.app.jni.VPinballToneMapper
import org.vpinball.app.ui.screens.common.ShadowBox

var globalEmissionScale by mutableFloatStateOf(0f)
    private set

var globalDifficulty by mutableFloatStateOf(0f)
    private set

var exposure by mutableFloatStateOf(0f)
    private set

var toneMapper by mutableStateOf(VPinballToneMapper.REINHARD)
    private set

var musicVolume by mutableIntStateOf(0)
    private set

var soundVolume by mutableIntStateOf(0)
    private set

@Composable
fun TableOptionsPage(onSave: (String) -> Unit, onActiveSliderChanged: (Boolean) -> Unit) {
    val density = LocalDensity.current

    val customOptions = remember { mutableStateOf(emptyList<VPinballCustomTableOption>()) }

    var offsetPosition by remember { mutableStateOf(Offset.Zero) }
    var resetExpanded by remember { mutableStateOf(false) }

    var activeSlider by remember { mutableStateOf<String?>(null) }

    fun handleUpdate() {
        val tableOptions =
            VPinballTableOptions(
                globalEmissionScale = globalEmissionScale,
                globalDifficulty = globalDifficulty,
                exposure = exposure,
                toneMapper = toneMapper,
                musicVolume = musicVolume,
                soundVolume = soundVolume,
            )

        VPinballManager.setTableOptions(tableOptions)
    }

    fun handleRefresh() {
        customOptions.value = VPinballManager.getCustomTableOptions()

        val tableOptions = VPinballManager.getTableOptions()

        globalEmissionScale = tableOptions.globalEmissionScale
        globalDifficulty = tableOptions.globalDifficulty
        exposure = tableOptions.exposure
        toneMapper = tableOptions.toneMapper
        musicVolume = tableOptions.musicVolume
        soundVolume = tableOptions.soundVolume
    }

    LaunchedEffect(Unit) { handleRefresh() }

    Box(modifier = Modifier.fillMaxSize()) {
        Column(
            modifier = Modifier.fillMaxSize(),
            verticalArrangement = Arrangement.spacedBy(16.dp),
            horizontalAlignment = Alignment.CenterHorizontally,
        ) {
            Text(
                text = "Table Options",
                color = Color.White,
                style = MaterialTheme.typography.titleMedium,
                modifier = Modifier.alpha(if (activeSlider != null) 0.0f else 1f),
            )

            Box(modifier = Modifier.fillMaxWidth().weight(1f)) {
                Column(modifier = Modifier.fillMaxSize().verticalScroll(rememberScrollState())) {
                    customOptions.value.forEach {
                        CustomTableOptionItem(
                            customTableOption = it,
                            activeSlider = activeSlider,
                            onActiveSliderChanged = {
                                activeSlider = it
                                onActiveSliderChanged(activeSlider != null)
                            },
                        )
                    }

                    ShadowBox(backgroundVisible = (activeSlider == "Day Night")) {
                        FormattedSliderItem(
                            label = "Day Night",
                            value = globalEmissionScale,
                            onValueChange = {
                                globalEmissionScale = it
                                handleUpdate()
                            },
                            onActiveChange = {
                                activeSlider = it
                                onActiveSliderChanged(activeSlider != null)
                            },
                            minValue = 0f,
                            maxValue = 1f,
                            visible = activeSlider == null || activeSlider == "Day Night",
                            format = FormattedSliderItemFormat.SCALED_PERCENTAGE,
                        )
                    }

                    ShadowBox(backgroundVisible = (activeSlider == "Difficulty")) {
                        FormattedSliderItem(
                            label = "Difficulty",
                            value = globalDifficulty,
                            onValueChange = {
                                globalDifficulty = it
                                handleUpdate()
                            },
                            onActiveChange = {
                                activeSlider = it
                                onActiveSliderChanged(activeSlider != null)
                            },
                            minValue = 0f,
                            maxValue = 1f,
                            visible = activeSlider == null || activeSlider == "Difficulty",
                            format = FormattedSliderItemFormat.SCALED_PERCENTAGE,
                        )
                    }

                    ShadowBox(backgroundVisible = (activeSlider == "Exposure")) {
                        FormattedSliderItem(
                            label = "Exposure",
                            value = exposure,
                            onValueChange = {
                                exposure = it
                                handleUpdate()
                            },
                            onActiveChange = {
                                activeSlider = it
                                onActiveSliderChanged(activeSlider != null)
                            },
                            minValue = 0f,
                            maxValue = 2f,
                            visible = activeSlider == null || activeSlider == "Exposure",
                            format = FormattedSliderItemFormat.SCALED_PERCENTAGE,
                        )
                    }

                    ShadowBox(backgroundVisible = false) {
                        EnumOptionItem(
                            label = "Tonemapper",
                            options = VPinballToneMapper.entries.toList(),
                            option = toneMapper,
                            onOptionChanged = {
                                toneMapper = it
                                handleUpdate()
                            },
                            visible = activeSlider == null,
                        )
                    }

                    ShadowBox(backgroundVisible = (activeSlider == "Music Volume")) {
                        FormattedSliderItem(
                            label = "Music Volume",
                            value = musicVolume.toFloat(),
                            onValueChange = {
                                musicVolume = it.toInt()
                                handleUpdate()
                            },
                            onActiveChange = {
                                activeSlider = it
                                onActiveSliderChanged(activeSlider != null)
                            },
                            minValue = 0f,
                            maxValue = 100f,
                            visible = activeSlider == null || activeSlider == "Music Volume",
                            format = FormattedSliderItemFormat.INT_PERCENTAGE,
                        )
                    }

                    ShadowBox(backgroundVisible = (activeSlider == "Sound Volume")) {
                        FormattedSliderItem(
                            label = "Sound Volume",
                            value = soundVolume.toFloat(),
                            onValueChange = {
                                soundVolume = it.toInt()
                                handleUpdate()
                            },
                            onActiveChange = {
                                activeSlider = it
                                onActiveSliderChanged(activeSlider != null)
                            },
                            minValue = 0f,
                            maxValue = 100f,
                            visible = activeSlider == null || activeSlider == "Sound Volume",
                            format = FormattedSliderItemFormat.INT_PERCENTAGE,
                        )
                    }
                }
            }

            Row(
                modifier = Modifier.fillMaxWidth().padding(horizontal = 16.dp).alpha(if (activeSlider != null) 0.0f else 1f),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically,
            ) {
                Button(
                    onClick = {
                        VPinballManager.saveCustomTableOptions()
                        VPinballManager.saveTableOptions()
                        onSave("Table Options\nSaved !")
                    },
                    modifier = Modifier.width(100.dp).height(40.dp),
                    shape = RoundedCornerShape(10.dp),
                    colors = ButtonDefaults.buttonColors(containerColor = Color.Gray),
                    contentPadding = PaddingValues(0.dp),
                ) {
                    Text(
                        text = "Save",
                        color = Color.White,
                        fontSize = MaterialTheme.typography.titleMedium.fontSize,
                        fontWeight = FontWeight.SemiBold,
                    )
                }

                Button(
                    onClick = { resetExpanded = true },
                    modifier =
                        Modifier.width(100.dp).height(40.dp).onGloballyPositioned { layoutCoordinates ->
                            val position = layoutCoordinates.boundsInWindow()
                            offsetPosition = Offset(position.left, position.top + position.height)
                        },
                    shape = RoundedCornerShape(10.dp),
                    colors = ButtonDefaults.buttonColors(containerColor = Color.Gray),
                    contentPadding = PaddingValues(0.dp),
                ) {
                    Text(
                        text = "Reset",
                        color = Color.White,
                        fontSize = MaterialTheme.typography.titleMedium.fontSize,
                        fontWeight = FontWeight.SemiBold,
                    )
                }
            }
        }
    }

    DropdownMenu(
        expanded = resetExpanded,
        onDismissRequest = { resetExpanded = false },
        offset = DpOffset(x = with(density) { offsetPosition.x.toDp() }, y = with(density) { offsetPosition.y.toDp() }),
    ) {
        Text(
            text = "Reset Table Options to...",
            style = MaterialTheme.typography.labelSmall,
            color = MaterialTheme.colorScheme.onSurfaceVariant,
            modifier = Modifier.padding(vertical = 4.dp).padding(horizontal = 12.dp),
        )

        HorizontalDivider()

        DropdownMenuItem(
            text = { Text("Table's Settings", style = MaterialTheme.typography.bodyLarge) },
            onClick = {
                resetExpanded = false

                VPinballManager.resetCustomTableOptions()
                VPinballManager.resetTableOptions()

                CoroutineScope(Dispatchers.Main).launch {
                    delay(100)
                    handleRefresh()
                }
            },
        )

        HorizontalDivider()

        DropdownMenuItem(
            text = { Text("System defaults", style = MaterialTheme.typography.bodyLarge) },
            onClick = { resetExpanded = false },
            enabled = false,
        )
    }
}
