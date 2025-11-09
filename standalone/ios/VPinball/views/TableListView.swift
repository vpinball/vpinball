import SwiftUI

enum TableViewMode: Int, Hashable {
    case grid = 0
    case list = 1
}

enum TableGridSize: Int, Hashable {
    case small = 0
    case medium = 1
    case large = 2
}

struct TableListView: View {
    @ObservedObject var vpinballViewModel = VPinballViewModel.shared
    @ObservedObject var tableManager = TableManager.shared

    var viewMode: TableViewMode
    var gridSize: TableGridSize
    var sortOrder: SortOrder
    var searchText: String
    @Binding var scrollToTable: Table?

    @State var selectedTable: Table?

    init(viewMode: TableViewMode = .grid, gridSize: TableGridSize = .medium, sortOrder: SortOrder = .reverse, searchText: String = "", scrollToTable: Binding<Table?> = .constant(nil)) {
        self.viewMode = viewMode
        self.gridSize = gridSize
        self.sortOrder = sortOrder
        self.searchText = searchText
        _scrollToTable = scrollToTable
    }

    var filteredTables: [Table] {
        return tableManager.filteredTables(searchText: searchText, sortOrder: sortOrder)
    }

    var body: some View {
        ZStack {
            if viewMode == .list {
                ScrollViewReader { proxy in
                    List {
                        ForEach(filteredTables) { table in
                            Button {
                                handlePlay(table)
                            }
                            label: {
                                HStack(spacing: 15) {
                                    TableListButton(table: table,
                                                    showTitle: false)
                                        .frame(height: 120)

                                    Text(table.name)
                                        .frame(maxWidth: .infinity,
                                               alignment: .leading)
                                        .multilineTextAlignment(.leading)
                                        .foregroundStyle(Color.white)
                                        .lineLimit(3)
                                        .padding(.horizontal, 12)
                                }
                            }
                            .id(table.uuid)
                            .swipeActions(edge: .trailing,
                                          allowsFullSwipe: false)
                            {
                                Button {
                                    withAnimation(nil) {
                                        handleDelete(table: table)
                                    }
                                } label: {
                                    Image(uiImage: UIImage(systemName: "trash")!.withTintColor(.white,
                                                                                               renderingMode: .alwaysOriginal))
                                    Text("Delete")
                                }
                                .tint(Color.vpxRed)
                            }
                            .listRowInsets(.init(top: 10,
                                                 leading: 10,
                                                 bottom: 10,
                                                 trailing: 10))
                            .listRowSeparatorTint(Color.darkGray)
                            .listRowBackground(selectedTable?.uuid == table.uuid ? Color.darkGray : Color.lightBlack)
                            .alignmentGuide(.listRowSeparatorLeading) { _ in
                                0
                            }
                        }
                    }
                    .listStyle(.plain)
                    .onChange(of: scrollToTable) {
                        if let table = scrollToTable {
                            proxy.scrollTo(table.uuid, anchor: .top)
                        }
                    }
                }
            } else {
                GeometryReader { geo in
                    let size = geo.size
                    let layout = computeColumns(containerWidth: size.width - 32,
                                                availableHeight: size.height,
                                                gridSize: gridSize)
                    ScrollView {
                        LazyVGrid(columns: Array(repeating: GridItem(.fixed(layout.cardWidth), spacing: layout.gap, alignment: .top), count: layout.columns), spacing: layout.gap) {
                            ForEach(filteredTables) { table in
                                EmptyView().id(table.uuid)
                                TableListButton(table: table)
                                    .opacity(selectedTable?.uuid == table.uuid ? 0.5 : 1)
                                    .onTapGesture { handlePlay(table) }
                                    .frame(width: layout.cardWidth, height: layout.cardWidth * 1.5)
                            }
                        }
                        .scrollTargetLayout()
                        .padding(.horizontal, 16)
                        .padding(.vertical, 16)
                    }
                    .scrollPosition(id: Binding(
                        get: { scrollToTable?.uuid },
                        set: { _ in scrollToTable = nil }
                    ), anchor: .top)
                }
            }

            if filteredTables.isEmpty && !searchText.isEmpty {
                GeometryReader { geometry in
                    VStack {
                        Spacer()

                        VStack(spacing: 40) {
                            TablePlaceholderImage()

                            VStack(spacing: 20) {
                                Text("Shoot Again!")
                                    .font(.title)
                                    .bold()
                                    .foregroundStyle(Color.vpxDarkYellow)
                                    .blinkEffect()

                                Text("Please make sure the table name is correct, or try searching for another table.")
                                    .font(.callout)
                                    .multilineTextAlignment(.center)
                                    .foregroundStyle(Color.white)
                                    .padding(.horizontal, 40)
                            }
                        }
                        .frame(height: geometry.size.height * 0.80)

                        Spacer()
                    }
                    .frame(maxWidth: .infinity)
                }
            }
        }
    }

