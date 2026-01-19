import SwiftUI
import UniformTypeIdentifiers

private var zipProgressHandler: ((Int, Int) -> Void)?

private let zipCallback: VPinballZipCallback = { current, total, _ in
    zipProgressHandler?(Int(current), Int(total))
}

@MainActor
class TableManager: ObservableObject {
    static let shared = TableManager()

    @Published private(set) var tables: [Table] = []

    private var tablesPath: String = ""
    private var tablesJSONPath: String = ""
    private var loadedTable: Table?

    private let vpinballViewModel = VPinballViewModel.shared

    init() {
        loadTablesPath()
        loadTables()
    }

    func refresh() async {
        await Task {
            loadTables()
        }.value
    }

    func getTable(uuid: String) -> Table? {
        return tables.first { $0.uuid == uuid }
    }

    func importTable(from url: URL) async -> Bool {
        let ext = url.pathExtension.lowercased()
        let fileName = url.lastPathComponent.removingPercentEncoding!

        vpinballViewModel.showProgressHUD(
            title: fileName,
            status: "Copying file"
        )

        try? await Task.sleep(nanoseconds: 100_000_000)

        let isSecurityScoped = url.startAccessingSecurityScopedResource()
        defer {
            if isSecurityScoped {
                url.stopAccessingSecurityScopedResource()
            }
        }

        await MainActor.run {
            self.vpinballViewModel.updateProgressHUD(
                progress: 60,
                status: ext == "vpx" ? "Importing table" : "Importing archive"
            )
        }

        try? await Task.sleep(nanoseconds: 100_000_000)

        let importedTables = await performImport(path: url.path) { progress in
            await MainActor.run {
                self.vpinballViewModel.updateProgressHUD(
                    progress: progress,
                    status: ext == "vpx" ? "Importing table" : "Importing archive"
                )
            }
        }

        await MainActor.run {
            self.vpinballViewModel.updateProgressHUD(
                progress: 100,
                status: "Import complete"
            )
        }

        try? await Task.sleep(nanoseconds: 500_000_000)

        vpinballViewModel.hideHUD()

        if let firstTable = importedTables.first {
            vpinballViewModel.scrollToTable = firstTable
        }

        return !importedTables.isEmpty
    }

    func deleteTable(table: Table) async -> Bool {
        return await Task {
            performDelete(table: table)
        }.value
    }

    func renameTable(table: Table, newName: String) async -> Bool {
        return await Task {
            performRename(table: table, newName: newName)
        }.value
    }

    func setTableImage(table: Table, imagePath: String) async -> Bool {
        return await Task {
            performSetImage(table: table, imagePath: imagePath)
        }.value
    }

    func setTableImage(table: Table, image: UIImage) async -> Bool {
        return await Task {
            performSetImageFromUIImage(table: table, image: image)
        }.value
    }

    func exportTable(table: Table) async -> String? {
        vpinballViewModel.showProgressHUD(title: table.name, status: "Compressing")

        try? await Task.sleep(nanoseconds: 100_000_000)

        let exportPath = await performExport(table: table) { progress, status in
            await MainActor.run {
                self.vpinballViewModel.updateProgressHUD(progress: progress, status: status)
            }
        }

        try? await Task.sleep(nanoseconds: 500_000_000)

        vpinballViewModel.hideHUD()

        return exportPath
    }

    func getLoadedTablePath(table: Table) -> String? {
        return getLoadedTablePathSync(table: table)
    }

    func clearLoadedTable(table: Table) {
        clearLoadedTableSync(table: table)
    }

    func extractTableScript(table: Table) async -> Bool {
        vpinballViewModel.showProgressHUD(title: table.name, status: "Extracting Script")

        try? await Task.sleep(nanoseconds: 100_000_000)

        let result = await Task {
            performExtractScript(table: table)
        }.value

        try? await Task.sleep(nanoseconds: 500_000_000)

        vpinballViewModel.hideHUD()

        return result
    }

    func resetTableIni(table: Table) -> Bool {
        let fullPath = (tablesPath as NSString).appendingPathComponent(table.path)
        let tableDir = (fullPath as NSString).deletingLastPathComponent
        let baseName = (fullPath as NSString).deletingPathExtension.components(separatedBy: "/").last ?? ""
        let iniPath = (tableDir as NSString).appendingPathComponent(baseName + ".ini")

        return TableFileOperations.delete(iniPath)
    }

