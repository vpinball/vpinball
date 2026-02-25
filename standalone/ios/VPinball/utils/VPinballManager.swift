import SwiftUI

class VPinballManager {
    static let shared = VPinballManager()

    let impactGenerators: [UIImpactFeedbackGenerator.FeedbackStyle: UIImpactFeedbackGenerator] = [
        .heavy: UIImpactFeedbackGenerator(style: .heavy),
        .light: UIImpactFeedbackGenerator(style: .light),
        .medium: UIImpactFeedbackGenerator(style: .medium),
        .rigid: UIImpactFeedbackGenerator(style: .rigid),
        .soft: UIImpactFeedbackGenerator(style: .soft),
    ]

    private init() {
        impactGenerators.forEach { $0.value.prepare() }
    }

    func startup() {
        VPinballInit { value, data in
            let event = VPinballEvent(rawValue: value)
            switch event {
            case .extractScript,
                 .loadingItems,
                 .loadingSounds,
                 .loadingImages,
                 .loadingFonts,
                 .loadingCollections,
                 .prerendering:
                if let data = data {
                    let json = String(cString: UnsafePointer<CChar>(data))
                    if let jsonData = json.data(using: .utf8),
                       let progressData = try? JSONDecoder().decode(ProgressEventData.self,
                                                                    from: jsonData)
                    {
                        let progress = progressData.progress
                        let eventName = event?.name
                        Task { @MainActor in
                            if let name = eventName {
                                VPinballModel.shared.updateHUD(progress: progress,
                                                               status: name)
                            } else {
                                VPinballModel.shared.updateHUD(progress: progress)
                            }
                            CATransaction.flush()
                        }
                    }
                }
            case .playerStarted:
                Task { @MainActor in
                    VPinballModel.shared.isPlaying = true
                }
            case .rumble:
                if let data = data {
                    let json = String(cString: UnsafePointer<CChar>(data))
                    if let jsonData = json.data(using: .utf8),
                       let rumbleData = try? JSONDecoder().decode(RumbleData.self,
                                                                  from: jsonData)
                    {
                        VPinballManager.shared.rumble(rumbleData)
                    }
                }
            case .playerClosed:
                Task { @MainActor in
                    let table = VPinballModel.shared.activeTable
                    VPinballModel.shared.activeTable = nil
                    VPinballModel.shared.isPlaying = false
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

    func rumble(_ data: RumbleData) {
        if data.lowFrequencyRumble > 0 || data.highFrequencyRumble > 0 {
            let style: UIImpactFeedbackGenerator.FeedbackStyle

            if data.lowFrequencyRumble == data.highFrequencyRumble {
                style = .rigid
            } else if data.lowFrequencyRumble > 20000 || data.highFrequencyRumble > 20000 {
                style = .heavy
            } else if data.lowFrequencyRumble > 10000 || data.highFrequencyRumble > 10000 {
                style = .medium
            } else if data.lowFrequencyRumble > 1000 || data.highFrequencyRumble > 1000 {
                style = .light
            } else {
                style = .soft
            }

            impactGenerators[style]?.impactOccurred()
        }
    }

    func play(table: Table) async -> Bool {
        if await MainActor.run(body: { VPinballModel.shared.activeTable != nil }) {
            return false
        }

        if loadValue(.standalone, "ResetLogOnPlay", true) {
            VPinballResetLog()
        }

        await MainActor.run {
            VPinballModel.shared.activeTable = table
            MainViewModel.shared.errorMessage = ""
            tableImageCache.removeAllObjects()

            VPinballModel.shared.showHUD(title: table.name,
                                         status: "Launching")
        }

        var success = true

        if let tablePath = await TableManager.shared.getLoadedTablePath(table: table) {
            if await Task.detached(
                priority: .userInitiated,
                operation: { [tablePath] in
                    VPinballStatus(rawValue: VPinballLoadTable(tablePath.cstring))
                }
            ).value == .success {
                if await MainActor.run(body: { VPinballStatus(rawValue: VPinballPlay()) }) != .success {
                    try? await Task.sleep(nanoseconds: 500_000_000)

                    await MainActor.run { VPinballModel.shared.activeTable = nil }

                    success = false

                    VPinballManager.log(.error, "unable to start table")
                }
            } else {
                try? await Task.sleep(nanoseconds: 500_000_000)

                await MainActor.run { VPinballModel.shared.activeTable = nil }

                success = false

                VPinballManager.log(.error, "unable to load table")
            }

            await MainActor.run {
                VPinballModel.shared.hideHUD()
            }

            return success
        } else {
            try? await Task.sleep(nanoseconds: 500_000_000)
            await MainActor.run {
                VPinballModel.shared.activeTable = nil
                VPinballModel.shared.hideHUD()
            }
            VPinballManager.log(.error, "unable to stage table")
            return false
        }
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