    func handleDelete(table: Table) {
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
            vpinballViewModel.setAction(action: .delete,
                                        table: table)
        }
    }

    func handlePlay(_ table: Table) {
        selectedTable = table

        DispatchQueue.main.asyncAfter(deadline: .now() + 0.25) {
            selectedTable = nil

            DispatchQueue.main.asyncAfter(deadline: .now() + 0.25) {
                vpinballViewModel.setAction(action: .play,
                                            table: table)
            }
        }
    }
}

extension TableListView {
    private var baseGap: CGFloat { 12 }
    private var ratio: CGFloat { 2 / 3 }
    private var minReadableWidth: CGFloat { 120 }
    private var minFloorRegular: CGFloat { 48 }
    private var minFloorCompact: CGFloat { 40 }

    private func heightFactor(_ gridSize: TableGridSize) -> CGFloat {
        switch gridSize {
        case .small: return 0.72
        case .medium: return 0.88
        case .large: return 1.0
        }
    }

    private func computeTiers(containerWidth: CGFloat, availableHeight: CGFloat, gap: CGFloat, minFloor: CGFloat) -> (small: Int, medium: Int, large: Int, maxWidthFromHeight: CGFloat) {
        let baseCap = max(60, availableHeight) * ratio
        func calculateColumns(capFactor: CGFloat) -> Int {
            var minWidth = minReadableWidth
            let effectiveCap = baseCap * capFactor
            var effectiveMin = min(minWidth, effectiveCap)
            var columns = Int(floor((containerWidth + gap) / (effectiveMin + gap)))
            while columns < 3 && minWidth > minFloor {
                minWidth -= 6
                effectiveMin = min(minWidth, effectiveCap)
                columns = Int(floor((containerWidth + gap) / (effectiveMin + gap)))
            }
            return max(1, columns)
        }
        let smallColumnsRaw = calculateColumns(capFactor: 0.72)
        let mediumColumnsRaw = calculateColumns(capFactor: 0.88)
        let largeColumnsRaw = calculateColumns(capFactor: 1.00)

        let smallColumns = max(3, smallColumnsRaw)
        var mediumColumns = min(mediumColumnsRaw, smallColumns - 1)
        if mediumColumns < 2 { mediumColumns = max(2, smallColumns - 1) }
        var largeColumns = min(largeColumnsRaw, mediumColumns - 1)
        if largeColumns < 1 { largeColumns = 1 }

        return (smallColumns, mediumColumns, largeColumns, baseCap)
    }

    private func cardWidthForColumns(_ columns: Int, containerWidth: CGFloat, heightCap: CGFloat, gridSize: TableGridSize, gap: CGFloat) -> CGFloat {
        let widthPerColumn = (containerWidth - gap * CGFloat(max(columns - 1, 0))) / CGFloat(max(columns, 1))
        var width = min(widthPerColumn, heightCap)
        if columns == 1 {
            let factor: CGFloat = {
                switch gridSize {
                case .small: return 0.86
                case .medium: return 0.94
                case .large: return 1.00
                }
            }()
            width = min(width, containerWidth * factor)
        }
        return floor(width)
    }

    private func computeColumns(containerWidth: CGFloat, availableHeight: CGFloat, gridSize: TableGridSize) -> (columns: Int, cardWidth: CGFloat, gap: CGFloat) {
        let layout: (gap: CGFloat, minFloor: CGFloat) = (availableHeight < 420) ? (8, minFloorCompact) : (baseGap, minFloorRegular)
        let tiers = computeTiers(containerWidth: containerWidth,
                                 availableHeight: availableHeight,
                                 gap: layout.gap,
                                 minFloor: layout.minFloor)

        let effectiveSmall = min(tiers.small, 6)
        var effectiveMedium = min(tiers.medium, effectiveSmall - 1)
        if effectiveMedium < 2 { effectiveMedium = max(1, effectiveSmall - 1) }
        var effectiveLarge = min(tiers.large, effectiveMedium - 1)
        if effectiveLarge < 1 { effectiveLarge = 1 }
        let columns: Int = {
            switch gridSize {
            case .small: return effectiveSmall
            case .medium: return effectiveMedium
            case .large: return effectiveLarge
            }
        }()
        let heightCap = max(60, availableHeight) * ratio * heightFactor(gridSize)
        let cardWidth = cardWidthForColumns(columns,
                                            containerWidth: containerWidth,
                                            heightCap: heightCap,
                                            gridSize: gridSize,
                                            gap: layout.gap)
        return (columns, cardWidth, layout.gap)
    }
}
