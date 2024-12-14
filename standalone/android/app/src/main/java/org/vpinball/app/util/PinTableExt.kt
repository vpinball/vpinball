package org.vpinball.app.util

import android.graphics.Bitmap
import android.graphics.BitmapFactory
import androidx.compose.ui.graphics.ImageBitmap
import androidx.compose.ui.graphics.asImageBitmap
import java.io.File
import java.io.FileOutputStream
import org.vpinball.app.VPinballManager
import org.vpinball.app.data.entity.PinTable

val PinTable.basePath: File
    get() = File(VPinballManager.getFilesDir(), uuid)

val PinTable.tableFile: File
    get() = File(basePath, path)

val PinTable.baseFilename: String
    get() = path.substringBeforeLast('.', path)

val PinTable.artworkFile: File
    get() = File(basePath, "${baseFilename}.png")

val PinTable.iniFile: File
    get() = File(basePath, "${baseFilename}.ini")

val PinTable.scriptFile: File
    get() = File(basePath, "${baseFilename}.vbs")

fun PinTable.hasArtwork(): Boolean {
    return artworkFile.exists()
}

private const val MAX_IMAGE_QUALITY = 100

fun PinTable.saveArtwork(bitmap: Bitmap) {
    FileOutputStream(artworkFile).use { outputStream -> bitmap.compress(Bitmap.CompressFormat.PNG, MAX_IMAGE_QUALITY, outputStream) }
}

fun PinTable.loadArtwork(): ImageBitmap? {
    return if (hasArtwork()) {
        BitmapFactory.decodeFile(artworkFile.absolutePath)?.asImageBitmap()
    } else {
        null
    }
}

fun PinTable.deleteArtwork() {
    artworkFile.delete()
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
