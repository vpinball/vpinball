package org.vpinball.app.ui

import android.content.Intent
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
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
import org.vpinball.app.VPinballViewModel
import org.vpinball.app.ui.screens.common.CodeWebViewDialog
import org.vpinball.app.ui.screens.landing.LandingScreen
import org.vpinball.app.ui.screens.loading.LoadingScreen
import org.vpinball.app.ui.screens.splash.SplashScreen
import org.vpinball.app.ui.theme.VPinballTheme
import org.vpinball.app.ui.theme.VpxRed
import org.vpinball.app.ui.util.koinActivityViewModel

@Composable
fun VPinballContent(viewModel: VPinballViewModel = koinActivityViewModel()) {
    val context = LocalContext.current
    val state by viewModel.state.collectAsStateWithLifecycle()
    var codeFile by remember { mutableStateOf<File?>(null) }

    VPinballTheme {
        if (state.splash) {
            SplashScreen()
        } else {
            Box(modifier = Modifier.fillMaxSize()) {
                LandingScreen(onViewFile = { file -> codeFile = file })

                if (state.loading) {
                    state.table?.let { table -> LoadingScreen(table, state.progress, state.status) }
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
