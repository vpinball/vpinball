import SwiftUI

struct TouchDownInterceptingContainer<Content: View>: View {
    let content: Content
    let onTouchDown: (() -> Void)?

    init(_ onTouchDown: (() -> Void)? = nil, @ViewBuilder content: () -> Content) {
        self.onTouchDown = onTouchDown
        self.content = content()
    }

    var body: some View {
        content
            .onTouchDownGesture {
                self.onTouchDown?()
            }
    }
}
