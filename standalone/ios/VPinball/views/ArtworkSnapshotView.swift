import AudioToolbox
import SwiftUI

struct ArtworkSnapshotView: View {
    @EnvironmentObject var vpinballViewModel: VPinballViewModel

    @State private var showFlash = false
    @State private var opacity: Double = 1.0

    let systemSoundID: SystemSoundID = 1108

    var body: some View {
        ZStack {
            if let image = vpinballViewModel.artworkImage {
                if showFlash {
                    Color.white
                        .ignoresSafeArea()
                } else {
                    Image(uiImage: image)
                        .resizable()
                        .opacity(opacity)
                        .ignoresSafeArea()
                }
            }
        }
        .onChange(of: vpinballViewModel.artworkImage) {
            handleArtwork()
        }
    }

    func handleArtwork() {
        if vpinballViewModel.artworkImage != nil {
            AudioServicesPlaySystemSound(systemSoundID)

            showFlash = true
            opacity = 1.0

            DispatchQueue.main.asyncAfter(deadline: .now() + 0.25) {
                showFlash = false

                DispatchQueue.main.asyncAfter(deadline: .now() + 1.5) {
                    withAnimation {
                        opacity = 0
                    }

                    DispatchQueue.main.asyncAfter(deadline: .now() + 1) {
                        vpinballViewModel.artworkImage = nil
                    }
                }
            }
        }
    }
}

#Preview {
    let viewModel = VPinballViewModel.shared

    ZStack {
        Color.blue.ignoresSafeArea()

        ArtworkSnapshotView()
            .onAppear {
                DispatchQueue.main.asyncAfter(deadline: .now() + 1) {
                    viewModel.artworkImage = UIImage(named: "table-placeholder")
                }
            }
    }
    .environmentObject(viewModel)
}
