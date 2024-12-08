package org.vpinball.app.ui.screens.touch

import androidx.compose.animation.core.animateFloatAsState
import androidx.compose.animation.core.tween
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.BoxWithConstraints
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.absoluteOffset
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.Checkbox
import androidx.compose.material3.CheckboxDefaults
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableStateListOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.runtime.snapshots.SnapshotStateList
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.scale
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.ImageBitmap
import androidx.compose.ui.graphics.asImageBitmap
import androidx.compose.ui.platform.LocalDensity
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import dev.chrisbanes.haze.HazeDefaults
import dev.chrisbanes.haze.HazeState
import dev.chrisbanes.haze.haze
import dev.chrisbanes.haze.hazeChild
import kotlinx.coroutines.delay
import org.vpinball.app.R
import org.vpinball.app.VPinballManager
import org.vpinball.app.VPinballViewModel
import org.vpinball.app.jni.VPinballSettingsSection.STANDALONE
import org.vpinball.app.jni.VPinballTouchAreas
import org.vpinball.app.ui.theme.Orange
import org.vpinball.app.ui.theme.Pink
import org.vpinball.app.ui.theme.Purple
import org.vpinball.app.ui.theme.VpxRed
import org.vpinball.app.ui.util.koinActivityViewModel
import org.vpinball.app.util.drawWithGradient

