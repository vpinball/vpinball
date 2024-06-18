import SwiftUI

// VPinball Enums

enum VPinballLogLevel: CInt {
    case debug
    case info
    case warn
    case error
}

enum VPinballStatus: CInt {
    case success
    case failure
}

enum VPinballSettingsSection: CInt {
    case standalone = 2
    case player = 3
    case dmd = 4
    case alpha = 5
    case backglass = 6
    case tableOverride = 11
    case tableOption = 12
}

enum VPinballViewMode: CInt {
    case desktopFSS
    case cabinet
    case desktopNoFSS

    static let all: [VPinballViewMode] = [.desktopFSS,
                                          .cabinet,
                                          .desktopNoFSS]

    var name: String {
        switch self {
        case .desktopFSS:
            return "Desktop & FSS"
        case .cabinet:
            return "Cabinet"
        case .desktopNoFSS:
            return "Desktop (no FSS)"
        }
    }
}

enum VPinballAO: CInt {
    case aoDisable
    case aoStatic
    case aoDynamic

    static let all: [VPinballAO] = [.aoDisable,
                                    .aoStatic,
                                    .aoDynamic]

    var name: String {
        switch self {
        case .aoDisable:
            return "Disable AO"
        case .aoStatic:
            return "Static AO"
        case .aoDynamic:
            return "Dynamic AO"
        }
    }
}

enum VPinballReflectionMode: CInt {
    case reflNone
    case reflBalls
    case reflStatic
    case reflStaticNBalls
    case reflStaticNDynamic
    case reflDynamic

    static let all: [VPinballReflectionMode] = [.reflNone,
                                                .reflBalls,
                                                .reflStatic,
                                                .reflStaticNBalls,
                                                .reflStaticNDynamic,
                                                .reflDynamic]

    var name: String {
        switch self {
        case .reflNone:
            return "Disable Reflections"
        case .reflBalls:
            return "Balls Only"
        case .reflStatic:
            return "Static Only"
        case .reflStaticNBalls:
            return "Static & Balls"
        case .reflStaticNDynamic:
            return "Static & Unsynced Dynamic"
        case .reflDynamic:
            return "Dynamic"
        }
    }
}

enum VPinballMaxTexDimension: CInt {
    case unlimited = 0
    case max256 = 256
    case max384 = 384
    case max512 = 512
    case max768 = 768
    case max1024 = 1024
    case max1280 = 1280
    case max1536 = 1536
    case max1792 = 1792
    case max2048 = 2048
    case max3172 = 3172
    case max4096 = 4096

    static let all: [VPinballMaxTexDimension] = [.max256,
                                                 .max384,
                                                 .max512,
                                                 .max768,
                                                 .max1024,
                                                 .max1280,
                                                 .max1536,
                                                 .max1792,
                                                 .max2048,
                                                 .max3172,
                                                 .max4096,
                                                 .unlimited]

    var name: String {
        return self == .unlimited ? "Unlimited" : String(rawValue)
    }
}

enum VPinballMSAASamples: CInt {
    case disabled = 1
    case samples4 = 4
    case samples6 = 6
    case samples8 = 8

    static let all: [VPinballMSAASamples] = [.disabled,
                                             .samples4,
                                             .samples6,
                                             .samples8]

    var name: String {
        return self == .disabled ? "Disabled" : "\(rawValue) Samples"
    }
}

enum VPinballAAFactor: CInt {
    case pct50 = 50
    case pct75 = 75
    case disabled = 100
    case pct125 = 125
    case pct133 = 133
    case pct150 = 150
    case pct175 = 175
    case pct200 = 200

    static let all: [VPinballAAFactor] = [.pct50,
                                          .pct75,
                                          .disabled,
                                          .pct125,
                                          .pct133,
                                          .pct150,
                                          .pct175,
                                          .pct200]

    var name: String {
        return self == .disabled ? "Disabled" : "\(rawValue)%"
    }

    var floatValue: Float {
        return Float(rawValue) / 100.0
    }

    static func fromFloat(_ value: Float) -> VPinballAAFactor? {
        return VPinballAAFactor(rawValue: CInt(value * 100))
    }
}

enum VPinballFXAA: CInt {
    case disabled
    case fastFXAA
    case standardFXAA
    case qualityFXAA
    case fastNFAA
    case standardDLAA
    case qualitySMAA

