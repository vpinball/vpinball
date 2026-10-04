import PhotosUI
import SwiftUI

struct MainView: View {
    @Environment(\.dismissSearch) var dismissSearch

    @State private var activeTableImage: UIImage?

    @ObservedObject var vpinballModel = VPinballModel.shared
    @ObservedObject var mainViewModel = MainViewModel.shared

    let settingsModel = SettingsModel()

    var body: some View {
        ZStack {
            NavigationStack {
                ZStack {
                    Color.lightBlack.ignoresSafeArea()

                    TableGridView(tables: vpinballModel.tables,
                                  viewMode: mainViewModel.tableViewMode,
                                  gridSize: mainViewModel.tableGridSize,
                                  sortOrder: mainViewModel.tableListSortOrder,
                                  searchText: mainViewModel.tableListSearchText,
                                  scrollToTable: $mainViewModel.scrollToTable)
                        .searchable(text: $mainViewModel.tableListSearchText)
                        .disabled(vpinballModel.tables.isEmpty)

                    if vpinballModel.tables.isEmpty {
                        GeometryReader { geometry in
                            VStack {
                                Spacer()

                                VStack(spacing: 40) {
                                    TableImagePlaceholderView()

                                    VStack(spacing: 20) {
                                        Text("Insert Tables")
                                            .font(.title)
                                            .bold()
                                            .foregroundStyle(Color.vpxDarkYellow)
                                            .blinkEffect()

                                        Group {
                                            Text("You can import tables by pressing the ")
                                                + Text(Image(systemName: "plus"))
                                                .foregroundStyle(Color.vpxDarkYellow)
                                                + Text(" button in the top right.")
                                        }
                                        .font(.callout)
                                        .foregroundStyle(Color.white)
                                        .multilineTextAlignment(.center)
                                        .padding(.horizontal, 40)

                                        Button(action: {
                                            Link.docs.open()
                                        }) {
                                            Text("Learn More...")
                                                .foregroundStyle(Color.vpxDarkYellow)
                                        }
                                    }
                                }
                                .frame(height: geometry.size.height * 0.90)

                                Spacer()
                            }
                            .frame(maxWidth: .infinity)
                        }
                    }
                }
                .navigationBarTitleDisplayMode(.inline)
                .toolbar {
                    ToolbarItem(placement: .topBarLeading) {
                        Button(action: {
                            mainViewModel.showSettings = true
                        }) {
                            Image(systemName: "gearshape")
                                .foregroundStyle(Color.vpxDarkYellow)
                        }
                    }
                    ToolbarItem(placement: .principal) {
                        Image("vpinball-logo")
                            .resizable()
                            .scaledToFit()
                            .frame(height: 33)
                    }
                    ToolbarItem(placement: .topBarTrailing) {
                        Menu(content: {
                            Picker("View Mode", selection: $mainViewModel.tableViewMode) {
                                Label("Grid",
                                      systemImage: "square.grid.3x3")
                                    .tag(TableViewMode.grid)
                                Label("List",
                                      systemImage: "list.bullet")
                                    .tag(TableViewMode.list)
                            }

                            if mainViewModel.tableViewMode == .grid {
                                Picker("Grid Size", selection: $mainViewModel.tableGridSize) {
                                    Label("Small",
                                          systemImage: "rectangle.split.3x1")
                                        .tag(TableGridSize.small)
                                    Label("Medium",
                                          systemImage: "rectangle.split.2x1")
                                        .tag(TableGridSize.medium)
                                    Label("Large",
                                          systemImage: "rectangle.portrait")
                                        .tag(TableGridSize.large)
                                }
                            }

                            Picker("Sort Order", selection: $mainViewModel.tableListSortOrder) {
                                Label("A-Z",
                                      systemImage: "arrow.up")
                                    .tag(SortOrder.forward)
                                Label("Z-A",
                                      systemImage: "arrow.down")
                                    .tag(SortOrder.reverse)
                            }
                        }, label: {
                            Image(systemName: "ellipsis.circle")
                                .foregroundStyle(Color.vpxDarkYellow)
                        })
                    }
                    ToolbarItem(placement: .topBarTrailing) {
                        Menu(content: {
                            Section("Import From...") {
                                Button(action: {
                                    mainViewModel.showImportTable = true
                                }) {
                                    Label("Files",
                                          systemImage: "doc")
                                }
                            }

                            Section("Built in...") {
                                Button(action: {
                                    mainViewModel.handleShowConfirmImportTable(url: Bundle.main.url(forResource: "assets/blankTable", withExtension: "vpx"))
                                }) {
                                    Text("blankTable.vpx")
                                }

                                Button(action: {
                                    mainViewModel.handleShowConfirmImportTable(url: Bundle.main.url(forResource: "assets/exampleTable", withExtension: "vpx"))
                                }) {
                                    Text("exampleTable.vpx")
                                }
                            }
                        }, label: {
                            Image(systemName: "plus")
                                .foregroundStyle(Color.vpxDarkYellow)
                        })
                    }
                }
                .toolbarBackground(Color.darkBlack,
                                   for: .navigationBar)
                .toolbarBackground(.visible,
                                   for: .navigationBar)
            }

            if let table = vpinballModel.activeTable {
                Group {
                    Color.lightBlack

                    if let activeTableImage {
                        Image(uiImage: activeTableImage)
                            .resizable()
                            .aspectRatio(contentMode: .fit)
                    } else {
                        TableImagePlaceholderView(contentMode: .fit)
                    }
                }
                .task(id: "\(table.uuid)_\(table.modifiedAt)") {
                    activeTableImage = await table.uiImageAsync()
                }
            }

            if vpinballModel.showHUD {
                HUDOverlayView()
            }
        }
        .ignoresSafeArea()
        .sheet(isPresented: $mainViewModel.showSettings,
               content: {
                   SettingsView(settingsModel: settingsModel)
                       .presentationDetents([.custom(CustomDetent.self)])
                       .presentationDragIndicator(.hidden)
                       .ignoresSafeArea()
               })
        .sheet(isPresented: $mainViewModel.showImportTable,
               content: {
                   DocumentPickerView(importFileURL: $mainViewModel.importTableURL)
                       .presentationDetents([.custom(CustomDetent.self)])
                       .presentationDragIndicator(.hidden)
                       .ignoresSafeArea()
               })
        .sheet(isPresented: $mainViewModel.showShare,
               content: {
                   ActivityViewControllerView(activityItems: $mainViewModel.shareItems,
                                              excludedActivityTypes: [.mail,
                                                                      .message,
                                                                      .postToFacebook])
                       .presentationDetents([.medium])
                       .presentationDragIndicator(.hidden)
                       .ignoresSafeArea()
               })
        .confirmationDialog("",
                            isPresented: $mainViewModel.showTableImage,
                            titleVisibility: .hidden)
        {
            Button("Photo Library") {
                mainViewModel.showTableImagePhotoPicker = true
            }

            Button("Reset",
                   role: .destructive)
            {
                mainViewModel.handleTableImageReset()
            }

            Button("Cancel", role: .cancel) {}
        }
        .photosPicker(isPresented: $mainViewModel.showTableImagePhotoPicker,
                      selection: $mainViewModel.tableImagePhotoItem,
                      matching: .any(of: [.images,
                                          .screenshots,
                                          .livePhotos]))
        .fullScreenCover(isPresented: $mainViewModel.showScript, content: {
            if let table = mainViewModel.selectedTable {
                CodeView(url: table.scriptURL,
                         language: .vbscript)
            }
        })
        .alert("Confirm Import Table", isPresented: $mainViewModel.showConfirmImportTable) {
            Button("OK") {
                mainViewModel.handleConfirmImportTable()
            }
            Button("Cancel", role: .cancel) {
                mainViewModel.confirmImportTableURL = nil
            }
        }
        message: {
            if let filename = mainViewModel.confirmImportTableURL?.lastPathComponent.removingPercentEncoding {
                Text("\nImport \"\(filename)\"?")
            }
        }
        .alert("Rename Table", isPresented: $mainViewModel.showRenameTable) {
            TextField("", text: $mainViewModel.renameTableName)
                .textInputAutocapitalization(.never)
            Button("Rename") {
                mainViewModel.handleRenameTable()
            }
            Button("Cancel", role: .cancel) {}
        }
        .alert("TILT!",
               isPresented: $mainViewModel.showError)
        {
            Button("Learn More") {
                Link.troubleshooting.open()
            }
            Button("OK") {}
        }
        message: {
            Text("\n\(mainViewModel.errorMessage)")
        }
        .onAppear {
            mainViewModel.handleAppear()
        }
        .onChange(of: vpinballModel.tables) {
            if vpinballModel.tables.isEmpty {
                mainViewModel.tableListSearchText = ""
                dismissSearch()
            }
        }
        .onChange(of: mainViewModel.tableImagePhotoItem) {
            mainViewModel.handleTableImagePhotoItem()
        }
    }
}

struct CustomDetent: CustomPresentationDetent {
    static func height(in context: Context) -> CGFloat? {
        return context.maxDetentValue - 1
    }
}

#Preview {
    MainView()
}
