package org.vpinball.app.util

import android.graphics.Bitmap
import android.graphics.BitmapFactory
import androidx.compose.ui.graphics.ImageBitmap
import androidx.compose.ui.graphics.asImageBitmap
import java.io.File
import java.io.FileOutputStream
import org.vpinball.app.VPinballManager
import org.vpinball.app.data.entity.PinTable

private const val MAX_IMAGE_QUALITY = 75

val PinTable.basePath: File
    get() = File(VPinballManager.getFilesDir(), uuid)

val PinTable.tableFile: File
    get() = File(basePath, path)

val PinTable.baseFilename: String
    get() = path.substringBeforeLast('.', path)

val PinTable.imageFile: File
    get() = File(basePath, "${baseFilename}.jpg")

val PinTable.iniFile: File
    get() = File(basePath, "${baseFilename}.ini")

val PinTable.scriptFile: File
    get() = File(basePath, "${baseFilename}.vbs")

fun PinTable.hasImage(): Boolean {
    return imageFile.exists()
}

fun PinTable.hasIni(): Boolean {
    return iniFile.exists()
}

fun PinTable.resetIni() {
    iniFile.delete()
}

fun PinTable.hasScript(): Boolean {
    return scriptFile.exists()
}

fun PinTable.deleteFiles() {
    basePath.deleteRecursively()
}

fun PinTable.loadImage(): ImageBitmap? {
    return if (hasImage()) {
        BitmapFactory.decodeFile(imageFile.absolutePath)?.asImageBitmap()
    } else {
        null
    }
}

fun PinTable.updateImage(bitmap: Bitmap) {
    FileOutputStream(imageFile).use { outputStream -> bitmap.compress(Bitmap.CompressFormat.JPEG, MAX_IMAGE_QUALITY, outputStream) }
}

fun PinTable.resetImage() {
    imageFile.delete()
}
