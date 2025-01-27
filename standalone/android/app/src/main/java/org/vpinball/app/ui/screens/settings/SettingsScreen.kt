package org.vpinball.app.ui.screens.settings

import android.content.Context
import android.content.Intent
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.text.selection.TextSelectionColors
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.ModalBottomSheet
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.OutlinedTextFieldDefaults
import androidx.compose.material3.Scaffold
import androidx.compose.material3.SheetState
import androidx.compose.material3.Slider
import androidx.compose.material3.SliderDefaults
import androidx.compose.material3.Switch
import androidx.compose.material3.SwitchDefaults
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.material3.TopAppBar
import androidx.compose.material3.rememberModalBottomSheetState
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.draw.clip
import androidx.compose.ui.draw.shadow
import androidx.compose.ui.focus.FocusRequester
import androidx.compose.ui.focus.focusRequester
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.text.LinkAnnotation
import androidx.compose.ui.text.SpanStyle
import androidx.compose.ui.text.TextLinkStyles
import androidx.compose.ui.text.TextRange
import androidx.compose.ui.text.buildAnnotatedString
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.input.TextFieldValue
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.text.withLink
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.DpOffset
import androidx.compose.ui.unit.dp
import java.io.File
import kotlinx.coroutines.launch
import org.koin.androidx.compose.koinViewModel
import org.vpinball.app.Credit
import org.vpinball.app.Link
import org.vpinball.app.R
import org.vpinball.app.VPinballManager
import org.vpinball.app.jni.VPinballAAFactor
import org.vpinball.app.jni.VPinballAO
import org.vpinball.app.jni.VPinballDisplayText
import org.vpinball.app.jni.VPinballExternalDMD
import org.vpinball.app.jni.VPinballFXAA
import org.vpinball.app.jni.VPinballGfxBackend
import org.vpinball.app.jni.VPinballMSAASamples
import org.vpinball.app.jni.VPinballMaxTexDimension
import org.vpinball.app.jni.VPinballReflectionMode
import org.vpinball.app.jni.VPinballSharpen
import org.vpinball.app.jni.VPinballViewMode
import org.vpinball.app.ui.screens.common.RoundedCard
import org.vpinball.app.ui.theme.VPinballTheme
import org.vpinball.app.ui.theme.VpxRed

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun SettingsScreen(
    webServerURL: String,
    onViewFile: (file: File) -> Unit,
    onDone: () -> Unit,
    modifier: Modifier = Modifier,
    viewModel: SettingsViewModel = koinViewModel(),
) {
    val context = LocalContext.current

    var showResetDialog by remember { mutableStateOf(false) }
    val sheetState: SheetState = rememberModalBottomSheetState(skipPartiallyExpanded = true)

    LaunchedEffect(Unit) { viewModel.loadSettings() }

    Scaffold(
        topBar = {
            TopAppBar(
                title = { Text("Settings", fontSize = MaterialTheme.typography.titleLarge.fontSize, fontWeight = FontWeight.Bold) },
                actions = {
                    TextButton(onClick = onDone) {
                        Text("Done", color = Color.VpxRed, fontSize = MaterialTheme.typography.titleMedium.fontSize, fontWeight = FontWeight.SemiBold)
                    }
                },
            )
        },
        content = { paddingValues ->
            LazyColumn(modifier = modifier.fillMaxSize().padding(paddingValues).padding(horizontal = 16.dp)) {
                item {
                    SectionHeader(title = "General")

                    RoundedCard {
                        SwitchRow(
                            label = "Haptics",
                            isChecked = viewModel.haptics,
                            onCheckedChange = { viewModel.handleHaptics(value = it) },
                            description = "Provide haptic feedback when balls collide with flippers, bumpers, and slingshots.",
                        )

                        HorizontalDivider(modifier = Modifier.padding(vertical = 4.dp))

                        SwitchRow(label = "AltColor", isChecked = viewModel.altColor, onCheckedChange = { viewModel.handleAltColor(value = it) })

                        HorizontalDivider(modifier = Modifier.padding(vertical = 4.dp))

                        SwitchRow(label = "AltSound", isChecked = viewModel.altSound, onCheckedChange = { viewModel.handleAltSound(value = it) })

                        HorizontalDivider(modifier = Modifier.padding(vertical = 4.dp))

                        SwitchRow(
                            label = "Force VR Rendering Mode",
                            isChecked = viewModel.renderingModeOverride,
                            onCheckedChange = { viewModel.handleRenderingModeOverride(value = it) },
                            description =
                                "Provide table scripts with `RenderingMode=2` " +
                                    "so backbox and cabinet are rendered. Useful for tables that do not provide FSS support.",
                        )

                        HorizontalDivider(modifier = Modifier.padding(vertical = 4.dp))

                        SwitchRow(
                            label = "Mobile LiveUI",
                            isChecked = viewModel.liveUIOverride,
                            onCheckedChange = { viewModel.handleLiveUIOverride(value = it) },
                            description = "If disabled, use Visual Pinball's built-in LiveUI",
                        )

                        HorizontalDivider(modifier = Modifier.padding(vertical = 4.dp))

                        EnumMenuRow(
                            label = "Graphics Backend",
                            options = VPinballGfxBackend.entries.toList(),
                            option = viewModel.gfxBackend,
                            onOptionChanged = { viewModel.handleGfxBackend(value = it) },
                        )
                    }
                }

                item {
                    SectionHeader(title = "External DMD")

                    RoundedCard {
                        EnumMenuRow(
                            label = "DMD Type",
                            options = VPinballExternalDMD.entries.toList(),
                            option = viewModel.externalDMD,
                            onOptionChanged = { viewModel.handleExternalDMD(value = it) },
                        )

                        if (viewModel.externalDMD != VPinballExternalDMD.NONE) {
                            HorizontalDivider(modifier = Modifier.padding(vertical = 4.dp))

                            if (viewModel.externalDMD == VPinballExternalDMD.DMD_SERVER) {
                                IPAddressPortInputRow(
                                    label = "DMDServer Address",
                                    value = viewModel.dmdServerAddr,
                                    validateAsIpAddress = true,
                                    onValueChange = { viewModel.handleDMDServerAddr(value = it) },
                                )
                                HorizontalDivider(modifier = Modifier.padding(vertical = 4.dp))

                                IPAddressPortInputRow(
                                    label = "DMDServer Port",
                                    value = viewModel.dmdServerPort.toString(),
                                    validateAsIpAddress = false,
                                    onValueChange = { viewModel.handleDMDServerPort(value = it.toInt()) },
                                )
                            } else if (viewModel.externalDMD == VPinballExternalDMD.ZEDMD_WIFI) {
                                IPAddressPortInputRow(
                                    label = "ZeDMD WiFi Address",
                                    value = viewModel.zedmdWiFiAddr,
                                    validateAsIpAddress = true,
                                    onValueChange = { viewModel.handleZeDMDWiFiAddr(value = it) },
                                )
                            }
                        }
                    }

                    Text(
                        text =
                            buildAnnotatedString {
                                append("Send PinMAME and FlexDMD DMDs to ZeDMD and Pixelcade devices using ")
                                withLink(LinkAnnotation.Url(Link.LIBDMDUTIL.url, TextLinkStyles(style = SpanStyle(color = Color.VpxRed)))) {
                                    append("DMDServer")
                                }
                                append(" and ")
                                withLink(LinkAnnotation.Url(Link.ZEDMDOS.url, TextLinkStyles(style = SpanStyle(color = Color.VpxRed)))) {
                                    append("ZeDMDOS")
                                }
                                append(".")
                            },
                        style = MaterialTheme.typography.bodyMedium,
                        color = MaterialTheme.colorScheme.onSurfaceVariant,
                        modifier = Modifier.padding(horizontal = 16.dp).padding(bottom = 8.dp),
                    )
                }

                item {
                    RoundedCard {
                        EnumMenuRow(
                            label = "Display",
                            options = VPinballViewMode.entries.toList(),
                            option = viewModel.bgSet,
                            onOptionChanged = { viewModel.handleBGSet(value = it) },
                        )
                    }
                }

                item {
                    SectionHeader(title = "Environment Lighting")

                    RoundedCard {
                        SwitchRow(
                            label = "Override Table Global Lighting",
                            isChecked = viewModel.overrideEmissionScale,
                            onCheckedChange = { viewModel.handleOverrideEmissionScale(value = it) },
                        )

                        HorizontalDivider(modifier = Modifier.padding(vertical = 4.dp))

                        SliderRow(
                            label = "Night/Day",
                            value = viewModel.emissionScale.toFloat(),
                            onValueChange = { viewModel.handleEmissionScale(value = it.toInt()) },
                            minValue = 0f,
                            maxValue = 100f,
                            enabled = viewModel.overrideEmissionScale,
                        )
                    }
                }

                item {
                    SectionHeader(title = "Ball Rendering")

                    RoundedCard {
                        SwitchRow(label = "Ball Trails", isChecked = viewModel.ballTrail, onCheckedChange = { viewModel.handleBallTrail(value = it) })

                        HorizontalDivider(modifier = Modifier.padding(vertical = 4.dp))

                        SliderRow(
                            label = "Strength",
                            value = viewModel.ballTrailStrength,
                            onValueChange = { viewModel.handleBallTrailStrength(value = it) },
                            minValue = 0f,
                            maxValue = 2f,
                        )

                        HorizontalDivider(modifier = Modifier.padding(vertical = 4.dp))

                        SwitchRow(
                            label = "Force Round Ball",
                            isChecked = viewModel.ballAntiStretch,
                            onCheckedChange = { viewModel.handleBallAntiStretch(value = it) },
                            description = "Compensate perspective stretch",
                        )

                        HorizontalDivider(modifier = Modifier.padding(vertical = 4.dp))

                        SwitchRow(
                            label = "Disable Lighting",
                            isChecked = viewModel.disableLightingForBalls,
                            onCheckedChange = { viewModel.handleDisableLightingForBalls(value = it) },
                        )
                    }
                }

                item {
                    SectionHeader(title = "Performance")

                    RoundedCard {
                        EnumMenuRow(
                            label = "Max Ambient Occlusion",
                            options = VPinballAO.entries.toList(),
                            option = viewModel.maxAO,
                            onOptionChanged = { viewModel.handleMaxAO(value = it) },
                        )

                        HorizontalDivider(modifier = Modifier.padding(vertical = 4.dp))

                        EnumMenuRow(
                            label = "Max Reflection Mode",
                            options = VPinballReflectionMode.entries.toList(),
                            option = viewModel.pfReflection,
                            onOptionChanged = { viewModel.handlePFReflection(value = it) },
                            sameLine = false,
                        )

                        HorizontalDivider(modifier = Modifier.padding(vertical = 4.dp))

                        EnumSliderRow(
                            label = "Max Texture Dimensions",
                            options = VPinballMaxTexDimension.entries.toList(),
                            value = viewModel.maxTexDimension,
                            onValueChange = { viewModel.handleMaxTexDimension(value = it) },
                            description = "Reduce this value if you experience crashes while loading tables.",
                        )

                        HorizontalDivider(modifier = Modifier.padding(vertical = 4.dp))

                        SwitchRow(
                            label = "Force Anisotropic Texture Filtering",
                            isChecked = viewModel.forceAnisotropicFiltering,
                            onCheckedChange = { viewModel.handleForceAnisotropicFiltering(value = it) },
                        )

                        HorizontalDivider(modifier = Modifier.padding(vertical = 4.dp))

                        SwitchRow(
                            label = "Force Bloom Off",
                            isChecked = viewModel.forceBloomOff,
                            onCheckedChange = { viewModel.handleForceBloomOff(value = it) },
                        )

                        HorizontalDivider(modifier = Modifier.padding(vertical = 4.dp))

                        SwitchRow(
                            label = "Disable Motion Blur",
                            isChecked = viewModel.forceMotionBlurOff,
                            onCheckedChange = { viewModel.handleForceMotionBlurOff(value = it) },
                        )

                        HorizontalDivider(modifier = Modifier.padding(vertical = 4.dp))

                        SliderRow(
                            label = "Elements Detail Level",
                            value = viewModel.alphaRampAccuracy.toFloat(),
                            onValueChange = { viewModel.handleAlphaRampAccuracy(value = it.toInt()) },
                            minValue = 0f,
                            maxValue = 10f,
                            description = "Decrease to increase performance.",
                        )
                    }
                }

                item {
                    SectionHeader(title = "Anti-Aliasing")

                    RoundedCard {
                        EnumSliderRow(
                            label = "MSAA Samples",
                            options = VPinballMSAASamples.entries.toList(),
                            value = viewModel.msaaSamples,
                            onValueChange = { viewModel.handleMSAASamples(value = it) },
                        )

                        HorizontalDivider(modifier = Modifier.padding(vertical = 4.dp))

                        EnumSliderRow(
                            label = "Supersampling",
                            options = VPinballAAFactor.entries.toList(),
                            value = viewModel.aaFactor,
                            onValueChange = { viewModel.handleAAFactor(value = it) },
                        )

                        HorizontalDivider(modifier = Modifier.padding(vertical = 4.dp))

                        EnumMenuRow(
                            label = "Post-processed AA",
                            options = VPinballFXAA.entries.toList(),
                            option = viewModel.fxaa,
                            onOptionChanged = { viewModel.handleFXAA(value = it) },
                        )

                        HorizontalDivider(modifier = Modifier.padding(vertical = 4.dp))

                        EnumMenuRow(
                            label = "Sharpen",
                            options = VPinballSharpen.entries.toList(),
                            option = viewModel.sharpen,
                            onOptionChanged = { viewModel.handleSharpen(value = it) },
                        )

                        HorizontalDivider(modifier = Modifier.padding(vertical = 4.dp))

                        SwitchRow(
                            label = "Enable ScaleFX for internal DMD",
                            isChecked = viewModel.scaleFXDMD,
                            onCheckedChange = { viewModel.handleScaleFXDMD(value = it) },
                        )
                    }
                }

                item {
                    SectionHeader(title = "Miscellaneous Features")

                    RoundedCard {
                        SwitchRow(
                            label = "Advanced Screen Space Reflections",
                            isChecked = viewModel.ssRefl,
                            onCheckedChange = { viewModel.handleSSRefl(value = it) },
                        )
                    }
                }

                item {
                    SectionHeader(title = "Web Server")

                    RoundedCard {
                        SwitchRow(label = "Enabled", isChecked = viewModel.webServer, onCheckedChange = { viewModel.handleWebServer(value = it) })

                        HorizontalDivider(modifier = Modifier.padding(vertical = 4.dp))

                        IPAddressPortInputRow(
                            label = "Web Server Port",
                            value = viewModel.webServerPort.toString(),
                            onValueChange = { viewModel.handleWebServerPort(value = it.toInt()) },
                            validateAsIpAddress = false,
                        )
                    }

                    if (viewModel.webServer) {
                        Text(
                            text =
                                buildAnnotatedString {
                                    append("Web Server is running and can be accessed at: ")
                                    withLink(LinkAnnotation.Url(webServerURL, TextLinkStyles(style = SpanStyle(color = Color.VpxRed)))) {
                                        append(webServerURL)
                                    }
                                },
                            style = MaterialTheme.typography.bodyMedium,
                            color = MaterialTheme.colorScheme.onSurfaceVariant,
                            modifier = Modifier.padding(horizontal = 16.dp).padding(bottom = 8.dp),
                        )
                    } else {
                        Text(
                            text = "Web Server is not running.",
                            style = MaterialTheme.typography.bodyMedium,
                            color = MaterialTheme.colorScheme.onSurfaceVariant,
                            modifier = Modifier.padding(horizontal = 16.dp).padding(bottom = 8.dp),
                        )
                    }
                }

                item {
                    SectionHeader(title = "Advanced")

                    RoundedCard {
                        SwitchRow(
                            label = "Reset Log on Play",
                            isChecked = viewModel.resetLogOnPlay,
                            onCheckedChange = { viewModel.handleResetLogOnPlay(value = it) },
                        )

                        HorizontalDivider(modifier = Modifier.padding(vertical = 4.dp))

                        ActionRow(
                            label = "Export vpinball.log...",
                            labelColor = Color.VpxRed,
                            onClick = { onViewFile(File(VPinballManager.getFilesDir(), "vpinball.log")) },
                            showDisclosure = false,
                        )
                    }
                }

                item {
                    RoundedCard {
                        ActionRow(
                            label = "Export VPinballX.ini...",
                            labelColor = Color.VpxRed,
                            onClick = { onViewFile(File(VPinballManager.getFilesDir(), "VPinballX.ini")) },
                            showDisclosure = false,
                        )
                    }
                }

                item {
                    SectionHeader(title = "Support")

                    RoundedCard {
                        ActionRow(label = "Learn More", onClick = { Link.DOCS.open(context) })

                        HorizontalDivider(modifier = Modifier.padding(vertical = 4.dp))

                        ActionRow(
                            label = "Contact Us",
                            onClick = {
                                val intent =
                                    Intent(Intent.ACTION_SEND).apply {
                                        type = "message/rfc822"
                                        putExtra(Intent.EXTRA_EMAIL, arrayOf("jsm174@gmail.com"))
                                        putExtra(Intent.EXTRA_SUBJECT, "Visual Pinball Feedback")
                                    }

                                context.startActivity(Intent.createChooser(intent, "Choose an Email Client"))
                            },
                        )

                        HorizontalDivider(modifier = Modifier.padding(vertical = 4.dp))

                        ActionRow(label = "Discord (Virtual Pinball Chat)", onClick = { Link.DISCORD.open(context) })
                    }
                }

                item {
                    SectionHeader(title = "Credits")

                    RoundedCard {
                        Credit.entries.forEachIndexed { index, credit ->
                            CreditRow(credit = credit, context = LocalContext.current)

                            if (index < Credit.entries.size - 1) {
                                HorizontalDivider(modifier = Modifier.padding(vertical = 4.dp))
                            }
                        }
                    }
                }

                item { RoundedCard { ActionRow(label = "License", onClick = { Link.LICENSES.open(context) }) } }

                item {
                    RoundedCard {
                        ActionRow(label = "Reset", labelColor = Color.VpxRed, onClick = { showResetDialog = true }, showDisclosure = false)
                    }
                }

                item {
                    Text(
                        text = VPinballManager.getVersionString(),
                        style = MaterialTheme.typography.bodySmall,
                        textAlign = TextAlign.Center,
                        modifier = Modifier.fillMaxWidth().padding(vertical = 20.dp),
                    )
                }
            }
        },
    )

    if (showResetDialog) {
        val coroutineScope = rememberCoroutineScope()

        ModalBottomSheet(
            onDismissRequest = { showResetDialog = false },
            sheetState = sheetState,
            containerColor = MaterialTheme.colorScheme.surface,
        ) {
            Column(modifier = Modifier.padding(16.dp)) {
                RoundedCard {
                    TextButton(
                        onClick = {
                            viewModel.handleResetTouchInstructions()
                            showResetDialog = false
                        },
                        modifier = Modifier.fillMaxWidth(),
                    ) {
                        Text(
                            text = "Reset Touch Instructions",
                            color = Color.VpxRed,
                            style = MaterialTheme.typography.bodyLarge,
                            fontWeight = FontWeight.Normal,
                        )
                    }

                    HorizontalDivider(modifier = Modifier.padding(vertical = 4.dp))

                    TextButton(
                        onClick = {
                            viewModel.handleResetAllSettings()
                            showResetDialog = false
                            coroutineScope.launch { onDone() }
                        },
                        modifier = Modifier.fillMaxWidth(),
                    ) {
                        Text(
                            text = "Reset All Settings",
                            color = Color.VpxRed,
                            style = MaterialTheme.typography.bodyLarge,
                            fontWeight = FontWeight.Normal,
                        )
                    }
                }

                Spacer(modifier = Modifier.height(8.dp))

                RoundedCard {
                    TextButton(onClick = { showResetDialog = false }, modifier = Modifier.fillMaxWidth()) {
                        Text(text = "Cancel", color = Color.VpxRed, style = MaterialTheme.typography.titleMedium, fontWeight = FontWeight.SemiBold)
                    }
                }
            }
        }
    }
}

