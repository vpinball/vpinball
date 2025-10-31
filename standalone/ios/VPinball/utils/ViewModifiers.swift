import SwiftUI

struct BlinkEffect: ViewModifier {
    @State var isVisible = true
    @State var loopCount = 0

    var interval: TimeInterval
    var maxLoop: Int = 0

    func body(content: Content) -> some View {
        content
            .opacity(isVisible ? 1 : 0)
            .onAppear {
                Timer.scheduledTimer(withTimeInterval: interval,
                                     repeats: true)
                { timer in
                    withAnimation {
                        isVisible.toggle()
                    }

                    if maxLoop > 0 {
                        loopCount += 1

                        if loopCount >= maxLoop * 2 {
                            timer.invalidate()
                        }
                    }
                }
            }
    }
}

struct GradientEffect: ViewModifier {
    let icon: String
    let contentMode: ContentMode

    func body(content: Content) -> some View {
        content
            .overlay(
                LinearGradient(
                    gradient: Gradient(colors: [
                        Color(hex: 0x555555),
                        Color(hex: 0x777777),
                        Color(hex: 0xBBBBBB),
                        Color(hex: 0xFFFFFF),
                        Color(hex: 0xBBBBBB),
                        Color(hex: 0x777777),
                        Color(hex: 0x555555),
                    ]),
                    startPoint: .topLeading,
                    endPoint: .bottomTrailing
                )
                .mask(
                    Image(icon)
                        .resizable()
                        .aspectRatio(contentMode: contentMode)
                )
            )
    }
}
