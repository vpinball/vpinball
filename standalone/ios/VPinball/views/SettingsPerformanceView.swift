import SwiftUI

struct SettingsPerformanceView: View {
    @EnvironmentObject var vpinballViewModel: VPinballViewModel
    @EnvironmentObject var settingsModel: SettingsModel

    let vpinballManager = VPinballManager.shared

    var body: some View {
        Section("Performance") {
            Picker("Max Ambient Occlusion", selection: $settingsModel.maxAO) {
                ForEach(VPinballAO.all,
                        id: \.self)
                { ao in
                    Text(ao.name)
                        .tag(ao)
                }
            }

            VStack(alignment: .leading) {
                Text("Max Reflection Mode")

                Picker("", selection: $settingsModel.maxReflectionMode) {
                    ForEach(VPinballReflectionMode.all,
                            id: \.self)
                    { reflectionMode in
                        Text(reflectionMode.name)
                            .tag(reflectionMode)
                    }
                }
            }

            VStack(alignment: .leading) {
                Text("Max Texture Dimensions")
                HStack {
                    Slider(
                        value: Binding(
                            get: { Double(settingsModel.maxTexDimensionIndex) },
                            set: { settingsModel.maxTexDimensionIndex = Int($0.rounded()) }
                        ),
                        in: 0 ... Double(VPinballMaxTexDimension.all.count - 1),
                        step: 1
                    )
                    Spacer()
                    Text(VPinballMaxTexDimension.all[settingsModel.maxTexDimensionIndex].name)
                        .foregroundStyle(Color.secondary)
                        .frame(minWidth: 90,
                               alignment: .trailing)
                }
                Text("Reduce this value if you experience crashes while loading tables.")
                    .font(.footnote)
                    .foregroundStyle(Color.secondary)
            }

            Toggle(isOn: $settingsModel.forceAniso) {
                Text("Force Anisotropic Texture Filtering")
            }
            .tint(Color.vpxRed)

            Toggle(isOn: $settingsModel.forceBloomOff) {
                Text("Force Bloom Filter Off")
            }
            .tint(Color.vpxRed)

            Toggle(isOn: $settingsModel.forceMotionBlurOff) {
                Text("Disable Ball Motion Blur")
            }
            .tint(Color.vpxRed)

            VStack(alignment: .leading) {
                Text("Elements Detail Level")

                Slider(
                    value: Binding(
                        get: { Double(settingsModel.alphaRampAccuracy) },
                        set: { settingsModel.alphaRampAccuracy = Int($0.rounded()) }
                    ),
                    in: 0 ... 10,
                    step: 1
                )

                Text("Decrease to increase performance")
                    .font(.footnote)
                    .foregroundStyle(Color.secondary)
            }
        }
        .onChange(of: settingsModel.maxAO) {
            handleMaxAO()
        }
        .onChange(of: settingsModel.maxReflectionMode) {
            handleMaxReflectionMode()
        }
        .onChange(of: settingsModel.maxTexDimensionIndex) {
            handleMaxTexDimension()
        }
        .onChange(of: settingsModel.forceAniso) {
            handleForceAniso()
        }
        .onChange(of: settingsModel.forceBloomOff) {
            handleForceBloomOff()
        }
        .onChange(of: settingsModel.forceMotionBlurOff) {
            handleForceMotionBlurOff()
        }
        .onChange(of: settingsModel.alphaRampAccuracy) {
            handleAlphaRampAccuracy()
        }
    }

    func handleMaxAO() {
        vpinballManager.saveValue(.player, "DisableAO", settingsModel.maxAO == .aoDisable)
        vpinballManager.saveValue(.player, "DynamicAO", settingsModel.maxAO == .aoDynamic)
    }

    func handleMaxReflectionMode() {
        vpinballManager.saveValue(.player, "PFReflection", settingsModel.maxReflectionMode.rawValue)
    }

    func handleMaxTexDimension() {
        vpinballManager.saveValue(.player, "MaxTexDimension", VPinballMaxTexDimension.all[settingsModel.maxTexDimensionIndex].rawValue)
    }

    func handleForceAniso() {
        vpinballManager.saveValue(.player, "ForceAnisotropicFiltering", settingsModel.forceAniso)
    }

    func handleForceBloomOff() {
        vpinballManager.saveValue(.player, "ForceBloomOff", settingsModel.forceBloomOff)
    }

    func handleForceMotionBlurOff() {
        vpinballManager.saveValue(.player, "ForceMotionBlurOff", settingsModel.forceMotionBlurOff)
    }

    func handleAlphaRampAccuracy() {
        vpinballManager.saveValue(.player, "AlphaRampAccuracy", settingsModel.alphaRampAccuracy)
    }
}

#Preview {
    List {
        SettingsPerformanceView()
    }
    .environmentObject(VPinballViewModel.shared)
    .environmentObject(SettingsModel())
}
