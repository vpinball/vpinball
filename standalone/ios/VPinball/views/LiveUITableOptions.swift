import SwiftUI

struct LiveUITableOptions: View {
    @EnvironmentObject var vpinballViewModel: VPinballViewModel

    @State var customTableOptions: [VPinballCustomTableOption] = []
    @State var refreshCustomTableOptions = UUID()
    @State var globalEmissionScale: Float = 0.0
    @State var globalDifficulty: Float = 0.0
    @State var exposure: Float = 0.0
    @State var toneMapper: VPinballToneMapper = .reinhard
    @State var musicVolume: Float = 0.0
    @State var soundVolume: Float = 0.0

    @State var activeSlider: String? = nil
    @State var isUpdating: Bool = false

    @Binding var isEditing: Bool

    let vpinballManager = VPinballManager.shared
    let incSpeed: Float = 0.0375

    var body: some View {
        VStack {
            Text("Table Options")
                .bold()
                .foregroundStyle(Color.white)
                .opacity(activeSlider == nil ? 1 : 0)

            ScrollView {
                VStack(spacing: 0) {
                    ForEach(customTableOptions.indices, id: \.self) { index in
                        CustomTableOptionView(customTableOption: $customTableOptions[index])
                    }
                    .id(refreshCustomTableOptions)

                    createSlider(
                        label: "Day Night",
                        value: $globalEmissionScale,
                        minValue: 0,
                        maxValue: 1,
                        step: 0.05 * incSpeed,
                        format: .scaledPercentage,
                        activeSlider: $activeSlider
                    )

                    createSlider(
                        label: "Difficulty",
                        value: $globalDifficulty,
                        minValue: 0,
                        maxValue: 1,
                        step: 0.05 * incSpeed,
                        format: .scaledPercentage,
                        activeSlider: $activeSlider
                    )

                    createSlider(
                        label: "Exposure",
                        value: $exposure,
                        minValue: 0,
                        maxValue: 2,
                        step: 0.05 * incSpeed,
                        format: .scaledPercentage,
                        activeSlider: $activeSlider
                    )

                    HStack {
                        Text("Tonemapper")
                            .bold()
                            .foregroundStyle(Color.white)
                            .padding(.leading, 10)

                        Spacer()

                        Picker("Tonemapper", selection: $toneMapper) {
                            ForEach(VPinballToneMapper.all,
                                    id: \.self)
                            { toneMapper in
                                Text(toneMapper.name)
                                    .tag(toneMapper)
                            }
                        }
                        .tint(Color.white)
                    }
                    .opacity(activeSlider == nil ? 1 : 0)
                    .padding(.top, 10)
                    .padding(.bottom, 20)

                    createSlider(
                        label: "Music Volume",
                        value: $musicVolume,
                        minValue: 0,
                        maxValue: 100,
                        step: 1,
                        format: .intPercentage,
                        activeSlider: $activeSlider
                    )

                    createSlider(
                        label: "Sound Volume",
                        value: $soundVolume,
                        minValue: 0,
                        maxValue: 100,
                        step: 1,
                        format: .intPercentage,
                        activeSlider: $activeSlider
                    )
                }
            }

            TouchDownInterceptingContainer {
                HStack(alignment: .top) {
                    Button(action: {
                        handleSave()
                    }, label: {
                        Text("Save")
                            .bold()
                            .foregroundStyle(Color.white)
                    })
                    .padding(.horizontal, 20)
                    .padding(.vertical, 10)
                    .background(.ultraThinMaterial, in:
                        RoundedRectangle(cornerRadius: 10.0))

                    Spacer()

                    Menu(content: {
                        Section("Reset Table Options to...") {
                            Button(action: {
                                handleReset()
                            }) {
                                Text("Table's settings")
                            }

                            Button(action: {
                                handleDefaults()
                            }) {
                                Text("System defaults")
                            }
                            .disabled(true)
                        }
                    }, label: {
                        Text("Reset")
                            .bold()
                            .foregroundStyle(Color.white)
                            .padding(.horizontal, 20)
                            .padding(.vertical, 10)
                            .background(.ultraThinMaterial, in:
                                RoundedRectangle(cornerRadius: 10.0))
                    })
                }
            }
            .opacity(!isEditing ? 1 : 0)
            .padding(10)
        }
        .padding(.bottom, 40)
        .onAppear {
            handleRefresh()
        }
        .onChange(of: activeSlider) {
            withAnimation(.easeInOut(duration: 0.2)) {
                isEditing = activeSlider != nil
            }
        }
        .onChange(of: [globalEmissionScale,
                       globalDifficulty,
                       exposure,
                       musicVolume,
                       soundVolume])
        {
            handleUpdate()
        }
        .onChange(of: toneMapper) {
            handleUpdate()
        }
    }

    func createSlider(
        label: String,
        value: Binding<Float>,
        minValue: Float,
        maxValue: Float,
        step: Float,
        format: FormattedActiveSliderFormat,
        activeSlider: Binding<String?>
    ) -> some View {
        ZStack {
            RoundedRectangle(cornerRadius: 10)
                .fill(Color.black.opacity(0.5))
                .padding(-10)
                .opacity(activeSlider.wrappedValue == label ? 1 : 0)

            FormattedActiveSlider(
                label: label,
                value: value,
                minValue: minValue,
                maxValue: maxValue,
                step: step,
                format: format,
                activeSlider: activeSlider
            )
        }
        .padding(10)
        .opacity(activeSlider.wrappedValue == nil
            || activeSlider.wrappedValue == label ? 1 : 0)
    }

    func handleRefresh() {
        isUpdating = true

        let tableOptions = vpinballManager.getTableOptions()
        globalEmissionScale = tableOptions.globalEmissionScale
        globalDifficulty = tableOptions.globalDifficulty
        exposure = tableOptions.exposure
        toneMapper = VPinballToneMapper(rawValue: tableOptions.toneMapper)!
        musicVolume = Float(tableOptions.musicVolume)
        soundVolume = Float(tableOptions.soundVolume)

        customTableOptions = vpinballManager.getCustomTableOptions()
        refreshCustomTableOptions = UUID()

        DispatchQueue.main.asyncAfter(deadline: .now() + 0.01) {
            isUpdating = false
        }
    }

    func handleUpdate() {
        if isUpdating {
            return
        }

        let tableOptions = VPinballTableOptions(globalEmissionScale: Float(globalEmissionScale),
                                                globalDifficulty: Float(globalDifficulty),
                                                exposure: Float(exposure),
                                                toneMapper: toneMapper.rawValue,
                                                musicVolume: CInt(musicVolume),
                                                soundVolume: CInt(soundVolume))

        vpinballManager.setTableOptions(tableOptions)
    }

    func handleSave() {
        vpinballManager.saveTableOptions()
        vpinballManager.saveCustomTableOptions()

        withAnimation {
            vpinballViewModel.showStatusHUD(icon: "noun-pinball-3169564",
                                            title: "Table Options\nSaved !")
        }

        DispatchQueue.main.asyncAfter(deadline: .now() + 2) {
            withAnimation {
                vpinballViewModel.hideHUD()
            }
        }
    }

    func handleDefaults() {
        vpinballManager.setDefaultTableOptions()
        vpinballManager.setDefaultCustomTableOptions()

        DispatchQueue.main.asyncAfter(deadline: .now() + 0.25) {
            handleRefresh()
        }
    }

    func handleReset() {
        vpinballManager.resetTableOptions()
        vpinballManager.resetCustomTableOptions()

        DispatchQueue.main.asyncAfter(deadline: .now() + 0.25) {
            handleRefresh()
        }
    }
}
