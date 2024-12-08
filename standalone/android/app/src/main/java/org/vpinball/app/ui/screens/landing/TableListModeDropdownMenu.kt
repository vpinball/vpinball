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
import androidx.compose.runtime.collectAsState
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import org.vpinball.app.R
import org.vpinball.app.TableListMode
import org.vpinball.app.TableListSortOrder
import org.vpinball.app.data.repository.TestPinTableRepository
import org.vpinball.app.ui.theme.VPinballTheme

@Composable
fun TableListModeDropdownMenu(expanded: Boolean, onDismissRequest: () -> Unit, viewModel: LandingScreenViewModel) {
    val tableListMode = viewModel.tableListMode.collectAsState().value
    val tableListSortOrder = viewModel.tableListSortOrder.collectAsState().value

    DropdownMenu(expanded = expanded, onDismissRequest = onDismissRequest) {
        DropdownMenuItem(
            text = {
                Text(
                    text = "2 Column",
                    style = MaterialTheme.typography.bodyLarge,
                    color = MaterialTheme.colorScheme.onSurface,
                    modifier = Modifier.weight(1f),
                )
            },
            leadingIcon = {
                if (tableListMode == TableListMode.TWO_COLUMN) {
                    Icon(
                        painter = painterResource(id = R.drawable.img_sf_checkmark),
                        contentDescription = "Selected",
                        tint = MaterialTheme.colorScheme.onSurface,
                        modifier = Modifier.size(14.dp),
                    )
                }
            },
            trailingIcon = {
                Icon(
                    painter = painterResource(id = R.drawable.img_sf_rectangle_split_2x1),
                    contentDescription = "2 Column",
                    tint = MaterialTheme.colorScheme.onSurface,
                    modifier = Modifier.size(20.dp),
                )
            },
            onClick = {
                viewModel.setTableListMode(TableListMode.TWO_COLUMN)
                onDismissRequest()
            },
        )

        HorizontalDivider()

        DropdownMenuItem(
            text = {
                Text(
                    text = "3 Column",
                    style = MaterialTheme.typography.bodyLarge,
                    color = MaterialTheme.colorScheme.onSurface,
                    modifier = Modifier.weight(1f),
                )
            },
            leadingIcon = {
                if (tableListMode == TableListMode.THREE_COLUMN) {
                    Icon(
                        painter = painterResource(id = R.drawable.img_sf_checkmark),
                        contentDescription = "Selected",
                        tint = MaterialTheme.colorScheme.onSurface,
                        modifier = Modifier.size(14.dp),
                    )
                }
            },
            trailingIcon = {
                Icon(
                    painter = painterResource(id = R.drawable.img_sf_rectangle_split_3x1),
                    contentDescription = "3 Column",
                    tint = MaterialTheme.colorScheme.onSurface,
                    modifier = Modifier.size(20.dp),
                )
            },
            onClick = {
                viewModel.setTableListMode(TableListMode.THREE_COLUMN)
                onDismissRequest()
            },
        )

        HorizontalDivider()

        DropdownMenuItem(
            text = {
                Text(
                    text = "List",
                    style = MaterialTheme.typography.bodyLarge,
                    color = MaterialTheme.colorScheme.onSurface,
                    modifier = Modifier.weight(1f),
                )
            },
            leadingIcon = {
                if (tableListMode == TableListMode.LIST) {
                    Icon(
                        painter = painterResource(id = R.drawable.img_sf_checkmark),
                        contentDescription = "Selected",
                        tint = MaterialTheme.colorScheme.onSurface,
                        modifier = Modifier.size(14.dp),
                    )
                }
            },
            trailingIcon = {
                Icon(
                    painter = painterResource(id = R.drawable.img_sf_list_bullet),
                    contentDescription = "List Mode",
                    tint = MaterialTheme.colorScheme.onSurface,
                    modifier = Modifier.size(20.dp),
                )
            },
            onClick = {
                viewModel.setTableListMode(TableListMode.LIST)
                onDismissRequest()
            },
        )

        HorizontalDivider()

        Spacer(modifier = Modifier.padding(all = 4.dp))

        HorizontalDivider()

        DropdownMenuItem(
            text = {
                Text(
                    text = "A-Z",
                    style = MaterialTheme.typography.bodyLarge,
                    color = MaterialTheme.colorScheme.onSurface,
                    modifier = Modifier.weight(1f),
                )
            },
            leadingIcon = {
                if (tableListSortOrder == TableListSortOrder.A_Z) {
                    Icon(
                        painter = painterResource(id = R.drawable.img_sf_checkmark),
                        contentDescription = "Selected",
                        tint = MaterialTheme.colorScheme.onSurface,
                        modifier = Modifier.size(14.dp),
                    )
                }
            },
            trailingIcon = {
                Icon(
                    painter = painterResource(id = R.drawable.img_sf_arrow_up),
                    contentDescription = "Ascending",
                    tint = MaterialTheme.colorScheme.onSurface,
                    modifier = Modifier.size(20.dp),
                )
            },
            onClick = {
                viewModel.setTableSortOrder(TableListSortOrder.A_Z)
                onDismissRequest()
            },
        )

        HorizontalDivider()

        DropdownMenuItem(
            text = {
                Text(
                    text = "Z-A",
                    style = MaterialTheme.typography.bodyLarge,
                    color = MaterialTheme.colorScheme.onSurface,
                    modifier = Modifier.weight(1f),
                )
            },
            leadingIcon = {
                if (tableListSortOrder == TableListSortOrder.Z_A) {
                    Icon(
                        painter = painterResource(id = R.drawable.img_sf_checkmark),
                        contentDescription = "Selected",
                        tint = MaterialTheme.colorScheme.onSurface,
                        modifier = Modifier.size(14.dp),
                    )
                }
            },
            trailingIcon = {
                Icon(
                    painter = painterResource(id = R.drawable.img_sf_arrow_down),
                    contentDescription = "Descending",
                    tint = MaterialTheme.colorScheme.onSurface,
                    modifier = Modifier.size(20.dp),
                )
            },
            onClick = {
                viewModel.setTableSortOrder(TableListSortOrder.Z_A)
                onDismissRequest()
            },
        )
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Preview
@Composable
private fun TableListModeDropdownMenuPreview() {
    val viewModel = LandingScreenViewModel(TestPinTableRepository)

    VPinballTheme {
        Scaffold(
            topBar = {
                TopAppBar(
                    title = { Text("VPinball") },
                    actions = { TableListModeDropdownMenu(expanded = true, onDismissRequest = {}, viewModel = viewModel) },
                )
            }
        ) { padding ->
            Column(modifier = Modifier.padding(padding)) { Text("test...") }
        }
    }
}
