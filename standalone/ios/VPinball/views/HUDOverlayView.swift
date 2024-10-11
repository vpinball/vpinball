import SwiftUI

enum HUDOverlayType {
    case status(icon: String?, title: String?, accessoryView: AnyView? = nil)
    case progress(title: String?, progress: Int, status: String?)
}

struct HUDOverlayView: View {
    @EnvironmentObject var vpinballViewModel: VPinballViewModel

    @State var isToggled: Bool = false

    var type: HUDOverlayType

    var body: some View {
        ZStack {
            Color.black.opacity(0.2)
                .ignoresSafeArea()

            switch type {
            case let .status(icon, title, accessoryView):
                VStack(spacing: 15) {
                    if let icon = icon {
                        Image(icon)
                            .resizable()
                            .aspectRatio(contentMode: .fit)
                            .frame(width: 50,
                                   height: 50)
                            .gradientEffect(icon: icon,
                                            size: CGSize(width: 50,
                                                         height: 50),
                                            contentMode: .fit)
                    }

                    Text(title ?? "")
                        .multilineTextAlignment(.center)
                        .font(.body)
                        .bold()
                        .foregroundStyle(Color.white)

                    if let accessoryView = accessoryView {
                        accessoryView
                            .padding(.top, 10)
                    }
                }
                .padding(20)
                .background(Color.black.opacity(0.9),
                            in: RoundedRectangle(cornerRadius: 10.0))

            case let .progress(title, progress, status):
                VStack {
                    Spacer()

                    VStack(spacing: 20) {
                        Text(title ?? " ")
                            .multilineTextAlignment(.center)
                            .font(.headline)
                            .bold()
                            .foregroundStyle(Color.white)

                        ProgressView(value: Double(progress),
                                     total: 100)
                            .progressViewStyle(.linear)
                            .tint(!vpinballViewModel.didReceiveMemoryWarning ? Color.vpxDarkYellow : Color.vpxRed)
                            .background(Color.black)
                            .cornerRadius(2)

                        if !vpinballViewModel.didReceiveMemoryWarning {
                            Text(status ?? " ")
                                .font(.caption)
                                .bold()
                                .foregroundStyle(Color.white)
                        } else {
                            Text("Low Memory Warning!")
                                .font(.caption)
                                .bold()
                                .foregroundStyle(Color.white)
                                .blinkEffect(maxLoop: 2)
                        }
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
}

#Preview {
    Group {
        HUDOverlayView(type: .status(icon: "arcade-button-svgrepo-com",
                                     title: "Launch Ball",
                                     accessoryView: AnyView(
                                         Button(action: {}) {
                                             HStack {
                                                 Image(systemName: "square")
                                                     .font(.subheadline)

                                                 Text("Don't show again")
                                                     .foregroundColor(.white)
                                                     .font(.subheadline)
                                             }
                                         }
                                     )))

        HUDOverlayView(type: .status(icon: "noun-pinball-3169564",
                                     title: "Ball Save"))

        HUDOverlayView(type: .progress(title: "Title",
                                       progress: 60,
                                       status: "Status"))
    }
    .environmentObject(VPinballViewModel.shared)
}
