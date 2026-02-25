package org.vpinball.app

import android.content.Context
import android.os.Build
import android.os.VibrationEffect
import android.os.Vibrator
import android.os.VibratorManager
import android.util.Size
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
import org.vpinball.app.jni.VPinballRumbleData
import org.vpinball.app.jni.VPinballSettingsSection
import org.vpinball.app.jni.VPinballSettingsSection.STANDALONE
import org.vpinball.app.jni.VPinballStatus
import org.vpinball.app.jni.VPinballWebServerData
import org.vpinball.app.ui.screens.landing.LandingScreenViewModel
import org.vpinball.app.util.FileUtils

object VPinballManager : KoinComponent {
    val vpinballJNI: VPinballJNI = VPinballJNI()

    private lateinit var context: Context
    private lateinit var cacheDir: File
    private lateinit var displaySize: Size
    private lateinit var vibrator: Vibrator

    private var playerActivity: VPinballPlayerActivity? = null
    private var mainActivity: VPinballActivity? = null

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

        vibrator =
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
                val vibratorManager = context.getSystemService(Context.VIBRATOR_MANAGER_SERVICE) as VibratorManager
                vibratorManager.defaultVibrator
            } else {
                context.getSystemService(Context.VIBRATOR_SERVICE) as Vibrator
            }

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
            val event = VPinballEvent.entries.find { it.ordinal == value } ?: return@VPinballInit

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
                VPinballEvent.LOADING_ITEMS,
                VPinballEvent.LOADING_SOUNDS,
                VPinballEvent.LOADING_IMAGES,
                VPinballEvent.LOADING_FONTS,
                VPinballEvent.LOADING_COLLECTIONS,
                VPinballEvent.PRERENDERING -> {
                    val progressData =
                        jsonData?.let { jsonStr ->
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
                VPinballEvent.PLAYER_STARTED -> {
                    lastProgressEvent = null
                    lastProgress = null
                    CoroutineScope(Dispatchers.Main).launch {
                        viewModel.isPlaying = true
                        delay(500)
                        viewModel.hideHUD()
                    }
                }
                VPinballEvent.RUMBLE -> {
                    val rumbleData =
                        jsonData?.let { jsonStr ->
                            try {
                                Json.decodeFromString<VPinballRumbleData>(jsonStr)
                            } catch (e: Exception) {
                                log(VPinballLogLevel.WARN, "Failed to parse rumble data JSON: $jsonStr - ${e.message}")
                                null
                            }
                        }
                    rumbleData?.let { rumble(it) }
                }
                VPinballEvent.PLAYER_CLOSED -> {
                    val tableToCleanup = viewModel.activeTable
                    viewModel.activeTable = null
                    CoroutineScope(Dispatchers.Main).launch {
                        viewModel.isPlaying = false
                        viewModel.hideHUD()
                        delay(100)

                        tableToCleanup?.let { table ->
                            if (SAFFileSystem.isUsingSAF()) {
                                viewModel.showHUD(table.name, "Saving changes...")
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
                    val webServerData =
                        jsonData?.let { jsonStr ->
                            try {
                                Json.decodeFromString<VPinballWebServerData>(jsonStr)
                            } catch (_: Exception) {
                                null
                            }
                        }
                    CoroutineScope(Dispatchers.Main).launch { viewModel.webServerURL = webServerData?.url }
                }
                VPinballEvent.COMMAND -> {
                    val commandData =
                        jsonData?.let { jsonStr ->
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
                else -> {
                    log(VPinballLogLevel.WARN, "event=${event}")
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

    fun isInitialized(): Boolean = synchronized(initLock) { initState == InitState.INITIALIZED }

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

    private fun rumble(data: VPinballRumbleData) {
        if (data.lowFrequencyRumble > 0 || data.highFrequencyRumble > 0) {
            val amplitude =
                when {
                    data.lowFrequencyRumble == data.highFrequencyRumble -> VibrationEffect.DEFAULT_AMPLITUDE
                    data.lowFrequencyRumble > 20000 || data.highFrequencyRumble > 20000 -> 255
                    data.lowFrequencyRumble > 10000 || data.highFrequencyRumble > 10000 -> 200
                    data.lowFrequencyRumble > 1000 || data.highFrequencyRumble > 1000 -> 100
                    else -> 50
                }
            vibrator.vibrate(VibrationEffect.createOneShot(15, amplitude))
        }
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

    suspend fun load(table: Table, onProgress: ((Int, String) -> Unit)? = null): Boolean {
        val viewModel = mainActivity?.viewModel ?: return false

        return withContext(Dispatchers.IO) {
            if (loadValue(STANDALONE, "ResetLogOnPlay", true)) {
                vpinballJNI.VPinballResetLog()
            }

            val tablePath = TableManager.getInstance().stageTable(table) { progress, status -> onProgress?.invoke(progress, status) }

            if (tablePath == null) {
                log(VPinballLogLevel.ERROR, "Unable to stage table: ${table.uuid}")
                delay(500)
                withContext(Dispatchers.Main) {
                    viewModel.activeTable = null
                    viewModel.hideHUD()
                    LandingScreenViewModel.triggerError("Unable to stage table.")
                }
                return@withContext false
            }

            if (vpinballJNI.VPinballLoadTable(tablePath) == VPinballStatus.SUCCESS.value) {
                true
            } else {
                log(VPinballLogLevel.ERROR, "Unable to load table: ${table.uuid}")
                delay(500)
                withContext(Dispatchers.Main) {
                    viewModel.activeTable = null
                    viewModel.hideHUD()
                    LandingScreenViewModel.triggerError("Unable to load table.")
                }
                false
            }
        }
    }

    fun play() {
        if (mainActivity?.viewModel?.activeTable == null) {
            log(VPinballLogLevel.ERROR, "No table loaded for playback")
            return
        }

        vpinballJNI.VPinballPlay()
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
