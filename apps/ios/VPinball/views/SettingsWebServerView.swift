import SwiftUI

struct SettingsWebServerView: View {
    @ObservedObject var settingsModel: SettingsModel
    @ObservedObject var vpinballModel = VPinballModel.shared

    var showInput: (String, String, UIKeyboardType, @escaping (String) -> Void) -> Void

    var body: some View {
        Section {
            Toggle(isOn: $settingsModel.webServer) {
                Text("Enabled")
            }
            .tint(Color.vpxRed)

            Button(action: {
                handleShowWebServerPort()
            }) {
                HStack {
                    Text("Port")
                        .foregroundStyle(Color.primary)
                    Spacer()
                    Text(String(settingsModel.webServerPort))
                        .foregroundStyle(Color.secondary)
                        .frame(minWidth: 50,
                               alignment: .trailing)
                }
            }
        }
        header: {
            Text("Web Server")
        }
        footer: {
            if let url = vpinballModel.webServerURL, !url.isEmpty {
                Text(.init("Web Server is running and can be accessed at: \(url)."))
            } else {
                Text("Web Server is not running.")
            }
        }
        .onChange(of: settingsModel.webServer) {
            settingsModel.handleWebServer()
        }
        .onChange(of: settingsModel.webServerPort) {
            settingsModel.handleWebServerPort()
        }
    }

    func handleShowWebServerPort() {
        showInput("Web Server Port",
                  String(settingsModel.webServerPort),
                  .numberPad,
                  handleWebServerPortConfirm)
    }

    func handleWebServerPortConfirm(value: String) {
        if let webServerPort = Int(value), webServerPort >= 0, webServerPort <= 65535 {
            settingsModel.webServerPort = webServerPort
        } else {
            DispatchQueue.main.asyncAfter(deadline: .now() + 0.01) {
                handleShowWebServerPort()
            }
        }
    }
}

#Preview {
    List {
        SettingsWebServerView(settingsModel: SettingsModel()) { _, _, _, _ in }
    }
}
