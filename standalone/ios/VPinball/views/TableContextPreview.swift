import SwiftUI

struct TableContextPreview: View {
    let table: Table

    var body: some View {
        TableImageView(table: table)
            .padding(4)
            .background(Color.lightBlack)
            .frame(height: 300)
            .clipShape(RoundedRectangle(cornerRadius: 8))
    }
}
