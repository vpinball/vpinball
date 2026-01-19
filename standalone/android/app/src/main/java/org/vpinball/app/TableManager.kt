package org.vpinball.app

import android.annotation.SuppressLint
import android.content.Context
import android.net.Uri
import java.io.File
import java.util.UUID
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.runBlocking
import kotlinx.coroutines.withContext
import kotlinx.serialization.Serializable
import kotlinx.serialization.json.Json
import org.vpinball.app.jni.VPinballLogLevel
import org.vpinball.app.jni.VPinballPath
import org.vpinball.app.jni.VPinballSettingsSection.STANDALONE
import org.vpinball.app.jni.VPinballStatus
import org.vpinball.app.jni.VPinballZipCallback
import org.vpinball.app.ui.screens.landing.LandingScreenViewModel

class TableManager(private val context: Context) {
    private val _tables = MutableStateFlow<List<Table>>(emptyList())
    val tables: StateFlow<List<Table>> = _tables.asStateFlow()

    private var safPath: String = ""
    private var tablesPath: String = ""
    private var tablesJSONPath: String = ""
    private var requiresStaging: Boolean = false
    private var loadedTable: Table? = null
    private var loadedTableWorkingDir: String = ""
    private val fileOps = TableFileOperations { tablesPath }

    init {
        loadPaths()
    }

    suspend fun refresh(onProgress: ((Int, String) -> Unit)? = null) {
        withContext(Dispatchers.IO) {
            loadPaths()
            loadTables(onProgress)
        }
    }

    fun getTable(uuid: String): Table? {
        return _tables.value.firstOrNull { it.uuid == uuid }
    }

    suspend fun reloadTableImage(table: Table): Table? {
        return withContext(Dispatchers.IO) {
            val tableDir = File(table.path).parent ?: return@withContext null
            val stem = File(table.path).nameWithoutExtension

            val imagePath = if (tableDir.isNotEmpty()) "$tableDir/$stem" else stem
            val pngPath = buildPath("$imagePath.png")
            val jpgPath = buildPath("$imagePath.jpg")

            val updatedImage =
                when {
                    fileOps.exists(pngPath) -> relativePath(pngPath, tablesPath)
                    fileOps.exists(jpgPath) -> relativePath(jpgPath, tablesPath)
                    else -> table.image
                }

            if (updatedImage != table.image) {
                val updatedTable = table.copy(image = updatedImage, modifiedAt = System.currentTimeMillis() / 1000)
                _tables.value = _tables.value.map { if (it.uuid == table.uuid) updatedTable else it }
                saveTables()
                updatedTable
            } else {
                table
            }
        }
    }

    suspend fun importTable(uri: Uri, onProgress: (Int, String) -> Unit): Boolean {
        return withContext(Dispatchers.IO) {
            val fileName = fileOps.getFileName(context, uri)
            if (fileName == null) {
                VPinballManager.log(VPinballLogLevel.ERROR, "Failed to get filename from URI")
                withContext(Dispatchers.Main) { VPinballManager.showError("Failed to get filename from file.") }
                return@withContext false
            }

            val tempFile = File(context.cacheDir, fileName)

            try {
                withContext(Dispatchers.Main) { onProgress(20, "Copying file") }

                delay(100)

                val copyResult = fileOps.copy(context, uri, tempFile)
                if (!copyResult) {
                    VPinballManager.log(VPinballLogLevel.ERROR, "Failed to copy file")
                    withContext(Dispatchers.Main) { VPinballManager.showError("Failed to copy file.") }
                    return@withContext false
                }

                val ext = tempFile.extension.lowercase()
                val isArchive = ext == "vpxz" || ext == "zip"
                val importMessage = if (isArchive) "Importing archive" else "Importing table"

                withContext(Dispatchers.Main) { onProgress(60, importMessage) }

                delay(100)

                val newTables =
                    performImport(tempFile.absolutePath) { progress -> withContext(Dispatchers.Main) { onProgress(progress, importMessage) } }
                tempFile.delete()

                if (newTables.isEmpty()) {
                    withContext(Dispatchers.Main) {
                        VPinballManager.showError("Failed to import table. Check that the file is a valid VPX or VPXZ file.")
                    }
                    return@withContext false
                }

                _tables.value = _tables.value + newTables
                saveTables()
                withContext(Dispatchers.Main) {
                    onProgress(100, "Import complete")
                    LandingScreenViewModel.triggerAddTables(newTables)
                }

                delay(500)

                true
            } catch (e: Exception) {
                VPinballManager.log(VPinballLogLevel.ERROR, "Import error: ${e.message}")
                tempFile.delete()
                withContext(Dispatchers.Main) { VPinballManager.showError("Import failed: ${e.message}") }
                false
            }
        }
    }

