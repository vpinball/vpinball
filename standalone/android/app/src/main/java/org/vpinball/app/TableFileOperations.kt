package org.vpinball.app

import android.content.Context
import android.net.Uri
import java.io.File
import java.io.FileInputStream
import java.io.FileOutputStream
import org.vpinball.app.jni.VPinballLogLevel
import org.vpinball.app.jni.VPinballPath

class TableFileOperations(private val getTablesPath: () -> String) {
    private fun toRelativePath(path: String): String {
        val tablesPath = getTablesPath()
        return if (path.startsWith(tablesPath)) path.drop(tablesPath.length) else path
    }

    fun exists(path: String): Boolean =
        if (SAFFileSystem.isSAFPath(path)) {
            SAFFileSystem.exists(toRelativePath(path))
        } else {
            File(path).exists()
        }

    fun read(path: String): String? =
        if (SAFFileSystem.isSAFPath(path)) {
            SAFFileSystem.read(toRelativePath(path))
        } else {
            try {
                File(path).readText()
            } catch (_: Exception) {
                null
            }
        }

    fun write(path: String, content: String): Boolean =
        if (SAFFileSystem.isSAFPath(path)) {
            SAFFileSystem.write(toRelativePath(path), content)
        } else {
            try {
                File(path).writeText(content)
                true
            } catch (_: Exception) {
                false
            }
        }

    fun copy(from: String, to: String): Boolean =
        if (SAFFileSystem.isSAFPath(to)) {
            try {
                FileInputStream(from).use { input ->
                    SAFFileSystem.openOutputStream(toRelativePath(to))?.use { output ->
                        input.copyTo(output)
                        true
                    } ?: false.also { VPinballManager.log(VPinballLogLevel.ERROR, "Failed to open output stream for SAF: ${toRelativePath(to)}") }
                }
            } catch (e: Exception) {
                VPinballManager.log(VPinballLogLevel.ERROR, "Failed to copy to SAF: ${e.message}")
                false
            }
        } else {
            try {
                File(from).copyTo(File(to), overwrite = true)
                true
            } catch (_: Exception) {
                false
            }
        }

    fun copy(context: Context, from: Uri, to: File): Boolean {
        return try {
            when (from.scheme) {
                "file" -> {
                    val fromFile = File(from.path ?: return false)
                    if (!fromFile.exists()) {
                        VPinballManager.log(VPinballLogLevel.ERROR, "Source file does not exist: ${fromFile.absolutePath}")
                        return false
                    }

                    if (fromFile.canonicalPath == to.canonicalPath) {
                        return true
                    }

                    fromFile.copyTo(to, overwrite = true)
                    true
                }
                "content" -> {
                    context.contentResolver.openInputStream(from)?.use { input -> FileOutputStream(to).use { output -> input.copyTo(output) } }
                    true
                }
                else -> {
                    VPinballManager.log(VPinballLogLevel.ERROR, "Unsupported URI scheme: ${from.scheme}")
                    false
                }
            }
        } catch (e: Exception) {
            VPinballManager.log(VPinballLogLevel.ERROR, "Failed to copy file: ${e.message}")
            false
        }
    }

    fun delete(path: String, onProgress: ((Int, String) -> Unit)? = null): Boolean {
        return if (SAFFileSystem.isSAFPath(path)) {
            SAFFileSystem.delete(toRelativePath(path), onProgress)
        } else {
            try {
                File(path).delete()
            } catch (_: Exception) {
                false
            }
        }
    }

    fun deleteDirectory(path: String, onProgress: ((Int, String) -> Unit)? = null): Boolean =
        if (SAFFileSystem.isSAFPath(path)) {
            SAFFileSystem.delete(toRelativePath(path), onProgress)
        } else {
            try {
                File(path).deleteRecursively()
            } catch (_: Exception) {
                false
            }
        }

    fun createDirectory(path: String): Boolean =
        if (SAFFileSystem.isSAFPath(path)) {
            true
        } else {
            try {
                File(path).mkdirs()
            } catch (_: Exception) {
                false
            }
        }

    fun listFiles(path: String, ext: String): List<String> {
        val files = mutableListOf<String>()

        if (SAFFileSystem.isSAFPath(path)) {
            val result = SAFFileSystem.listFilesRecursive(ext.removePrefix("."))
            if (result.isNotEmpty()) {
                try {
                    val jsonArray = org.json.JSONArray(result)
                    for (i in 0 until jsonArray.length()) {
                        val relativePath = jsonArray.getString(i)
                        files.add("$path$relativePath")
                    }
                } catch (e: Exception) {
                    VPinballManager.log(VPinballLogLevel.ERROR, "Error parsing SAF file list: ${e.message}")
                }
            }
            return files
        }

        val preferencesPath = VPinballManager.getPath(VPinballPath.PREFERENCES)
        val assetsPath = VPinballManager.getPath(VPinballPath.ASSETS)

        try {
            File(path)
                .walkTopDown()
                .onEnter { dir -> dir.absolutePath != preferencesPath && dir.absolutePath != assetsPath }
                .forEach { file ->
                    if (file.isFile) {
                        if (ext.isEmpty() || file.extension.lowercase() == ext.lowercase().removePrefix(".")) {
                            files.add(file.absolutePath)
                        }
                    }
                }
        } catch (e: Exception) {
            VPinballManager.log(VPinballLogLevel.ERROR, "Error listing files in $path: ${e.message}")
        }

        return files
    }

    fun copyDirectory(from: String, to: String, onProgress: ((Int, String) -> Unit)? = null): Boolean {
        if (SAFFileSystem.isSAFPath(from) && !SAFFileSystem.isSAFPath(to)) {
            return SAFFileSystem.copyToFilesystem(toRelativePath(from), to, onProgress)
        }

        if (!SAFFileSystem.isSAFPath(from) && SAFFileSystem.isSAFPath(to)) {
            return SAFFileSystem.copyDirectory(from, toRelativePath(to))
        }

        return try {
            val fromDir = File(from)
            val toDir = File(to)

            if (toDir.exists()) {
                toDir.deleteRecursively()
            }

            toDir.mkdirs()

            fromDir.listFiles()?.forEach { file ->
                val destPath = File(toDir, file.name)

                if (file.isDirectory) {
                    if (!copyDirectory(file.absolutePath, destPath.absolutePath)) {
                        return false
                    }
                } else {
                    file.copyTo(destPath, overwrite = true)
                }
            }

            true
        } catch (_: Exception) {
            false
        }
    }

    fun getFileName(context: Context, uri: Uri): String? {
        return try {
            when (uri.scheme) {
                "file" -> {
                    File(uri.path ?: return null).name
                }
                "content" -> {
                    val cursor = context.contentResolver.query(uri, null, null, null, null)
                    cursor?.use {
                        val nameIndex = it.getColumnIndex(android.provider.OpenableColumns.DISPLAY_NAME)
                        it.moveToFirst()
                        it.getString(nameIndex)
                    }
                }
                else -> null
            }
        } catch (e: Exception) {
            VPinballManager.log(VPinballLogLevel.ERROR, "Failed to get filename from URI: ${e.message}")
            null
        }
    }
}
