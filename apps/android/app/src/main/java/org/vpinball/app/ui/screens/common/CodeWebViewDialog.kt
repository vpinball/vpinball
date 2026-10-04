package org.vpinball.app.ui.screens.common

import android.graphics.Color
import android.view.ViewGroup
import android.webkit.WebChromeClient
import android.webkit.WebView
import android.webkit.WebViewClient
import androidx.compose.animation.core.animateFloatAsState
import androidx.compose.animation.core.tween
import androidx.compose.foundation.background
import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.navigationBarsPadding
import androidx.compose.foundation.layout.offset
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.requiredSize
import androidx.compose.foundation.layout.statusBarsPadding
import androidx.compose.material3.BottomAppBarDefaults
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.material3.TopAppBar
import androidx.compose.runtime.Composable
import androidx.compose.runtime.DisposableEffect
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableFloatStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalConfiguration
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.platform.LocalDensity
import androidx.compose.ui.platform.LocalView
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.text.font.FontWeight
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
import org.vpinball.app.ui.theme.VpxRed

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun CodeWebViewDialog(file: File, canClear: Boolean, onDismissRequest: () -> Unit, onShare: (file: File) -> Unit, modifier: Modifier = Modifier) {
    var content by remember { mutableStateOf<String?>(null) }

    val context = LocalContext.current
    val darkMode = isSystemInDarkTheme()
    val coroutineScope = rememberCoroutineScope()

    val configuration = LocalConfiguration.current
    val density = LocalDensity.current
    val screenHeightPx = with(density) { configuration.screenHeightDp.dp.toPx() }

    var isVisible by remember { mutableFloatStateOf(0f) }

    val animatedOffset by
        animateFloatAsState(
            targetValue = if (isVisible > 0.5f) 0f else screenHeightPx,
            animationSpec = tween(durationMillis = 300),
            label = "sheetOffset",
        )

    LaunchedEffect(file) {
        launch(Dispatchers.IO) {
            val fileData = readFileContent(file)
            content = loadHtmlTemplate(context, CodeLanguage.fromFile(file).monacoType, fileData, darkMode)
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

        DisposableEffect(darkMode, dialogWindow) {
            dialogWindow?.let { window ->
                val insetsController = WindowCompat.getInsetsController(window, window.decorView)
                insetsController.isAppearanceLightStatusBars = !darkMode
            }
            onDispose {}
        }

        Box(modifier = modifier.fillMaxSize().offset { IntOffset(0, animatedOffset.roundToInt()) }) {
            Surface(modifier = Modifier.fillMaxSize()) {
                Column {
                    TopAppBar(
                        modifier = Modifier.statusBarsPadding(),
                        title = { Text(text = file.name, fontSize = MaterialTheme.typography.titleLarge.fontSize, fontWeight = FontWeight.Bold) },
                        actions = {
                            TextButton(onClick = { dismiss() }) {
                                Text(
                                    text = "Done",
                                    color = androidx.compose.ui.graphics.Color.VpxRed,
                                    fontSize = MaterialTheme.typography.titleMedium.fontSize,
                                    fontWeight = FontWeight.SemiBold,
                                )
                            }
                        },
                    )
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
                        modifier = Modifier.fillMaxSize().weight(1f),
                    ) { webView ->
                        val htmlContent = content
                        if (htmlContent != null) {
                            webView.loadDataWithBaseURL("file:///android_asset/", htmlContent, "text/html", "UTF-8", null)
                        } else {
                            webView.loadDataWithBaseURL(
                                "file:///android_asset/",
                                """<!DOCTYPE html>
                                   <html><head><meta charset="UTF-8">
                                   <meta name="viewport" content="width=device-width, initial-scale=1.0">
                                   <style>
                                   body { margin: 0; padding: 0; font-family: system-ui; }
                                   .loading { display: flex; align-items: center; justify-content: center; height: 100vh; color: #666; font-size: 16px; }
                                   </style></head>
                                   <body><div class="loading">Loading...</div></body></html>""",
                                "text/html",
                                "UTF-8",
                                null,
                            )
                        }
                    }
                    Row(modifier = Modifier.fillMaxWidth().background(BottomAppBarDefaults.containerColor).navigationBarsPadding()) {
                        if (file.exists()) {
                            IconButton(onClick = { onShare(file) }, modifier = Modifier.padding(start = 4.dp, top = 4.dp, bottom = 4.dp)) {
                                Icon(
                                    painter = painterResource(R.drawable.img_sf_square_and_arrow_up),
                                    modifier = Modifier.requiredSize(24.dp),
                                    tint = androidx.compose.ui.graphics.Color.VpxRed,
                                    contentDescription = "Share",
                                )
                            }
                        }

                        Spacer(modifier = Modifier.weight(1f))

                        if (canClear) {
                            TextButton(
                                onClick = {
                                    if (file.exists()) {
                                        FileOutputStream(file).use { output -> output.write("".toByteArray()) }
                                    }
                                    content = loadHtmlTemplate(context, CodeLanguage.fromFile(file).monacoType, "", darkMode)
                                },
                                modifier = Modifier.padding(end = 4.dp, top = 4.dp, bottom = 4.dp),
                            ) {
                                Text(
                                    text = "Clear",
                                    color = androidx.compose.ui.graphics.Color.VpxRed,
                                    fontSize = MaterialTheme.typography.titleMedium.fontSize,
                                    fontWeight = FontWeight.SemiBold,
                                )
                            }
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

private fun loadHtmlTemplate(context: android.content.Context, language: String, logContent: String, darkMode: Boolean): String {
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

    return try {
        val inputStream = context.assets.open("assets/web/code-editor.html")
        val templateContent = inputStream.bufferedReader().use { it.readText() }

        templateContent.replace("{{THEME}}", theme).replace("{{LANGUAGE}}", language).replace("{{CONTENT}}", escapedContent)
    } catch (e: Exception) {
        "<html><body><div style='display: flex; align-items: center; justify-content: center; height: 100vh; color: red; font-size: 16px; text-align: center; padding: 20px;'>Failed to load editor template: ${e.message}</div></body></html>"
    }
}