    suspend fun deleteTable(table: Table, onProgress: ((Int, String) -> Unit)? = null): Boolean {
        return withContext(Dispatchers.IO) {
            val result = performDelete(table, onProgress)
            if (result) {
                withContext(Dispatchers.Main) { LandingScreenViewModel.triggerRemoveTable(table) }
            }
            result
        }
    }

    suspend fun renameTable(table: Table, newName: String): Boolean {
        return withContext(Dispatchers.IO) {
            val updatedTable = performRename(table, newName)
            withContext(Dispatchers.Main) { LandingScreenViewModel.triggerUpdateTable(updatedTable) }
            true
        }
    }

    suspend fun setTableImage(table: Table, imagePath: String): Boolean {
        return withContext(Dispatchers.IO) {
            val updatedTable = performSetImage(table, imagePath)
            if (updatedTable != null) {
                withContext(Dispatchers.Main) { LandingScreenViewModel.triggerUpdateTable(updatedTable) }
                true
            } else {
                false
            }
        }
    }

    suspend fun exportTable(table: Table, onProgress: ((Int, String) -> Unit)? = null): String? {
        return withContext(Dispatchers.IO) { performExport(table, onProgress) }
    }

    suspend fun stageTable(table: Table, onProgress: ((Int, String) -> Unit)? = null): String? {
        return withContext(Dispatchers.IO) { performStage(table, onProgress) }
    }

    suspend fun cleanupLoadedTable(table: Table, onProgress: ((Int, String) -> Unit)? = null) {
        withContext(Dispatchers.IO) { performCleanup(table, onProgress) }
    }

    suspend fun extractTableScript(table: Table, onProgress: ((Int, String) -> Unit)? = null): Boolean {
        return withContext(Dispatchers.IO) { performExtractScript(table, onProgress) }
    }

    fun resetTableIni(table: Table): Boolean {
        loadPaths()
        val iniRelativePath = table.path.substringBeforeLast('.') + ".ini"
        val iniFullPath = buildPath(iniRelativePath)
        return fileOps.delete(iniFullPath)
    }

    private fun loadPaths() {
        safPath = VPinballManager.loadValue(STANDALONE, "SAFPath", "")

        tablesPath =
            if (safPath.isNotEmpty()) {
                if (!safPath.endsWith("/")) "$safPath/" else safPath
            } else {
                val path = VPinballManager.getPath(VPinballPath.TABLES)
                if (!path.endsWith("/")) "$path/" else path
            }

        requiresStaging = safPath.isNotEmpty()

        tablesJSONPath =
            if (safPath.isEmpty()) {
                File(VPinballManager.getPath(VPinballPath.PREFERENCES), "tables.json").absolutePath
            } else {
                "${tablesPath}tables.json"
            }

        if (!requiresStaging && !fileOps.exists(tablesPath)) {
            fileOps.createDirectory(tablesPath)
        }
    }

