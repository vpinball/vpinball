package org.vpinball.app.ui.screens.settings

import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.ViewModel
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import org.vpinball.app.SAFFileSystem
import org.vpinball.app.VPinballManager
import org.vpinball.app.jni.VPinballExternalDMD
import org.vpinball.app.jni.VPinballGfxBackend
import org.vpinball.app.jni.VPinballMaxTexDimension
import org.vpinball.app.jni.VPinballSettingsSection.PLAYER
import org.vpinball.app.jni.VPinballSettingsSection.STANDALONE
import org.vpinball.app.jni.VPinballStorageMode
import org.vpinball.app.jni.VPinballViewMode
import org.vpinball.app.ui.screens.landing.LandingScreenViewModel

class SettingsViewModel : ViewModel() {
    // General

    var haptics by mutableStateOf(false)
        private set

    var renderingModeOverride by mutableStateOf(false)
        private set

    var gfxBackend by mutableStateOf(VPinballGfxBackend.OPENGLES)
        private set

    var storageMode by mutableStateOf(VPinballStorageMode.INTERNAL)
        private set

    var currentTablesPath by mutableStateOf("")
        private set

    // Display

    var bgSet by mutableStateOf(VPinballViewMode.DESKTOP_FSS)
        private set

    // Performance

    var maxTexDimension by mutableStateOf(VPinballMaxTexDimension.MAX_768)
        private set

    var alphaRampAccuracy by mutableIntStateOf(0)
        private set

    // External DMD

    var externalDMD by mutableStateOf(VPinballExternalDMD.NONE)
        private set

    var dmdServerAddr by mutableStateOf("0.0.0.0")
        private set

    var dmdServerPort by mutableIntStateOf(6789)
        private set

    var zedmdWiFiAddr by mutableStateOf("zedmd-wifi.local")
        private set

    // Web Server

    var webServer by mutableStateOf(false)
        private set

    var webServerPort by mutableIntStateOf(0)
        private set

    // Advanced

    var resetLogOnPlay by mutableStateOf(false)
        private set

    var needsTableReload by mutableStateOf(false)
        private set

    fun loadSettings() {
        // General

        haptics = VPinballManager.loadValue(STANDALONE, "Haptics", true)
        renderingModeOverride = (VPinballManager.loadValue(STANDALONE, "RenderingModeOverride", 2) == 2)
        gfxBackend = VPinballGfxBackend.fromString(VPinballManager.loadValue(PLAYER, "GfxBackend", VPinballGfxBackend.OPENGLES.value))

        val savedTablesPath = VPinballManager.loadValue(STANDALONE, "TablesPath", "")
        storageMode = VPinballStorageMode.fromTablesPath(savedTablesPath)

        currentTablesPath =
            when (storageMode) {
                VPinballStorageMode.INTERNAL -> ""
                VPinballStorageMode.CUSTOM -> {
                    if (SAFFileSystem.isUsingSAF()) {
                        val displayPath = SAFFileSystem.getExternalStorageDisplayPath()
                        if (displayPath.isNotEmpty()) displayPath else savedTablesPath
                    } else {
                        savedTablesPath
                    }
                }
            }

        // Display

        bgSet = VPinballViewMode.fromInt(VPinballManager.loadValue(PLAYER, "BGSet", VPinballViewMode.DESKTOP_FSS.value))

        // Performance

        maxTexDimension =
            VPinballMaxTexDimension.fromInt(VPinballManager.loadValue(PLAYER, "MaxTexDimension", VPinballMaxTexDimension.MAX_1024.value))
        alphaRampAccuracy = VPinballManager.loadValue(PLAYER, "AlphaRampAccuracy", 10)

        // External DMD

        externalDMD =
            when {
                VPinballManager.loadValue(STANDALONE, "DMDServer", false) -> VPinballExternalDMD.DMD_SERVER

                VPinballManager.loadValue(STANDALONE, "ZeDMDWiFi", false) -> VPinballExternalDMD.ZEDMD_WIFI

                else -> VPinballExternalDMD.NONE
            }

        dmdServerAddr = VPinballManager.loadValue(STANDALONE, "DMDServerAddr", "0.0.0.0")
        dmdServerPort = VPinballManager.loadValue(STANDALONE, "DMDServerPort", 6789)
        zedmdWiFiAddr = VPinballManager.loadValue(STANDALONE, "ZeDMDWiFiAddr", "zedmd-wifi.local")

        // Web Server

        webServer = VPinballManager.loadValue(STANDALONE, "WebServer", false)
        webServerPort = VPinballManager.loadValue(STANDALONE, "WebServerPort", 2112)

        // Advanced

        resetLogOnPlay = VPinballManager.loadValue(STANDALONE, "ResetLogOnPlay", true)
    }

