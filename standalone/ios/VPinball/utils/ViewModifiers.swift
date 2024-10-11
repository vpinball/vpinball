import SwiftUI

struct OnTouchDownGesture: ViewModifier {
    @State var tapped = false

    var callback: (() -> Void)? = nil

    func body(content: Content) -> some View {
        content
            .simultaneousGesture(DragGesture(minimumDistance: 0)
                .onChanged { _ in
                    if !self.tapped {
                        self.tapped = true
                        self.callback?()
                    }
                }
                .onEnded { _ in
                    self.tapped = false
                })
    }
}

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
    let size: CGSize
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
                        .frame(width: size.width, height: size.height)
                )
            )
    }
}

struct GradientEffectWithAspectRatio: ViewModifier {
    let icon: String
    let aspectRatio: CGFloat
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
                        .aspectRatio(aspectRatio,
                                     contentMode: contentMode)
                )
            )
    }
}
