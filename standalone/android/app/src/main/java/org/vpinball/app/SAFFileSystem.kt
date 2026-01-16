package org.vpinball.app

import android.content.Context
import android.content.Intent
import android.net.Uri
import android.provider.DocumentsContract
import androidx.core.net.toUri
import java.io.File
import java.io.FileInputStream
import java.io.FileOutputStream
import java.io.InputStream
import java.io.OutputStream
import org.json.JSONArray
import org.vpinball.app.jni.VPinballLogLevel
import org.vpinball.app.jni.VPinballSettingsSection
import org.vpinball.app.jni.VPinballSettingsSection.STANDALONE

object SAFFileSystem {
    private lateinit var context: Context

    private val uriCache = mutableMapOf<String, Uri?>()

    fun initialize(context: Context) {
        this.context = context.applicationContext
    }

    fun isSAFPath(path: String): Boolean {
        return path.startsWith("content://")
    }

    fun setExternalStorageUri(uri: Uri) {
        try {
            context.contentResolver.takePersistableUriPermission(uri, Intent.FLAG_GRANT_READ_URI_PERMISSION or Intent.FLAG_GRANT_WRITE_URI_PERMISSION)
        } catch (e: Exception) {
            VPinballManager.log(VPinballLogLevel.ERROR, "Failed to take persistent permission: ${e.message}")
            return
        }

        VPinballManager.saveValue(VPinballSettingsSection.STANDALONE, "SAFPath", uri.toString())
        uriCache.clear()
    }

    fun getExternalStorageUri(): Uri? {
        val safPath = VPinballManager.loadValue(STANDALONE, "SAFPath", "")
        return if (safPath.isNotEmpty()) safPath.toUri() else null
    }

    fun isUsingSAF(): Boolean {
        return VPinballManager.loadValue(STANDALONE, "SAFPath", "").isNotEmpty()
    }

    fun clearExternalStorageUri() {
        VPinballManager.saveValue(VPinballSettingsSection.STANDALONE, "SAFPath", "")
        uriCache.clear()
    }

    fun getExternalStorageDisplayPath(): String {
        val uri = getExternalStorageUri() ?: return ""

        try {
            val docId = DocumentsContract.getTreeDocumentId(uri)
            val split = docId.split(":")

            if (split.size >= 2) {
                val type = split[0]
                val path = split[1]

                return when {
                    type.equals("primary", ignoreCase = true) -> "/storage/emulated/0/$path"
                    else -> "/storage/$type/$path"
                }
            }

            return docId
        } catch (e: Exception) {
            return uri.toString()
        }
    }

    fun write(relativePath: String, content: String): Boolean {
        val uri = getExternalStorageUri()
        if (uri == null) {
            VPinballManager.log(VPinballLogLevel.ERROR, "No external storage URI set")
            return false
        }

        return try {
            val docUri = buildDocumentUri(uri, relativePath, createIfMissing = true)
            if (docUri == null) {
                VPinballManager.log(VPinballLogLevel.ERROR, "Failed to build document URI for: $relativePath")
                return false
            }

            context.contentResolver.openOutputStream(docUri, "wt")?.use { output ->
                output.write(content.toByteArray())
                output.flush()
            }

            true
        } catch (e: Exception) {
            VPinballManager.log(VPinballLogLevel.ERROR, "Exception: ${e.message}")
            false
        }
    }

    fun read(relativePath: String): String? {
        val uri = getExternalStorageUri()
        if (uri == null) {
            VPinballManager.log(VPinballLogLevel.ERROR, "No external storage URI set")
            return null
        }

        return try {
            val docUri = buildDocumentUri(uri, relativePath) ?: return null
            context.contentResolver.openInputStream(docUri)?.use { input -> input.bufferedReader().readText() }
        } catch (e: Exception) {
            VPinballManager.log(VPinballLogLevel.ERROR, "Exception: ${e.message}")
            null
        }
    }

