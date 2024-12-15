package org.vpinball.app

import android.content.Context
import android.graphics.Bitmap
import android.net.Uri
import android.os.Build
import android.os.VibrationEffect
import android.os.Vibrator
import android.os.VibratorManager
import android.util.Log
import android.util.Size
import androidx.lifecycle.viewmodel.compose.viewModel
import java.io.File
import java.util.UUID
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking
import kotlinx.coroutines.withContext
import org.vpinball.app.data.entity.PinTable
import org.vpinball.app.jni.VPinballCustomTableOption
import org.vpinball.app.jni.VPinballEvent
import org.vpinball.app.jni.VPinballGfxBackend
import org.vpinball.app.jni.VPinballJNI
import org.vpinball.app.jni.VPinballLogLevel
import org.vpinball.app.jni.VPinballProgressData
import org.vpinball.app.jni.VPinballRumbleData
import org.vpinball.app.jni.VPinballScriptErrorData
import org.vpinball.app.jni.VPinballSettingsSection
import org.vpinball.app.jni.VPinballSettingsSection.PLAYER
import org.vpinball.app.jni.VPinballSettingsSection.STANDALONE
import org.vpinball.app.jni.VPinballStatus
import org.vpinball.app.jni.VPinballTableOptions
import org.vpinball.app.jni.VPinballViewSetup
import org.vpinball.app.jni.VPinballWebServerData
import org.vpinball.app.util.FileUtils
import org.vpinball.app.util.basePath
import org.vpinball.app.util.hasArtwork
import org.vpinball.app.util.saveArtwork
import org.vpinball.app.util.tableFile

object VPinballManager {
    private const val TAG = "VPinballManager"

    private var vpinballJNI: VPinballJNI = VPinballJNI()

    private lateinit var activity: VPinballActivity
    private lateinit var filesDir: File
    private lateinit var cacheDir: File
    private lateinit var displaySize: Size
    private lateinit var vibrator: Vibrator

    private var activeTable: PinTable? = null
    private var haptics = false

    private var error: String? = null

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

        vpinballJNI.VPinballInit { value, data ->
            val viewModel = activity.viewModel
            val event = VPinballEvent.entries.find { it.ordinal == value }
            when (event) {
                VPinballEvent.ARCHIVE_UNCOMPRESSING,
                VPinballEvent.ARCHIVE_COMPRESSING,
                VPinballEvent.LOADING_ITEMS,
                VPinballEvent.LOADING_SOUNDS,
                VPinballEvent.LOADING_IMAGES,
                VPinballEvent.LOADING_FONTS,
                VPinballEvent.LOADING_COLLECTIONS,
                VPinballEvent.PRERENDERING -> {
                    val progressData = data as? VPinballProgressData
                    Log.v(TAG, "event=${event.name}, data=${progressData}")
                    progressData?.let {
                        CoroutineScope(Dispatchers.Main).launch {
                            viewModel.title(activeTable?.name ?: "")
                            viewModel.progress(progressData.progress)
                            viewModel.status(event.text)
                        }
                    }
                }
                VPinballEvent.PLAYER_STARTED -> {
                    log(VPinballLogLevel.INFO, "event=${event.name}")
                    CoroutineScope(Dispatchers.Main).launch {
                        viewModel.playing(true)
                        delay(500)
                        viewModel.loading(false)
                        if (loadValue(STANDALONE, "TouchInstructions", true)) {
                            viewModel.touchInstructions(true)
                        }
                        if (loadValue(STANDALONE, "TouchOverlay", false)) {
                            viewModel.touchOverlay(true)
                        }
                    }
                }
                VPinballEvent.RUMBLE -> {
                    if (haptics) {
                        val rumbleData = data as? VPinballRumbleData
                        rumbleData?.let { rumble(it) }
                    }
                }
                VPinballEvent.SCRIPT_ERROR -> {
                    if (error == null) {
                        val scriptErrorData = data as? VPinballScriptErrorData
                        error =
                            scriptErrorData?.let { "${it.error.text} on line ${it.line}, position ${it.position}:\n\n${it.description}" }
                                ?: "Script error."
                    }
                }
                VPinballEvent.LIVE_UI_TOGGLE -> {
                    log(VPinballLogLevel.INFO, "event=${event.name}")
                    CoroutineScope(Dispatchers.Main).launch {
                        viewModel.toggleLiveUI()
                        setPlayState(!viewModel.isLiveUI())
                    }
                }
                VPinballEvent.LIVE_UI_UPDATE -> {}
                VPinballEvent.PLAYER_CLOSING -> {
                    log(VPinballLogLevel.INFO, "event=${event.name}")
                    if (error == null) {
                        runBlocking { delay(500) }
                        captureAndSaveBitmap()
                    }
                }
                VPinballEvent.PLAYER_CLOSED -> {
                    log(VPinballLogLevel.INFO, "event=${event.name}")
                }
                VPinballEvent.STOPPED -> {
                    log(VPinballLogLevel.INFO, "event=${event.name}")
                    activeTable = null
                    CoroutineScope(Dispatchers.Main).launch {
                        viewModel.stopped()
                        error?.let { error ->
                            delay(500)
                            showError(error)
                        }
                    }
                }
                VPinballEvent.WEB_SERVER -> {
                    val webServerData = data as? VPinballWebServerData
                    log(VPinballLogLevel.INFO, "event=${event.name}, data=${webServerData}")
                    webServerData?.let { CoroutineScope(Dispatchers.Main).launch { viewModel.webServerURL = webServerData.url } }
                }
                else -> {
                    log(VPinballLogLevel.WARN, "event=${event}")
                }
            }
            null
        }

