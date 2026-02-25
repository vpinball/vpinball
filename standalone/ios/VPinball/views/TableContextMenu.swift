import SwiftUI

struct TableContextMenu: View {
    let table: Table

    private var hasScript: Bool {
        table.hasScriptFile()
    }

    private var hasIni: Bool {
        table.hasIniFile()
    }

    var body: some View {
        Section(table.name) {
            Button(action: { handleAction(.rename) }) {
                Label("Rename", systemImage: "pencil")
            }

            Button(action: { handleAction(.tableImage) }) {
                Label("Table Image", systemImage: "photo")
            }

            Divider()

            Button(action: { handleAction(.viewScript) }) {
                Label(hasScript ? "View Script" : "Extract Script",
                      systemImage: "applescript")
            }

            Divider()

            Button(action: { handleAction(.share) }) {
                Label("Share", systemImage: "square.and.arrow.up")
            }

            Divider()

            Button(role: .destructive, action: { handleAction(.reset) }) {
                Label("Reset", systemImage: "gobackward")
            }
            .disabled(!hasIni)

            Divider()

            Button(role: .destructive, action: { handleAction(.delete, delay: 1.0) }) {
                Label("Delete", systemImage: "trash")
            }
        }
    }

    private func handleAction(_ type: MainViewModel.ActionType, delay: TimeInterval = 0.5) {
        DispatchQueue.main.asyncAfter(deadline: .now() + delay) {
            MainViewModel.shared.setAction(type, table: table)
        }
    }
}
