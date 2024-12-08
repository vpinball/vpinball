package org.vpinball.app.ui.screens.landing

import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.MutableState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalDensity
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.unit.DpOffset
import androidx.compose.ui.unit.dp
import org.vpinball.app.R
import org.vpinball.app.data.entity.PinTable
import org.vpinball.app.ui.theme.VpxRed
import org.vpinball.app.util.hasIni
import org.vpinball.app.util.hasScript

@Composable
fun TableListItemDropdownMenu(
    table: PinTable,
    expanded: MutableState<Boolean>,
    onRename: () -> Unit,
    onChangeArtwork: () -> Unit,
    onViewScript: () -> Unit,
    onShare: () -> Unit,
    onReset: () -> Unit,
    onDelete: () -> Unit,
    offsetProvider: () -> Offset,
) {
    val density = LocalDensity.current
    val offset = with(density) { DpOffset(x = offsetProvider().x.toDp(), y = offsetProvider().y.toDp()) }

    var scriptExists by remember { mutableStateOf(false) }
    var iniExists by remember { mutableStateOf(false) }

    DropdownMenu(expanded = expanded.value, onDismissRequest = { expanded.value = false }, offset = offset) {
        LaunchedEffect(expanded.value) {
            if (expanded.value) {
                scriptExists = table.hasScript()
                iniExists = table.hasIni()
            }
        }

        Text(
            text = table.name,
            style = MaterialTheme.typography.labelSmall,
            color = MaterialTheme.colorScheme.onSurfaceVariant,
            modifier = Modifier.padding(vertical = 4.dp).padding(horizontal = 12.dp),
        )

        HorizontalDivider()

        DropdownMenuItem(
            text = { Text("Rename", style = MaterialTheme.typography.bodyLarge) },
            trailingIcon = {
                Icon(painter = painterResource(id = R.drawable.img_sf_pencil), contentDescription = null, modifier = Modifier.size(20.dp))
            },
            onClick = {
                expanded.value = false
                onRename()
            },
        )

        HorizontalDivider()

        DropdownMenuItem(
            text = { Text("Change Artwork", style = MaterialTheme.typography.bodyLarge) },
            trailingIcon = {
                Icon(painter = painterResource(id = R.drawable.img_sf_photo), contentDescription = null, modifier = Modifier.size(20.dp))
            },
            onClick = {
                expanded.value = false
                onChangeArtwork()
            },
        )

        HorizontalDivider()

        DropdownMenuItem(
            text = { Text(text = if (scriptExists) "View Script" else "Extract Script", style = MaterialTheme.typography.bodyLarge) },
            trailingIcon = {
                Icon(painter = painterResource(id = R.drawable.img_sf_applescript), contentDescription = null, modifier = Modifier.size(20.dp))
            },
            onClick = {
                expanded.value = false
                onViewScript()
            },
        )

        HorizontalDivider()

        DropdownMenuItem(
            text = { Text("Share", style = MaterialTheme.typography.bodyLarge) },
            trailingIcon = {
                Icon(
                    painter = painterResource(id = R.drawable.img_sf_square_and_arrow_up),
                    contentDescription = null,
                    modifier = Modifier.size(20.dp),
                )
            },
            onClick = {
                expanded.value = false
                onShare()
            },
        )

        HorizontalDivider()

        DropdownMenuItem(
            text = { Text(text = "Reset", style = MaterialTheme.typography.bodyLarge, color = if (iniExists) Color.VpxRed else Color.Gray) },
            trailingIcon = {
                Icon(
                    painter = painterResource(id = R.drawable.img_sf_arrow_trianglehead_counterclockwise),
                    contentDescription = null,
                    modifier = Modifier.size(20.dp),
                    tint = if (iniExists) Color.VpxRed else Color.Gray,
                )
            },
            onClick = {
                expanded.value = false
                onReset()
            },
            enabled = iniExists,
        )

        HorizontalDivider()

        DropdownMenuItem(
            text = { Text("Delete", style = MaterialTheme.typography.bodyLarge, color = Color.VpxRed) },
            trailingIcon = {
                Icon(
                    painter = painterResource(id = R.drawable.img_sf_trash),
                    contentDescription = null,
                    modifier = Modifier.size(20.dp),
                    tint = Color.VpxRed,
                )
            },
            onClick = {
                expanded.value = false
                onDelete()
            },
        )
    }
}
