import SwiftUI

struct BlinkEffect: ViewModifier {
    @State private var isVisible = true

    let interval: TimeInterval

    func body(content: Content) -> some View {
        content
            .opacity(isVisible ? 1 : 0)
            .task {
                while !Task.isCancelled {
                    try? await Task.sleep(for: .seconds(interval))
                    withAnimation {
                        isVisible.toggle()
                    }
                }
            }
    }
}

struct GradientEffect: ViewModifier {
    @Environment(\.colorScheme) private var colorScheme

    let icon: String
    let contentMode: ContentMode

    private var colors: [Color] {
        colorScheme == .dark
            ? [0x555555, 0x777777, 0xBBBBBB, 0xFFFFFF, 0xBBBBBB, 0x777777, 0x555555].map { Color(hex: $0) }
            : [0x8E8E93, 0x6E6E73, 0x48484C, 0x6E6E73, 0x8E8E93].map { Color(hex: $0) }
    }

    func body(content: Content) -> some View {
        content
            .overlay(
                LinearGradient(
                    gradient: Gradient(colors: colors),
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

struct TableCardButtonStyle: ButtonStyle {
    func makeBody(configuration: Configuration) -> some View {
        configuration.label
            .scaleEffect(configuration.isPressed ? 0.95 : 1)
            .opacity(configuration.isPressed ? 0.8 : 1)
            .animation(.spring(duration: 0.25), value: configuration.isPressed)
    }
}

struct AmbientBackgroundView: View {
    @Environment(\.colorScheme) private var colorScheme

    var body: some View {
        ZStack {
            if colorScheme == .dark {
                Color.lightBlack

                RadialGradient(colors: [Color.vpxRed.opacity(0.10), .clear],
                               center: .init(x: 0.05, y: 0.12),
                               startRadius: 0,
                               endRadius: 420)

                RadialGradient(colors: [Color(hex: 0x3A2BFF).opacity(0.07), .clear],
                               center: .init(x: 0.9, y: 0.18),
                               startRadius: 0,
                               endRadius: 360)

                RadialGradient(colors: [Color.vpxDarkYellow.opacity(0.05), .clear],
                               center: .init(x: 0.9, y: 0.95),
                               startRadius: 0,
                               endRadius: 480)
            } else {
                Color(.systemGroupedBackground)
            }
        }
        .ignoresSafeArea()
    }
}
