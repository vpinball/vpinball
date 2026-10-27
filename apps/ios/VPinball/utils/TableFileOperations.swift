import Foundation

enum TableFileOperations {
    static func exists(_ url: URL) -> Bool {
        return FileManager.default.fileExists(atPath: url.path)
    }

    static func read(_ url: URL) -> String? {
        return try? String(contentsOf: url, encoding: .utf8)
    }

    static func write(_ url: URL, content: String) -> Bool {
        do {
            try content.write(to: url, atomically: true, encoding: .utf8)
            return true
        } catch {
            return false
        }
    }

    static func copy(from: URL, to: URL) -> Bool {
        do {
            if exists(to) {
                try FileManager.default.removeItem(at: to)
            }
            try FileManager.default.copyItem(at: from, to: to)
            return true
        } catch {
            return false
        }
    }

    static func delete(_ url: URL) -> Bool {
        do {
            try FileManager.default.removeItem(at: url)
            return true
        } catch {
            return false
        }
    }

    static func deleteDirectory(_ url: URL) -> Bool {
        do {
            try FileManager.default.removeItem(at: url)
            return true
        } catch {
            return false
        }
    }

    static func createDirectory(_ url: URL) -> Bool {
        do {
            try FileManager.default.createDirectory(at: url, withIntermediateDirectories: true)
            return true
        } catch {
            return false
        }
    }

    static func listTableURLs(_ url: URL) -> [URL] {
        var tables: [URL] = []
        let preferencesURL = URL(fileURLWithPath: VPinballManager.shared.getPath(.preferences)).standardizedFileURL

        if let enumerator = FileManager.default.enumerator(at: url,
                                                           includingPropertiesForKeys: [.isDirectoryKey],
                                                           options: [])
        {
            for case let fileURL as URL in enumerator {
                let isDirectory = (try? fileURL.resourceValues(forKeys: [.isDirectoryKey]))?.isDirectory == true
                let ext = fileURL.pathExtension.lowercased()

                if isDirectory {
                    if fileURL.standardizedFileURL == preferencesURL || fileURL.lastPathComponent == "Inbox" {
                        enumerator.skipDescendants()
                    } else if ext == "vpz" {
                        tables.append(fileURL)
                        enumerator.skipDescendants()
                    }
                } else if ext == "vpx" || ext == "vpz" {
                    tables.append(fileURL)
                }
            }
        }

        return tables
    }

    static func copyDirectory(from: URL, to: URL) -> Bool {
        do {
            if exists(to) {
                try FileManager.default.removeItem(at: to)
            }

            try FileManager.default.createDirectory(at: to, withIntermediateDirectories: true)

            let contents = try FileManager.default.contentsOfDirectory(at: from,
                                                                       includingPropertiesForKeys: [.isDirectoryKey])

            for item in contents {
                let destItem = to.appendingPathComponent(item.lastPathComponent)
                let resourceValues = try item.resourceValues(forKeys: [.isDirectoryKey])

                if resourceValues.isDirectory == true {
                    if !copyDirectory(from: item, to: destItem) {
                        return false
                    }
                } else {
                    try FileManager.default.copyItem(at: item, to: destItem)
                }
            }

            return true
        } catch {
            return false
        }
    }
}
