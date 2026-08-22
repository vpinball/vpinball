# Rendering in 10.8.1

[← Index](README.md#-english) · 🇬🇧 English · [🇫🇷 Français](rendering_fra.md)

All of these live in `[Player]`. The frame pacing work
([`3da46f1ef`](https://github.com/vpinball/vpinball/commit/3da46f1ef)) replaced
the DX9-era latency knobs — see [Removed](removed_eng.md#rendering).

## Synchronisation

`SyncMode` is the setting that decides how the picture is paced, and its default
changed: **frame pacing is now the default**, not vertical sync.

| Value | Mode | Trade-off |
|---|---|---|
| `0` | No sync | tearing, lowest latency |
| `1` | Vertical sync | no tearing, high visual latency |
| `2` | Adaptive sync | vsync except for late frames, still high latency |
| `3` | **Frame pacing** (default) | paces rendering to cut latency; can stutter on an underpowered machine |

`MaxFramerate` limits the rate, and its two special values are worth knowing:

| Value | Effect |
|---|---|
| `-1` (default) | limit to the display's refresh rate |
| `0` | no limit at all |
| any other | limit to that value — steadier framerate, less heat |

Note the inversion: `-1` is the sensible default and `0` removes the cap
entirely — reading these as plain numbers gets it backwards.

Whatever you ask for, **as soon as `SyncMode` is not `0` the value is clamped to
the display**, in `Player::Init`: above the refresh rate it is brought back down
to it, below it it is rounded to an integer division of it (60 → 30 → 20, never
under 24 FPS). So `SyncMode = 3` with `MaxFramerate = 240` on a 144 Hz screen
runs at 144, not 240.

### Variable refresh rate (G-Sync, FreeSync)

That clamp is exactly what a VRR panel does not want, since the whole point of
VRR is that the display follows the game rather than the reverse. But turning
synchronisation off is not automatically the right answer either, because frame
pacing is not vertical sync: mode `3` runs a different game loop
(`FramePacingGameLoop`) that aims to have the frame ready just in time for the
vblank, keeping physics and input stepping meanwhile. It cooperates with a
variable refresh rate rather than fighting it.

Two setups make sense:

- `SyncMode = 3` with `MaxFramerate` set to the display's refresh rate. The
  clamp then becomes a no-op, since you are asking for exactly what the screen
  does, and you keep the low-latency pacing. Start here.
- `SyncMode = 0` with `MaxFramerate` set by hand, which frees VPX from the
  clamp and lets the monitor pace the game. Capping a couple of Hz below the top
  of the VRR window so as never to leave it is general VRR practice rather than
  anything specific to VPX.

Modes `1` and `2` are the ones to avoid on a VRR screen: both impose a fixed
cadence, along with the visual latency that comes with it. VR is a separate
case — sync is forced off there and pacing is left to the runtime, see
[VR](vr_eng.md).

## Anti-aliasing

Three independent stages, which stack:

| Key | Values | Meaning |
|---|---|---|
| `AAFactor` | 0.5–2.0, default 1.0 | supersampling — renders at that multiple of the resolution, then downsamples. 2.0 means four times the pixels |
| `MSAASamples` | `0` disabled, `1` 4×, `2` 6×, `3` 8× | geometry aliasing only, costs GPU memory |
| `FXAA` | `0` disabled … `7` | post-process: Fast/Standard/Quality FXAA, Fast NFAA, Standard DLAA, Quality SMAA, Quality FAAA |

`AAFactor` is brute force and by far the most expensive: 200 % is twice the
resolution in each direction, so four times the work. MSAA is cheaper but only
touches geometry edges. FXAA is nearly free and blurs a little.

`Sharpen` (`0` disabled, `1` CAS, `2` Bilateral CAS) exists mostly to counter
that blur.

## Reflections and textures

| Key | Values | Meaning |
|---|---|---|
| `PFReflection` | `0`–`5`, default `5` | Disable / Balls Only / Static Only / Static & Balls / Static & Unsynced Dynamic / **Dynamic** |
| `SSRefl` | bool | screen-space reflections |
| `MaxTexDimension` | 512–16384, default 16384 (1536 on mobile) | images above this are scaled down on load |

`PFReflection` at `2` (Static Only) is the one with no runtime cost, except in
VR. `5` is the default and the best looking.

## Stereo

`Stereo3D` selects the mode, and the fake modes are gone
([`7c456c07a`](https://github.com/vpinball/vpinball/commit/7c456c07a)) — what
remains is real stereo: Top/Bottom, interlaced variants, side-by-side, anaglyph.
`Stereo3DEnabled` toggles it, and the eye separation, brightness, saturation,
defocus and per-eye contrast tune it.

## The one cosmetic setting worth knowing

`BallAntiStretch` ("Unstretch Ball", default **off**) compensates the render
stretch that makes a fast ball look egg-shaped. It changes nothing about
physics — but on a cabinet playfield it is usually wanted, and it is off unless
asked for.

## Source

- `src/core/Settings_properties.inl` — every key above, with its range and default
- `src/renderer/Renderer.cpp` — where the post-process chain is assembled
