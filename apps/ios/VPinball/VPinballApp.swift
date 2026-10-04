import Foundation
import SwiftUI

struct VPinballAppView: View {
    @ObservedObject var vpinballModel = VPinballModel.shared

    var body: some View {
        ZStack {
            if vpinballModel.showSplash {
                SplashView()
                    .onAppear {
                        handleAppear()
                    }
            } else {
                MainView()
                    .opacity(vpinballModel.showMainView ? 1 : 0)
            }
        }
        .onChange(of: vpinballModel.isPlaying) {
            vpinballModel.showMainView = !vpinballModel.isPlaying

            StatusBarManager.shared.setHidden(!vpinballModel.showMainView,
                                              animated: false)
        }
    }

    func handleAppear() {
        VPinballManager.shared.startup()
        vpinballModel.startSplashTimer()
    }
}
