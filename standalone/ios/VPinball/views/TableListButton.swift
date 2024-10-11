import PhotosUI
import SwiftData
import SwiftUI

struct TableListButton: View {
    @EnvironmentObject var vpinballViewModel: VPinballViewModel

    @State var table: PinTable
    @State var selectedTable: PinTable?

    var showTitle = true

    var body: some View {
        TableImageView(table: table)
            .overlay {
                if showTitle {
                    VStack {
                        Spacer()

                        VStack {
                            Text(table.name)
                                .multilineTextAlignment(.center)
                                .font(.footnote)
                                .bold()
                                .foregroundStyle(Color.white)
                                .padding(.vertical, 4)
                        }
                        .frame(maxWidth: .infinity)
                        .background(.ultraThinMaterial, in:
                            RoundedRectangle(cornerRadius: 6.0))
                    }
                    .padding(8)
                }
            }
            .contextMenu {
                Section(table.name) {
                    Button(action: {
                        handleShowRename()
                    }) {
                        Label("Rename",
                              systemImage: "pencil")
                    }

                    Button(action: {
                        handleShowChangeArtwork()
                    }) {
                        Label("Change Artwork",
                              systemImage: "photo")
                    }

                    Divider()

                    Button(action: {
                        handleViewScript()
                    }) {
                        Label(table.hasScript() ? "View Script" : "Extract Script",
                              systemImage: "applescript")
                    }

                    Divider()

                    Button(action: {
                        handleShare()
                    }) {
                        Label("Share",
                              systemImage: "square.and.arrow.up")
                    }

                    Divider()

                    Button(role: .destructive,
                           action: {
                               handleReset()
                           }) {
                        Label("Reset",
                              systemImage: "gobackward")
                    }
                    .disabled(!table.hasIni())

                    Divider()

                    Button(role: .destructive,
                           action: {
                               handleDelete()
                           }) {
                        Label("Delete",
                              systemImage: "trash")
                    }
                }
            }
            preview: {
                TableImageView(table: table)
                    .padding(4)
                    .background(Color.black)
                    .frame(height: 300)
            }
            .id(table.lastUpdate)
    }

    func handleShowRename() {
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
            vpinballViewModel.setAction(action: .rename,
                                        table: table)
        }
    }

    func handleShowChangeArtwork() {
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
            vpinballViewModel.setAction(action: .changeArtwork,
                                        table: table)
        }
    }

    func handleViewScript() {
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
            vpinballViewModel.setAction(action: .viewScript,
                                        table: table)
        }
    }

    func handleShare() {
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
            vpinballViewModel.setAction(action: .share,
                                        table: table)
        }
    }

    func handleReset() {
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
            vpinballViewModel.setAction(action: .reset,
                                        table: table)
        }
    }

    func handleDelete() {
        DispatchQueue.main.asyncAfter(deadline: .now() + 1.0) {
            vpinballViewModel.setAction(action: .delete,
                                        table: table)
        }
    }
}