    fun openInputStream(relativePath: String): InputStream? {
        val uri = getExternalStorageUri()
        if (uri == null) {
            VPinballManager.log(VPinballLogLevel.ERROR, "No external storage URI set")
            return null
        }

        return try {
            val docUri = buildDocumentUri(uri, relativePath) ?: return null
            context.contentResolver.openInputStream(docUri)
        } catch (e: Exception) {
            VPinballManager.log(VPinballLogLevel.ERROR, "Exception: ${e.message}")
            null
        }
    }

    fun openOutputStream(relativePath: String): OutputStream? {
        val uri = getExternalStorageUri()
        if (uri == null) {
            VPinballManager.log(VPinballLogLevel.ERROR, "No external storage URI set")
            return null
        }

        return try {
            val docUri = buildDocumentUri(uri, relativePath, createIfMissing = true) ?: return null
            context.contentResolver.openOutputStream(docUri, "wt")
        } catch (e: Exception) {
            VPinballManager.log(VPinballLogLevel.ERROR, "Exception: ${e.message}")
            null
        }
    }

    fun exists(relativePath: String): Boolean {
        val uri = getExternalStorageUri() ?: return false
        val docUri = buildDocumentUri(uri, relativePath)
        return docUri != null
    }

    fun listFilesRecursive(extension: String): String {
        val uri = getExternalStorageUri()
        if (uri == null) {
            VPinballManager.log(VPinballLogLevel.ERROR, "No external storage URI set")
            return "[]"
        }

        val results = mutableListOf<String>()

        fun scanRecursive(docUri: Uri, currentPath: String = "") {
            try {
                val childrenUri = DocumentsContract.buildChildDocumentsUriUsingTree(uri, DocumentsContract.getDocumentId(docUri))

                context.contentResolver
                    .query(
                        childrenUri,
                        arrayOf(
                            DocumentsContract.Document.COLUMN_DISPLAY_NAME,
                            DocumentsContract.Document.COLUMN_MIME_TYPE,
                            DocumentsContract.Document.COLUMN_DOCUMENT_ID,
                        ),
                        null,
                        null,
                        null,
                    )
                    ?.use { cursor ->
                        val nameIndex = cursor.getColumnIndexOrThrow(DocumentsContract.Document.COLUMN_DISPLAY_NAME)
                        val mimeIndex = cursor.getColumnIndexOrThrow(DocumentsContract.Document.COLUMN_MIME_TYPE)
                        val idIndex = cursor.getColumnIndexOrThrow(DocumentsContract.Document.COLUMN_DOCUMENT_ID)

                        while (cursor.moveToNext()) {
                            val name = cursor.getString(nameIndex)
                            val mimeType = cursor.getString(mimeIndex)
                            val documentId = cursor.getString(idIndex)

                            if (mimeType == DocumentsContract.Document.MIME_TYPE_DIR) {
                                val childUri = DocumentsContract.buildDocumentUriUsingTree(uri, documentId)
                                scanRecursive(childUri, "$currentPath$name/")
                            } else if (name.endsWith(extension, ignoreCase = true)) {
                                results.add("$currentPath$name")
                            }
                        }
                    }
            } catch (e: Exception) {
                VPinballManager.log(VPinballLogLevel.ERROR, "Exception in $currentPath: ${e.message}")
            }
        }

        val treeDocumentId = DocumentsContract.getTreeDocumentId(uri)
        val rootDocUri = DocumentsContract.buildDocumentUriUsingTree(uri, treeDocumentId)
        scanRecursive(rootDocUri)

        return JSONArray(results).toString()
    }

