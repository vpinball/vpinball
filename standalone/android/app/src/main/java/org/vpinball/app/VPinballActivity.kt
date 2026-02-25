package org.vpinball.app

import android.content.Intent
import android.os.Bundle
import android.view.WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.runtime.LaunchedEffect
import androidx.lifecycle.lifecycleScope
import kotlinx.coroutines.launch
import org.koin.androidx.viewmodel.ext.android.viewModel
import org.vpinball.app.ui.VPinballContent
import org.vpinball.app.ui.screens.landing.LandingScreenViewModel

class VPinballActivity : ComponentActivity() {
    val viewModel: VPinballModel by viewModel()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        window.addFlags(FLAG_KEEP_SCREEN_ON)

        VPinballManager.onActivityReady(this)

        VPinballManager.whenReady { TableManager.initialize(this) }

        handleIntent(intent)

        setContent {
            LaunchedEffect(Unit) { viewModel.startSplashTimer() }

            LaunchedEffect(viewModel.showHUD, viewModel.activeTable) {
                if (viewModel.showHUD && viewModel.activeTable != null) {
                    val table = viewModel.activeTable!!

                    val success = VPinballManager.load(table) { progress, status -> lifecycleScope.launch { viewModel.updateHUD(progress, status) } }

                    if (success) {
                        val intent = Intent(this@VPinballActivity, VPinballPlayerActivity::class.java)
                        startActivity(intent)
                        @Suppress("DEPRECATION") overridePendingTransition(android.R.anim.fade_in, android.R.anim.fade_out)
                    }
                }
            }

            VPinballContent(viewModel = viewModel)
        }
    }

    override fun onNewIntent(intent: Intent) {
        super.onNewIntent(intent)
        handleIntent(intent)
    }

    override fun onDestroy() {
        VPinballManager.setMainActivity(null)
        super.onDestroy()
    }

    private fun handleIntent(intent: Intent?) {
        val uri = intent?.data ?: return
        LandingScreenViewModel.triggerOpenUri(uri)
    }
}
