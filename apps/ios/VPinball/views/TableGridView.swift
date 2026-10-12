import SwiftUI

private let gridPadding: CGFloat = 16
private let gridSpacing: CGFloat = 12

struct TableGridView: View {
    let tables: [Table]
    let gridSize: TableGridSize
    let sortOrder: SortOrder
    let searchText: String
    let searchPresented: Bool
    let namespace: Namespace.ID
    @Binding var scrollToTable: Table?

    @State private var contentSize: CGSize = .zero

    private var isSearching: Bool {
        searchPresented || !searchText.isEmpty
    }

    private var columnCount: Int {
        gridSize.columns(width: contentSize.width - gridPadding * 2,
                         height: contentSize.height,
                         spacing: gridSpacing,
                         aspect: TableItemView.aspect)
    }

    private var cardWidth: CGFloat {
        let count = CGFloat(columnCount)
        return max(0, (contentSize.width - gridPadding * 2 - gridSpacing * (count - 1)) / count)
    }

    private var sortedTables: [Table] {
        tables.sorted {
            let result = $0.name.localizedCaseInsensitiveCompare($1.name)
            return sortOrder == .forward ? result == .orderedAscending : result == .orderedDescending
        }
    }

    var body: some View {
        let sorted = sortedTables

        ZStack {
            library(sorted)
                .ignoresSafeArea(.keyboard)
                .allowsHitTesting(!isSearching)

            if isSearching {
                searchResults(sorted.filter { searchText.isEmpty || $0.name.localizedCaseInsensitiveContains(searchText) })
                    .background(AmbientBackgroundView())
            }
        }
    }

    private func library(_ sorted: [Table]) -> some View {
        let recent = Array(tables
            .filter { $0.lastPlayedAt != nil }
            .sorted { ($0.lastPlayedAt ?? 0) > ($1.lastPlayedAt ?? 0) }
            .prefix(6))
        let favorites = sorted.filter(\.isFavorite)

        return ScrollViewReader { proxy in
            ScrollView {
                VStack(alignment: .leading, spacing: 18) {
                    if !recent.isEmpty {
                        MarqueeHeaderView(title: "Recently Played",
                                          sourcePrefix: "recent",
                                          inRecentlyPlayed: true,
                                          tables: recent,
                                          cardWidth: cardWidth,
                                          namespace: namespace)
                    }

                    if !favorites.isEmpty {
                        MarqueeHeaderView(title: "Favorites",
                                          showsCount: true,
                                          sourcePrefix: "favorite",
                                          tables: favorites,
                                          cardWidth: cardWidth,
                                          namespace: namespace)
                    }

                    sectionHeader(title: "All Tables", count: sorted.count)

                    tableGrid(sorted)
                }
                .padding(.horizontal, gridPadding)
                .padding(.vertical, 12)
            }
            .onGeometryChange(for: CGSize.self) { proxy in
                proxy.size
            } action: { size in
                var transaction = Transaction()
                transaction.disablesAnimations = contentSize == .zero
                withTransaction(transaction) {
                    contentSize = size
                }
            }
            .scrollEdgeEffectStyle(.hard, for: .bottom)
            .scrollEdgeEffectHidden(true, for: .bottom)
            .animation(.snappy, value: columnCount)
            .animation(.snappy, value: favorites.count)
            .animation(.snappy, value: recent.count)
            .onChange(of: scrollToTable) { _, newValue in
                guard let table = newValue else { return }
                withAnimation(.snappy) {
                    proxy.scrollTo(table.uuid, anchor: .top)
                }
                scrollToTable = nil
            }
        }
    }

