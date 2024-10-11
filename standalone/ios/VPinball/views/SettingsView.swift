import MessageUI
import SwiftUI

struct SettingsView: View {
    enum ExportFile: Identifiable {
        case log
        case ini

        var id: Int {
            hashValue
        }

        var name: String {
            switch self {
            case .log:
                return "vpinball.log"
            case .ini:
                return "VPinballX.ini"
            }
        }

        var language: CodeLanguage {
            switch self {
            case .log:
                return .log
            case .ini:
                return .ini
            }
        }

        var allowsClear: Bool {
            switch self {
            case .log:
                return true
            case .ini:
                return false
            }
        }
    }

    @EnvironmentObject var vpinballViewModel: VPinballViewModel
    @EnvironmentObject var settingsModel: SettingsModel

    @Environment(\.presentationMode) var presentationMode

    @State var inputTitle: String = ""
    @State var inputValue: String = ""
    @State var inputKeyboardType: UIKeyboardType = .default
    @State var inputConfirmHandler: ((String) -> Void)? = nil
    @State var showInput: Bool = false

    @State var showContactUs = false
    @State var result: Result<MFMailComposeResult, Error>? = nil
    @State var showExport: ExportFile? = nil
    @State var showReset = false

    var focusSection: String?

    let vpinballManager = VPinballManager.shared

