package org.vpinball.app

import android.content.Context
import android.content.Intent
import android.util.Size
import androidx.lifecycle.lifecycleScope
import java.io.File
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import kotlinx.serialization.json.Json
import org.koin.core.component.KoinComponent
import org.libsdl.app.SDL
import org.vpinball.app.jni.VPinballCommandData
import org.vpinball.app.jni.VPinballEvent
import org.vpinball.app.jni.VPinballJNI
import org.vpinball.app.jni.VPinballLogLevel
import org.vpinball.app.jni.VPinballPath
import org.vpinball.app.jni.VPinballProgressData
import org.vpinball.app.jni.VPinballSettingsSection
import org.vpinball.app.jni.VPinballWebServerData
import org.vpinball.app.ui.screens.landing.LandingScreenViewModel
import org.vpinball.app.util.FileUtils

object VPinballManager : KoinComponent {
    val vpinballJNI: VPinballJNI = VPinballJNI()

    private lateinit var context: Context
    private lateinit var cacheDir: File
    private lateinit var displaySize: Size

    private var playerActivity: VPinballPlayerActivity? = null
    private var mainActivity: VPinballActivity? = null
    private var pendingTablePath: String? = null

    val model: VPinballModel?
        get() = mainActivity?.viewModel

    private var lastProgressEvent: VPinballEvent? = null
    private var lastProgress: Int? = null

    enum class InitState {
        NOT_INITIALIZED,
        SDL_READY,
        INITIALIZED,
    }

    private var initState = InitState.NOT_INITIALIZED
    private val initLock = Object()
    private val pendingCallbacks = mutableListOf<() -> Unit>()

    fun initialize(context: Context) {
        this.context = context.applicationContext
        cacheDir = context.cacheDir

        val displayMetrics = context.resources.displayMetrics
        val width = displayMetrics.widthPixels
        val height = displayMetrics.heightPixels
        displaySize = if (width > height) Size(height, width) else Size(width, height)

        SAFFileSystem.initialize(context)
    }

    fun onActivityReady(activity: VPinballActivity) {
        synchronized(initLock) {
            mainActivity = activity
            SDL.setContext(activity)

            if (initState != InitState.NOT_INITIALIZED) {
                return
            }

            initState = InitState.SDL_READY
            performInit()
        }
    }

    private fun performInit() {
        vpinballJNI.VPinballInit { value, jsonData ->
            val activity = mainActivity ?: return@VPinballInit
            val viewModel = activity.viewModel
            val event = VPinballEvent.entries.find { it.value == value } ?: return@VPinballInit

            when (event) {
                VPinballEvent.INIT_COMPLETE -> {
                    CoroutineScope(Dispatchers.IO).launch {
                        runCatching { FileUtils.copyAssets(context.assets, "", File(vpinballJNI.VPinballGetPath(VPinballPath.ROOT.value))) }

                        synchronized(initLock) {
                            initState = InitState.INITIALIZED
                            pendingCallbacks.forEach { callback -> CoroutineScope(Dispatchers.Main).launch { callback() } }
                            pendingCallbacks.clear()
                        }
                    }
                }
                VPinballEvent.EXTRACT_SCRIPT,
                VPinballEvent.LOADING -> {
                    val progressData = jsonData?.let { jsonStr ->
                        try {
                            Json.decodeFromString<VPinballProgressData>(jsonStr)
                        } catch (e: Exception) {
                            log(VPinballLogLevel.WARN, "Failed to parse progress data JSON: $jsonStr - ${e.message}")
                            null
                        }
                    }

                    val shouldUpdate = lastProgressEvent != event || lastProgress != progressData?.progress
                    if (shouldUpdate) {
                        log(VPinballLogLevel.INFO, "event=${event.name}, data=${progressData}")
                        lastProgressEvent = event
                        lastProgress = progressData?.progress

                        progressData?.let { CoroutineScope(Dispatchers.Main).launch { viewModel.updateHUD(progressData.progress, event.text ?: "") } }
                    }
                }
                VPinballEvent.PLAYER_READY -> {
                    lastProgressEvent = null
                    lastProgress = null
                    CoroutineScope(Dispatchers.Main).launch {
                        viewModel.hideHUD()
                        playerActivity?.hideLoadingOverlay()
                    }
                }
                VPinballEvent.PLAYER_FAILED -> {
                    CoroutineScope(Dispatchers.Main).launch { onPlayerFailed("Unable to load table.") }
                }
                VPinballEvent.PLAYER_CLOSED -> {
                    val tableToCleanup = viewModel.activeTable
                    viewModel.activeTable = null
                    CoroutineScope(Dispatchers.Main).launch {
                        viewModel.hideHUD()
                        delay(100)

                        tableToCleanup?.let { table ->
                            if (SAFFileSystem.isUsingSAF()) {
                                viewModel.showHUD("Saving changes...")
                                delay(50)

                                withContext(Dispatchers.IO) {
                                    TableManager.getInstance().cleanupLoadedTable(table) { progress, status ->
                                        CoroutineScope(Dispatchers.Main).launch { viewModel.updateHUD(progress, status) }
                                    }
                                }

                                viewModel.hideHUD()
                            } else {
                                withContext(Dispatchers.IO) { TableManager.getInstance().cleanupLoadedTable(table) }
                            }

                            withContext(Dispatchers.IO) {
                                TableManager.getInstance().reloadTableImage(table)?.let { updatedTable ->
                                    LandingScreenViewModel.triggerUpdateTable(updatedTable)
                                }
                            }
                        }

                        delay(2000)
                        playerActivity?.finish()
                    }
                }
                VPinballEvent.WEB_SERVER -> {
                    val webServerData = jsonData?.let { jsonStr ->
                        try {
                            Json.decodeFromString<VPinballWebServerData>(jsonStr)
                        } catch (_: Exception) {
                            null
                        }
                    }
                    CoroutineScope(Dispatchers.Main).launch { viewModel.webServerURL = webServerData?.url }
                }
                VPinballEvent.COMMAND -> {
                    val commandData = jsonData?.let { jsonStr ->
                        try {
                            Json.decodeFromString<VPinballCommandData>(jsonStr)
                        } catch (e: Exception) {
                            log(VPinballLogLevel.WARN, "Failed to parse command data JSON: $jsonStr - ${e.message}")
                            null
                        }
                    }
                    commandData?.let {
                        if (it.command == "reloadTables") {
                            CoroutineScope(Dispatchers.IO).launch {
                                TableManager.getInstance().refresh()
                                LandingScreenViewModel.triggerRefresh()
                            }
                        }
                    }
                }
            }
        }
    }

