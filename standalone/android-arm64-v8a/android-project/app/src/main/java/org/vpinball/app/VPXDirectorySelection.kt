package org.vpinball.app

import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Button
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontStyle
import androidx.compose.ui.unit.dp
import androidx.navigation.NavHostController

@Composable
fun VPXDirSelection(controller: NavHostController) {
    val activity = LocalContext.current as VpxLauncherActivity

    Column(modifier = Modifier.padding(horizontal = 14.dp)) {

        Column(modifier = Modifier.weight(1f).fillMaxWidth(),
            horizontalAlignment = Alignment.CenterHorizontally) {
            Spacer(modifier = Modifier.height(48.dp))
            Text(stringResource(R.string.select_dir_msg))
            Box(contentAlignment = Alignment.Center, modifier = Modifier.fillMaxSize()) {
                Button(
                    onClick = {
                        // Try to open the VPX directory
                        activity.openVpxDirectory(true)
                    }
                ) {
                    Text(stringResource(R.string.button_select_dir))
                }
            }
        }

        Text(text = stringResource(R.string.permissions_msg),
            fontStyle = FontStyle.Italic,
            modifier = Modifier.padding(vertical = 24.dp))
    }
}