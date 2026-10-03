# VPZ Table Pack File Format

**Warning: this file format is a preliminary definition, not yet ready for production use. It may change in backward incompatible ways at any time.**

Visual Pinball X tables can be saved as a *pack*: a container holding the complete table definition and all of its assets in standard, easily editable formats. It is designed as an open alternative to the binary `.vpx` file (an OLE container), meant for easy diffing, hand editing, and interoperability with DCC tools.

A pack comes in 2 forms, which are strictly equivalent:
- a **directory** (which may or may not be named with a `.vpz` extension)
- a **`.vpz` file**, which is the same content stored as a zip archive

All text files of a pack (JSON documents, VBS script) are **UTF-8** encoded.


## Naming

Every named entity (scene node, collection, material, image, sound, font, mesh) is stored in a file named after its **unique name**. Names are therefore *not* stored inside the files, as they are already defined by the file names.

For filesystem portability, names are sanitized when mapped to file names: characters invalid on common filesystems (`/ \ : * ? " < > |` and control characters) are replaced by `_`, trailing dots and spaces are stripped, and empty results become `_`. Collisions between sanitized names are resolved by a `_2`, `_3`, ... suffix. Names containing such characters may therefore not round-trip exactly; portable names are recommended.


## Pack layout

```
manifest.json                  pack identification and properties
table.json                     the table definition
script.vbs                     the VBS game script
parts/<name>.json              one file per scene node (part or layer)
collections/<name>.json        one file per collection
materials/<name>.json          one file per material
renderprobes/<name>.json       one file per render probe
images/<name>.<ext>            image, in its original import format (untouched)
images/<name>.json             image properties sidecar
sounds/<name>.<ext>            sound, in its original import format (untouched)
sounds/<name>.json             sound properties sidecar
fonts/<name>.<ext>             font file
fonts/<name>.json              font properties sidecar
meshes/<name>.glb              primitive mesh, glTF binary
```

Every JSON document carries a `"$type"` property identifying its content (`"manifest"`, `"table"`, a part type name, `"collection"`, `"material"`, `"renderprobe"`, `"image"`, `"sound"`, `"font"`). It is always the first property of the document.

Properties are serialized in a canonical order, so that files only change when their content does, making them suitable for versioning (e.g. in a git repository).


## manifest.json

Identifies the pack and holds the pack properties:

| Property       | Content                                                                  |
| -------------- | ------------------------------------------------------------------------ |
| `$type`        | Always `"manifest"`                                                      |
| `file_format`  | Always `"vpinball-pack"` (a pack may be partial, not just a whole table) |
| `file_version` | Integer, currently `1`                                                   |
| `name`         | Table name                                                               |
| `author`       | Table author                                                             |
| `version`      | Table version                                                            |
| `description`  | Table description                                                        |
| `save_date`    | Date the pack was written                                                |


## table.json

The complete table definition: all persisted PinTable properties (dimensions, physics, rendering, view layouts, ...) plus the table information fields (`table_name`, `author`, `table_version`, `release_date`, `author_email`, `web_site`, `blurb`, `description`, `rules`, `date_saved`, `save_rev`) and `custom_tags`.

Notable structure:
- `parts`, `collections`, `materials`, `renderprobes`: ordered lists of the unique names of the corresponding entities, preserved to keep the editor ordering (part z-order). File resolution is a loader concern (see Naming above); files present in the folders but absent from the lists are still loaded.
- `desktop_view`, `cabinet_view`, `fullsinglescreen_view`: the 3 view layouts, each a sub object with `mode`, `rotation`, `inclination`, `layback`, `fov`, `view_x/y/z`, `scale_x/y/z`, `horizontal_ofs`, `vertical_ofs`, `window_top_z_ofs`, `window_bot_z_ofs`. `is_fullsinglescreen_view_enabled` selects whether the FSS layout may be used.
- `vbs_script`: reference to the script file (`script.vbs`).


## parts/, collections/, materials/, renderprobes/

One JSON file per scene node, collection, material and render probe, named after its unique name. Part files carry a `"$type"` naming the part type (`"bumper"`, `"flipper"`, `"primitive"`, `"surface"`, `"light"`, `"partgroup"`, ...), render probe files use `"renderprobe"`. References to other scene nodes or assets use their unique names (e.g. a primitive references its image by name and its mesh by a `mesh` property pointing to `meshes/<name>.glb`).

A partial pack (assets and/or scene nodes without a `table.json`) is also a valid pack. This is what the editor's part export produces: the selected parts (with their part group ancestors and the members of the selected groups), the collections they belong to (filtered to the exported members), and only the assets the parts reference by name.


## Asset sidecars

Each binary asset file has a sibling JSON sidecar, named `<name>.json`, holding the properties that were assigned at import time. The data file itself is found by matching the sidecar's file name stem.

- **images**: `$type` `"image"`, `import_path`, `width`, `height`, `alpha_test` (0-255, negative when disabled), `md5` (32 hex digits), `opaque`, optional `link` (legacy binary sharing)
- **sounds**: `$type` `"sound"`, `import_path`, `output_target` (`"playfield"` or `"backglass"`), `volume_offset`, `left_right_offset` (-100 full left, +100 full right), `rear_front_offset` (-100 full rear, +100 full front)
- **fonts**: `$type` `"font"`, `import_path`


## meshes/

Primitive meshes are stored as glTF binary (`.glb`) files, in meters, with +Y up and the player side of the table toward +Z, matching the convention of DCC tools (Blender glTF import/export defaults). Mesh animation frames are stored as morph targets.
