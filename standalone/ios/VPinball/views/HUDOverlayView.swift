import SwiftUI

struct HUDOverlayView: View {
    @ObservedObject var vpinballModel = VPinballModel.shared

    var body: some View {
        ZStack {
            Color.black.opacity(0.2)
                .ignoresSafeArea()

            VStack {
                Spacer()

                VStack(spacing: 20) {
                    Text(vpinballModel.hudTitle ?? " ")
                        .multilineTextAlignment(.center)
                        .font(.headline)
                        .bold()
                        .foregroundStyle(Color.white)

                    ProgressView(value: Double(vpinballModel.hudProgress),
                                 total: 100)
                        .progressViewStyle(.linear)
                        .tint(Color.vpxDarkYellow)
                        .background(Color.black)
                        .cornerRadius(2)

                    Text(vpinballModel.hudStatus ?? " ")
                        .font(.caption)
                        .bold()
                        .foregroundStyle(Color.white)
                }
                .padding()
                .background(.ultraThinMaterial,
                            in: RoundedRectangle(cornerRadius: 10.0))
                .padding(.horizontal, 20)
                .padding(.bottom, 30)
            }
        }
    }
}

#Preview {
    HUDOverlayView()
}
