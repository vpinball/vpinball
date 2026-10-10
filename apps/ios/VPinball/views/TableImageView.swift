import SwiftUI

struct TableImageView: View {
    static let thumbnailMaxPixelSize = 1024

    let table: Table

    @State private var image: UIImage?

    init(table: Table) {
        self.table = table
        _image = State(initialValue: table.cachedThumbnail(maxPixelSize: Self.thumbnailMaxPixelSize))
    }

    private var shape: RoundedRectangle {
        RoundedRectangle(cornerRadius: TableItemView.cornerRadius, style: .continuous)
    }

    var body: some View {
        Group {
            if let image {
                Color.clear
                    .aspectRatio(TableItemView.aspect, contentMode: .fit)
                    .overlay {
                        Image(uiImage: image)
                            .resizable()
                            .scaledToFill()
                    }
            } else {
                ZStack {
                    Color.darkBlack
                    TableImagePlaceholderView()
                        .padding(.horizontal, 2)
                        .environment(\.colorScheme, .dark)
                }
                .aspectRatio(TableItemView.aspect, contentMode: .fit)
            }
        }
        .clipShape(shape)
        .overlay(shape.strokeBorder(.primary.opacity(0.12), lineWidth: 1))
        .task(id: "\(table.uuid)_\(table.modifiedAt)") {
            if image == nil || image != table.cachedThumbnail(maxPixelSize: Self.thumbnailMaxPixelSize) {
                image = await table.thumbnailAsync(maxPixelSize: Self.thumbnailMaxPixelSize)
            }
        }
    }
}
