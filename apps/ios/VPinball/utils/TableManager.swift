import SwiftUI
import UniformTypeIdentifiers

let tableImageCache = NSCache<NSString, UIImage>()

private var zipProgressHandler: ((Int, Int) -> Void)?

private let zipCallback: VPinballZipCallback = { current, total, _ in
    zipProgressHandler?(Int(current), Int(total))
}

@MainActor
class TableManager {
    static let shared = TableManager()

    var tables: [Table] {
        get { VPinballModel.shared.tables }
        set { VPinballModel.shared.tables = newValue }
    }

    private var tablesURL: URL!
    private var tablesJSONURL: URL!
    private var loadedTable: Table?

    init() {
        loadTablesPath()
        Task {
            await loadTables()
        }
    }

    func refresh() async {
        await loadTables()
    }

    func getTable(uuid: String) -> Table? {
        return tables.first { $0.uuid == uuid }
    }

    func importTable(from url: URL) async -> Bool {
        let ext = url.pathExtension.lowercased()
        let fileName = url.lastPathComponent.removingPercentEncoding ?? url.lastPathComponent

        VPinballModel.shared.showHUD(
            title: fileName,
            status: "Copying file"
        )

        try? await Task.sleep(nanoseconds: 100_000_000)

        let stagingDir = FileManager.default.temporaryDirectory
            .appendingPathComponent("vpinball_staging_\(Int(Date().timeIntervalSince1970))")

        defer {
            _ = TableFileOperations.deleteDirectory(stagingDir)
        }

        if !TableFileOperations.createDirectory(stagingDir) {
            VPinballModel.shared.hideHUD()
            return false
        }

        let stagedURL = stagingDir.appendingPathComponent(url.lastPathComponent)

        let isSecurityScoped = url.startAccessingSecurityScopedResource()

        VPinballModel.shared.updateHUD(
            progress: 10,
            status: "Copying file"
        )

        let copied = await coordinatedCopy(from: url,
                                           to: stagedURL)

        if isSecurityScoped {
            url.stopAccessingSecurityScopedResource()
        }

        if !copied {
            VPinballModel.shared.hideHUD()
            return false
        }

        VPinballModel.shared.updateHUD(
            progress: 60,
            status: ext == "vpx" ? "Importing table" : "Importing archive"
        )

        try? await Task.sleep(nanoseconds: 100_000_000)

        let importedTable = await performImport(url: stagedURL) { progress in
            await MainActor.run {
                VPinballModel.shared.updateHUD(
                    progress: progress,
                    status: ext == "vpx" ? "Importing table" : "Importing archive"
                )
            }
        }

        VPinballModel.shared.updateHUD(
            progress: 100,
            status: "Import complete"
        )

        try? await Task.sleep(nanoseconds: 500_000_000)

        VPinballModel.shared.hideHUD()

        if let table = importedTable {
            MainViewModel.shared.scrollToTable = table
        }

        return importedTable != nil
    }

    func deleteTable(table: Table) async -> Bool {
        let tablesURL = tablesURL!
        let tableFileURL = tablesURL.appendingPathComponent(table.path)
        let tableDirURL = tableFileURL.deletingLastPathComponent()
        let result = await Task.detached(priority: .userInitiated) {
            let tableDir = tableDirURL

            if tableDir.path.isEmpty || tableDir.path == tablesURL.path || tableDir.path + "/" == tablesURL.path {
                return false
            }

            if TableFileOperations.exists(tableDir) {
                let vpxFiles = TableFileOperations.listFiles(tableDir,
                                                             ext: ".vpx")

                if vpxFiles.count <= 1 {
                    if !TableFileOperations.deleteDirectory(tableDir) {
                        return false
                    }
                } else {
                    if !TableFileOperations.delete(tableFileURL) {
                        return false
                    }
                }
            }

            return true
        }.value

        if result {
            tables.removeAll { $0.uuid == table.uuid }
            saveTables()
        }

        return result
    }

    func renameTable(table: Table, newName: String) async -> Bool {
        return performRename(table: table,
                             newName: newName)
    }

