package org.vpinball.app.ui.screens.loading

import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.MaterialTheme
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.produceState
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.blur
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.ImageBitmap
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.unit.dp
import dev.chrisbanes.haze.hazeSource
import dev.chrisbanes.haze.rememberHazeState
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import org.vpinball.app.R
import org.vpinball.app.Table
import org.vpinball.app.ui.screens.common.ProgressOverlay
import org.vpinball.app.util.drawWithGradient
import org.vpinball.app.util.loadImage

@Composable
fun LoadingScreen(table: Table, progress: Int, status: String?, modifier: Modifier = Modifier) {
    val hazeState = rememberHazeState()

    val bitmap by
        produceState<ImageBitmap?>(null, table.uuid, table.image, table.modifiedAt) { value = withContext(Dispatchers.IO) { table.loadImage() } }

    Box(modifier = modifier.fillMaxSize().background(MaterialTheme.colorScheme.background)) {
        Box(modifier = Modifier.fillMaxSize().hazeSource(hazeState)) {
            val image = bitmap
            if (image != null) {
                Image(
                    bitmap = image,
                    contentDescription = null,
                    modifier = Modifier.fillMaxSize().blur(40.dp),
                    contentScale = ContentScale.Crop,
                )
                Box(modifier = Modifier.fillMaxSize().background(Color.Black.copy(alpha = 0.35f)))
                Image(bitmap = image, contentDescription = null, modifier = Modifier.fillMaxSize(), contentScale = ContentScale.Fit)
            } else {
                Image(
                    painter = painterResource(R.drawable.img_table_placeholder),
                    contentDescription = null,
                    modifier = Modifier.fillMaxSize().padding(vertical = 40.dp).drawWithGradient(),
                    contentScale = ContentScale.Fit,
                )
            }
        }

        ProgressOverlay(title = table.name, progress = progress, status = status, hazeState = hazeState)
    }
}
