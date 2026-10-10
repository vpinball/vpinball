import SwiftUI

struct TableContextPreview: View {
    let table: Table

    var body: some View {
        TableItemView(table: table, showTitle: false)
            .padding(8)
            .background(Color(.systemBackground))
            .frame(height: 320)
    }
}
