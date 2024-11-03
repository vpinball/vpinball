
import SwiftUI

struct TablePlaceholderImage: View {
    var contentMode: ContentMode = .fit

    var body: some View {
        Image("table-placeholder")
            .resizable()
            .aspectRatio(1179 / 2556,
                         contentMode: contentMode)
            .background(Color.lightBlack)
            .gradientEffect(icon: "table-placeholder",
                            aspectRatio: 1179 / 2556,
                            contentMode: contentMode)
    }
}