    static let all: [VPinballFXAA] = [.disabled,
                                      .fastFXAA,
                                      .standardFXAA,
                                      .qualityFXAA,
                                      .fastNFAA,
                                      .standardDLAA,
                                      .qualitySMAA]

    var name: String {
        switch self {
        case .disabled:
            return "Disabled"
        case .fastFXAA:
            return "Fast FXAA"
        case .standardFXAA:
            return "Standard FXAA"
        case .qualityFXAA:
            return "Quality FXAA"
        case .fastNFAA:
            return "Fast NFAA"
        case .standardDLAA:
            return "Standard DLAA"
        case .qualitySMAA:
            return "Quality SMAA"
        }
    }
}

enum VPinballSharpen: CInt {
    case disabled
    case cas
    case bilateralCAS

    static let all: [VPinballSharpen] = [.disabled,
                                         .cas,
                                         .bilateralCAS]

    var name: String {
        switch self {
        case .disabled:
            return "Disabled"
        case .cas:
            return "CAS"
        case .bilateralCAS:
            return "Bilateral CAS"
        }
    }
}

enum VPinballToneMapper: CInt {
    case reinhard
    case agx
    case filmic
    case neutral
    case agxPunchy

    static let all: [VPinballToneMapper] = [.reinhard,
                                            .agx,
                                            .filmic,
                                            .neutral,
                                            .agxPunchy]

    var name: String {
        switch self {
        case .reinhard:
            return "Reinhard"
        case .agx:
            return "AgX"
        case .filmic:
            return "Filmic"
        case .neutral:
            return "Neutral"
        case .agxPunchy:
            return "AgX Punchy"
        }
    }
}

enum VPinballViewLayoutMode: CInt {
    case legacy
    case camera
    case window

    static let all: [VPinballViewLayoutMode] = [.legacy,
                                                .camera,
                                                .window]

    var name: String {
        switch self {
        case .legacy:
            return "Legacy"
        case .camera:
            return "Camera"
        case .window:
            return "Window"
        }
    }
}

enum VPinballExternalDMD: CInt {
    case none
    case dmdServer
    case zedmdWiFi

    static let all: [VPinballExternalDMD] = [.none,
                                             .dmdServer,
                                             .zedmdWiFi]

    var name: String {
        switch self {
        case .none:
            return "None"
        case .dmdServer:
            return "DMDServer"
        case .zedmdWiFi:
            return "ZeDMD WiFi"
        }
    }
}

// VPinball Event Enums

enum VPinballEvent: CInt {
    case archiveUncompressing
    case archiveCompressing
    case loadingItems
    case loadingSounds
    case loadingImages
    case loadingFonts
    case loadingCollections
    case playerStarting
    case windowCreated
    case metalLayerIOS
    case prerendering
    case playerStarted
    case rumble
    case scriptError
    case liveUIToggle
    case liveUIUpdate
    case playerClosing
    case playerClosed
    case stopped
    case webServer

    var name: String? {
        switch self {
        case .archiveUncompressing:
            return "Ucompressing"
        case .archiveCompressing:
            return "Compressing"
        case .loadingItems:
            return "Loading Items"
        case .loadingSounds:
            return "Loading Sounds"
        case .loadingImages:
            return "Loading Images"
        case .loadingFonts:
            return "Loading Fonts"
        case .loadingCollections:
            return "Loading Collections"
        case .prerendering:
            return "Prerendering Static Parts"
        default:
            return nil
        }
    }
}

enum VPinballScriptErrorType: CInt {
    case compile
    case runtime

    var name: String {
        switch self {
        case .compile:
            return "Compile error"
        case .runtime:
            return "Runtime error"
        }
    }
}

enum VPinballOptionUnit: CInt {
    case noUnit
    case percent

    func formatValue(_ value: Float) -> String {
        switch self {
        case .noUnit:
            return String(format: "%.1f", value)
        case .percent:
            return String(format: "%.1f %%", value * 100.0)
        }
    }
}

// VPinball Touch Areas

struct VPinballTouchArea {
    let left: CGFloat
    let top: CGFloat
    let right: CGFloat
    let bottom: CGFloat
    let label: String
}