    private fun loadTables(onProgress: ((Int, String) -> Unit)? = null) {
        var loadedTables = mutableListOf<Table>()

        onProgress?.invoke(10, "Loading tables...")

        val jsonExists = fileOps.exists(tablesJSONPath)

        if (jsonExists) {
            val content = fileOps.read(tablesJSONPath)
            if (content != null) {
                try {
                    @Serializable data class TablesResponse(val tableCount: Int, val tables: List<Table>)
                    val response = Json.decodeFromString<TablesResponse>(content)
                    loadedTables = response.tables.toMutableList()
                } catch (e: Exception) {
                    VPinballManager.log(VPinballLogLevel.ERROR, "Failed to parse tables.json: ${e.message}")
                }
            }
        }

        onProgress?.invoke(30, "Validating tables...")

        val seen = mutableSetOf<String>()
        loadedTables.removeAll { table ->
            if (table.uuid.isEmpty() || seen.contains(table.uuid)) {
                return@removeAll true
            }
            seen.add(table.uuid)

            val fullPath = buildPath(table.path)
            if (!fileOps.exists(fullPath)) {
                return@removeAll true
            }

            false
        }

        onProgress?.invoke(50, "Scanning for images...")

        loadedTables.forEachIndexed { index, table ->
            if (table.image.isEmpty()) {
                val tableDir = File(table.path).parent ?: ""
                val stem = File(table.path).nameWithoutExtension

                val imagePath =
                    if (tableDir.isNotEmpty()) {
                        "$tableDir/$stem"
                    } else {
                        stem
                    }

                val pngPath = buildPath("$imagePath.png")
                val jpgPath = buildPath("$imagePath.jpg")

                val updatedImage =
                    when {
                        fileOps.exists(pngPath) -> relativePath(pngPath, tablesPath)
                        fileOps.exists(jpgPath) -> relativePath(jpgPath, tablesPath)
                        else -> table.image
                    }

                if (updatedImage != table.image) {
                    loadedTables[index] = table.copy(image = updatedImage)
                }
            }
        }

        onProgress?.invoke(70, "Scanning for tables...")
        val vpxFiles = fileOps.listFiles(tablesPath, ".vpx")

        onProgress?.invoke(90, "Finalizing...")
        for (filePath in vpxFiles) {
            if (!loadedTables.any { buildPath(it.path) == filePath }) {
                createTable(filePath).let { loadedTables.add(it) }
            }
        }

        _tables.value = loadedTables
        saveTables()
        onProgress?.invoke(100, "Complete")
    }

    private fun saveTables() {
        val sorted = _tables.value.sortedBy { it.name.lowercase() }

        @Serializable data class TablesResponse(val tableCount: Int, val tables: List<Table>)

        val response = TablesResponse(tableCount = sorted.size, tables = sorted)

        try {
            val json = Json {
                prettyPrint = true
                encodeDefaults = true
                prettyPrintIndent = "  "
            }
            val jsonString = json.encodeToString(TablesResponse.serializer(), response)
            fileOps.write(tablesJSONPath, jsonString)
        } catch (e: Exception) {
            VPinballManager.log(VPinballLogLevel.ERROR, "Failed to save tables.json: ${e.message}")
        }
    }

    private fun createTable(path: String): Table {
        val relativePath = relativePath(path, tablesPath)
        val stem = File(relativePath).nameWithoutExtension
        val parentPath = File(relativePath).parent ?: ""
        val now = System.currentTimeMillis() / 1000

        val imageBasePath = if (parentPath.isNotEmpty()) "$parentPath/$stem" else stem
        val image =
            when {
                fileOps.exists(buildPath("$imageBasePath.png")) -> relativePath(buildPath("$imageBasePath.png"), tablesPath)
                fileOps.exists(buildPath("$imageBasePath.jpg")) -> relativePath(buildPath("$imageBasePath.jpg"), tablesPath)
                else -> ""
            }

        return Table(uuid = generateUUID(), name = stem.replace("_", " "), path = relativePath, image = image, createdAt = now, modifiedAt = now)
    }

    private fun generateUUID(): String {
        var uuid = UUID.randomUUID().toString().lowercase()
        while (_tables.value.any { it.uuid == uuid }) {
            uuid = UUID.randomUUID().toString().lowercase()
            VPinballManager.log(VPinballLogLevel.WARN, "UUID collision detected: $uuid, regenerating")
        }
        return uuid
    }

    private fun sanitizeName(name: String): String {
        val invalidChars = setOf(' ', '_', '/', '\\', ':', '*', '?', '"', '<', '>', '|', '.', '&', '\'', '(', ')')
        return name.map { if (it in invalidChars) '-' else it }.joinToString("").replace(Regex("-+"), "-").trim('-').ifEmpty { "table" }
    }

    private fun getUniqueFolder(baseName: String): String {
        val sanitized = sanitizeName(baseName)
        return generateSequence(sanitized) { current ->
                val counter = current.substringAfterLast("-", "1").toIntOrNull()?.plus(1) ?: 2
                "$sanitized-$counter"
            }
            .first { !fileOps.exists(buildPath(it)) }
    }

