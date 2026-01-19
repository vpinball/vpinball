import Foundation

enum TableFileOperations {
    static func exists(_ path: String) -> Bool {
        return FileManager.default.fileExists(atPath: path)
    }

    static func read(_ path: String) -> String? {
        return try? String(contentsOfFile: path,
                           encoding: .utf8)
    }

    static func write(_ path: String, content: String) -> Bool {
        do {
            try content.write(toFile: path,
                              atomically: true,
                              encoding: .utf8)
            return true
        } catch {
            return false
        }
    }

    static func copy(from: String, to: String) -> Bool {
        do {
            if exists(to) {
                try FileManager.default.removeItem(atPath: to)
            }
            try FileManager.default.copyItem(atPath: from,
                                             toPath: to)
            return true
        } catch {
            return false
        }
    }

    static func delete(_ path: String) -> Bool {
        do {
            try FileManager.default.removeItem(atPath: path)
            return true
        } catch {
            return false
        }
    }

    static func deleteDirectory(_ path: String) -> Bool {
        do {
            try FileManager.default.removeItem(atPath: path)
            return true
        } catch {
            return false
        }
    }

    static func createDirectory(_ path: String) -> Bool {
        do {
            try FileManager.default.createDirectory(atPath: path,
                                                    withIntermediateDirectories: true)
            return true
        } catch {
            return false
        }
    }

    static func listFiles(_ path: String, ext: String) -> [String] {
        var files: [String] = []
        let preferencesPath = VPinballManager.shared.getPath(.preferences)

        if let enumerator = FileManager.default.enumerator(atPath: path) {
            while let file = enumerator.nextObject() as? String {
                let fullPath = (path as NSString).appendingPathComponent(file)
                var isDirectory: ObjCBool = false

                if FileManager.default.fileExists(atPath: fullPath,
                                                  isDirectory: &isDirectory)
                {
                    if isDirectory.boolValue {
                        if fullPath == preferencesPath {
                            enumerator.skipDescendants()
                        }
                    } else {
                        if ext.isEmpty || (fullPath as NSString).pathExtension.lowercased() == ext.lowercased().replacingOccurrences(of: ".",
                                                                                                                                     with: "")
                        {
                            files.append(fullPath)
                        }
                    }
                }
            }
        }

        return files
    }

    static func copyDirectory(from: String, to: String) -> Bool {
        do {
            if exists(to) {
                try FileManager.default.removeItem(atPath: to)
            }

            try FileManager.default.createDirectory(atPath: to,
                                                    withIntermediateDirectories: true)

            let contents = try FileManager.default.contentsOfDirectory(atPath: from)

            for item in contents {
                let sourcePath = (from as NSString).appendingPathComponent(item)
                let destPath = (to as NSString).appendingPathComponent(item)

                var isDirectory: ObjCBool = false
                if FileManager.default.fileExists(atPath: sourcePath,
                                                  isDirectory: &isDirectory)
                {
                    if isDirectory.boolValue {
                        if !copyDirectory(from: sourcePath, to: destPath) {
                            return false
                        }
                    } else {
                        try FileManager.default.copyItem(atPath: sourcePath,
                                                         toPath: destPath)
                    }
                }
            }

            return true
        } catch {
            return false
        }
    }
}
