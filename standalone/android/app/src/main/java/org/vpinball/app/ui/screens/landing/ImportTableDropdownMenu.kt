package org.vpinball.app.ui.screens.landing

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.material3.TopAppBar
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import org.vpinball.app.R
import org.vpinball.app.ui.theme.VPinballTheme

@Composable
fun ImportTableDropdownMenu(expanded: Boolean, onDismissRequest: () -> Unit, onFiles: () -> Unit, onExampleTable: () -> Unit) {
    DropdownMenu(expanded = expanded, onDismissRequest = onDismissRequest) {
        Text(
            text = "Import from...",
            style = MaterialTheme.typography.labelSmall,
            color = MaterialTheme.colorScheme.onSurfaceVariant,
            modifier = Modifier.padding(vertical = 4.dp).padding(horizontal = 12.dp),
        )

        HorizontalDivider()

        DropdownMenuItem(
            text = {
                Text(
                    text = "Files",
                    style = MaterialTheme.typography.bodyLarge,
                    color = MaterialTheme.colorScheme.onSurface,
                    modifier = Modifier.weight(1f),
                )
            },
            trailingIcon = {
                Icon(
                    painter = painterResource(id = R.drawable.img_sf_document),
                    contentDescription = "File Icon",
                    tint = MaterialTheme.colorScheme.onSurface,
                    modifier = Modifier.size(20.dp),
                )
            },
            onClick = onFiles,
        )

        HorizontalDivider()

        Spacer(modifier = Modifier.padding(all = 4.dp))

        HorizontalDivider()

        Text(
            text = "Built in...",
            style = MaterialTheme.typography.labelSmall,
            color = MaterialTheme.colorScheme.onSurfaceVariant,
            modifier = Modifier.padding(vertical = 4.dp).padding(horizontal = 12.dp),
        )

        HorizontalDivider()

        DropdownMenuItem(
            text = { Text(text = "exampleTable.vpx", style = MaterialTheme.typography.bodyLarge, color = MaterialTheme.colorScheme.onSurface) },
            onClick = onExampleTable,
        )
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Preview
@Composable
private fun ImportTableDropdownMenuPreview() {
    VPinballTheme {
        Scaffold(
            topBar = {
                TopAppBar(
                    title = { Text("VPinBall") },
                    actions = { ImportTableDropdownMenu(expanded = true, onDismissRequest = {}, onExampleTable = {}, onFiles = {}) },
                )
            }
        ) { padding ->
            Column(modifier = Modifier.padding(padding)) { Text("test...") }
        }
    }
}