    private fun updateTable(table: Table, transform: (Table) -> Table): Table {
        val updated = transform(table)
        _tables.value = _tables.value.map { if (it.uuid == table.uuid) updated else it }
        saveTables()
        return updated
    }

    private fun buildPath(relativePath: String): String {
        return if (requiresStaging) {
            "$tablesPath$relativePath"
        } else {
            File(tablesPath, relativePath).absolutePath
        }
    }

    private fun relativePath(fullPath: String, basePath: String): String {
        if (fullPath.isEmpty() || basePath.isEmpty() || !fullPath.startsWith(basePath)) {
            return fullPath
        }
        return fullPath.drop(basePath.length).removePrefix("/")
    }

    private suspend fun performImport(path: String, onProgress: (suspend (Int) -> Unit)? = null): List<Table> {
        if (!fileOps.exists(path)) {
            VPinballManager.log(VPinballLogLevel.ERROR, "File does not exist")
            return emptyList()
        }

        val ext = File(path).extension.lowercase()

        return when (ext) {
            "vpxz",
            "zip" -> importArchive(path, onProgress)
            "vpx" -> {
                val table = importVPX(path)
                if (table != null) listOf(table) else emptyList()
            }
            else -> {
                VPinballManager.log(VPinballLogLevel.ERROR, "Unsupported file extension: $ext")
                emptyList()
            }
        }
    }

    private fun importVPX(path: String): Table? {
        val stem = File(path).nameWithoutExtension
        val name = stem.replace("_", " ")

        val folderName = getUniqueFolder(name)
        val destFolder = buildPath(folderName)

        if (!fileOps.createDirectory(destFolder)) {
            VPinballManager.log(VPinballLogLevel.ERROR, "Failed to create directory")
            return null
        }

        val fileName = File(path).name
        val destFile = buildPath("$folderName/$fileName")

        if (!fileOps.copy(path, destFile)) {
            VPinballManager.log(VPinballLogLevel.ERROR, "Failed to copy file")
            return null
        }

        return createTable(destFile)
    }

    private suspend fun importArchive(path: String, onProgress: (suspend (Int) -> Unit)? = null): List<Table> {
        val tempDir = File(context.cacheDir, "vpinball_import_${System.currentTimeMillis()}").absolutePath

        if (!fileOps.createDirectory(tempDir)) {
            return emptyList()
        }

        try {
            val result =
                VPinballManager.vpinballJNI.VPinballZipExtract(
                    path,
                    tempDir,
                    VPinballZipCallback { current, total, _ ->
                        if (total > 0) {
                            val extractProgress = 60 + ((current.toDouble() / total) * 35).toInt()
                            runBlocking(Dispatchers.Main) { onProgress?.invoke(extractProgress) }
                        }
                    },
                )

            if (result != VPinballStatus.SUCCESS.value) {
                VPinballManager.log(VPinballLogLevel.ERROR, "Failed to extract archive")
                fileOps.deleteDirectory(tempDir)
                return emptyList()
            }

            val vpxFiles = fileOps.listFiles(tempDir, ".vpx")

            if (vpxFiles.isEmpty()) {
                fileOps.deleteDirectory(tempDir)
                return emptyList()
            }

            val importedTables = mutableListOf<Table>()
            for (vpxFile in vpxFiles) {
                val vpxName = File(vpxFile).nameWithoutExtension
                val sourceDir = File(vpxFile).parent ?: ""

                val folderName = getUniqueFolder(vpxName)
                val destFolder = buildPath(folderName)

                if (!fileOps.createDirectory(destFolder)) {
                    continue
                }

                if (!fileOps.copyDirectory(sourceDir, destFolder)) {
                    continue
                }

                val destVpxFile = buildPath("$folderName/${File(vpxFile).name}")
                createTable(destVpxFile).let { importedTables.add(it) }
            }

            fileOps.deleteDirectory(tempDir)
            return importedTables
        } catch (e: Exception) {
            VPinballManager.log(VPinballLogLevel.ERROR, "Failed to import archive: ${e.message}")
            fileOps.deleteDirectory(tempDir)
            return emptyList()
        }
    }

