package org.vpinball.app.util

import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.util.LruCache
import androidx.compose.ui.graphics.ImageBitmap
import androidx.compose.ui.graphics.asImageBitmap
import java.io.File
import java.io.FileOutputStream
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import org.vpinball.app.SAFFileSystem
import org.vpinball.app.Table
import org.vpinball.app.TableManager
import org.vpinball.app.VPinballManager
import org.vpinball.app.jni.VPinballLogLevel
import org.vpinball.app.jni.VPinballPath
import org.vpinball.app.ui.screens.landing.LandingScreenViewModel

private const val MAX_IMAGE_QUALITY = 80
private const val IMAGE_CACHE_SIZE = 64

private val tableImageCache = LruCache<String, ImageBitmap>(IMAGE_CACHE_SIZE)

val Table.baseFilename: String
    get() = fileName.substringBeforeLast('.', fileName)

fun Table.resetIni() {
    TableManager.getInstance().resetTableIni(this)

    CoroutineScope(Dispatchers.Main).launch { LandingScreenViewModel.triggerUpdateTable(this@resetIni) }
}

fun Table.loadImage(): ImageBitmap? {
    if (image.isEmpty()) return null

    val cacheKey = "${uuid}_${modifiedAt}"
    tableImageCache.get(cacheKey)?.let {
        return it
    }

    try {
        val bitmap =
            if (SAFFileSystem.isUsingSAF()) {
                val inputStream = SAFFileSystem.openInputStream(image) ?: return null
                inputStream.use { stream -> BitmapFactory.decodeStream(stream)?.asImageBitmap() }
            } else {
                BitmapFactory.decodeFile(imagePath)?.asImageBitmap()
            }

        if (bitmap != null) {
            tableImageCache.put(cacheKey, bitmap)
        }

        return bitmap
    } catch (e: Exception) {
        VPinballManager.log(VPinballLogLevel.ERROR, "Failed to load image: ${e.message}")
        return null
    }
}

suspend fun Table.updateImage(bitmap: Bitmap) {
    val cacheDir = VPinballManager.getCacheDir()
    val tempFile = File(cacheDir, "temp_image_${uuid}.jpg")

    try {
        FileOutputStream(tempFile).use { outputStream -> bitmap.compress(Bitmap.CompressFormat.JPEG, MAX_IMAGE_QUALITY, outputStream) }
        TableManager.getInstance().setTableImage(this, tempFile.absolutePath)
    } finally {
        tempFile.delete()
    }
}

suspend fun Table.resetImage() {
    TableManager.getInstance().setTableImage(this, "")
}

fun Table.hasScriptFile(): Boolean {
    val tablesPath = VPinballManager.getPath(VPinballPath.TABLES)
    val scriptRelativePath = path.substringBeforeLast('.') + ".vbs"
    val scriptFullPath =
        if (SAFFileSystem.isUsingSAF()) {
            "$tablesPath$scriptRelativePath"
        } else {
            java.io.File(tablesPath, scriptRelativePath).absolutePath
        }

    return if (SAFFileSystem.isUsingSAF()) {
        SAFFileSystem.exists(scriptRelativePath)
    } else {
        java.io.File(scriptFullPath).exists()
    }
}

fun Table.hasIniFile(): Boolean {
    return if (SAFFileSystem.isUsingSAF()) {
        val iniRelativePath = path.substringBeforeLast('.') + ".ini"
        SAFFileSystem.exists(iniRelativePath)
    } else {
        java.io.File(iniPath).exists()
    }
}
