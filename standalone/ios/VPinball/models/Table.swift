import UIKit

struct Table: Codable, Identifiable, Hashable {
    let uuid: String
    let name: String
    let path: String
    let image: String
    let createdAt: Int64
    let modifiedAt: Int64

    var id: String { uuid }

    var tableId: UUID {
        UUID(uuidString: uuid) ?? UUID()
    }

    var fileName: String {
        (path as NSString).lastPathComponent
    }

    var uiImage: UIImage? {
        if !image.isEmpty {
            return UIImage(contentsOfFile: imagePath)
        } else {
            return nil
        }
    }

    var imagePath: String {
        let tablesPath = VPinballManager.shared.getPath(.tables)
        return (tablesPath as NSString).appendingPathComponent(image)
    }

    var fullPath: String {
        let tablesPath = VPinballManager.shared.getPath(.tables)
        return (tablesPath as NSString).appendingPathComponent(path)
    }

    var baseURL: URL {
        URL(fileURLWithPath: fullPath).deletingLastPathComponent()
    }

    var basePath: String {
        return baseURL.path
    }

    var fullURL: URL {
        URL(fileURLWithPath: fullPath)
    }

    var scriptURL: URL {
        return fullURL.deletingPathExtension().appendingPathExtension("vbs")
    }

    var scriptPath: String {
        return scriptURL.path
    }

    var iniURL: URL {
        return fullURL.deletingPathExtension().appendingPathExtension("ini")
    }

    var iniPath: String {
        return iniURL.path
    }

    func exists() -> Bool {
        return FileManager.default.fileExists(atPath: fullPath)
    }

    func hasScriptFile() -> Bool {
        return FileManager.default.fileExists(atPath: scriptPath)
    }

    func hasIniFile() -> Bool {
        return FileManager.default.fileExists(atPath: iniPath)
    }

    func hasImageFile() -> Bool {
        return FileManager.default.fileExists(atPath: imagePath)
    }
}

struct TablesResponse: Codable {
    let tableCount: Int
    let tables: [Table]
}
