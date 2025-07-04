import Foundation
import SwiftUI
import WebKit

struct CodeWebView: UIViewRepresentable {
    @Environment(\.colorScheme) var colorScheme

    var language: CodeLanguage
    var code: String

    func makeUIView(context _: Context) -> WKWebView {
        let configuration = WKWebViewConfiguration()
        configuration.preferences.setValue(true, forKey: "developerExtrasEnabled")

        let webView = WKWebView(frame: .zero,
                                configuration: configuration)
        webView.isInspectable = true
        webView.isOpaque = false
        webView.scrollView.bouncesZoom = false
        webView.scrollView.bounces = false

        return webView
    }

    func updateUIView(_ uiView: WKWebView, context _: Context) {
        let theme = colorScheme == .light ? "light" : "dark"
        let monacoLanguage = language.monacoType

        func escapeForJSON(_ string: String) -> String {
            return string
                .replacingOccurrences(of: "\\", with: "\\\\")
                .replacingOccurrences(of: "\"", with: "\\\"")
                .replacingOccurrences(of: "\n", with: "\\n")
                .replacingOccurrences(of: "\r", with: "\\r")
                .replacingOccurrences(of: "\t", with: "\\t")
                .replacingOccurrences(of: "\u{08}", with: "\\b")
                .replacingOccurrences(of: "\u{0C}", with: "\\f")
                .replacingOccurrences(of: "/", with: "\\/")
        }

        let escapedCode = escapeForJSON(code)

        let templatePath = Bundle.main.path(forResource: "code-editor", ofType: "html", inDirectory: "assets/web")
        if templatePath == nil {
            uiView.loadHTMLString("<html><body><div style='padding: 20px; text-align: center; color: red;'>Failed to load editor template</div></body></html>",
                                  baseURL: Bundle.main.resourceURL)
            return
        }

        let templateContent: String
        do {
            templateContent = try String(contentsOfFile: templatePath!, encoding: .utf8)
        } catch {
            uiView.loadHTMLString("<html><body><div style='padding: 20px; text-align: center; color: red;'>Failed to load editor template</div></body></html>",
                                  baseURL: Bundle.main.resourceURL)
            return
        }

        let fullHtml = templateContent
            .replacingOccurrences(of: "{{THEME}}", with: theme)
            .replacingOccurrences(of: "{{LANGUAGE}}", with: monacoLanguage)
            .replacingOccurrences(of: "{{CONTENT}}", with: escapedCode)

        uiView.loadHTMLString(fullHtml,
                              baseURL: Bundle.main.resourceURL)
    }
}