@Composable
private fun SectionHeader(title: String) {
    Text(
        text = title.uppercase(),
        color = MaterialTheme.colorScheme.onSurfaceVariant,
        style = MaterialTheme.typography.titleSmall,
        modifier = Modifier.padding(horizontal = 16.dp).padding(top = 16.dp),
    )
}

@Composable
private fun SwitchRow(
    label: String,
    isChecked: Boolean,
    onCheckedChange: (Boolean) -> Unit,
    modifier: Modifier = Modifier,
    description: String? = null,
) {
    Column(modifier = modifier) {
        Row(verticalAlignment = Alignment.CenterVertically) {
            Text(
                text = label,
                style = MaterialTheme.typography.bodyLarge,
                color = MaterialTheme.colorScheme.onSurface,
                modifier = Modifier.weight(1f),
            )

            Switch(
                checked = isChecked,
                onCheckedChange = onCheckedChange,
                colors = SwitchDefaults.colors(checkedThumbColor = Color.White, checkedTrackColor = Color.VpxRed),
            )
        }

        if (!description.isNullOrEmpty()) {
            Text(
                text = description,
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant,
                modifier = Modifier.padding(bottom = 8.dp),
            )
        }
    }
}

@Composable
@OptIn(ExperimentalMaterial3Api::class)
private fun SliderRow(
    label: String,
    value: Float,
    onValueChange: (Float) -> Unit,
    enabled: Boolean = true,
    minValue: Float = 0f,
    maxValue: Float = 100f,
    description: String? = null,
) {
    val alpha = if (enabled) 1f else 0.5f

    Column(modifier = Modifier.alpha(alpha).then(if (!enabled) Modifier.pointerInput(Unit) {} else Modifier)) {
        Text(
            text = label,
            style = MaterialTheme.typography.bodyLarge,
            color = MaterialTheme.colorScheme.onSurface,
            modifier = Modifier.padding(top = 8.dp),
        )

        Row(verticalAlignment = Alignment.CenterVertically) {
            Slider(
                value = value,
                onValueChange = onValueChange,
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
                modifier = Modifier.padding(vertical = 8.dp).weight(1f),
                enabled = enabled,
            )
        }

        if (!description.isNullOrEmpty()) {
            Text(
                text = description,
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant,
                modifier = Modifier.padding(bottom = 8.dp),
            )
        }
    }
}

