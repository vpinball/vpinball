import SwiftUI

struct LiveUIPointOfView: View {
    @EnvironmentObject var vpinballViewModel: VPinballViewModel

    @State var viewMode: VPinballViewLayoutMode = .camera
    @State var fov: Float = 0.0
    @State var layback: Float = 0.0
    @State var viewHOfs: Float = 0.0
    @State var viewVOfs: Float = 0.0
    @State var sceneScaleX: Float = 0.0
    @State var sceneScaleY: Float = 0.0
    @State var sceneScaleZ: Float = 0.0
    @State var lookAt: Float = 0.0
    @State var viewX: Float = 0.0
    @State var viewY: Float = 0.0
    @State var viewZ: Float = 0.0
    @State var windowTopZOfs: Float = 0.0
    @State var windowBottomZOfs: Float = 0.0
    @State var viewportRotation: Float = 0.0

    @State var activeSlider: String? = nil
    @State var isScaleLocked: Bool = true
    @State var isUpdating: Bool = false

    @Binding var isEditing: Bool

    let vpinballManager = VPinballManager.shared
    let incSpeed: Float = 0.0375

    var body: some View {
        ZStack {
            VStack {
                Text("Point of View")
                    .bold()
                    .foregroundStyle(Color.white)
                    .opacity(activeSlider == nil ? 1 : 0)

                ScrollView {
                    VStack(spacing: 0) {
                        HStack {
                            Text("View Mode Layout")
                                .bold()
                                .foregroundStyle(Color.white)
                                .padding(.leading, 10)

                            Spacer()
                            Picker("", selection: $viewMode) {
                                ForEach(VPinballViewLayoutMode.all,
                                        id: \.self)
                                { viewMode in
                                    Text(viewMode.name)
                                        .tag(viewMode)
                                }
                            }
                            .tint(Color.white)
                        }
                        .opacity(activeSlider == nil ? 1 : 0)
                        .padding(.top, 10)
                        .padding(.bottom, 20)

                        if viewMode == .legacy {
                            /*
                             m_tweakPageOptions.push_back(BS_ViewMode);
                             m_tweakPageOptions.push_back(BS_LookAt);
                             m_tweakPageOptions.push_back(BS_FOV);
                             m_tweakPageOptions.push_back(BS_Layback);
                             m_tweakPageOptions.push_back(BS_XYZScale);
                             m_tweakPageOptions.push_back(BS_XScale);
                             m_tweakPageOptions.push_back(BS_YScale);
                             m_tweakPageOptions.push_back(BS_ZScale);
                             m_tweakPageOptions.push_back(BS_XOffset);
                             m_tweakPageOptions.push_back(BS_YOffset);
                             m_tweakPageOptions.push_back(BS_ZOffset);
                             */

                            createSlider(
                                label: "Inclination",
                                value: $lookAt,
                                minValue: -50,
                                maxValue: 80,
                                step: incSpeed,
                                format: .degrees,
                                activeSlider: $activeSlider
                            )

                            createSlider(
                                label: "Field Of View (overall scale)",
                                value: $fov,
                                minValue: 1,
                                maxValue: 175,
                                step: incSpeed,
                                format: .degrees,
                                activeSlider: $activeSlider
                            )

                            createSlider(
                                label: "Layback",
                                value: $layback,
                                minValue: -100,
                                maxValue: 100,
                                step: incSpeed,
                                format: .decimal,
                                activeSlider: $activeSlider
                            )

                            ZStack {
                                RoundedRectangle(cornerRadius: 10)
                                    .fill(Color.black.opacity(0.5))
                                    .padding(-10)
                                    .opacity(isScaleLocked &&
                                        (activeSlider == "Table X Scale" ||
                                            activeSlider == "Table Y Scale" ||
                                            activeSlider == "Table Z Scale") ? 1 : 0)

                                HStack(spacing: 10) {
                                    Button(action: {
                                        isScaleLocked.toggle()
                                    }) {
                                        Image(systemName: isScaleLocked ? "lock" : "lock.slash")
                                            .resizable()
                                            .aspectRatio(contentMode: .fit)
                                            .frame(width: 20,
                                                   height: 20)
                                            .foregroundStyle(Color.white)
                                    }
                                    .opacity(activeSlider == nil ||
                                        (isScaleLocked &&
                                            (activeSlider == "Table X Scale" ||
                                                activeSlider == "Table Y Scale" ||
                                                activeSlider == "Table Z Scale")) ? 1 : 0)

                                    if isScaleLocked {
                                        VStack(spacing: 20) {
                                            createLockSlider(
                                                label: "Table X Scale",
                                                value: $sceneScaleX,
                                                minValue: 0.1,
                                                maxValue: 2,
                                                step: 0.005 * incSpeed,
                                                format: .scaledPercentage,
                                                activeSlider: $activeSlider,
                                                otherLabels: ["Table Y Scale",
                                                              "Table Z Scale"]
                                            )

                                            createLockSlider(
                                                label: "Table Y Scale",
                                                value: $sceneScaleY,
                                                minValue: 0.1,
                                                maxValue: 2,
                                                step: 0.005 * incSpeed,
                                                format: .scaledPercentage,
                                                activeSlider: $activeSlider,
                                                otherLabels: ["Table X Scale",
                                                              "Table Z Scale"]
                                            )

                                            createLockSlider(
                                                label: "Table Z Scale",
                                                value: $sceneScaleZ,
                                                minValue: 0.1,
                                                maxValue: 2,
                                                step: 0.005 * incSpeed,
                                                format: .scaledPercentage,
                                                activeSlider: $activeSlider,
                                                otherLabels: ["Table X Scale",
                                                              "Table Y Scale"]
                                            )
                                        }
                                    } else {
                                        VStack(spacing: 0) {
                                            createSlider(
                                                label: "Table X Scale",
                                                value: $sceneScaleX,
                                                minValue: 0.1,
                                                maxValue: 2,
                                                step: 0.005 * incSpeed,
                                                format: .scaledPercentage,
                                                activeSlider: $activeSlider
                                            )

                                            createSlider(
                                                label: "Table Y Scale",
                                                value: $sceneScaleY,
                                                minValue: 0.1,
                                                maxValue: 2,
                                                step: 0.005 * incSpeed,
                                                format: .scaledPercentage,
                                                activeSlider: $activeSlider
                                            )

                                            createSlider(
                                                label: "Table Z Scale",
                                                value: $sceneScaleZ,
                                                minValue: 0.1,
                                                maxValue: 2,
                                                step: 0.005 * incSpeed,
                                                format: .scaledPercentage,
                                                activeSlider: $activeSlider
                                            )
                                        }
                                        .padding(-10)
                                    }
                                }
                            }
                            .padding(10)
                            .opacity(activeSlider == nil ||
                                activeSlider == "Table X Scale" ||
                                activeSlider == "Table Y Scale" ||
                                activeSlider == "Table Z Scale" ? 1 : 0)

                            createSlider(
                                label: "X Offset",
                                value: $viewX,
                                minValue: VPinballUnitConverter.cmToVPU(-70),
                                maxValue: VPinballUnitConverter.cmToVPU(70),
                                step: 10 * incSpeed,
                                format: .vpuCM,
                                activeSlider: $activeSlider
                            )

                            createSlider(
                                label: "Y Offset",
                                value: $viewY,
                                minValue: VPinballUnitConverter.cmToVPU(-70),
                                maxValue: VPinballUnitConverter.cmToVPU(400),
                                step: 10 * incSpeed,
                                format: .vpuCM,
                                activeSlider: $activeSlider
                            )

                            createSlider(
                                label: "Z Offset",
                                value: $viewZ,
                                minValue: VPinballUnitConverter.cmToVPU(0),
                                maxValue: VPinballUnitConverter.cmToVPU(200),
                                step: 100 * incSpeed,
                                format: .vpuCM,
                                activeSlider: $activeSlider
                            )
                        } else if viewMode == .camera {
                            /*
                             m_tweakPageOptions.push_back(BS_ViewMode);
                             m_tweakPageOptions.push_back(BS_FOV);
                             m_tweakPageOptions.push_back(BS_ViewHOfs);
                             m_tweakPageOptions.push_back(BS_ViewVOfs);
                             m_tweakPageOptions.push_back(BS_XYZScale);
                             m_tweakPageOptions.push_back(BS_XScale);
                             m_tweakPageOptions.push_back(BS_YScale);
                             m_tweakPageOptions.push_back(BS_ZScale);
                             m_tweakPageOptions.push_back(BS_LookAt);
                             m_tweakPageOptions.push_back(BS_XOffset);
                             m_tweakPageOptions.push_back(BS_YOffset);
                             m_tweakPageOptions.push_back(BS_ZOffset);
                             */

                            createSlider(
                                label: "Field Of View (overall scale)",
                                value: $fov,
                                minValue: 1,
                                maxValue: 175,
                                step: incSpeed,
                                format: .degrees,
                                activeSlider: $activeSlider
                            )

                            createSlider(
                                label: "Horizontal Offset",
                                value: $viewHOfs,
                                minValue: -100,
                                maxValue: 100,
                                step: incSpeed,
                                format: .decimal,
                                activeSlider: $activeSlider
                            )

                            createSlider(
                                label: "Vertical Offset",
                                value: $viewVOfs,
                                minValue: -150,
                                maxValue: 150,
                                step: incSpeed,
                                format: .decimal,
                                activeSlider: $activeSlider
                            )

                            ZStack {
                                RoundedRectangle(cornerRadius: 10)
                                    .fill(Color.black.opacity(0.5))
                                    .padding(-10)
                                    .opacity(isScaleLocked &&
                                        (activeSlider == "Table X Scale" ||
                                            activeSlider == "Table Y Scale" ||
                                            activeSlider == "Table Z Scale") ? 1 : 0)

                                HStack(spacing: 10) {
                                    Button(action: {
                                        isScaleLocked.toggle()
                                    }) {
                                        Image(systemName: isScaleLocked ? "lock" : "lock.slash")
                                            .resizable()
                                            .aspectRatio(contentMode: .fit)
                                            .frame(width: 20,
                                                   height: 20)
                                            .foregroundStyle(Color.white)
                                    }
                                    .opacity(activeSlider == nil ||
                                        (isScaleLocked &&
                                            (activeSlider == "Table X Scale" ||
                                                activeSlider == "Table Y Scale" ||
                                                activeSlider == "Table Z Scale")) ? 1 : 0)

                                    if isScaleLocked {
                                        VStack(spacing: 20) {
                                            createLockSlider(
                                                label: "Table X Scale",
                                                value: $sceneScaleX,
                                                minValue: 0.1,
                                                maxValue: 2,
                                                step: 0.005 * incSpeed,
                                                format: .scaledPercentage,
                                                activeSlider: $activeSlider,
                                                otherLabels: ["Table Y Scale",
                                                              "Table Z Scale"]
                                            )

                                            createLockSlider(
                                                label: "Table Y Scale",
                                                value: $sceneScaleY,
                                                minValue: 0.1,
                                                maxValue: 2,
                                                step: 0.005 * incSpeed,
                                                format: .scaledPercentage,
                                                activeSlider: $activeSlider,
                                                otherLabels: ["Table X Scale",
                                                              "Table Z Scale"]
                                            )

                                            createLockSlider(
                                                label: "Table Z Scale",
                                                value: $sceneScaleZ,
                                                minValue: 0.1,
                                                maxValue: 2,
                                                step: 0.005 * incSpeed,
                                                format: .scaledPercentage,
                                                activeSlider: $activeSlider,
                                                otherLabels: ["Table X Scale",
                                                              "Table Y Scale"]
                                            )
                                        }
                                    } else {
                                        VStack(spacing: 0) {
                                            createSlider(
                                                label: "Table X Scale",
                                                value: $sceneScaleX,
                                                minValue: 0.1,
                                                maxValue: 2,
                                                step: 0.005 * incSpeed,
                                                format: .scaledPercentage,
                                                activeSlider: $activeSlider
                                            )

                                            createSlider(
                                                label: "Table Y Scale",
                                                value: $sceneScaleY,
                                                minValue: 0.1,
                                                maxValue: 2,
                                                step: 0.005 * incSpeed,
                                                format: .scaledPercentage,
                                                activeSlider: $activeSlider
                                            )

                                            createSlider(
                                                label: "Table Z Scale",
                                                value: $sceneScaleZ,
                                                minValue: 0.1,
                                                maxValue: 2,
                                                step: 0.005 * incSpeed,
                                                format: .scaledPercentage,
                                                activeSlider: $activeSlider
                                            )
                                        }
                                        .padding(-10)
                                    }
                                }
                            }
                            .padding(10)
                            .opacity(activeSlider == nil ||
                                activeSlider == "Table X Scale" ||
                                activeSlider == "Table Y Scale" ||
                                activeSlider == "Table Z Scale" ? 1 : 0)

                            createSlider(
                                label: "Look At",
                                value: $lookAt,
                                minValue: -20,
                                maxValue: 200,
                                step: incSpeed,
                                format: .percentage,
                                activeSlider: $activeSlider
                            )

                            createSlider(
                                label: "Camera X",
                                value: $viewX,
                                minValue: VPinballUnitConverter.cmToVPU(-70),
                                maxValue: VPinballUnitConverter.cmToVPU(70),
                                step: 10 * incSpeed,
                                format: .vpuCM,
                                activeSlider: $activeSlider
                            )

                            createSlider(
                                label: "Camera Y",
                                value: $viewY,
                                minValue: VPinballUnitConverter.cmToVPU(-70),
                                maxValue: VPinballUnitConverter.cmToVPU(400),
                                step: 10 * incSpeed,
                                format: .vpuCM,
                                activeSlider: $activeSlider
                            )

                            createSlider(
                                label: "Camera Z",
                                value: $viewZ,
                                minValue: VPinballUnitConverter.cmToVPU(0),
                                maxValue: VPinballUnitConverter.cmToVPU(200),
                                step: 10 * incSpeed,
                                format: .vpuCM,
                                activeSlider: $activeSlider
                            )
                        } else if viewMode == .window {
                            /*
                             m_tweakPageOptions.push_back(BS_ViewMode);
                             m_tweakPageOptions.push_back(BS_ViewHOfs);
                             m_tweakPageOptions.push_back(BS_ViewVOfs);
                             m_tweakPageOptions.push_back(BS_XYZScale);
                             m_tweakPageOptions.push_back(BS_XScale);
                             m_tweakPageOptions.push_back(BS_YScale);
                             m_tweakPageOptions.push_back(BS_WndTopZOfs);
                             m_tweakPageOptions.push_back(BS_WndBottomZOfs);
                             m_tweakPageOptions.push_back(BS_XOffset);
                             m_tweakPageOptions.push_back(BS_YOffset);
                             m_tweakPageOptions.push_back(BS_ZOffset);
                             */

                            createSlider(
                                label: "Horizontal Offset",
                                value: $viewHOfs,
                                minValue: -100,
                                maxValue: 100,
                                step: incSpeed,
                                format: .cm,
                                activeSlider: $activeSlider
                            )

                            createSlider(
                                label: "Vertical Offset",
                                value: $viewVOfs,
                                minValue: -150,
                                maxValue: 150,
                                step: incSpeed,
                                format: .cm,
                                activeSlider: $activeSlider
                            )

                            ZStack {
                                RoundedRectangle(cornerRadius: 10)
                                    .fill(Color.black.opacity(0.5))
                                    .padding(-10)
                                    .opacity(isScaleLocked &&
                                        (activeSlider == "Table X Scale" ||
                                            activeSlider == "Table YZ Scale") ? 1 : 0)

                                HStack(spacing: 10) {
                                    Button(action: {
                                        isScaleLocked.toggle()
                                    }) {
                                        Image(systemName: isScaleLocked ? "lock" : "lock.slash")
                                            .resizable()
                                            .aspectRatio(contentMode: .fit)
                                            .frame(width: 20,
                                                   height: 20)
                                            .foregroundStyle(Color.white)
                                    }
                                    .opacity(activeSlider == nil ||
                                        (isScaleLocked &&
                                            (activeSlider == "Table X Scale" ||
                                                activeSlider == "Table YZ Scale")) ? 1 : 0)

                                    if isScaleLocked {
                                        VStack(spacing: 20) {
                                            createLockSlider(
                                                label: "Table X Scale",
                                                value: $sceneScaleX,
                                                minValue: 0.1,
                                                maxValue: 2,
                                                step: 0.005 * incSpeed,
                                                format: .scaledPercentage,
                                                activeSlider: $activeSlider,
                                                otherLabels: ["Table YZ Scale"]
                                            )

                                            createLockSlider(
                                                label: "Table YZ Scale",
                                                value: $sceneScaleY,
                                                minValue: 0.1,
                                                maxValue: 2,
                                                step: 0.005 * incSpeed,
                                                format: .scaledPercentage,
                                                activeSlider: $activeSlider,
                                                otherLabels: ["Table X Scale"]
                                            )
                                        }
                                    } else {
                                        VStack(spacing: 0) {
                                            createSlider(
                                                label: "Table X Scale",
                                                value: $sceneScaleX,
                                                minValue: 0.1,
                                                maxValue: 2,
                                                step: 0.005 * incSpeed,
                                                format: .scaledPercentage,
                                                activeSlider: $activeSlider
                                            )

                                            createSlider(
                                                label: "Table YZ Scale",
                                                value: $sceneScaleY,
                                                minValue: 0.1,
                                                maxValue: 2,
                                                step: 0.005 * incSpeed,
                                                format: .scaledPercentage,
                                                activeSlider: $activeSlider
                                            )
                                        }
                                        .padding(-10)
                                    }
                                }
                            }
                            .padding(10)
                            .opacity(activeSlider == nil ||
                                activeSlider == "Table X Scale" ||
                                activeSlider == "Table YZ Scale" ? 1 : 0)

                            createSlider(
                                label: "Window Top Z Ofs.",
                                value: $windowTopZOfs,
                                minValue: VPinballUnitConverter.cmToVPU(-70),
                                maxValue: VPinballUnitConverter.cmToVPU(70),
                                step: 10 * incSpeed,
                                format: .vpuCM,
                                activeSlider: $activeSlider
                            )

                            createSlider(
                                label: "Window Bottom Z Ofs.",
                                value: $windowBottomZOfs,
                                minValue: VPinballUnitConverter.cmToVPU(-70),
                                maxValue: VPinballUnitConverter.cmToVPU(70),
                                step: 10 * incSpeed,
                                format: .vpuCM,
                                activeSlider: $activeSlider
                            )

                            createSlider(
                                label: "Player X",
                                value: $viewX,
                                minValue: VPinballUnitConverter.cmToVPU(-70),
                                maxValue: VPinballUnitConverter.cmToVPU(70),
                                step: 10 * incSpeed,
                                format: .vpuCM,
                                activeSlider: $activeSlider
                            )

                            createSlider(
                                label: "Player Y",
                                value: $viewY,
                                minValue: VPinballUnitConverter.cmToVPU(-70),
                                maxValue: VPinballUnitConverter.cmToVPU(400),
                                step: 10 * incSpeed,
                                format: .vpuCM,
                                activeSlider: $activeSlider
                            )

                            createSlider(
                                label: "Player Z",
                                value: $viewZ,
                                minValue: VPinballUnitConverter.cmToVPU(0),
                                maxValue: VPinballUnitConverter.cmToVPU(200),
                                step: 10 * incSpeed,
                                format: .vpuCM,
                                activeSlider: $activeSlider
                            )
                        }
                    }

                    /*
                     createSlider(
                     label: "Viewport Rotation",
                     value: $viewportRotation,
                     minValue: 0,
                     maxValue: 359,
                     step: 0.1,
                     format: .degrees,
                     activeSlider: $activeSlider
                     )
                     */
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
                            Section("Reset Point of View to...") {
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
        }
        .onAppear {
            handleRefresh()
        }
        .onChange(of: activeSlider) {
            withAnimation(.easeInOut(duration: 0.2)) {
                isEditing = activeSlider != nil
            }
        }
        .onChange(of: viewMode) {
            handleUpdate()
        }
        .onChange(of: [lookAt,
                       fov,
                       layback,
                       viewHOfs,
                       viewVOfs,
                       windowTopZOfs,
                       windowBottomZOfs,
                       viewX,
                       viewY,
                       viewZ,
                       viewportRotation])
        {
            handleUpdate()
        }
        .onChange(of: sceneScaleX) { oldValue, newValue in
            if !isScaleLocked {
                handleUpdate()
                return
            }
            if isUpdating {
                return
            }
            isUpdating = true
            let diff = newValue - oldValue
            if sceneScaleY + diff < 0 || sceneScaleZ + diff < 0 {
                sceneScaleX = oldValue
            } else {
                sceneScaleY += diff
                sceneScaleZ += diff
            }
            DispatchQueue.main.asyncAfter(deadline: .now() + 0.01) {
                isUpdating = false
                handleUpdate()
            }
        }
        .onChange(of: sceneScaleY) { oldValue, newValue in
            if !isScaleLocked {
                handleUpdate()
                return
            }
            if isUpdating {
                return
            }
            isUpdating = true
            let diff = newValue - oldValue
            if sceneScaleX + diff < 0 || sceneScaleZ + diff < 0 {
                sceneScaleY = oldValue
            } else {
                sceneScaleX += diff
                sceneScaleZ += diff
            }
            DispatchQueue.main.asyncAfter(deadline: .now() + 0.01) {
                isUpdating = false
                handleUpdate()
            }
        }
        .onChange(of: sceneScaleZ) { oldValue, newValue in
            if !isScaleLocked {
                handleUpdate()
                return
            }
            if isUpdating {
                return
            }
            isUpdating = true
            let diff = newValue - oldValue
            if sceneScaleX + diff < 0 || sceneScaleY + diff < 0 {
                sceneScaleZ = oldValue
            } else {
                sceneScaleX += diff
                sceneScaleY += diff
            }
            DispatchQueue.main.asyncAfter(deadline: .now() + 0.01) {
                isUpdating = false
                handleUpdate()
            }
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

    func createLockSlider(
        label: String,
        value: Binding<Float>,
        minValue: Float,
        maxValue: Float,
        step: Float,
        format: FormattedActiveSliderFormat,
        activeSlider: Binding<String?>,
        otherLabels: [String]
    ) -> some View {
        FormattedActiveSlider(
            label: label,
            value: value,
            minValue: minValue,
            maxValue: maxValue,
            step: step,
            format: format,
            activeSlider: activeSlider
        )
        .opacity(
            activeSlider.wrappedValue == nil ||
                activeSlider.wrappedValue == label ||
                otherLabels.contains(activeSlider.wrappedValue ?? "") ? 1 : 0
        )
        .animation(
            .easeInOut(duration: 0.3),
            value: activeSlider.wrappedValue == nil ||
                activeSlider.wrappedValue == label ||
                otherLabels.contains(activeSlider.wrappedValue ?? "")
        )
    }

    func handleRefresh() {
        isUpdating = true

        let viewSetup = vpinballManager.getViewSetup()
        viewMode = VPinballViewLayoutMode(rawValue: viewSetup.viewMode)!
        lookAt = viewSetup.lookAt
        fov = viewSetup.fov
        layback = viewSetup.layback
        viewHOfs = viewSetup.viewHOfs
        viewVOfs = viewSetup.viewVOfs
        sceneScaleX = viewSetup.sceneScaleX
        sceneScaleY = viewSetup.sceneScaleY
        sceneScaleZ = viewSetup.sceneScaleZ
        windowTopZOfs = viewSetup.windowTopZOfs
        windowBottomZOfs = viewSetup.windowBottomZOfs
        viewX = viewSetup.viewX
        viewY = viewSetup.viewY
        viewZ = viewSetup.viewZ
        viewportRotation = viewSetup.viewportRotation

        DispatchQueue.main.asyncAfter(deadline: .now() + 0.01) {
            isUpdating = false
        }
    }

    func handleUpdate() {
        if isUpdating {
            return
        }

        let viewSetup = VPinballViewSetup(viewMode: viewMode.rawValue,
                                          sceneScaleX: Float(sceneScaleX),
                                          sceneScaleY: Float(sceneScaleY),
                                          sceneScaleZ: Float(sceneScaleZ),
                                          viewX: Float(viewX),
                                          viewY: Float(viewY),
                                          viewZ: Float(viewZ),
                                          lookAt: Float(lookAt),
                                          viewportRotation: Float(viewportRotation),
                                          fov: Float(fov),
                                          layback: Float(layback),
                                          viewHOfs: Float(viewHOfs),
                                          viewVOfs: Float(viewVOfs),
                                          windowTopZOfs: Float(windowTopZOfs),
                                          windowBottomZOfs: Float(windowBottomZOfs))

        vpinballManager.setViewSetup(viewSetup)
    }

    func handleSave() {
        vpinballManager.saveViewSetup()

        withAnimation {
            vpinballViewModel.showStatusHUD(icon: "noun-pinball-3169564",
                                            title: "Point of View\nSaved !")
        }

        DispatchQueue.main.asyncAfter(deadline: .now() + 2) {
            withAnimation {
                vpinballViewModel.hideHUD()
            }
        }
    }

    func handleDefaults() {
        vpinballManager.setDefaultViewSetup()

        DispatchQueue.main.asyncAfter(deadline: .now() + 0.25) {
            handleRefresh()
        }
    }

    func handleReset() {
        vpinballManager.resetViewSetup()

        DispatchQueue.main.asyncAfter(deadline: .now() + 0.25) {
            handleRefresh()
        }
    }
}
