# View and cabinet fitting in 10.8.1

[← Index](README.md#-english) · 🇬🇧 English · [🇫🇷 Français](view_fra.md)

10.8.1 adds automatic view fitting for cabinets, and with it a dependency most
tables have never had to satisfy: the height of the glass.

## Three projections

`ViewLayoutMode` decides how the scene is projected. It is set per view mode and
per table, and reachable in game through **F12 → Point of View**:

| Mode | What it does |
|---|---|
| `0` Legacy | pre-10.8 rendering — a fit over bounding vertices, skewed by a layback angle. Visually incorrect stretches by design. |
| `1` Camera | classic perspective, viewer placed relative to the bottom centre of the table. The desktop mode. |
| `2` Window | oblique reprojection, viewer placed relative to the bottom centre of **the screen**. Designed for cabinets. |

Window is the one that treats your screen as the physical glass of the cabinet,
with the viewer positioned relative to it in real units. That is also why head
tracking only works there: moving your eye has to produce an asymmetric frustum,
which needs a screen that exists in the model.

The keys are `TableOverride.ViewCabMode`, `ViewDTMode` and `ViewFSSMode` — one
per view mode (Cabinet, Desktop, FSS), stored as a per-table override rather
than in the application settings.

## Automatic fitting

`Player.CabinetAutofitMode` applies to the Cabinet view:

| Value | Name | Effect |
|---|---|---|
| `0` | Manual | nothing is computed; the table's own point of view is used |
| `1` | Fit Table | uniform scale, fitted on table width |
| `2` | Fit Screen | non-uniform stretch, table fills the screen |

The difference is two lines of `ViewSetup::SetWindowAutofit`:

```cpp
mSceneScaleX = (screenHeight / table->GetTableWidth()) * (table->GetHeight() / screenWidth);
mSceneScaleY = allowNonUniformStretch ? 1.f : mSceneScaleX;
```

**Fit Table** keeps proportions — Y is scaled like X — at the cost of not showing
everything: part of the apron, or of the top of the table, falls outside the
frame. `Player.CabinetAutofitPos` (0–20 %, default 5 %) then decides where the
resting flipper bats sit on screen, so the same reference point holds from table
to table.

**Fit Screen** cuts nothing but stretches one axis against the other. On a
head-tracked cabinet this is the wrong choice: parallax depends on rendered
geometry matching real geometry, and a stretched scene answers a horizontal head
movement differently from a vertical one.

Selecting either mode forces `mMode = VLM_WINDOW`, so autofit and the Window
projection come together.

## What autofit needs from the table

Two things, and only one of them is under the table's control.

**The screen's physical dimensions**, `Player.ScreenWidth` and
`Player.ScreenHeight`, in centimetres. Autofit refuses to run without them:

```
Screen dimensions must be defined before using automatic point of view
```

These are the dimensions of the visible playfield area, not the panel's diagonal.
Get them wrong and every table is misframed identically.

**The glass heights**, `m_glassBottomHeight` and `m_glassTopHeight`, which become
the projection volume:

```cpp
Vertex2D glass = table->EvaluateGlassHeight();
if (table->m_glassTopHeight != table->m_glassBottomHeight) { /* the table declares it */ }
else { glassNotification("Missing glass position guessed to be {..}cm / {..}cm"); }
mWindowBottomZOfs = bottomHeight;
mWindowTopZOfs    = topHeight;
```

A flat glass was the old default, so a table that never set these is detected by
its glass being horizontal, and VPX **guesses** both heights by analysing element
bounds. When the table does declare them, VPX uses its values but compares them
to its own estimate and warns if they differ by more than an inch:

```
Glass height was evaluated to X cm / Y cm
It differs from the defined glass position Z cm / W cm
```

Those two notifications are the first thing to read when a table sits badly in
Window mode. They say whether the frame rests on declared geometry or on a guess.

## Why tables sit badly

Window mode exercises geometry that Camera and Legacy never did. A table can look
perfect on a desktop and still show, on a cabinet:

- **apron or cabinet walls drawn where they should not be** — geometry that was
  only ever seen from a viewpoint that hid it;
- **elements displaced in proportion to their height** above the playfield, which
  is the signature of a wrong projection volume rather than of misplaced parts:
  a painted insert cannot move, a ramp 10 cm up moves the most;
- **a frame that cuts too much or too little**, when the glass heights were
  guessed rather than declared.

None of this is visible without a cabinet running Window mode, which is why it
went unnoticed: the people who could fix the tables are the least likely to meet
the problem.

## Source

- `src/renderer/ViewSetup.h` — `ViewLayoutMode`, the three projections
- `src/renderer/ViewSetup.cpp` — `SetWindowAutofit`, glass heights, scene scales
- `src/core/player.cpp` — `SetCabinetAutoFitMode`, `SetCabinetAutoFitPos`
- `src/ui/live/ingameui/PointOfViewSettingsPage.cpp` — the F12 page
- `src/core/Settings_properties.inl` — `CabinetAutofitMode`, `CabinetAutofitPos`, `BGSet`
