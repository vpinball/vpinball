package org.vpinball.app.ui

import android.content.Intent
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.core.content.FileProvider
import java.io.File
import org.vpinball.app.CodeLanguage
import org.vpinball.app.VPinballModel
import org.vpinball.app.ui.screens.common.CodeWebViewDialog
import org.vpinball.app.ui.screens.landing.LandingScreen
import org.vpinball.app.ui.screens.loading.LoadingScreen
import org.vpinball.app.ui.screens.splash.SplashScreen
import org.vpinball.app.ui.theme.VPinballTheme
import org.vpinball.app.ui.util.koinActivityViewModel

@Composable
fun VPinballContent(viewModel: VPinballModel = koinActivityViewModel()) {
    val context = LocalContext.current
    var codeFile by remember { mutableStateOf<File?>(null) }

    VPinballTheme {
        if (viewModel.showSplash) {
            SplashScreen()
        } else {
            Box(modifier = Modifier.fillMaxSize()) {
                LandingScreen(onViewFile = { file -> codeFile = file })

                if (viewModel.showHUD) {
                    viewModel.activeTable?.let { table -> LoadingScreen(table, viewModel.hudProgress, viewModel.hudStatus) }
                }
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
