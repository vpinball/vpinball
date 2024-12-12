import PhotosUI
import SwiftData
import SwiftUI

struct MainView: View {
    @EnvironmentObject var vpinballViewModel: VPinballViewModel

    @Environment(\.modelContext) var modelContext
    @Environment(\.dismissSearch) var dismissSearch

    @State var settingsSection: String?
    @State var showSettings = false

    @State var confirmImportFileURL: URL?
    @State var showConfirmImportFile = false

    @State var importFileURL: URL?
    @State var showImport = false

    @State var tableListMode: TableListMode = .column2
    @State var tableListSortOrder: SortOrder = .forward
    @State var tableListSearchText = ""
    @State var tableListScrollToTableId: UUID?

    @State var selectedTable: PinTable? = nil

    @State var shareItems: [Any] = []
    @State var showShare = false

    @State var renameTableName = ""
    @State var showRename = false

    @State var showChangeArtwork = false
    @State var changeArtworkPhotoItem: PhotosPickerItem?
    @State var showChangeArtworkPhotoPicker = false

    @State var showScript = false

    @State var errorMessage = ""
    @State var showError = false

    @Query var tables: [PinTable]

    let vpinballManager = VPinballManager.shared
    let settingsModel = SettingsModel()

    var body: some View {
        ZStack {
            NavigationStack {
                ZStack {
                    Color.lightBlack.ignoresSafeArea()

                    TableListView(mode: tableListMode,
                                  sortOrder: tableListSortOrder,
                                  searchText: tableListSearchText,
                                  scrollToTableId: $tableListScrollToTableId)
                        .searchable(text: $tableListSearchText)
                        .disabled(tables.isEmpty)

                    if tables.isEmpty {
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
                            Picker("Table List", selection: $tableListMode) {
                                Label("2 Column",
                                      systemImage: "rectangle.split.2x1")
                                    .tag(TableListMode.column2)
                                Label("3 Column",
                                      systemImage: "rectangle.split.3x1")
                                    .tag(TableListMode.column3)
                                Label("List",
                                      systemImage: "list.bullet")
                                    .tag(TableListMode.list)
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
                                    handleShowImport()
                                }) {
                                    Label("Files",
                                          systemImage: "doc")
                                }
                            }

                            Section("Built in...") {
                                Button(action: {
                                    handleImportExampleTable()
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
                HUDOverlayView(type: vpinballViewModel.hudType!)
            }
        }
        .ignoresSafeArea()
        .sheet(isPresented: $showSettings,
               content: {
                   SettingsView(focusSection: settingsSection)
                       .presentationDetents([.custom(CustomDetent.self)])
                       .presentationDragIndicator(.hidden)
                       .ignoresSafeArea()
                       .environmentObject(settingsModel)
               })
        .sheet(isPresented: $showImport,
               content: {
                   DocumentPickerView(importFileURL: $importFileURL)
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
                            isPresented: $showChangeArtwork,
                            titleVisibility: .hidden)
        {
            Button("Photo Library") {
                handleShowChangeArtworkPhotoPicker()
            }

            Button("Reset",
                   role: .destructive)
            {
                handleChangeArtworkReset()
            }

            Button("Cancel", role: .cancel) {}
        }
        .photosPicker(isPresented: $showChangeArtworkPhotoPicker,
                      selection: $changeArtworkPhotoItem,
                      matching: .any(of: [.images,
                                          .screenshots,
                                          .livePhotos]))
        .fullScreenCover(isPresented: $showScript, content: {
            if let table = selectedTable {
                CodeView(url: table.scriptURL,
                         language: .vbscript)
            }
        })
        .alert("Warning!",
               isPresented: $vpinballViewModel.showLowMemoryNotice)
        {
            Button("Settings", role: .cancel) {
                handleLowMemoryNotice(showSettings: true)
            }

            Button("OK") {
                handleLowMemoryNotice()
            }
        }
        message: {
            Text("Low memory conditions were detected. To prevent crashes, consider reducing Max Texture Dimensions.")
        }
        .alert("Confirm Import Table", isPresented: $showConfirmImportFile) {
            Button("OK") {
                handleConfirmImportFile()
            }
            Button("Cancel", role: .cancel) {
                handleConfirmImportFileCancel()
            }
        }
        message: {
            if let filename = confirmImportFileURL?.lastPathComponent.removingPercentEncoding {
                Text("\nImport \"\(filename)\"?")
            }
        }
        .alert("Rename Table", isPresented: $showRename) {
            TextField("", text: $renameTableName)
                .textInputAutocapitalization(.never)
            Button("Rename") {
                handleRename()
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
        .onOpenURL { url in
            if url.scheme == "file" {
                handleShowConfirmImportFile(url: url)
            }
        }
        .onChange(of: tableListMode) {
            handleTableListMode()
        }
        .onChange(of: tableListSortOrder) {
            handleTableListSortOrder()
        }
        .onChange(of: importFileURL) {
            handleShowConfirmImportFile(url: importFileURL)
        }
        .onChange(of: tables) {
            handleTables()
        }
        .onChange(of: vpinballViewModel.didSetAction) {
            handleAction()
        }
        .onChange(of: changeArtworkPhotoItem) {
            handleChangeArtworkPhotoItem()
        }
        .onChange(of: selectedTable) {}
    }

    func handleAppear() {
        tableListMode = TableListMode(rawValue: vpinballManager.loadValue(.standalone,
                                                                          "TableListMode",
                                                                          TableListMode.column2.rawValue)) ?? .column2

        tableListSortOrder = vpinballManager.loadValue(.standalone,
                                                       "TableListSort",
                                                       1) == 1 ? .forward : .reverse

        for table in tables where !table.exists() {
            modelContext.delete(table)
        }
        try? modelContext.save()
    }

    func handleShowConfirmImportFile(url: URL?) {
        if let url = url {
            importFileURL = nil
            confirmImportFileURL = url
            showConfirmImportFile = true
        }
    }

    func handleConfirmImportFile() {
        if let url = confirmImportFileURL {
            handleImport(url: url)
        }
        confirmImportFileURL = nil
    }

    func handleConfirmImportFileCancel() {
        confirmImportFileURL = nil
    }

    func handleShowSettings(section: String? = nil) {
        settingsSection = section

        showSettings = true
    }

    func handleTableListMode() {
        vpinballManager.saveValue(.standalone, "TableListMode", tableListMode.rawValue)
    }

    func handleTableListSortOrder() {
        vpinballManager.saveValue(.standalone, "TableListSort", tableListSortOrder == .forward ? 1 : 0)
    }

    func handleShowImport() {
        showImport = true
    }

    func handleImportExampleTable() {
        handleShowConfirmImportFile(url: Bundle.main.url(forResource: "assets/exampleTable",
                                                         withExtension: "vpx"))
    }

    func handleImport(url: URL?) {
        if let url = url {
            Task {
                let scopedResource = url.startAccessingSecurityScopedResource()

                if let table = await vpinballManager.import(url: url) {
                    modelContext.insert(table)
                    try? modelContext.save()

                    tableListScrollToTableId = table.tableId
                } else {
                    handleShowError(message: "Unable to import table.")
                }

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
        if tables.isEmpty {
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
            handlePlay()
        case .rename:
            handleShowRename()
        case .changeArtwork:
            handleShowChangeArtwork()
        case .viewScript:
            handleViewScript()
        case .share:
            handleShare()
        case .reset:
            handleReset()
        case .delete:
            handleDelete()
        case .stopped:
            handleStopped()
        default:
            break
        }
    }

    func handleLowMemoryNotice(showSettings: Bool = false) {
        vpinballManager.saveValue(.standalone, "LowMemoryNotice", 0)

        if showSettings {
            handleShowSettings(section: "performance")
        }
    }

    func handleShowRename() {
        if let selectedTable = selectedTable {
            renameTableName = selectedTable.name
            showRename = true
            tableListScrollToTableId = nil
        }
    }

    func handleRename() {
        if let selectedTable = selectedTable {
            selectedTable.name = renameTableName
            selectedTable.update(context: modelContext)
            tableListScrollToTableId = selectedTable.tableId
        }
    }

    func handleShowChangeArtwork() {
        changeArtworkPhotoItem = nil
        showChangeArtwork = true
    }

    func handleShowChangeArtworkPhotoPicker() {
        showChangeArtworkPhotoPicker = true
    }

    func handleChangeArtworkPhotoItem() {
        if let selectedTable = selectedTable {
            if let item = changeArtworkPhotoItem {
                item.loadTransferable(type: Data.self) { result in
                    switch result {
                    case let .success(data?):
                        DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
                            if let image = UIImage(data: data) {
                                selectedTable.imageData = image.resizeWithAspectFit(newSize: CGSize(width: 1179,
                                                                                                    height: 2556))?.jpegData(compressionQuality: 0.75)
                            } else {
                                selectedTable.imageData = nil
                            }
                            selectedTable.update(context: modelContext)
                        }
                    default:
                        break
                    }
                }
            }
        }
    }

    func handleChangeArtworkReset() {
        if let selectedTable = selectedTable {
            DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
                selectedTable.imageData = nil
                selectedTable.update(context: modelContext)
            }
        }
    }

    func handleViewScript() {
        if let selectedTable = selectedTable {
            if selectedTable.hasScript() {
                showScript = true
            } else {
                Task {
                    if await vpinballManager.extractScript(table: selectedTable) == true {
                        selectedTable.update(context: modelContext)
                        showScript = true
                    } else {
                        handleShowError(message: "Unable to extract script")
                    }
                }
            }
        }
    }

    func handleShare() {
        if let selectedTable = selectedTable {
            Task {
                if let url = await vpinballManager.share(table: selectedTable) {
                    shareItems = [url]
                    showShare = true
                }
            }
        }
    }

    func handleReset() {
        if let selectedTable = selectedTable {
            try? FileManager.default.removeItem(at: selectedTable.iniURL)
            selectedTable.update(context: modelContext)
            DispatchQueue.main.asyncAfter(deadline: .now() + 0.25) {
                tableListScrollToTableId = selectedTable.tableId
            }
        }
    }

    func handleDelete() {
        if let selectedTable = selectedTable {
            try? FileManager.default.removeItem(at: selectedTable.baseURL)
            modelContext.delete(selectedTable)
        }
    }

    func handlePlay() {
        if vpinballManager.activeTable != nil {
            return
        }

        if let selectedTable = selectedTable {
            Task {
                if await vpinballManager.play(table: selectedTable) != true {
                    handleShowError(message: vpinballViewModel.scriptError ?? "Unable to load table")
                }
            }
        }
    }

    func handleStopped() {
        if let scriptError = vpinballViewModel.scriptError {
            handleShowError(message: scriptError)
        }
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
        .environmentObject(VPinballViewModel.shared)
}
