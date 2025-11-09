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
import org.vpinball.app.jni.VPinballCommandData
import org.vpinball.app.jni.VPinballEvent
import org.vpinball.app.jni.VPinballJNI
import org.vpinball.app.jni.VPinballLogLevel
import org.vpinball.app.jni.VPinballProgressData
import org.vpinball.app.jni.VPinballRumbleData
import org.vpinball.app.jni.VPinballScriptErrorData
import org.vpinball.app.jni.VPinballScriptErrorType
import org.vpinball.app.jni.VPinballSettingsSection
import org.vpinball.app.jni.VPinballSettingsSection.STANDALONE
import org.vpinball.app.jni.VPinballStatus
import org.vpinball.app.jni.VPinballWebServerData
import org.vpinball.app.ui.screens.landing.LandingScreenViewModel
import org.vpinball.app.util.FileUtils

object VPinballManager : KoinComponent {
    val vpinballJNI: VPinballJNI = VPinballJNI()

    internal lateinit var activity: VPinballActivity
    private lateinit var filesDir: File
    private lateinit var cacheDir: File
    private lateinit var displaySize: Size
    private lateinit var vibrator: Vibrator

    private var activeTable: Table? = null
    private var error: String? = null

    private var lastProgressEvent: VPinballEvent? = null
    private var lastProgress: Int? = null

    fun initialize(activity: VPinballActivity) {
        this.activity = activity

        filesDir = activity.filesDir
        cacheDir = activity.cacheDir

        val displayMetrics = activity.resources.displayMetrics
        val width = displayMetrics.widthPixels
        val height = displayMetrics.heightPixels
        displaySize = if (width > height) Size(height, width) else Size(width, height)

        vibrator =
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
                val vibratorManager = activity.getSystemService(Context.VIBRATOR_MANAGER_SERVICE) as VibratorManager
                vibratorManager.defaultVibrator
            } else {
                activity.getSystemService(Context.VIBRATOR_SERVICE) as Vibrator
            }

        SAFFileSystem.initialize(activity)
    }

    fun startup() {
        runCatching { FileUtils.copyAssets(activity.assets, "", activity.filesDir) }

        vpinballJNI.VPinballInit { value, jsonData ->
            val viewModel = activity.viewModel
            val event = VPinballEvent.entries.find { it.ordinal == value }
            when (event) {
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

                        progressData?.let {
                            CoroutineScope(Dispatchers.Main).launch {
                                viewModel.title(activeTable?.name ?: "")
                                viewModel.progress(progressData.progress)
                                viewModel.status(event.text)
                            }
                        }
                    }
                }
                VPinballEvent.PLAYER_STARTED -> {
                    lastProgressEvent = null
                    lastProgress = null
                    CoroutineScope(Dispatchers.Main).launch {
                        viewModel.playing(true)
                        delay(500)
                        viewModel.loading(false)
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
                VPinballEvent.SCRIPT_ERROR -> {
                    if (error == null) {
                        val scriptErrorData =
                            jsonData?.let { jsonStr ->
                                try {
                                    Json.decodeFromString<VPinballScriptErrorData>(jsonStr)
                                } catch (e: Exception) {
                                    log(VPinballLogLevel.WARN, "Failed to parse script error data JSON: $jsonStr - ${e.message}")
                                    null
                                }
                            }
                        error =
                            scriptErrorData?.let {
                                val errorType = VPinballScriptErrorType.fromInt(it.error)
                                "${errorType.text} on line ${it.line}, position ${it.position}:\n\n${it.description}"
                            } ?: "Script error."
                    }
                }
                VPinballEvent.PLAYER_CLOSED -> {
                    val tableToCleanup = activeTable
                    activeTable = null
                    CoroutineScope(Dispatchers.Main).launch {
                        viewModel.playing(false)

                        error?.let { error ->
                            delay(500)
                            showError(error)
                        }

                        viewModel.stopped()
                        delay(100)

                        tableToCleanup?.let { table ->
                            if (SAFFileSystem.isUsingSAF()) {
                                viewModel.loading(true, null)
                                viewModel.title(table.name)
                                viewModel.progress(0)
                                viewModel.status("Saving changes...")
                                delay(50)

                                withContext(Dispatchers.IO) {
                                    TableManager.getInstance().cleanupLoadedTable(table) { progress, status ->
                                        CoroutineScope(Dispatchers.Main).launch {
                                            viewModel.progress(progress)
                                            viewModel.status(status)
                                        }
                                    }
                                }

                                viewModel.loading(false)
                            } else {
                                withContext(Dispatchers.IO) { TableManager.getInstance().cleanupLoadedTable(table) }
                            }

                            withContext(Dispatchers.IO) {
                                TableManager.getInstance().reloadTableImage(table)?.let { updatedTable ->
                                    LandingScreenViewModel.triggerUpdateTable(updatedTable)
                                }
                            }
                        }
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
                    CoroutineScope(Dispatchers.Main).launch { viewModel.webServerURL = webServerData?.url ?: "" }
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

    fun getDisplaySize(): Size {
        return displaySize
    }

    fun getFilesDir(): File {
        return filesDir
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

    fun play(table: Table) {
        if (activeTable != null) return
        activeTable = table
        error = null
        CoroutineScope(Dispatchers.IO).launch {
            if (loadValue(STANDALONE, "ResetLogOnPlay", true)) {
                vpinballJNI.VPinballResetLog()
            }

            val viewModel = activity.viewModel
            if (SAFFileSystem.isUsingSAF()) {
                withContext(Dispatchers.Main) {
                    viewModel.loading(true, table)
                    viewModel.title(table.name)
                    viewModel.progress(0)
                    viewModel.status("")
                }
            }

            val tablePath =
                TableManager.getInstance().stageTable(table) { progress, status ->
                    CoroutineScope(Dispatchers.Main).launch {
                        viewModel.progress(progress)
                        viewModel.status(status)
                    }
                }
            if (tablePath == null) {
                log(VPinballLogLevel.ERROR, "Unable to stage table: ${table.uuid}")
                delay(500)
                withContext(Dispatchers.Main) {
                    viewModel.stopped()
                    showError("Unable to stage table.")
                    activeTable = null
                }
                return@launch
            }

            withContext(Dispatchers.Main) { viewModel.loading(true, table) }
            if (vpinballJNI.VPinballLoadTable(tablePath) == VPinballStatus.SUCCESS.value) {
                vpinballJNI.VPinballPlay()
            } else {
                delay(500)
                withContext(Dispatchers.Main) {
                    viewModel.stopped()
                    showError("Unable to load table.")
                    activeTable = null
                }
            }
        }
    }

    fun stop() {
        vpinballJNI.VPinballStop()
    }

    fun showError(message: String) {
        CoroutineScope(Dispatchers.Main).launch {
            delay(250)
            activity.viewModel.setError(message)
        }
    }

    fun getTablesPath(): String {
        val customPath = loadValue(STANDALONE, "TablesPath", "")
        return customPath.ifEmpty { File(activity.filesDir, "tables").absolutePath }
    }
}
