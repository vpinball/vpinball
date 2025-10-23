package org.vpinball.app

import android.os.Bundle
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import org.libsdl.app.SDLActivity
import org.vpinball.app.jni.VPinballLogLevel

class VPinballPlayerActivity : SDLActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        if (BuildConfig.IS_QUEST) {
            VPinballManager.vpinballJNI.VPinballInitOpenXR(this)
        }

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

    override fun onStop() {
        VPinballManager.log(VPinballLogLevel.INFO, "VPinballPlayerActivity: onStop: isFinishing=$isFinishing")

        if (!isFinishing) {
            VPinballManager.log(VPinballLogLevel.INFO, "VPinballPlayerActivity: force quit detected, exiting")
            System.exit(0)
        }

        super.onStop()
    }

    override fun onDestroy() {
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