@Composable
private fun <T> EnumMenuRow(label: String, options: List<T>, option: T, onOptionChanged: (T) -> Unit, sameLine: Boolean = true) where
T : VPinballDisplayText {
    var expanded by remember { mutableStateOf(false) }

    Column(modifier = Modifier) {
        if (!sameLine) {
            Text(
                text = label,
                style = MaterialTheme.typography.bodyLarge,
                color = MaterialTheme.colorScheme.onSurface,
                modifier = Modifier.padding(vertical = 8.dp),
            )
        }

        Row(modifier = Modifier.padding(vertical = 8.dp), verticalAlignment = Alignment.CenterVertically) {
            Text(
                text = if (sameLine) label else "",
                style = MaterialTheme.typography.bodyLarge,
                color = MaterialTheme.colorScheme.onSurface,
                modifier = Modifier.weight(1f),
            )

            Box {
                Row(verticalAlignment = Alignment.CenterVertically, modifier = Modifier.clickable { expanded = true }) {
                    Text(text = option.text, style = MaterialTheme.typography.bodyLarge, color = MaterialTheme.colorScheme.onSurfaceVariant)

                    Icon(
                        painter = painterResource(id = R.drawable.img_sf_chevron_up_chevron_down),
                        contentDescription = null,
                        tint = MaterialTheme.colorScheme.onSurfaceVariant,
                        modifier = Modifier.size(14.dp).padding(start = 4.dp),
                    )
                }

                DropdownMenu(expanded = expanded, onDismissRequest = { expanded = false }, offset = DpOffset(x = 0.dp, y = 0.dp)) {
                    options.forEach { option ->
                        DropdownMenuItem(
                            text = {
                                Text(
                                    text = option.text,
                                    style = MaterialTheme.typography.bodyLarge,
                                    color = MaterialTheme.colorScheme.onSurfaceVariant,
                                )
                            },
                            onClick = {
                                onOptionChanged(option)
                                expanded = false
                            },
                        )
                    }
                }
            }
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
private fun <T> EnumSliderRow(label: String, options: List<T>, value: T, onValueChange: (T) -> Unit, description: String? = null) where
T : VPinballDisplayText {
    Column(modifier = Modifier) {
        Text(
            text = label,
            style = MaterialTheme.typography.bodyLarge,
            color = MaterialTheme.colorScheme.onSurface,
            modifier = Modifier.padding(top = 8.dp),
        )

        Row(modifier = Modifier.padding(vertical = 8.dp), verticalAlignment = Alignment.CenterVertically) {
            Slider(
                value = options.indexOf(value).toFloat(),
                onValueChange = { onValueChange(options[it.toInt()]) },
                valueRange = 0f..(options.size - 1).toFloat(),
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
                text = value.text,
                style = MaterialTheme.typography.bodyLarge,
                color = MaterialTheme.colorScheme.onSurfaceVariant,
                modifier = Modifier.padding(start = 16.dp),
            )
        }

        if (!description.isNullOrEmpty()) {
            Text(
                text = description,
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant,
                modifier = Modifier.padding(bottom = 8.dp),
            )
        }
    }
}

@Composable
private fun IPAddressPortInputRow(
    label: String,
    value: String,
    onValueChange: (String) -> Unit,
    modifier: Modifier = Modifier,
    validateAsIpAddress: Boolean = true,
) {
    var isDialogOpen by remember { mutableStateOf(false) }
    var tempValue by remember { mutableStateOf(TextFieldValue(value)) }
    val focusRequester = remember { FocusRequester() }

    fun isValidIpAddress(value: String): Boolean {
        val ipOrMdnsRegex =
            Regex(
                "^((25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9]?[0-9])\\.(25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9]?[0-9])\\." +
                    "(25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9]?[0-9])\\.(25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9]?[0-9]))\$|" +
                    "^[a-zA-Z0-9-]+(\\.[a-zA-Z0-9-]+)*\\.local\$"
            )
        return ipOrMdnsRegex.matches(value)
    }

    fun isValidPort(value: String): Boolean = value.toIntOrNull()?.let { it in 0..65535 } == true

    Row(verticalAlignment = Alignment.CenterVertically, modifier = modifier.padding(vertical = 8.dp)) {
        Text(text = label, style = MaterialTheme.typography.bodyLarge, color = MaterialTheme.colorScheme.onSurface, modifier = Modifier.weight(1f))

        Text(
            text = value,
            style = MaterialTheme.typography.bodyLarge,
            color = MaterialTheme.colorScheme.onSurfaceVariant,
            modifier = Modifier.clickable { isDialogOpen = true },
        )
    }

    if (isDialogOpen) {
        AlertDialog(
            title = { Text(text = label, style = MaterialTheme.typography.titleMedium) },
            text = {
                OutlinedTextField(
                    value = tempValue,
                    onValueChange = { tempValue = it },
                    colors =
                        OutlinedTextFieldDefaults.colors(
                            cursorColor = Color.VpxRed,
                            selectionColors = TextSelectionColors(handleColor = Color.Transparent, backgroundColor = Color.VpxRed.copy(alpha = 0.5f)),
                            focusedBorderColor = MaterialTheme.colorScheme.onSurfaceVariant,
                        ),
                    singleLine = true,
                    modifier = Modifier.focusRequester(focusRequester),
                )
                LaunchedEffect(Unit) {
                    tempValue = tempValue.copy(selection = TextRange(tempValue.text.length))
                    focusRequester.requestFocus()
                }
            },
            onDismissRequest = {},
            confirmButton = {
                TextButton(
                    onClick = {
                        onValueChange(tempValue.text)
                        isDialogOpen = false
                    },
                    enabled = if (validateAsIpAddress) isValidIpAddress(tempValue.text) else isValidPort(tempValue.text),
                ) {
                    Text(
                        text = "OK",
                        color =
                            if (validateAsIpAddress && isValidIpAddress(tempValue.text) || !validateAsIpAddress && isValidPort(tempValue.text)) {
                                Color.VpxRed
                            } else {
                                Color.Gray
                            },
                        fontSize = MaterialTheme.typography.titleMedium.fontSize,
                        fontWeight = FontWeight.SemiBold,
                    )
                }
            },
            dismissButton = {
                TextButton(onClick = { isDialogOpen = false }) {
                    Text(
                        text = "Cancel",
                        color = Color.VpxRed,
                        fontSize = MaterialTheme.typography.titleMedium.fontSize,
                        fontWeight = FontWeight.SemiBold,
                    )
                }
            },
        )
    }
}

@Composable
private fun ActionRow(
    label: String,
    onClick: () -> Unit,
    modifier: Modifier = Modifier,
    showDisclosure: Boolean = true,
    labelColor: Color = MaterialTheme.colorScheme.onSurface,
) {
    Row(verticalAlignment = Alignment.CenterVertically, modifier = modifier.padding(vertical = 8.dp).clickable { onClick() }) {
        Text(text = label, style = MaterialTheme.typography.bodyLarge, color = labelColor, modifier = Modifier.weight(1f))

        if (showDisclosure) {
            Icon(
                painter = painterResource(id = R.drawable.img_sf_chevron_right),
                contentDescription = null,
                tint = MaterialTheme.colorScheme.onSurfaceVariant,
                modifier = Modifier.size(12.dp),
            )
        }
    }
}

@Composable
private fun CreditRow(credit: Credit, context: Context) {
    Column(
        modifier =
            Modifier.then(
                    if (credit.link != null) {
                        Modifier.clickable { credit.link.open(context) }
                    } else {
                        Modifier
                    }
                )
                .padding(vertical = 8.dp)
    ) {
        Row(verticalAlignment = Alignment.CenterVertically) {
            Text(
                text = credit.displayName,
                style = MaterialTheme.typography.bodyLarge,
                color = MaterialTheme.colorScheme.onSurface,
                modifier = Modifier.weight(1f),
            )

            if (credit.link != null) {
                Icon(
                    painter = painterResource(id = R.drawable.img_sf_chevron_right),
                    contentDescription = null,
                    tint = MaterialTheme.colorScheme.onSurfaceVariant,
                    modifier = Modifier.size(12.dp),
                )
            }
        }

        credit.authors?.let {
            Text(
                text = it,
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant,
                modifier = Modifier.padding(top = 8.dp),
            )
        }
    }
}

@Preview
@Composable
private fun SettingsScreenPreview() {
    VPinballTheme { SettingsScreen("test.url", onDone = {}, onViewFile = { _ -> }, viewModel = SettingsViewModel()) }
}