    func setTableImage(table: Table, imagePath: String) async -> Bool {
        let tablesURL = tablesURL!
        let tableFileURL = tablesURL.appendingPathComponent(table.path)
        let workingDir = tableFileURL.deletingLastPathComponent()
        let tableImageURL = tablesURL.appendingPathComponent(table.image)

        let (success, resolvedImage) = await Task.detached(priority: .userInitiated) {
            func relativePath(of url: URL, baseURL: URL) -> String {
                let fullPath = url.resolvingSymlinksInPath().path
                let basePath = baseURL.path

                if fullPath.isEmpty || basePath.isEmpty {
                    return fullPath
                }

                if !fullPath.hasPrefix(basePath) {
                    return fullPath
                }

                var result = String(fullPath.dropFirst(basePath.count))
                if result.hasPrefix("/") {
                    result = String(result.dropFirst())
                }

                return result
            }

            if imagePath.isEmpty {
                if !table.image.isEmpty && TableFileOperations.exists(tableImageURL) {
                    _ = TableFileOperations.delete(tableImageURL)
                }
                return (true, "")
            }

            if imagePath.hasPrefix("/") {
                let sourceURL = URL(fileURLWithPath: imagePath)
                if let sourceImage = UIImage(contentsOfFile: imagePath),
                   let jpegData = sourceImage.jpegData(compressionQuality: 0.8)
                {
                    let destURL = workingDir.appendingPathComponent(table.stem + ".jpg")
                    do {
                        try jpegData.write(to: destURL)
                    } catch {
                        return (false, "")
                    }

                    let existingPath = workingDir.appendingPathComponent(tableFileURL.deletingPathExtension().lastPathComponent + ".jpg").path
                    if imagePath != existingPath
                        && sourceURL.pathExtension.lowercased() == "png"
                    {
                        _ = TableFileOperations.delete(sourceURL)
                    }

                    let rel = relativePath(of: destURL, baseURL: tablesURL)
                    return (true, rel)
                }
                return (false, "")
            }

            return (true, imagePath)
        }.value

        if success, let index = tables.firstIndex(where: { $0.uuid == table.uuid }) {
            let now = Int64(Date().timeIntervalSince1970)
            tables[index] = table.with(image: resolvedImage, modifiedAt: now)
            saveTables()
            return true
        }

        return false
    }

    func setTableImage(table: Table, image: UIImage) async -> Bool {
        let tablesURL = tablesURL!
        let tableFileURL = tablesURL.appendingPathComponent(table.path)
        let workingDir = tableFileURL.deletingLastPathComponent()

        if let jpegData = image.jpegData(compressionQuality: 0.8) {
            let (success, resolvedImage) = await Task.detached(priority: .userInitiated) {
                func relativePath(of url: URL, baseURL: URL) -> String {
                    let fullPath = url.resolvingSymlinksInPath().path
                    let basePath = baseURL.path

                    if fullPath.isEmpty || basePath.isEmpty {
                        return fullPath
                    }

                    if !fullPath.hasPrefix(basePath) {
                        return fullPath
                    }

                    var result = String(fullPath.dropFirst(basePath.count))
                    if result.hasPrefix("/") {
                        result = String(result.dropFirst())
                    }

                    return result
                }

                let destURL = workingDir.appendingPathComponent(tableFileURL.deletingPathExtension().lastPathComponent + ".jpg")
                do {
                    try jpegData.write(to: destURL)
                } catch {
                    return (false, "")
                }

                let rel = relativePath(of: destURL, baseURL: tablesURL)
                return (true, rel)
            }.value

            if success, let index = tables.firstIndex(where: { $0.uuid == table.uuid }) {
                let now = Int64(Date().timeIntervalSince1970)
                tables[index] = table.with(image: resolvedImage, modifiedAt: now)
                saveTables()
                return true
            }
        }

        return false
    }

    func exportTable(table: Table) async -> String? {
        VPinballModel.shared.showHUD(title: table.name,
                                     status: "Compressing")

        try? await Task.sleep(nanoseconds: 100_000_000)

        let exportPath = await performExport(table: table) { progress, status in
            await MainActor.run {
                VPinballModel.shared.updateHUD(progress: progress,
                                               status: status)
            }
        }

        try? await Task.sleep(nanoseconds: 500_000_000)

        VPinballModel.shared.hideHUD()

        return exportPath
    }

    func getLoadedTablePath(table: Table) -> String? {
        if table.path.isEmpty {
            return nil
        }

        loadedTable = table
        return table.fullPath
    }

    func clearLoadedTable(table: Table) {
        if loadedTable?.uuid != table.uuid {
            return
        }

        loadedTable = nil
    }

