package org.vpinball.app.ui.screens.splash

import androidx.compose.foundation.Image
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import org.vpinball.app.R
import org.vpinball.app.VPinballManager
import org.vpinball.app.ui.theme.LightBlack

@Composable
fun SplashScreen(modifier: Modifier = Modifier) {
    Scaffold(
        modifier = modifier.fillMaxSize(),
        containerColor = Color.LightBlack,
        content = { paddingValues ->
            Column(modifier = Modifier.fillMaxSize().padding(paddingValues), horizontalAlignment = Alignment.CenterHorizontally) {
                Spacer(modifier = Modifier.weight(1f))

                Image(
                    painter = painterResource(id = R.drawable.img_vpinball_logo),
                    contentDescription = null,
                    modifier = Modifier.weight(1f).align(Alignment.CenterHorizontally),
                )

                Spacer(modifier = Modifier.weight(1f))

                Text(
                    text = VPinballManager.getVersionString(),
                    color = Color.White,
                    textAlign = TextAlign.Center,
                    style = MaterialTheme.typography.bodySmall,
                    modifier = Modifier.padding(24.dp).align(Alignment.CenterHorizontally),
                )
            }
        },
    )
}
