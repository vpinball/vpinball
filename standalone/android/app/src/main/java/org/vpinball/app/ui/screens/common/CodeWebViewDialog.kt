package org.vpinball.app.ui.screens.common

import android.graphics.Color
import android.text.Html
import android.view.ViewGroup
import android.view.Window
import android.webkit.WebChromeClient
import android.webkit.WebView
import android.webkit.WebViewClient
import androidx.compose.foundation.background
import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.foundation.layout.Box
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
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
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

    val darkMode = isSystemInDarkTheme()
    LaunchedEffect(file) {
        launch(Dispatchers.IO) {
            val fileData = readFileContent(file)
            content = generateHtmlWithPrism(CodeLanguage.fromFile(file).prismType, fileData, darkMode)
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
                val htmlContent = content
                if (htmlContent == null) {
                    Box(modifier = Modifier.fillMaxSize(), contentAlignment = Alignment.Center) { Text("Loading data...") }
                } else {
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
                    ) {
                        it.loadDataWithBaseURL("file:///android_asset/", htmlContent, "text/html", "UTF-8", null)
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
                                content = ""
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

private fun generateHtmlWithPrism(language: String, logContent: String, darkMode: Boolean): String {
    val cssFile = if (darkMode) "prism-dark.css" else "prism-light.css"
    val prismCss = "<link rel=\"stylesheet\" href=\"assets/prismjs/$cssFile\">"
    val prismJs = "<script src=\"assets/prismjs/prism.js\"></script>"
    val escapedLogContent = Html.escapeHtml(logContent)

    return """
        <!DOCTYPE html>
        <html lang="en">
        <head>
            <meta charset="UTF-8">
            <meta name="viewport" content="width=device-width, initial-scale=1.0">
            $prismCss
            $prismJs
            <style>
                body {
                    margin: 0;
                    visibility: hidden;
                }
            </style>
            <script>
                window.onload = function() {
                    Prism.highlightAll();
                    document.body.style.visibility = 'visible';
                };
            </script>
        </head>
        <body class="line-numbers">
            <pre><code class="language-$language">$escapedLogContent</code></pre>
        </body>
        </html>
        """
        .trimIndent()
}
