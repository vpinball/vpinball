package org.vpinball.app

import android.os.Bundle
import org.libsdl.app.SDLActivity

/**
 * A sample wrapper class that just calls SDLActivity
 */
class VPinballActivity : SDLActivity() {
    val arguments = mutableListOf<String>()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        intent.extras?.let {
            arguments.add("-fd")
            it.getInt("dirFd").let { arguments.add(it.toString()) }
            arguments.add("-play")
            it.getString("table")?.let { arguments.add(it) }
        }
    }

    override fun getArguments(): Array<String> {
        return arguments.toTypedArray()
    }

    override fun getLibraries(): Array<String> {
        return arrayOf(
            "SDL2",
            "SDL2_image",
            "SDL2_ttf",
            "FreeImage",
            "bass",
            "pinmame.3.6",
            "altsound",
            "zedmd",
            "serum",
            "dmdutil",
            "pupdmd",
            "dof",
            "sockpp",
            "vpinball"
        )
    }

}
