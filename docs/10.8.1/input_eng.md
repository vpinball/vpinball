# Input and nudge in 10.8.1

[← Index](README.md#-english) · 🇬🇧 English · [🇫🇷 Français](input_fra.md)

10.8.1 replaced the plunger and nudge input handling. The old
`Mapping.PlungerPos` / `Mapping.NudgeX1` keys are gone and are no longer read.
What follows is the schema that replaced them, and the chain a reading travels
through before it reaches the ball.

## Devices

Every device is declared in `[Input]`:

```ini
Devices = SDLJoy_PSC0041701862884E45J009
Device.SDLJoy_PSC0041701862884E45J009.Type =
Device.SDLJoy_PSC0041701862884E45J009.Name =
Device.SDLJoy_PSC0041701862884E45J009.NoAutoLayout = 1
```

`NoAutoLayout = 1` stops VPX proposing its default layout for that device. This
matters more than it looks: the default mappings — including their unit scales —
are only ever installed by `ApplyDefaultDeviceMapping`, which runs when a device
is auto-detected **and** the layout prompt is accepted. A config that already
holds mappings, or that sets `NoAutoLayout`, keeps whatever it has.

## Sensor mappings

Sensors are counted, then mapped:

```ini
PlungerSensorCount = 1
Mapping.Plunger0.Position = <device>;514;P;0.000000;1.000000;1.000000
Mapping.Plunger0.Velocity = <device>;517;V;0.000000;12.500000;1.000000

NudgeSensorCount = 1
Mapping.Nudge0.AccX = <device>;512;A;0.030000;9.806650;1.000000
Mapping.Nudge0.AccY = <device>;513;A;0.030000;9.806650;1.000000
Mapping.Nudge0.Type = 1
Mapping.Nudge0.Strength = 1.000000
```

Each mapping line has six fields:

```
device ; axis ; type ; deadZone ; scale ; limit
```

| Field | Meaning |
|---|---|
| `device` | the device id declared above |
| `axis` | SDL axis number |
| `type` | `P` position, `V` velocity, `A` acceleration |
| `deadZone` | fraction of the axis range nulled around rest, `0`–`0.3` |
| `scale` | **unit conversion** — see below |
| `limit` | clamp applied to the normalized value |

They are applied in that order, in `SensorMapping::UpdateValue`:

1. the raw SDL value is normalized to −1…+1 (÷ 32768);
2. the dead zone is removed and the remainder rescaled — `(v − dz) / (1 − dz)`,
   so clearing the dead zone starts from zero rather than jumping;
3. the value is clamped to ±`limit`;
4. it is multiplied by `scale`.

The dead zone therefore bites **before** the scale, on the raw axis fraction.
The same 3 % swallows 0.29 m/s² on a 1 g board and 2.4 m/s² on an 8 g one.

## The scale is a unit, not a sensitivity

`scale` converts the normalized axis value into the unit the physics engine
expects: **m/s² for accelerations, m/s for velocities, per-unit/s for the
plunger** (one unit being the plunger's travel length). Left at the default of
1.0, a full-scale reading reaches the engine as 1 m/s² instead of 1 g, and the
plunger impulse comes out two orders of magnitude short — the `/100` in
`HitPlunger::HitTest` is meant to be absorbed by this scale.

These are the values the in-game sensor setup page offers as presets:

| Sensor | Scale | Meaning |
|---|---|---|
| Accelerometer, ±1 g | `9.80665` | full deflection = 1 g |
| Accelerometer, ±2 g | `19.61330` | |
| Accelerometer, ±4 g | `39.22660` | |
| Accelerometer, ±8 g | `78.45320` | |
| Pinscape plunger velocity | `12.5` | per-unit/s over the plunger frame |
| Pinscape nudge velocity | `4096 / (20 × 1000)` | board reports mm/s ×20 over ±4096 |

Sensitivity is a separate field: `Mapping.NudgeN.Strength`, `0`–`2` around a
neutral `1`. Driving sensitivity through `scale` lies to the engine about what
the sensor is, and makes a 4 g or 8 g board impossible to describe.

## The three nudge modes

`Mapping.NudgeN.Type` selects how a reading becomes cabinet motion:

**0 — Game Controller.** For a gamepad or VR stick. Reads how far and how fast
the stick is pushed to infer a nudge. Nothing to do with an accelerometer.

**1 — Intent Sensor.** For cabinets with an accelerometer board. It does not
pass your acceleration through: `NudgeIntentHandler` detects peaks, discards the
cabinet's own ringing (a smaller peak within 300 ms of a stronger one), and above
a threshold injects a calibrated 25 ms half-cosine impulse whose amplitude is the
peak. Below that threshold **nothing at all** reaches the ball. Designed for
boards that are noisy, biased or slow.

**2 — Cabinet Sensor.** For a fast, clean, low-latency sensor. The measurement
drives the cabinet directly, smoothed by a 4 ms EMA and rescaled from the real
cabinet's mass (`Mapping.NudgeN.CabWeight`, kg) to the simulated 113 kg. No
threshold, so small shakes count — and so does sensor noise.

The intent threshold is **1 m/s², hardcoded** in
`NudgeIntentHandler::EvaluateImpulse`, and it is compared *after* `Strength` is
applied. Raising Strength therefore makes nudges both stronger and easier to
recognise. It is not exposed in any UI.

## The full chain

For an accelerometer, in order:

```
raw axis → dead zone → limit → scale (m/s²)
  → Kalman estimator (always, both modes)
  → × Strength
  → mode: intent threshold + impulse   |   EMA + cabinet weight
  → damped cabinet oscillator (113 kg; 9.3 Hz ζ0.052 X, 5.8 Hz ζ0.055 Y)
  → plumb pendulum (100 mm rod, 1 kHz)
  → tilt when the rod passes PlumbThresholdAngle
```

The Kalman estimator (`MotionKalmanAxis`) is not a smoother — its job is bias
removal, so a board mounted a degree off level does not read as a permanent
shove. One consequence is worth knowing: **it takes its very first sample as
bias**. A sensor that starts moving before it has ever been still is read as
offset, not motion.

## Tilt

Two keys in `[Player]`:

| Key | Range | Default | Meaning |
|---|---|---|---|
| `PlumbThresholdAngle` | 0.15–4 ° | 1.0 | rod angle that trips the tilt |
| `PlumbDamping` | 0–2 | 1.0 | 0 swings forever, 2 settles twice as fast as life |

VPX does not compare acceleration to a threshold. It simulates a bob on a 100 mm
rod, driven by gravity plus the cabinet's acceleration, integrated every
millisecond, and tilts when the rod's angle passes the threshold. A short violent
knock can leave it unmoved while three rhythmic taps trip it — which no
instantaneous threshold can express.

The shove a threshold needs is not obvious, and depends on the mode. Simulating
the chain above with a 100 ms push, at Strength 1.0, gives the acceleration the
sensor must see:

| Threshold | Intent | Cabinet |
|---|---|---|
| 0.15° | 1.0 m/s² | 0.5 m/s² |
| 1.00° (default) | 6.7 m/s² | 3.5 m/s² |
| 2.08° | 14.0 m/s² | 7.4 m/s² |
| 4.00° | 27.2 m/s² | 14.5 m/s² |

Two things follow. Intent needs roughly twice the shove of Cabinet for the same
angle, because only the peak of your knock survives the 25 ms impulse. And on a
±1 g board — full scale 9.81 m/s² — anything past about 1.4° in Intent mode is
**unreachable**: the sensor saturates before the pendulum gets there, so the top
two thirds of the slider do nothing.

## Source

- `src/input/SensorMapping.h` — the six fields, the processing order, the unit scales
- `src/input/InputManager.cpp` — device declaration, `ApplyDefaultDeviceMapping`
- `src/physics/cabinet/NudgeIntentHandler.cpp` — peak detection, impulse, threshold
- `src/physics/cabinet/CabinetNudgeSensor.cpp` — the cabinet-sensor path
- `src/physics/cabinet/MotionKalmanAxis.h` — the estimator and its bias assumption
- `src/physics/cabinet/PlumbHandler.cpp` — the pendulum