    private func loadTablesPath() {
        tablesPath = VPinballManager.shared.getPath(.tables)
        tablesJSONPath = (VPinballManager.shared.getPath(.preferences) as NSString).appendingPathComponent("tables.json")

        if !TableFileOperations.exists(tablesPath) {
            _ = TableFileOperations.createDirectory(tablesPath)
        }
    }

    func loadTables() {
        var loadedTables: [Table] = []

        if TableFileOperations.exists(tablesJSONPath) {
            if let content = TableFileOperations.read(tablesJSONPath),
               let jsonData = content.data(using: .utf8)
            {
                if let response = try? JSONDecoder().decode(TablesResponse.self, from: jsonData) {
                    loadedTables = response.tables
                }
            }
        }

        var seen = Set<String>()
        loadedTables.removeAll { table in
            if table.uuid.isEmpty || seen.contains(table.uuid) {
                return true
            }
            seen.insert(table.uuid)

            let fullPath = (tablesPath as NSString).appendingPathComponent(table.path)
            if !TableFileOperations.exists(fullPath) {
                VPinballManager.log(.info, "Removing table with missing file: \(fullPath)")
                return true
            }

            return false
        }

        for i in 0 ..< loadedTables.count {
            if loadedTables[i].image.isEmpty {
                let fullPath = (tablesPath as NSString).appendingPathComponent(loadedTables[i].path)
                let parentPath = (fullPath as NSString).deletingLastPathComponent
                let stem = (fullPath as NSString).deletingPathExtension.components(separatedBy: "/").last ?? ""
                let pngPath = (parentPath as NSString).appendingPathComponent(stem + ".png")
                let jpgPath = (parentPath as NSString).appendingPathComponent(stem + ".jpg")

                if TableFileOperations.exists(pngPath) {
                    loadedTables[i] = Table(
                        uuid: loadedTables[i].uuid,
                        name: loadedTables[i].name,
                        path: loadedTables[i].path,
                        image: relativePath(fullPath: pngPath, basePath: tablesPath),
                        createdAt: loadedTables[i].createdAt,
                        modifiedAt: loadedTables[i].modifiedAt
                    )
                } else if TableFileOperations.exists(jpgPath) {
                    loadedTables[i] = Table(
                        uuid: loadedTables[i].uuid,
                        name: loadedTables[i].name,
                        path: loadedTables[i].path,
                        image: relativePath(fullPath: jpgPath, basePath: tablesPath),
                        createdAt: loadedTables[i].createdAt,
                        modifiedAt: loadedTables[i].modifiedAt
                    )
                }
            }
        }

        let vpxFiles = TableFileOperations.listFiles(tablesPath, ext: ".vpx")
        for filePath in vpxFiles {
            if !loadedTables.contains(where: { (tablesPath as NSString).appendingPathComponent($0.path) == filePath }) {
                let newTable = createTable(path: filePath)
                loadedTables.append(newTable)
            }
        }

        tables = loadedTables
        saveTables()
    }

    private func saveTables() {
        let sorted = tables.sorted { $0.name.localizedCaseInsensitiveCompare($1.name) == .orderedAscending }
        let response = TablesResponse(tableCount: sorted.count, tables: sorted)

        let encoder = JSONEncoder()
        encoder.outputFormatting = [.prettyPrinted, .withoutEscapingSlashes]

        if let jsonData = try? encoder.encode(response),
           let jsonString = String(data: jsonData, encoding: .utf8)
        {
            _ = TableFileOperations.write(tablesJSONPath, content: jsonString)
        }
    }

