# Mini Frontend

VPX can start into a lightweight, gamepad/cabinet/touch friendly frontend instead of the editors. It
runs inside the player (a session on an empty table), so it benefits from the same renderer, input
and plugin infrastructure as playing a table.

Start it with:

    VPinballX -Frontend

The frontend scans the folders listed in the `Frontend.TableFolders` setting for `.vpx` tables, `.vpz`
packs and VPZ folder packs, reads their `.info` sidecar files and `medias` folders (see
[FileLayout.md](FileLayout.md) for the conventions, e.g. `(Wheel)`, `(Playfield)`, `(Backglass)`,
`(Topper)`, `(GameInfo)` media files) and presents them as a table wheel.

## Media

Each `medias` slot accepts a still image (`.png`, `.jpg`, `.webp`, ...), an animated image (`.apng`,
or a `.png` carrying APNG animation chunks) or a video (`.mp4`, `.mkv`, `.webm`, `.avi`, `.mov`,
`.mpg`, `.mpeg`, `.m4v`, `.f4v`, `.wmv`, `.flv`). Videos and animated images are decoded through the
shared media service (FFmpeg runtime libraries, lazily loaded on Windows) and play muted on a loop:
a `(Wheel) table.mp4` animates its wheel cell, a `(Backglass)` video plays on the preview pane and
on the cabinet's backglass window, and so on. When a media type has both a still image and an
animated media, the animated one is selected. Decoding starts when the media becomes visible and is
paused when it scrolls out of view; sessions that stay unused are closed.

## Settings

| Setting | Description |
| --- | --- |
| Frontend.TableFolders | Semicolon separated list of root folders recursively scanned for tables |
| Frontend.SetupDone | Set when the guided setup has been run (or skipped) |
| Frontend.DisplaySignature | Display setup under which the guided setup was last run, used to propose the assistant again when the display setup changes |
| Frontend.CaptureDelayMs | Delay in milliseconds after starting a table before recording a media capture (0..60000, default 5000) |
| Frontend.Theme | Name of the theme used to lay out the frontend (empty = first theme found) |
| Frontend.ThemeFolders | Semicolon separated list of root folders scanned for theme subfolders |

## Navigation

The table catalog (media panes and wheel) is the frontend's base view: the wheel is navigated with
the flipper buttons, the UI navigation actions, the arrow keys or by dragging/tapping on touch
devices — holding a navigation direction scrolls continuously, the speed ramping up over time. The
in-game UI desktop chrome opens on top of it — with the same top tab bar, category rail and bottom
hint bar as the pause menu of a play session — through the in-game UI toggle action; selecting
**Games** closes it back to the wheel. Escape (the exit button) first steps back out of the
frontend's own modals, then quits the application like the chrome's **Quit** tab. Confirm (Start
button, Enter/Space) on the wheel opens the chrome straight on the **Table** view, the page
proposing the actions on the selected table; the launch ball button plays the selected table
directly.

The chrome's top bar offers:

- **Games**: back to the table catalog (the base view).
- **Table**: the actions on the selected catalog entry (see below).
- **Settings**: the engine settings pages (display, audio, input, ..., including the frontend themes
  page).
- **Plugins**: the plugin settings.
- **About**: the system information page.
- **Quit**: terminates the application.

The Table view offers:

- **Play**: launches the selected table. Leaving the table comes back to the frontend.
- **Edit in live editor**: opens the table in the live editor.
- **Table Info**: a page editing the table's `.info` metadata (title, description, manufacturer,
  year, authors) and its user fields (rating, favorite, tags).
- **Manage Media**: a page listing the discovered media of the table, with deletion.
- **Capture media**: starts the table, waits `Frontend.CaptureDelayMs`, captures playfield and
  backglass stills into the table's `medias` folder and returns to the frontend.
- **Rescan tables**: scans the table folders again.
- **Enter VR** / **Exit VR**: only on XR builds, when a headset is detected / a VR frontend session is
  running.

