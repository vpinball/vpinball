package org.vpinball.app.ui.screens.landing

import androidx.compose.material3.AlertDialog
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import org.vpinball.app.ui.screens.common.AlertButton

@Composable
fun ImportConfirmDialog(filename: String?, onConfirm: () -> Unit, onDismiss: () -> Unit) {
    AlertDialog(
        title = { Text(text = "Confirm Import Table", style = MaterialTheme.typography.titleMedium) },
        text = { Text("Import \"${filename}\"?") },
        onDismissRequest = {},
        confirmButton = { AlertButton(text = "OK", onClick = onConfirm) },
        dismissButton = { AlertButton(text = "Cancel", onClick = onDismiss) },
    )
}
