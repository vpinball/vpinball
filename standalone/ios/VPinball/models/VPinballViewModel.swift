import SwiftUI

@MainActor
class VPinballViewModel: ObservableObject {
    static let shared = VPinballViewModel()

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

    struct Action: Equatable {
        let id = UUID()
        let type: ActionType
        let table: Table?
        static func == (lhs: Action, rhs: Action) -> Bool {
            lhs.id == rhs.id
        }
    }

    @Published var action: Action?
    @Published var showHUD = false
    @Published var isPlaying = false
    @Published var tableImage: UIImage?
    @Published var webServerURL: String?
    @Published var scrollToTable: Table?
    @Published var hudTitle: String?
    @Published var hudProgress: Int = 0
    @Published var hudStatus: String?
    @Published var showSplash = true
    @Published var showMainView = true
    @Published var openURL: URL?

    @Published var tableViewMode: TableViewMode = .grid {
        didSet { VPinballManager.shared.saveValue(.standalone, "TableViewMode", tableViewMode.rawValue) }
    }

    @Published var tableGridSize: TableGridSize = .medium {
        didSet { VPinballManager.shared.saveValue(.standalone, "TableGridSize", tableGridSize.rawValue) }
    }

    @Published var tableListSortOrder: SortOrder = .forward {
        didSet { VPinballManager.shared.saveValue(.standalone, "TableListSort", tableListSortOrder == .forward ? 1 : 0) }
    }

    @Published var alertError: String?

    func setAction(_ type: ActionType, table: Table? = nil) {
        action = Action(type: type, table: table)
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

    func showProgressHUD(title: String, status: String) {
        hudTitle = title
        hudProgress = 0
        hudStatus = status
        showHUD = true
    }

    func updateProgressHUD(progress: Int, status: String) {
        hudProgress = progress
        hudStatus = status
    }

    func updateProgressHUD(progress: Int) {
        hudProgress = progress
    }

    func hideHUD() {
        showHUD = false
    }

    func startSplashTimer() {
        Task { @MainActor in
            try? await Task.sleep(nanoseconds: 2_000_000_000)
            withAnimation {
                self.showSplash = false
            }
        }
    }
}
