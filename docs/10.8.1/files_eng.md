# Files and folders in 10.8.1

[← Index](README.md#-english) · 🇬🇧 English · [🇫🇷 Français](files_fra.md)

Where VPX reads and writes is decided by `FileLocator`, and the rule that
surprises people is that **the file layout is not a setting**. It is deduced at
every start from where `VPinballX.ini` is found.

## The two layouts

`FileLocator::UpdateFileLayoutMode` looks for the ini in two places, in this
order:

| Ini found in | Layout | Consequence |
|---|---|---|
| the preference folder | `AppPrefData` (default) | settings and log in the preference folder, per-table files next to the `.vpx` |
| next to the executable | `AppOnly` (legacy) | everything together in the application folder |
| neither | `AppPrefData` | a fresh preference folder is created |

The order matters: an ini sitting in the preference folder **wins**. Dropping a
copy next to the executable changes nothing on its own — to go back to the
single-folder layout the preference copy has to be gone, not just duplicated.

`AppOnly` is the pre-10.8 behaviour, from a time when an application could write
anywhere. It still works, and it needs write access to the install folder.

## Where each thing lives

The preference folder is `SDL_GetPrefPath(nullptr, "VPinballX")` followed by a
`major.minor` subfolder — so `~/.local/share/VPinballX/10.8/` on Linux,
`%AppData%\VPinballX\10.8\` on Windows. Settings are grouped per minor version
on purpose: minor revisions are not allowed to require a user setup change.

| Content | `AppPrefData` | `AppOnly` |
|---|---|---|
| `VPinballX.ini` | preference folder | application folder |
| `vpinball.log` | preference folder | preference folder |
| `assets/`, `scripts/`, `plugins/`, `docs/`, `shaders-x.y.z/` | application folder, read only | idem |
| `user/` — highscores, per-table saved data | next to the `.vpx` | shared, in the application folder |
| `music/` — used by `PlayMusic()` | next to the `.vpx` | application folder |
| `cache/` — decoded textures | `cache/<table title>/` next to the `.vpx` | preference folder |
| `autosave/` | next to the `.vpx` | application folder |

Table files landing next to the table, rather than in one shared pile, is what
the modern layout buys: it works best when each table has its own folder, and
it survives moving a table around.

## The user folder

There is nothing to create by hand. VPX creates `user/` the first time a table
actually writes into it — a highscore, a saved table option — and logs a line
when it does. An empty install simply has no `user/` folder yet, which is
normal and not a sign of a broken setup.

Coming from an older install, an existing shared `user/` folder is still
usable: it is on the read path if you drop it either next to the binary or in
the preference folder.

## Where scripts are searched

`FileLocator::SearchScript` walks these, in order, and takes the first hit
(case-insensitively):

1. the table's own folder, then `<table>/user`, then `<table>/scripts`
2. the application folder, then its `user`, `scripts` and `tables` subfolders
3. the preference folder, then its `user` and `scripts` subfolders

## Migration on first start

When the preference folder for this `major.minor` does not exist yet, VPX
builds it rather than starting empty. It looks, in order, for:

1. the preference folder of a previous version — scanning down from the current
   one to 10.0, then the un-versioned `VPinballX` folder used before the
   `major.minor` layout — and copies its whole content recursively;
2. failing that, a `VPinballX.ini` sitting next to the executable, which it
   copies in;
3. failing that, on Windows only, the pre-10.8 registry settings.

One exception: if an ini next to the executable already declares the current
`major.minor` in its `[Version]` section, VPX creates the preference folder and
stops there, leaving the legacy layout alone.