    private fun buildDocumentUri(treeUri: Uri, relativePath: String, createIfMissing: Boolean = false): Uri? {
        if (relativePath.isEmpty()) return treeUri

        val cacheKey = "$treeUri:$relativePath"
        if (!createIfMissing && uriCache.containsKey(cacheKey)) {
            return uriCache[cacheKey]
        }

        try {
            val treeDocumentId = DocumentsContract.getTreeDocumentId(treeUri)
            var currentUri = DocumentsContract.buildDocumentUriUsingTree(treeUri, treeDocumentId)

            val segments = relativePath.split("/").filter { it.isNotEmpty() }

            for ((index, segment) in segments.withIndex()) {
                val isLastSegment = (index == segments.size - 1)

                val childrenUri = DocumentsContract.buildChildDocumentsUriUsingTree(treeUri, DocumentsContract.getDocumentId(currentUri))

                var foundUri: Uri? = null

                context.contentResolver
                    .query(
                        childrenUri,
                        arrayOf(
                            DocumentsContract.Document.COLUMN_DISPLAY_NAME,
                            DocumentsContract.Document.COLUMN_DOCUMENT_ID,
                            DocumentsContract.Document.COLUMN_MIME_TYPE,
                        ),
                        null,
                        null,
                        null,
                    )
                    ?.use { cursor ->
                        val nameIndex = cursor.getColumnIndexOrThrow(DocumentsContract.Document.COLUMN_DISPLAY_NAME)
                        val idIndex = cursor.getColumnIndexOrThrow(DocumentsContract.Document.COLUMN_DOCUMENT_ID)

                        while (cursor.moveToNext()) {
                            val name = cursor.getString(nameIndex)
                            if (name == segment) {
                                val documentId = cursor.getString(idIndex)
                                foundUri = DocumentsContract.buildDocumentUriUsingTree(treeUri, documentId)
                                break
                            }
                        }
                    }

                if (foundUri != null) {
                    currentUri = foundUri
                } else if (createIfMissing) {
                    val mimeType =
                        if (isLastSegment && segment.contains(".")) {
                            "application/octet-stream"
                        } else {
                            DocumentsContract.Document.MIME_TYPE_DIR
                        }

                    val newUri = DocumentsContract.createDocument(context.contentResolver, currentUri, mimeType, segment)

                    if (newUri != null) {
                        currentUri = newUri
                    } else {
                        VPinballManager.log(VPinballLogLevel.ERROR, "Failed to create: $segment")
                        return null
                    }
                } else {
                    return null
                }
            }

            if (!createIfMissing) {
                uriCache[cacheKey] = currentUri
            }

            return currentUri
        } catch (e: Exception) {
            VPinballManager.log(VPinballLogLevel.ERROR, "Exception: ${e.message}")

            if (!createIfMissing) {
                uriCache[cacheKey] = null
            }

            return null
        }
    }

