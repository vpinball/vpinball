import SwiftUI

class SettingsModel: ObservableObject {
    // General

    @Published var haptics: Bool = false
    @Published var renderingModeOverride: Bool = false
    @Published var viewMode: VPinballViewMode = .desktopFSS
    @Published var resetLogOnPlay: Bool = false

    // External DMD

    @Published var externalDMD: VPinballExternalDMD = .none
    @Published var dmdServerAddr: String = ""
    @Published var dmdServerPort: Int = 0
    @Published var zedmdWiFiAddr: String = ""

    // Performance

    @Published var maxTexDimensionIndex: Int = 0
    @Published var alphaRampAccuracy: Int = 0

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
        renderingModeOverride = (vpinballManager.loadValue(.standalone, "RenderingModeOverride", 2) == 2)

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

        // Display

        viewMode = VPinballViewMode(rawValue: vpinballManager.loadValue(.player, "BGSet", VPinballViewMode.desktopFSS.rawValue)) ?? .desktopFSS

        // Performance

        maxTexDimensionIndex = VPinballMaxTexDimension(rawValue: vpinballManager.loadValue(.player,
                                                                                           "MaxTexDimension",
                                                                                           1024))
            .flatMap { VPinballMaxTexDimension.all.firstIndex(of: $0) } ?? 0

        alphaRampAccuracy = vpinballManager.loadValue(.player, "AlphaRampAccuracy", 10)

        // Web Server

        webServer = vpinballManager.loadValue(.standalone, "WebServer", false)
        webServerPort = vpinballManager.loadValue(.standalone, "WebServerPort", 2112)

        // Advanced

        resetLogOnPlay = vpinballManager.loadValue(.standalone, "ResetLogOnPlay", true)
    }

    func reset() {
        load()
    }
}
