package org.vpinball.app.jni

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

enum class VPinballStatus(val value: Int) {
    SUCCESS(0),
    FAILURE(1),
}

enum class VPinballSettingsSection(val value: Int) {
    STANDALONE(2),
    PLAYER(3),
    DMD(4),
    ALPHA(5),
    BACKGLASS(6),
    TABLE_OVERRIDE(11),
    TABLE_OPTION(12);

    companion object {
        @JvmStatic
        fun fromInt(value: Int): VPinballSettingsSection {
            return VPinballSettingsSection.entries.firstOrNull { it.value == value } ?: throw IllegalArgumentException("Unknown value: $value")
        }
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

enum class VPinballAO(val value: Int, override val text: String) : VPinballDisplayText {
    AO_DISABLE(0, "Disable AO"),
    AO_STATIC(1, "Static AO"),
    AO_DYNAMIC(2, "Dynamic AO");

    companion object {
        @JvmStatic fun fromInt(value: Int): VPinballAO = entries.firstOrNull { it.value == value } ?: AO_DISABLE
    }
}

enum class VPinballReflectionMode(val value: Int, override val text: String) : VPinballDisplayText {
    REFL_NONE(0, "Disable Reflections"),
    REFL_BALLS(1, "Balls Only"),
    REFL_STATIC(2, "Static Only"),
    REFL_STATIC_N_BALLS(3, "Static & Balls"),
    REFL_STATIC_N_DYNAMIC(4, "Static & Unsynced Dynamic"),
    REFL_DYNAMIC(5, "Dynamic");

    companion object {
        @JvmStatic fun fromInt(value: Int): VPinballReflectionMode = entries.firstOrNull { it.value == value } ?: REFL_NONE
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

enum class VPinballMSAASamples(val value: Int, override val text: String) : VPinballDisplayText {
    DISABLED(1, "Disabled"),
    SAMPLES_4(4, "4 Samples"),
    SAMPLES_6(6, "6 Samples"),
    SAMPLES_8(8, "8 Samples");

    companion object {
        @JvmStatic fun fromInt(value: Int): VPinballMSAASamples = entries.firstOrNull { it.value == value } ?: DISABLED
    }
}

enum class VPinballAAFactor(val value: Int, override val text: String) : VPinballDisplayText {
    PCT_50(50, "50%"),
    PCT_75(75, "75%"),
    DISABLED(100, "Disabled"),
    PCT_125(125, "125%"),
    PCT_133(133, "133%"),
    PCT_150(150, "150%"),
    PCT_175(175, "175%"),
    PCT_200(200, "200%");

    val floatValue: Float
        get() = value / 100.0f

    companion object {
        fun fromFloat(value: Float): VPinballAAFactor = entries.firstOrNull { it.value == (value * 100).toInt() } ?: DISABLED
    }
}

enum class VPinballFXAA(val value: Int, override val text: String) : VPinballDisplayText {
    DISABLED(0, "Disabled"),
    FAST_FXAA(1, "Fast FXAA"),
    STANDARD_FXAA(2, "Standard FXAA"),
    QUALITY_FXAA(3, "Quality FXAA"),
    FAST_NFAA(4, "Fast NFAA"),
    STANDARD_DLAA(5, "Standard DLAA"),
    QUALITY_SMAA(6, "Quality SMAA");

    companion object {
        @JvmStatic fun fromInt(value: Int): VPinballFXAA = entries.firstOrNull { it.value == value } ?: DISABLED
    }
}

enum class VPinballSharpen(val value: Int, override val text: String) : VPinballDisplayText {
    DISABLED(0, "Disabled"),
    CAS(1, "CAS"),
    BILATERAL_CAS(2, "Bilateral CAS");

    companion object {
        @JvmStatic fun fromInt(value: Int): VPinballSharpen = entries.firstOrNull { it.value == value } ?: DISABLED
    }
}

enum class VPinballToneMapper(val value: Int, override val text: String) : VPinballDisplayText {
    REINHARD(0, "Reinhard"),
    AGX(1, "AgX"),
    FILMIC(2, "Filmic"),
    NEUTRAL(3, "Neutral"),
    AGX_PUNCHY(4, "AgX Punchy");

    companion object {
        @JvmStatic fun fromInt(value: Int): VPinballToneMapper = entries.firstOrNull { it.value == value } ?: REINHARD
    }
}

enum class VPinballViewLayoutMode(val value: Int, override val text: String) : VPinballDisplayText {
    LEGACY(0, "Legacy"),
    CAMERA(1, "Camera"),
    WINDOW(2, "Window");

    companion object {
        @JvmStatic fun fromInt(value: Int): VPinballViewLayoutMode = entries.firstOrNull { it.value == value } ?: LEGACY
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

// VPinball Event Enums

enum class VPinballEvent(val value: Int) {
    ARCHIVE_UNCOMPRESSING(0),
    ARCHIVE_COMPRESSING(1),
    LOADING_ITEMS(2),
    LOADING_SOUNDS(3),
    LOADING_IMAGES(4),
    LOADING_FONTS(5),
    LOADING_COLLECTIONS(6),
    PLAYER_STARTING(7),
    WINDOW_CREATED(8),
    METAL_LAYER_IOS(9),
    PRERENDERING(10),
    PLAYER_STARTED(11),
    RUMBLE(12),
    SCRIPT_ERROR(13),
    LIVE_UI_TOGGLE(14),
    LIVE_UI_UPDATE(15),
    PLAYER_CLOSING(16),
    PLAYER_CLOSED(17),
    STOPPED(18),
    WEB_SERVER(19);

    val text: String?
        get() =
            when (this) {
                ARCHIVE_UNCOMPRESSING -> "Uncompressing"
                ARCHIVE_COMPRESSING -> "Compressing"
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

enum class VPinballOptionUnit(val value: Int) {
    NO_UNIT(0),
    PERCENT(1);

    fun formatValue(value: Float): String =
        when (this) {
            NO_UNIT -> String.format("%.1f", value)
            PERCENT -> String.format("%.1f %%", value * 100.0f)
        }

    companion object {
        @JvmStatic
        fun fromInt(value: Int): VPinballOptionUnit {
            return VPinballOptionUnit.entries.firstOrNull { it.value == value } ?: throw IllegalArgumentException("Unknown value: $value")
        }
    }
}

// VPinball Touch Areas

data class VPinballTouchArea(val left: Float, val top: Float, val right: Float, val bottom: Float, val label: String)

val VPinballTouchAreas: List<List<VPinballTouchArea>> =
    listOf(
        listOf(VPinballTouchArea(left = 50f, top = 0f, right = 100f, bottom = 10f, label = "Menu")),
        listOf(VPinballTouchArea(left = 0f, top = 0f, right = 50f, bottom = 10f, label = "Coin")),
        listOf(
            VPinballTouchArea(left = 0f, top = 10f, right = 50f, bottom = 30f, label = "Left\nMagna-Save"),
            VPinballTouchArea(left = 50f, top = 10f, right = 100f, bottom = 30f, label = "Right\nMagna-Save"),
        ),
        listOf(
            VPinballTouchArea(left = 0f, top = 30f, right = 50f, bottom = 60f, label = "Left\nNudge"),
            VPinballTouchArea(left = 50f, top = 30f, right = 100f, bottom = 60f, label = "Right\nNudge"),
            VPinballTouchArea(left = 30f, top = 60f, right = 70f, bottom = 100f, label = "Center\nNudge"),
        ),
        listOf(
            VPinballTouchArea(left = 0f, top = 60f, right = 30f, bottom = 90f, label = "Left\nFlipper"),
            VPinballTouchArea(left = 70f, top = 60f, right = 100f, bottom = 90f, label = "Right\nFlipper"),
        ),
        listOf(VPinballTouchArea(left = 70f, top = 90f, right = 100f, bottom = 100f, label = "Plunger")),
        listOf(VPinballTouchArea(left = 0f, top = 90f, right = 30f, bottom = 100f, label = "Start")),
    )

// VPinball Unit Converter

object VPinballUnitConverter {
    fun cmToVPU(cm: Float): Float = cm * (50.0f / (2.54f * 1.0625f))

    fun vpuToCM(vpu: Float): Float = vpu * (2.54f * 1.0625f / 50.0f)
}

// VPinball Callbacks

fun interface VPinballEventCallback {
    fun onEvent(event: Int, data: Any?): Any?
}

// VPinball Objects

data class VPinballProgressData(val progress: Int)

data class VPinballScriptErrorData(val error: VPinballScriptErrorType, val line: Int, val position: Int, val description: String)

data class VPinballWebServerData(val url: String)

data class VPinballRumbleData(val lowFrequencyRumble: Int, val highFrequencyRumble: Int, val durationMs: Int)

data class VPinballCustomTableOption(
    var section: VPinballSettingsSection,
    var id: String,
    var name: String,
    var showMask: Int,
    var minValue: Float,
    var maxValue: Float,
    var step: Float,
    var defaultValue: Float,
    var unit: VPinballOptionUnit,
    var literals: String,
    var value: Float,
)

data class VPinballTableOptions(
    var globalEmissionScale: Float,
    var globalDifficulty: Float,
    var exposure: Float,
    var toneMapper: VPinballToneMapper,
    var musicVolume: Int,
    var soundVolume: Int,
)

data class VPinballViewSetup(
    var viewMode: VPinballViewLayoutMode,
    var sceneScaleX: Float,
    var sceneScaleY: Float,
    var sceneScaleZ: Float,
    var viewX: Float,
    var viewY: Float,
    var viewZ: Float,
    var lookAt: Float,
    var viewportRotation: Float,
    var fov: Float,
    var layback: Float,
    var viewHOfs: Float,
    var viewVOfs: Float,
    var windowTopZOfs: Float,
    var windowBottomZOfs: Float,
)
