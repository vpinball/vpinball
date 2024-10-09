import SwiftUI

struct SettingsBallRenderingView: View {
    @EnvironmentObject var vpinballViewModel: VPinballViewModel
    @EnvironmentObject var settingsModel: SettingsModel

    let vpinballManager = VPinballManager.shared

    var body: some View {
        Section("Ball Rendering") {
            Toggle(isOn: $settingsModel.ballTrail) {
                Text("Ball Trails")
            }
            .tint(Color.vpxRed)

            VStack(alignment: .leading) {
                Text("Strength")

                Slider(
                    value: Binding(
                        get: { Double(settingsModel.ballTrailStrength) },
                        set: { settingsModel.ballTrailStrength = Float($0) }
                    ),
                    in: 0 ... 2,
                    step: 0.001
                )
            }

            VStack(alignment: .leading) {
                Toggle(isOn: $settingsModel.ballAntiStretch) {
                    Text("Force Round Ball")
                }
                .tint(Color.vpxRed)

                Text("Compensate perspective stretch")
                    .font(.footnote)
                    .foregroundStyle(Color.secondary)
            }

            Toggle(isOn: $settingsModel.disableLightingForBalls) {
                Text("Disable Lighting")
            }
            .tint(Color.vpxRed)
        }
        .onChange(of: settingsModel.ballTrail) {
            handleBallTrail()
        }
        .onChange(of: settingsModel.ballTrailStrength) {
            handleBallTrailStrength()
        }
        .onChange(of: settingsModel.ballAntiStretch) {
            handleBallAntiStretch()
        }
        .onChange(of: settingsModel.disableLightingForBalls) {
            handleDisableLightingForBalls()
        }
    }

    func handleBallTrail() {
        vpinballManager.saveValue(.player, "BallTrail", settingsModel.ballTrail)
    }

    func handleBallTrailStrength() {
        vpinballManager.saveValue(.player, "BallTrailStrength", settingsModel.ballTrailStrength)
    }

    func handleBallAntiStretch() {
        vpinballManager.saveValue(.player, "BallAntiStretch", settingsModel.ballAntiStretch)
    }

    func handleDisableLightingForBalls() {
        vpinballManager.saveValue(.player, "DisableLightingForBalls", settingsModel.disableLightingForBalls)
    }
}

#Preview {
    List {
        SettingsView()
    }
    .environmentObject(VPinballViewModel.shared)
    .environmentObject(SettingsModel())
}