    func extractTableScript(table: Table) async -> Bool {
        VPinballModel.shared.showHUD(title: table.name,
                                     status: "Extracting Script")

        try? await Task.sleep(nanoseconds: 100_000_000)

        let tablesURL = tablesURL!
        let tableFileURL = tablesURL.appendingPathComponent(table.path)
        let tablePath = tableFileURL.path
        let scriptPath = tableFileURL.deletingPathExtension().appendingPathExtension("vbs").path
        let result = await Task.detached(priority: .userInitiated) {
            if TableFileOperations.exists(URL(fileURLWithPath: scriptPath)) {
                return true
            }

            if VPinballStatus(rawValue: VPinballExtractTableScript(tablePath.cstring)) != .success {
                VPinballManager.log(.error, "Failed to extract script from table: \(tablePath)")
                return false
            }

            if TableFileOperations.exists(URL(fileURLWithPath: scriptPath)) {
                VPinballManager.log(.info, "Successfully extracted script: \(scriptPath)")
                return true
            }

            VPinballManager.log(.warn, "Script file not found after extraction")
            return false
        }.value

        if result, let index = tables.firstIndex(where: { $0.uuid == table.uuid }) {
            let now = Int64(Date().timeIntervalSince1970)
            tables[index] = tables[index].with(modifiedAt: now)
            saveTables()
        }

        try? await Task.sleep(nanoseconds: 500_000_000)

        VPinballModel.shared.hideHUD()

        return result
    }

    func resetTableIni(table: Table) -> Bool {
        let tablesURL = tablesURL!
        let tableFileURL = tablesURL.appendingPathComponent(table.path)
        let iniURL = tableFileURL.deletingPathExtension().appendingPathExtension("ini")
        let deleted = TableFileOperations.delete(iniURL)
        if deleted, let index = tables.firstIndex(where: { $0.uuid == table.uuid }) {
            let now = Int64(Date().timeIntervalSince1970)
            tables[index] = table.with(modifiedAt: now)
            saveTables()
        }
        return deleted
    }

    private func loadTablesPath() {
        tablesURL = URL(fileURLWithPath: VPinballManager.shared.getPath(.tables))
        tablesJSONURL = URL(fileURLWithPath: VPinballManager.shared.getPath(.preferences))
            .appendingPathComponent("tables.json")

        if !TableFileOperations.exists(tablesURL) {
            _ = TableFileOperations.createDirectory(tablesURL)
        }
    }

