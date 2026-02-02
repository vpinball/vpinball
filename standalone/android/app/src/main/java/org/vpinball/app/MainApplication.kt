package org.vpinball.app

import android.app.Application
import org.koin.android.ext.koin.androidContext
import org.koin.core.context.startKoin
import org.libsdl.app.SDL
import org.vpinball.app.di.appModule

class MainApplication : Application() {
    override fun onCreate() {
        super.onCreate()

        loadNativeLibraries()

        startKoin {
            androidContext(this@MainApplication)
            modules(appModule)
        }

        SDL.setupJNI()
        SDL.initialize()

        VPinballManager.initialize(this)
    }

    private fun loadNativeLibraries() {
        System.loadLibrary("SDL3")
        System.loadLibrary("SDL3_image")
        System.loadLibrary("SDL3_ttf")
        System.loadLibrary("freeimage")
        System.loadLibrary("pinmame")
        System.loadLibrary("altsound")
        System.loadLibrary("dmdutil")
        System.loadLibrary("dof")
        System.loadLibrary("pupdmd")
        System.loadLibrary("vni")
        System.loadLibrary("avcodec")
        System.loadLibrary("avdevice")
        System.loadLibrary("avfilter")
        System.loadLibrary("avformat")
        System.loadLibrary("avutil")
        System.loadLibrary("swresample")
        System.loadLibrary("swscale")
        System.loadLibrary("vpinball")
    }
}
