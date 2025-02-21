package org.vpinball.app

import android.graphics.Bitmap
import android.os.Bundle
import android.view.ViewGroup
import android.widget.RelativeLayout
import androidx.activity.enableEdgeToEdge
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.toArgb
import androidx.compose.ui.platform.ComposeView
import androidx.lifecycle.setViewTreeLifecycleOwner
import androidx.savedstate.setViewTreeSavedStateRegistryOwner
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import org.koin.androidx.viewmodel.ext.android.viewModel
import org.libsdl.app.SDLActivity
import org.vpinball.app.ui.VPinballContent
import org.vpinball.app.util.FileUtils
import org.vpinball.app.util.captureBitmap

class VPinballActivity : SDLActivity() {
    val viewModel: VPinballViewModel by viewModel<VPinballViewModel>()

    override fun onCreate(savedInstanceState: Bundle?) {
        enableEdgeToEdge()

        super.onCreate(savedInstanceState)

        runCatching { FileUtils.copyAssets(this.assets, "", this.filesDir) }

        initCompose()
    }

    private fun initCompose() {
        val layoutParams =
            RelativeLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT).apply {
                addRule(RelativeLayout.CENTER_IN_PARENT)
            }

        val composeView = ComposeView(this).apply { setContent { VPinballContent() } }

        val relativeLayout = getContentView() as? RelativeLayout
        relativeLayout?.apply {
            setBackgroundColor(Color.Transparent.toArgb())
            setViewTreeLifecycleOwner(this@VPinballActivity)
            setViewTreeSavedStateRegistryOwner(this@VPinballActivity)
            addView(composeView, layoutParams)
        }

        CoroutineScope(Dispatchers.IO).launch {
            delay(1000)
            VPinballManager.initialize(this@VPinballActivity)
        }
    }

    fun captureBitmap(callback: (Bitmap?) -> Unit) {
        mSurface.captureBitmap(callback)
    }

    override fun getLibraries(): Array<String> =
        arrayOf(
            "SDL3",
            "SDL3_image",
            "SDL3_ttf",
            "SDL3_mixer",
            "freeimage",
            "bass",
            "pinmame",
            "altsound",
            "zedmd",
            "serum",
            "dmdutil",
            "dof",
            "pupdmd",
            "sockpp",
            "vpinball",
        )
}