    func loadTables() async {
        let tablesURL = tablesURL!
        let tablesJSONURL = tablesJSONURL!

        let loadedTables = await Task.detached(priority: .userInitiated) {
            func relativePath(of url: URL, baseURL: URL) -> String {
                let fullPath = url.resolvingSymlinksInPath().path
                let basePath = baseURL.path

                if fullPath.isEmpty || basePath.isEmpty {
                    return fullPath
                }

                if !fullPath.hasPrefix(basePath) {
                    return fullPath
                }

                var result = String(fullPath.dropFirst(basePath.count))
                if result.hasPrefix("/") {
                    result = String(result.dropFirst())
                }

                return result
            }

            func createTable(url: URL, existingUUIDs: inout Set<String>) -> Table {
                var uuid = UUID().uuidString.lowercased()
                while existingUUIDs.contains(uuid) {
                    uuid = UUID().uuidString.lowercased()
                    VPinballManager.log(.warn, "UUID collision detected: \(uuid), regenerating")
                }
                existingUUIDs.insert(uuid)

                let relPath = relativePath(of: url, baseURL: tablesURL)

                var name = url.deletingPathExtension().lastPathComponent
                name = name.replacingOccurrences(of: "_",
                                                 with: " ")

                let now = Int64(Date().timeIntervalSince1970)

                let parentURL = url.deletingLastPathComponent()
                let stem = url.deletingPathExtension().lastPathComponent
                let pngURL = parentURL.appendingPathComponent(stem + ".png")
                let jpgURL = parentURL.appendingPathComponent(stem + ".jpg")

                var image = ""
                if TableFileOperations.exists(pngURL) {
                    image = relativePath(of: pngURL, baseURL: tablesURL)
                } else if TableFileOperations.exists(jpgURL) {
                    image = relativePath(of: jpgURL, baseURL: tablesURL)
                }

                return Table(uuid: uuid,
                             name: name,
                             path: relPath,
                             image: image,
                             createdAt: now,
                             modifiedAt: now)
            }

            var loadedTables: [Table] = []

            if TableFileOperations.exists(tablesJSONURL) {
                if let content = TableFileOperations.read(tablesJSONURL),
                   let jsonData = content.data(using: .utf8)
                {
                    if let response = try? JSONDecoder().decode(TablesResponse.self,
                                                                from: jsonData)
                    {
                        loadedTables = response.tables
                    }
                }
            }

            func fileModifiedAt(_ url: URL) -> Int64? {
                if let attrs = try? FileManager.default.attributesOfItem(atPath: url.path),
                   let date = attrs[.modificationDate] as? Date
                {
                    return Int64(date.timeIntervalSince1970)
                }
                return nil
            }

            var bumpedMods = 0
            for i in 0 ..< loadedTables.count {
                let table = loadedTables[i]
                let cleanPath = table.path.hasPrefix("/") ? relativePath(of: URL(fileURLWithPath: table.path), baseURL: tablesURL) : table.path
                let cleanImage = table.image.hasPrefix("/") ? relativePath(of: URL(fileURLWithPath: table.image), baseURL: tablesURL) : table.image

                var updated = table
                if cleanPath != table.path || cleanImage != table.image {
                    updated = updated.with(path: cleanPath, image: cleanImage)
                }

                var latestMod = updated.modifiedAt
                let tableFileURL = tablesURL.appendingPathComponent(updated.path)
                if let fileMod = fileModifiedAt(tableFileURL) {
                    latestMod = max(latestMod, fileMod)
                }
                if !updated.image.isEmpty {
                    let imageURL = tablesURL.appendingPathComponent(updated.image)
                    if let imageMod = fileModifiedAt(imageURL) {
                        latestMod = max(latestMod, imageMod)
                    }
                }
                if latestMod != updated.modifiedAt {
                    updated = updated.with(modifiedAt: latestMod)
                    bumpedMods += 1
                }

                loadedTables[i] = updated
            }

            var seen = Set<String>()
            loadedTables.removeAll { table in
                if table.uuid.isEmpty || seen.contains(table.uuid) {
                    return true
                }
                seen.insert(table.uuid)

                let tableFileURL = tablesURL.appendingPathComponent(table.path)
                if !TableFileOperations.exists(tableFileURL) {
                    VPinballManager.log(.info, "Removing table with missing file: \(tableFileURL.path)")
                    return true
                }

                return false
            }

            var autoImages = 0
            for i in 0 ..< loadedTables.count {
                if loadedTables[i].image.isEmpty {
                    let table = loadedTables[i]
                    let tableFileURL = tablesURL.appendingPathComponent(table.path)
                    let parentURL = tableFileURL.deletingLastPathComponent()
                    let stem = tableFileURL.deletingPathExtension().lastPathComponent
                    let pngURL = parentURL.appendingPathComponent(stem + ".png")
                    let jpgURL = parentURL.appendingPathComponent(stem + ".jpg")

                    if TableFileOperations.exists(pngURL) {
                        var updated = table.with(image: relativePath(of: pngURL, baseURL: tablesURL))
                        if let imageMod = fileModifiedAt(pngURL) {
                            updated = updated.with(modifiedAt: max(updated.modifiedAt, imageMod))
                        }
                        loadedTables[i] = updated
                        autoImages += 1
                    } else if TableFileOperations.exists(jpgURL) {
                        var updated = table.with(image: relativePath(of: jpgURL, baseURL: tablesURL))
                        if let imageMod = fileModifiedAt(jpgURL) {
                            updated = updated.with(modifiedAt: max(updated.modifiedAt, imageMod))
                        }
                        loadedTables[i] = updated
                        autoImages += 1
                    }
                }
            }

            var seenPaths = Set<String>()
            loadedTables.removeAll { table in
                if seenPaths.contains(table.path) {
                    return true
                }
                seenPaths.insert(table.path)
                return false
            }

            let vpxFiles = TableFileOperations.listFiles(tablesURL,
                                                         ext: ".vpx")
            var existingUUIDs = Set(loadedTables.map { $0.uuid })
            for fileURL in vpxFiles {
                let relPath = relativePath(of: fileURL, baseURL: tablesURL)
                if !loadedTables.contains(where: { $0.path == relPath }) {
                    let newTable = createTable(url: fileURL, existingUUIDs: &existingUUIDs)
                    loadedTables.append(newTable)
                }
            }

            return loadedTables
        }.value

        tables = loadedTables
        saveTables()
    }

