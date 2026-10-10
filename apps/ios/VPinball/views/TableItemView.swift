import SwiftUI

struct TableItemView: View {
    static let aspect: CGFloat = 1 / 2
    static let cornerRadius: CGFloat = 14

    let table: Table
    var showTitle = true

    var body: some View {
        if showTitle {
            VStack(spacing: 8) {
                TableImageView(table: table)

                Text(table.name)
                    .font(.subheadline.weight(.medium))
                    .foregroundStyle(.primary)
                    .lineLimit(3)
                    .multilineTextAlignment(.leading)
                    .frame(maxWidth: .infinity, alignment: .leading)
                    .padding(.horizontal, 2)
            }
        } else {
            TableImageView(table: table)
        }
    }
}
