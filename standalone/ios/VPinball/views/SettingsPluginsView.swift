import SwiftUI

struct SettingsPluginsView: View {
    @EnvironmentObject var vpinballViewModel: VPinballViewModel
    @EnvironmentObject var settingsModel: SettingsModel

    let vpinballManager = VPinballManager.shared

    var body: some View {
        Section {
            Toggle(isOn: $settingsModel.pluginAlphaDMD) {
                Text("AlphaDMD")
            }
            .tint(Color.vpxRed)

            Toggle(isOn: $settingsModel.pluginB2S) {
                Text("B2S")
            }
            .tint(Color.vpxRed)

            Toggle(isOn: $settingsModel.pluginDMDUtil) {
                Text("DMDUtil")
            }
            .tint(Color.vpxRed)

            Toggle(isOn: $settingsModel.pluginFlexDMD) {
                Text("FlexDMD")
            }
            .tint(Color.vpxRed)

            Toggle(isOn: $settingsModel.pluginPinMAME) {
                Text("PinMAME")
            }
            .tint(Color.vpxRed)

            Toggle(isOn: $settingsModel.pluginPUP) {
                Text("PUP")
            }
            .tint(Color.vpxRed)

            Toggle(isOn: $settingsModel.pluginRemoteControl) {
                Text("Remote Control")
            }
            .tint(Color.vpxRed)

            Toggle(isOn: $settingsModel.pluginScoreView) {
                Text("ScoreView")
            }
            .tint(Color.vpxRed)

            Toggle(isOn: $settingsModel.pluginSerum) {
                Text("Serum")
            }
            .tint(Color.vpxRed)
        }
        header: {
            Text("Plugins (Experimental)")
        }
        .onChange(of: settingsModel.pluginAlphaDMD) {
            handlePluginAlphaDMD()
        }
        .onChange(of: settingsModel.pluginB2S) {
            handlePluginB2S()
        }
        .onChange(of: settingsModel.pluginDMDUtil) {
            handlePluginDMDUtil()
        }
        .onChange(of: settingsModel.pluginFlexDMD) {
            handlePluginFlexDMD()
        }
        .onChange(of: settingsModel.pluginPinMAME) {
            handlePluginPinMAME()
        }
        .onChange(of: settingsModel.pluginPUP) {
            handlePluginPUP()
        }
        .onChange(of: settingsModel.pluginRemoteControl) {
            handlePluginRemoteControl()
        }
        .onChange(of: settingsModel.pluginScoreView) {
            handlePluginScoreView()
        }
        .onChange(of: settingsModel.pluginSerum) {
            handlePluginSerum()
        }
    }

    func handlePluginAlphaDMD() {
        vpinballManager.saveValue(.pluginAlphaDMD, "Enable", settingsModel.pluginAlphaDMD)
    }

    func handlePluginB2S() {
        vpinballManager.saveValue(.pluginB2S, "Enable", settingsModel.pluginB2S)
    }

    func handlePluginDMDUtil() {
        vpinballManager.saveValue(.pluginDMDUtil, "Enable", settingsModel.pluginDMDUtil)
    }

    func handlePluginFlexDMD() {
        vpinballManager.saveValue(.pluginFlexDMD, "Enable", settingsModel.pluginFlexDMD)
    }

    func handlePluginPinMAME() {
        vpinballManager.saveValue(.pluginPinMAME, "Enable", settingsModel.pluginPinMAME)
    }

    func handlePluginPUP() {
        vpinballManager.saveValue(.pluginPUP, "Enable", settingsModel.pluginPUP)
    }

    func handlePluginRemoteControl() {
        vpinballManager.saveValue(.pluginRemoteControl, "Enable", settingsModel.pluginRemoteControl)
    }

    func handlePluginScoreView() {
        vpinballManager.saveValue(.pluginScoreView, "Enable", settingsModel.pluginScoreView)
    }

    func handlePluginSerum() {
        vpinballManager.saveValue(.pluginSerum, "Enable", settingsModel.pluginSerum)
    }
}

#Preview {
    List {
        SettingsView()
    }
    .environmentObject(VPinballViewModel.shared)
    .environmentObject(SettingsModel())
}