    private fun performDelete(table: Table, onProgress: ((Int, String) -> Unit)? = null): Boolean {
        val tablePath = buildPath(table.path)
        val tableDir = File(table.path).parent?.let { buildPath(it) } ?: return false

        if (tableDir.isEmpty() || tableDir == tablesPath || "$tableDir/" == tablesPath) {
            return false
        }

        if (fileOps.exists(tableDir)) {
            val vpxFiles = fileOps.listFiles(tableDir, ".vpx")

            if (vpxFiles.size <= 1) {
                if (!fileOps.deleteDirectory(tableDir, onProgress)) {
                    return false
                }
            } else {
                if (!fileOps.delete(tablePath, onProgress)) {
                    return false
                }
            }
        }

        _tables.value = _tables.value.filter { it.uuid != table.uuid }
        saveTables()

        return true
    }

    private fun performRename(table: Table, newName: String): Table {
        return updateTable(table) { it.copy(name = newName, modifiedAt = System.currentTimeMillis() / 1000) }
    }

    private fun performSetImage(table: Table, imagePath: String): Table? {
        if (imagePath.isEmpty()) {
            if (table.image.isNotEmpty()) {
                val currentImagePath = buildPath(table.image)
                if (fileOps.exists(currentImagePath)) {
                    fileOps.delete(currentImagePath)
                }
            }

            val now = System.currentTimeMillis() / 1000
            val updatedTable = table.copy(image = "", modifiedAt = now)
            _tables.value = _tables.value.map { if (it.uuid == table.uuid) updatedTable else it }
            saveTables()

            return updatedTable
        }

        if (imagePath.startsWith("/")) {
            if (table.image.isNotEmpty()) {
                val currentImagePath = buildPath(table.image)
                if (fileOps.exists(currentImagePath)) {
                    fileOps.delete(currentImagePath)
                    if (requiresStaging) {
                        SAFFileSystem.clearUriCache(table.image)
                    }
                }
            }

            val baseName = File(table.path).nameWithoutExtension
            val destPath = buildPath("${File(table.path).parent}/$baseName.jpg")

            if (!fileOps.copy(imagePath, destPath)) {
                return null
            }

            if (requiresStaging) {
                SAFFileSystem.clearUriCache("${File(table.path).parent}/$baseName.jpg")
            }

            val relPath = relativePath(destPath, tablesPath)
            val now = System.currentTimeMillis() / 1000
            val updatedTable = table.copy(image = relPath, modifiedAt = now)

            _tables.value = _tables.value.map { if (it.uuid == table.uuid) updatedTable else it }
            saveTables()

            return updatedTable
        }

        val now = System.currentTimeMillis() / 1000
        val updatedTable = table.copy(image = imagePath, modifiedAt = now)
        _tables.value = _tables.value.map { if (it.uuid == table.uuid) updatedTable else it }
        saveTables()

        return updatedTable
    }

    private suspend fun performExport(table: Table, onProgress: ((Int, String) -> Unit)? = null): String? {
        val sanitizedName = sanitizeName(table.name)
        val tempFile = File(context.cacheDir, "$sanitizedName.vpxz")

        if (tempFile.exists()) {
            tempFile.delete()
        }

        val tableDir = File(table.path).parent ?: return null

        try {
            val tableDirToCompressFinal =
                if (requiresStaging) {
                    withContext(Dispatchers.Main) { onProgress?.invoke(10, "Copying files") }

                    val stagingBaseDir = File(VPinballManager.getPath(VPinballPath.PREFERENCES), "saf")
                    val stagingTableDir = File(stagingBaseDir, tableDir)

                    if (stagingTableDir.exists()) {
                        fileOps.deleteDirectory(stagingTableDir.absolutePath)
                    }

                    if (!fileOps.createDirectory(stagingTableDir.absolutePath)) {
                        return null
                    }

                    val sourceTableDir = buildPath(tableDir)

                    if (!fileOps.copyDirectory(sourceTableDir, stagingTableDir.absolutePath)) {
                        return null
                    }

                    withContext(Dispatchers.Main) { onProgress?.invoke(50, "Copy complete") }
                    stagingTableDir.absolutePath
                } else {
                    File(buildPath(tableDir)).absolutePath
                }

            withContext(Dispatchers.Main) { onProgress?.invoke(60, "Compressing") }

            delay(100)

            val result =
                VPinballManager.vpinballJNI.VPinballZipCreate(
                    tableDirToCompressFinal,
                    tempFile.absolutePath,
                    VPinballZipCallback { current, total, _ ->
                        runBlocking(Dispatchers.Main) { onProgress?.invoke(60 + (current * 39 / maxOf(total, 1)), "Compressing") }
                    },
                )

            if (result != VPinballStatus.SUCCESS.value) {
                VPinballManager.log(VPinballLogLevel.ERROR, "Failed to create zip")
                return null
            }

            withContext(Dispatchers.Main) { onProgress?.invoke(100, "Complete") }

            delay(500)

            return tempFile.absolutePath
        } catch (e: Exception) {
            VPinballManager.log(VPinballLogLevel.ERROR, "Failed to export table: ${e.message}")
            return null
        }
    }

