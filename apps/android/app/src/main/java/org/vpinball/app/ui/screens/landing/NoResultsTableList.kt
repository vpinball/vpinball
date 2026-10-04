package org.vpinball.app.ui.screens.landing

import androidx.compose.animation.core.RepeatMode
import androidx.compose.animation.core.animateFloat
import androidx.compose.animation.core.infiniteRepeatable
import androidx.compose.animation.core.rememberInfiniteTransition
import androidx.compose.animation.core.tween
import androidx.compose.foundation.Image
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import org.vpinball.app.R
import org.vpinball.app.ui.theme.VpxDarkYellow
import org.vpinball.app.util.drawWithGradient

@Composable
fun NoResultsTableList(modifier: Modifier = Modifier) {
    val infiniteTransition = rememberInfiniteTransition(label = "LandingScreen Infinite Transition")
    val textAlpha by
        infiniteTransition.animateFloat(
            initialValue = 1f,
            targetValue = 0f,
            animationSpec = infiniteRepeatable(animation = tween(durationMillis = 750), repeatMode = RepeatMode.Reverse),
            label = "LandingScreen textAlpha transition",
        )

    Column(modifier = modifier, horizontalAlignment = Alignment.CenterHorizontally, verticalArrangement = Arrangement.spacedBy(20.dp)) {
        Image(
            painter = painterResource(R.drawable.img_table_placeholder),
            contentDescription = null,
            modifier = Modifier.weight(0.9f).drawWithGradient(),
        )

        Column(
            modifier = Modifier.fillMaxWidth(),
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.spacedBy(10.dp),
        ) {
            Text(
                text = "Shoot Again!",
                modifier = Modifier.alpha(textAlpha),
                style = MaterialTheme.typography.headlineSmall,
                fontWeight = FontWeight.Bold,
                color = Color.VpxDarkYellow,
            )
            Text(
                text = "Please make sure the table name is correct, or try searching for another table.",
                textAlign = TextAlign.Center,
                color = Color.White,
            )
        }
    }
}
