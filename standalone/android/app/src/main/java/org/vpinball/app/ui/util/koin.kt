package org.vpinball.app.ui.util

import androidx.activity.ComponentActivity
import androidx.compose.runtime.Composable
import androidx.compose.ui.platform.LocalContext
import androidx.lifecycle.ViewModel
import org.koin.androidx.compose.koinViewModel

@Composable
inline fun <reified T : ViewModel> koinActivityViewModel() = koinViewModel<T>(viewModelStoreOwner = LocalContext.current as ComponentActivity)