## Presentations

The layout is chosen automatically from the `Player.BGSet` setting and the display:

- **Cabinet** (`Player.BGSet` = cabinet): the playfield window shows the frontend, the other displays
  (backglass, score view, topper) receive the selected table's media through the ancillary renderer
  broadcast used by plugins.
- **Desktop**: playfield media in the center, backglass media on the top left, table informations on
  the top right, the wheel at the bottom.
- **Full Single Screen / mobile** (`Player.BGSet` = FSS, or a portrait display): backglass media above
  the projected playfield media, wheel at the bottom, touch drag navigation.
- **XR** (builds with OpenXR support): when a VR session can run, a VR toggle button is shown at the
  center of the wheel screen; pressing it, selecting *Enter VR*, or pressing the stereo selection
  action restarts the frontend inside the headset. The frontend is submitted as a head locked quad
  layer, so the whole UI (media, wheel, menus) is available in VR. Playing a table from the XR
  frontend stays in VR, and leaving the table returns to the XR frontend. *Exit VR* or the stereo
  selection action returns to the desktop frontend.

## Themes

The frontend layout can be themed. A theme is a folder containing media files named after the
aspect ratio they target, for example `layout-21x9.webp` for a 21:9 display or `layout-9x16.webp`
for a portrait one:

- `layout-AR.<img>` (required): a color keyed **still image** map that defines where each UI part
  is rendered. The image is read on load and each colored shape is turned into a zone:
  - RGB 0,255,0 (green): playfield preview (min/max rectangle)
  - RGB 0,0,255 (blue): backglass preview (min/max rectangle)
  - RGB 0,255,255 (cyan): instruction card preview (min/max rectangle)
  - RGB 255,0,255 (magenta) and RGB 255,255,255 (white): reserved zones (the top and bottom chrome
    bars are drawn at the display's edges and do not use theme zones)
  - RGB 255,0,0 (red): table wheel lower and upper bounds, scanned from left to right by column
    (missing columns are interpolated from their neighbours)
  - RGB 255,255,0 (yellow): table information pane (title, manufacturer, year, play statistics)
  - the zone colors are the vertices of the RGB cube (except black): they stay correctly
    identified even on lossy formats like webp; transparent pixels and any other color are
    ignored, so theme creators can use them as layout helpers

  Zones that are absent from the map simply hide the corresponding UI part.
- `underlay-AR.<img|video>` (optional): drawn under the media and UI parts.
- `overlay-AR.<img|video>` (optional): drawn over everything.
- `wheel-empty-AR.<img|video>` (optional): placeholder drawn in wheel cells for tables without
  wheel or playfield media, with the table name overlaid on several lines.

Files without an aspect ratio suffix (`layout.webp`, ...) are accepted as a universal fallback.
Underlay, overlay and wheel-empty accept the same still image formats as `layout` plus animated
medias (videos like `.mp4`, animated images like `.apng`), which play on a loop like the table
medias.
When several aspect ratio variants exist, the frontend picks the closest one that is at least as
wide as the display (the closest bigger match); if none fits, the widest available variant is used.
The selected images are fit on the display height and centered, cropping the sides as needed.

Themes are discovered as subfolders of:

- the application's `assets/frontend` folder, which ships the bundled `base` theme;
- each root folder listed in `Frontend.ThemeFolders`.

The active theme is selected by its folder name in `Frontend.Theme`; an empty value picks the first
theme found. Both can be managed from the frontend's *Settings > Frontend Themes* page, which lists
the discovered themes and lets the user add or remove theme folders. When no usable theme is found,
a default layout matching the presentation (desktop, cabinet or FSS) is generated, so the frontend
always renders through the theme code path.

## Command line

See [Command Line.md](Command%20Line.md). In short: `-Frontend` starts the frontend, `-LiveEdit`
starts the live editor and `-Edit` starts the Windows editor.
