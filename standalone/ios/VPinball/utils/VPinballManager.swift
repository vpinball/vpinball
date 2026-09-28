import SwiftUI

class VPinballManager {
    static let shared = VPinballManager()

    private init() {}

    func startup() {
        HapticsManager.shared.start()

        VPinballInit({ value, data in
            let event = VPinballEvent(rawValue: value)
            switch event {
            case .extractScript,
                 .loading:
                if let data = data {
                    let json = String(cString: UnsafePointer<CChar>(data))
                    if let jsonData = json.data(using: .utf8),
                       let progressData = try? JSONDecoder().decode(ProgressEventData.self,
                                                                    from: jsonData)
                    {
                        let progress = progressData.progress
                        let eventName = event?.name
                        VPinballManager.runOnMain {
                            if let name = eventName {
                                VPinballModel.shared.updateHUD(progress: progress,
                                                               status: name)
                            } else {
                                VPinballModel.shared.updateHUD(progress: progress)
                            }
                        }
                    }
                }
            case .playerReady:
                VPinballManager.runOnMain {
                    VPinballModel.shared.isPlaying = true
                    VPinballModel.shared.hideHUD()
                }
            case .playerFailed:
                Task { @MainActor in
                    VPinballManager.onPlayerFailed(message: "Unable to load table")
                }
            case .playerClosed:
                Task { @MainActor in
                    let table = VPinballModel.shared.activeTable
                    VPinballModel.shared.activeTable = nil
                    VPinballModel.shared.isPlaying = false
                    VPinballModel.shared.hideHUD()
                    MainViewModel.shared.setAction(.stopped)

                    if let table {
                        Task {
                            TableManager.shared.clearLoadedTable(table: table)
                            await TableManager.shared.loadTables()
                            try? await Task.sleep(nanoseconds: 1_000_000_000)
                            await TableManager.shared.loadTables()
                        }
                    }
                }
            case .webServer:
                if let data = data {
                    let json = String(cString: UnsafePointer<CChar>(data))
                    if let jsonData = json.data(using: .utf8),
                       let webServerData = try? JSONDecoder().decode(WebServerData.self,
                                                                     from: jsonData)
                    {
                        Task { @MainActor in
                            VPinballModel.shared.webServerURL = webServerData.url
                        }
                    }
                } else {
                    Task { @MainActor in
                        VPinballModel.shared.webServerURL = nil
                    }
                }
            case .command:
                if let data = data {
                    let json = String(cString: UnsafePointer<CChar>(data))
                    if let jsonData = json.data(using: .utf8),
                       let commandData = try? JSONDecoder().decode(CommandData.self,
                                                                   from: jsonData)
                    {
                        if commandData.command == "reloadTables" {
                            Task {
                                await TableManager.shared.loadTables()
                            }
                        }
                    }
                }
            default:
                break
            }
        }, { lowFrequencySpeed, highFrequencySpeed, durationMs in
            HapticsManager.shared.play(lowFrequencySpeed: lowFrequencySpeed,
                                       highFrequencySpeed: highFrequencySpeed,
                                       durationMs: durationMs)
        })
    }

    private static func runOnMain(_ body: @escaping @MainActor () -> Void) {
        if Thread.isMainThread {
            MainActor.assumeIsolated(body)
            CATransaction.flush()
        } else {
            Task { @MainActor in body() }
        }
    }

    static func log(_ level: VPinballLogLevel, _ message: String) {
        VPinballLog(level.rawValue, message.cstring)
    }

    func loadValue(_ section: VPinballSettingsSection, _ key: String, _ defaultValue: CInt) -> CInt {
        return VPinballLoadValueInt(section.rawValue.cstring, key.cstring, defaultValue)
    }

    func loadValue(_ section: VPinballSettingsSection, _ key: String, _ defaultValue: Int) -> Int {
        return Int(loadValue(section, key, CInt(defaultValue)))
    }

    func loadValue(_ section: VPinballSettingsSection, _ key: String, _ defaultValue: Float) -> Float {
        return VPinballLoadValueFloat(section.rawValue.cstring, key.cstring, defaultValue)
    }

    func loadValue(_ section: VPinballSettingsSection, _ key: String, _ defaultValue: Bool) -> Bool {
        return VPinballLoadValueBool(section.rawValue.cstring, key.cstring, defaultValue ? 1 : 0) != 0
    }

    func loadValue(_ section: VPinballSettingsSection, _ key: String, _ defaultValue: String) -> String {
        return String(cString: VPinballLoadValueString(section.rawValue.cstring, key.cstring, defaultValue.cstring))
    }

    func saveValue(_ section: VPinballSettingsSection, _ key: String, _ value: CInt) {
        VPinballSaveValueInt(section.rawValue.cstring, key.cstring, value)
    }

    func saveValue(_ section: VPinballSettingsSection, _ key: String, _ value: Int) {
        saveValue(section, key, CInt(value))
    }

    func saveValue(_ section: VPinballSettingsSection, _ key: String, _ value: Float) {
        VPinballSaveValueFloat(section.rawValue.cstring, key.cstring, value)
    }

    func saveValue(_ section: VPinballSettingsSection, _ key: String, _ value: Bool) {
        VPinballSaveValueBool(section.rawValue.cstring, key.cstring, value ? 1 : 0)
    }

    func saveValue(_ section: VPinballSettingsSection, _ key: String, _ value: String) {
        VPinballSaveValueString(section.rawValue.cstring, key.cstring, value.cstring)
    }

    func play(table: Table) async {
        if await MainActor.run(body: { VPinballModel.shared.activeTable != nil }) {
            return
        }

        await MainActor.run {
            VPinballModel.shared.activeTable = table
            MainViewModel.shared.errorMessage = ""
            tableImageCache.removeAllObjects()

            VPinballModel.shared.showHUD(title: table.name,
                                         status: "Launching")
        }

        if let tablePath = await TableManager.shared.getLoadedTablePath(table: table),
           await MainActor.run(body: { VPinballStatus(rawValue: VPinballPlay(tablePath.cstring)) }) == .success
        {
            return
        }

        VPinballManager.log(.error, "unable to play table")
        try? await Task.sleep(nanoseconds: 500_000_000)
        await VPinballManager.onPlayerFailed(message: "Unable to load table")
    }

    @MainActor
    static func onPlayerFailed(message: String) {
        VPinballModel.shared.activeTable = nil
        VPinballModel.shared.isPlaying = false
        VPinballModel.shared.hideHUD()
        MainViewModel.shared.handleShowError(message: message)
    }

    func stop() {
        Task { @MainActor in
            if let table = VPinballModel.shared.activeTable {
                TableManager.shared.clearLoadedTable(table: table)
            }
        }
        VPinballStop()
    }

    func resetIni() {
        _ = VPinballResetIni()
    }

    func updateWebServer() {
        VPinballUpdateWebServer()
    }

    func getPath(_ pathType: VPinballPath) -> String {
        return String(cString: VPinballGetPath(pathType.rawValue))
    }
}
