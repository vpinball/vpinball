package org.vpinball.app.ui.screens.liveui

import android.graphics.Bitmap
import android.media.MediaPlayer
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.asImageBitmap
import androidx.compose.ui.platform.LocalContext
import kotlinx.coroutines.delay
import org.vpinball.app.R

@Composable
fun CameraOverlay(capturedImage: Bitmap, onComplete: () -> Unit) {
    val context = LocalContext.current
    val mediaPlayer = remember { MediaPlayer.create(context, R.raw.shutter) }
    var showFlash by remember { mutableStateOf(true) }

    LaunchedEffect(capturedImage) {
        mediaPlayer.start()

        delay(300)
        showFlash = false
        delay(2000)
        onComplete()
    }

    Box(modifier = Modifier.fillMaxSize().background(Color.Black.copy(alpha = 0.8f))) {
        Image(bitmap = capturedImage.asImageBitmap(), contentDescription = "Captured Image", modifier = Modifier.fillMaxWidth())
    }

    if (showFlash) {
        Box(modifier = Modifier.fillMaxSize().background(Color.White))
    }
}
