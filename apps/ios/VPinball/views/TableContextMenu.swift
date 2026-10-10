import SwiftUI

struct TableContextMenu: View {
    let table: Table
    var inRecentlyPlayed = false

    private var hasScript: Bool {
        table.hasScriptFile()
    }

    private var hasIni: Bool {
        table.hasIniFile()
    }

    var body: some View {
        Section(table.name) {
            Button(action: { handleAction(.toggleFavorite, delay: 0) }) {
                Label(table.isFavorite ? "Remove from Favorites" : "Add to Favorites",
                      systemImage: table.isFavorite ? "heart.slash" : "heart")
            }

            if inRecentlyPlayed {
                Button(action: { handleAction(.removeFromRecent, delay: 0) }) {
                    Label("Remove from Recently Played", systemImage: "clock.badge.xmark")
                }
            }
        }

        Section {
            Button(action: { handleAction(.rename) }) {
                Label("Rename", systemImage: "pencil")
            }

            Button(action: { handleAction(.chooseImage) }) {
                Label("Set Image", systemImage: "photo.on.rectangle.angled")
            }
        }

        Section {
            Button(action: { handleAction(.viewScript) }) {
                Label(hasScript ? "View Script" : "Extract Script",
                      systemImage: "applescript")
            }

            Button(action: { handleAction(.share) }) {
                Label("Share", systemImage: "square.and.arrow.up")
            }
        }

        Section {
            Button(action: { handleAction(.resetImage) }) {
                Label("Reset Image", systemImage: "photo")
            }
            .disabled(table.image.isEmpty)

            Button(action: { handleAction(.resetSettings) }) {
                Label("Reset Settings", systemImage: "slider.horizontal.3")
            }
            .disabled(!hasIni)
        }

        Section {
            Button(action: { handleAction(.delete) }) {
                Label("Delete", systemImage: "trash")
            }
        }
    }

    private func handleAction(_ type: MainViewModel.ActionType, delay: TimeInterval = 0.5) {
        if delay == 0 {
            MainViewModel.shared.setAction(type, table: table)
            return
        }
        DispatchQueue.main.asyncAfter(deadline: .now() + delay) {
            MainViewModel.shared.setAction(type, table: table)
        }
    }
}
