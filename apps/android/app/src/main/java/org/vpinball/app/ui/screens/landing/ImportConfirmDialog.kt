package org.vpinball.app.ui.screens.landing

import androidx.compose.material3.AlertDialog
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import org.vpinball.app.ui.theme.VpxRed

@Composable
fun ImportConfirmDialog(filename: String?, onConfirm: () -> Unit, onDismiss: () -> Unit) {
    AlertDialog(
        title = { Text(text = "Confirm Import Table", style = MaterialTheme.typography.titleMedium) },
        text = { Text("Import \"${filename}\"?") },
        onDismissRequest = {},
        confirmButton = {
            TextButton(onClick = onConfirm) {
                Text(text = "OK", color = Color.VpxRed, fontSize = MaterialTheme.typography.titleMedium.fontSize, fontWeight = FontWeight.SemiBold)
            }
        },
        dismissButton = {
            TextButton(onClick = onDismiss) {
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
