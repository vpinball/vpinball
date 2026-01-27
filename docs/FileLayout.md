# File Layout

Visual Pinball X started in the 2000's as a Windows only all-in-one application with a simple file layout scheme: put everything in a global folder mixing all types of assets together. The growth of the ecosystem over the years adding lots of new components and optional features, multiple front ends, cross-platform support rendered this file layout scheme obsolete, needing a bit of organization between components for smoother cooperation.


Note: for the ease of transition of existing installations, the legacy file layout is still supported but discouraged, and support will likely be dropped at some point.


## Visual Pinball X installation

Visual Pinball X installation is made of 3 parts: the application, its preferences, and the data. Each host platform defines where these parts are located:
- The main application
  - Windows: `C:\Program Files\...`
  - MacOS: `/Applications/VPinballX_BGFX.app/`
  - iOS, Android & Meta Quest: system application folder (not directly accessible)
- The settings
  - Windows: `C:\Users\xxx\AppData\Roaming\...`
  - MacOS: `/Users/xxx/Library/Application Support/VPinballX`
  - Linux: `/home/xxx/.local/shared/VPinballX`
  - Android & Meta Quest: `/data/data/org.vpinball.app/files/`
  - iOS: preferences are stored in the app's Documents directory
- The 'Tables' folder to store all table datas
  - Windows: in a user created folder inside `C:\Users\xxx\Documents\...`
  - MacOS: in a user created folder inside `/Users/xxx/Documents`
  - Android & Meta Quest: `/data/data/org.vpinball.app/files/`
  - iOS: Tables are stored in the app's Documents directory

Mobile platform notes:
- To simplify file management, VPX includes a **built-in web server** on all mobile platforms. Enable it in settings to upload tables and transfer files from any browser on the same network.
- On Android, on first launch, VPX copies required assets from the APK to the app's internal storage, typically: `/data/data/org.vpinball.app/files/assets/`
- On iOS, to provide additional user-friendly file access:
  - The Documents folder is accessible via the **Files app** on the device
  - When connected to a Mac, files can be transferred through **Finder**

Starting with VPX 10.8.1, settings are stored in a subfolder per minor version, that is to say 10.8, 10.9,... . This allows to have multiple minor versions installed on the same computer, without facing setting conflicts.

## Global Settings and table overrides

Visual Pinball X is a very flexible game engine with thousands of settings allowing it to be used in a wide variety of usecases ranging from desktop computer, virtual reality, pinball cabinets, mobile phones, minipinball devices based on single chip computers,...


The settings system is based on 2 files:
- the global settings file, named 'VPinballX.ini',
- an optional table override file which is loaded and applied when playing a table, named against the table filename or containing folder (see below).

The table override file allows to tweak a few settings for playing a given table by overriding the settings defined in the global settings.

All the settings can be adjusted using the in-game UI, available when playing. When clicking the save button, the user is asked wether the modifications should be saved as global preferences, or as table overrides. Settings that have been overriden for the played table can be identified by a circle & dot icon in the in-game UI.

For advanced uses, it is possible to specify the global and table override ini files on the command line.


## Table's folder organization

A Visual Pinball X table may come as a single 'vpx' file, but it will usually benefit from additional companion files: table overide ini file, backglass file, pupvideo folder, DMD colorization, musics,... Therefore, it is preferred to store **each table in its own folder** along its companion files.


Moreover, to simplify and unify the way things are managed between the VPX application, the core plugins and third-party components, a common file search scheme is defined, based on a **folder per table** logic. Companion files are searched following these 3 steps:
- first search along the table file, with a name matching the played table file,
- then search along the table file, with a name matching the name of the folder containing the played table file,
- finally, eventually search in a custom legacy folder (custom behavior is defined by each component).