    private func createTable(path: String) -> Table {
        let uuid = generateUUID()
        let relativePath = self.relativePath(fullPath: path, basePath: tablesPath)

        var name = (path as NSString).deletingPathExtension.components(separatedBy: "/").last ?? ""
        name = name.replacingOccurrences(of: "_", with: " ")

        let now = Int64(Date().timeIntervalSince1970)

        let parentPath = (path as NSString).deletingLastPathComponent
        let stem = (path as NSString).deletingPathExtension.components(separatedBy: "/").last ?? ""
        let pngPath = (parentPath as NSString).appendingPathComponent(stem + ".png")
        let jpgPath = (parentPath as NSString).appendingPathComponent(stem + ".jpg")

        var image = ""
        if TableFileOperations.exists(pngPath) {
            image = self.relativePath(fullPath: pngPath, basePath: tablesPath)
        } else if TableFileOperations.exists(jpgPath) {
            image = self.relativePath(fullPath: jpgPath, basePath: tablesPath)
        }

        return Table(uuid: uuid, name: name, path: relativePath, image: image, createdAt: now, modifiedAt: now)
    }

    private func findTable(path: String) -> Table? {
        return tables.first { (tablesPath as NSString).appendingPathComponent($0.path) == path }
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

        while TableFileOperations.exists((tablesPath as NSString).appendingPathComponent(candidate)) {
            candidate = "\(sanitized)-\(counter)"
            counter += 1
        }

        return candidate
    }

