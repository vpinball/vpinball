package org.vpinball.app

import android.content.Intent
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.ElevatedButton
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.material3.TopAppBar
import androidx.compose.runtime.Composable
import androidx.compose.runtime.remember
import androidx.compose.runtime.snapshots.SnapshotStateList
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import androidx.navigation.NavHostController

@Composable
fun Tables(controller: NavHostController, model: VPXViewModel) {
    PinballTables(model.tablesList, controller)
}


// Rudimentary presentation of all tables on a list.
// It works but it's ugly. Hopefully to be replaced with a nice looking UI.
@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun PinballTables(tablesList: SnapshotStateList<String>, controller: NavHostController) {
    val tables = remember { tablesList }
    val activity = LocalContext.current as VpxLauncherActivity

    Scaffold(
        topBar = { TopAppBar(title = {Text(stringResource(R.string.main_title))}) },

        content = { paddingValues ->
            Column(modifier = Modifier.fillMaxSize().padding(paddingValues)) {

                Column(
                    verticalArrangement = Arrangement.spacedBy(8.dp),
                    modifier = Modifier
                        .verticalScroll(rememberScrollState())
                        .weight(1f)
                        .fillMaxWidth()
                        .padding(horizontal = 8.dp)
                ) {
                    tables.forEach { table ->
                        Text(text = table,
                            modifier = Modifier.fillMaxWidth().clickable {
                                val intent = Intent(activity, VPinballActivity::class.java).apply {
                                    putExtra("table", "$table.vpx")
                                    putExtra("dirFd", activity.vpxDirFd!!.detachFd())
                                }
                                activity.startActivity(intent)
                            })

                        HorizontalDivider()
                    }
                }

                Spacer(modifier = Modifier.height(16.dp))

                Row(
                    horizontalArrangement = Arrangement.SpaceEvenly,
                    modifier = Modifier
                        .padding(8.dp)
                        .fillMaxWidth()
                ) {
                    ElevatedButton(onClick = { activity.openVpxDirectory(false) }) {
                        Text(stringResource(R.string.button_refresh))
                    }

                    ElevatedButton(onClick = {
                        controller.navigate(NavigationItem.Settings.route)
                    }) {
                        Text(stringResource(R.string.settings))
                    }
                }

            }
        }
    )
}
