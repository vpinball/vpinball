import SwiftUI

struct SettingsPerformanceView: View {
    @ObservedObject var settingsModel: SettingsModel

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
        }
        .onChange(of: settingsModel.maxTexDimensionIndex) {
            settingsModel.handleMaxTexDimension()
        }
    }
}

#Preview {
    List {
        SettingsPerformanceView(settingsModel: SettingsModel())
    }
}
