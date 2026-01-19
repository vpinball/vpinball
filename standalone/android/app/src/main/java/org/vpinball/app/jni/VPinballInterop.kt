package org.vpinball.app.jni

import kotlinx.serialization.Serializable

interface VPinballDisplayText {
    val text: String
}

// VPinball Enums

enum class VPinballLogLevel(val value: Int) {
    DEBUG(0),
    INFO(1),
    WARN(2),
    ERROR(3),
}

enum class VPinballPath(val value: Int) {
    ROOT(0),
    TABLES(1),
    PREFERENCES(2),
    ASSETS(3),
}

enum class VPinballStatus(val value: Int) {
    SUCCESS(0),
    FAILURE(1),
}

enum class VPinballSettingsSection(val value: String) {
    STANDALONE("Standalone"),
    PLAYER("Player"),
    PLUGIN_DMDUTIL("Plugin.DMDUtil");

    companion object {
        @JvmStatic
        fun fromValue(value: String): VPinballSettingsSection =
            entries.firstOrNull { it.value == value } ?: throw IllegalArgumentException("Unknown value: $value")
    }
}

enum class VPinballViewMode(val value: Int, override val text: String) : VPinballDisplayText {
    DESKTOP_FSS(0, "Desktop & FSS"),
    CABINET(1, "Cabinet"),
    DESKTOP_NO_FSS(2, "Desktop (no FSS)");

    companion object {
        @JvmStatic fun fromInt(value: Int): VPinballViewMode = entries.firstOrNull { it.value == value } ?: DESKTOP_FSS
    }
}

enum class VPinballMaxTexDimension(val value: Int, override val text: String) : VPinballDisplayText {
    MAX_256(256, "256"),
    MAX_384(384, "384"),
    MAX_512(512, "512"),
    MAX_768(768, "768"),
    MAX_1024(1024, "1024"),
    MAX_1280(1280, "1280"),
    MAX_1536(1536, "1536"),
    MAX_1792(1792, "1792"),
    MAX_2048(2048, "2048"),
    MAX_3172(3172, "3172"),
    MAX_4096(4096, "4096"),
    UNLIMITED(0, "Unlimited");

    companion object {
        @JvmStatic fun fromInt(value: Int): VPinballMaxTexDimension = entries.firstOrNull { it.value == value } ?: UNLIMITED
    }
}

enum class VPinballExternalDMD(val value: Int, override val text: String) : VPinballDisplayText {
    NONE(0, "None"),
    DMD_SERVER(1, "DMDServer"),
    ZEDMD_WIFI(2, "ZeDMD WiFi");

    companion object {
        @JvmStatic fun fromInt(value: Int): VPinballExternalDMD = entries.firstOrNull { it.value == value } ?: NONE
    }
}

enum class VPinballGfxBackend(val value: String, override val text: String) : VPinballDisplayText {
    OPENGLES("OpenGLES", "OpenGLES"),
    VULKAN("Vulkan", "Vulkan");

    companion object {
        fun fromString(value: String): VPinballGfxBackend = entries.firstOrNull { it.value.equals(value, ignoreCase = true) } ?: OPENGLES
    }
}

enum class VPinballStorageMode(override val text: String) : VPinballDisplayText {
    INTERNAL("Internal"),
    CUSTOM("Custom");

    companion object {
        fun fromSAFPath(safPath: String): VPinballStorageMode {
            return if (safPath.isEmpty()) INTERNAL else CUSTOM
        }
    }
}

// VPinball Event Enums

enum class VPinballEvent(val value: Int) {
    INIT_COMPLETE(0),
    LOADING_ITEMS(1),
    LOADING_SOUNDS(2),
    LOADING_IMAGES(3),
    LOADING_FONTS(4),
    LOADING_COLLECTIONS(5),
    PRERENDERING(6),
    PLAYER_STARTED(7),
    RUMBLE(8),
    SCRIPT_ERROR(9),
    PLAYER_CLOSED(10),
    WEB_SERVER(11),
    COMMAND(12);

    val text: String?
        get() =
            when (this) {
                LOADING_ITEMS -> "Loading Items"
                LOADING_SOUNDS -> "Loading Sounds"
                LOADING_IMAGES -> "Loading Images"
                LOADING_FONTS -> "Loading Fonts"
                LOADING_COLLECTIONS -> "Loading Collections"
                PRERENDERING -> "Prerendering Static Parts"
                else -> null
            }
}

enum class VPinballScriptErrorType(val value: Int) {
    COMPILE(0),
    RUNTIME(1);

    val text: String
        get() =
            when (this) {
                COMPILE -> "Compile error"
                RUNTIME -> "Runtime error"
            }

    companion object {
        @JvmStatic
        fun fromInt(value: Int): VPinballScriptErrorType {
            return entries.firstOrNull { it.value == value } ?: throw IllegalArgumentException("Unknown value: $value")
        }
    }
}

// VPinball Callbacks

fun interface VPinballEventCallback {
    fun onEvent(event: Int, jsonData: String?)
}

fun interface VPinballZipCallback {
    fun onProgress(current: Int, total: Int, filename: String)
}

// VPinball Objects

@Serializable data class VPinballProgressData(val progress: Int)

@Serializable data class VPinballRumbleData(val lowFrequencyRumble: Int, val highFrequencyRumble: Int, val durationMs: Int)

@Serializable data class VPinballScriptErrorData(val error: Int, val line: Int, val position: Int, val description: String)

@Serializable data class VPinballWebServerData(val url: String)

@Serializable data class VPinballCommandData(val command: String, val data: String? = null)