        CoroutineScope(Dispatchers.Main).launch {
            delay(500)
            setIniDefaults()
            updateWebServer()
        }
    }

    fun getDisplaySize(): Size {
        return displaySize
    }

    fun getFilesDir(): File {
        return filesDir
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

    fun captureBitmap(callback: (Bitmap?) -> Unit) {
        activity.captureBitmap { bitmap -> callback(bitmap) }
    }

    private fun captureAndSaveBitmap() {
        activeTable?.let { table ->
            if (table.hasArtwork()) {
                return
            }
            captureBitmap { bitmap ->
                if (bitmap != null) {
                    saveBitmap(bitmap)
                }
            }
        }
    }

    fun saveBitmap(bitmap: Bitmap) {
        activeTable?.saveArtwork(bitmap)
    }

    private fun setIniDefaults() {
        saveValue(STANDALONE, "RenderingModeOverride", loadValue(STANDALONE, "RenderingModeOverride", 2))

        saveValue(PLAYER, "GfxBackend", loadValue(PLAYER, "GfxBackend", VPinballGfxBackend.OPENGLES.value))
        saveValue(PLAYER, "MaxTexDimension", loadValue(PLAYER, "MaxTexDimension", 1024))
        saveValue(PLAYER, "ScreenWidth", loadValue(PLAYER, "ScreenWidth", 15.4f))
        saveValue(PLAYER, "ScreenHeight", loadValue(PLAYER, "ScreenHeight", 7.1f))
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
        loadValue(section, key, if (defaultValue) 1 else 0) == 1

    fun loadValue(section: VPinballSettingsSection, key: String, defaultValue: String): String =
        vpinballJNI.VPinballLoadValueString(section.value, key, defaultValue)

    fun saveValue(section: VPinballSettingsSection, key: String, value: Int) {
        vpinballJNI.VPinballSaveValueInt(section.value, key, value)
    }

    fun saveValue(section: VPinballSettingsSection, key: String, value: Float) {
        vpinballJNI.VPinballSaveValueFloat(section.value, key, value)
    }

    fun saveValue(section: VPinballSettingsSection, key: String, value: Boolean) {
        saveValue(section, key, if (value) 1 else 0)
    }

    fun saveValue(section: VPinballSettingsSection, key: String, value: String) {
        vpinballJNI.VPinballSaveValueString(section.value, key, value)
    }

    fun resetIni() {
        vpinballJNI.VPinballResetIni()

        setIniDefaults()
    }

    fun toggleFPS() {
        vpinballJNI.VPinballToggleFPS()
    }

    fun setPlayState(enable: Boolean) {
        vpinballJNI.VPinballSetPlayState(if (enable) 1 else 0)
    }

    fun getCustomTableOptions(): List<VPinballCustomTableOption> {
        val count = vpinballJNI.VPinballGetCustomTableOptionsCount()
        val options = mutableListOf<VPinballCustomTableOption>()
        for (i in 0 until count) {
            val option = vpinballJNI.VPinballGetCustomTableOption(i)
            if (option != null) {
                options.add(option)
            }
        }
        return options
    }

    fun setCustomTableOption(customTableOption: VPinballCustomTableOption) {
        vpinballJNI.VPinballSetCustomTableOption(customTableOption)
    }

    fun resetCustomTableOptions() {
        vpinballJNI.VPinballResetCustomTableOptions()
    }

    fun saveCustomTableOptions() {
        vpinballJNI.VPinballSaveCustomTableOptions()
    }

    fun getTableOptions(): VPinballTableOptions {
        return vpinballJNI.VPinballGetTableOptions()
    }

    fun setTableOptions(tableOptions: VPinballTableOptions) {
        vpinballJNI.VPinballSetTableOptions(tableOptions)
    }

    fun resetTableOptions() {
        vpinballJNI.VPinballResetTableOptions()
    }

    fun saveTableOptions() {
        vpinballJNI.VPinballSaveTableOptions()
    }

    fun getViewSetup(): VPinballViewSetup {
        return vpinballJNI.VPinballGetViewSetup()
    }

    fun setViewSetup(viewSetup: VPinballViewSetup) {
        vpinballJNI.VPinballSetViewSetup(viewSetup)
    }

    fun setDefaultViewSetup() {
        vpinballJNI.VPinballSetDefaultViewSetup()
    }

    fun resetViewSetup() {
        vpinballJNI.VPinballResetViewSetup()
    }

    fun saveViewSetup() {
        vpinballJNI.VPinballSaveViewSetup()
    }

    //
    // Import process:
    //
    // Delete contents of cacheDir
    // Determine filename from uri
    // Copy uri to cacheDir
    //
    // If zip or vpxz, uncompress, and delete zip or vpxz
    // If vpx, do nothing
    //
    // Search for first vpx file in cacheDir
    // If found, generate uuid folder in filesDir,
    // and recursively copy from vpx file folder into uuid folder
    //

    fun importUri(
        context: Context,
        uri: Uri,
        onUpdate: (Int, String) -> Unit,
        onComplete: (uuid: String, path: String) -> Unit,
        onError: () -> Unit,
    ) {
        CoroutineScope(Dispatchers.IO).launch {
            try {
                cacheDir.deleteRecursively()
                cacheDir.mkdir()
                val filename = FileUtils.filenameFromUri(context, uri)
                if (filename == null) {
                    log(VPinballLogLevel.ERROR, "Unable to get filename: uri=$uri")
                    withContext(Dispatchers.Main) {
                        onError()
                        showErrorAndReset("Unable to import table.")
                    }
                    return@launch
                }
                withContext(Dispatchers.Main) { onUpdate(0, "Staging") }
                val outputFile = File(cacheDir, filename)
                FileUtils.copyFile(context, uri, outputFile) { progress -> launch(Dispatchers.Main) { onUpdate(progress, "Copying") } }
                if (!outputFile.extension.equals("vpx", ignoreCase = true)) {
                    if (vpinballJNI.VPinballUncompress(outputFile.absolutePath) != VPinballStatus.SUCCESS.value) {
                        log(VPinballLogLevel.ERROR, "Failed to uncompress file")
                        withContext(Dispatchers.Main) {
                            onError()
                            showErrorAndReset("Unable to import table.")
                        }
                        return@launch
                    }
                    outputFile.delete()
                }
                val vpxFile = FileUtils.findFileByExtension(cacheDir, "vpx")
                if (vpxFile == null) {
                    log(VPinballLogLevel.ERROR, "Unable to find vpx file")
                    withContext(Dispatchers.Main) {
                        onError()
                        showErrorAndReset("Unable to import table.")
                    }
                    return@launch
                }
                val uuid = UUID.randomUUID().toString()
                val uuidFolder = File(filesDir, uuid)
                if (!uuidFolder.mkdir()) {
                    log(VPinballLogLevel.ERROR, "Failed to create UUID folder")
                    withContext(Dispatchers.Main) {
                        onError()
                        showErrorAndReset("Unable to import table.")
                    }
                    return@launch
                }
                FileUtils.copyDirectoryContents(vpxFile.parentFile!!, uuidFolder)
                val newVpxFile = FileUtils.findFileByExtension(uuidFolder, "vpx")
                if (newVpxFile == null) {
                    log(VPinballLogLevel.ERROR, "Unable to find vpx file in UUID folder")
                    withContext(Dispatchers.Main) {
                        onError()
                        showErrorAndReset("Unable to import table.")
                    }
                    return@launch
                }
                withContext(Dispatchers.Main) { onComplete(uuid, newVpxFile.name) }
            } catch (e: Exception) {
                log(VPinballLogLevel.ERROR, "An error occurred: ${e.message}")
                withContext(Dispatchers.Main) {
                    onError()
                    showErrorAndReset("Unable to import script.")
                }
            }
        }
    }

    fun extractScript(table: PinTable, onComplete: () -> Unit, onError: () -> Unit) {
        if (activeTable != null) return
        activeTable = table
        CoroutineScope(Dispatchers.IO).launch {
            if (!table.tableFile.exists()) {
                withContext(Dispatchers.Main) {
                    onError()
                    showErrorAndReset("Unable to extract script.")
                }
                return@launch
            }
            val status = vpinballJNI.VPinballExtractScript(table.tableFile.absolutePath)
            withContext(Dispatchers.Main) {
                if (status == VPinballStatus.SUCCESS.value) {
                    onComplete()
                } else {
                    onError()
                    showErrorAndReset("Unable to extract script.")
                }
                activeTable = null
            }
        }
    }

    fun share(table: PinTable, onComplete: (file: File) -> Unit, onError: () -> Unit) {
        if (activeTable != null) return
        activeTable = table
        CoroutineScope(Dispatchers.IO).launch {
            if (!table.tableFile.exists()) {
                withContext(Dispatchers.Main) {
                    onError()
                    showErrorAndReset("Unable to share table.")
                }
                return@launch
            }
            try {
                cacheDir.deleteRecursively()
                cacheDir.mkdir()
                val name = table.name.replace(Regex("[ ]"), "_")
                val shareFile = File(cacheDir, "${name}.vpxz")
                val status = vpinballJNI.VPinballCompress(table.basePath.absolutePath, shareFile.absolutePath)
                withContext(Dispatchers.Main) {
                    if (status == VPinballStatus.SUCCESS.value) {
                        onComplete(shareFile)
                        activeTable = null
                    } else {
                        onError()
                        showErrorAndReset("Unable to share table.")
                    }
                }
            } catch (e: Exception) {
                log(VPinballLogLevel.ERROR, "An error occurred: ${e.message}")
                showErrorAndReset("Unable to share table.")
            }
        }
    }

    fun play(table: PinTable) {
        if (activeTable != null) return
        activeTable = table
        error = null
        CoroutineScope(Dispatchers.IO).launch {
            if (!table.tableFile.exists()) {
                showErrorAndReset("Unable to load table.")
                return@launch
            }
            if (loadValue(STANDALONE, "ResetLogOnPlay", true)) {
                vpinballJNI.VPinballResetLog()
            }
            haptics = loadValue(STANDALONE, "Haptics", true)
            withContext(Dispatchers.Main) { activity.viewModel.loading(true, table) }
            if (vpinballJNI.VPinballLoad(table.tableFile.absolutePath) == VPinballStatus.SUCCESS.value) {
                vpinballJNI.VPinballPlay()
            } else {
                delay(500)
                activity.viewModel.stopped()
                showErrorAndReset("Unable to load table.")
            }
        }
    }

    fun stop() {
        vpinballJNI.VPinballStop()
    }

    fun showError(message: String) {
        activity.viewModel.setError(message)
    }

    private fun showErrorAndReset(message: String) {
        CoroutineScope(Dispatchers.Main).launch {
            delay(250)
            showError(message)
            activeTable = null
        }
    }
}
