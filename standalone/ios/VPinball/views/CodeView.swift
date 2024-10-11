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
        NavigationStack {
            Group {
                if !content.isEmpty {
                    CodeWebView(language: language,
                                code: content)

                } else {
                    Text("No lines")
                }
            }
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .topBarLeading) {
                    Text(url.lastPathComponent)
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
                        .disabled(content.isEmpty)

                        Spacer()

                        if allowsClear {
                            Button(action: {
                                handleClear()
                            }) {
                                Text("Clear")
                            }
                            .tint(Color.vpxRed)
                            .disabled(content.isEmpty)
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

    func handleAppear() {
        URLSession.shared.dataTask(with: url) { data, _, _ in
            if let data = data, let content = String(data: data,
                                                     encoding: .utf8)
            {
                self.content = content
            }
        }.resume()
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
    let documentsDirectory = FileManager.default.urls(for: .documentDirectory,
                                                      in: .userDomainMask).first!

    let url = documentsDirectory.appendingPathComponent("vpinball.log")

    return CodeView(url: url,
                    language: .log,
                    allowsClear: true)
}
