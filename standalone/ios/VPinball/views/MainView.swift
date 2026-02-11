import PhotosUI
import SwiftUI

struct MainView: View {
    @Environment(\.dismissSearch) var dismissSearch

    @State var settingsSection: String?
    @State var showSettings = false

    @State var confirmImportTableURL: URL?
    @State var showConfirmImportTable = false

    @State var importTableURL: URL?
    @State var showImportTable = false

    @State var tableViewMode: TableViewMode = .grid
    @State var tableGridSize: TableGridSize = .medium
    @State var tableListSortOrder: SortOrder = .forward
    @State var tableListSearchText = ""
    @State var tableListScrollToTable: Table?

    @State var selectedTable: Table? = nil

    @State var shareItems: [Any] = []
    @State var showShare = false

    @State var renameTableName = ""
    @State var showRenameTable = false

    @State var showTableImage = false
    @State var tableImagePhotoItem: PhotosPickerItem?
    @State var showTableImagePhotoPicker = false

    @State var showScript = false

    @State var errorMessage = ""
    @State var showError = false

    @ObservedObject var tableManager = TableManager.shared
    @ObservedObject var vpinballViewModel = VPinballViewModel.shared

    let vpinballManager = VPinballManager.shared
    let settingsModel = SettingsModel()

    var body: some View {
        ZStack {
            NavigationStack {
                ZStack {
                    Color.lightBlack.ignoresSafeArea()

                    TableListView(viewMode: tableViewMode,
                                  gridSize: tableGridSize,
                                  sortOrder: tableListSortOrder,
                                  searchText: tableListSearchText,
                                  scrollToTable: $tableListScrollToTable)
                        .searchable(text: $tableListSearchText)
                        .disabled(tableManager.tables.isEmpty)

                    if tableManager.tables.isEmpty {
                        GeometryReader { geometry in
                            VStack {
                                Spacer()

                                VStack(spacing: 40) {
                                    TablePlaceholderImage()

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
                                            handleLearnMore(link: Link.docs)
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
                            handleShowSettings()
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
                            Picker("View Mode", selection: $tableViewMode) {
                                Label("Grid",
                                      systemImage: "square.grid.3x3")
                                    .tag(TableViewMode.grid)
                                Label("List",
                                      systemImage: "list.bullet")
                                    .tag(TableViewMode.list)
                            }

                            if tableViewMode == .grid {
                                Picker("Grid Size", selection: $tableGridSize) {
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

                            Picker("Sort Order", selection: $tableListSortOrder) {
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
                                    handleShowImportTable()
                                }) {
                                    Label("Files",
                                          systemImage: "doc")
                                }
                            }

                            Section("Built in...") {
                                Button(action: {
                                    handleImportTableFromAssets("blankTable")
                                }) {
                                    Text("blankTable.vpx")
                                }

                                Button(action: {
                                    handleImportTableFromAssets("exampleTable")
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

            if let table = vpinballManager.activeTable {
                Color.lightBlack

                if let uiImage = table.uiImage {
                    Image(uiImage: uiImage)
                        .resizable()
                        .aspectRatio(contentMode: .fit)
                } else {
                    TablePlaceholderImage(contentMode: .fit)
                }
            }

            if vpinballViewModel.showHUD {
                HUDOverlayView()
            }
        }
        .ignoresSafeArea()
        .sheet(isPresented: $showSettings,
               content: {
                   SettingsView(settingsModel: settingsModel, focusSection: settingsSection)
                       .presentationDetents([.custom(CustomDetent.self)])
                       .presentationDragIndicator(.hidden)
                       .ignoresSafeArea()
               })
        .sheet(isPresented: $showImportTable,
               content: {
                   DocumentPickerView(importFileURL: $importTableURL)
                       .presentationDetents([.custom(CustomDetent.self)])
                       .presentationDragIndicator(.hidden)
                       .ignoresSafeArea()
               })
        .sheet(isPresented: $showShare,
               content: {
                   ActivityViewControllerView(activityItems: $shareItems,
                                              excludedActivityTypes: [.mail,
                                                                      .message,
                                                                      .postToFacebook])
                       .presentationDetents([.medium])
                       .presentationDragIndicator(.hidden)
                       .ignoresSafeArea()
               })
        .confirmationDialog("",
                            isPresented: $showTableImage,
                            titleVisibility: .hidden)
        {
            Button("Photo Library") {
                handleShowTableImagePhotoPicker()
            }

            Button("Reset",
                   role: .destructive)
            {
                handleTableImageReset()
            }

            Button("Cancel", role: .cancel) {}
        }
        .photosPicker(isPresented: $showTableImagePhotoPicker,
                      selection: $tableImagePhotoItem,
                      matching: .any(of: [.images,
                                          .screenshots,
                                          .livePhotos]))
        .fullScreenCover(isPresented: $showScript, content: {
            if let table = selectedTable {
                CodeView(url: table.scriptURL,
                         language: .vbscript)
            }
        })
        .alert("Confirm Import Table", isPresented: $showConfirmImportTable) {
            Button("OK") {
                handleConfirmImportTable()
            }
            Button("Cancel", role: .cancel) {
                handleConfirmImportTableCancel()
            }
        }
        message: {
            if let filename = confirmImportTableURL?.lastPathComponent.removingPercentEncoding {
                Text("\nImport \"\(filename)\"?")
            }
        }
        .alert("Rename Table", isPresented: $showRenameTable) {
            TextField("", text: $renameTableName)
                .textInputAutocapitalization(.never)
            Button("Rename") {
                handleRenameTable()
            }
            Button("Cancel", role: .cancel) {}
        }
        .alert("TILT!",
               isPresented: $showError)
        {
            Button("Learn More") {
                handleLearnMore(link: Link.troubleshooting)
            }
            Button("OK") {}
        }
        message: {
            Text("\n\(errorMessage)")
        }
        .onAppear {
            handleAppear()
        }
        // .onOpenURL { url in
        //     if url.scheme == "file" {
        //         handleShowConfirmImportFile(url: url)
        //     }
        // }
        .onChange(of: tableViewMode) {
            handleTableViewMode()
        }
        .onChange(of: tableGridSize) {
            handleTableGridSize()
        }
        .onChange(of: tableListSortOrder) {
            handleTableListSortOrder()
        }
        .onChange(of: importTableURL) {
            handleShowConfirmImportTable(url: importTableURL)
        }
        .onChange(of: tableManager.tables) {
            handleTables()
        }
        .onChange(of: vpinballViewModel.didSetAction) {
            handleAction()
        }
        .onChange(of: vpinballViewModel.scrollToTable) {
            tableListScrollToTable = vpinballViewModel.scrollToTable
        }
        .onChange(of: tableImagePhotoItem) {
            handleTableImagePhotoItem()
        }
        .onChange(of: selectedTable) {}
    }

    func handleAppear() {
        tableViewMode = TableViewMode(rawValue: vpinballManager.loadValue(.standalone,
                                                                          "TableViewMode",
                                                                          TableViewMode.grid.rawValue)) ?? .grid

        tableGridSize = TableGridSize(rawValue: vpinballManager.loadValue(.standalone,
                                                                          "TableGridSize",
                                                                          TableGridSize.medium.rawValue)) ?? .medium

        tableListSortOrder = vpinballManager.loadValue(.standalone,
                                                       "TableListSort",
                                                       1) == 1 ? .forward : .reverse

        Task {
            await tableManager.refresh()
        }
    }

    func handleShowConfirmImportTable(url: URL?) {
        if let url = url {
            importTableURL = nil
            confirmImportTableURL = url
            showConfirmImportTable = true
        }
    }

    func handleConfirmImportTable() {
        if let url = confirmImportTableURL {
            handleImportTable(url: url)
        }
        confirmImportTableURL = nil
    }

    func handleConfirmImportTableCancel() {
        confirmImportTableURL = nil
    }

    func handleShowSettings(section: String? = nil) {
        settingsSection = section

        showSettings = true
    }

    func handleTableViewMode() {
        vpinballManager.saveValue(.standalone, "TableViewMode", tableViewMode.rawValue)
    }

    func handleTableGridSize() {
        vpinballManager.saveValue(.standalone, "TableGridSize", tableGridSize.rawValue)
    }

    func handleTableListSortOrder() {
        vpinballManager.saveValue(.standalone, "TableListSort", tableListSortOrder == .forward ? 1 : 0)
    }

    func handleShowImportTable() {
        showImportTable = true
    }

    func handleImportTableFromAssets(_ name: String) {
        handleShowConfirmImportTable(url: Bundle.main.url(forResource: "assets/\(name)",
                                                          withExtension: "vpx"))
    }

    func handleImportTable(url: URL?) {
        if let url = url {
            Task {
                let scopedResource = url.startAccessingSecurityScopedResource()

                _ = await tableManager.importTable(from: url)

                if scopedResource {
                    url.stopAccessingSecurityScopedResource()
                }
            }
        }
    }

    func handleLearnMore(link: Link) {
        link.open()
    }

    func handleTables() {
        if tableManager.tables.isEmpty {
            tableListSearchText = ""
            dismissSearch()
        }
    }

    func handleAction() {
        UIApplication.shared.sendAction(#selector(UIResponder.resignFirstResponder),
                                        to: nil,
                                        from: nil,
                                        for: nil)

        selectedTable = vpinballViewModel.table

        switch vpinballViewModel.action {
        case .play:
            handlePlayTable()
        case .rename:
            handleShowRenameTable()
        case .tableImage:
            handleShowTableImage()
        case .viewScript:
            handleViewTableScript()
        case .share:
            handleShareTable()
        case .reset:
            handleResetTable()
        case .delete:
            handleDeleteTable()
        case .stopped:
            handleTableStopped()
        default:
            break
        }
    }

    func handleShowRenameTable() {
        if let selectedTable = selectedTable {
            renameTableName = selectedTable.name
            showRenameTable = true
            tableListScrollToTable = nil
        }
    }

    func handleRenameTable() {
        if let selectedTable = selectedTable {
            Task {
                _ = await tableManager.renameTable(table: selectedTable,
                                                   newName: renameTableName)
                tableListScrollToTable = selectedTable
            }
        }
    }

    func handleShowTableImage() {
        tableImagePhotoItem = nil
        showTableImage = true
    }

    func handleShowTableImagePhotoPicker() {
        showTableImagePhotoPicker = true
    }

    func handleTableImagePhotoItem() {
        if let selectedTable = selectedTable {
            if let item = tableImagePhotoItem {
                item.loadTransferable(type: Data.self) { result in
                    switch result {
                    case let .success(data?):
                        DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
                            if let image = UIImage(data: data) {
                                Task {
                                    await tableManager.setTableImage(table: selectedTable,
                                                                     image: image)
                                }
                            } else {
                                Task {
                                    await tableManager.setTableImage(table: selectedTable,
                                                                     imagePath: "")
                                }
                            }
                        }
                    default:
                        break
                    }
                }
            }
        }
    }

    func handleTableImageReset() {
        if let selectedTable = selectedTable {
            DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
                Task {
                    await tableManager.setTableImage(table: selectedTable,
                                                     imagePath: "")
                }
            }
        }
    }

    func handleViewTableScript() {
        if let selectedTable = selectedTable {
            if selectedTable.hasScriptFile() {
                showScript = true
            } else {
                Task {
                    if await tableManager.extractTableScript(table: selectedTable) {
                        showScript = true
                    } else {
                        handleShowError(message: "Unable to extract script")
                    }
                }
            }
        }
    }

    func handleShareTable() {
        if let selectedTable = selectedTable {
            Task {
                if let path = await tableManager.exportTable(table: selectedTable) {
                    shareItems = [URL(fileURLWithPath: path)]
                    showShare = true
                }
            }
        }
    }

    func handleResetTable() {
        if let selectedTable = selectedTable {
            _ = tableManager.resetTableIni(table: selectedTable)
            DispatchQueue.main.asyncAfter(deadline: .now() + 0.25) {
                tableListScrollToTable = selectedTable
            }
        }
    }

    func handleDeleteTable() {
        if let selectedTable = selectedTable {
            Task {
                await tableManager.deleteTable(table: selectedTable)
            }
        }
    }

    func handlePlayTable() {
        if vpinballManager.activeTable != nil {
            return
        }

        if let selectedTable = selectedTable {
            Task {
                if await vpinballManager.play(table: selectedTable) != true {
                    handleShowError(message: "Unable to load table")
                }
            }
        }
    }

    func handleTableStopped() {
    }

    func handleShowError(message: String) {
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
            errorMessage = message
            showError = true
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
