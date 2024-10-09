import Foundation
import SwiftData
import SwiftUI

@main
struct VPinballApp: App {
    @State var showSplash = true

    init() {
        UISearchBar.appearance().overrideUserInterfaceStyle = .dark
    }

    var body: some Scene {
        WindowGroup {
            if showSplash {
                SplashView()
                    .onAppear {
                        handleAppear()
                    }
            } else {
                MainView()
            }
        }
        .environmentObject(VPinballViewModel.shared)
        .modelContainer(for: PinTable.self)
    }

    func handleAppear() {
        DispatchQueue.main.asyncAfter(deadline: .now() + 2) {
            withAnimation {
                showSplash = false
            }
        }
    }
}