    private fun performStage(table: Table, onProgress: ((Int, String) -> Unit)? = null): String? {
        if (table.path.isEmpty()) return null

        if (requiresStaging) {
            return performStageToCache(table, onProgress)
        }

        val fullPath = buildPath(table.path)
        loadedTable = table
        loadedTableWorkingDir = File(fullPath).parent?.takeIf { it.isNotEmpty() } ?: "."
        return fullPath
    }

    private fun cleanupSafCache(maxCached: Int = 10) {
        val safDir = File(VPinballManager.getPath(VPinballPath.PREFERENCES), "saf")
        if (!safDir.exists()) return

        val folders = safDir.listFiles()?.filter { it.isDirectory } ?: return
        if (folders.size <= maxCached) return

        folders.sortedBy { it.lastModified() }.take(folders.size - maxCached).forEach { fileOps.deleteDirectory(it.absolutePath) }
    }

    private fun performStageToCache(table: Table, onProgress: ((Int, String) -> Unit)?): String? {
        cleanupSafCache()

        val tableDir = File(table.path).parent ?: ""
        val fileName = File(table.path).name

        onProgress?.invoke(10, "Staging table...")
        val cachePath = File(VPinballManager.getPath(VPinballPath.PREFERENCES), "saf/$tableDir").absolutePath

        if (fileOps.exists(cachePath)) {
            fileOps.deleteDirectory(cachePath)
        }
        fileOps.createDirectory(cachePath)

        onProgress?.invoke(30, "Copying table files...")
        if (!fileOps.copyDirectory(buildPath(tableDir), cachePath)) {
            VPinballManager.log(VPinballLogLevel.ERROR, "Failed to copy to staging cache")
            return null
        }

        onProgress?.invoke(100, "Ready")
        loadedTable = table
        loadedTableWorkingDir = cachePath
        return File(cachePath, fileName).absolutePath
    }

    private fun performCleanup(table: Table, onProgress: ((Int, String) -> Unit)? = null) {
        if (loadedTable?.uuid != table.uuid) {
            return
        }

        if (requiresStaging && loadedTableWorkingDir.isNotEmpty()) {
            onProgress?.invoke(10, "Saving changes...")
            val tableDir = File(table.path).parent ?: ""

            val changedFileExtensions = listOf("txt", "ini", "cfg", "xml", "nv", "jpg", "png")
            val workingDirFile = File(loadedTableWorkingDir)
            val filesToCopy =
                workingDirFile.listFiles()?.filter { file -> file.isFile && changedFileExtensions.contains(file.extension.lowercase()) }
                    ?: emptyList()

            if (filesToCopy.isNotEmpty()) {
                onProgress?.invoke(30, "Copying ${filesToCopy.size} file(s)...")
                var copiedCount = 0
                var failedCount = 0

                filesToCopy.forEach { file ->
                    val relativePath = file.name
                    val destPath = buildPath("$tableDir/$relativePath")

                    if (fileOps.copy(file.absolutePath, destPath)) {
                        copiedCount++
                        val progress = 30 + ((copiedCount * 60) / filesToCopy.size)
                        onProgress?.invoke(progress, "Copied $copiedCount/${filesToCopy.size}")
                    } else {
                        failedCount++
                        VPinballManager.log(VPinballLogLevel.ERROR, "Failed to copy back: ${file.name}")
                    }
                }

                onProgress?.invoke(100, "Complete")

                if (failedCount > 0) {
                    VPinballManager.log(VPinballLogLevel.WARN, "Sync complete: $copiedCount succeeded, $failedCount failed")
                }
            } else {
                onProgress?.invoke(100, "No changes to save")
            }
        }

        loadedTable = null
        loadedTableWorkingDir = ""
    }

