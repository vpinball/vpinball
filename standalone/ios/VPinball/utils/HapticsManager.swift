import CoreHaptics

class HapticsManager {
    static let shared = HapticsManager()

    private let queue = DispatchQueue(label: "org.vpinball.haptics")
    private var engine: CHHapticEngine?
    private var player: CHHapticAdvancedPatternPlayer?
    private var needsRestart = false
    private var isRunning = false
    private var stopTime = DispatchTime.now()

    private init() {}

    func start() {
        queue.async {
            if !CHHapticEngine.capabilitiesForHardware().supportsHaptics {
                return
            }

            do {
                let engine = try CHHapticEngine()
                engine.playsHapticsOnly = true
                engine.stoppedHandler = { _ in
                    self.markForRestart()
                }
                engine.resetHandler = {
                    self.markForRestart()
                }
                self.engine = engine
                self.restart()
            } catch {
                self.engine = nil
            }
        }
    }

    func play(lowFrequencySpeed: Float, highFrequencySpeed: Float, durationMs: UInt32) {
        queue.async {
            self.rumble(lowFrequencySpeed: lowFrequencySpeed,
                        highFrequencySpeed: highFrequencySpeed,
                        durationMs: durationMs)
        }
    }

    private func markForRestart() {
        queue.async {
            self.needsRestart = true
        }
    }

    private func restart() {
        if let engine {
            needsRestart = false
            isRunning = false
            player = nil

            do {
                try engine.start()
                let event = CHHapticEvent(eventType: .hapticContinuous,
                                          parameters: [
                                              CHHapticEventParameter(parameterID: .hapticIntensity, value: 1),
                                              CHHapticEventParameter(parameterID: .hapticSharpness, value: 0),
                                          ],
                                          relativeTime: 0,
                                          duration: 30)
                let pattern = try CHHapticPattern(events: [event], parameters: [])
                let player = try engine.makeAdvancedPlayer(with: pattern)
                player.loopEnabled = true
                self.player = player
            } catch {
                needsRestart = true
            }
        }
    }

    private func rumble(lowFrequencySpeed: Float, highFrequencySpeed: Float, durationMs: UInt32) {
        if needsRestart {
            restart()
        }

        if let player {
            let intensity = max(lowFrequencySpeed, highFrequencySpeed)
            if intensity <= 0 {
                stop()
                return
            }

            do {
                if !isRunning {
                    try player.start(atTime: CHHapticTimeImmediate)
                    isRunning = true
                }

                try player.sendParameters([
                    CHHapticDynamicParameter(parameterID: .hapticIntensityControl,
                                             value: cbrtf(intensity),
                                             relativeTime: 0),
                    CHHapticDynamicParameter(parameterID: .hapticSharpnessControl,
                                             value: highFrequencySpeed / (lowFrequencySpeed + highFrequencySpeed),
                                             relativeTime: 0),
                ], atTime: CHHapticTimeImmediate)

                let stopAt = DispatchTime.now() + .milliseconds(Int(durationMs))
                stopTime = stopAt
                queue.asyncAfter(deadline: stopAt) {
                    if self.stopTime == stopAt {
                        self.stop()
                    }
                }
            } catch {
                needsRestart = true
            }
        }
    }

    private func stop() {
        if isRunning, let player {
            isRunning = false
            try? player.stop(atTime: CHHapticTimeImmediate)
        }
    }
}
