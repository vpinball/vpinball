package org.vpinball.app.util

import android.content.Context
import android.content.res.AssetManager
import android.net.Uri
import android.provider.OpenableColumns
import android.util.Log
import java.io.File
import java.io.FileNotFoundException
import java.io.FileOutputStream
import java.io.IOException
import java.io.InputStream

object FileUtils {
    private const val TAG = "FileUtils"

    @Throws(IOException::class)
    fun copyAssets(assetManager: AssetManager, srcDir: String, dstDir: File?) {
        val files = assetManager.list(srcDir) ?: emptyArray()
        for (file in files) {
            val srcPath = if (srcDir.isNotEmpty()) "$srcDir/$file" else file
            val dstFile = File(dstDir, file)

            // if (dstFile.exists() && !dstFile.isDirectory) continue

            try {
                assetManager.open(srcPath).use { inputStream ->
                    Log.v(TAG, "Copying $srcPath to $dstFile")
                    copyFile(inputStream, dstFile)
                }
            } catch (e: FileNotFoundException) {
                dstFile.mkdirs()
                copyAssets(assetManager, srcPath, dstFile)
                Log.v(TAG, "File not found. Making directories and copying assets.", e)
            } catch (e: IOException) {
                Log.e(TAG, "Unable to copy $srcPath to $dstFile", e)
            }
        }
    }

    private const val MAX_BUF_SIZE = 8192

    @Throws(IOException::class)
    fun copyFile(inputStream: InputStream, dstFile: File, onProgress: (progress: Int) -> Unit = {}) {
        FileOutputStream(dstFile).use { out ->
            val buf = ByteArray(MAX_BUF_SIZE)
            var len: Int
            var bytesCopied = 0L
            var lastProgress = 0
            val totalSize = inputStream.available().toLong().takeIf { it > 0 } ?: -1

            while (inputStream.read(buf).also { len = it } > 0) {
                out.write(buf, 0, len)
                bytesCopied += len

                if (totalSize > 0) {
                    val progress = ((bytesCopied.toDouble() / totalSize) * 100).toInt()
                    if (progress > lastProgress) {
                        lastProgress = progress
                        onProgress(progress)
                    }
                }
            }
        }
    }

    @Throws(IOException::class)
    fun copyFile(context: Context, uri: Uri, dstFile: File, onProgress: (progress: Int) -> Unit = {}) {
        context.contentResolver.openInputStream(uri)?.use { inputStream -> copyFile(inputStream, dstFile, onProgress) }
            ?: throw IOException("Unable to open InputStream for URI: $uri")
    }

    @Throws(IOException::class)
    fun copyDirectoryContents(srcDir: File, dstDir: File, overwrite: Boolean = true) {
        if (!dstDir.exists() && !dstDir.mkdirs()) {
            throw IOException("Failed to create destination directory: ${dstDir.absolutePath}")
        }

        srcDir.listFiles()?.forEach { file ->
            val targetFile = File(dstDir, file.name)
            if (file.isDirectory) {
                file.copyRecursively(targetFile, overwrite)
            } else {
                file.copyTo(targetFile, overwrite)
            }
        }
    }

    fun findFileByExtension(directory: File, extension: String): File? {
        directory.listFiles()?.forEach { file ->
            when {
                file.isDirectory ->
                    findFileByExtension(file, extension)?.let {
                        return it
                    }

                file.extension.equals(extension, ignoreCase = true) -> return file
            }
        }
        return null
    }

    fun filenameFromUri(context: Context, uri: Uri): String? {
        return when (uri.scheme) {
            "content" -> {
                context.contentResolver.query(uri, null, null, null, null)?.use { cursor ->
                    if (cursor.moveToFirst()) {
                        cursor.getString(cursor.getColumnIndexOrThrow(OpenableColumns.DISPLAY_NAME))
                    } else null
                }
            }

            "file" -> {
                File(uri.path ?: "").name
            }

            else -> null
        }
    }

    fun hasValidExtension(filename: String, extensions: Array<String>): Boolean {
        return extensions.any { filename.endsWith(it, ignoreCase = true) }
    }
}
