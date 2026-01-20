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

    // Web Server

    @Published var webServer: Bool = false
    @Published var webServerPort: Int = 0

    let vpinballManager = VPinballManager.shared

    init() {
        load()
    }

    func load() {
        // General

        if UIDevice.current.userInterfaceIdiom == .pad {
            haptics = false
            vpinballManager.saveValue(.standalone, "Haptics", false)
        } else {
            haptics = vpinballManager.loadValue(.standalone, "Haptics", true)
        }
        renderingModeOverride = (vpinballManager.loadValue(.standalone, "RenderingModeOverride", 2) == 2)

        // External DMD

        if vpinballManager.loadValue(.pluginDMDUtil, "DMDServer", false) {
            externalDMD = .dmdServer
        } else if vpinballManager.loadValue(.pluginDMDUtil, "ZeDMDWiFi", false) {
            externalDMD = .zedmdWiFi
        } else {
            externalDMD = .none
        }

        dmdServerAddr = vpinballManager.loadValue(.pluginDMDUtil, "DMDServerAddr", "0.0.0.0")
        dmdServerPort = vpinballManager.loadValue(.pluginDMDUtil, "DMDServerPort", 6789)
        zedmdWiFiAddr = vpinballManager.loadValue(.pluginDMDUtil, "ZeDMDWiFiAddr", "zedmd-wifi.local")

        // Display

        viewMode = VPinballViewMode(rawValue: vpinballManager.loadValue(.player, "BGSet", VPinballViewMode.desktopFSS.rawValue)) ?? .desktopFSS

        // Performance

        maxTexDimensionIndex = VPinballMaxTexDimension(rawValue: vpinballManager.loadValue(.player,
                                                                                           "MaxTexDimension",
                                                                                           1024))
            .flatMap { VPinballMaxTexDimension.all.firstIndex(of: $0) } ?? 0

        // Web Server

        webServer = vpinballManager.loadValue(.standalone, "WebServer", false)
        webServerPort = vpinballManager.loadValue(.standalone, "WebServerPort", 2112)

        // Advanced

        resetLogOnPlay = vpinballManager.loadValue(.standalone, "ResetLogOnPlay", true)
    }

    func reset() {
        load()
    }

    func handleHaptics() {
        vpinballManager.saveValue(.standalone, "Haptics", haptics)
    }

    func handleRenderingModeOverride() {
        vpinballManager.saveValue(.standalone, "RenderingModeOverride", renderingModeOverride ? 2 : -1)
    }

    func handleViewMode() {
        vpinballManager.saveValue(.player, "BGSet", viewMode.rawValue)
    }

    func handleResetLogOnPlay() {
        vpinballManager.saveValue(.standalone, "ResetLogOnPlay", resetLogOnPlay)
    }

    func handleExternalDMD() {
        vpinballManager.saveValue(.pluginDMDUtil, "DMDServer", externalDMD == .dmdServer)
        vpinballManager.saveValue(.pluginDMDUtil, "ZeDMDWiFi", externalDMD == .zedmdWiFi)
        vpinballManager.saveValue(.pluginDMDUtil, "Enable", externalDMD != .none)
    }

    func handleDMDServerAddr() {
        vpinballManager.saveValue(.pluginDMDUtil, "DMDServerAddr", dmdServerAddr)
    }

    func handleDMDServerPort() {
        vpinballManager.saveValue(.pluginDMDUtil, "DMDServerPort", dmdServerPort)
    }

    func handleZeDMDWiFiAddr() {
        vpinballManager.saveValue(.pluginDMDUtil, "ZeDMDWiFiAddr", zedmdWiFiAddr)
    }

    func handleWebServer() {
        vpinballManager.saveValue(.standalone, "WebServer", webServer)
        Task {
            vpinballManager.updateWebServer()
        }
    }

    func handleWebServerPort() {
        vpinballManager.saveValue(.standalone, "WebServerPort", Int(webServerPort))
        Task {
            vpinballManager.updateWebServer()
        }
    }

    func handleMaxTexDimension() {
        vpinballManager.saveValue(.player, "MaxTexDimension", VPinballMaxTexDimension.all[maxTexDimensionIndex].rawValue)
    }
}
