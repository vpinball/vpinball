import Foundation
import ZIPFoundation

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

        if let enumerator = FileManager.default.enumerator(atPath: path) {
            while let file = enumerator.nextObject() as? String {
                let fullPath = (path as NSString).appendingPathComponent(file)
                var isDirectory: ObjCBool = false

                if FileManager.default.fileExists(atPath: fullPath,
                                                  isDirectory: &isDirectory)
                {
                    if !isDirectory.boolValue {
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

    static func addDirectoryToArchive(archive: Archive, directoryPath: String, basePath: String, onProgress: ((Int) async -> Void)? = nil) async throws {
        let fileManager = FileManager.default
        let enumerator = fileManager.enumerator(atPath: directoryPath)

        var allFiles: [String] = []
        while let file = enumerator?.nextObject() as? String {
            allFiles.append(file)
        }

        let fileCount = allFiles.filter { file in
            let fullPath = (directoryPath as NSString).appendingPathComponent(file)
            var isDirectory: ObjCBool = false
            fileManager.fileExists(atPath: fullPath,
                                   isDirectory: &isDirectory)
            return !isDirectory.boolValue
        }.count

        var processedFiles = 0

        for file in allFiles {
            let fullPath = (directoryPath as NSString).appendingPathComponent(file)
            var isDirectory: ObjCBool = false

            if fileManager.fileExists(atPath: fullPath,
                                      isDirectory: &isDirectory)
            {
                let relativePath = String(fullPath.dropFirst(basePath.count + 1))

                if isDirectory.boolValue {
                    try archive.addEntry(with: relativePath + "/",
                                         type: .directory,
                                         uncompressedSize: 0,
                                         provider: { (_: Int64, _: Int) -> Data in
                                             return Data()
                                         })
                } else {
                    try archive.addEntry(with: relativePath,
                                         relativeTo: URL(fileURLWithPath: basePath))

                    processedFiles += 1
                    if fileCount > 0 {
                        let progress = (processedFiles * 100) / fileCount
                        await onProgress?(progress)
                    }
                }
            }
        }
    }
}
