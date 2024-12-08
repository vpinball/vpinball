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
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
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
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.layout.boundsInWindow
import androidx.compose.ui.layout.onGloballyPositioned
import androidx.compose.ui.platform.LocalDensity
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.DpOffset
import androidx.compose.ui.unit.dp
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import org.vpinball.app.R
import org.vpinball.app.VPinballManager
import org.vpinball.app.jni.VPinballUnitConverter
import org.vpinball.app.jni.VPinballViewLayoutMode
import org.vpinball.app.jni.VPinballViewSetup
import org.vpinball.app.ui.screens.common.ShadowBox

var viewMode by mutableStateOf(VPinballViewLayoutMode.CAMERA)
    private set

var fov by mutableFloatStateOf(0f)
    private set

var layback by mutableFloatStateOf(0f)
    private set

var viewHOfs by mutableFloatStateOf(0f)
    private set

var viewVOfs by mutableFloatStateOf(0f)
    private set

var sceneScaleX by mutableFloatStateOf(0f)
    private set

var sceneScaleY by mutableFloatStateOf(0f)
    private set

var sceneScaleZ by mutableFloatStateOf(0f)
    private set

var lookAt by mutableFloatStateOf(0f)
    private set

var viewX by mutableFloatStateOf(0f)
    private set

var viewY by mutableFloatStateOf(0f)
    private set

var viewZ by mutableFloatStateOf(0f)
    private set

var windowTopZOfs by mutableFloatStateOf(0f)
    private set

var windowBottomZOfs by mutableFloatStateOf(0f)
    private set

var viewportRotation by mutableFloatStateOf(0f)
    private set