    @ViewBuilder
    private func searchResults(_ results: [Table]) -> some View {
        if results.isEmpty {
            VStack(spacing: 24) {
                TableImagePlaceholderView()
                    .frame(maxHeight: 360)

                VStack(spacing: 12) {
                    Text("Shoot Again!")
                        .font(.title2)
                        .bold()
                        .blinkEffect()

                    Text("Check the spelling or try a new search.")
                        .font(.body)
                        .multilineTextAlignment(.center)
                        .foregroundStyle(Color.subtitle)
                        .padding(.horizontal, 40)
                }
            }
            .padding(.vertical, 32)
            .frame(maxWidth: .infinity, maxHeight: .infinity)
        } else {
            ScrollView {
                VStack(alignment: .leading, spacing: 18) {
                    sectionHeader(title: "Results", count: results.count)

                    tableGrid(results)
                }
                .padding(.horizontal, gridPadding)
                .padding(.vertical, 12)
            }
            .scrollEdgeEffectStyle(.hard, for: .bottom)
            .scrollEdgeEffectHidden(true, for: .bottom)
            .scrollDismissesKeyboard(.immediately)
        }
    }

    private func tableGrid(_ items: [Table]) -> some View {
        LazyVGrid(columns: Array(repeating: GridItem(.flexible(), spacing: gridSpacing, alignment: .top),
                                 count: columnCount),
                  spacing: 16)
        {
            ForEach(items) { table in
                Button {
                    MainViewModel.shared.play(table, sourceID: table.uuid)
                } label: {
                    TableItemView(table: table)
                }
                .buttonStyle(TableCardButtonStyle())
                .matchedTransitionSource(id: table.uuid, in: namespace)
                .contextMenu {
                    TableContextMenu(table: table)
                } preview: {
                    TableContextPreview(table: table)
                }
                .tint(.primary)
                .id(table.uuid)
            }
        }
    }

    private func sectionHeader(title: String, count: Int) -> some View {
        HStack(alignment: .firstTextBaseline) {
            Text(title)
                .font(.title2.weight(.bold))
            Spacer()
            Text("\(count)")
                .font(.subheadline.weight(.semibold))
                .foregroundStyle(.secondary)
        }
        .padding(.horizontal, 4)
    }
}

struct MarqueeHeaderView: View {
    let title: String
    var showsCount = false
    let sourcePrefix: String
    var inRecentlyPlayed = false
    let tables: [Table]
    let cardWidth: CGFloat
    let namespace: Namespace.ID

    @State private var scrollPosition = ScrollPosition(edge: .leading)

    var body: some View {
        VStack(alignment: .leading, spacing: 14) {
            HStack(alignment: .firstTextBaseline) {
                Text(title)
                    .font(.title2.weight(.bold))
                if showsCount {
                    Spacer()
                    Text("\(tables.count)")
                        .font(.subheadline.weight(.semibold))
                        .foregroundStyle(.secondary)
                }
            }
            .padding(.horizontal, 4)

            ScrollView(.horizontal) {
                HStack(alignment: .top, spacing: gridSpacing) {
                    ForEach(Array(tables.enumerated()), id: \.offset) { index, table in
                        Button {
                            MainViewModel.shared.play(table, sourceID: "\(sourcePrefix)-\(table.uuid)")
                        } label: {
                            TableItemView(table: table)
                                .frame(width: cardWidth)
                        }
                        .buttonStyle(TableCardButtonStyle())
                        .matchedTransitionSource(id: "\(sourcePrefix)-\(table.uuid)", in: namespace)
                        .contextMenu {
                            TableContextMenu(table: table, inRecentlyPlayed: inRecentlyPlayed)
                        } preview: {
                            TableContextPreview(table: table)
                        }
                        .tint(.primary)
                        .id("\(sourcePrefix)-\(index)-\(table.uuid)")
                    }
                }
            }
            .scrollPosition($scrollPosition)
            .scrollIndicators(.hidden)
            .contentMargins(.horizontal, gridPadding, for: .scrollContent)
            .padding(.horizontal, -gridPadding)
            .onChange(of: tables.first?.uuid) { _, _ in
                guard inRecentlyPlayed else { return }
                withAnimation(.snappy) {
                    scrollPosition.scrollTo(edge: .leading)
                }
            }
        }
    }
}
