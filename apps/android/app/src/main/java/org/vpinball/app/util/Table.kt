package org.vpinball.app.util

import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.util.LruCache
import androidx.compose.ui.graphics.ImageBitmap
import androidx.compose.ui.graphics.asImageBitmap
import java.io.File
import java.io.FileOutputStream
import java.io.InputStream
import org.vpinball.app.SAFFileSystem
import org.vpinball.app.Table
import org.vpinball.app.TableManager
import org.vpinball.app.VPinballManager
import org.vpinball.app.jni.VPinballLogLevel
import org.vpinball.app.jni.VPinballPath
import org.vpinball.app.ui.screens.landing.LandingScreenViewModel

private const val MAX_IMAGE_QUALITY = 80
private const val IMAGE_CACHE_BYTES = 96 * 1024 * 1024
const val THUMBNAIL_MAX_PIXELS = 1024

private val tableImageCache =
    object : LruCache<String, ImageBitmap>(IMAGE_CACHE_BYTES) {
        override fun sizeOf(key: String, value: ImageBitmap): Int = value.width * value.height * 4
    }

fun Table.resetIni() {
    TableManager.getInstance().resetTableIni(this)
    LandingScreenViewModel.triggerUpdateTable(this)
}

fun Table.cachedImage(maxPixels: Int = 0): ImageBitmap? {
    if (image.isEmpty()) return null
    return tableImageCache.get("${uuid}_${modifiedAt}_$maxPixels")
}

fun Table.loadImage(maxPixels: Int = 0): ImageBitmap? {
    if (image.isEmpty()) return null

    val cacheKey = "${uuid}_${modifiedAt}_$maxPixels"
    tableImageCache.get(cacheKey)?.let {
        return it
    }

    try {
        val bitmap = decodeImage(maxPixels)?.asImageBitmap()
        if (bitmap != null) {
            tableImageCache.put(cacheKey, bitmap)
        }
        return bitmap
    } catch (e: Exception) {
        VPinballManager.log(VPinballLogLevel.ERROR, "Failed to load image: ${e.message}")
        return null
    }
}

private fun Table.openImageStream(): InputStream? =
    if (SAFFileSystem.isUsingSAF()) SAFFileSystem.openInputStream(image) else File(imagePath).takeIf { it.exists() }?.inputStream()

private fun Table.decodeImage(maxPixels: Int): Bitmap? {
    val options = BitmapFactory.Options()
    if (maxPixels > 0) {
        options.inJustDecodeBounds = true
        openImageStream()?.use { BitmapFactory.decodeStream(it, null, options) }
        val largest = maxOf(options.outWidth, options.outHeight)
        var sampleSize = 1
        while (largest / (sampleSize * 2) >= maxPixels) {
            sampleSize *= 2
        }
        options.inSampleSize = sampleSize
        options.inJustDecodeBounds = false
    }
    options.inPreferredConfig = Bitmap.Config.ARGB_8888
    return openImageStream()?.use { BitmapFactory.decodeStream(it, null, options) }
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

    return if (SAFFileSystem.isUsingSAF()) {
        SAFFileSystem.exists(scriptRelativePath)
    } else {
        File(tablesPath, scriptRelativePath).exists()
    }
}

fun Table.hasIniFile(): Boolean {
    return if (SAFFileSystem.isUsingSAF()) {
        val iniRelativePath = path.substringBeforeLast('.') + ".ini"
        SAFFileSystem.exists(iniRelativePath)
    } else {
        File(iniPath).exists()
    }
}
