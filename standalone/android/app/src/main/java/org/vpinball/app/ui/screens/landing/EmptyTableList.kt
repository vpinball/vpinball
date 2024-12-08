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
import androidx.compose.runtime.remember
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.toArgb
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.text.LinkAnnotation
import androidx.compose.ui.text.SpanStyle
import androidx.compose.ui.text.TextLinkStyles
import androidx.compose.ui.text.buildAnnotatedString
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.text.withLink
import androidx.compose.ui.unit.dp
import androidx.core.graphics.ColorUtils
import org.vpinball.app.Link
import org.vpinball.app.R
import org.vpinball.app.ui.theme.VpxDarkYellow
import org.vpinball.app.util.drawWithGradient

@Composable
fun EmptyTablesList(modifier: Modifier = Modifier) {
    val infiniteTransition = rememberInfiniteTransition(label = "LandingScreen Infinite Transition")
    val textAlpha by
        infiniteTransition.animateFloat(
            initialValue = 1f,
            targetValue = 0f,
            animationSpec = infiniteRepeatable(animation = tween(durationMillis = 750), repeatMode = RepeatMode.Reverse),
            label = "LandingScreen textAlpha transition",
        )

    val context = LocalContext.current
    val learnMoreLink = remember {
        buildAnnotatedString {
            withLink(
                LinkAnnotation.Clickable(
                    tag = "link",
                    styles =
                        TextLinkStyles(
                            style = SpanStyle(Color.VpxDarkYellow),
                            pressedStyle = SpanStyle(Color(ColorUtils.blendARGB(Color.VpxDarkYellow.toArgb(), Color.Black.toArgb(), 0.2f))),
                        ),
                    linkInteractionListener = { Link.DOCS.open(context) },
                )
            ) {
                append("Learn More...")
            }
        }
    }

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
                text = "Insert Tables",
                modifier = Modifier.alpha(textAlpha),
                style = MaterialTheme.typography.headlineSmall,
                fontWeight = FontWeight.Bold,
                color = Color.VpxDarkYellow,
            )
            Text(text = "You can import tables by pressing the + button in the top right.", textAlign = TextAlign.Center, color = Color.White)

            Text(text = learnMoreLink)
        }
    }
}
