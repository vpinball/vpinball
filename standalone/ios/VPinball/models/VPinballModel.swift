import SwiftUI

@MainActor
class VPinballModel: ObservableObject {
    static let shared = VPinballModel()

    @Published var tables: [Table] = []
    @Published var activeTable: Table?
    @Published var isPlaying = false
    @Published var webServerURL: String?
    @Published var hudTitle: String?
    @Published var hudProgress: Int = 0
    @Published var hudStatus: String?
    @Published var showSplash = true
    @Published var showMainView = true
    @Published var showHUD = false

    func showHUD(title: String, status: String) {
        hudTitle = title
        hudProgress = 0
        hudStatus = status
        showHUD = true
    }

    func updateHUD(progress: Int, status: String) {
        hudProgress = progress
        hudStatus = status
    }

    func updateHUD(progress: Int) {
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
