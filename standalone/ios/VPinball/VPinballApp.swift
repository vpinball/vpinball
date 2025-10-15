import Foundation
import SwiftUI

struct VPinballAppView: View {
    @ObservedObject var vpinballViewModel = VPinballViewModel.shared

    var body: some View {
        ZStack {
            if vpinballViewModel.showSplash {
                SplashView()
                    .onAppear {
                        handleAppear()
                    }
            } else {
                MainView()
                    .opacity(vpinballViewModel.showMainView ? 1 : 0)
            }
        }
        .onChange(of: vpinballViewModel.isPlaying) {
            vpinballViewModel.showMainView = !vpinballViewModel.isPlaying

            StatusBarManager.shared.setHidden(!vpinballViewModel.showMainView,
                                              animated: false)
        }
    }

    func handleAppear() {
        VPinballManager.shared.startup()
        vpinballViewModel.startSplashTimer()
    }
}
