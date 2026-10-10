package org.vpinball.app.ui.screens.common

import android.graphics.Color
import android.view.ViewGroup
import android.webkit.WebChromeClient
import android.webkit.WebView
import android.webkit.WebViewClient
import androidx.compose.animation.core.animateFloatAsState
import androidx.compose.animation.core.tween
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.WindowInsets
import androidx.compose.foundation.layout.asPaddingValues
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.navigationBars
import androidx.compose.foundation.layout.navigationBarsPadding
import androidx.compose.foundation.layout.offset
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.statusBars
import androidx.compose.foundation.layout.statusBarsPadding
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.DisposableEffect
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableFloatStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalConfiguration
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.platform.LocalDensity
import androidx.compose.ui.platform.LocalView
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.IntOffset
import androidx.compose.ui.unit.dp
import androidx.compose.ui.viewinterop.AndroidView
import androidx.compose.ui.window.Dialog
import androidx.compose.ui.window.DialogProperties
import androidx.compose.ui.window.DialogWindowProvider
import androidx.core.view.WindowCompat
import java.io.File
import java.io.FileOutputStream
import kotlin.math.roundToInt
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import org.vpinball.app.CodeLanguage
import org.vpinball.app.R
import org.vpinball.app.ui.theme.VPinballTheme
import org.vpinball.app.ui.theme.isDarkAppearance

private val CodeBarHeight = 76.dp
private val EditorDark = androidx.compose.ui.graphics.Color(0xFF1E1E1E)
private val EditorLight = androidx.compose.ui.graphics.Color(0xFFFFFFFE)

