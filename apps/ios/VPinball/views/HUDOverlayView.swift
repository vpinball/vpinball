import SwiftUI

struct HUDOverlayView: View {
    @ObservedObject var vpinballModel = VPinballModel.shared

    var body: some View {
        ZStack {
            Color(.systemBackground).opacity(0.25)
                .ignoresSafeArea()

            VStack {
                Spacer()

                VStack(spacing: 18) {
                    Text(vpinballModel.hudTitle ?? " ")
                        .multilineTextAlignment(.center)
                        .font(.headline)
                        .bold()
                        .foregroundStyle(.primary)

                    ProgressView(value: Double(vpinballModel.hudProgress),
                                 total: 100)
                        .progressViewStyle(.linear)
                        .tint(Color.vpxDarkYellow)

                    Text(vpinballModel.hudStatus ?? " ")
                        .font(.caption.weight(.semibold))
                        .foregroundStyle(.secondary)
                        .contentTransition(.opacity)
                }
                .padding(22)
                .glassPanel(cornerRadius: 26)
                .environment(\.colorScheme, .dark)
                .padding(.horizontal, 20)
                .padding(.bottom, 30)
            }
        }
    }
}

#Preview {
    HUDOverlayView()
}
