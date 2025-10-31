
import SwiftUI

struct TableImageView: View {
    var table: Table
    var selected: Bool = false

    var body: some View {
        Group {
            if let image = table.uiImage {
                Image(uiImage: image)
                    .resizable()
                    .scaledToFit()
                    .padding(.horizontal, 2)
            } else {
                TablePlaceholderImage()
                    .padding(.horizontal, 2)
            }
        }
        .id("\(table.uuid)_\(table.image)_\(table.modifiedAt)")
        .clipShape(RoundedRectangle(cornerRadius: 6))
        .opacity(selected ? 0.5 : 1.0)
        .animation(.spring(duration: 0.2),
                   value: selected)
    }
}
