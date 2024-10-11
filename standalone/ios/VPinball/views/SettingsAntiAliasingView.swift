import SwiftUI

struct SettingsAntiAliasingView: View {
    @EnvironmentObject var vpinballViewModel: VPinballViewModel
    @EnvironmentObject var settingsModel: SettingsModel

    let vpinballManager = VPinballManager.shared

    var body: some View {
        Section("Anti-Aliasing") {
            VStack(alignment: .leading) {
                Text("MSAA Samples")
                HStack {
                    Slider(
                        value: Binding(
                            get: { Double(settingsModel.msaaSamplesIndex) },
                            set: { settingsModel.msaaSamplesIndex = Int($0.rounded()) }
                        ),
                        in: 0 ... Double(VPinballMSAASamples.all.count - 1),
                        step: 1
                    )
                    Spacer()
                    Text(VPinballMSAASamples.all[settingsModel.msaaSamplesIndex].name)
                        .foregroundStyle(Color.secondary)
                        .frame(minWidth: 90,
                               alignment: .trailing)
                }
            }

            VStack(alignment: .leading) {
                Text("Supersampling")
                HStack {
                    Slider(
                        value: Binding(
                            get: { Double(settingsModel.aaFactorIndex) },
                            set: { settingsModel.aaFactorIndex = Int($0.rounded()) }
                        ),
                        in: 0 ... Double(VPinballAAFactor.all.count - 1),
                        step: 1
                    )
                    Spacer()
                    Text(VPinballAAFactor.all[settingsModel.aaFactorIndex].name)
                        .foregroundStyle(Color.secondary)
                        .frame(minWidth: 90,
                               alignment: .trailing)
                }
            }

            Picker("Post-processed AA", selection: $settingsModel.fxaa) {
                ForEach(VPinballFXAA.all,
                        id: \.self)
                { fxaa in
                    Text(fxaa.name)
                        .tag(fxaa)
                }
            }

            Picker("Sharpen", selection: $settingsModel.sharpen) {
                ForEach(VPinballSharpen.all,
                        id: \.self)
                { sharpen in
                    Text(sharpen.name)
                        .tag(sharpen)
                }
            }

            Toggle(isOn: $settingsModel.scaleFXDMD) {
                Text("Enable ScaleFX for internal DMD")
            }
            .tint(Color.vpxRed)
        }
        .onChange(of: settingsModel.msaaSamplesIndex) {
            handleMSAASamples()
        }
        .onChange(of: settingsModel.aaFactorIndex) {
            handleAAFactor()
        }
        .onChange(of: settingsModel.fxaa) {
            handleFXAA()
        }
        .onChange(of: settingsModel.sharpen) {
            handleSharpen()
        }
        .onChange(of: settingsModel.scaleFXDMD) {
            handleScaleFXDMD()
        }
    }

    func handleMSAASamples() {
        vpinballManager.saveValue(.player, "MSAASamples", VPinballMSAASamples.all[settingsModel.msaaSamplesIndex].rawValue)
    }

    func handleAAFactor() {
        let aaFactor = VPinballAAFactor.all[settingsModel.aaFactorIndex].floatValue

        vpinballManager.saveValue(.player, "USEAA", aaFactor > 1.0)
        vpinballManager.saveValue(.player, "AAFactor", aaFactor)
    }

    func handleFXAA() {
        vpinballManager.saveValue(.player, "FXAA", settingsModel.fxaa.rawValue)
    }

    func handleSharpen() {
        vpinballManager.saveValue(.player, "Sharpen", settingsModel.sharpen.rawValue)
    }

    func handleScaleFXDMD() {
        vpinballManager.saveValue(.player, "ScaleFXDMD", settingsModel.scaleFXDMD)
    }
}

#Preview {
    List {
        SettingsView()
    }
    .environmentObject(VPinballViewModel.shared)
    .environmentObject(SettingsModel())
}
