import SwiftUI

class SettingsModel: ObservableObject {
    // General

    @Published var haptics: Bool = false
    @Published var altColor: Bool = false
    @Published var altSound: Bool = false
    @Published var renderingModeOverride: Bool = false
    @Published var liveUIOverride: Bool = false
    @Published var viewMode: VPinballViewMode = .desktopFSS
    @Published var ssreflection: Bool = false
    @Published var resetLogOnPlay: Bool = false

    // External DMD

    @Published var externalDMD: VPinballExternalDMD = .none
    @Published var dmdServerAddr: String = ""
    @Published var dmdServerPort: Int = 0
    @Published var zedmdWiFiAddr: String = ""

    // Environment Lighting

    @Published var overrideEmissionScale: Bool = false
    @Published var nightDay: Int = 0

    // Ball Rendering

    @Published var ballTrail: Bool = false
    @Published var ballTrailStrength: Float = 0.0
    @Published var ballAntiStretch: Bool = false
    @Published var disableLightingForBalls: Bool = false

    // Performance

    @Published var maxAO: VPinballAO = .aoDisable
    @Published var maxReflectionMode: VPinballReflectionMode = .reflNone
    @Published var maxTexDimensionIndex: Int = 0
    @Published var forceAniso: Bool = false
    @Published var forceBloomOff: Bool = false
    @Published var forceMotionBlurOff: Bool = false
    @Published var alphaRampAccuracy: Int = 0

    // Anti Aliasing

    @Published var msaaSamplesIndex: Int = 0
    @Published var aaFactorIndex: Int = 0
    @Published var fxaa: VPinballFXAA = .disabled
    @Published var sharpen: VPinballSharpen = .disabled
    @Published var scaleFXDMD: Bool = false

    // Web Server

    @Published var webServer: Bool = false
    @Published var webServerPort: Int = 0

    let vpinballManager = VPinballManager.shared

    init() {
        load()
    }

    func load() {
        // General

        haptics = vpinballManager.loadValue(.standalone, "Haptics", true)
        altColor = vpinballManager.loadValue(.standalone, "AltColor", true)
        altSound = vpinballManager.loadValue(.standalone, "AltSound", true)
        renderingModeOverride = (vpinballManager.loadValue(.standalone, "RenderingModeOverride", 2) == 2)
        liveUIOverride = vpinballManager.loadValue(.standalone, "LiveUIOverride", true)
        viewMode = VPinballViewMode(rawValue: vpinballManager.loadValue(.player, "BGSet", VPinballViewMode.desktopFSS.rawValue)) ?? .desktopFSS
        ssreflection = vpinballManager.loadValue(.player, "SSRefl", false)
        resetLogOnPlay = vpinballManager.loadValue(.standalone, "ResetLogOnPlay", true)

        // External DMD

        if vpinballManager.loadValue(.standalone, "DMDServer", false) {
            externalDMD = .dmdServer
        } else if vpinballManager.loadValue(.standalone, "ZeDMDWiFi", false) {
            externalDMD = .zedmdWiFi
        } else {
            externalDMD = .none
        }

        dmdServerAddr = vpinballManager.loadValue(.standalone, "DMDServerAddr", "0.0.0.0")
        dmdServerPort = vpinballManager.loadValue(.standalone, "DMDServerPort", 6789)
        zedmdWiFiAddr = vpinballManager.loadValue(.standalone, "ZeDMDWiFiAddr", "zedmd-wifi.local")

        // Environment Lighting

        overrideEmissionScale = vpinballManager.loadValue(.tableOverride, "OverrideEmissionScale", false)
        nightDay = Int(vpinballManager.loadValue(.player, "EmissionScale", 0.5) * 100)

        // Ball Rendering

        ballTrail = vpinballManager.loadValue(.player, "BallTrail", true)
        ballTrailStrength = vpinballManager.loadValue(.player, "BallTrailStrength", 0.5)
        ballAntiStretch = vpinballManager.loadValue(.player, "BallAntiStretch", false)
        disableLightingForBalls = vpinballManager.loadValue(.player, "DisableLightingForBalls", false)

        // Performance

        let disableAO = vpinballManager.loadValue(.player, "DisableAO", false)
        let dynAO = vpinballManager.loadValue(.player, "DynamicAO", true)

        maxAO = disableAO ? .aoDisable : dynAO ? .aoDynamic : .aoStatic

        maxReflectionMode = VPinballReflectionMode(rawValue: vpinballManager.loadValue(.player,
                                                                                       "PFReflection",
                                                                                       VPinballReflectionMode.reflStaticNDynamic.rawValue)) ?? VPinballReflectionMode.reflStaticNDynamic

        maxTexDimensionIndex = VPinballMaxTexDimension(rawValue: vpinballManager.loadValue(.player,
                                                                                           "MaxTexDimension",
                                                                                           768))
            .flatMap { VPinballMaxTexDimension.all.firstIndex(of: $0) } ?? 0

        forceAniso = vpinballManager.loadValue(.player, "ForceAnisotropicFiltering", true)
        forceBloomOff = vpinballManager.loadValue(.player, "ForceBloomOff", false)
        forceMotionBlurOff = vpinballManager.loadValue(.player, "ForceMotionBlurOff", false)
        alphaRampAccuracy = vpinballManager.loadValue(.player, "AlphaRampAccuracy", 10)

        // Anti Aliasing

        msaaSamplesIndex = VPinballMSAASamples(rawValue: vpinballManager.loadValue(.player,
                                                                                   "MSAASamples",
                                                                                   1))
            .flatMap { VPinballMSAASamples.all.firstIndex(of: $0) } ?? 0

        aaFactorIndex = VPinballAAFactor.fromFloat(vpinballManager.loadValue(.player,
                                                                             "AAFactor",
                                                                             vpinballManager.loadValue(.player, "USEAA", false) ? 1.5 : 1.0))
            .flatMap { VPinballAAFactor.all.firstIndex(of: $0) } ?? 0

        fxaa = VPinballFXAA(rawValue: vpinballManager.loadValue(.player,
                                                                "FXAA",
                                                                VPinballFXAA.standardFXAA.rawValue)) ?? VPinballFXAA.standardFXAA

        sharpen = VPinballSharpen(rawValue: vpinballManager.loadValue(.player,
                                                                      "Sharpen",
                                                                      VPinballSharpen.disabled.rawValue)) ?? VPinballSharpen.disabled

        scaleFXDMD = vpinballManager.loadValue(.player,
                                               "ScaleFXDMD",
                                               false)

        // Web Server

        webServer = vpinballManager.loadValue(.standalone, "WebServer", false)
        webServerPort = vpinballManager.loadValue(.standalone, "WebServerPort", 2112)
    }

    func reset() {
        load()
    }
}
