package org.vpinball.app

import android.content.Intent
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import androidx.lifecycle.lifecycleScope
import kotlinx.coroutines.launch
import org.koin.android.ext.android.inject
import org.vpinball.app.ui.VPinballContent

class VPinballActivity : ComponentActivity() {
    val viewModel: VPinballViewModel by inject()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        VPinballManager.onActivityReady(this)

        VPinballManager.whenReady { TableManager.initialize(this) }

        setContent {
            val state by viewModel.state.collectAsStateWithLifecycle()

            LaunchedEffect(Unit) { viewModel.startSplashTimer() }

            LaunchedEffect(state.loading, state.table) {
                if (state.loading && state.table != null) {
                    val table = state.table!!

                    val success =
                        VPinballManager.load(table) { progress, status ->
                            lifecycleScope.launch {
                                viewModel.progress(progress)
                                viewModel.status(status)
                            }
                        }

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

    override fun onDestroy() {
        VPinballManager.setMainActivity(null)
        super.onDestroy()
    }
}
