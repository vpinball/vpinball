import SwiftUI

struct SettingsEnvironmentLightingView: View {
    @EnvironmentObject var vpinballViewModel: VPinballViewModel
    @EnvironmentObject var settingsModel: SettingsModel

    let vpinballManager = VPinballManager.shared

    var body: some View {
        Section("Environment Lighting") {
            Toggle(isOn: $settingsModel.overrideEmissionScale) {
                Text("Override Table Global Lighting")
            }
            .tint(Color.vpxRed)

            VStack(alignment: .leading) {
                Text("Night/Day")
                    .opacity(settingsModel.overrideEmissionScale ? 1 : 0.5)

                Slider(
                    value: Binding(
                        get: { Double(settingsModel.nightDay) },
                        set: { settingsModel.nightDay = Int($0) }
                    ),
                    in: 0 ... 100,
                    step: 1
                )
            }
            .disabled(!settingsModel.overrideEmissionScale)
        }
        .onChange(of: settingsModel.overrideEmissionScale) {
            handleOverrideEmissionScale()
        }
        .onChange(of: settingsModel.nightDay) {
            handleNightDay()
        }
    }

    func handleOverrideEmissionScale() {
        vpinballManager.saveValue(.tableOverride, "OverrideEmissionScale", settingsModel.overrideEmissionScale)
    }

    func handleNightDay() {
        vpinballManager.saveValue(.player, "EmissionScale", Float(settingsModel.nightDay) / 100)
    }
}

#Preview {
    List {
        SettingsView()
    }
    .environmentObject(VPinballViewModel.shared)
    .environmentObject(SettingsModel())
}
