package org.vpinball.app.util

import android.graphics.Bitmap
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint

fun Bitmap.resizeWithAspectFit(newWidth: Int, newHeight: Int): Bitmap {
    val widthRatio = newWidth.toFloat() / this.width
    val heightRatio = newHeight.toFloat() / this.height
    val aspectFitRatio = minOf(widthRatio, heightRatio)

    val aspectFitWidth = (this.width * aspectFitRatio).toInt()
    val aspectFitHeight = (this.height * aspectFitRatio).toInt()

    val resizedBitmap = Bitmap.createBitmap(newWidth, newHeight, Bitmap.Config.ARGB_8888)
    val canvas = Canvas(resizedBitmap)

    val paint = Paint().apply { color = Color.BLACK }
    canvas.drawRect(0f, 0f, newWidth.toFloat(), newHeight.toFloat(), paint)

    val left = (newWidth - aspectFitWidth) / 2f
    val top = (newHeight - aspectFitHeight) / 2f
    val destRect = android.graphics.RectF(left, top, left + aspectFitWidth, top + aspectFitHeight)
    canvas.drawBitmap(this, null, destRect, null)

    return resizedBitmap
}
