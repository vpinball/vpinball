import SwiftUI

enum FormattedActiveSliderFormat {
    case decimal
    case degrees
    case percentage
    case scaledPercentage
    case intPercentage
    case cm
    case vpuCM

    func formatValue(_ value: Float) -> String {
        switch self {
        case .decimal:
            return String(format: "%.1f", value)
        case .degrees:
            return String(format: "%.1f \u{00B0}", value)
        case .percentage:
            return String(format: "%.1f %%", value)
        case .scaledPercentage:
            return String(format: "%.1f %%", value * 100.0)
        case .intPercentage:
            return String(format: "%d %%", CInt(value))
        case .cm:
            return String(format: "%.1f cm", value)
        case .vpuCM:
            return String(format: "%.1f cm", VPinballUnitConverter.vpuToCM(value))
        }
    }
}

struct FormattedActiveSlider: View {
    var label: String
    @Binding var value: Float
    var minValue: Float
    var maxValue: Float
    var step: Float
    var format: FormattedActiveSliderFormat
    @Binding var activeSlider: String?

    @State private var isEditing = false

    var body: some View {
        VStack(alignment: .leading,
               spacing: 4)
        {
            Text(label)
                .bold()
                .foregroundStyle(Color.white)

            HStack {
                Slider(
                    value: Binding(
                        get: { Double(value) },
                        set: { newValue in
                            let roundedValue = round(newValue * 1000) / 1000
                            if abs(roundedValue - Double(maxValue)) < Double(step) {
                                value = maxValue
                            } else {
                                value = Float(roundedValue)
                            }
                        }
                    ),
                    in: Double(minValue) ... Double(maxValue),
                    step: Double(step),
                    onEditingChanged: { editing in
                        isEditing = editing
                        withAnimation(.easeInOut(duration: 0.2)) {
                            activeSlider = editing ? label : nil
                        }
                    }
                )

                Text(format.formatValue(value))
                    .foregroundStyle(Color.white)
                    .frame(width: 75,
                           alignment: .trailing)
            }
        }
    }
}
