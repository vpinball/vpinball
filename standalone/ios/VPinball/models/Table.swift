import UIKit

struct Table: Codable, Identifiable, Hashable {
    let uuid: String
    let name: String
    let path: String
    let image: String
    let createdAt: Int64
    let modifiedAt: Int64

    var id: String {
        uuid
    }

    var tableId: UUID {
        UUID(uuidString: uuid) ?? UUID()
    }

    var fullURL: URL {
        URL(fileURLWithPath: VPinballManager.shared.getPath(.tables)).appendingPathComponent(path)
    }

    var fullPath: String {
        fullURL.path
    }

    var baseURL: URL {
        fullURL.deletingLastPathComponent()
    }

    var basePath: String {
        baseURL.path
    }

    var fileName: String {
        fullURL.lastPathComponent
    }

    var stem: String {
        fullURL.deletingPathExtension().lastPathComponent
    }

    var imageURL: URL {
        URL(fileURLWithPath: VPinballManager.shared.getPath(.tables)).appendingPathComponent(image)
    }

    var imagePath: String {
        imageURL.path
    }

    var scriptURL: URL {
        fullURL.deletingPathExtension().appendingPathExtension("vbs")
    }

    var scriptPath: String {
        scriptURL.path
    }

    var iniURL: URL {
        fullURL.deletingPathExtension().appendingPathExtension("ini")
    }

    var iniPath: String {
        iniURL.path
    }

    var uiImage: UIImage? {
        if !image.isEmpty {
            let cache = tableImageCache
            let cacheKey = "\(uuid)_\(modifiedAt)" as NSString
            if let cached = cache.object(forKey: cacheKey) {
                return cached
            }
            if let loaded = UIImage(contentsOfFile: imagePath) {
                cache.setObject(loaded, forKey: cacheKey)
                return loaded
            }
        }
        return nil
    }

    func uiImageAsync() async -> UIImage? {
        if image.isEmpty {
            return nil
        }

        let cache = tableImageCache
        let cacheKey = "\(uuid)_\(modifiedAt)" as NSString
        if let cached = cache.object(forKey: cacheKey) {
            return cached
        }

        let imagePath = imagePath
        return await Task.detached(priority: .utility) {
            if let loaded = UIImage(contentsOfFile: imagePath) {
                cache.setObject(loaded, forKey: cacheKey)
                return loaded
            }
            return nil
        }.value
    }

    func with(name: String? = nil, path: String? = nil, image: String? = nil, modifiedAt: Int64? = nil) -> Table {
        Table(uuid: uuid,
              name: name ?? self.name,
              path: path ?? self.path,
              image: image ?? self.image,
              createdAt: createdAt,
              modifiedAt: modifiedAt ?? self.modifiedAt)
    }

    func exists() -> Bool {
        return FileManager.default.fileExists(atPath: fullPath)
    }

    func existsAsync() async -> Bool {
        let fullPath = fullPath
        return await Task.detached(priority: .utility) {
            FileManager.default.fileExists(atPath: fullPath)
        }.value
    }

    func hasScriptFile() -> Bool {
        return FileManager.default.fileExists(atPath: scriptPath)
    }

    func hasScriptFileAsync() async -> Bool {
        let scriptPath = scriptPath
        return await Task.detached(priority: .utility) {
            FileManager.default.fileExists(atPath: scriptPath)
        }.value
    }

    func hasIniFile() -> Bool {
        return FileManager.default.fileExists(atPath: iniPath)
    }

    func hasIniFileAsync() async -> Bool {
        let iniPath = iniPath
        return await Task.detached(priority: .utility) {
            FileManager.default.fileExists(atPath: iniPath)
        }.value
    }

    func hasImageFile() -> Bool {
        return FileManager.default.fileExists(atPath: imagePath)
    }

    func hasImageFileAsync() async -> Bool {
        let imagePath = imagePath
        return await Task.detached(priority: .utility) {
            FileManager.default.fileExists(atPath: imagePath)
        }.value
    }
}

struct TablesResponse: Codable {
    let tableCount: Int
    let tables: [Table]
}
