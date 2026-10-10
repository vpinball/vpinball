package org.vpinball.app.ui.screens.landing

import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.MutableState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.platform.LocalDensity
import androidx.compose.ui.unit.DpOffset
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import org.vpinball.app.R
import org.vpinball.app.Table
import org.vpinball.app.ui.screens.common.AppDropdownMenu
import org.vpinball.app.ui.screens.common.AppMenuDivider
import org.vpinball.app.ui.screens.common.AppMenuGroupGap
import org.vpinball.app.ui.screens.common.AppMenuHeader
import org.vpinball.app.ui.screens.common.AppMenuItem
import org.vpinball.app.util.hasIniFile
import org.vpinball.app.util.hasScriptFile

@Composable
fun TableContextMenu(
    table: Table,
    expanded: MutableState<Boolean>,
    onToggleFavorite: () -> Unit,
    onRemoveFromRecent: (() -> Unit)?,
    onRename: () -> Unit,
    onSetImage: () -> Unit,
    onViewScript: () -> Unit,
    onShare: () -> Unit,
    onResetImage: () -> Unit,
    onResetSettings: () -> Unit,
    onDelete: () -> Unit,
    offsetProvider: () -> Offset,
) {
    val density = LocalDensity.current
    val offset = with(density) { DpOffset(x = offsetProvider().x.toDp(), y = offsetProvider().y.toDp()) }

    var scriptExists by remember { mutableStateOf(false) }
    var iniExists by remember { mutableStateOf(false) }

    fun choose(action: () -> Unit) {
        expanded.value = false
        action()
    }

    AppDropdownMenu(expanded = expanded.value, onDismissRequest = { expanded.value = false }, offset = offset) {
        LaunchedEffect(expanded.value) {
            if (expanded.value) {
                withContext(Dispatchers.IO) {
                    scriptExists = table.hasScriptFile()
                    iniExists = table.hasIniFile()
                }
            }
        }

        AppMenuHeader(table.name)

        AppMenuItem(
            label = if (table.isFavorite) "Remove from Favorites" else "Add to Favorites",
            iconRes = if (table.isFavorite) R.drawable.img_sf_heart_slash else R.drawable.img_sf_heart,
        ) {
            choose(onToggleFavorite)
        }
        if (onRemoveFromRecent != null) {
            AppMenuDivider()
            AppMenuItem(label = "Remove from Recently Played", iconRes = R.drawable.img_sf_clock_badge_xmark) { choose(onRemoveFromRecent) }
        }

        AppMenuGroupGap()

        AppMenuItem(label = "Rename", iconRes = R.drawable.img_sf_pencil) { choose(onRename) }
        AppMenuDivider()
        AppMenuItem(label = "Set Image", iconRes = R.drawable.img_sf_photo_on_rectangle_angled) { choose(onSetImage) }

        AppMenuGroupGap()

        AppMenuItem(label = if (scriptExists) "View Script" else "Extract Script", iconRes = R.drawable.img_sf_applescript) { choose(onViewScript) }
        AppMenuDivider()
        AppMenuItem(label = "Share", iconRes = R.drawable.img_sf_square_and_arrow_up) { choose(onShare) }

        AppMenuGroupGap()

        AppMenuItem(label = "Reset Image", iconRes = R.drawable.img_sf_photo, enabled = table.image.isNotEmpty()) { choose(onResetImage) }
        AppMenuDivider()
        AppMenuItem(label = "Reset Settings", iconRes = R.drawable.img_sf_slider_horizontal_3, enabled = iniExists) { choose(onResetSettings) }

        AppMenuGroupGap()

        AppMenuItem(label = "Delete", iconRes = R.drawable.img_sf_trash) { choose(onDelete) }
    }
}
