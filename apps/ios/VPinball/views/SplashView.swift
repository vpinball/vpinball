import SwiftUI

struct SplashView: View {
    let startTime = Date.now

    var body: some View {
        ZStack {
            Color.lightBlack.ignoresSafeArea()

            VStack {
                Spacer()
                TimelineView(.animation) { timeline in
                    let elapsedTime = startTime.distance(to: timeline.date)
                    Image("vpinball-logo")
                        .resizable()
                        .aspectRatio(contentMode: .fit)
                        .frame(maxWidth: 300)
                        .visualEffect { content, proxy in
                            content
                                .colorEffect(ShaderLibrary.shimmer(
                                    .float2(proxy.size),
                                    .float(elapsedTime),
                                    .float(0.2),
                                    .float(2.0),
                                    .float(0.4)
                                ))
                        }
                }
                .padding(50)
                Spacer()
            }
            .ignoresSafeArea()

            VStack {
                Spacer()

                Text(String(cString: VPinballGetVersionStringFull()))
                    .font(.caption)
                    .bold()
                    .multilineTextAlignment(.center)
                    .foregroundStyle(Color.white)
            }
            .padding(.bottom, 10)
        }
    }
}

#Preview {
    SplashView()
}
