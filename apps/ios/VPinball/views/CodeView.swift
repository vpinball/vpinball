import SwiftUI

enum CodeLanguage {
    case ini
    case log
    case vbscript

    var name: String {
        switch self {
        case .ini:
            return "ini"
        case .log:
            return "log"
        case .vbscript:
            return "visual-basic"
        }
    }

    var monacoType: String {
        switch self {
        case .ini:
            return "ini"
        case .log:
            return "plaintext"
        case .vbscript:
            return "vb"
        }
    }
}

struct CodeView: View {
    @Environment(\.presentationMode) var presentationMode

    @State var content: String = ""

    @State var showShare = false
    @State var shareSheetItems: [Any] = []

    let url: URL
    let language: CodeLanguage

    var allowsClear: Bool = false

    var body: some View {
        GeometryReader { geometry in
            NavigationStack {
                CodeWebView(language: language,
                            code: content)
                    .navigationBarTitleDisplayMode(.inline)
                    .toolbar {
                        ToolbarItem(placement: .topBarLeading) {
                            Text(url.lastPathComponent)
                                .lineLimit(1)
                                .truncationMode(.middle)
                                .frame(maxWidth: geometry.size.width * 0.6, alignment: .leading)
                        }

                        ToolbarItem(placement: .topBarTrailing) {
                            Button(action: {
                                handleDone()
                            }) {
                                Text("Done")
                                    .bold()
                            }
                            .tint(Color.vpxRed)
                        }
                        ToolbarItem(placement: .bottomBar) {
                            HStack {
                                Button(action: {
                                    handleShare()
                                }) {
                                    Image(systemName: "square.and.arrow.up")
                                }
                                .tint(Color.vpxRed)

                                Spacer()

                                if allowsClear {
                                    Button(action: {
                                        handleClear()
                                    }) {
                                        Text("Clear")
                                    }
                                    .tint(Color.vpxRed)
                                }
                            }
                        }
                    }
                    .toolbarBackground(.visible, for: .navigationBar)
                    .toolbarBackground(.visible, for: .bottomBar)
            }
            .onAppear {
                handleAppear()
            }
            .sheet(isPresented: $showShare,
                   content: {
                       ActivityViewControllerView(activityItems: $shareSheetItems,
                                                  excludedActivityTypes: [.postToFacebook])
                           .presentationDetents([.medium])
                           .presentationDragIndicator(.hidden)
                           .ignoresSafeArea()
                   })
        }
    }

    func handleAppear() {
        if let data = try? Data(contentsOf: url) {
            content = String(decoding: data, as: UTF8.self)
        }
    }

    func handleDone() {
        presentationMode.wrappedValue.dismiss()
    }

    func handleShare() {
        shareSheetItems = [url]
        showShare = true
    }

    func handleClear() {
        do {
            let fileHandle = try FileHandle(forWritingTo: url)
            try fileHandle.truncate(atOffset: 0)
            fileHandle.closeFile()

            content = ""
        } catch {}
    }
}

#Preview {
    let url = URL(fileURLWithPath: VPinballManager.shared.getPath(.preferences)).appendingPathComponent("vpinball.log")

    return CodeView(url: url,
                    language: .log,
                    allowsClear: true)
}
