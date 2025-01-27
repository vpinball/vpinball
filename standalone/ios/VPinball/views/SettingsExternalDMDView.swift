import SwiftUI

struct SettingsExternalDMDView: View {
    @EnvironmentObject var vpinballViewModel: VPinballViewModel
    @EnvironmentObject var settingsModel: SettingsModel

    var showInput: (String, String, UIKeyboardType, @escaping (String) -> Void) -> Void

    let vpinballManager = VPinballManager.shared

    var body: some View {
        Section {
            Picker("DMD Type", selection: $settingsModel.externalDMD) {
                ForEach(VPinballExternalDMD.all, id: \.self) { externalDMD in
                    Text(externalDMD.name)
                        .tag(externalDMD)
                }
            }

            if settingsModel.externalDMD != .none {
                Button(action: {
                    handleShowAddr()
                }) {
                    HStack {
                        Text("Address")
                            .foregroundStyle(Color.primary)
                        Spacer()
                        Text(settingsModel.externalDMD == .dmdServer ?
                            settingsModel.dmdServerAddr : settingsModel.zedmdWiFiAddr)
                            .foregroundStyle(Color.secondary)
                            .frame(minWidth: 50,
                                   alignment: .trailing)
                    }
                }

                if settingsModel.externalDMD == .dmdServer {
                    Button(action: {
                        handleShowPort()
                    }) {
                        HStack {
                            Text("Port")
                                .foregroundStyle(Color.primary)
                            Spacer()
                            Text(String(settingsModel.dmdServerPort))
                                .foregroundStyle(Color.secondary)
                                .frame(minWidth: 50,
                                       alignment: .trailing)
                        }
                    }
                }
            }
        }
        header: {
            Text("External DMD")
        }
        footer: {
            Text(.init("Send PinMAME and FlexDMD DMDs to ZeDMD and Pixelcade devices using [DMDServer](\(Link.libdmdutil.url.absoluteString)) and [ZeDMDOS](\(Link.zedmdos.url.absoluteString))."))
        }
        .onChange(of: settingsModel.externalDMD) {
            handleExternalDMD()
        }
        .onChange(of: [settingsModel.dmdServerAddr,
                       settingsModel.zedmdWiFiAddr])
        {
            handleAddr()
        }
        .onChange(of: settingsModel.dmdServerPort) {
            handlePort()
        }
    }

    func handleExternalDMD() {
        vpinballManager.saveValue(.standalone, "DMDServer", settingsModel.externalDMD == .dmdServer)
        vpinballManager.saveValue(.standalone, "ZeDMDWiFi", settingsModel.externalDMD == .zedmdWiFi)
    }

    func handleShowAddr() {
        switch settingsModel.externalDMD {
        case .dmdServer:
            showInput("DMDServer Address",
                      settingsModel.dmdServerAddr,
                      .asciiCapable,
                      handleAddrConfirm)
        case .zedmdWiFi:
            showInput("ZeDMD WiFi Address",
                      settingsModel.zedmdWiFiAddr,
                      .asciiCapable,
                      handleAddrConfirm)
        default:
            break
        }
    }

    func handleAddrConfirm(value: String) {
        var sin = sockaddr_in()
        let ipv4Valid = inet_pton(AF_INET, value.cString(using: .ascii), &sin.sin_addr) == 1

        let domainRegex = "^[a-zA-Z0-9.-]{1,253}$"
        let domainValid = value.range(of: domainRegex, options: .regularExpression) != nil

        if ipv4Valid || domainValid {
            switch settingsModel.externalDMD {
            case .dmdServer:
                settingsModel.dmdServerAddr = value
            case .zedmdWiFi:
                settingsModel.zedmdWiFiAddr = value
            default:
                break
            }
        } else {
            DispatchQueue.main.asyncAfter(deadline: .now() + 0.01) {
                handleShowAddr()
            }
        }
    }

    func handleAddr() {
        switch settingsModel.externalDMD {
        case .dmdServer:
            vpinballManager.saveValue(.standalone, "DMDServerAddr", settingsModel.dmdServerAddr)
        case .zedmdWiFi:
            vpinballManager.saveValue(.standalone, "ZeDMDWiFiAddr", settingsModel.zedmdWiFiAddr)
        default:
            break
        }
    }

    func handleShowPort() {
        switch settingsModel.externalDMD {
        case .dmdServer:
            showInput("DMDServer Port",
                      String(settingsModel.dmdServerPort),
                      .numberPad,
                      handlePortConfirm)
        default:
            break
        }
    }

    func handlePortConfirm(value: String) {
        if let port = Int(value), port >= 0 && port <= 65535 {
            switch settingsModel.externalDMD {
            case .dmdServer:
                settingsModel.dmdServerPort = port
            default:
                break
            }
        } else {
            DispatchQueue.main.asyncAfter(deadline: .now() + 0.01) {
                handleShowPort()
            }
        }
    }

    func handlePort() {
        switch settingsModel.externalDMD {
        case .dmdServer:
            vpinballManager.saveValue(.standalone, "DMDServerPort", settingsModel.dmdServerPort)
        default:
            break
        }
    }
}

#Preview {
    List {
        SettingsView()
    }
    .environmentObject(VPinballViewModel.shared)
    .environmentObject(SettingsModel())
}
