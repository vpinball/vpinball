package org.vpinball.app.ui.screens.landing

import androidx.compose.runtime.Composable
import org.vpinball.app.R
import org.vpinball.app.ui.screens.common.AppDropdownMenu
import org.vpinball.app.ui.screens.common.AppMenuDivider
import org.vpinball.app.ui.screens.common.AppMenuGroupGap
import org.vpinball.app.ui.screens.common.AppMenuHeader
import org.vpinball.app.ui.screens.common.AppMenuItem

@Composable
fun ImportTableDropdownMenu(
    expanded: Boolean,
    onDismissRequest: () -> Unit,
    onFiles: () -> Unit,
    onBlankTable: () -> Unit,
    onExampleTable: () -> Unit,
) {
    AppDropdownMenu(expanded = expanded, onDismissRequest = onDismissRequest) {
        AppMenuHeader("Import from...")
        AppMenuItem(label = "Files", iconRes = R.drawable.img_sf_doc, onClick = onFiles)

        AppMenuGroupGap()

        AppMenuHeader("Built in...")
        AppMenuItem(label = "blankTable.vpx", iconRes = R.drawable.img_sf_doc_text, onClick = onBlankTable)
        AppMenuDivider()
        AppMenuItem(label = "exampleTable.vpx", iconRes = R.drawable.img_sf_doc_text, onClick = onExampleTable)
    }
}