@Composable
fun TouchInstructionsScreen(viewModel: VPinballViewModel = koinActivityViewModel()) {
    var activeGroupIndex by remember { mutableIntStateOf(-1) }
    var showAll by remember { mutableStateOf(false) }
    val scaleEffects = rememberScaleEffects()

    var showOverlay by remember { mutableStateOf(false) }
    var dontShowAgain by remember { mutableStateOf(false) }

    var tableImageBitmap by remember { mutableStateOf<ImageBitmap?>(null) }

    val groupColors = listOf(Color.Red, Color.Green, Color.Blue, Color.Orange, Color.Purple, Color.Yellow, Color.Pink)

    val scaleDuration = 500
    val holdDuration = 1000

    val hazeState = remember { HazeState() }

    LaunchedEffect(Unit) {
        while (true) {
            VPinballManager.captureBitmap { bitmap -> tableImageBitmap = bitmap?.asImageBitmap() }
            delay(30)
        }
    }

    LaunchedEffect(Unit) {
        delay(500)

        animateGroups(scaleEffects, scaleDuration, holdDuration) { groupIndex -> activeGroupIndex = groupIndex }
        delay(scaleDuration.toLong())
        showAll = true
        showOverlay = true
    }

    BoxWithConstraints(modifier = Modifier.fillMaxSize()) {
        val width = constraints.maxWidth.toFloat()
        val height = constraints.maxHeight.toFloat()

        val density = LocalDensity.current

        tableImageBitmap?.let { bitmap ->
            Image(bitmap = bitmap, contentDescription = null, modifier = Modifier.fillMaxSize().haze(state = hazeState))
        }

        VPinballTouchAreas.forEachIndexed { groupIndex, group ->
            group.forEachIndexed { areaIndex, region ->
                val animatedScale by
                    animateFloatAsState(
                        targetValue =
                            if (showAll || (groupIndex == activeGroupIndex && scaleEffects[groupIndex][areaIndex] > 1)) {
                                1.5f
                            } else {
                                1f
                            },
                        animationSpec = tween(durationMillis = scaleDuration),
                        label = "",
                    )

                val animatedAlpha by
                    animateFloatAsState(
                        targetValue = if (showAll || groupIndex == activeGroupIndex) 1f else 0f,
                        animationSpec = tween(durationMillis = scaleDuration),
                        label = "",
                    )

                Box(
                    modifier =
                        Modifier.absoluteOffset(
                                x = with(density) { (region.left / 100f * width).toDp() },
                                y = with(density) { (region.top / 100f * height).toDp() },
                            )
                            .size(
                                width = with(density) { ((region.right - region.left) / 100f * width).toDp() },
                                height = with(density) { ((region.bottom - region.top) / 100f * height).toDp() },
                            )
                            .border(1.dp, Color.White.copy(alpha = 0.5f * animatedAlpha)),
                    contentAlignment = Alignment.Center,
                ) {
                    if (animatedAlpha > 0f) {
                        Box(modifier = Modifier.fillMaxSize().background(groupColors[groupIndex].copy(alpha = 0.8f * animatedAlpha)))

                        Box(
                            modifier =
                                Modifier.fillMaxSize().hazeChild(state = hazeState) {
                                    backgroundColor = Color.Black
                                    blurRadius = 20.dp
                                    noiseFactor = HazeDefaults.noiseFactor
                                    alpha = 0.8f * animatedAlpha
                                }
                        ) {
                            Text(
                                text = region.label,
                                fontSize = 12.sp,
                                color = Color.White.copy(alpha = animatedAlpha),
                                textAlign = TextAlign.Center,
                                modifier = Modifier.scale(animatedScale).align(Alignment.Center),
                            )
                        }
                    }
                }
            }
        }

        if (showOverlay) {
            Box(
                modifier =
                    Modifier.fillMaxSize().background(Color.Black.copy(alpha = 0.5f)).clickable {
                        showOverlay = false
                        viewModel.touchInstructions(false)
                    },
                contentAlignment = Alignment.Center,
            ) {
                Column(
                    horizontalAlignment = Alignment.CenterHorizontally,
                    modifier = Modifier.background(Color.Black.copy(alpha = 0.8f), shape = RoundedCornerShape(12.dp)).padding(20.dp),
                    verticalArrangement = Arrangement.spacedBy(12.dp),
                ) {
                    Text(text = "Launch Ball", style = MaterialTheme.typography.titleMedium, color = Color.White, textAlign = TextAlign.Center)

                    Image(
                        painter = painterResource(R.drawable.img_svgrepo_arcade_button),
                        contentDescription = null,
                        modifier = Modifier.size(50.dp).drawWithGradient(),
                    )

                    Row(modifier = Modifier, verticalAlignment = Alignment.CenterVertically) {
                        Box(modifier = Modifier.size(12.dp).scale(0.8f)) {
                            Checkbox(
                                checked = dontShowAgain,
                                onCheckedChange = {
                                    dontShowAgain = it

                                    VPinballManager.saveValue(STANDALONE, "TouchInstructions", !it)
                                },
                                colors =
                                    CheckboxDefaults.colors(checkedColor = Color.VpxRed, uncheckedColor = Color.VpxRed, checkmarkColor = Color.Black),
                            )
                        }

                        Spacer(modifier = Modifier.width(8.dp))

                        Text(text = "Don't show again", style = MaterialTheme.typography.bodySmall, color = Color.White)
                    }
                }
            }
        }
    }
}

suspend fun animateGroups(
    scaleEffects: SnapshotStateList<SnapshotStateList<Float>>,
    scaleDuration: Int,
    holdDuration: Int,
    updateActiveGroup: (Int) -> Unit,
) {
    for (groupIndex in VPinballTouchAreas.indices) {
        updateActiveGroup(groupIndex)

        VPinballTouchAreas[groupIndex].forEachIndexed { areaIndex, _ -> scaleEffects[groupIndex][areaIndex] = 1.5f }

        delay(holdDuration.toLong())

        VPinballTouchAreas[groupIndex].forEachIndexed { areaIndex, _ -> scaleEffects[groupIndex][areaIndex] = 1f }

        delay(scaleDuration.toLong())
    }

    updateActiveGroup(-1)
}

@Composable
fun rememberScaleEffects(): SnapshotStateList<SnapshotStateList<Float>> =
    mutableStateListOf<SnapshotStateList<Float>>().apply {
        VPinballTouchAreas.forEach { group -> add(mutableStateListOf<Float>().apply { repeat(group.size) { add(1f) } }) }
    }