The following tree is an example of this file organization:
```
Table Name (Manufacturer Year)/              <= We created a dedicated folder to hold all the files for this table
├── Table file v1.1.vpx                      <= This is the main VPX file
├── Table file v1.1.ini                      <= This file holds the table setting overrides
├── Table file v1.1.vbs                      <= For some reason, we decided to override the table script by a custom script
├── Table file v1.1.scv                      <= ScoreView plugin will use this layout file if present (otherwise defaulting to its global scoreview folder)
├── Table file v1.0.vpx                      <= For example, we decided to keep an older version for reference
├── Table Name (Manufacturer Year).directb2s <= We chose to name the backglass after the folder name, as it is shared between all table versions
├── Table Name (Manufacturer Year).info      <= This is an information file to store frontend datas, we decided to name after the folder name (not directly linked to VPX, see below)
├── Installation.txt                         <= Additional file the table authors decided to include
├── Rulesheet.pdf                            <= Additional file the table authors decided to include
├── altsound/                                <= AltSound plugin will look here for altsound datas
│   └── xxx
│       └── ...
├── cache/                                   <= VPX cache some informations for smoother play, they will be stored here
│   └── ...
├── medias/                                  <= This is a common folder to store frontend files (not directly linked to VPX, see below)
│   ├── (Backglass) Table Name (Manufacturer Year).mp4
│   ├── (Playfield) Table Name (Manufacturer Year).mp4
│   ├── (Wheel) Table Name (Manufacturer Year).apng
│   └── ...
├── music/                                   <= Folder from which music are loaded when script use the PlayMusic command
│   ├── Multiball Theme.ogg
│   └── ...
├── pinmame/                                 <= PinMAME plugin will look here for rom & nvram files
│   ├── rom/
│   │   ├── xxx.zip
│   │   └── yyy.zip
│   └── nvram/
│       ├── xxx.nv
│       └── yyy.nv
├── pupvideos/                               <= PinUp player plugin will look here for pinup videos
│   └── xxx
│       └── ...
├── scripts/                                 <= When a table loads additional script, they are searched here as well as in core script folder
│   └── ...
├── serum/                                   <= Serum plugin will look here for colorization files
│   └── xxx
│       └── xxx.crz
├── Table Name.UltraDMD/                     <= Folder with FlexDMD or UltraDMD content (name is directly defined in the table script)
│   └── ...
└── user/                                    <= VPX stores values saved from script in this folder
    └── VPReg.stg
```


## Zero install table deployment guidelines

When a table is added to an existing install, it may require a few steps to get it up and running. To simplify this process, the following guidelines are proposed. These are not directly linked to VPX implementation and are just guidelines for frontends that would want to use them as a reference:
- Store frontend medias in a `medias` folder, following the same naming scheme as VPX and plugins does, prepended by the media type in brace. The predefined media types are:
  - `(Playfield)`, `(Backglass)`, `(DMD)`, `(FullDMD)`, `(Topper)`, `(Other)`, `(RealDMD)`, `(RealColorDMD)`: medias for the corresponding displays
  - `(Wheel)`, `(GameHelp)`, `(GameInfo)`, `(Loading)`: wheel image, rule card, game flyer, loading video
  - `(Audio)`, `(AudioLaunch)`: audio when showing/loading
- Have a common information file distributed with the table file, following the same naming scheme with a `.info` extension. This file is meant to be used by frontends as a distributed database, allowing to avoid additional installation steps when a new table is added, also allowing multiple frontends to share the same datas (for example, setup with 2 frontends with one for VR and one for desktop). The json file is read and written by frontends with a set of predefined or custom frontends fields:
  - the 'Info' field contains main static informations relating to the file/folder they apply.
  - The 'User' field contains user related informations at user/frontend discretion.


The following is an example of the proposed information file format (fields could be freely added/removed):
```
{
	"Info": {
		"Title": "...",                    <= Display title
		"Description": "...",              <= Additional short description
		"Authors": ["xxx", "yyy"],         <= List of the authors
		"Manufacturer": "...",             <= Reference to the manufacturer of the real table that inspired this file, or name that the authors chose to 'brand' themselves
		"Year": 2026,                      <= Year of production of the first real table that inspired this file, or year of first sharing of this file
		"VPSId": "...",                    <= [Virtual Pinball Datasheet](https://github.com/xantari/VPS.Database/) Id if this file is referenced in this database
		"IPDBId": "...",                   <= [IPDB](https://www.ipdb.org/) Id, which gives a reference to a real table that this virtual table is linked to (recreation, inspired by, ...)
	},
	"User": {
		"Rating": 8,                       <= number between 0 (unrated) to 10
		"Favorite": 0,                     <= flag between '1' and '0'
		"LastRun": "2023-11-18T23:00:00Z", <= date of last start or undefined
		"StartCount": 58,                  <= number of start
		"RunTime": 11530,                  <= cumulated duration of running (seconds)
		"Tags": ["xx", "yy"]               <= list of user defined tags
	}
}
```


<sub><sup>[Information applicable to version 10.8.1 Beta]</sup></sub>
