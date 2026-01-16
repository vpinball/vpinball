package org.vpinball.app.util

import android.graphics.Bitmap
import android.graphics.BitmapFactory
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

val Table.baseFilename: String
    get() = fileName.substringBeforeLast('.', fileName)

fun Table.resetIni() {
    TableManager.getInstance().resetTableIni(this)

    CoroutineScope(Dispatchers.Main).launch { LandingScreenViewModel.triggerUpdateTable(this@resetIni) }
}

fun Table.loadImage(): ImageBitmap? {
    if (image.isEmpty()) return null

    try {
        if (SAFFileSystem.isUsingSAF()) {
            val inputStream = SAFFileSystem.openInputStream(image) ?: return null
            return inputStream.use { stream -> BitmapFactory.decodeStream(stream)?.asImageBitmap() }
        } else {
            return BitmapFactory.decodeFile(imagePath)?.asImageBitmap()
        }
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
    val tablesPath = VPinballManager.getPath(VPinballPath.TABLES)
    val iniRelativePath = path.substringBeforeLast('.') + ".ini"
    val iniFullPath =
        if (SAFFileSystem.isUsingSAF()) {
            "$tablesPath$iniRelativePath"
        } else {
            java.io.File(tablesPath, iniRelativePath).absolutePath
        }

    return if (SAFFileSystem.isUsingSAF()) {
        SAFFileSystem.exists(iniRelativePath)
    } else {
        java.io.File(iniFullPath).exists()
    }
}
