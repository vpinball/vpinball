import SwiftUI
import WebKit

struct CodeWebView: UIViewRepresentable {
    @Environment(\.colorScheme) var colorScheme

    var language: CodeLanguage
    var code: String

    func makeUIView(context _: Context) -> WKWebView {
        let webView = WKWebView()
        webView.isInspectable = true
        webView.isOpaque = false
        webView.scrollView.bouncesZoom = false
        webView.scrollView.bounces = false
        return webView
    }

    func updateUIView(_ uiView: WKWebView, context _: Context) {
        let fullHtml = """
        <!DOCTYPE html>
        <html lang="en">
        <head>
            <meta charset="UTF-8">
            <meta name="viewport" content="width=device-width, initial-scale=1.0">
            <link rel=\"stylesheet\" href=\"assets/prismjs/prism-\(colorScheme == .light ? "light" : "dark").css\">
            <style>body { margin: 0; }</style>
        </head>
        <body class="line-numbers">
        <pre><code class="language-\(language.name)">\(code.htmlEscaped)</code></pre>
        <script src=\"assets/prismjs/prism.js\"></script>
        </body>
        </html>
        """

        uiView.loadHTMLString(fullHtml,
                              baseURL: Bundle.main.resourceURL)
    }
}