    // General

    fun handleHaptics(value: Boolean) {
        haptics = value
        VPinballManager.saveValue(STANDALONE, "Haptics", haptics)
    }

    fun handleRenderingModeOverride(value: Boolean) {
        renderingModeOverride = value
        VPinballManager.saveValue(STANDALONE, "RenderingModeOverride", if (renderingModeOverride) 2 else -1)
    }

    fun handleGfxBackend(value: VPinballGfxBackend) {
        gfxBackend = value
        VPinballManager.saveValue(PLAYER, "GfxBackend", value.value)
    }

    fun handleStorageMode(mode: VPinballStorageMode) {
        when (mode) {
            VPinballStorageMode.INTERNAL -> {
                SAFFileSystem.clearExternalStorageUri()
                storageMode = VPinballStorageMode.INTERNAL
                currentTablesPath = VPinballManager.getTablesPath()
                needsTableReload = true
            }
            VPinballStorageMode.CUSTOM -> {
                storageMode = VPinballStorageMode.CUSTOM
            }
        }
    }

    fun handleCustomStorageUri(uri: android.net.Uri) {
        SAFFileSystem.setExternalStorageUri(uri)
        storageMode = VPinballStorageMode.CUSTOM

        val displayPath = SAFFileSystem.getExternalStorageDisplayPath()
        currentTablesPath = if (displayPath.isNotEmpty()) displayPath else VPinballManager.getTablesPath()

        needsTableReload = true
    }

    fun triggerTableReloadIfNeeded() {
        if (needsTableReload) {
            needsTableReload = false
            CoroutineScope(Dispatchers.Main).launch { LandingScreenViewModel.triggerRefresh() }
        }
    }

    // Display

    fun handleBGSet(value: VPinballViewMode) {
        bgSet = value
        VPinballManager.saveValue(PLAYER, "BGSet", bgSet.value)
    }

    // Performance

    fun handleMaxTexDimension(value: VPinballMaxTexDimension) {
        maxTexDimension = value
        VPinballManager.saveValue(PLAYER, "MaxTexDimension", value.value)
    }

    fun handleAlphaRampAccuracy(value: Int) {
        alphaRampAccuracy = value
        VPinballManager.saveValue(PLAYER, "AlphaRampAccuracy", alphaRampAccuracy)
    }

    // External DMD

    fun handleExternalDMD(value: VPinballExternalDMD) {
        externalDMD = value
        VPinballManager.saveValue(STANDALONE, "DMDServer", externalDMD == VPinballExternalDMD.DMD_SERVER)
        VPinballManager.saveValue(STANDALONE, "ZeDMDWiFi", externalDMD == VPinballExternalDMD.ZEDMD_WIFI)
    }

    fun handleDMDServerAddr(value: String) {
        dmdServerAddr = value
        VPinballManager.saveValue(STANDALONE, "DMDServerAddr", dmdServerAddr)
    }

    fun handleDMDServerPort(value: Int) {
        dmdServerPort = value
        VPinballManager.saveValue(STANDALONE, "DMDServerPort", dmdServerPort)
    }

    fun handleZeDMDWiFiAddr(value: String) {
        zedmdWiFiAddr = value
        VPinballManager.saveValue(STANDALONE, "ZeDMDWiFiAddr", zedmdWiFiAddr)
    }

    // Web Server

    fun handleWebServer(value: Boolean) {
        webServer = value
        VPinballManager.saveValue(STANDALONE, "WebServer", webServer)
        VPinballManager.updateWebServer()
    }

    fun handleWebServerPort(value: Int) {
        webServerPort = value
        VPinballManager.saveValue(STANDALONE, "WebServerPort", webServerPort)
        VPinballManager.updateWebServer()
    }

    // Advanced

    fun handleResetLogOnPlay(value: Boolean) {
        resetLogOnPlay = value
        VPinballManager.saveValue(STANDALONE, "ResetLogOnPlay", resetLogOnPlay)
    }

    // Reset

    fun handleResetAllSettings() {
        VPinballManager.resetIni()
    }
}
