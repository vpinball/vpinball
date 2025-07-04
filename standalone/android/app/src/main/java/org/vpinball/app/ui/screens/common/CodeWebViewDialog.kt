package org.vpinball.app.ui.screens.common

import android.graphics.Color
import android.view.ViewGroup
import android.view.Window
import android.webkit.WebChromeClient
import android.webkit.WebView
import android.webkit.WebViewClient
import androidx.compose.foundation.background
import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.requiredSize
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
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.ReadOnlyComposable
import androidx.compose.runtime.SideEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.platform.LocalView
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.viewinterop.AndroidView
import androidx.compose.ui.window.DialogWindowProvider
import java.io.BufferedReader
import java.io.File
import java.io.FileInputStream
import java.io.FileOutputStream
import java.io.InputStreamReader
import kotlinx.coroutines.Dispatchers
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
    LaunchedEffect(file) {
        launch(Dispatchers.IO) {
            val fileData = readFileContent(file)
            content = loadHtmlTemplate(context, CodeLanguage.fromFile(file).monacoType, fileData, darkMode)
        }
    }
    DialogFullScreen(onDismissRequest = onDismissRequest) {
        val dialogWindow = getDialogWindow()

        // we want to remove the dialog dimming animations as it looks janky
        // with the webview
        SideEffect {
            dialogWindow.let { window ->
                window?.setDimAmount(0f)
                window?.setWindowAnimations(-1)
            }
        }

        // we are using surface & column instead of Scaffold due to the Scaffold
        // manipulating the content margins for the webview which results in
        // the html being cut off.
        Surface(modifier = modifier.fillMaxSize()) {
            Column {
                TopAppBar(
                    title = { Text(text = file.name, fontSize = MaterialTheme.typography.titleLarge.fontSize, fontWeight = FontWeight.Bold) },
                    actions = {
                        TextButton(onClick = onDismissRequest) {
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
                Row(modifier = Modifier.fillMaxWidth().background(BottomAppBarDefaults.containerColor)) {
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

@ReadOnlyComposable @Composable private fun getDialogWindow(): Window? = (LocalView.current.parent as? DialogWindowProvider)?.window

private fun readFileContent(file: File): String {
    if (file.exists()) {
        FileInputStream(file).use { inputStream ->
            val reader = BufferedReader(InputStreamReader(inputStream))
            return reader.readText()
        }
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
