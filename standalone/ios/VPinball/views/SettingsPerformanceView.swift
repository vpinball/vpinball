import SwiftUI

struct SettingsPerformanceView: View {
    @ObservedObject var settingsModel: SettingsModel
    @ObservedObject var vpinballViewModel = VPinballViewModel.shared

    let vpinballManager = VPinballManager.shared

    var body: some View {
        Section("Performance") {
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
        .onChange(of: settingsModel.maxTexDimensionIndex) {
            handleMaxTexDimension()
        }
        .onChange(of: settingsModel.alphaRampAccuracy) {
            handleAlphaRampAccuracy()
        }
    }

    func handleMaxTexDimension() {
        vpinballManager.saveValue(.player, "MaxTexDimension", VPinballMaxTexDimension.all[settingsModel.maxTexDimensionIndex].rawValue)
    }

    func handleAlphaRampAccuracy() {
        vpinballManager.saveValue(.player, "AlphaRampAccuracy", settingsModel.alphaRampAccuracy)
    }
}

#Preview {
    List {
        SettingsPerformanceView(settingsModel: SettingsModel())
    }
}