    fun copyToFilesystem(relativePath: String, destPath: String, onProgress: ((Int, String) -> Unit)? = null): Boolean {
        val uri = getExternalStorageUri()
        if (uri == null) {
            VPinballManager.log(VPinballLogLevel.ERROR, "No external storage URI")
            return false
        }

        var totalFiles: Int
        var copiedFiles = 0

        fun countFiles(srcRelPath: String): Int {
            val srcUri = buildDocumentUri(uri, srcRelPath) ?: return 0
            var count = 0

            try {
                context.contentResolver.query(srcUri, arrayOf(DocumentsContract.Document.COLUMN_MIME_TYPE), null, null, null)?.use { cursor ->
                    if (cursor.moveToFirst()) {
                        val mimeIndex = cursor.getColumnIndex(DocumentsContract.Document.COLUMN_MIME_TYPE)
                        val mimeType = cursor.getString(mimeIndex)

                        if (mimeType == DocumentsContract.Document.MIME_TYPE_DIR) {
                            val childrenUri = DocumentsContract.buildChildDocumentsUriUsingTree(uri, DocumentsContract.getDocumentId(srcUri))

                            context.contentResolver
                                .query(childrenUri, arrayOf(DocumentsContract.Document.COLUMN_DISPLAY_NAME), null, null, null)
                                ?.use { childCursor ->
                                    val nameIndex = childCursor.getColumnIndex(DocumentsContract.Document.COLUMN_DISPLAY_NAME)

                                    while (childCursor.moveToNext()) {
                                        val name = childCursor.getString(nameIndex)
                                        val childPath = if (srcRelPath.isEmpty()) name else "$srcRelPath/$name"
                                        count += countFiles(childPath)
                                    }
                                }
                        } else {
                            count = 1
                        }
                    }
                }
            } catch (e: Exception) {
                VPinballManager.log(VPinballLogLevel.ERROR, "Exception: ${e.message}")
            }

            return count
        }

        totalFiles = countFiles(relativePath)
        onProgress?.invoke(0, "Copying table files... (0/$totalFiles)")

        fun copyRecursive(srcRelPath: String, dstPath: String): Boolean {
            val srcUri = buildDocumentUri(uri, srcRelPath)
            if (srcUri == null) {
                VPinballManager.log(VPinballLogLevel.ERROR, "Source not found: $srcRelPath")
                return false
            }

            try {
                context.contentResolver.query(srcUri, arrayOf(DocumentsContract.Document.COLUMN_MIME_TYPE), null, null, null)?.use { cursor ->
                    if (cursor.moveToFirst()) {
                        val mimeIndex = cursor.getColumnIndex(DocumentsContract.Document.COLUMN_MIME_TYPE)
                        val mimeType = cursor.getString(mimeIndex)

                        if (mimeType == DocumentsContract.Document.MIME_TYPE_DIR) {
                            File(dstPath).mkdirs()

                            val childrenUri = DocumentsContract.buildChildDocumentsUriUsingTree(uri, DocumentsContract.getDocumentId(srcUri))

                            context.contentResolver
                                .query(childrenUri, arrayOf(DocumentsContract.Document.COLUMN_DISPLAY_NAME), null, null, null)
                                ?.use { childCursor ->
                                    val nameIndex = childCursor.getColumnIndex(DocumentsContract.Document.COLUMN_DISPLAY_NAME)

                                    while (childCursor.moveToNext()) {
                                        val name = childCursor.getString(nameIndex)
                                        val childSrcPath = if (srcRelPath.isEmpty()) name else "$srcRelPath/$name"
                                        val childDstPath = "$dstPath/$name"

                                        if (!copyRecursive(childSrcPath, childDstPath)) {
                                            return false
                                        }
                                    }
                                }

                            return true
                        } else {
                            context.contentResolver.openInputStream(srcUri)?.use { input ->
                                FileOutputStream(dstPath).use { output -> input.copyTo(output) }
                            }
                            copiedFiles++
                            val progressPercent = if (totalFiles > 0) (copiedFiles * 100) / totalFiles else 0
                            onProgress?.invoke(progressPercent, "Copying table files... ($copiedFiles/$totalFiles)")
                            return true
                        }
                    }
                }
            } catch (e: Exception) {
                VPinballManager.log(VPinballLogLevel.ERROR, "Exception: ${e.message}")
                return false
            }

            return false
        }

        val result = copyRecursive(relativePath, destPath)

        if (result) {
            onProgress?.invoke(100, "Copy complete")
        }

        return result
    }

