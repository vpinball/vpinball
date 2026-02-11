import PhotosUI
import SwiftUI

struct TableItemView: View {
    let table: Table

    var showTitle = true
    var enableContextMenu = true

    var body: some View {
        let content = ZStack(alignment: .bottom) {
            GeometryReader { geometry in
                TableImageView(table: table)
                    .frame(width: geometry.size.width,
                           height: geometry.size.height)
            }
            .aspectRatio(2 / 3,
                         contentMode: .fit)

            if showTitle {
                VStack {
                    Text(table.name)
                        .multilineTextAlignment(.center)
                        .font(.footnote)
                        .bold()
                        .foregroundStyle(Color.white)
                        .padding(.horizontal, 7)
                        .padding(.vertical, 5)
                }
                .frame(maxWidth: .infinity)
                .background(
                    .ultraThinMaterial,
                    in: RoundedRectangle(cornerRadius: 6)
                )
            }
        }
        Group {
            if enableContextMenu {
                content
                    .contextMenu {
                        TableContextMenu(table: table)
                    } preview: {
                        TableContextPreview(table: table)
                    }
            } else {
                content
            }
        }
    }
}
