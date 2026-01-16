import SwiftUI

class VPinballManager {
    static let shared = VPinballManager()

    var activeTable: Table?

    let impactGenerators: [UIImpactFeedbackGenerator.FeedbackStyle: UIImpactFeedbackGenerator] = [
        .heavy: UIImpactFeedbackGenerator(style: .heavy),
        .light: UIImpactFeedbackGenerator(style: .light),
        .medium: UIImpactFeedbackGenerator(style: .medium),
        .rigid: UIImpactFeedbackGenerator(style: .rigid),
        .soft: UIImpactFeedbackGenerator(style: .soft),
    ]

    let vpinballViewModel = VPinballViewModel.shared

    private init() {
        impactGenerators.forEach { $0.value.prepare() }
    }

    func startup() {
        VPinballInit { value, data in
            let vpinballManager = VPinballManager.shared
            let vpinballViewModel = VPinballViewModel.shared
            let event = VPinballEvent(rawValue: value)
            switch event {
            case .loadingItems,
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
                        let apply = {
                            if let name = event?.name {
                                vpinballViewModel.updateProgressHUD(progress: progressData.progress,
                                                                    status: name)
                            } else {
                                vpinballViewModel.updateProgressHUD(progress: progressData.progress)
                            }
                        }

                        if Thread.isMainThread {
                            apply()
                            CATransaction.flush()
                            RunLoop.main.run(mode: .default,
                                             before: Date().addingTimeInterval(0))
                        } else {
                            DispatchQueue.main.async {
                                apply()
                            }
                        }
                    }
                }
            case .playerStarted:
                vpinballViewModel.isPlaying = true
            case .rumble:
                if let data = data {
                    let json = String(cString: UnsafePointer<CChar>(data))
                    if let jsonData = json.data(using: .utf8),
                       let rumbleData = try? JSONDecoder().decode(RumbleData.self,
                                                                  from: jsonData)
                    {
                        vpinballManager.rumble(rumbleData)
                    }
                }
            case .scriptError:
                if vpinballViewModel.scriptError == nil,
                   let data = data
                {
                    let json = String(cString: UnsafePointer<CChar>(data))
                    if let jsonData = json.data(using: .utf8),
                       let scriptErrorData = try? JSONDecoder().decode(ScriptErrorData.self,
                                                                       from: jsonData)
                    {
                        let type = VPinballScriptErrorType(rawValue: CInt(scriptErrorData.error))!
                        vpinballViewModel.scriptError = "\(type.name) on line \(scriptErrorData.line), position \(scriptErrorData.position):\n\n\(scriptErrorData.description)"
                    }
                }
            case .playerClosed:
                vpinballViewModel.isPlaying = false

                if let table = vpinballManager.activeTable {
                    Task {
                        await TableManager.shared.clearLoadedTable(table: table)
                        await TableManager.shared.loadTables()
                    }
                }

                vpinballManager.activeTable = nil

                if vpinballViewModel.scriptError != nil {
                    DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
                        vpinballViewModel.setAction(action: .showError,
                                                    table: nil)
                    }
                } else {
                    vpinballViewModel.setAction(action: .stopped)
                }
            case .webServer:
                if let data = data {
                    let json = String(cString: UnsafePointer<CChar>(data))
                    if let jsonData = json.data(using: .utf8),
                       let webServerData = try? JSONDecoder().decode(WebServerData.self,
                                                                     from: jsonData)
                    {
                        DispatchQueue.main.async {
                            vpinballViewModel.webServerURL = webServerData.url
                        }
                    }
                } else {
                    DispatchQueue.main.async {
                        vpinballViewModel.webServerURL = nil
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

            impactGenerators[style]!.impactOccurred()
        }
    }

    func play(table: Table) async -> Bool {
        if activeTable != nil {
            return false
        }

        if loadValue(.standalone, "ResetLogOnPlay", true) {
            VPinballResetLog()
        }

        activeTable = table
        vpinballViewModel.scriptError = nil

        await MainActor.run {
            vpinballViewModel.showProgressHUD(title: table.name,
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

                    activeTable = nil

                    success = false

                    VPinballManager.log(.error, "unable to start table")
                }
            } else {
                try? await Task.sleep(nanoseconds: 500_000_000)

                activeTable = nil

                success = false

                VPinballManager.log(.error, "unable to load table")
            }

            await MainActor.run {
                vpinballViewModel.hideHUD()
            }

            return success
        } else {
            try? await Task.sleep(nanoseconds: 500_000_000)
            activeTable = nil
            await MainActor.run {
                vpinballViewModel.hideHUD()
            }
            VPinballManager.log(.error, "unable to stage table")
            return false
        }
    }

    func stop() {
        if let table = activeTable {
            Task {
                await TableManager.shared.clearLoadedTable(table: table)
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
