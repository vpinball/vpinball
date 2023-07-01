# Visual Pinball X Flavors

VPX comes in 2 flavors: either a DirectX 9 build or an OpenGL build. The main differences are described here.

A preliminary standalone player is also available that runs on a lot of platforms: Linux, Mac, Android, RaspBerry,...

## DirectX 9
This is the original flavor of VPX which has been developped along the last 20+ years.

It runs on all Windows build starting from Windows XP and is the reference build.

## OpenGL
This build was initially started to add support for Virtual Reality. It was merged back in 10.8, adding a bunch of unique features. It needs a computer with OpenGL 4.3 and is unlikely to run on older Windows version (anything before Windows 10 may work but is not tested nor supported).

The following features are only offered by the OpenGL version:
- Virtual Reality support,
- Real stereo rendering for stereo TV or anaglyph glasses,
- Hardware Multisample Antialiasing (MSAA) with support antialiasing between pre-rendered and dynamically rendered parts,
- Hardware Adaptive sync (a way of synchronizing between CPU and GPU which leads to less stutters when using 'Frame Pacing' and 'Adaptive Sync' settings),
- Enhanced texture compression (gives more performance with lower quality impact).

<sub><sup>[Informations applicable to version 10.8 Beta]</sup></sub>