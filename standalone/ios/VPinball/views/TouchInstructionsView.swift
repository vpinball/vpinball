import SwiftUI

struct TouchInstructionsView: View {
    @EnvironmentObject var vpinballViewModel: VPinballViewModel

    @State private var showAll = false
    @State private var isDontShowAgain = false
    @State private var scaleEffects: [[CGFloat]] = Array(repeating: [CGFloat](repeating: 1.0,
                                                                              count: 11),
                                                         count: 7)

    let groupColors: [Color] = [.red,
                                .green,
                                .blue,
                                .orange,
                                .purple,
                                .yellow,
                                .pink]

    let scaleDuration: Double = 0.25
    let holdDuration: Double = 1

    let vpinballManager = VPinballManager.shared

    var body: some View {
        GeometryReader { geometry in
            ZStack {
                ForEach(0 ..< VPinballTouchAreas.count, id: \.self) { groupIndex in
                    ForEach(0 ..< VPinballTouchAreas[groupIndex].count, id: \.self) { areaIndex in
                        let region = VPinballTouchAreas[groupIndex][areaIndex]
                        let regionWidth = geometry.size.width * (region.right - region.left) / 100
                        let regionHeight = geometry.size.height * (region.bottom - region.top) / 100
                        let x = geometry.size.width * region.left / 100 + regionWidth / 2
                        let y = geometry.size.height * region.top / 100 + regionHeight / 2

                        ZStack {
                            if scaleEffects[groupIndex][areaIndex] > 1 || showAll {
                                Rectangle()
                                    .fill(groupColors[groupIndex].opacity(0.1))
                                    .frame(width: regionWidth,
                                           height: regionHeight)
                                    .position(x: x,
                                              y: y)

                                Rectangle()
                                    .fill(.ultraThinMaterial.opacity(0.9))
                                    .border(Color.white.opacity(0.5),
                                            width: 1)
                                    .frame(width: regionWidth,
                                           height: regionHeight)
                                    .position(x: x,
                                              y: y)
                            }

                            VStack {
                                if scaleEffects[groupIndex][areaIndex] > 1 || showAll {
                                    Text(region.label)
                                        .font(.caption)
                                        .bold()
                                        .multilineTextAlignment(.center)
                                        .foregroundStyle(Color.white)
                                        .padding(5)
                                }
                            }
                            .frame(width: regionWidth, height: regionHeight)
                            .scaleEffect(showAll ? 1.5 : scaleEffects[groupIndex][areaIndex])
                            .position(x: x,
                                      y: y)
                        }
                    }
                }

                if vpinballViewModel.showHUD {
                    HUDOverlayView(type: vpinballViewModel.hudType!)
                        .onTapGesture {
                            handleLaunch()
                        }
                }
            }
        }
        .ignoresSafeArea()
        .onAppear {
            handleAppear()
        }
    }

    func handleAppear() {
        vpinballManager.sdlUIWindow?.isUserInteractionEnabled = false

        DispatchQueue.main.asyncAfter(deadline: .now() + 2) {
            let totalGroupDelay = Double(VPinballTouchAreas.count) * (scaleDuration + holdDuration + scaleDuration + scaleDuration)

            for groupIndex in 0 ..< VPinballTouchAreas.count {
                let groupAnimationDelay = Double(groupIndex) * (scaleDuration + holdDuration + scaleDuration + scaleDuration)

                DispatchQueue.main.asyncAfter(deadline: .now() + groupAnimationDelay) {
                    for rectIndex in 0 ..< VPinballTouchAreas[groupIndex].count {
                        withAnimation(Animation.easeInOut(duration: scaleDuration)) {
                            scaleEffects[groupIndex][rectIndex] = 1.5
                        }

                        DispatchQueue.main.asyncAfter(deadline: .now() + scaleDuration + holdDuration) {
                            withAnimation(Animation.easeInOut(duration: scaleDuration)) {
                                scaleEffects[groupIndex][rectIndex] = 1
                            }
                        }
                    }
                }
            }

            DispatchQueue.main.asyncAfter(deadline: .now() + totalGroupDelay) {
                withAnimation {
                    showAll = true
                }

                DispatchQueue.main.asyncAfter(deadline: .now() + 1.5) {
                    vpinballManager.sdlUIWindow?.isUserInteractionEnabled = true

                    withAnimation {
                        vpinballViewModel.showStatusHUD(icon: "arcade-button-svgrepo-com",
                                                        title: "Launch Ball !",
                                                        accessoryView: AnyView(
                                                            TouchInstructionsToggle(isOn: $isDontShowAgain)
                                                        ))
                    }
                }
            }
        }
    }

    func handleLaunch() {
        withAnimation {
            vpinballManager.saveValue(.standalone,
                                      "TouchInstructions",
                                      !isDontShowAgain)

            showAll = false
            vpinballViewModel.hideHUD()
            vpinballViewModel.showTouchInstructions = false
        }
    }
}

struct TouchInstructionsToggle: View {
    @Binding var isOn: Bool

    var body: some View {
        Button(action: handleToggle) {
            HStack {
                Image(systemName: isOn ? "checkmark.square.fill" : "square")
                    .font(.subheadline)

                Text("Don't show again")
                    .foregroundColor(.white)
                    .font(.subheadline)
            }
        }
        .padding(.top, 10)
    }

    func handleToggle() {
        isOn.toggle()
    }
}

#Preview {
    ZStack {
        Color.gray.ignoresSafeArea()

        TouchInstructionsView()
    }
    .environmentObject(VPinballViewModel.shared)
}
