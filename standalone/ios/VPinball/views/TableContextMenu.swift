import SwiftUI

struct TableContextMenu: View {
    @ObservedObject var vpinballViewModel = VPinballViewModel.shared
    let table: Table

    @State private var hasScript = false
    @State private var hasIni = false

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
        .task(id: "\(table.uuid)_\(table.modifiedAt)") {
            let script = await table.hasScriptFileAsync()
            let ini = await table.hasIniFileAsync()
            await MainActor.run {
                hasScript = script
                hasIni = ini
            }
        }
    }

    private func handleAction(_ type: VPinballViewModel.ActionType, delay: TimeInterval = 0.5) {
        DispatchQueue.main.asyncAfter(deadline: .now() + delay) {
            vpinballViewModel.setAction(type, table: table)
        }
    }
}
