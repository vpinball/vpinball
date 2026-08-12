# Settings removed in 10.8.1

[← Index](README.md#-english) · 🇬🇧 English · [🇫🇷 Français](removed_fra.md)

Settings that existed in 10.8 and no longer do. VPX ignores them silently, so an
`ini` carried over from 10.8 keeps lines that look meaningful and do nothing.

Each entry gives what the setting did, the commit that removed it, and what
replaced it — or the fact that nothing did, which is the part worth knowing
before migrating a configuration.

## Input

The input system was rewritten twice: once in March 2025
([`954ceb8d2`](https://github.com/vpinball/vpinball/commit/954ceb8d2)) and again
in October
([`f6252e874`](https://github.com/vpinball/vpinball/commit/f6252e874)). Axes are
no longer named settings; they are fields of a sensor mapping — see
[Input](input_eng.md#sensor-mappings).

| Removed | Was | Replaced by |
|---|---|---|
| `LRAxis`, `UDAxis` | accelerometer axis numbers | the axis field of `Mapping.NudgeN.AccX` / `AccY` |
| `LRAxisFlip`, `UDAxisFlip` | axis inversion | a negative `scale` in the mapping |
| `PlungerAxis`, `PlungerSpeedAxis` | plunger axis numbers | `Mapping.Plunger0.Position` / `.Velocity` |
| `ReversePlungerAxis` | plunger inversion | a negative `scale` in the mapping |
| `AccelVelocityInput` | "the board sends velocity, not acceleration" | the mapping's type field: `V` instead of `A` |
| `EnableNudgeFilter` | anti-noise filter on the raw signal | the dead zone field, plus the Kalman estimator VPX now always applies |
| `PlungerFilter` | never implemented | nothing ([`5b690a030`](https://github.com/vpinball/vpinball/commit/5b690a030)) |
| `InputApi` | which input backend to use | nothing — SDL only |
| `EnableMouseInPlayer` | allow mouse input in game | nothing |
| `DisableESC`, `EscapeKey` | disable or remap the escape key | a normal action mapping, remappable in **F12 → Input** |
| `Joy*Key` (all of them) | one setting per joystick button | `Mapping.<Action>` with a device id and a button number |

## Tilt and nudge

Legacy tilt was removed in November 2025
([`89527dd2b`](https://github.com/vpinball/vpinball/commit/89527dd2b)); the
commit notes it had been disabled for years. The plumb simulation replaced it —
see [Input](input_eng.md#tilt).

| Removed | Was | Replaced by |
|---|---|---|
| `TiltAmount`, `TiltTriggerTime` | legacy tilt sensitivity and window | `PlumbThresholdAngle` |
| `JoltAmount`, `JoltTriggerTime` | legacy jolt detection | nothing — the plumb covers it |
| `PlumbInertia` | plumb inertia | `PlumbDamping`, which is a damping factor rather than an inertia |
| `EnableLegacyNudge`, `LegacyNudgeStrength` | VP9 nudge model | `KeyboardNudgeMode` (which still offers the VP9 model) and `KeyboardNudgeStrength` |

## Rendering

| Removed | Was | Replaced by | Commit |
|---|---|---|---|
| `Stereo3DFake`, `Stereo3DMaxSeparation`, `Stereo3DOffset`, `Stereo3DYAxis`, `Stereo3DZPD` | fake stereo, a 2D approximation | nothing — real stereo only, via `Stereo3D` | [`7c456c07a`](https://github.com/vpinball/vpinball/commit/7c456c07a) |
| `Anaglyph` | anaglyph as a separate mode | a `Stereo3D` mode, with its calibration in F12 | [`7c456c07a`](https://github.com/vpinball/vpinball/commit/7c456c07a) |
| `ScaleFX`, `ScaleFXDMD` | ScaleFX upscaler | nothing | [`8d018bbec`](https://github.com/vpinball/vpinball/commit/8d018bbec) |
| `AdaptiveVSync`, `EnableLegacyMaximumPreRenderedFrames`, `MaxPrerenderedFrames` | frame pacing knobs inherited from DX9 | `SyncMode` (four values, including frame pacing) and `MaxFramerate` | [`0f556f09b`](https://github.com/vpinball/vpinball/commit/0f556f09b) |
| `DisableDWM` | disable the Windows compositor | nothing | [`9cad8a131`](https://github.com/vpinball/vpinball/commit/9cad8a131) |
| `BWRendering` | black and white rendering | nothing | [`da11f5481`](https://github.com/vpinball/vpinball/commit/da11f5481) |

## External windows

Backglass, DMD and PUP windows were owned by the standalone build, each with its
own position and size settings. They became plugins and rendered surfaces
([`779f7be15`](https://github.com/vpinball/vpinball/commit/779f7be15),
[`25ec2f640`](https://github.com/vpinball/vpinball/commit/25ec2f640),
[`6f0e23fe0`](https://github.com/vpinball/vpinball/commit/6f0e23fe0)).

| Removed | Replaced by |
|---|---|
| `B2SBackglassX/Y/Width/Height/Rotation` | the `[Backglass]` window section |
| `B2SDMDX/Y/Width/Height/Rotation/FlipY` | the `[ScoreView]` window section |
| `B2SWindows`, `B2SPlugins`, `B2SDualMode`, `B2SHide*` | `[Plugin.B2SLegacy]` |
| `PUPWindows`, `PUPCapture`, `PUPPlugin` | `[Plugin.PUP]` |
| `DOFPlugin` | `[Plugin.DOF]` |
| `DMDServer`, `DMDServerAddr`, `DMDServerPort` | the DMDUtil plugin's own settings |
| `AltColor`, `AltSound` | `[Plugin.AltColor]`, `[Plugin.AltSound]` |

## A note on the November 2025 migration

A series of commits titled *Migrate … props* moved every setting to a property
registry. Names mostly survived, but some moved section, and defaults were
re-declared in one place — `src/core/Settings_properties.inl`, which is now the
authoritative list of what exists.

A setting missing from that file does not exist, whatever an old ini or an old
guide says.