    fun copyDirectory(sourcePath: String, destPath: String): Boolean {
        try {
            val sourceFile = File(sourcePath)
            if (!sourceFile.exists()) {
                VPinballManager.log(VPinballLogLevel.ERROR, "Source doesn't exist: $sourcePath")
                return false
            }

            if (sourceFile.isFile) {
                val uri = getExternalStorageUri()
                if (uri == null) {
                    VPinballManager.log(VPinballLogLevel.ERROR, "No external storage URI")
                    return false
                }

                val destUri = buildDocumentUri(uri, destPath, createIfMissing = true)
                if (destUri == null) {
                    VPinballManager.log(VPinballLogLevel.ERROR, "Failed to create: $destPath")
                    return false
                }

                try {
                    val fileSize = sourceFile.length()
                    FileInputStream(sourceFile).use { input ->
                        context.contentResolver.openOutputStream(destUri, "w")?.use { output ->
                            val bytesCopied = input.copyTo(output)
                            output.flush()
                            if (bytesCopied != fileSize) {
                                VPinballManager.log(VPinballLogLevel.ERROR, "Size mismatch! Expected $fileSize, got $bytesCopied")
                                return false
                            }
                        }
                            ?: run {
                                VPinballManager.log(VPinballLogLevel.ERROR, "Failed to open output stream")
                                return false
                            }
                    }
                    return true
                } catch (e: Exception) {
                    VPinballManager.log(VPinballLogLevel.ERROR, "Failed to copy file: ${e.message}")
                    return false
                }
            }

            if (!sourceFile.isDirectory) {
                VPinballManager.log(VPinballLogLevel.ERROR, "Source is neither file nor directory: $sourcePath")
                return false
            }

            fun copyRecursive(srcFile: File, destRelativePath: String): Boolean {
                if (srcFile.isDirectory) {
                    val children = srcFile.listFiles() ?: emptyArray()
                    for (child in children) {
                        val childDestPath =
                            if (destRelativePath.isEmpty()) {
                                child.name
                            } else {
                                "$destRelativePath/${child.name}"
                            }
                        if (!copyRecursive(child, childDestPath)) {
                            return false
                        }
                    }
                    return true
                } else {
                    val uri = getExternalStorageUri()
                    if (uri == null) {
                        VPinballManager.log(VPinballLogLevel.ERROR, "No external storage URI")
                        return false
                    }

                    val destUri = buildDocumentUri(uri, destRelativePath, createIfMissing = true)
                    if (destUri == null) {
                        VPinballManager.log(VPinballLogLevel.ERROR, "Failed to create: $destRelativePath")
                        return false
                    }

                    try {
                        FileInputStream(srcFile).use { input ->
                            context.contentResolver.openOutputStream(destUri, "wt")?.use { output -> input.copyTo(output) } ?: return false
                        }
                        return true
                    } catch (e: Exception) {
                        VPinballManager.log(VPinballLogLevel.ERROR, "Failed to copy file: ${e.message}")
                        return false
                    }
                }
            }

            return copyRecursive(sourceFile, destPath)
        } catch (e: Exception) {
            VPinballManager.log(VPinballLogLevel.ERROR, "Exception: ${e.message}")
            return false
        }
    }

    fun clearUriCache(relativePath: String) {
        val uri = getExternalStorageUri() ?: return
        val cacheKey = "$uri:$relativePath"
        uriCache.remove(cacheKey)
    }

    fun delete(relativePath: String, onProgress: ((Int, String) -> Unit)? = null): Boolean {
        val uri = getExternalStorageUri()
        if (uri == null) {
            VPinballManager.log(VPinballLogLevel.ERROR, "No external storage URI")
            return false
        }

        val docUri = buildDocumentUri(uri, relativePath)
        if (docUri == null) {
            VPinballManager.log(VPinballLogLevel.ERROR, "Path not found: $relativePath")
            return false
        }

        return try {
            if (onProgress != null) {
                onProgress(10, "Counting files...")
                val fileCount = countFilesRecursive(docUri, uri)

                if (fileCount > 1) {
                    onProgress(20, "Deleting $fileCount files...")
                    val deletedCount = intArrayOf(0)
                    val result = deleteRecursive(docUri, uri, fileCount, deletedCount, onProgress)
                    if (result) {
                        val cacheKey = "$uri:$relativePath"
                        uriCache.remove(cacheKey)
                        onProgress(100, "Deleted")
                    }
                    return result
                }
            }

            val deleted = DocumentsContract.deleteDocument(context.contentResolver, docUri)
            if (deleted) {
                val cacheKey = "$uri:$relativePath"
                uriCache.remove(cacheKey)
            }
            onProgress?.invoke(100, "Deleted")
            deleted
        } catch (e: Exception) {
            VPinballManager.log(VPinballLogLevel.ERROR, "Exception: ${e.message}")
            false
        }
    }

