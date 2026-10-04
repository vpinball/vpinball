package org.vpinball.app.ui.screens.settings

import androidx.activity.compose.BackHandler
import androidx.compose.animation.core.animateFloatAsState
import androidx.compose.animation.core.tween
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.gestures.detectVerticalDragGestures
import androidx.compose.foundation.interaction.MutableInteractionSource
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.navigationBarsPadding
import androidx.compose.foundation.layout.offset
import androidx.compose.foundation.layout.statusBarsPadding
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableFloatStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.platform.LocalConfiguration
import androidx.compose.ui.platform.LocalDensity
import androidx.compose.ui.unit.IntOffset
import androidx.compose.ui.unit.dp
import androidx.compose.ui.window.Dialog
import androidx.compose.ui.window.DialogProperties
import java.io.File
import kotlin.math.roundToInt
import kotlinx.coroutines.launch
import org.koin.androidx.compose.koinViewModel

@Composable
fun SettingsBottomSheet(
    webServerURL: String,
    show: Boolean,
    onDismissRequest: () -> Unit,
    onViewFile: (file: File) -> Unit,
    modifier: Modifier = Modifier,
    viewModel: SettingsViewModel = koinViewModel(),
) {
    if (!show) return

    val coroutineScope = rememberCoroutineScope()
    val configuration = LocalConfiguration.current
    val density = LocalDensity.current
    val screenHeightPx = with(density) { configuration.screenHeightDp.dp.toPx() }

    var dragOffset by remember { mutableFloatStateOf(0f) }
    var isVisible by remember { mutableFloatStateOf(0f) }

    val animatedOffset by
        animateFloatAsState(
            targetValue = if (isVisible > 0.5f) dragOffset else screenHeightPx,
            animationSpec = tween(durationMillis = 300),
            label = "sheetOffset",
        )

    val animatedScrim by
        animateFloatAsState(targetValue = if (isVisible > 0.5f) 0.5f else 0f, animationSpec = tween(durationMillis = 300), label = "scrimAlpha")

    LaunchedEffect(show) { isVisible = 1f }

    fun dismiss() {
        coroutineScope.launch {
            viewModel.triggerTableReloadIfNeeded()
            isVisible = 0f
            kotlinx.coroutines.delay(300)
            onDismissRequest()
        }
    }

    Dialog(
        onDismissRequest = { dismiss() },
        properties =
            DialogProperties(
                dismissOnBackPress = true,
                dismissOnClickOutside = false,
                usePlatformDefaultWidth = false,
                decorFitsSystemWindows = false,
            ),
    ) {
        BackHandler { dismiss() }

        Box(modifier = modifier.fillMaxSize()) {
            Box(
                modifier =
                    Modifier.fillMaxSize().background(Color.Black.copy(alpha = animatedScrim)).clickable(
                        interactionSource = remember { MutableInteractionSource() },
                        indication = null,
                    ) {
                        dismiss()
                    }
            )

            Surface(
                modifier =
                    Modifier.align(Alignment.BottomCenter)
                        .fillMaxWidth()
                        .statusBarsPadding()
                        .offset { IntOffset(0, animatedOffset.roundToInt()) }
                        .pointerInput(Unit) {
                            detectVerticalDragGestures(
                                onDragEnd = {
                                    if (dragOffset > screenHeightPx * 0.2f) {
                                        dismiss()
                                    } else {
                                        dragOffset = 0f
                                    }
                                },
                                onVerticalDrag = { _, dragAmount -> dragOffset = (dragOffset + dragAmount).coerceAtLeast(0f) },
                            )
                        },
                color = MaterialTheme.colorScheme.background,
                shape = RoundedCornerShape(topStart = 12.dp, topEnd = 12.dp),
            ) {
                Box(modifier = Modifier.navigationBarsPadding()) {
                    SettingsScreen(webServerURL = webServerURL, onDone = { dismiss() }, onViewFile = onViewFile, viewModel = viewModel)
                }
            }
        }
    }
}
