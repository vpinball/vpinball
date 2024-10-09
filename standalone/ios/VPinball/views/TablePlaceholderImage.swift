
import SwiftUI

struct TablePlaceholderImage: View {
    var contentMode: ContentMode = .fit

    var body: some View {
        Image("TablePlaceholder")
            .resizable()
            .aspectRatio(1179 / 2556,
                         contentMode: contentMode)
            .background(Color.lightBlack)
            .gradientEffect(icon: "TablePlaceholder",
                            aspectRatio: 1179 / 2556,
                            contentMode: contentMode)
    }
}