@Composable
fun CodeWebViewDialog(file: File, canClear: Boolean, onDismissRequest: () -> Unit, onShare: (file: File) -> Unit, modifier: Modifier = Modifier) {
    var content by remember { mutableStateOf<String?>(null) }

    val context = LocalContext.current
    val darkMode = isDarkAppearance()
    val coroutineScope = rememberCoroutineScope()

    val configuration = LocalConfiguration.current
    val density = LocalDensity.current
    val screenHeightPx = with(density) { configuration.screenHeightDp.dp.toPx() }

    val statusBarPadding = WindowInsets.statusBars.asPaddingValues().calculateTopPadding()
    val navBarPadding = WindowInsets.navigationBars.asPaddingValues().calculateBottomPadding()
    val editorTopPadding = statusBarPadding + CodeBarHeight + 8.dp
    val editorBottomPadding = navBarPadding + CircleButtonSize + 36.dp

    var isVisible by remember { mutableFloatStateOf(0f) }

    val animatedOffset by
        animateFloatAsState(
            targetValue = if (isVisible > 0.5f) 0f else screenHeightPx,
            animationSpec = tween(durationMillis = 300),
            label = "sheetOffset",
        )

    LaunchedEffect(file, darkMode, editorTopPadding, editorBottomPadding) {
        launch(Dispatchers.IO) {
            val fileData = readFileContent(file)
            content =
                loadHtmlTemplate(
                    context,
                    CodeLanguage.fromFile(file).monacoType,
                    fileData,
                    darkMode,
                    editorTopPadding.value,
                    editorBottomPadding.value,
                )
        }
    }

    LaunchedEffect(Unit) { isVisible = 1f }

    fun dismiss() {
        coroutineScope.launch {
            isVisible = 0f
            delay(300)
            onDismissRequest()
        }
    }

    Dialog(
        onDismissRequest = { dismiss() },
        properties =
            DialogProperties(
                dismissOnBackPress = true,
                dismissOnClickOutside = false,
                usePlatformDefaultWidth = false,
                decorFitsSystemWindows = false,
            ),
    ) {
        val dialogWindow = (LocalView.current.parent as? DialogWindowProvider)?.window

        VPinballTheme {
            DisposableEffect(darkMode, dialogWindow) {
                dialogWindow?.let { window ->
                    val insetsController = WindowCompat.getInsetsController(window, window.decorView)
                    insetsController.isAppearanceLightStatusBars = !darkMode
                    insetsController.isAppearanceLightNavigationBars = !darkMode
                }
                onDispose {}
            }

            val editorBackground = if (darkMode) EditorDark else EditorLight
            val barColor = MaterialTheme.colorScheme.surface.copy(alpha = 0.82f)

            Box(modifier = modifier.fillMaxSize().offset { IntOffset(0, animatedOffset.roundToInt()) }.background(editorBackground)) {
                AndroidView(
                    factory = { ctx ->
                        WebView(ctx).apply {
                            layoutParams = ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT)
                            settings.javaScriptEnabled = true
                            webChromeClient = WebChromeClient()
                            webViewClient = WebViewClient()
                            settings.domStorageEnabled = true
                            setBackgroundColor(Color.TRANSPARENT)
                        }
                    },
                    modifier = Modifier.fillMaxSize(),
                ) { webView ->
                    val htmlContent = content
                    if (htmlContent != null) {
                        webView.loadDataWithBaseURL("file:///android_asset/", htmlContent, "text/html", "UTF-8", null)
                    }
                }

                Box(modifier = Modifier.fillMaxWidth().height(statusBarPadding + CodeBarHeight).background(barColor))

                Row(
                    modifier = Modifier.fillMaxWidth().statusBarsPadding().height(CodeBarHeight).padding(horizontal = 16.dp),
                    verticalAlignment = Alignment.CenterVertically,
                ) {
                    Text(
                        text = file.name,
                        style = MaterialTheme.typography.titleMedium,
                        fontWeight = FontWeight.SemiBold,
                        color = MaterialTheme.colorScheme.onSurface,
                        maxLines = 1,
                        overflow = TextOverflow.Ellipsis,
                        modifier = Modifier.weight(1f),
                    )
                    CircleIconButton(iconRes = R.drawable.img_sf_xmark, contentDescription = "Close") { dismiss() }
                }

                Row(
                    modifier =
                        Modifier.align(Alignment.BottomCenter).fillMaxWidth().navigationBarsPadding().padding(horizontal = 16.dp, vertical = 12.dp),
                    verticalAlignment = Alignment.CenterVertically,
                ) {
                    if (file.exists()) {
                        CircleIconButton(iconRes = R.drawable.img_sf_square_and_arrow_up, contentDescription = "Share") { onShare(file) }
                    }

                    Spacer(modifier = Modifier.weight(1f))

                    if (canClear) {
                        CircleIconButton(iconRes = R.drawable.img_sf_trash, contentDescription = "Clear") {
                            if (file.exists()) {
                                FileOutputStream(file).use { output -> output.write("".toByteArray()) }
                            }
                            content =
                                loadHtmlTemplate(
                                    context,
                                    CodeLanguage.fromFile(file).monacoType,
                                    "",
                                    darkMode,
                                    editorTopPadding.value,
                                    editorBottomPadding.value,
                                )
                        }
                    }
                }
            }
        }
    }
}

private fun readFileContent(file: File): String {
    if (file.exists()) {
        return file.readBytes().toString(Charsets.UTF_8)
    } else {
        return "File not found."
    }
}

private fun loadHtmlTemplate(
    context: android.content.Context,
    language: String,
    logContent: String,
    darkMode: Boolean,
    paddingTop: Float,
    paddingBottom: Float,
): String {
    val theme = if (darkMode) "dark" else "light"

    fun escapeForJSON(input: String): String {
        return input
            .replace("\\", "\\\\")
            .replace("\"", "\\\"")
            .replace("\n", "\\n")
            .replace("\r", "\\r")
            .replace("\t", "\\t")
            .replace("\b", "\\b")
            .replace("\u000C", "\\f")
            .replace("/", "\\/")
    }

    val escapedContent = escapeForJSON(logContent)
    val insets = "<style>:root{--safe-top:${paddingTop}px;--safe-bottom:${paddingBottom}px;}</style></head>"

    return try {
        val inputStream = context.assets.open("assets/web/code-editor.html")
        val templateContent = inputStream.bufferedReader().use { it.readText() }

        templateContent
            .replace("{{THEME}}", theme)
            .replace("{{LANGUAGE}}", language)
            .replace("{{CONTENT}}", escapedContent)
            .replace("</head>", insets)
    } catch (e: Exception) {
        "<html><body><div style='display: flex; align-items: center; justify-content: center; height: 100vh; color: red; font-size: 16px; text-align: center; padding: 20px;'>Failed to load editor template: ${e.message}</div></body></html>"
    }
}
