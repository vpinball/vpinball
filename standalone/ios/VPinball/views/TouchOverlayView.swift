import SwiftUI

struct TouchOverlayView: View {
    var body: some View {
        GeometryReader { geometry in
            ZStack {
                ForEach(0 ..< VPinballTouchAreas.count, id: \.self) { groupIndex in
                    ForEach(0 ..< VPinballTouchAreas[groupIndex].count, id: \.self) { areaIndex in
                        let region = VPinballTouchAreas[groupIndex][areaIndex]
                        let regionWidth = geometry.size.width * (region.right - region.left) / 100
                        let regionHeight = geometry.size.height * (region.bottom - region.top) / 100
                        let x = geometry.size.width * region.left / 100 + regionWidth / 2
                        let y = geometry.size.height * region.top / 100 + regionHeight / 2

                        Color.clear
                            .border(Color.white.opacity(0.2),
                                    width: 1)
                            .frame(width: regionWidth,
                                   height: regionHeight)
                            .position(x: x, y: y)
                    }
                }
            }
        }
        .ignoresSafeArea()
    }
}

#Preview {
    ZStack {
        Color.gray.ignoresSafeArea()

        TouchOverlayView()
    }
}
