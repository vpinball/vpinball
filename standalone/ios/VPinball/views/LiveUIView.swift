import SwiftUI

struct LiveUIView: View {
    @EnvironmentObject var vpinballViewModel: VPinballViewModel

    @State private var isEditing: Bool = false

    let vpinballManager = VPinballManager.shared

    var body: some View {
        ZStack {
            Color.black.opacity(!isEditing ? 0.2 : 0)
                .ignoresSafeArea()

            ZStack {
                RoundedRectangle(cornerRadius: 10.0)
                    .fill(Color.black.opacity(0.5))
                    .opacity(!isEditing ? 1 : 0)

                VStack(spacing: 20) {
                    TouchDownInterceptingContainer {
                        HStack(alignment: .top) {
                            Button(action: handleResume) {
                                VStack {
                                    Rectangle()
                                        .fill(.ultraThinMaterial)
                                        .frame(width: 60,
                                               height: 60)
                                        .cornerRadius(10.0)
                                        .overlay {
                                            Image("noun-pinball-machine-17955")
                                                .resizable()
                                                .renderingMode(.template)
                                                .aspectRatio(contentMode: .fit)
                                                .padding(12)
                                                .tint(.white)
                                        }

                                    Text("Resume")
                                        .font(.footnote)
                                        .bold()
                                }
                                .tint(.white)
                            }

                            Button(action: handleOverlay) {
                                VStack {
                                    Rectangle()
                                        .fill(.ultraThinMaterial)
                                        .frame(width: 60,
                                               height: 60)
                                        .cornerRadius(10.0)
                                        .overlay {
                                            Image(systemName: vpinballViewModel.showTouchOverlay ? "square.stack.3d.down.forward.fill" : "square.stack.3d.down.forward")
                                                .resizable()
                                                .aspectRatio(contentMode: .fit)
                                                .padding(10)
                                                .padding(.top, 2)
                                                .tint(.white)
                                        }

                                    Text("Overlay")
                                        .font(.footnote)
                                        .bold()
                                }
                                .tint(.white)
                            }

                            Button(action: handleFPS) {
                                VStack {
                                    Rectangle()
                                        .fill(.ultraThinMaterial)
                                        .frame(width: 60,
                                               height: 60)
                                        .cornerRadius(10.0)
                                        .overlay {
                                            Image(systemName: "gauge.open.with.lines.needle.33percent")
                                                .resizable()
                                                .aspectRatio(contentMode: .fit)
                                                .padding(14)
                                                .tint(.white)
                                        }

                                    Text("FPS")
                                        .font(.footnote)
                                        .bold()
                                }
                                .tint(.white)
                            }

                            Button(action: handleArtwork) {
                                VStack {
                                    Rectangle()
                                        .fill(.ultraThinMaterial)
                                        .frame(width: 60,
                                               height: 60)
                                        .cornerRadius(10.0)
                                        .overlay {
                                            Image(systemName: "photo.on.rectangle")
                                                .resizable()
                                                .aspectRatio(contentMode: .fit)
                                                .padding(14)
                                                .tint(.white)
                                        }

                                    Text("Artwork")
                                        .font(.footnote)
                                        .bold()
                                }
                                .tint(.white)
                            }

                            Button(action: handleQuit) {
                                VStack {
                                    Rectangle()
                                        .fill(.ultraThinMaterial)
                                        .frame(width: 60,
                                               height: 60)
                                        .cornerRadius(10.0)
                                        .overlay {
                                            Image(systemName: "house")
                                                .resizable()
                                                .aspectRatio(contentMode: .fit)
                                                .padding(14)
                                                .tint(.white)
                                        }

                                    Text("Quit")
                                        .font(.footnote)
                                        .bold()
                                }
                                .tint(.white)
                            }
                        }
                    }
                    .opacity(!isEditing ? 1 : 0)
                    .padding(.top, 20)

                    TabView {
                        LiveUITableOptions(isEditing: $isEditing)
                        LiveUIPointOfView(isEditing: $isEditing)
                    }
                    .tabViewStyle(.page(indexDisplayMode: isEditing ? .never : .automatic))
                }
            }
            .padding(20)
        }

        if vpinballViewModel.showHUD {
            HUDOverlayView(type: vpinballViewModel.hudType!)
        }

        ArtworkSnapshotView()
    }

    func handleResume() {
        vpinballViewModel.showLiveUI = false

        DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
            vpinballManager.setPlayState(enable: true)
        }
    }

    func handleOverlay() {
        vpinballViewModel.showTouchOverlay.toggle()
    }

    func handleFPS() {
        vpinballManager.toggleFPS()
    }

    func handleArtwork() {
        if let image = vpinballManager.snapshot(force: true) {
            vpinballViewModel.artworkImage = image
        }
    }

    func handleQuit() {
        vpinballViewModel.showLiveUI = false

        DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
            vpinballManager.setPlayState(enable: true)

            DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
                vpinballManager.stop()
            }
        }
    }
}

#Preview {
    ZStack {
        Color.gray.ignoresSafeArea()

        LiveUIView()
    }
    .environmentObject(VPinballViewModel.shared)
}
