package org.vpinball.app.ui.screens.liveui

import android.graphics.Bitmap
import androidx.compose.animation.AnimatedVisibility
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.pager.HorizontalPager
import androidx.compose.foundation.pager.rememberPagerState
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import org.vpinball.app.R
import org.vpinball.app.VPinballManager
import org.vpinball.app.VPinballViewModel
import org.vpinball.app.jni.VPinballSettingsSection.STANDALONE
import org.vpinball.app.ui.theme.VpxRed
import org.vpinball.app.ui.util.koinActivityViewModel
import org.vpinball.app.util.drawWithGradient

@Composable
fun LiveUIOverlay(onResume: () -> Unit, modifier: Modifier = Modifier, viewModel: VPinballViewModel = koinActivityViewModel()) {
    val state by viewModel.state.collectAsStateWithLifecycle()

    var capturedImage by remember { mutableStateOf<Bitmap?>(null) }
    val pagerState = rememberPagerState(pageCount = { 2 })

    var saveOverlayTitle by remember { mutableStateOf("") }
    var showSaveOverlay by remember { mutableStateOf(false) }

    var sliderActive by remember { mutableStateOf(false) }

    fun onSave(title: String) {
        saveOverlayTitle = title
        showSaveOverlay = true
        CoroutineScope(Dispatchers.Main).launch {
            delay(2000)
            showSaveOverlay = false
        }
    }

    Box(
        modifier =
            modifier
                .fillMaxSize()
                .padding(vertical = 40.dp)
                .padding(horizontal = 20.dp)
                .clip(RoundedCornerShape(16.dp))
                .background(Color.Black.copy(alpha = if (sliderActive) 0.0f else 0.5f))
    ) {
        Column(modifier = Modifier.fillMaxSize(), verticalArrangement = Arrangement.spacedBy(20.dp)) {
            Row(
                modifier = Modifier.fillMaxWidth().padding(top = 20.dp).alpha(if (sliderActive) 0.0f else 1f),
                horizontalArrangement = Arrangement.SpaceEvenly,
            ) {
                Column(horizontalAlignment = Alignment.CenterHorizontally) {
                    IconButton(onClick = onResume, modifier = Modifier.size(60.dp).clip(RoundedCornerShape(10.dp)).background(Color.Gray)) {
                        Icon(
                            painter = painterResource(id = R.drawable.img_noun_pinball_machine_17955),
                            contentDescription = null,
                            tint = Color.White,
                            modifier = Modifier.padding(10.dp),
                        )
                    }
                    Text(text = "Resume", color = Color.White, fontSize = 12.sp)
                }

                Column(horizontalAlignment = Alignment.CenterHorizontally) {
                    IconButton(
                        onClick = {
                            val touchOverlay = !VPinballManager.loadValue(STANDALONE, "TouchOverlay", false)
                            VPinballManager.saveValue(STANDALONE, "TouchOverlay", touchOverlay)
                            viewModel.touchOverlay(touchOverlay)
                        },
                        modifier = Modifier.size(60.dp).clip(RoundedCornerShape(10.dp)).background(Color.Gray),
                    ) {
                        Icon(
                            painter =
                                painterResource(
                                    id =
                                        if (state.touchOverlay) R.drawable.img_sf_square_stack_3d_down_forward_fill
                                        else R.drawable.img_sf_square_stack_3d_down_forward
                                ),
                            contentDescription = null,
                            tint = Color.White,
                        )
                    }
                    Text(text = "Overlay", color = Color.White, fontSize = 12.sp)
                }

                Column(horizontalAlignment = Alignment.CenterHorizontally) {
                    IconButton(
                        onClick = { VPinballManager.toggleFPS() },
                        modifier = Modifier.size(60.dp).clip(RoundedCornerShape(10.dp)).background(Color.Gray),
                    ) {
                        Icon(
                            painter = painterResource(id = R.drawable.img_sf_gauge_open_with_lines_needle_33percent),
                            contentDescription = null,
                            tint = Color.White,
                        )
                    }
                    Text(text = "FPS", color = Color.White, fontSize = 12.sp)
                }

                Column(horizontalAlignment = Alignment.CenterHorizontally) {
                    IconButton(
                        onClick = {
                            VPinballManager.captureBitmap { bitmap ->
                                if (bitmap != null) {
                                    capturedImage = bitmap

                                    CoroutineScope(Dispatchers.IO).launch { VPinballManager.saveBitmap(bitmap) }
                                }
                            }
                        },
                        modifier = Modifier.size(60.dp).clip(RoundedCornerShape(10.dp)).background(Color.Gray),
                    ) {
                        Icon(painter = painterResource(id = R.drawable.img_sf_photo_on_rectangle), contentDescription = null, tint = Color.White)
                    }
                    Text(text = "Artwork", color = Color.White, fontSize = 12.sp)
                }

                Column(horizontalAlignment = Alignment.CenterHorizontally) {
                    IconButton(
                        onClick = {
                            onResume()
                            CoroutineScope(Dispatchers.Main).launch {
                                delay(500)
                                VPinballManager.stop()
                            }
                        },
                        modifier = Modifier.size(60.dp).clip(RoundedCornerShape(10.dp)).background(Color.Gray),
                    ) {
                        Icon(painter = painterResource(id = R.drawable.img_sf_house), contentDescription = null, tint = Color.White)
                    }
                    Text(text = "Quit", color = Color.White, fontSize = 12.sp)
                }
            }

            HorizontalPager(state = pagerState, modifier = Modifier.fillMaxWidth().weight(1f)) { page ->
                if (page == 0) {
                    TableOptionsPage(onSave = { title -> onSave(title) }, onActiveSliderChanged = { sliderActive = it })
                } else {
                    PointOfViewPage(onSave = { title -> onSave(title) }, onActiveSliderChanged = { sliderActive = it })
                }
            }

            Row(Modifier.fillMaxWidth().padding(bottom = 20.dp).alpha(if (sliderActive) 0.0f else 1f), horizontalArrangement = Arrangement.Center) {
                repeat(pagerState.pageCount) { iteration ->
                    val color = if (pagerState.currentPage == iteration) Color.VpxRed else Color.LightGray
                    Box(modifier = Modifier.padding(vertical = 2.dp, horizontal = 4.dp).clip(CircleShape).background(color).size(8.dp))
                }
            }
        }
    }

    capturedImage?.let { image -> CameraOverlay(capturedImage = image, onComplete = { capturedImage = null }) }

    AnimatedVisibility(visible = showSaveOverlay, modifier = Modifier.fillMaxSize()) {
        Box(modifier = Modifier.fillMaxSize().background(Color.Black.copy(alpha = 0.5f)), contentAlignment = Alignment.Center) {
            Column(
                horizontalAlignment = Alignment.CenterHorizontally,
                modifier = Modifier.background(Color.Black.copy(alpha = 0.8f), shape = RoundedCornerShape(12.dp)).padding(20.dp),
                verticalArrangement = Arrangement.spacedBy(12.dp),
            ) {
                Image(
                    painter = painterResource(R.drawable.img_noun_pinball_3169564),
                    contentDescription = null,
                    modifier = Modifier.size(50.dp).drawWithGradient(),
                )

                Text(text = saveOverlayTitle, style = MaterialTheme.typography.titleMedium, color = Color.White, textAlign = TextAlign.Center)
            }
        }
    }
}
