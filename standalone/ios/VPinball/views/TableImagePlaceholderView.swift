
import SwiftUI

struct TableImagePlaceholderView: View {
    var contentMode: ContentMode = .fit

    var body: some View {
        Image("table-placeholder")
            .resizable()
            .aspectRatio(contentMode: contentMode)
            .gradientEffect(icon: "table-placeholder",
                            contentMode: contentMode)
    }
}
