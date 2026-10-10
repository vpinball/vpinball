import SwiftUI

@MainActor
class SettingsModel: ObservableObject {
    // General

    @Published var appearance: AppAppearance = .dark
    @Published var renderingModeOverride: Bool = false

    // External DMD

    @Published var externalDMD: VPinballExternalDMD = .none
    @Published var dmdServerAddr: String = ""
    @Published var dmdServerPort: Int = 0
    @Published var zedmdWiFiAddr: String = ""

    // Web Server

    @Published var webServer: Bool = false
    @Published var webServerPort: Int = 0

    let vpinballManager = VPinballManager.shared

    init() {
        load()
    }

    func load() {
        // General

        appearance = AppAppearance.current
        renderingModeOverride = (vpinballManager.loadValue(.standalone, "RenderingModeOverride", -1) == 2)

        // External DMD

        if vpinballManager.loadValue(.pluginDMDUtil, "DMDServer", false) {
            externalDMD = .dmdServer
        } else if vpinballManager.loadValue(.pluginDMDUtil, "ZeDMDWiFiEnabled", false) {
            externalDMD = .zedmdWiFi
        } else {
            externalDMD = .none
        }

        dmdServerAddr = vpinballManager.loadValue(.pluginDMDUtil, "DMDServerAddr", "localhost")
        dmdServerPort = vpinballManager.loadValue(.pluginDMDUtil, "DMDServerPort", 6789)
        zedmdWiFiAddr = vpinballManager.loadValue(.pluginDMDUtil, "ZeDMDWiFiAddr", "zedmd-wifi.local")

        // Web Server

        webServer = vpinballManager.loadValue(.standalone, "WebServer", false)
        webServerPort = vpinballManager.loadValue(.standalone, "WebServerPort", 2112)
    }

    func reset() {
        load()
    }

    func handleAppearance() {
        vpinballManager.saveValue(.standalone, "Appearance", appearance.rawValue)
        appearance.apply()
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
}
