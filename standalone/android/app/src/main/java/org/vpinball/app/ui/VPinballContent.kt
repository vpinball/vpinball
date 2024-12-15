package org.vpinball.app.ui

import android.content.Intent
import android.view.WindowInsets
import androidx.compose.animation.AnimatedVisibility
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
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.font.FontWeight
import androidx.core.content.FileProvider
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import java.io.File
import kotlinx.coroutines.flow.last
import kotlinx.coroutines.launch
import org.vpinball.app.CodeLanguage
import org.vpinball.app.Link
import org.vpinball.app.VPinballManager
import org.vpinball.app.VPinballViewModel
import org.vpinball.app.ui.screens.common.CodeWebViewDialog
import org.vpinball.app.ui.screens.landing.LandingScreen
import org.vpinball.app.ui.screens.liveui.LiveUIOverlay
import org.vpinball.app.ui.screens.loading.LoadingScreen
import org.vpinball.app.ui.screens.splash.SplashScreen
import org.vpinball.app.ui.screens.touch.TouchInstructionsScreen
import org.vpinball.app.ui.screens.touch.TouchOverlayScreen
import org.vpinball.app.ui.theme.VPinballTheme
import org.vpinball.app.ui.theme.VpxRed
import org.vpinball.app.ui.util.koinActivityViewModel
import org.vpinball.app.util.deleteFiles
import org.vpinball.app.util.getActivity

@Composable
fun VPinballContent(viewModel: VPinballViewModel = koinActivityViewModel()) {
    val context = LocalContext.current
    val activity = context.getActivity()
    val state by viewModel.state.collectAsStateWithLifecycle()
    var codeFile by remember { mutableStateOf<File?>(null) }
    val scope = rememberCoroutineScope()

    LaunchedEffect(state.playing, state.table) {
        if (state.playing) {
            activity?.window?.decorView?.windowInsetsController?.hide(WindowInsets.Type.statusBars() or WindowInsets.Type.navigationBars())
        } else {
            activity?.window?.decorView?.windowInsetsController?.show(WindowInsets.Type.statusBars() or WindowInsets.Type.navigationBars())
        }
    }

    VPinballTheme {
        if (state.splash) {
            SplashScreen()
        } else {
            AnimatedVisibility(visible = !(state.playing || state.loading), modifier = Modifier.fillMaxSize()) {
                LandingScreen(
                    webServerURL = viewModel.webServerURL,
                    progress = viewModel.progress,
                    status = viewModel.status,
                    onTableImported = { uuid, path -> scope.launch { viewModel.saveImportTable(uuid, path).last() } },
                    onRenameTable = { table, name -> scope.launch { viewModel.renameTable(table, name).last() } },
                    onChangeTableArtwork = { table -> scope.launch { viewModel.markTableAsModified(table).last() } },
                    onDeleteTable = { table ->
                        table.deleteFiles()
                        scope.launch { viewModel.deleteTable(table).last() }
                    },
                    onViewFile = { file -> codeFile = file },
                )
            }

            if (state.playing) {
                if (state.touchInstructions) {
                    TouchInstructionsScreen()
                } else {
                    if (state.touchOverlay) {
                        TouchOverlayScreen()
                    }

                    AnimatedVisibility(visible = state.liveUI, modifier = Modifier.fillMaxSize()) {
                        LiveUIOverlay(
                            onResume = {
                                viewModel.toggleLiveUI()
                                VPinballManager.setPlayState(true)
                            }
                        )
                    }
                }
            }

            if (state.loading) {
                LoadingScreen(state.table!!, state.progress, state.status)
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