    private fun countFilesRecursive(docUri: Uri, treeUri: Uri): Int {
        var count = 0
        try {
            context.contentResolver.query(docUri, arrayOf(DocumentsContract.Document.COLUMN_MIME_TYPE), null, null, null)?.use { cursor ->
                if (cursor.moveToFirst()) {
                    val mimeIndex = cursor.getColumnIndex(DocumentsContract.Document.COLUMN_MIME_TYPE)
                    val mimeType = cursor.getString(mimeIndex)

                    if (mimeType == DocumentsContract.Document.MIME_TYPE_DIR) {
                        val childrenUri = DocumentsContract.buildChildDocumentsUriUsingTree(treeUri, DocumentsContract.getDocumentId(docUri))
                        context.contentResolver
                            .query(
                                childrenUri,
                                arrayOf(DocumentsContract.Document.COLUMN_DOCUMENT_ID, DocumentsContract.Document.COLUMN_MIME_TYPE),
                                null,
                                null,
                                null,
                            )
                            ?.use { childCursor ->
                                while (childCursor.moveToNext()) {
                                    val childId = childCursor.getString(0)
                                    val childMimeType = childCursor.getString(1)
                                    val childUri = DocumentsContract.buildDocumentUriUsingTree(treeUri, childId)

                                    if (childMimeType == DocumentsContract.Document.MIME_TYPE_DIR) {
                                        count += countFilesRecursive(childUri, treeUri)
                                    } else {
                                        count++
                                    }
                                }
                            }
                    } else {
                        count = 1
                    }
                }
            }
        } catch (e: Exception) {
            VPinballManager.log(VPinballLogLevel.ERROR, "Exception: ${e.message}")
        }
        return count
    }

    private fun deleteRecursive(docUri: Uri, treeUri: Uri, totalFiles: Int, deletedCount: IntArray, onProgress: (Int, String) -> Unit): Boolean {
        try {
            context.contentResolver.query(docUri, arrayOf(DocumentsContract.Document.COLUMN_MIME_TYPE), null, null, null)?.use { cursor ->
                if (cursor.moveToFirst()) {
                    val mimeIndex = cursor.getColumnIndex(DocumentsContract.Document.COLUMN_MIME_TYPE)
                    val mimeType = cursor.getString(mimeIndex)

                    if (mimeType == DocumentsContract.Document.MIME_TYPE_DIR) {
                        val childrenUri = DocumentsContract.buildChildDocumentsUriUsingTree(treeUri, DocumentsContract.getDocumentId(docUri))
                        val childUris = mutableListOf<Uri>()

                        context.contentResolver.query(childrenUri, arrayOf(DocumentsContract.Document.COLUMN_DOCUMENT_ID), null, null, null)?.use {
                            childCursor ->
                            while (childCursor.moveToNext()) {
                                val childId = childCursor.getString(0)
                                childUris.add(DocumentsContract.buildDocumentUriUsingTree(treeUri, childId))
                            }
                        }

                        childUris.forEach { childUri ->
                            if (!deleteRecursive(childUri, treeUri, totalFiles, deletedCount, onProgress)) {
                                return false
                            }
                        }
                    } else {
                        deletedCount[0]++
                        val progress = 20 + ((deletedCount[0] * 80) / totalFiles)
                        val currentCount = deletedCount[0]
                        onProgress(progress, "Deleting... ($currentCount/$totalFiles)")
                    }
                }
            }

            return DocumentsContract.deleteDocument(context.contentResolver, docUri)
        } catch (e: Exception) {
            VPinballManager.log(VPinballLogLevel.ERROR, "Exception: ${e.message}")
            return false
        }
    }
}
