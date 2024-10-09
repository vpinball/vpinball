import SwiftUI
import UIKit
import UniformTypeIdentifiers

struct DocumentPickerView: UIViewControllerRepresentable {
    @Binding var importFileURL: URL?

    func makeCoordinator() -> Coordinator {
        Coordinator(self)
    }

    func makeUIViewController(context: Context) -> UIDocumentPickerViewController {
        let picker = UIDocumentPickerViewController(forOpeningContentTypes: [UTType.vpx, UTType.vpxz, UTType.zip],
                                                    asCopy: !ProcessInfo.processInfo.isMacCatalystApp)
        picker.delegate = context.coordinator

        UISearchBar.appearance().overrideUserInterfaceStyle = .unspecified

        return picker
    }

    func updateUIViewController(_: UIDocumentPickerViewController, context _: Context) {}

    class Coordinator: NSObject, UIDocumentPickerDelegate {
        var parent: DocumentPickerView

        init(_ parent: DocumentPickerView) {
            self.parent = parent
        }

        func documentPicker(_: UIDocumentPickerViewController, didPickDocumentsAt urls: [URL]) {
            UISearchBar.appearance().overrideUserInterfaceStyle = .dark

            parent.importFileURL = urls.first
        }

        func documentPickerWasCancelled(_: UIDocumentPickerViewController) {
            UISearchBar.appearance().overrideUserInterfaceStyle = .dark

            parent.importFileURL = nil
        }
    }
}
