
import SwiftUI

struct TableImageView: View {
    var table: PinTable
    var selected: Bool = false

    var body: some View {
        Group {
            if let image = table.uiImage {
                Image(uiImage: image)
                    .resizable()
                    .aspectRatio(1179 / 2556,
                                 contentMode: .fit)

            } else {
                TablePlaceholderImage()
            }
        }
        .clipShape(RoundedRectangle(cornerRadius: 6))
        .padding(2)
        .background(Color.black)
        .clipShape(RoundedRectangle(cornerRadius: 6))
        .padding(2)
        .background(Color.vpxDarkYellow)
        .clipShape(RoundedRectangle(cornerRadius: 6))
        .opacity(selected ? 0.5 : 1.0)
        .animation(.spring(duration: 0.2),
                   value: selected)
    }
}
