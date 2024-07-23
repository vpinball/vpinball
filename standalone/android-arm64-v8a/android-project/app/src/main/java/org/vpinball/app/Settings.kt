package org.vpinball.app

import android.content.Intent
import android.net.Uri
import android.os.Build
import android.os.Environment
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.ArrowBack
import androidx.compose.material.icons.outlined.Edit
import androidx.compose.material.icons.outlined.Home
import androidx.compose.material.icons.outlined.Share
import androidx.compose.material3.Button
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.material3.TopAppBar
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.stringResource
import androidx.core.content.FileProvider
import androidx.core.net.toUri
import androidx.datastore.preferences.core.booleanPreferencesKey
import androidx.datastore.preferences.core.edit
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import androidx.lifecycle.lifecycleScope
import androidx.navigation.NavHostController
import com.alorma.compose.settings.ui.SettingsMenuLink
import com.alorma.compose.settings.ui.SettingsSwitch
import kotlinx.coroutines.flow.map
import kotlinx.coroutines.launch
import java.io.File
import java.net.Inet4Address
import java.net.NetworkInterface

// From https://stackoverflow.com/questions/6064510/how-to-get-ip-address-of-the-device-from-code
fun getIpv4HostAddress(): String {
    NetworkInterface.getNetworkInterfaces()?.toList()?.map { networkInterface ->
        networkInterface.inetAddresses?.toList()?.find {
            !it.isLoopbackAddress && it is Inet4Address
        }?.let { return it.hostAddress }
    }
    return ""
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun Settings(controller: NavHostController) {
    val activity = LocalContext.current as VpxLauncherActivity
//    var webServerState = activity.dataStore.data
//        .map { it[booleanPreferencesKey("webserver")] ?: false }
//        .collectAsStateWithLifecycle(true)

    var webServerState = activity.model.webServerState.collectAsStateWithLifecycle(false)

    Scaffold(
        topBar = { TopAppBar(
            title = { Text(stringResource(R.string.settings))},
            navigationIcon = {
                IconButton(onClick = { controller.navigateUp() }) {
                    Icon(
                        imageVector = Icons.AutoMirrored.Filled.ArrowBack,
                        contentDescription = "Back"
                    )
                }
            }
        ) },
        content = { paddingValues ->
            Column(modifier = Modifier.fillMaxSize().padding(paddingValues)) {

                // Web Server toggle
                SettingsSwitch(
                    enabled = false, // Disabled until the c++ web server can be made to run standalone
                    state = webServerState.value,
                    onCheckedChange = { state ->
                        activity.lifecycleScope.launch {
                            activity.dataStore.edit {
                                it[booleanPreferencesKey("webserver")] = state
                            }
                        }
                    },
                    title = { Text(text = "Web Server") },
                    subtitle = { Text(text = if (webServerState.value) "${getIpv4HostAddress()}:2112" else "Off") },
                    icon = { Icon(imageVector = Icons.Outlined.Home, contentDescription = null) },
                )

                // Button to send a copy of the VPX log to another Android app
                SettingsMenuLink(
                    title = { Text(stringResource(R.string.share_logs)) },
                    icon = { Icon(imageVector = Icons.Outlined.Share, contentDescription = null) },
                    subtitle = { Text(stringResource(R.string.share_logs_subtext)) },
                    onClick = { },
                    action = {
                        Button(
                            onClick = {
                                activity.copyVPXLog()?.let {
                                    val logUri =
                                        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
                                            FileProvider.getUriForFile(
                                                activity,
                                                "org.vpinball",
                                                it
                                            )
                                        } else {
                                            Uri.fromFile(it)
                                        }

                                    val sendIntent = Intent().apply {
                                        action = Intent.ACTION_SEND
                                        putExtra(Intent.EXTRA_STREAM, logUri)
                                        type = "text/plain"
                                        flags = Intent.FLAG_GRANT_READ_URI_PERMISSION
                                    }
                                    val shareIntent = Intent.createChooser(sendIntent, "VPX Logs")
                                    activity.startActivity(shareIntent)
                                }
                            },
                            enabled = true,
                            content = { Text(stringResource(R.string.logs_button)) }
                        )
                    }
                )

                // Option for changing the VPX directory
                SettingsMenuLink(
                    title = { Text(stringResource(R.string.new_dir_selection)) },
                    icon = { Icon(imageVector = Icons.Outlined.Edit, contentDescription = null) },
                    subtitle = { Text(stringResource(R.string.new_dir_explanation)) },
                    onClick = { },
                    action = {
                        Button(
                            onClick = {
                                val suggestedVpxDir =
                                    File(Environment.getExternalStorageDirectory(), "vpx").toUri()
                                activity.askUserForVpxDirectory(suggestedVpxDir)
                            },
                            enabled = true,
                            content = { Text(stringResource(R.string.button_new_dir)) }
                        )
                    }
                )

            }
        }
    )
}
