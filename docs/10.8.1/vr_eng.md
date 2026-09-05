# VR in 10.8.1

[← Index](README.md#-english) · 🇬🇧 English · [🇫🇷 Français](vr_fra.md)

OpenVR was removed in June 2026
([`6524789e3`](https://github.com/vpinball/vpinball/commit/6524789e3)); VR now
runs on OpenXR only. Settings live in `[PlayerVR]`, and the VR options that were
in the Win32 dialogs moved into the in-game UI
([`be980d774`](https://github.com/vpinball/vpinball/commit/be980d774)).

## Turning it on

| Key | Values | Default | Meaning |
|---|---|---|---|
| `AskToTurnOn` | `0` Enabled, `1` Autodetect, `2` Disabled | `0` on VR-capable builds, `2` elsewhere | whether VR is used |

## Placing the cabinet

These are the settings that make a VR cabinet feel right, and they are all
per-table overridable:

| Key | Range | Default | Meaning |
|---|---|---|---|
| `Orientation` | −180…180 ° | 0 | rotation of the view |
| `TableX`, `TableY`, `TableZ` | −100…100 | 0 | position offsets |
| `LockFeetToGround` | bool | on | keeps the cabinet's feet on the floor rather than letting it float |
| `AddBackglass` | bool | off | adds a default backglass display to the scene |

`LockFeetToGround` is the one that changes the impression most: without it the
cabinet can appear to hover, which reads as wrong even when the height is right.

## Controller-based centring

Rather than nudging offsets by hand, the view can be centred by pointing the
controllers at the real cabinet. Two settings calibrate that:

| Key | Range | Default | Meaning |
|---|---|---|---|
| `ControllerCabYOffset` | −150…50 | 0 | Y offset applied when centring with the controllers |
| `ControllerLockbarScale` | 0.5–2.0 | 1.0 | lockbar size ratio used by that centring |

## The preview window

VR draws a flat preview on the desktop, and it is a window like any other — same
eleven keys as the outputs in [Windows](window_eng.md#four-outputs-one-schema),
prefixed `Preview`: `PreviewDisplay`, `PreviewFullScreen`, `PreviewWndX/Y`,
`PreviewWidth/Height`, `PreviewFSWidth/FSHeight`, `PreviewRefreshRate`,
`PreviewColorDepth`.

`ShrinkPreview` (default off) reduces it, which costs less than rendering it full
size.

## Source

- `src/core/Settings_properties.inl` — the `[PlayerVR]` block
- `src/ui/live/ingameui/` — the VR pages, since the Win32 dialogs went away