    private func saveTables() {
        let sorted = tables.sorted { $0.name.localizedCaseInsensitiveCompare($1.name) == .orderedAscending }
        let response = TablesResponse(tableCount: sorted.count,
                                      tables: sorted)

        let encoder = JSONEncoder()
        encoder.outputFormatting = [.prettyPrinted,
                                    .withoutEscapingSlashes]

        if let jsonData = try? encoder.encode(response),
           let jsonString = String(data: jsonData,
                                   encoding: .utf8)
        {
            _ = TableFileOperations.write(tablesJSONURL,
                                          content: jsonString)
        }
    }

    private func createTable(url: URL) -> Table {
        let uuid = generateUUID()
        let relPath = relativePath(of: url)

        var name = url.deletingPathExtension().lastPathComponent
        name = name.replacingOccurrences(of: "_",
                                         with: " ")

        let now = Int64(Date().timeIntervalSince1970)

        let parentURL = url.deletingLastPathComponent()
        let stem = url.deletingPathExtension().lastPathComponent
        let pngURL = parentURL.appendingPathComponent(stem + ".png")
        let jpgURL = parentURL.appendingPathComponent(stem + ".jpg")

        var image = ""
        if TableFileOperations.exists(pngURL) {
            image = relativePath(of: pngURL)
        } else if TableFileOperations.exists(jpgURL) {
            image = relativePath(of: jpgURL)
        }

        return Table(uuid: uuid,
                     name: name,
                     path: relPath,
                     image: image,
                     createdAt: now,
                     modifiedAt: now)
    }

    private func findTable(url: URL) -> Table? {
        let relPath = relativePath(of: url)
        return tables.first { $0.path == relPath }
    }

    private func generateUUID() -> String {
        var uuid = UUID().uuidString.lowercased()
        while tables.contains(where: { $0.uuid == uuid }) {
            uuid = UUID().uuidString.lowercased()
            VPinballManager.log(.warn, "UUID collision detected: \(uuid), regenerating")
        }
        return uuid
    }

    private func sanitizeName(_ name: String) -> String {
        let invalidChars = CharacterSet(charactersIn: " _/\\:*?\"<>|.&'()")
        let sanitized = name.components(separatedBy: invalidChars).joined(separator: "-")

        var result = ""
        var lastWasHyphen = false
        for char in sanitized {
            if char == "-" {
                if !lastWasHyphen {
                    result.append(char)
                    lastWasHyphen = true
                }
            } else {
                result.append(char)
                lastWasHyphen = false
            }
        }

        while result.hasPrefix("-") {
            result.removeFirst()
        }
        while result.hasSuffix("-") {
            result.removeLast()
        }

        return result.isEmpty ? "table" : result
    }

    private func getUniqueFolder(baseName: String) -> String {
        let sanitized = sanitizeName(baseName)
        var candidate = sanitized
        var counter = 2

        while TableFileOperations.exists(tablesURL.appendingPathComponent(candidate)) {
            candidate = "\(sanitized)-\(counter)"
            counter += 1
        }

        return candidate
    }

    private func coordinatedCopy(from sourceURL: URL, to destURL: URL) async -> Bool {
        await Task.detached {
            var coordinatorError: NSError?
            var copySuccess = false

            let coordinator = NSFileCoordinator()
            coordinator.coordinate(readingItemAt: sourceURL, options: [], error: &coordinatorError) { coordinatedURL in
                do {
                    if FileManager.default.fileExists(atPath: destURL.path) {
                        try FileManager.default.removeItem(at: destURL)
                    }
                    try FileManager.default.copyItem(at: coordinatedURL, to: destURL)
                    copySuccess = true
                } catch {}
            }

            return copySuccess
        }.value
    }

    private func relativePath(of url: URL) -> String {
        let fullPath = url.resolvingSymlinksInPath().path
        let basePath = tablesURL.path

        if fullPath.isEmpty || basePath.isEmpty {
            return fullPath
        }

        if !fullPath.hasPrefix(basePath) {
            return fullPath
        }

        var result = String(fullPath.dropFirst(basePath.count))
        if result.hasPrefix("/") {
            result = String(result.dropFirst())
        }

        return result
    }

