package org.vpinball.app.ui.screens.touch

import androidx.compose.foundation.border
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.BoxWithConstraints
import androidx.compose.foundation.layout.absoluteOffset
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.size
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalDensity
import androidx.compose.ui.unit.dp
import org.vpinball.app.jni.VPinballTouchAreas

@Composable
fun TouchOverlayScreen() {
    val density = LocalDensity.current

    BoxWithConstraints(modifier = Modifier.fillMaxSize()) {
        val screenWidthPx = constraints.maxWidth.toFloat()
        val screenHeightPx = constraints.maxHeight.toFloat()

        VPinballTouchAreas.forEach { group ->
            group.forEach { region ->
                Box(
                    modifier =
                        Modifier.absoluteOffset(
                                x = with(density) { (region.left / 100f * screenWidthPx).toDp() },
                                y = with(density) { (region.top / 100f * screenHeightPx).toDp() },
                            )
                            .size(
                                width = with(density) { ((region.right - region.left) / 100f * screenWidthPx).toDp() },
                                height = with(density) { ((region.bottom - region.top) / 100f * screenHeightPx).toDp() },
                            )
                            .border(1.dp, Color.LightGray.copy(alpha = 0.1f))
                )
            }
        }
    }
}
