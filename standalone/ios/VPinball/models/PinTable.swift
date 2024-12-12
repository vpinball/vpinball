import SwiftData
import UIKit

@Model
class PinTable {
    @Attribute(.unique)
    var tableId: UUID

    var name: String
    var path: String

    @Attribute(.externalStorage)
    var imageData: Data?

    var createdAt: Date = Date()
    var lastUpdate: Date = Date()

    init(url: URL) {
        let fileManager = FileManager.default
        let documentsDirectory = fileManager.urls(for: .documentDirectory,
                                                  in: .userDomainMask).first!

        var baseURL: URL
        var tableId: UUID

        repeat {
            tableId = UUID()
            baseURL = documentsDirectory.appendingPathComponent(tableId.uuidString)
        } while fileManager.fileExists(atPath: baseURL.path)

        self.tableId = tableId

        name = url.deletingPathExtension().lastPathComponent.replacingOccurrences(of: "[_]",
                                                                                  with: " ",
                                                                                  options: .regularExpression,
                                                                                  range: nil)

        path = url.lastPathComponent

        createdAt = Date()
        lastUpdate = Date()
    }

    @Transient var uiImage: UIImage? {
        return imageData != nil ? UIImage(data: imageData!) : nil
    }

    @Transient var baseURL: URL {
        let documentsDirectory = FileManager.default.urls(for: .documentDirectory,
                                                          in: .userDomainMask).first!
        return documentsDirectory.appendingPathComponent(tableId.uuidString)
    }

    @Transient var basePath: String {
        return baseURL.path
    }

    @Transient var fullURL: URL {
        return baseURL.appendingPathComponent(path)
    }

    @Transient var fullPath: String {
        return fullURL.path
    }

    @Transient var scriptURL: URL {
        return fullURL.deletingPathExtension().appendingPathExtension("vbs")
    }

    @Transient var scriptPath: String {
        return scriptURL.path
    }

    @Transient var iniURL: URL {
        return fullURL.deletingPathExtension().appendingPathExtension("ini")
    }

    @Transient var iniPath: String {
        return iniURL.path
    }

    func exists() -> Bool {
        return FileManager.default.fileExists(atPath: fullPath)
    }

    func hasScript() -> Bool {
        return FileManager.default.fileExists(atPath: scriptPath)
    }

    func hasIni() -> Bool {
        return FileManager.default.fileExists(atPath: iniPath)
    }

    func update(context: ModelContext) {
        lastUpdate = Date()
        try? context.save()
    }
}

extension PinTable {
    static func predicate(
        searchText: String
    ) -> Predicate<PinTable> {
        return #Predicate<PinTable> { table in
            searchText.isEmpty || table.name.localizedStandardContains(searchText)
        }
    }
}

extension PinTable: Identifiable {}
