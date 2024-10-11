import SwiftUI

struct OverlayView: View {
    @EnvironmentObject var vpinballViewModel: VPinballViewModel

    var body: some View {
        ZStack {
            if vpinballViewModel.liveUIOverride {
                if vpinballViewModel.showLiveUI {
                    if vpinballViewModel.showTouchOverlay {
                        TouchOverlayView()
                    }
                    LiveUIView()
                } else if vpinballViewModel.showTouchInstructions {
                    TouchInstructionsView()
                } else if vpinballViewModel.showTouchOverlay {
                    TouchOverlayView()
                }
            }
        }
    }
}

#Preview {
    let vpinballViewModel = VPinballViewModel.shared

    ZStack {
        Color.gray.ignoresSafeArea()

        OverlayView()
            .onAppear {
                vpinballViewModel.liveUIOverride = true
                vpinballViewModel.showTouchInstructions = true
                vpinballViewModel.showTouchOverlay = true
                vpinballViewModel.showLiveUI = true
            }
    }
    .environmentObject(vpinballViewModel)
}
