package org.vpinball.app.ui.screens.landing

import androidx.compose.animation.Crossfade
import androidx.compose.foundation.Image
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.runtime.Composable
import androidx.compose.runtime.derivedStateOf
import androidx.compose.runtime.getValue
import androidx.compose.runtime.produceState
import androidx.compose.runtime.remember
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.ImageBitmap
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.res.painterResource
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import org.vpinball.app.R
import org.vpinball.app.Table
import org.vpinball.app.TableImageState
import org.vpinball.app.util.drawWithGradient
import org.vpinball.app.util.loadImage

@Composable
fun TableImageView(table: Table, modifier: Modifier = Modifier) {
    val bitmap by
        produceState<ImageBitmap?>(null, table.uuid, table.image, table.modifiedAt) { value = withContext(Dispatchers.IO) { table.loadImage() } }

    val imageState by remember {
        derivedStateOf {
            if (bitmap != null) {
                TableImageState.IMAGE_LOADED
            } else {
                TableImageState.NO_IMAGE
            }
        }
    }

    Crossfade(imageState, modifier = modifier, label = "table_image_cross_fade") { state ->
        when (state) {
            TableImageState.IMAGE_LOADED -> {
                bitmap?.let { bmp ->
                    Box(modifier = Modifier.fillMaxSize()) {
                        Image(
                            bitmap = bmp,
                            contentDescription = null,
                            modifier = Modifier.fillMaxSize(),
                            contentScale = ContentScale.Fit,
                            alignment = Alignment.Center,
                        )
                    }
                }
            }

            TableImageState.LOADING_IMAGE -> {
                Box {}
            }

            else -> {
                Box(modifier = Modifier.fillMaxSize()) {
                    Image(
                        painter = painterResource(R.drawable.img_table_placeholder),
                        contentDescription = null,
                        modifier = Modifier.fillMaxSize().drawWithGradient(),
                        contentScale = ContentScale.Fit,
                        alignment = Alignment.Center,
                    )
                }
            }
        }
    }
}