    var body: some View {
        NavigationView {
            ScrollViewReader { proxy in
                List {
                    Section("General") {
                        VStack(alignment: .leading) {
                            Toggle(isOn: $settingsModel.haptics) {
                                Text("Haptics")
                            }
                            .tint(Color.vpxRed)

                            Text("Provide haptic feedback when balls collide with flippers, bumpers, and slingshots.")
                                .font(.footnote)
                                .foregroundStyle(Color.secondary)
                        }

                        Toggle(isOn: $settingsModel.altColor) {
                            Text("AltColor")
                        }
                        .tint(Color.vpxRed)

                        Toggle(isOn: $settingsModel.altSound) {
                            Text("AltSound")
                        }
                        .tint(Color.vpxRed)

                        VStack(alignment: .leading) {
                            Toggle(isOn: $settingsModel.renderingModeOverride) {
                                Text("Force VR Rendering Mode")
                            }
                            .tint(Color.vpxRed)

                            Text(.init("Provide table scripts with `RenderingMode=2` so backbox and cabinet are rendered. Useful for tables that do not provide FSS support."))
                                .font(.footnote)
                                .foregroundStyle(Color.secondary)
                        }

                        VStack(alignment: .leading) {
                            Toggle(isOn: $settingsModel.liveUIOverride) {
                                Text("Mobile LiveUI")
                            }
                            .tint(Color.vpxRed)

                            Text("If disabled, use Visual Pinball's built in LiveUI.")
                                .font(.footnote)
                                .foregroundStyle(Color.secondary)
                        }
                    }

                    SettingsExternalDMDView(showInput: handleShowInput)

                    Picker("Display", selection: $settingsModel.viewMode) {
                        ForEach(VPinballViewMode.all,
                                id: \.self)
                        { viewMode in
                            Text(viewMode.name)
                                .tag(viewMode)
                        }
                    }

                    SettingsEnvironmentLightingView()

                    SettingsBallRenderingView()

                    SettingsPerformanceView()
                        .id("performance")

                    SettingsAntiAliasingView()

                    Section("Miscellaneous Features") {
                        Toggle(isOn: $settingsModel.ssreflection) {
                            Text("Additional Screen Space Reflections")
                        }
                        .tint(Color.vpxRed)
                    }

                    SettingsWebServerView(showInput: handleShowInput)

                    Section("Advanced") {
                        Toggle(isOn: $settingsModel.resetLogOnPlay) {
                            Text("Reset Log on Play")
                        }
                        .tint(Color.vpxRed)

                        Button("Export \(ExportFile.log.name)...") {
                            handleShowExport(.log)
                        }
                        .foregroundStyle(Color.vpxRed)
                    }

                    Section {
                        Button("Export \(ExportFile.ini.name)...") {
                            handleShowExport(.ini)
                        }
                        .foregroundStyle(Color.vpxRed)
                    }

                    Section("Support") {
                        Button(action: {
                            handleLink(Link.docs)
                        }, label: {
                            HStack {
                                Text("Learn More")
                                    .foregroundStyle(Color.primary)
                                Spacer()
                                Image(systemName: "chevron.right")
                                    .font(.system(size: UIFont.systemFontSize,
                                                  weight: .semibold))
                                    .foregroundStyle(Color.secondary)
                            }
                        })

                        let canSendMail = MFMailComposeViewController.canSendMail()

                        Button(action: {
                            handleContactUs()
                        }, label: {
                            HStack {
                                Text("Contact Us")
                                    .foregroundStyle(Color.primary)
                                Spacer()
                                Image(systemName: "chevron.right")
                                    .font(.system(size: UIFont.systemFontSize,
                                                  weight: .semibold))
                                    .foregroundStyle(Color.secondary)
                            }
                            .opacity(canSendMail ? 1.0 : 0.4)
                        })
                        .disabled(!canSendMail)

                        Button(action: {
                            handleLink(Link.discord)
                        }, label: {
                            HStack {
                                Text("Discord (Virtual Pinball Chat)")
                                    .foregroundStyle(Color.primary)
                                Spacer()
                                Image(systemName: "chevron.right")
                                    .font(.system(size: UIFont.systemFontSize,
                                                  weight: .semibold))
                                    .foregroundStyle(Color.secondary)
                            }
                        })
                    }

                    Section("Credits") {
                        ForEach(Credit.all, id: \.self) { credit in
                            if let link = credit.link {
                                Button(action: {
                                    handleLink(link)
                                }, label: {
                                    VStack(alignment: .leading,
                                           spacing: 10)
                                    {
                                        HStack(alignment: .center) {
                                            VStack(alignment: .leading) {
                                                Text(credit.name)
                                                    .foregroundStyle(Color.primary)
                                            }
                                            Spacer()
                                            Image(systemName: "chevron.right")
                                                .font(.system(size: UIFont.systemFontSize,
                                                              weight: .semibold))
                                                .foregroundStyle(Color.secondary)
                                        }
                                        if let authors = credit.authors {
                                            Text(authors)
                                                .font(.footnote)
                                                .foregroundStyle(Color.secondary)
                                        }
                                    }
                                })
                            } else {
                                VStack(alignment: .leading,
                                       spacing: 10)
                                {
                                    Text(credit.name)
                                        .foregroundStyle(Color.primary)

                                    if let authors = credit.authors {
                                        Text(authors)
                                            .font(.footnote)
                                            .foregroundStyle(Color.secondary)
                                    }
                                }
                            }
                        }
                    }

                    Section {
                        Button(action: {
                            handleLink(Link.licenses)
                        }, label: {
                            VStack(alignment: .leading,
                                   spacing: 10)
                            {
                                HStack(alignment: .center) {
                                    VStack(alignment: .leading) {
                                        Text("License")
                                            .foregroundStyle(Color.primary)
                                    }
                                    Spacer()
                                    Image(systemName: "chevron.right")
                                        .font(.system(size: UIFont.systemFontSize,
                                                      weight: .semibold))
                                        .foregroundStyle(Color.secondary)
                                }
                            }
                        })
                    }

                    Section {
                        Button("Reset",
                               role: .destructive)
                        {
                            handleReset()
                        }
                    }

                    Text(String(cString: VPinballGetVersionStringFull()))
                        .font(.caption)
                        .foregroundStyle(Color.secondary)
                        .multilineTextAlignment(.center)
                        .frame(maxWidth: .infinity,
                               alignment: .center)
                        .listRowBackground(Color.clear)
                }
                .navigationBarTitleDisplayMode(.large)
                .navigationBarTitle("Settings", displayMode: .large)
                .toolbar {
                    ToolbarItem(placement: .topBarTrailing) {
                        Button(action: {
                            handleDismiss()
                        }) {
                            Text("Done")
                                .bold()
                                .foregroundStyle(Color.vpxRed)
                        }
                    }
                }
                .onAppear {
                    if let focusSection = focusSection {
                        DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
                            withAnimation {
                                proxy.scrollTo(focusSection,
                                               anchor: .top)
                            }
                        }
                    }
                }
            }
        }
        .alert(inputTitle,
               isPresented: $showInput)
        {
            TextField("",
                      text: $inputValue)
                .keyboardType(inputKeyboardType)
                .autocorrectionDisabled()
                .autocapitalization(.none)
            Button("OK") {
                handleInputConfirm()
            }
            Button("Cancel",
                   role: .cancel) {}
        }
        .fullScreenCover(item: $showExport) { exportFile in
            if let url = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask).first?.appendingPathComponent(exportFile.name) {
                CodeView(url: url,
                         language: exportFile.language,
                         allowsClear: exportFile.allowsClear)
            }
        }
        .sheet(isPresented: $showContactUs) {
            MailComposeViewControllerView(result: self.$result)
                .ignoresSafeArea()
        }
        .confirmationDialog("",
                            isPresented: $showReset,
                            titleVisibility: .hidden)
        {
            Button("Reset Touch Instructions") {
                handleResetTouchInstructions()
            }

            Button("Reset All Settings",
                   role: .destructive)
            {
                handleResetAllSettings()
            }
        }
        .onChange(of: settingsModel.haptics) {
            handleHaptics()
        }
        .onChange(of: settingsModel.altColor) {
            handleAltColor()
        }
        .onChange(of: settingsModel.altSound) {
            handleAltSound()
        }
        .onChange(of: settingsModel.renderingModeOverride) {
            handleRenderingModeOverride()
        }
        .onChange(of: settingsModel.liveUIOverride) {
            handleLiveUIOverride()
        }
        .onChange(of: settingsModel.viewMode) {
            handleViewMode()
        }
        .onChange(of: settingsModel.ssreflection) {
            handleSSReflection()
        }
        .onChange(of: settingsModel.resetLogOnPlay) {
            handleResetLogOnPlay()
        }
    }

    func handleHaptics() {
        vpinballManager.saveValue(.standalone, "Haptics", settingsModel.haptics)
    }

    func handleAltColor() {
        vpinballManager.saveValue(.standalone, "AltColor", settingsModel.altColor)
    }

    func handleAltSound() {
        vpinballManager.saveValue(.standalone, "AltSound", settingsModel.altSound)
    }

    func handleRenderingModeOverride() {
        vpinballManager.saveValue(.standalone, "RenderingModeOverride", settingsModel.renderingModeOverride ? 2 : -1)
    }

    func handleLiveUIOverride() {
        vpinballManager.saveValue(.standalone, "LiveUIOverride", settingsModel.liveUIOverride)
    }

    func handleViewMode() {
        vpinballManager.saveValue(.player, "BGSet", settingsModel.viewMode.rawValue)
    }

    func handleSSReflection() {
        vpinballManager.saveValue(.player, "SSRefl", settingsModel.ssreflection)
    }

    func handleResetLogOnPlay() {
        vpinballManager.saveValue(.standalone, "ResetLogOnPlay", settingsModel.resetLogOnPlay)
    }

    func handleShowInput(title: String, value: String, keyboardType: UIKeyboardType, confirmHandler: @escaping (String) -> Void) {
        inputTitle = title
        inputValue = value
        inputKeyboardType = keyboardType
        inputConfirmHandler = confirmHandler
        showInput = true
    }

    func handleInputConfirm() {
        inputConfirmHandler?(inputValue)
    }

    func handleShowExport(_ exportFile: ExportFile) {
        showExport = exportFile
    }

    func handleContactUs() {
        showContactUs = true
    }

    func handleLink(_ link: Link) {
        link.open()
    }

    func handleReset() {
        showReset = true
    }

    func handleResetTouchInstructions() {
        vpinballManager.saveValue(.standalone, "TouchInstructions", true)
    }

    func handleResetAllSettings() {
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.25) {
            vpinballManager.resetIni()
            vpinballManager.updateWebServer()

            settingsModel.reset()
        }

        handleDismiss()
    }

    func handleDismiss() {
        presentationMode.wrappedValue.dismiss()
    }
}

#Preview {
    SettingsView()
        .environmentObject(VPinballViewModel.shared)
        .environmentObject(SettingsModel())
}