    private func performImport(url: URL, onProgress: ((Int) async -> Void)? = nil) async -> Table? {
        if !TableFileOperations.exists(url) {
            return nil
        }

        let ext = url.pathExtension.lowercased()

        if ext == "vpxz" || ext == "zip" {
            if let destVpxFile = await importArchive(url: url,
                                                     onProgress: onProgress)
            {
                let newTable = createTable(url: destVpxFile)
                tables.append(newTable)
                saveTables()
                return newTable
            }
            return nil
        } else if ext == "vpx" {
            if let destVpxFile = await importVPX(url: url) {
                let newTable = createTable(url: destVpxFile)
                tables.append(newTable)
                saveTables()
                return newTable
            }
            return nil
        }

        return nil
    }

    private func importVPX(url: URL) async -> URL? {
        let stem = url.deletingPathExtension().lastPathComponent
        let name = stem.replacingOccurrences(of: "_", with: " ")
        let folderName = getUniqueFolder(baseName: name)
        let destFolder = tablesURL.appendingPathComponent(folderName)
        let destFile = destFolder.appendingPathComponent(url.lastPathComponent)

        let result = await Task.detached(priority: .userInitiated) {
            if !TableFileOperations.createDirectory(destFolder) {
                return false
            }
            if !TableFileOperations.copy(from: url,
                                         to: destFile)
            {
                return false
            }
            return true
        }.value

        return result ? destFile : nil
    }

    private func importArchive(url: URL, onProgress: ((Int) async -> Void)? = nil) async -> URL? {
        let tempDir = FileManager.default.temporaryDirectory
            .appendingPathComponent("vpinball_import_\(Int(Date().timeIntervalSince1970))")

        if !TableFileOperations.createDirectory(tempDir) {
            return nil
        }

        defer {
            _ = TableFileOperations.deleteDirectory(tempDir)
        }

        zipProgressHandler = { current, total in
            if total > 0 {
                let extractProgress = 60 + (current * 35 / total)
                Task { @MainActor in
                    await onProgress?(extractProgress)
                }
            }
        }

        let result = await Task.detached {
            VPinballZipExtract(url.path.cstring, tempDir.path.cstring, zipCallback)
        }.value
        zipProgressHandler = nil

        if VPinballStatus(rawValue: result) != .success {
            return nil
        }

        let vpxFiles = await Task.detached(priority: .userInitiated) {
            TableFileOperations.listFiles(tempDir, ext: ".vpx")
        }.value

        if vpxFiles.count != 1 {
            MainViewModel.shared.handleShowError(message: vpxFiles.isEmpty
                ? "No tables found in archive"
                : "Archive contains multiple tables")
            return nil
        }

        let vpxURL = vpxFiles[0]
        let vpxName = vpxURL.deletingPathExtension().lastPathComponent
        let sourceDir = vpxURL.deletingLastPathComponent()

        let folderName = getUniqueFolder(baseName: vpxName)
        let destFolder = tablesURL.appendingPathComponent(folderName)
        let destVpxFile = destFolder.appendingPathComponent(vpxURL.lastPathComponent)

        let copied = await Task.detached(priority: .userInitiated) {
            if !TableFileOperations.createDirectory(destFolder) {
                return false
            }
            if !TableFileOperations.copyDirectory(from: sourceDir,
                                                  to: destFolder)
            {
                return false
            }
            return true
        }.value

        return copied ? destVpxFile : nil
    }

    private func performRename(table: Table, newName: String) -> Bool {
        if let index = tables.firstIndex(where: { $0.uuid == table.uuid }) {
            let now = Int64(Date().timeIntervalSince1970)
            tables[index] = tables[index].with(name: newName, modifiedAt: now)
            saveTables()
            return true
        }
        return false
    }

    private func performExport(table: Table, onProgress: ((Int, String) async -> Void)? = nil) async -> String? {
        let sanitizedName = sanitizeName(table.name)
        let tempURL = FileManager.default.temporaryDirectory.appendingPathComponent(sanitizedName + ".vpxz")
        let tableDirURL = table.baseURL

        await onProgress?(60, "Compressing")

        zipProgressHandler = { current, total in
            let adjustedProgress = 60 + (current * 39 / max(total, 1))
            Task { @MainActor in
                await onProgress?(adjustedProgress, "Compressing")
            }
        }

        let result = await Task.detached {
            if TableFileOperations.exists(tempURL) {
                _ = TableFileOperations.delete(tempURL)
            }

            return VPinballZipCreate(tableDirURL.path.cstring, tempURL.path.cstring, zipCallback)
        }.value

        zipProgressHandler = nil

        if VPinballStatus(rawValue: result) != .success {
            return nil
        }

        await onProgress?(100, "Complete")
        return tempURL.path
    }
}
