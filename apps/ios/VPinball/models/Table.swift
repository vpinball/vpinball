import ImageIO
import UIKit

struct Table: Codable, Identifiable, Hashable {
    let uuid: String
    let name: String
    let path: String
    let image: String
    let createdAt: Int64
    let modifiedAt: Int64
    let lastPlayedAt: Int64?
    let isFavorite: Bool

    init(uuid: String,
         name: String,
         path: String,
         image: String,
         createdAt: Int64,
         modifiedAt: Int64,
         lastPlayedAt: Int64? = nil,
         isFavorite: Bool = false)
    {
        self.uuid = uuid
        self.name = name
        self.path = path
        self.image = image
        self.createdAt = createdAt
        self.modifiedAt = modifiedAt
        self.lastPlayedAt = lastPlayedAt
        self.isFavorite = isFavorite
    }

    init(from decoder: Decoder) throws {
        let container = try decoder.container(keyedBy: CodingKeys.self)
        uuid = try container.decode(String.self, forKey: .uuid)
        name = try container.decode(String.self, forKey: .name)
        path = try container.decode(String.self, forKey: .path)
        image = try container.decode(String.self, forKey: .image)
        createdAt = try container.decode(Int64.self, forKey: .createdAt)
        modifiedAt = try container.decode(Int64.self, forKey: .modifiedAt)
        lastPlayedAt = try container.decodeIfPresent(Int64.self, forKey: .lastPlayedAt)
        isFavorite = try container.decodeIfPresent(Bool.self, forKey: .isFavorite) ?? false
    }

    var id: String {
        uuid
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

    private var imageCacheKey: String {
        "\(uuid)_\(modifiedAt)"
    }

    func uiImageAsync() async -> UIImage? {
        if image.isEmpty {
            return nil
        }

        let cacheKey = imageCacheKey as NSString
        if let cached = tableImageCache.object(forKey: cacheKey) {
            return cached
        }

        let imagePath = imagePath
        let loaded = await Task.detached(priority: .utility) {
            UIImage(contentsOfFile: imagePath)?.preparingForDisplay()
        }.value

        if let loaded {
            tableImageCache.setObject(loaded, forKey: cacheKey, cost: loaded.byteCost)
        }
        return loaded
    }

    func cachedThumbnail(maxPixelSize: Int) -> UIImage? {
        if image.isEmpty {
            return nil
        }
        return tableImageCache.object(forKey: "\(imageCacheKey)_\(maxPixelSize)" as NSString)
    }

    func thumbnailAsync(maxPixelSize: Int) async -> UIImage? {
        if image.isEmpty {
            return nil
        }

        let cacheKey = "\(imageCacheKey)_\(maxPixelSize)" as NSString
        if let cached = tableImageCache.object(forKey: cacheKey) {
            return cached
        }

        let imageURL = imageURL
        let loaded = await Task.detached(priority: .utility) { () -> UIImage? in
            guard let source = CGImageSourceCreateWithURL(imageURL as CFURL, nil) else { return nil }
            let options: [CFString: Any] = [
                kCGImageSourceCreateThumbnailFromImageAlways: true,
                kCGImageSourceCreateThumbnailWithTransform: true,
                kCGImageSourceShouldCacheImmediately: true,
                kCGImageSourceThumbnailMaxPixelSize: maxPixelSize,
            ]
            guard let cgImage = CGImageSourceCreateThumbnailAtIndex(source, 0, options as CFDictionary) else { return nil }
            return UIImage(cgImage: cgImage)
        }.value

        if let loaded {
            tableImageCache.setObject(loaded, forKey: cacheKey, cost: loaded.byteCost)
        }
        return loaded
    }

    func with(name: String? = nil, path: String? = nil, image: String? = nil, modifiedAt: Int64? = nil) -> Table {
        Table(uuid: uuid,
              name: name ?? self.name,
              path: path ?? self.path,
              image: image ?? self.image,
              createdAt: createdAt,
              modifiedAt: modifiedAt ?? self.modifiedAt,
              lastPlayedAt: lastPlayedAt,
              isFavorite: isFavorite)
    }

    func played(at date: Int64?) -> Table {
        Table(uuid: uuid,
              name: name,
              path: path,
              image: image,
              createdAt: createdAt,
              modifiedAt: modifiedAt,
              lastPlayedAt: date,
              isFavorite: isFavorite)
    }

    func favorite(_ isFavorite: Bool) -> Table {
        Table(uuid: uuid,
              name: name,
              path: path,
              image: image,
              createdAt: createdAt,
              modifiedAt: modifiedAt,
              lastPlayedAt: lastPlayedAt,
              isFavorite: isFavorite)
    }

    func exists() -> Bool {
        return FileManager.default.fileExists(atPath: fullPath)
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
}

struct TablesResponse: Codable {
    let tableCount: Int
    let tables: [Table]
}

private extension UIImage {
    var byteCost: Int {
        guard let cgImage else { return 0 }
        return cgImage.bytesPerRow * cgImage.height
    }
}