let VPinballTouchAreas: [[VPinballTouchArea]] = [
    [VPinballTouchArea(left: 50,
                       top: 0,
                       right: 100,
                       bottom: 10,
                       label: "Menu")],
    [VPinballTouchArea(left: 0,
                       top: 0,
                       right: 50,
                       bottom: 10,
                       label: "Coin")],
    [VPinballTouchArea(left: 0,
                       top: 10,
                       right: 50,
                       bottom: 30,
                       label: "Left\nMagna-Save"),
     VPinballTouchArea(left: 50,
                       top: 10,
                       right: 100,
                       bottom: 30,
                       label: "Right\nMagna-Save")],
    [VPinballTouchArea(left: 0,
                       top: 30,
                       right: 50,
                       bottom: 60,
                       label: "Left\nNudge"),
     VPinballTouchArea(left: 50,
                       top: 30,
                       right: 100,
                       bottom: 60,
                       label: "Right\nNudge"),
     VPinballTouchArea(left: 30,
                       top: 60,
                       right: 70,
                       bottom: 100,
                       label: "Center\nNudge")],
    [VPinballTouchArea(left: 0,
                       top: 60,
                       right: 30,
                       bottom: 90,
                       label: "Left\nFlipper"),
     VPinballTouchArea(left: 70,
                       top: 60,
                       right: 100,
                       bottom: 90,
                       label: "Right\nFlipper")],
    [VPinballTouchArea(left: 70,
                       top: 90,
                       right: 100,
                       bottom: 100,
                       label: "Plunger")],
    [VPinballTouchArea(left: 0,
                       top: 90,
                       right: 30,
                       bottom: 100,
                       label: "Start")],
]

// VPinball Unit Converter

enum VPinballUnitConverter {
    static func cmToVPU(_ cm: Float) -> Float {
        return cm * (50.0 / (2.54 * 1.0625))
    }

    static func vpuToCM(_ vpu: Float) -> Float {
        return vpu * (2.54 * 1.0625 / 50.0)
    }
}

// VPinball Objects

struct VPinballProgressData {
    var progress: CInt
}

struct VPinballWindowCreatedData {
    var window: Unmanaged<UIWindow>?
}

struct VPinballScriptErrorData {
    var error: CInt
    var line: CInt
    var position: CInt
    var description: UnsafePointer<CChar>?
}

struct VPinballRumbleData {
    var low_frequency_rumble: UInt16
    var high_frequency_rumble: UInt16
    var duration_ms: UInt32
}

struct VPinballWebServerData {
    var url: UnsafePointer<CChar>?
}

struct VPinballTableOptions {
    var globalEmissionScale: Float = 0.0
    var globalDifficulty: Float = 0.0
    var exposure: Float = 0.0
    var toneMapper: CInt = 0
    var musicVolume: CInt = 0
    var soundVolume: CInt = 0
}

struct VPinballCustomTableOption {
    var section: CInt = 0
    var id: UnsafePointer<CChar>?
    var name: UnsafePointer<CChar>?
    var showMask: CInt = 0
    var minValue: Float = 0.0
    var maxValue: Float = 0.0
    var step: Float = 0.0
    var defaultValue: Float = 0.0
    var unit: CInt = 0
    var literals: UnsafePointer<CChar>?
    var value: Float = 0.0
}

struct VPinballViewSetup {
    var viewMode: CInt = 0
    var sceneScaleX: Float = 0.0
    var sceneScaleY: Float = 0.0
    var sceneScaleZ: Float = 0.0
    var viewX: Float = 0.0
    var viewY: Float = 0.0
    var viewZ: Float = 0.0
    var lookAt: Float = 0.0
    var viewportRotation: Float = 0.0
    var fov: Float = 0.0
    var layback: Float = 0.0
    var viewHOfs: Float = 0.0
    var viewVOfs: Float = 0.0
    var windowTopZOfs: Float = 0.0
    var windowBottomZOfs: Float = 0.0
}

// VPinball Callbacks

typealias VPinballEventCallback = @convention(c) (CInt, UnsafeRawPointer?) -> UnsafeRawPointer?

// VPinball C Definitions

@_silgen_name("VPinballInit")
func VPinballInit(_ callback: VPinballEventCallback)

@_silgen_name("VPinballLog")
func VPinballLog(_ level: CInt, _ pMessage: UnsafePointer<CChar>)

@_silgen_name("VPinballResetLog")
func VPinballResetLog()

@_silgen_name("VPinballLoadValueInt")
func VPinballLoadValueInt(_ section: CInt, _ pKey: UnsafePointer<CChar>, _ defaultValue: CInt) -> CInt

