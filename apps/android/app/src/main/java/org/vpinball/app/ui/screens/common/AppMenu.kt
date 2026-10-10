package org.vpinball.app.ui.screens.common

import androidx.compose.foundation.layout.ColumnScope
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.DpOffset
import androidx.compose.ui.unit.dp

private val MenuWidth = 260.dp
private val MenuShape = RoundedCornerShape(22.dp)

@Composable
fun AppDropdownMenu(
    expanded: Boolean,
    onDismissRequest: () -> Unit,
    modifier: Modifier = Modifier,
    offset: DpOffset = DpOffset(0.dp, 0.dp),
    content: @Composable ColumnScope.() -> Unit,
) {
    DropdownMenu(
        expanded = expanded,
        onDismissRequest = onDismissRequest,
        modifier = modifier.width(MenuWidth),
        offset = offset,
        shape = MenuShape,
        containerColor = MaterialTheme.colorScheme.surfaceContainer,
        tonalElevation = 0.dp,
        shadowElevation = 6.dp,
        content = content,
    )
}

@Composable
fun AppMenuHeader(text: String) {
    Text(
        text = text,
        style = MaterialTheme.typography.labelMedium,
        color = MaterialTheme.colorScheme.onSurfaceVariant,
        modifier = Modifier.padding(horizontal = 16.dp, vertical = 8.dp),
    )
}

@Composable
fun AppMenuDivider() {
    HorizontalDivider(color = MaterialTheme.colorScheme.onSurface.copy(alpha = 0.1f), modifier = Modifier.padding(start = 16.dp))
}

@Composable
fun AppMenuGroupGap() {
    HorizontalDivider(thickness = 6.dp, color = MaterialTheme.colorScheme.surfaceContainerHigh)
}

@Composable
fun AppMenuItem(
    label: String,
    iconRes: Int? = null,
    enabled: Boolean = true,
    selected: Boolean = false,
    onClick: () -> Unit,
) {
    val tint = if (enabled) MaterialTheme.colorScheme.onSurface else MaterialTheme.colorScheme.onSurface.copy(alpha = 0.38f)

    DropdownMenuItem(
        text = { Text(text = label, style = MaterialTheme.typography.bodyLarge, fontWeight = FontWeight.Normal, color = tint) },
        leadingIcon =
            iconRes?.let { res ->
                { Icon(painter = painterResource(id = res), contentDescription = null, tint = tint, modifier = Modifier.size(20.dp)) }
            },
        trailingIcon =
            if (selected) {
                {
                    Icon(
                        painter = painterResource(id = org.vpinball.app.R.drawable.img_sf_checkmark),
                        contentDescription = "Selected",
                        tint = tint,
                        modifier = Modifier.size(14.dp),
                    )
                }
            } else {
                null
            },
        enabled = enabled,
        onClick = onClick,
    )
}
