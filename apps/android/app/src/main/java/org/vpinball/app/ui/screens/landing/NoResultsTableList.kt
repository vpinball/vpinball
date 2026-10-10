package org.vpinball.app.ui.screens.landing

import androidx.compose.foundation.Image
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import org.vpinball.app.R
import org.vpinball.app.ui.theme.subtitleColor
import org.vpinball.app.util.drawWithGradient

@Composable
fun NoResultsTableList(modifier: Modifier = Modifier) {
    Column(modifier = modifier, horizontalAlignment = Alignment.CenterHorizontally, verticalArrangement = Arrangement.spacedBy(24.dp)) {
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
            BlinkingTitle(text = "Shoot Again!")

            Text(
                text = "Check the spelling or try a new search.",
                textAlign = TextAlign.Center,
                style = MaterialTheme.typography.bodyLarge,
                color = subtitleColor,
                modifier = Modifier.padding(horizontal = 40.dp),
            )
        }
    }
}
