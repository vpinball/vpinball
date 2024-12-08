package org.vpinball.app.ui.screens.loading

import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.runtime.Composable
import androidx.compose.runtime.remember
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.res.painterResource
import dev.chrisbanes.haze.HazeState
import dev.chrisbanes.haze.haze
import org.vpinball.app.R
import org.vpinball.app.data.entity.PinTable
import org.vpinball.app.ui.screens.common.ProgressOverlay
import org.vpinball.app.ui.theme.LightBlack
import org.vpinball.app.util.drawWithGradient
import org.vpinball.app.util.loadArtwork

@Composable
fun LoadingScreen(table: PinTable, progress: Int, status: String?, modifier: Modifier = Modifier) {
    val hazeState = remember { HazeState() }

    val bitmap = remember { table.loadArtwork() }

    Box(modifier = modifier.fillMaxSize().background(Color.LightBlack)) {
        if (bitmap != null) {
            Image(bitmap = bitmap, contentDescription = null, modifier = Modifier.fillMaxSize().haze(state = hazeState))
        } else {
            Image(
                painter = painterResource(R.drawable.img_table_placeholder),
                contentDescription = null,
                modifier = Modifier.fillMaxSize().haze(state = hazeState).drawWithGradient(),
            )
        }

        ProgressOverlay(title = table.name, progress = progress, status = status, hazeState = hazeState)
    }
}
