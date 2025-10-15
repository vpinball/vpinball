import SwiftUI

class VPinballViewModel: ObservableObject {
    static let shared = VPinballViewModel()

    enum Action {
        case play
        case stopped
        case rename
        case tableImage
        case viewScript
        case share
        case reset
        case delete
        case showError
    }

    @Published var didSetAction: UUID?
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

    var action: Action?
    var table: Table?
    var scriptError: String?

    func setAction(action: Action, table: Table? = nil) {
        self.action = action
        self.table = table

        didSetAction = UUID()
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
        DispatchQueue.main.asyncAfter(deadline: .now() + 2) {
            withAnimation {
                self.showSplash = false
            }
        }
    }
}
