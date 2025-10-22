package org.vpinball.app

import android.os.Bundle
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import org.libsdl.app.SDLActivity

class VPinballPlayerActivity : SDLActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        VPinballManager.setPlayerActivity(this)

        CoroutineScope(Dispatchers.Main).launch {
            delay(2000)
            VPinballManager.play()
        }
    }

    override fun onWindowFocusChanged(hasFocus: Boolean) {
        super.onWindowFocusChanged(hasFocus)
        if (hasFocus) {
            setWindowStyle(true)
        }
    }

    override fun onDestroy() {
        VPinballManager.stop()
        VPinballManager.setPlayerActivity(null)
        super.onDestroy()
    }

    override fun finish() {
        super.finish()
        overridePendingTransition(android.R.anim.fade_in, android.R.anim.fade_out)
    }

    @Suppress("DEPRECATION")
    override fun onBackPressed() {
        VPinballManager.stop()
        finish()
    }

    override fun getLibraries(): Array<String> = arrayOf("vpinball")
}
