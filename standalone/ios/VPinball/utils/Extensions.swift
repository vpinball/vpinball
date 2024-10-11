import SwiftUI
import UniformTypeIdentifiers

extension Color {
    static let darkBlack = Color(hex: 0x0C0C0C)
    static let lightBlack = Color(hex: 0x101010)
    static let darkGray = Color(hex: 0x3C3C3C)

    static let vpxRed = Color(hex: 0xFD251D)
    static let vpxLightYellow = Color(hex: 0xFFFD8E)
    static let vpxDarkYellow = Color(hex: 0xFEF716)

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

    var htmlEscaped: String {
        return replacingOccurrences(of: "&", with: "&amp;")
            .replacingOccurrences(of: "<", with: "&lt;")
            .replacingOccurrences(of: ">", with: "&gt;")
            .replacingOccurrences(of: "\"", with: "&quot;")
            .replacingOccurrences(of: "'", with: "&#39;")
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

extension UIImage {
    func resizeWithAspectFit(newSize: CGSize) -> UIImage? {
        let widthRatio = newSize.width / size.width
        let heightRatio = newSize.height / size.height

        let aspectFitRatio = min(widthRatio, heightRatio)
        let aspectFitSize = CGSize(width: size.width * aspectFitRatio,
                                   height: size.height * aspectFitRatio)

        let renderer = UIGraphicsImageRenderer(size: newSize)
        return renderer.image { context in
            UIColor.black.setFill()
            context.fill(CGRect(origin: .zero, size: newSize))

            let origin = CGPoint(
                x: (newSize.width - aspectFitSize.width) / 2.0,
                y: (newSize.height - aspectFitSize.height) / 2.0
            )
            draw(in: CGRect(origin: origin,
                            size: aspectFitSize))
        }
    }
}

extension MTLTexture {
    func bytes() -> UnsafeMutableRawPointer {
        let width = self.width
        let height = self.height
        let rowBytes = self.width * 4
        let p = malloc(width * height * 4)!
        getBytes(p,
                 bytesPerRow: rowBytes,
                 from: MTLRegionMake2D(0, 0, width, height), mipmapLevel: 0)
        return p
    }

    func toImage() -> CGImage? {
        let p = bytes()
        let pColorSpace = CGColorSpaceCreateDeviceRGB()
        let rawBitmapInfo = CGImageAlphaInfo.noneSkipFirst.rawValue | CGBitmapInfo.byteOrder32Little.rawValue
        let bitmapInfo = CGBitmapInfo(rawValue: rawBitmapInfo)

        let selftureSize = width * height * 4
        let rowBytes = width * 4
        if let provider = CGDataProvider(dataInfo: nil,
                                         data: p,
                                         size: selftureSize,
                                         releaseData: { _, p, _ in
                                             p.deallocate()
                                         })
        {
            return CGImage(width: width,
                           height: height,
                           bitsPerComponent: 8,
                           bitsPerPixel: 32,
                           bytesPerRow: rowBytes,
                           space: pColorSpace,
                           bitmapInfo: bitmapInfo,
                           provider: provider,
                           decode: nil,
                           shouldInterpolate: true,
                           intent: CGColorRenderingIntent.defaultIntent)
        }
        return nil
    }
}

extension CAMetalLayer {
    func toUIImage() -> UIImage? {
        guard let drawable = nextDrawable() else {
            return nil
        }

        let texture = drawable.texture

        if let cgImage = texture.toImage() {
            return UIImage(cgImage: cgImage)
        }

        return nil
    }
}

extension View {
    func onTouchDownGesture(callback: (() -> Void)? = nil) -> some View {
        modifier(OnTouchDownGesture(callback: callback))
    }

    func blinkEffect(interval: TimeInterval = 0.75, maxLoop: Int = 0) -> some View {
        modifier(BlinkEffect(interval: interval,
                             maxLoop: maxLoop))
    }

    func gradientEffect(icon: String, size: CGSize, contentMode: ContentMode) -> some View {
        modifier(GradientEffect(icon: icon,
                                size: size,
                                contentMode: contentMode))
    }

    func gradientEffect(icon: String, aspectRatio: CGFloat, contentMode: ContentMode) -> some View {
        modifier(GradientEffectWithAspectRatio(icon: icon,
                                               aspectRatio: aspectRatio,
                                               contentMode: contentMode))
    }
}
