import MessageUI
import SwiftUI

struct SettingsView: View {
    @ObservedObject var settingsModel: SettingsModel

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

    var body: some View {
        NavigationStack {
            List {
                Section("Appearance") {
                    Picker("Mode", selection: $settingsModel.appearance) {
                        ForEach(AppAppearance.allCases) { appearance in
                            Text(appearance.title).tag(appearance)
                        }
                    }
                }

                SettingsExternalDMDView(settingsModel: settingsModel, showInput: handleShowInput)

                SettingsWebServerView(settingsModel: settingsModel, showInput: handleShowInput)

                Section("Miscellaneous") {
                    VStack(alignment: .leading, spacing: 8) {
                        Toggle(isOn: $settingsModel.renderingModeOverride) {
                            Text("Force VR Rendering Mode")
                        }
                        .tint(Color.vpxRed)

                        Text(.init("Provide table scripts with `RenderingMode=2` so backbox and cabinet are rendered. Useful for tables that do not provide FSS support."))
                            .font(.footnote)
                            .foregroundStyle(Color.secondary)
                    }
                    .padding(.vertical, 6)
                }

                Section("Advanced") {
                    Button("Export \(ExportFile.log.name)...") {
                        handleShowExport(.log)
                    }
                    .tint(Color.vpxRed)
                }

                Section {
                    Button("Export \(ExportFile.ini.name)...") {
                        handleShowExport(.ini)
                    }
                    .tint(Color.vpxRed)
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

                VStack(spacing: 4) {
                    Button(action: {
                        handleLink(Link.vpinball)
                    }, label: {
                        HStack(spacing: 4) {
                            Text(BuildInfo.provenance)
                            Image(systemName: "arrow.up.right.square")
                        }
                    })
                    .buttonStyle(.plain)

                    Text(BuildInfo.version)
                }
                .font(.caption)
                .foregroundStyle(Color.secondary)
                .multilineTextAlignment(.center)
                .frame(maxWidth: .infinity,
                       alignment: .center)
                .listRowBackground(Color.clear)
            }
            .navigationTitle("Settings")
            .navigationBarTitleDisplayMode(.large)
            .toolbar {
                ToolbarItem(placement: .topBarTrailing) {
                    Button(role: .close) {
                        handleDismiss()
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
            let url = URL(fileURLWithPath: VPinballManager.shared.getPath(.preferences)).appendingPathComponent(exportFile.name)
            CodeView(url: url,
                     language: exportFile.language,
                     allowsClear: exportFile.allowsClear)
        }
        .sheet(isPresented: $showContactUs) {
            MailComposeViewControllerView(result: self.$result)
                .ignoresSafeArea()
        }
        .confirmationDialog("",
                            isPresented: $showReset,
                            titleVisibility: .hidden)
        {
            Button("Reset All Settings",
                   role: .destructive)
            {
                handleResetAllSettings()
            }
        }
        .onChange(of: settingsModel.appearance) {
            settingsModel.handleAppearance()
        }
        .onChange(of: settingsModel.renderingModeOverride) {
            settingsModel.handleRenderingModeOverride()
        }
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

    func handleResetAllSettings() {
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.25) {
            VPinballManager.shared.resetIni()
            VPinballManager.shared.updateWebServer()

            settingsModel.reset()
        }

        handleDismiss()
    }

    func handleDismiss() {
        presentationMode.wrappedValue.dismiss()
    }
}

#Preview {
    SettingsView(settingsModel: SettingsModel())
}
