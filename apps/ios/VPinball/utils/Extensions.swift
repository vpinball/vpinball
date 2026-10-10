import SwiftUI
import UniformTypeIdentifiers

extension Color {
    static let darkBlack = Color(hex: 0x0C0C0C)
    static let lightBlack = Color(hex: 0x101010)
    static let subtitle = Color.primary.opacity(0.7)
    static let editorBackground = Color(UIColor { traits in
        traits.userInterfaceStyle == .dark ? UIColor(Color(hex: 0x1E1E1E)) : UIColor(Color(hex: 0xFFFFFE))
    })

    static let vpxRed = Color(hex: 0xFD251D)
    static let vpxDarkYellow = Color(hex: 0xFEF716)

    static let vpxAccent = Color(UIColor { traits in
        traits.userInterfaceStyle == .dark
            ? UIColor(Color.vpxDarkYellow)
            : UIColor(Color(hex: 0xC8161C))
    })

    init(hex: UInt) {
        self.init(
            red: Double((hex >> 16) & 0xFF) / 255.0,
            green: Double((hex >> 8) & 0xFF) / 255.0,
            blue: Double(hex & 0xFF) / 255.0
        )
    }
}

extension String {
    var cstring: UnsafePointer<CChar> {
        (self as NSString).cString(using: String.Encoding.utf8.rawValue)!
    }
}

extension UTType {
    static var vpx: UTType {
        UTType(exportedAs: "org.vpinball.vpx")
    }

    static var vpxz: UTType {
        UTType(exportedAs: "org.vpinball.vpxz")
    }
}

extension View {
    func blinkEffect(interval: TimeInterval = 0.75) -> some View {
        modifier(BlinkEffect(interval: interval))
    }

    func gradientEffect(icon: String, contentMode: ContentMode) -> some View {
        modifier(GradientEffect(icon: icon,
                                contentMode: contentMode))
    }

    func glassPanel(cornerRadius: CGFloat) -> some View {
        glassEffect(.regular, in: .rect(cornerRadius: cornerRadius, style: .continuous))
    }
}
