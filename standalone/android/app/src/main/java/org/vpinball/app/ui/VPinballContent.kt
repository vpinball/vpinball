package org.vpinball.app.ui

import android.content.Intent
import android.view.WindowInsets
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.font.FontWeight
import androidx.core.content.FileProvider
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import java.io.File
import org.vpinball.app.CodeLanguage
import org.vpinball.app.Link
import org.vpinball.app.TableManager
import org.vpinball.app.VPinballManager
import org.vpinball.app.VPinballViewModel
import org.vpinball.app.jni.VPinballLogLevel
import org.vpinball.app.ui.screens.common.CodeWebViewDialog
import org.vpinball.app.ui.screens.landing.LandingScreen
import org.vpinball.app.ui.screens.landing.LandingScreenViewModel
import org.vpinball.app.ui.screens.loading.LoadingScreen
import org.vpinball.app.ui.screens.splash.SplashScreen
import org.vpinball.app.ui.theme.VPinballTheme
import org.vpinball.app.ui.theme.VpxRed
import org.vpinball.app.ui.util.koinActivityViewModel
import org.vpinball.app.util.getActivity

@Composable
fun VPinballContent(isAppInit: Boolean = false, viewModel: VPinballViewModel = koinActivityViewModel()) {
    val context = LocalContext.current
    val activity = context.getActivity()
    val state by viewModel.state.collectAsStateWithLifecycle()
    var codeFile by remember { mutableStateOf<File?>(null) }
    var showSplash by remember { mutableStateOf(false) }

    val progress = remember { mutableStateOf(0) }
    val status = remember { mutableStateOf("") }

    LaunchedEffect(state.progress, state.status) {
        progress.value = state.progress
        status.value = state.status ?: ""
    }

    LaunchedEffect(state.playing, isAppInit) {
        activity?.window?.decorView?.windowInsetsController?.let { controller ->
            if (state.playing) {
                controller.hide(WindowInsets.Type.statusBars() or WindowInsets.Type.navigationBars())
            } else {
                controller.show(WindowInsets.Type.statusBars() or WindowInsets.Type.navigationBars())
                controller.setSystemBarsAppearance(0, android.view.WindowInsetsController.APPEARANCE_LIGHT_STATUS_BARS)

                state.table?.let { table ->
                    kotlinx.coroutines.delay(300)
                    LandingScreenViewModel.triggerScrollToTable(table)
                }
            }
        }
    }

    LaunchedEffect(isAppInit) {
        if (isAppInit && state.splash) {
            kotlinx.coroutines.delay(150)
            showSplash = true
            VPinballManager.startup()
            TableManager.initialize(context.applicationContext)
            viewModel.startSplashTimer()
        }
    }

    VPinballTheme {
        if (!isAppInit || !showSplash) {
            // Wait for SDL initialization and status bar to settle
        } else if (state.splash) {
            SplashScreen()
        } else {
            Box(modifier = Modifier.fillMaxSize()) {
                if (!state.playing && !state.loading) {
                    LandingScreen(
                        webServerURL = viewModel.webServerURL,
                        progress = progress,
                        status = status,
                        onTableImported = { uuid, path -> },
                        onRenameTable = { table, name -> viewModel.launchInViewModelScope { TableManager.getInstance().renameTable(table, name) } },
                        onTableImage = { table -> },
                        onDeleteTable = { table -> VPinballManager.log(VPinballLogLevel.INFO, "Deleted table: ${table.uuid}") },
                        onViewFile = { file -> codeFile = file },
                    )
                }

                state.table?.let { table ->
                    if (state.loading) {
                        LoadingScreen(table, state.progress, state.status)
                    }
                }
            }

            if (state.error != null) {
                AlertDialog(
                    title = { Text(text = "TILT!", style = MaterialTheme.typography.titleMedium) },
                    text = { Text(state.error!!) },
                    onDismissRequest = {},
                    confirmButton = {
                        TextButton(onClick = { viewModel.clearError() }) {
                            Text(
                                text = "OK",
                                color = Color.VpxRed,
                                fontSize = MaterialTheme.typography.titleMedium.fontSize,
                                fontWeight = FontWeight.SemiBold,
                            )
                        }
                    },
                    dismissButton = {
                        TextButton(
                            onClick = {
                                viewModel.clearError()
                                Link.TROUBLESHOOTING.open(context = context)
                            }
                        ) {
                            Text(
                                text = "Learn More",
                                color = Color.VpxRed,
                                fontSize = MaterialTheme.typography.titleMedium.fontSize,
                                fontWeight = FontWeight.SemiBold,
                            )
                        }
                    },
                )
            }

            codeFile?.let {
                CodeWebViewDialog(
                    file = it,
                    canClear = CodeLanguage.fromFile(it) == CodeLanguage.LOG,
                    onDismissRequest = { codeFile = null },
                    onShare = {
                        val fileUri = FileProvider.getUriForFile(context, "${context.packageName}.fileprovider", it)
                        val shareIntent =
                            Intent(Intent.ACTION_SEND).apply {
                                type = "application/octet-stream"
                                putExtra(Intent.EXTRA_STREAM, fileUri)
                                addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION)
                            }
                        context.startActivity(Intent.createChooser(shareIntent, "Share File: $it"))
                    },
                )
            }
        }
    }
}
