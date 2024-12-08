package org.vpinball.app.util

import android.graphics.Bitmap
import android.view.PixelCopy
import android.view.PixelCopy.OnPixelCopyFinishedListener
import android.view.SurfaceView

fun SurfaceView.captureBitmap(onSuccess: (Bitmap) -> Unit) {
    val bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888)
    val listener = OnPixelCopyFinishedListener { onSuccess(bitmap) }
    PixelCopy.request(this, bitmap, listener, handler)
}
