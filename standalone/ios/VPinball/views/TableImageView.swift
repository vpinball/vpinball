
import SwiftUI

struct TableImageView: View {
    var table: Table
    var selected: Bool = false
    @State private var image: UIImage?

    var body: some View {
        Group {
            if let image {
                Image(uiImage: image)
                    .resizable()
                    .scaledToFit()
                    .padding(.horizontal, 2)
            } else {
                TableImagePlaceholderView()
                    .padding(.horizontal, 2)
            }
        }
        .clipShape(RoundedRectangle(cornerRadius: 6))
        .opacity(selected ? 0.5 : 1.0)
        .animation(.spring(duration: 0.2),
                   value: selected)
        .task(id: "\(table.uuid)_\(table.modifiedAt)") {
            image = await table.uiImageAsync()
        }
    }
}
