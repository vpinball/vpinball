package org.vpinball.app.ui.screens.landing

import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import org.vpinball.app.R
import org.vpinball.app.TableGridSize
import org.vpinball.app.TableListSortOrder
import org.vpinball.app.ui.screens.common.AppDropdownMenu
import org.vpinball.app.ui.screens.common.AppMenuDivider
import org.vpinball.app.ui.screens.common.AppMenuGroupGap
import org.vpinball.app.ui.screens.common.AppMenuHeader
import org.vpinball.app.ui.screens.common.AppMenuItem

@Composable
fun TableListModeDropdownMenu(expanded: Boolean, onDismissRequest: () -> Unit, viewModel: LandingScreenViewModel) {
    val tableGridSize = viewModel.tableGridSize.collectAsState().value
    val tableListSortOrder = viewModel.tableListSortOrder.collectAsState().value

    fun choose(action: () -> Unit) {
        action()
        onDismissRequest()
    }

    AppDropdownMenu(expanded = expanded, onDismissRequest = onDismissRequest) {
        AppMenuHeader("Grid Size")
        AppMenuItem(label = "Small", iconRes = R.drawable.img_sf_square_grid_4x3_fill, selected = tableGridSize == TableGridSize.SMALL) {
            choose { viewModel.setTableGridSize(TableGridSize.SMALL) }
        }
        AppMenuDivider()
        AppMenuItem(label = "Medium", iconRes = R.drawable.img_sf_square_grid_3x3_fill, selected = tableGridSize == TableGridSize.MEDIUM) {
            choose { viewModel.setTableGridSize(TableGridSize.MEDIUM) }
        }
        AppMenuDivider()
        AppMenuItem(label = "Large", iconRes = R.drawable.img_sf_square_grid_2x2_fill, selected = tableGridSize == TableGridSize.LARGE) {
            choose { viewModel.setTableGridSize(TableGridSize.LARGE) }
        }

        AppMenuGroupGap()

        AppMenuHeader("Sort By")
        AppMenuItem(label = "Name A-Z", selected = tableListSortOrder == TableListSortOrder.A_Z) {
            choose { viewModel.setTableSortOrder(TableListSortOrder.A_Z) }
        }
        AppMenuDivider()
        AppMenuItem(label = "Name Z-A", selected = tableListSortOrder == TableListSortOrder.Z_A) {
            choose { viewModel.setTableSortOrder(TableListSortOrder.Z_A) }
        }
    }
}
