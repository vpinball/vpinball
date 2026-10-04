import SwiftUI

@MainActor
class SettingsModel: ObservableObject {
    // General

    @Published var renderingModeOverride: Bool = false

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

        renderingModeOverride = (vpinballManager.loadValue(.standalone, "RenderingModeOverride", -1) == 2)

        // External DMD

        if vpinballManager.loadValue(.pluginDMDUtil, "DMDServer", false) {
            externalDMD = .dmdServer
        } else if vpinballManager.loadValue(.pluginDMDUtil, "ZeDMDWiFiEnabled", false) {
            externalDMD = .zedmdWiFi
        } else {
            externalDMD = .none
        }

        dmdServerAddr = vpinballManager.loadValue(.pluginDMDUtil, "DMDServerAddr", "0.0.0.0")
        dmdServerPort = vpinballManager.loadValue(.pluginDMDUtil, "DMDServerPort", 6789)
        zedmdWiFiAddr = vpinballManager.loadValue(.pluginDMDUtil, "ZeDMDWiFiAddr", "zedmd-wifi.local")

        // Performance

        maxTexDimensionIndex = VPinballMaxTexDimension(rawValue: vpinballManager.loadValue(.player,
                                                                                           "MaxTexDimension",
                                                                                           3072))
            .flatMap { VPinballMaxTexDimension.all.firstIndex(of: $0) } ?? 0

        // Web Server

        webServer = vpinballManager.loadValue(.standalone, "WebServer", false)
        webServerPort = vpinballManager.loadValue(.standalone, "WebServerPort", 2112)
    }

    func reset() {
        load()
    }

    func handleRenderingModeOverride() {
        vpinballManager.saveValue(.standalone, "RenderingModeOverride", renderingModeOverride ? 2 : -1)
    }

    func handleExternalDMD() {
        vpinballManager.saveValue(.pluginDMDUtil, "DMDServer", externalDMD == .dmdServer)
        vpinballManager.saveValue(.pluginDMDUtil, "ZeDMDWiFiEnabled", externalDMD == .zedmdWiFi)
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
        vpinballManager.updateWebServer()
    }

    func handleWebServerPort() {
        vpinballManager.saveValue(.standalone, "WebServerPort", Int(webServerPort))
        vpinballManager.updateWebServer()
    }

    func handleMaxTexDimension() {
        vpinballManager.saveValue(.player, "MaxTexDimension", VPinballMaxTexDimension.all[maxTexDimensionIndex].rawValue)
    }
}
