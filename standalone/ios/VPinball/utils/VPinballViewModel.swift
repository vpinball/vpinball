import SwiftUI

class VPinballViewModel: ObservableObject {
    static let shared = VPinballViewModel()

    enum Action {
        case play
        case stopped
        case rename
        case changeArtwork
        case viewScript
        case share
        case reset
        case delete
    }

    @Published var didSetAction: UUID?
    @Published var hudType: HUDOverlayType?
    @Published var showHUD = false
    @Published var liveUIOverride = false
    @Published var showTouchInstructions = false
    @Published var showTouchOverlay = false
    @Published var showLowMemoryNotice = false
    @Published var showLiveUI = false
    @Published var artworkImage: UIImage?
    @Published var webServerURL: String?
    @Published var didReceiveMemoryWarning = false

    var action: Action?
    var table: PinTable?
    var scriptError: String?
    var hudTitle: String?
    var hudStatus: String?

    func setAction(action: Action, table: PinTable? = nil) {
        self.action = action
        self.table = table

        didSetAction = UUID()
    }

    func showStatusHUD(icon: String, title: String, accessoryView: AnyView? = nil) {
        hudType = .status(icon: icon,
                          title: title,
                          accessoryView: accessoryView)

        showHUD = true
    }

    func showProgressHUD(title: String, status: String) {
        didReceiveMemoryWarning = false

        hudTitle = title
        hudStatus = status

        hudType = .progress(title: title,
                            progress: 0,
                            status: status)

        showHUD = true
    }

    func updateProgressHUD(progress: Int, status: String) {
        hudStatus = status

        hudType = .progress(title: hudTitle,
                            progress: progress,
                            status: status)
    }

    func updateProgressHUD(progress: Int) {
        hudType = .progress(title: hudTitle,
                            progress: progress,
                            status: hudStatus)
    }

    func hideHUD() {
        showHUD = false
    }
}
