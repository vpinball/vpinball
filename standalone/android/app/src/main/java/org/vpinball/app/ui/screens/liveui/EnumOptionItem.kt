package org.vpinball.app.ui.screens.liveui

import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.unit.DpOffset
import androidx.compose.ui.unit.dp
import org.vpinball.app.R
import org.vpinball.app.jni.VPinballDisplayText

@Composable
fun <T> EnumOptionItem(label: String, options: List<T>, option: T, onOptionChanged: (T) -> Unit, visible: Boolean) where T : VPinballDisplayText {
    var expanded by remember { mutableStateOf(false) }

    Row(modifier = Modifier.padding(vertical = 8.dp).alpha(if (visible) 1.0f else 0.0f), verticalAlignment = Alignment.CenterVertically) {
        Text(text = label, style = MaterialTheme.typography.bodyLarge, color = Color.White, modifier = Modifier.weight(1f))

        Box {
            Row(verticalAlignment = Alignment.CenterVertically, modifier = Modifier.clickable { expanded = true }) {
                Text(text = option.text, style = MaterialTheme.typography.bodyLarge, color = Color.White)

                Icon(
                    painter = painterResource(id = R.drawable.img_sf_chevron_up_chevron_down),
                    contentDescription = null,
                    tint = Color.White,
                    modifier = Modifier.size(14.dp).padding(start = 4.dp),
                )
            }

            DropdownMenu(expanded = expanded, onDismissRequest = { expanded = false }, offset = DpOffset(x = 0.dp, y = 0.dp)) {
                options.forEach { option ->
                    DropdownMenuItem(
                        text = {
                            Text(text = option.text, style = MaterialTheme.typography.bodyLarge, color = MaterialTheme.colorScheme.onSurfaceVariant)
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