    fun whenReady(callback: () -> Unit) {
        synchronized(initLock) {
            if (initState == InitState.INITIALIZED) {
                callback()
            } else {
                pendingCallbacks.add(callback)
            }
        }
    }

    fun setPlayerActivity(activity: VPinballPlayerActivity?) {
        playerActivity = activity
    }

    fun setMainActivity(activity: VPinballActivity?) {
        synchronized(initLock) {
            mainActivity = activity
            if (activity != null) {
                SDL.setContext(activity)
            }
        }
    }

    fun getDisplaySize(): Size {
        return displaySize
    }

    fun getCacheDir(): File {
        return cacheDir
    }

    fun log(level: VPinballLogLevel, message: String) {
        vpinballJNI.VPinballLog(level.value, message)
    }

    fun updateWebServer() {
        vpinballJNI.VPinballUpdateWebServer()
    }

    fun getVersionString(): String = vpinballJNI.VPinballGetVersionStringFull()

    fun loadValue(section: VPinballSettingsSection, key: String, defaultValue: Int): Int =
        vpinballJNI.VPinballLoadValueInt(section.value, key, defaultValue)

    fun loadValue(section: VPinballSettingsSection, key: String, defaultValue: Float): Float =
        vpinballJNI.VPinballLoadValueFloat(section.value, key, defaultValue)

    fun loadValue(section: VPinballSettingsSection, key: String, defaultValue: Boolean): Boolean =
        vpinballJNI.VPinballLoadValueBool(section.value, key, defaultValue)

    fun loadValue(section: VPinballSettingsSection, key: String, defaultValue: String): String =
        vpinballJNI.VPinballLoadValueString(section.value, key, defaultValue)

    fun saveValue(section: VPinballSettingsSection, key: String, value: Int) {
        vpinballJNI.VPinballSaveValueInt(section.value, key, value)
    }

    fun saveValue(section: VPinballSettingsSection, key: String, value: Float) {
        vpinballJNI.VPinballSaveValueFloat(section.value, key, value)
    }

    fun saveValue(section: VPinballSettingsSection, key: String, value: Boolean) {
        vpinballJNI.VPinballSaveValueBool(section.value, key, value)
    }

    fun saveValue(section: VPinballSettingsSection, key: String, value: String) {
        vpinballJNI.VPinballSaveValueString(section.value, key, value)
    }

    fun resetIni() {
        vpinballJNI.VPinballResetIni()
    }

    fun play(table: Table) {
        val activity = mainActivity ?: return
        val viewModel = activity.viewModel
        if (viewModel.activeTable != null) return

        viewModel.activeTable = table
        viewModel.showHUD("Launching")

        activity.lifecycleScope.launch {
            val tablePath =
                TableManager.getInstance().stageTable(table) { progress, status ->
                    launch(Dispatchers.Main) { viewModel.updateHUD(progress, status) }
                }
            if (tablePath == null) {
                log(VPinballLogLevel.ERROR, "Unable to stage table: ${table.uuid}")
                delay(500)
                onPlayerFailed("Unable to stage table.")
                return@launch
            }

            pendingTablePath = tablePath
            activity.startActivity(Intent(activity, VPinballPlayerActivity::class.java))
            @Suppress("DEPRECATION") activity.overridePendingTransition(android.R.anim.fade_in, android.R.anim.fade_out)
        }
    }

    fun startPlayer() {
        val tablePath = pendingTablePath
        if (model?.activeTable == null || tablePath == null) {
            log(VPinballLogLevel.ERROR, "No table staged for playback")
            return
        }

        pendingTablePath = null
        vpinballJNI.VPinballPlay(tablePath)
    }

    private fun onPlayerFailed(message: String) {
        model?.activeTable = null
        model?.hideHUD()
        playerActivity?.finish()
        LandingScreenViewModel.triggerError(message)
    }

    fun stop() {
        vpinballJNI.VPinballStop()
    }

    fun showError(message: String) {
        CoroutineScope(Dispatchers.Main).launch {
            delay(250)
            LandingScreenViewModel.triggerError(message)
        }
    }

    fun getPath(pathType: VPinballPath): String = vpinballJNI.VPinballGetPath(pathType.value)
}
