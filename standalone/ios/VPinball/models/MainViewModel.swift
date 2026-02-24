import PhotosUI
import SwiftUI

@MainActor
class MainViewModel: ObservableObject {
    static let shared = MainViewModel()

    enum ActionType {
        case play
        case stopped
        case rename
        case tableImage
        case viewScript
        case share
        case reset
        case delete
    }

    @Published var selectedTable: Table?

    @Published var showSettings = false

    @Published var confirmImportTableURL: URL?
    @Published var showConfirmImportTable = false

    @Published var importTableURL: URL? {
        didSet { handleImportTableURL() }
    }

    @Published var showImportTable = false

    @Published var tableListSearchText = ""

    @Published var shareItems: [Any] = []
    @Published var showShare = false

    @Published var renameTableName = ""
    @Published var showRenameTable = false

    @Published var showTableImage = false
    @Published var tableImagePhotoItem: PhotosPickerItem?
    @Published var showTableImagePhotoPicker = false

    @Published var showScript = false

    @Published var errorMessage = ""
    @Published var showError = false

    @Published var openURL: URL? {
        didSet { handleOpenURL() }
    }

    @Published var scrollToTable: Table?

    @Published var tableViewMode: TableViewMode = .grid {
        didSet { VPinballManager.shared.saveValue(.standalone, "TableViewMode", tableViewMode.rawValue) }
    }

    @Published var tableGridSize: TableGridSize = .medium {
        didSet { VPinballManager.shared.saveValue(.standalone, "TableGridSize", tableGridSize.rawValue) }
    }

    @Published var tableListSortOrder: SortOrder = .forward {
        didSet { VPinballManager.shared.saveValue(.standalone, "TableListSort", tableListSortOrder == .forward ? 1 : 0) }
    }

    func loadSettings() {
        let manager = VPinballManager.shared
        tableViewMode = TableViewMode(rawValue: manager.loadValue(.standalone,
                                                                  "TableViewMode",
                                                                  TableViewMode.grid.rawValue)) ?? .grid
        tableGridSize = TableGridSize(rawValue: manager.loadValue(.standalone,
                                                                  "TableGridSize",
                                                                  TableGridSize.medium.rawValue)) ?? .medium
        tableListSortOrder = manager.loadValue(.standalone, "TableListSort", 1) == 1 ? .forward : .reverse
    }

    func setAction(_ type: ActionType, table: Table? = nil) {
        UIApplication.shared.sendAction(#selector(UIResponder.resignFirstResponder),
                                        to: nil,
                                        from: nil,
                                        for: nil)

        selectedTable = table

        switch type {
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
            break
        }
    }

    func handleAppear() {
        loadSettings()

        Task {
            await TableManager.shared.refresh()
        }

        handleOpenURL()
    }

    func handleOpenURL() {
        if let url = openURL {
            openURL = nil
            handleShowConfirmImportTable(url: url)
        }
    }

    func handleImportTableURL() {
        handleShowConfirmImportTable(url: importTableURL)
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

    func handleImportTable(url: URL?) {
        if let url = url {
            Task {
                _ = await TableManager.shared.importTable(from: url)
            }
        }
    }

    func handleShowRenameTable() {
        if let selectedTable = selectedTable {
            renameTableName = selectedTable.name
            showRenameTable = true
            scrollToTable = nil
        }
    }

    func handleRenameTable() {
        if let selectedTable = selectedTable {
            Task {
                _ = await TableManager.shared.renameTable(table: selectedTable,
                                                          newName: renameTableName)
                scrollToTable = selectedTable
            }
        }
    }

    func handleShowTableImage() {
        tableImagePhotoItem = nil
        showTableImage = true
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
                                    await TableManager.shared.setTableImage(table: selectedTable,
                                                                            image: image)
                                }
                            } else {
                                Task {
                                    await TableManager.shared.setTableImage(table: selectedTable,
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
                    await TableManager.shared.setTableImage(table: selectedTable,
                                                            imagePath: "")
                }
            }
        }
    }

    func handleViewTableScript() {
        if let selectedTable = selectedTable {
            Task {
                let hasScript = await selectedTable.hasScriptFileAsync()
                if hasScript {
                    await MainActor.run {
                        showScript = true
                    }
                    return
                }

                if await TableManager.shared.extractTableScript(table: selectedTable) {
                    await MainActor.run {
                        showScript = true
                    }
                } else {
                    await MainActor.run {
                        handleShowError(message: "Unable to extract script")
                    }
                }
            }
        }
    }

    func handleShareTable() {
        if let selectedTable = selectedTable {
            Task {
                if let path = await TableManager.shared.exportTable(table: selectedTable) {
                    shareItems = [URL(fileURLWithPath: path)]
                    showShare = true
                }
            }
        }
    }

    func handleResetTable() {
        if let selectedTable = selectedTable {
            _ = TableManager.shared.resetTableIni(table: selectedTable)
        }
    }

    func handleDeleteTable() {
        if let selectedTable = selectedTable {
            Task {
                await TableManager.shared.deleteTable(table: selectedTable)
            }
        }
    }

    func handlePlayTable() {
        if VPinballModel.shared.activeTable != nil {
            return
        }

        if let selectedTable = selectedTable {
            Task {
                if await VPinballManager.shared.play(table: selectedTable) != true {
                    VPinballModel.shared.activeTable = nil
                    handleShowError(message: "Unable to load table")
                }
            }
        }
    }

    func handleShowError(message: String) {
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
            self.errorMessage = message
            self.showError = true
        }
    }
}