    private func relativePath(fullPath: String, basePath: String) -> String {
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

    private func performImport(path: String, onProgress: ((Int) async -> Void)? = nil) async -> [Table] {
        if !TableFileOperations.exists(path) {
            return []
        }

        let ext = (path as NSString).pathExtension.lowercased()

        if ext == "vpxz" || ext == "zip" {
            return await importArchive(path: path, onProgress: onProgress)
        } else if ext == "vpx" {
            if let table = importVPX(path: path) {
                return [table]
            }
        }

        return []
    }

    private func importVPX(path: String) -> Table? {
        let stem = (path as NSString).deletingPathExtension.components(separatedBy: "/").last ?? ""
        let name = stem.replacingOccurrences(of: "_", with: " ")

        let folderName = getUniqueFolder(baseName: name)
        let destFolder = (tablesPath as NSString).appendingPathComponent(folderName)

        if !TableFileOperations.createDirectory(destFolder) {
            return nil
        }

        let fileName = (path as NSString).lastPathComponent
        let destFile = (destFolder as NSString).appendingPathComponent(fileName)

        if !TableFileOperations.copy(from: path, to: destFile) {
            return nil
        }

        let newTable = createTable(path: destFile)
        tables.append(newTable)
        saveTables()
        VPinballManager.log(.info, "Successfully imported table: \(name)")
        return newTable
    }

    private func importArchive(path: String, onProgress: ((Int) async -> Void)? = nil) async -> [Table] {
        let tempDir = (NSTemporaryDirectory() as NSString).appendingPathComponent("vpinball_import_\(Int(Date().timeIntervalSince1970))")

        if !TableFileOperations.createDirectory(tempDir) {
            return []
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
            VPinballZipExtract(path.cstring, tempDir.cstring, zipCallback)
        }.value
        zipProgressHandler = nil

        if VPinballStatus(rawValue: result) != .success {
            VPinballManager.log(.error, "Archive extraction failed")
            return []
        }

        VPinballManager.log(.info, "Archive extraction complete")

        let vpxFiles = TableFileOperations.listFiles(tempDir, ext: ".vpx")

        if vpxFiles.isEmpty {
            return []
        }

        var newTables: [Table] = []

        for vpxFile in vpxFiles {
            let vpxName = (vpxFile as NSString).deletingPathExtension.components(separatedBy: "/").last ?? ""
            let sourceDir = (vpxFile as NSString).deletingLastPathComponent
            let fileName = (vpxFile as NSString).lastPathComponent

            let folderName = getUniqueFolder(baseName: vpxName)
            let destFolder = (tablesPath as NSString).appendingPathComponent(folderName)

            if !TableFileOperations.createDirectory(destFolder) {
                continue
            }

            if !TableFileOperations.copyDirectory(from: sourceDir, to: destFolder) {
                continue
            }

            let destVpxFile = (destFolder as NSString).appendingPathComponent(fileName)
            let newTable = createTable(path: destVpxFile)
            newTables.append(newTable)
            VPinballManager.log(.info, "Successfully imported table from archive: \(vpxName)")
        }

        if !newTables.isEmpty {
            tables.append(contentsOf: newTables)
            saveTables()
        }

        return newTables
    }

    private func performDelete(table: Table) -> Bool {
        let tablePath = (tablesPath as NSString).appendingPathComponent(table.path)
        let tableDir = (tablePath as NSString).deletingLastPathComponent

        if tableDir.isEmpty || tableDir == tablesPath || tableDir + "/" == tablesPath {
            return false
        }

        if TableFileOperations.exists(tableDir) {
            let vpxFiles = TableFileOperations.listFiles(tableDir, ext: ".vpx")

            if vpxFiles.count <= 1 {
                if !TableFileOperations.deleteDirectory(tableDir) {
                    return false
                }
            } else {
                if !TableFileOperations.delete(tablePath) {
                    return false
                }
            }
        }

        tables.removeAll { $0.uuid == table.uuid }
        saveTables()

        return true
    }

    private func performRename(table: Table, newName: String) -> Bool {
        if let index = tables.firstIndex(where: { $0.uuid == table.uuid }) {
            let now = Int64(Date().timeIntervalSince1970)
            tables[index] = Table(
                uuid: tables[index].uuid,
                name: newName,
                path: tables[index].path,
                image: tables[index].image,
                createdAt: tables[index].createdAt,
                modifiedAt: now
            )

            saveTables()
            return true
        } else {
            return false
        }
    }

    private func performSetImage(table: Table, imagePath: String) -> Bool {
        if let index = tables.firstIndex(where: { $0.uuid == table.uuid }) {
            if imagePath.isEmpty {
                if !table.image.isEmpty {
                    let currentImagePath = (tablesPath as NSString).appendingPathComponent(table.image)
                    if TableFileOperations.exists(currentImagePath) {
                        _ = TableFileOperations.delete(currentImagePath)
                    }
                }

                let now = Int64(Date().timeIntervalSince1970)
                tables[index] = Table(
                    uuid: table.uuid,
                    name: table.name,
                    path: table.path,
                    image: "",
                    createdAt: table.createdAt,
                    modifiedAt: now
                )

                saveTables()
                return true
            }

            if imagePath.hasPrefix("/") {
                if let sourceImage = UIImage(contentsOfFile: imagePath) {
                    if let jpegData = sourceImage.jpegData(compressionQuality: 0.8) {
                        let fullPath = (tablesPath as NSString).appendingPathComponent(table.path)
                        let baseName = (fullPath as NSString).deletingPathExtension.components(separatedBy: "/").last ?? ""

                        let workingDir: String
                        if let loadedTable = loadedTable, loadedTable.uuid == table.uuid {
                            let loadedPath = (tablesPath as NSString).appendingPathComponent(loadedTable.path)
                            workingDir = (loadedPath as NSString).deletingLastPathComponent
                        } else {
                            workingDir = (fullPath as NSString).deletingLastPathComponent
                        }

                        let destPath = (workingDir as NSString).appendingPathComponent(baseName + ".jpg")

                        do {
                            try jpegData.write(to: URL(fileURLWithPath: destPath))
                        } catch {
                            return false
                        }

                        if imagePath != destPath && (imagePath as NSString).pathExtension.lowercased() == "png" {
                            _ = TableFileOperations.delete(imagePath)
                        }

                        let relPath = relativePath(fullPath: destPath, basePath: tablesPath)
                        let now = Int64(Date().timeIntervalSince1970)

                        tables[index] = Table(
                            uuid: table.uuid,
                            name: table.name,
                            path: table.path,
                            image: relPath,
                            createdAt: table.createdAt,
                            modifiedAt: now
                        )

                        saveTables()
                        return true
                    } else {
                        return false
                    }
                } else {
                    return false
                }
            }

            let now = Int64(Date().timeIntervalSince1970)
            tables[index] = Table(
                uuid: table.uuid,
                name: table.name,
                path: table.path,
                image: imagePath,
                createdAt: table.createdAt,
                modifiedAt: now
            )

            saveTables()
            return true
        } else {
            return false
        }
    }

    private func performSetImageFromUIImage(table: Table, image: UIImage) -> Bool {
        if let index = tables.firstIndex(where: { $0.uuid == table.uuid }) {
            if let jpegData = image.jpegData(compressionQuality: 0.8) {
                let fullPath = (tablesPath as NSString).appendingPathComponent(table.path)
                let baseName = (fullPath as NSString).deletingPathExtension.components(separatedBy: "/").last ?? ""

                let workingDir: String
                if let loadedTable = loadedTable, loadedTable.uuid == table.uuid {
                    let loadedPath = (tablesPath as NSString).appendingPathComponent(loadedTable.path)
                    workingDir = (loadedPath as NSString).deletingLastPathComponent
                } else {
                    workingDir = (fullPath as NSString).deletingLastPathComponent
                }

                let destPath = (workingDir as NSString).appendingPathComponent(baseName + ".jpg")

                do {
                    try jpegData.write(to: URL(fileURLWithPath: destPath))
                } catch {
                    return false
                }

                let relPath = relativePath(fullPath: destPath, basePath: tablesPath)
                let now = Int64(Date().timeIntervalSince1970)

                tables[index] = Table(
                    uuid: table.uuid,
                    name: table.name,
                    path: table.path,
                    image: relPath,
                    createdAt: table.createdAt,
                    modifiedAt: now
                )

                saveTables()
                return true
            } else {
                return false
            }
        } else {
            return false
        }
    }

    private func performExport(table: Table, onProgress: ((Int, String) async -> Void)? = nil) async -> String? {
        let sanitizedName = sanitizeName(table.name)
        let tempFile = (NSTemporaryDirectory() as NSString).appendingPathComponent(sanitizedName + ".vpxz")

        if TableFileOperations.exists(tempFile) {
            _ = TableFileOperations.delete(tempFile)
        }

        let fullPath = (tablesPath as NSString).appendingPathComponent(table.path)
        let tableDirToCompress = (fullPath as NSString).deletingLastPathComponent

        await onProgress?(60, "Compressing")

        zipProgressHandler = { current, total in
            let adjustedProgress = 60 + (current * 39 / max(total, 1))
            Task { @MainActor in
                await onProgress?(adjustedProgress, "Compressing")
            }
        }

        let result = await Task.detached {
            VPinballZipCreate(tableDirToCompress.cstring, tempFile.cstring, zipCallback)
        }.value
        zipProgressHandler = nil

        if VPinballStatus(rawValue: result) != .success {
            return nil
        }

        await onProgress?(100, "Complete")
        return tempFile
    }

    private func getLoadedTablePathSync(table: Table) -> String? {
        if table.path.isEmpty {
            return nil
        }

        loadedTable = table
        let fullPath = (tablesPath as NSString).appendingPathComponent(table.path)
        return fullPath
    }

    private func clearLoadedTableSync(table: Table) {
        if loadedTable?.uuid != table.uuid {
            return
        }

        loadedTable = nil
    }

    private func performExtractScript(table: Table) -> Bool {
        let fullPath = (tablesPath as NSString).appendingPathComponent(table.path)
        let tableDir = (fullPath as NSString).deletingLastPathComponent
        let baseName = (fullPath as NSString).deletingPathExtension.components(separatedBy: "/").last ?? ""
        let scriptPath = (tableDir as NSString).appendingPathComponent(baseName + ".vbs")

        if TableFileOperations.exists(scriptPath) {
            return true
        }

        if VPinballStatus(rawValue: VPinballLoadTable(fullPath.cstring)) != .success {
            VPinballManager.log(.error, "Failed to load table for script extraction: \(fullPath)")
            return false
        }

        if VPinballStatus(rawValue: VPinballExtractTableScript()) != .success {
            VPinballManager.log(.error, "Failed to extract script from table")
            return false
        }

        if TableFileOperations.exists(scriptPath) {
            VPinballManager.log(.info, "Successfully extracted script: \(scriptPath)")
            return true
        }

        VPinballManager.log(.warn, "Script file not found after extraction")
        return false
    }

    func filteredTables(searchText: String, sortOrder: SortOrder) -> [Table] {
        var filtered = tables

        if !searchText.isEmpty {
            filtered = filtered.filter { table in
                table.name.localizedCaseInsensitiveContains(searchText)
            }
        }

        filtered = filtered.sorted { table1, table2 in
            if sortOrder == .forward {
                return table1.name.localizedCaseInsensitiveCompare(table2.name) == .orderedAscending
            } else {
                return table1.name.localizedCaseInsensitiveCompare(table2.name) == .orderedDescending
            }
        }

        return filtered
    }
}