@Composable
fun PointOfViewPage(onSave: (String) -> Unit, onActiveSliderChanged: (Boolean) -> Unit) {
    val density = LocalDensity.current

    var offsetPosition by remember { mutableStateOf(Offset.Zero) }
    var resetExpanded by remember { mutableStateOf(false) }

    var activeSlider by remember { mutableStateOf<String?>(null) }
    var isScaleLocked by remember { mutableStateOf(true) }

    fun handleRefresh() {
        val viewSetup = VPinballManager.getViewSetup()

        viewMode = viewSetup.viewMode
        lookAt = viewSetup.lookAt
        fov = viewSetup.fov
        layback = viewSetup.layback
        viewHOfs = viewSetup.viewHOfs
        viewVOfs = viewSetup.viewVOfs
        sceneScaleX = viewSetup.sceneScaleX
        sceneScaleY = viewSetup.sceneScaleY
        sceneScaleZ = viewSetup.sceneScaleZ
        windowTopZOfs = viewSetup.windowTopZOfs
        windowBottomZOfs = viewSetup.windowBottomZOfs
        viewX = viewSetup.viewX
        viewY = viewSetup.viewY
        viewZ = viewSetup.viewZ
        viewportRotation = viewSetup.viewportRotation
    }

    fun handleUpdate() {
        val viewSetup =
            VPinballViewSetup(
                viewMode = viewMode,
                sceneScaleX = sceneScaleX,
                sceneScaleY = sceneScaleY,
                sceneScaleZ = sceneScaleZ,
                viewX = viewX,
                viewY = viewY,
                viewZ = viewZ,
                lookAt = lookAt,
                viewportRotation = viewportRotation,
                fov = fov,
                layback = layback,
                viewHOfs = viewHOfs,
                viewVOfs = viewVOfs,
                windowTopZOfs = windowTopZOfs,
                windowBottomZOfs = windowBottomZOfs,
            )

        VPinballManager.setViewSetup(viewSetup)
    }

    fun handleScaleChange(axis: String, newValue: Float) {
        if (!isScaleLocked) {
            when (axis) {
                "X" -> sceneScaleX = newValue
                "Y" -> sceneScaleY = newValue
                "Z" -> sceneScaleZ = newValue
            }
            handleUpdate()
            return
        }

        val oldValue =
            when (axis) {
                "X" -> sceneScaleX
                "Y" -> sceneScaleY
                "Z" -> sceneScaleZ
                else -> 0f
            }

        val diff = newValue - oldValue

        when (axis) {
            "X" -> {
                if (sceneScaleY + diff < 0 || sceneScaleZ + diff < 0) {
                    sceneScaleX = oldValue
                } else {
                    sceneScaleX = newValue
                    sceneScaleY += diff
                    sceneScaleZ += diff
                }
            }
            "Y" -> {
                if (sceneScaleX + diff < 0 || sceneScaleZ + diff < 0) {
                    sceneScaleY = oldValue
                } else {
                    sceneScaleY = newValue
                    sceneScaleX += diff
                    sceneScaleZ += diff
                }
            }
            "Z" -> {
                if (sceneScaleX + diff < 0 || sceneScaleY + diff < 0) {
                    sceneScaleZ = oldValue
                } else {
                    sceneScaleZ = newValue
                    sceneScaleX += diff
                    sceneScaleY += diff
                }
            }
        }

        handleUpdate()
    }

    LaunchedEffect(Unit) { handleRefresh() }

    Box(modifier = Modifier.fillMaxSize()) {
        Column(
            modifier = Modifier.fillMaxSize(),
            verticalArrangement = Arrangement.spacedBy(16.dp),
            horizontalAlignment = Alignment.CenterHorizontally,
        ) {
            Text(
                text = "Point of View",
                color = Color.White,
                style = MaterialTheme.typography.titleMedium,
                modifier = Modifier.alpha(if (activeSlider != null) 0.0f else 1f),
            )

            Box(modifier = Modifier.fillMaxWidth().weight(1f)) {
                Column(modifier = Modifier.fillMaxSize().verticalScroll(rememberScrollState())) {
                    ShadowBox(backgroundVisible = false) {
                        EnumOptionItem(
                            label = "View Mode Layout",
                            options = VPinballViewLayoutMode.entries.toList(),
                            option = viewMode,
                            onOptionChanged = {
                                viewMode = it
                                handleUpdate()
                            },
                            visible = activeSlider == null,
                        )
                    }

                    when (viewMode) {
                        VPinballViewLayoutMode.LEGACY -> {
                            ShadowBox(backgroundVisible = (activeSlider == "Inclination Y")) {
                                FormattedSliderItem(
                                    label = "Inclination",
                                    value = lookAt,
                                    onValueChange = {
                                        lookAt = it
                                        handleUpdate()
                                    },
                                    onActiveChange = {
                                        activeSlider = it
                                        onActiveSliderChanged(activeSlider != null)
                                    },
                                    minValue = -50f,
                                    maxValue = 80f,
                                    visible = activeSlider == null || activeSlider == "Inclination",
                                    format = FormattedSliderItemFormat.DEGREES,
                                )
                            }

                            ShadowBox(backgroundVisible = (activeSlider == "Field Of View (overall scale)")) {
                                FormattedSliderItem(
                                    label = "Field Of View (overall scale)",
                                    value = fov,
                                    onValueChange = {
                                        fov = it
                                        handleUpdate()
                                    },
                                    onActiveChange = {
                                        activeSlider = it
                                        onActiveSliderChanged(activeSlider != null)
                                    },
                                    minValue = 1f,
                                    maxValue = 175f,
                                    visible = activeSlider == null || activeSlider == "Field Of View (overall scale)",
                                    format = FormattedSliderItemFormat.DEGREES,
                                )
                            }

                            ShadowBox(backgroundVisible = (activeSlider == "Layback")) {
                                FormattedSliderItem(
                                    label = "Layback",
                                    value = layback,
                                    onValueChange = {
                                        layback = it
                                        handleUpdate()
                                    },
                                    onActiveChange = {
                                        activeSlider = it
                                        onActiveSliderChanged(activeSlider != null)
                                    },
                                    minValue = -100f,
                                    maxValue = 100f,
                                    visible = activeSlider == null || activeSlider == "Layback",
                                    format = FormattedSliderItemFormat.DECIMAL,
                                )
                            }

                            val showGroup =
                                (isScaleLocked &&
                                    (activeSlider == "Table X Scale" || activeSlider == "Table Y Scale" || activeSlider == "Table Z Scale"))

                            ShadowBox(backgroundVisible = showGroup, horizontalPadding = 0.dp) {
                                Row(verticalAlignment = Alignment.CenterVertically) {
                                    IconButton(
                                        onClick = { isScaleLocked = !isScaleLocked },
                                        modifier = Modifier.padding(start = 16.dp).alpha(if (activeSlider != null && !showGroup) 0.0f else 1f),
                                    ) {
                                        Icon(
                                            painter =
                                                painterResource(id = if (isScaleLocked) R.drawable.img_sf_lock else R.drawable.img_sf_lock_slash),
                                            contentDescription = null,
                                            tint = Color.White,
                                            modifier = Modifier.size(24.dp),
                                        )
                                    }
                                    Column(modifier = Modifier.weight(1f)) {
                                        ShadowBox(backgroundVisible = (!showGroup && activeSlider == "Table X Scale")) {
                                            FormattedSliderItem(
                                                label = "Table X Scale",
                                                value = sceneScaleX,
                                                onValueChange = { newValue -> handleScaleChange("X", newValue) },
                                                onActiveChange = {
                                                    activeSlider = it
                                                    onActiveSliderChanged(activeSlider != null)
                                                },
                                                minValue = 0.1f,
                                                maxValue = 2f,
                                                visible = ((activeSlider == null) || (!showGroup && activeSlider == "Table X Scale") || showGroup),
                                                format = FormattedSliderItemFormat.SCALED_PERCENTAGE,
                                            )
                                        }

                                        ShadowBox(backgroundVisible = (!showGroup && activeSlider == "Table Y Scale")) {
                                            FormattedSliderItem(
                                                label = "Table Y Scale",
                                                value = sceneScaleY,
                                                onValueChange = { newValue -> handleScaleChange("Y", newValue) },
                                                onActiveChange = {
                                                    activeSlider = it
                                                    onActiveSliderChanged(activeSlider != null)
                                                },
                                                minValue = 0.1f,
                                                maxValue = 2f,
                                                visible = ((activeSlider == null) || (!showGroup && activeSlider == "Table Y Scale") || showGroup),
                                                format = FormattedSliderItemFormat.SCALED_PERCENTAGE,
                                            )
                                        }

                                        ShadowBox(backgroundVisible = (!showGroup && activeSlider == "Table Z Scale")) {
                                            FormattedSliderItem(
                                                label = "Table Z Scale",
                                                value = sceneScaleZ,
                                                onValueChange = { newValue -> handleScaleChange("Z", newValue) },
                                                onActiveChange = {
                                                    activeSlider = it
                                                    onActiveSliderChanged(activeSlider != null)
                                                },
                                                minValue = 0.1f,
                                                maxValue = 2f,
                                                visible = ((activeSlider == null) || (!showGroup && activeSlider == "Table Z Scale") || showGroup),
                                                format = FormattedSliderItemFormat.SCALED_PERCENTAGE,
                                            )
                                        }
                                    }
                                }
                            }

                            ShadowBox(backgroundVisible = (activeSlider == "X Offset")) {
                                FormattedSliderItem(
                                    label = "X Offset",
                                    value = viewX,
                                    onValueChange = {
                                        viewX = it
                                        handleUpdate()
                                    },
                                    onActiveChange = {
                                        activeSlider = it
                                        onActiveSliderChanged(activeSlider != null)
                                    },
                                    minValue = VPinballUnitConverter.cmToVPU(-70f),
                                    maxValue = VPinballUnitConverter.cmToVPU(70f),
                                    visible = activeSlider == null || activeSlider == "X Offset",
                                    format = FormattedSliderItemFormat.VPU_CM,
                                )
                            }

                            ShadowBox(backgroundVisible = (activeSlider == "Y Offset")) {
                                FormattedSliderItem(
                                    label = "Y Offset",
                                    value = viewY,
                                    onValueChange = {
                                        viewY = it
                                        handleUpdate()
                                    },
                                    onActiveChange = {
                                        activeSlider = it
                                        onActiveSliderChanged(activeSlider != null)
                                    },
                                    minValue = VPinballUnitConverter.cmToVPU(-70f),
                                    maxValue = VPinballUnitConverter.cmToVPU(400f),
                                    visible = activeSlider == null || activeSlider == "Y Offset",
                                    format = FormattedSliderItemFormat.VPU_CM,
                                )
                            }

                            ShadowBox(backgroundVisible = (activeSlider == "Z Offset")) {
                                FormattedSliderItem(
                                    label = "Z Offset",
                                    value = viewZ,
                                    onValueChange = {
                                        viewZ = it
                                        handleUpdate()
                                    },
                                    onActiveChange = {
                                        activeSlider = it
                                        onActiveSliderChanged(activeSlider != null)
                                    },
                                    minValue = VPinballUnitConverter.cmToVPU(0f),
                                    maxValue = VPinballUnitConverter.cmToVPU(200f),
                                    visible = activeSlider == null || activeSlider == "Z Offset",
                                    format = FormattedSliderItemFormat.VPU_CM,
                                )
                            }
                        }
                        VPinballViewLayoutMode.CAMERA -> {
                            ShadowBox(backgroundVisible = (activeSlider == "Field Of View (overall scale)")) {
                                FormattedSliderItem(
                                    label = "Field Of View (overall scale)",
                                    value = fov,
                                    onValueChange = {
                                        fov = it
                                        handleUpdate()
                                    },
                                    onActiveChange = {
                                        activeSlider = it
                                        onActiveSliderChanged(activeSlider != null)
                                    },
                                    minValue = 1f,
                                    maxValue = 175f,
                                    visible = activeSlider == null || activeSlider == "Field Of View (overall scale)",
                                    format = FormattedSliderItemFormat.DEGREES,
                                )
                            }

                            ShadowBox(backgroundVisible = (activeSlider == "Horizontal Offset")) {
                                FormattedSliderItem(
                                    label = "Horizontal Offset",
                                    value = viewHOfs,
                                    onValueChange = {
                                        viewHOfs = it
                                        handleUpdate()
                                    },
                                    onActiveChange = {
                                        activeSlider = it
                                        onActiveSliderChanged(activeSlider != null)
                                    },
                                    minValue = -100f,
                                    maxValue = 100f,
                                    visible = activeSlider == null || activeSlider == "Horizontal Offset",
                                    format = FormattedSliderItemFormat.DECIMAL,
                                )
                            }

                            ShadowBox(backgroundVisible = (activeSlider == "Vertical Offset")) {
                                FormattedSliderItem(
                                    label = "Vertical Offset",
                                    value = viewVOfs,
                                    onValueChange = {
                                        viewVOfs = it
                                        handleUpdate()
                                    },
                                    onActiveChange = {
                                        activeSlider = it
                                        onActiveSliderChanged(activeSlider != null)
                                    },
                                    minValue = -150f,
                                    maxValue = 150f,
                                    visible = activeSlider == null || activeSlider == "Vertical Offset",
                                    format = FormattedSliderItemFormat.DECIMAL,
                                )
                            }

                            val showGroup =
                                (isScaleLocked &&
                                    (activeSlider == "Table X Scale" || activeSlider == "Table Y Scale" || activeSlider == "Table Z Scale"))

                            ShadowBox(backgroundVisible = showGroup, horizontalPadding = 0.dp) {
                                Row(verticalAlignment = Alignment.CenterVertically) {
                                    IconButton(
                                        onClick = { isScaleLocked = !isScaleLocked },
                                        modifier = Modifier.padding(start = 16.dp).alpha(if (activeSlider != null && !showGroup) 0.0f else 1f),
                                    ) {
                                        Icon(
                                            painter =
                                                painterResource(id = if (isScaleLocked) R.drawable.img_sf_lock else R.drawable.img_sf_lock_slash),
                                            contentDescription = null,
                                            tint = Color.White,
                                            modifier = Modifier.size(24.dp),
                                        )
                                    }
                                    Column(modifier = Modifier.weight(1f)) {
                                        ShadowBox(backgroundVisible = (!showGroup && activeSlider == "Table X Scale")) {
                                            FormattedSliderItem(
                                                label = "Table X Scale",
                                                value = sceneScaleX,
                                                onValueChange = { newValue -> handleScaleChange("X", newValue) },
                                                onActiveChange = {
                                                    activeSlider = it
                                                    onActiveSliderChanged(activeSlider != null)
                                                },
                                                minValue = 0.1f,
                                                maxValue = 2f,
                                                visible = ((activeSlider == null) || (!showGroup && activeSlider == "Table X Scale") || showGroup),
                                                format = FormattedSliderItemFormat.SCALED_PERCENTAGE,
                                            )
                                        }

                                        ShadowBox(backgroundVisible = (!showGroup && activeSlider == "Table Y Scale")) {
                                            FormattedSliderItem(
                                                label = "Table Y Scale",
                                                value = sceneScaleY,
                                                onValueChange = { newValue -> handleScaleChange("Y", newValue) },
                                                onActiveChange = {
                                                    activeSlider = it
                                                    onActiveSliderChanged(activeSlider != null)
                                                },
                                                minValue = 0.1f,
                                                maxValue = 2f,
                                                visible = ((activeSlider == null) || (!showGroup && activeSlider == "Table Y Scale") || showGroup),
                                                format = FormattedSliderItemFormat.SCALED_PERCENTAGE,
                                            )
                                        }

                                        ShadowBox(backgroundVisible = (!showGroup && activeSlider == "Table Z Scale")) {
                                            FormattedSliderItem(
                                                label = "Table Z Scale",
                                                value = sceneScaleZ,
                                                onValueChange = { newValue -> handleScaleChange("Z", newValue) },
                                                onActiveChange = {
                                                    activeSlider = it
                                                    onActiveSliderChanged(activeSlider != null)
                                                },
                                                minValue = 0.1f,
                                                maxValue = 2f,
                                                visible = ((activeSlider == null) || (!showGroup && activeSlider == "Table Z Scale") || showGroup),
                                                format = FormattedSliderItemFormat.SCALED_PERCENTAGE,
                                            )
                                        }
                                    }
                                }
                            }

                            ShadowBox(backgroundVisible = (activeSlider == "Look At")) {
                                FormattedSliderItem(
                                    label = "Look At",
                                    value = lookAt,
                                    onValueChange = {
                                        lookAt = it
                                        handleUpdate()
                                    },
                                    onActiveChange = {
                                        activeSlider = it
                                        onActiveSliderChanged(activeSlider != null)
                                    },
                                    minValue = -20f,
                                    maxValue = 200f,
                                    visible = activeSlider == null || activeSlider == "Look At",
                                    format = FormattedSliderItemFormat.PERCENTAGE,
                                )
                            }

                            ShadowBox(backgroundVisible = (activeSlider == "Camera X")) {
                                FormattedSliderItem(
                                    label = "Camera X",
                                    value = viewX,
                                    onValueChange = {
                                        viewX = it
                                        handleUpdate()
                                    },
                                    onActiveChange = {
                                        activeSlider = it
                                        onActiveSliderChanged(activeSlider != null)
                                    },
                                    minValue = VPinballUnitConverter.cmToVPU(-70f),
                                    maxValue = VPinballUnitConverter.cmToVPU(70f),
                                    visible = activeSlider == null || activeSlider == "Camera X",
                                    format = FormattedSliderItemFormat.VPU_CM,
                                )
                            }

                            ShadowBox(backgroundVisible = (activeSlider == "Camera Y")) {
                                FormattedSliderItem(
                                    label = "Camera Y",
                                    value = viewY,
                                    onValueChange = {
                                        viewY = it
                                        handleUpdate()
                                    },
                                    onActiveChange = {
                                        activeSlider = it
                                        onActiveSliderChanged(activeSlider != null)
                                    },
                                    minValue = VPinballUnitConverter.cmToVPU(-70f),
                                    maxValue = VPinballUnitConverter.cmToVPU(400f),
                                    visible = activeSlider == null || activeSlider == "Camera Y",
                                    format = FormattedSliderItemFormat.VPU_CM,
                                )
                            }

                            ShadowBox(backgroundVisible = (activeSlider == "Camera Z")) {
                                FormattedSliderItem(
                                    label = "Camera Z",
                                    value = viewZ,
                                    onValueChange = {
                                        viewZ = it
                                        handleUpdate()
                                    },
                                    onActiveChange = {
                                        activeSlider = it
                                        onActiveSliderChanged(activeSlider != null)
                                    },
                                    minValue = VPinballUnitConverter.cmToVPU(0f),
                                    maxValue = VPinballUnitConverter.cmToVPU(200f),
                                    visible = activeSlider == null || activeSlider == "Camera Z",
                                    format = FormattedSliderItemFormat.VPU_CM,
                                )
                            }
                        }
                        VPinballViewLayoutMode.WINDOW -> {
                            ShadowBox(backgroundVisible = (activeSlider == "Horizontal Offset")) {
                                FormattedSliderItem(
                                    label = "Horizontal Offset",
                                    value = viewHOfs,
                                    onValueChange = {
                                        viewHOfs = it
                                        handleUpdate()
                                    },
                                    onActiveChange = {
                                        activeSlider = it
                                        onActiveSliderChanged(activeSlider != null)
                                    },
                                    minValue = -100f,
                                    maxValue = 100f,
                                    visible = activeSlider == null || activeSlider == "Horizontal Offset",
                                    format = FormattedSliderItemFormat.CM,
                                )
                            }

                            ShadowBox(backgroundVisible = (activeSlider == "Vertical Offset")) {
                                FormattedSliderItem(
                                    label = "Vertical Offset",
                                    value = viewVOfs,
                                    onValueChange = {
                                        viewVOfs = it
                                        handleUpdate()
                                    },
                                    onActiveChange = {
                                        activeSlider = it
                                        onActiveSliderChanged(activeSlider != null)
                                    },
                                    minValue = -150f,
                                    maxValue = 150f,
                                    visible = activeSlider == null || activeSlider == "Vertical Offset",
                                    format = FormattedSliderItemFormat.CM,
                                )
                            }

                            val showGroup = (isScaleLocked && (activeSlider == "Table X Scale" || activeSlider == "Table YZ Scale"))

                            ShadowBox(backgroundVisible = showGroup, horizontalPadding = 0.dp) {
                                Row(verticalAlignment = Alignment.CenterVertically) {
                                    IconButton(
                                        onClick = { isScaleLocked = !isScaleLocked },
                                        modifier = Modifier.padding(start = 16.dp).alpha(if (activeSlider != null && !showGroup) 0.0f else 1f),
                                    ) {
                                        Icon(
                                            painter =
                                                painterResource(id = if (isScaleLocked) R.drawable.img_sf_lock else R.drawable.img_sf_lock_slash),
                                            contentDescription = null,
                                            tint = Color.White,
                                            modifier = Modifier.size(24.dp),
                                        )
                                    }
                                    Column(modifier = Modifier.weight(1f)) {
                                        ShadowBox(backgroundVisible = (!showGroup && activeSlider == "Table X Scale")) {
                                            FormattedSliderItem(
                                                label = "Table X Scale",
                                                value = sceneScaleX,
                                                onValueChange = { newValue -> handleScaleChange("X", newValue) },
                                                onActiveChange = {
                                                    activeSlider = it
                                                    onActiveSliderChanged(activeSlider != null)
                                                },
                                                minValue = 0.1f,
                                                maxValue = 2f,
                                                visible = ((activeSlider == null) || (!showGroup && activeSlider == "Table X Scale") || showGroup),
                                                format = FormattedSliderItemFormat.SCALED_PERCENTAGE,
                                            )
                                        }

                                        ShadowBox(backgroundVisible = (!showGroup && activeSlider == "Table YZ Scale")) {
                                            FormattedSliderItem(
                                                label = "Table YZ Scale",
                                                value = sceneScaleY,
                                                onValueChange = { newValue -> handleScaleChange("Y", newValue) },
                                                onActiveChange = {
                                                    activeSlider = it
                                                    onActiveSliderChanged(activeSlider != null)
                                                },
                                                minValue = 0.1f,
                                                maxValue = 2f,
                                                visible = ((activeSlider == null) || (!showGroup && activeSlider == "Table YZ Scale") || showGroup),
                                                format = FormattedSliderItemFormat.SCALED_PERCENTAGE,
                                            )
                                        }
                                    }
                                }
                            }

                            ShadowBox(backgroundVisible = (activeSlider == "Window Top Z Ofs.")) {
                                FormattedSliderItem(
                                    label = "Window Top Z Ofs.",
                                    value = windowTopZOfs,
                                    onValueChange = {
                                        windowTopZOfs = it
                                        handleUpdate()
                                    },
                                    onActiveChange = {
                                        activeSlider = it
                                        onActiveSliderChanged(activeSlider != null)
                                    },
                                    minValue = VPinballUnitConverter.cmToVPU(-70f),
                                    maxValue = VPinballUnitConverter.cmToVPU(70f),
                                    visible = activeSlider == null || activeSlider == "Window Top Z Ofs.",
                                    format = FormattedSliderItemFormat.VPU_CM,
                                )
                            }

                            ShadowBox(backgroundVisible = (activeSlider == "Window Bottom Z Ofs.")) {
                                FormattedSliderItem(
                                    label = "Window Bottom Z Ofs.",
                                    value = windowBottomZOfs,
                                    onValueChange = {
                                        windowBottomZOfs = it
                                        handleUpdate()
                                    },
                                    onActiveChange = {
                                        activeSlider = it
                                        onActiveSliderChanged(activeSlider != null)
                                    },
                                    minValue = VPinballUnitConverter.cmToVPU(-70f),
                                    maxValue = VPinballUnitConverter.cmToVPU(70f),
                                    visible = activeSlider == null || activeSlider == "Window Bottom Z Ofs.",
                                    format = FormattedSliderItemFormat.VPU_CM,
                                )
                            }

                            ShadowBox(backgroundVisible = (activeSlider == "Player X")) {
                                FormattedSliderItem(
                                    label = "Player X",
                                    value = viewX,
                                    onValueChange = {
                                        viewX = it
                                        handleUpdate()
                                    },
                                    onActiveChange = {
                                        activeSlider = it
                                        onActiveSliderChanged(activeSlider != null)
                                    },
                                    minValue = VPinballUnitConverter.cmToVPU(-70f),
                                    maxValue = VPinballUnitConverter.cmToVPU(70f),
                                    visible = activeSlider == null || activeSlider == "Player X",
                                    format = FormattedSliderItemFormat.VPU_CM,
                                )
                            }

                            ShadowBox(backgroundVisible = activeSlider == "Player Y") {
                                FormattedSliderItem(
                                    label = "Player Y",
                                    value = viewY,
                                    onValueChange = {
                                        viewY = it
                                        handleUpdate()
                                    },
                                    onActiveChange = {
                                        activeSlider = it
                                        onActiveSliderChanged(activeSlider != null)
                                    },
                                    minValue = VPinballUnitConverter.cmToVPU(-70f),
                                    maxValue = VPinballUnitConverter.cmToVPU(400f),
                                    visible = activeSlider == null || activeSlider == "Player Y",
                                    format = FormattedSliderItemFormat.VPU_CM,
                                )
                            }

                            ShadowBox(backgroundVisible = (activeSlider == "Player Z")) {
                                FormattedSliderItem(
                                    label = "Player Z",
                                    value = viewZ,
                                    onValueChange = {
                                        viewZ = it
                                        handleUpdate()
                                    },
                                    onActiveChange = {
                                        activeSlider = it
                                        onActiveSliderChanged(activeSlider != null)
                                    },
                                    minValue = VPinballUnitConverter.cmToVPU(0f),
                                    maxValue = VPinballUnitConverter.cmToVPU(200f),
                                    visible = activeSlider == null || activeSlider == "Player Z",
                                    format = FormattedSliderItemFormat.VPU_CM,
                                )
                            }
                        }
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
                        VPinballManager.saveViewSetup()
                        onSave("Point of View\nSaved!")
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

                VPinballManager.resetViewSetup()

                CoroutineScope(Dispatchers.Main).launch {
                    delay(100)
                    handleRefresh()
                }
            },
        )

        HorizontalDivider()

        DropdownMenuItem(
            text = { Text("System defaults", style = MaterialTheme.typography.bodyLarge) },
            onClick = {
                resetExpanded = false

                VPinballManager.setDefaultViewSetup()

                CoroutineScope(Dispatchers.Main).launch {
                    delay(100)
                    handleRefresh()
                }
            },
        )
    }
}
