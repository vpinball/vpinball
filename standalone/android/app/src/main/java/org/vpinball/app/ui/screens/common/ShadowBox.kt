package org.vpinball.app.ui.screens.common

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.BoxScope
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.dp

@Composable
fun ShadowBox(backgroundVisible: Boolean, horizontalPadding: Dp = 16.dp, content: @Composable BoxScope.() -> Unit) {
    Box(
        modifier =
            Modifier.fillMaxSize()
                .clip(RoundedCornerShape(8.dp))
                .background(Color.Black.copy(alpha = if (backgroundVisible) 0.5f else 0f))
                .padding(vertical = 8.dp, horizontal = horizontalPadding)
    ) {
        content()
    }
}
