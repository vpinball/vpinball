package org.vpinball.app

import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import kotlin.time.Duration.Companion.milliseconds
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch

class VPinballModel : ViewModel() {
    var tables by mutableStateOf(emptyList<Table>())
    var activeTable: Table? = null
    var isPlaying by mutableStateOf(false)
    var webServerURL by mutableStateOf<String?>(null)
    var hudTitle by mutableStateOf<String?>(null)
    var hudProgress by mutableIntStateOf(0)
    var hudStatus by mutableStateOf<String?>(null)
    var showSplash by mutableStateOf(true)
    var showHUD by mutableStateOf(false)

    private var splashTimerStarted = false

    fun startSplashTimer() {
        if (!splashTimerStarted) {
            splashTimerStarted = true
            viewModelScope.launch {
                delay(SPLASH_DELAY_DURATION)
                showSplash = false
            }
        }
    }

    fun showHUD(title: String, status: String) {
        hudTitle = title
        hudProgress = 0
        hudStatus = status
        showHUD = true
    }

    fun updateHUD(progress: Int, status: String) {
        hudProgress = progress
        hudStatus = status
    }

    fun updateHUD(progress: Int) {
        hudProgress = progress
    }

    fun hideHUD() {
        showHUD = false
    }

    fun launchInViewModelScope(block: suspend () -> Unit) {
        viewModelScope.launch { block() }
    }

    companion object {
        private val SPLASH_DELAY_DURATION = 2000.milliseconds
    }
}
