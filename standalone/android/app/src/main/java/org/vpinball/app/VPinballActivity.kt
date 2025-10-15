package org.vpinball.app

import android.os.Bundle
import android.view.ViewGroup
import android.widget.RelativeLayout
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.toArgb
import androidx.compose.ui.platform.ComposeView
import androidx.lifecycle.setViewTreeLifecycleOwner
import androidx.savedstate.setViewTreeSavedStateRegistryOwner
import org.koin.androidx.viewmodel.ext.android.viewModel
import org.libsdl.app.SDLActivity
import org.vpinball.app.ui.VPinballContent

class VPinballActivity : SDLActivity() {
    val viewModel: VPinballViewModel by viewModel<VPinballViewModel>()

    private var isAppInit by mutableStateOf(false)
    private var composeView: ComposeView? = null

    companion object {
        const val COMMAND_APP_INIT_COMPLETED = 0x8001
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        window.decorView.windowInsetsController?.show(android.view.WindowInsets.Type.statusBars() or android.view.WindowInsets.Type.navigationBars())

        window.decorView.windowInsetsController?.setSystemBarsAppearance(0, android.view.WindowInsetsController.APPEARANCE_LIGHT_STATUS_BARS)

        VPinballManager.initialize(this)

        initCompose()
    }

    override fun onUnhandledMessage(command: Int, param: Any?): Boolean {
        if (command == COMMAND_APP_INIT_COMPLETED) {
            isAppInit = true
            return true
        }

        return super.onUnhandledMessage(command, param)
    }

    private fun initCompose() {
        val layoutParams = RelativeLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT)

        composeView = ComposeView(this).apply { setContent { VPinballContent(isAppInit = isAppInit) } }

        val relativeLayout = getContentView() as? RelativeLayout
        relativeLayout?.apply {
            setBackgroundColor(Color.Transparent.toArgb())
            setViewTreeLifecycleOwner(this@VPinballActivity)
            setViewTreeSavedStateRegistryOwner(this@VPinballActivity)
            addView(composeView, layoutParams)
        }
    }

    override fun getLibraries(): Array<String> =
        arrayOf(
            "SDL3",
            "SDL3_image",
            "SDL3_ttf",
            "freeimage",
            "pinmame",
            "altsound",
            "dmdutil",
            "dof",
            "pupdmd",
            "avcodec",
            "avdevice",
            "avfilter",
            "avformat",
            "avutil",
            "swresample",
            "swscale",
            "vpinball",
        )
}
