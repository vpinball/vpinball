package org.vpinball.app.ui.screens.landing

import androidx.compose.animation.Crossfade
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.aspectRatio
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.MaterialTheme
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.produceState
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.unit.dp
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import org.vpinball.app.R
import org.vpinball.app.Table
import org.vpinball.app.ui.theme.DarkBlack
import org.vpinball.app.util.THUMBNAIL_MAX_PIXELS
import org.vpinball.app.util.cachedImage
import org.vpinball.app.util.drawWithGradient
import org.vpinball.app.util.loadImage

const val TABLE_CARD_ASPECT = 1f / 2f
val TableCardShape = RoundedCornerShape(14.dp)

@Composable
fun TableImageView(table: Table, modifier: Modifier = Modifier) {
    val bitmap by
        produceState(table.cachedImage(THUMBNAIL_MAX_PIXELS), table.uuid, table.image, table.modifiedAt) {
            value = table.cachedImage(THUMBNAIL_MAX_PIXELS) ?: withContext(Dispatchers.IO) { table.loadImage(THUMBNAIL_MAX_PIXELS) }
        }

    Box(
        modifier =
            modifier
                .fillMaxWidth()
                .aspectRatio(TABLE_CARD_ASPECT)
                .clip(TableCardShape)
                .border(1.dp, MaterialTheme.colorScheme.onBackground.copy(alpha = 0.12f), TableCardShape)
    ) {
        Crossfade(bitmap, label = "table_image_cross_fade") { image ->
            if (image != null) {
                Image(bitmap = image, contentDescription = null, modifier = Modifier.fillMaxSize(), contentScale = ContentScale.Crop)
            } else {
                Box(modifier = Modifier.fillMaxSize().background(Color.DarkBlack), contentAlignment = Alignment.Center) {
                    Image(
                        painter = painterResource(R.drawable.img_table_placeholder),
                        contentDescription = null,
                        modifier = Modifier.fillMaxSize().padding(horizontal = 2.dp).drawWithGradient(dark = true),
                        contentScale = ContentScale.Fit,
                    )
                }
            }
        }
    }
}