@_silgen_name("VPinballLoadValueFloat")
func VPinballLoadValueFloat(_ section: CInt, _ pKey: UnsafePointer<CChar>, _ defaultValue: Float) -> Float

@_silgen_name("VPinballLoadValueString")
func VPinballLoadValueString(_ section: CInt, _ pKey: UnsafePointer<CChar>, _ defaultValue: UnsafePointer<CChar>) -> UnsafePointer<CChar>

@_silgen_name("VPinballSaveValueInt")
func VPinballSaveValueInt(_ section: CInt, _ pKey: UnsafePointer<CChar>, _ value: CInt)

@_silgen_name("VPinballSaveValueFloat")
func VPinballSaveValueFloat(_ section: CInt, _ pKey: UnsafePointer<CChar>, _ value: Float)

@_silgen_name("VPinballSaveValueString")
func VPinballSaveValueString(_ section: CInt, _ pKey: UnsafePointer<CChar>, _ value: UnsafePointer<CChar>)

@_silgen_name("VPinballGetVersionStringFull")
func VPinballGetVersionStringFull() -> UnsafePointer<CChar>

@_silgen_name("VPinballUncompress")
func VPinballUncompress(_ pSource: UnsafePointer<CChar>) -> CInt

@_silgen_name("VPinballCompress")
func VPinballCompress(_ pSource: UnsafePointer<CChar>, _ pDestination: UnsafePointer<CChar>) -> CInt

@_silgen_name("VPinballUpdateWebServer")
func VPinballUpdateWebServer()

@_silgen_name("VPinballResetIni")
func VPinballResetIni() -> CInt

@_silgen_name("VPinballLoad")
func VPinballLoad(_ pSource: UnsafePointer<CChar>) -> CInt

@_silgen_name("VPinballExtractScript")
func VPinballExtractScript(_ pSource: UnsafePointer<CChar>) -> CInt

@_silgen_name("VPinballPlay")
func VPinballPlay() -> CInt

@_silgen_name("VPinballStop")
func VPinballStop()

@_silgen_name("VPinballSetPlayState")
func VPinballSetPlayState(_ enable: CInt) -> CInt

@_silgen_name("VPinballGetTableOptions")
func VPinballGetTableOptions(_ viewSetup: UnsafePointer<VPinballTableOptions>)

@_silgen_name("VPinballSetTableOptions")
func VPinballSetTableOptions(_ viewSetup: UnsafePointer<VPinballTableOptions>)

@_silgen_name("VPinballSetDefaultTableOptions")
func VPinballSetDefaultTableOptions()

@_silgen_name("VPinballResetTableOptions")
func VPinballResetTableOptions()

@_silgen_name("VPinballSaveTableOptions")
func VPinballSaveTableOptions()

@_silgen_name("VPinballGetCustomTableOptionsCount")
func VPinballGetCustomTableOptionsCount() -> CInt

@_silgen_name("VPinballGetCustomTableOption")
func VPinballGetCustomTableOption(_ index: CInt, _ customTableOption: UnsafePointer<VPinballCustomTableOption>)

@_silgen_name("VPinballSetCustomTableOption")
func VPinballSetCustomTableOption(_ customTableOption: UnsafePointer<VPinballCustomTableOption>)

@_silgen_name("VPinballSetDefaultCustomTableOptions")
func VPinballSetDefaultCustomTableOptions()

@_silgen_name("VPinballResetCustomTableOptions")
func VPinballResetCustomTableOptions()

@_silgen_name("VPinballSaveCustomTableOptions")
func VPinballSaveCustomTableOptions()

@_silgen_name("VPinballGetViewSetup")
func VPinballGetViewSetup(_ viewSetup: UnsafePointer<VPinballViewSetup>)

@_silgen_name("VPinballSetViewSetup")
func VPinballSetViewSetup(_ viewSetup: UnsafePointer<VPinballViewSetup>)

@_silgen_name("VPinballSetDefaultViewSetup")
func VPinballSetDefaultViewSetup()

@_silgen_name("VPinballResetViewSetup")
func VPinballResetViewSetup()

@_silgen_name("VPinballSaveViewSetup")
func VPinballSaveViewSetup()

@_silgen_name("VPinballToggleFPS")
func VPinballToggleFPS()