    private fun performExtractScript(table: Table, onProgress: ((Int, String) -> Unit)? = null): Boolean {
        val tableDir = File(table.path).parent ?: return false
        val baseName = File(table.path).nameWithoutExtension
        val scriptPath = buildPath("$tableDir/$baseName.vbs")

        if (fileOps.exists(scriptPath)) {
            return true
        }

        val fullPath: String
        val cachePath: String?

        if (requiresStaging) {
            onProgress?.invoke(10, "Staging table...")
            cachePath = File(VPinballManager.getPath(VPinballPath.PREFERENCES), "saf/$tableDir").absolutePath

            if (fileOps.exists(cachePath)) {
                fileOps.deleteDirectory(cachePath)
            }

            fileOps.createDirectory(cachePath)

            onProgress?.invoke(30, "Copying table files...")
            val sourceTableDir = buildPath(tableDir)
            if (!fileOps.copyDirectory(sourceTableDir, cachePath)) {
                VPinballManager.log(VPinballLogLevel.ERROR, "Failed to copy to staging cache")
                return false
            }

            onProgress?.invoke(70, "Extracting script...")
            fullPath = File(cachePath, File(table.path).name).absolutePath
        } else {
            onProgress?.invoke(50, "Extracting script...")
            fullPath = buildPath(table.path)
            cachePath = null
        }

        if (VPinballManager.vpinballJNI.VPinballLoadTable(fullPath) != VPinballStatus.SUCCESS.value) {
            VPinballManager.log(VPinballLogLevel.ERROR, "Failed to load table for script extraction: $fullPath")
            cachePath?.let { fileOps.deleteDirectory(it) }
            return false
        }

        if (VPinballManager.vpinballJNI.VPinballExtractTableScript() != VPinballStatus.SUCCESS.value) {
            VPinballManager.log(VPinballLogLevel.ERROR, "Failed to extract script from table")
            cachePath?.let { fileOps.deleteDirectory(it) }
            return false
        }

        if (requiresStaging && cachePath != null) {
            onProgress?.invoke(90, "Copying script back...")
            val stagedScriptPath = File(cachePath, "$baseName.vbs").absolutePath

            val destScriptPath = buildPath("$tableDir/$baseName.vbs")
            if (!fileOps.copy(stagedScriptPath, destScriptPath)) {
                VPinballManager.log(VPinballLogLevel.ERROR, "Failed to copy script back to SAF")
                fileOps.deleteDirectory(cachePath)
                return false
            }

            fileOps.deleteDirectory(cachePath)
        }

        onProgress?.invoke(100, "Complete")

        return if (fileOps.exists(scriptPath)) {
            true
        } else {
            VPinballManager.log(VPinballLogLevel.WARN, "Script file not found after extraction")
            false
        }
    }

    companion object {
        @SuppressLint("StaticFieldLeak") private var instance: TableManager? = null

        fun initialize(context: Context) {
            if (instance == null) {
                instance = TableManager(context.applicationContext)
            }
        }

        fun getInstance(): TableManager {
            return instance ?: throw IllegalStateException("TableManager not initialized")
        }

        suspend fun loadTables(onProgress: ((Int, String) -> Unit)? = null): List<Table> {
            val instance = getInstance()
            instance.refresh(onProgress)
            return instance.tables.value
        }

        suspend fun importTable(uri: Uri, onUpdate: (Int, String) -> Unit, onComplete: (String, String) -> Unit, onError: () -> Unit) {
            val instance = getInstance()
            val result = instance.importTable(uri, onUpdate)
            if (result) {
                onComplete("imported", "")
            } else {
                onError()
            }
        }

        suspend fun shareTable(table: Table, onProgress: ((Int, String) -> Unit)? = null, onComplete: (String) -> Unit, onError: () -> Unit) {
            val instance = getInstance()
            val path = instance.exportTable(table, onProgress)
            if (path != null) {
                onComplete(path)
            } else {
                onError()
            }
        }

        suspend fun extractTableScript(table: Table, onProgress: ((Int, String) -> Unit)? = null, onComplete: () -> Unit, onError: () -> Unit) {
            val instance = getInstance()
            if (instance.extractTableScript(table, onProgress)) {
                onComplete()
            } else {
                onError()
            }
        }
    }
}
