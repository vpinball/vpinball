package org.vpinball.app.ui.screens.common

import android.content.Context
import android.view.WindowInsets
import android.view.WindowManager
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.requiredHeight
import androidx.compose.foundation.layout.requiredWidth
import androidx.compose.runtime.Composable
import androidx.compose.runtime.remember
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.platform.LocalDensity
import androidx.compose.ui.window.Dialog
import androidx.compose.ui.window.DialogProperties

private val fullScreenDialogProperties =
    DialogProperties(dismissOnBackPress = true, dismissOnClickOutside = false, usePlatformDefaultWidth = false, decorFitsSystemWindows = false)

@Composable
fun DialogFullScreen(onDismissRequest: () -> Unit, properties: DialogProperties = fullScreenDialogProperties, content: @Composable () -> Unit) {
    Dialog(onDismissRequest = onDismissRequest, properties = properties) {
        val context = LocalContext.current
        val windowManager = remember { context.getSystemService(Context.WINDOW_SERVICE) as WindowManager }
        val metrics = windowManager.currentWindowMetrics
        val width =
            with(LocalDensity.current) {
                val insets = metrics.windowInsets.getInsets(WindowInsets.Type.systemBars() or WindowInsets.Type.navigationBars())
                (metrics.bounds.width() - insets.left - insets.right).toDp()
            }
        val height =
            with(LocalDensity.current) {
                val insets = metrics.windowInsets.getInsets(WindowInsets.Type.systemBars() or WindowInsets.Type.navigationBars())
                (metrics.bounds.height() - insets.top - insets.bottom).toDp()
            }
        Box(modifier = Modifier.requiredWidth(width).requiredHeight(height)) { content() }
    }
}
