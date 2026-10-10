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
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.FilledTonalButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import org.vpinball.app.Link
import org.vpinball.app.R
import org.vpinball.app.ui.theme.subtitleColor
import org.vpinball.app.util.drawWithGradient

@Composable
fun BlinkingTitle(text: String, modifier: Modifier = Modifier) {
    val infiniteTransition = rememberInfiniteTransition(label = "blink")
    val alpha by
        infiniteTransition.animateFloat(
            initialValue = 1f,
            targetValue = 0f,
            animationSpec = infiniteRepeatable(animation = tween(durationMillis = 750), repeatMode = RepeatMode.Reverse),
            label = "blink_alpha",
        )

    Text(
        text = text,
        modifier = modifier.alpha(alpha),
        style = MaterialTheme.typography.headlineSmall,
        fontWeight = FontWeight.Bold,
        color = MaterialTheme.colorScheme.onBackground,
    )
}

@Composable
fun EmptyTablesList(modifier: Modifier = Modifier) {
    val context = LocalContext.current

    Column(modifier = modifier, horizontalAlignment = Alignment.CenterHorizontally, verticalArrangement = Arrangement.spacedBy(32.dp)) {
        Image(
            painter = painterResource(R.drawable.img_table_placeholder),
            contentDescription = null,
            modifier = Modifier.weight(0.9f).drawWithGradient(),
        )

        Column(
            modifier = Modifier.fillMaxWidth(),
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.spacedBy(12.dp),
        ) {
            BlinkingTitle(text = "Free Play")

            Text(
                text = "Tap + to add your first table.",
                textAlign = TextAlign.Center,
                style = MaterialTheme.typography.bodyLarge,
                color = subtitleColor,
                modifier = Modifier.padding(horizontal = 40.dp),
            )

            FilledTonalButton(onClick = { Link.DOCS.open(context) }, modifier = Modifier.padding(top = 4.dp)) {
                Text(text = "Learn More...", fontWeight = FontWeight.SemiBold, color = MaterialTheme.colorScheme.onBackground)
            }
        }
    }
}
