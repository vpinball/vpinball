package org.vpinball.app.ui.screens.common

import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import org.vpinball.app.ui.theme.VpxRed

@Composable
fun AlertButton(text: String, enabled: Boolean = true, destructive: Boolean = false, onClick: () -> Unit) {
    TextButton(
        onClick = onClick,
        enabled = enabled,
        colors = ButtonDefaults.textButtonColors(contentColor = if (destructive) Color.VpxRed else MaterialTheme.colorScheme.onSurface),
    ) {
        Text(text = text, fontSize = MaterialTheme.typography.titleMedium.fontSize, fontWeight = FontWeight.SemiBold)
    }
}
