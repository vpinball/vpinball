import SwiftUI

struct SplashView: View {
    let startTime = Date.now

    var body: some View {
        ZStack {
            AmbientBackgroundView()

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

                VStack(spacing: 4) {
                    Text(BuildInfo.provenance)

                    Text(BuildInfo.version)
                }
                .font(.caption)
                .foregroundStyle(Color.secondary)
                .multilineTextAlignment(.center)
            }
            .padding(.bottom, 24)
        }
    }
}

#Preview {
    SplashView()
}
