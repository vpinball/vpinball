# Visual Pinball X Flavors

VPX comes in multiple flavors that corresponds to different steps in its long development history.
The main differences between these flavors are described here.

Beside the graphic backend flavor, VPX is provided as:
- a full featured editor and player for Windows system
- a standalone player for all platforms (including Windows), targeted at devices supporting Vulkan (desktop and single-board computer), Metal (Apple devices) or OpenGL ES 3.0 with GLSL 3.0 (mobile and single-board computer).


## BGFX
This build was introduced with version 10.9.0. It uses the BGFX library to run VPX using the latest graphic backends like Vulkan, Metal, DirectX 11, latest OpenGL & OpenGL ES.
It is the most up-to-date, feature complete and actively developed flavor. This is the flavor that should be used in almost all situations.

The following features are offered by the BGFX version:
- Virtual and Augmented Reality support (via OpenXR),
- HDR display support when using DirectX 11 (displays that support high luminance output),
- Real stereo rendering for 3D-stereo TVs or anaglyph glasses,
- Enhanced texture compression (gives more performance with lower quality impact),
- Variable Refresh Rate display support,
- Multithreaded engine that decouples rendering from gameplay allowing very low and stable input latency,
- Additional output windows, for example for DMD or score displays.

This flavor is available as a window player/editor application and as a standalone player that runs on a lot of platforms: Windows, Linux, macOS, iOS, Android, Meta Quest, Raspberry Pi, web browsers (WebAssembly),...


## DirectX 9
This is the original flavor of VPX which has been developed during the last 20+ years.

It runs on all Windows builds starting from Windows 7 (building with older VC++ for XP may still be possible) and is the reference build. Nowadays, it is only used as a reference build to ensure improvements are backward compatible but it should not be used anymore as if has a limited feature set and is largely deprecated (DirectX 9 was replaced by DirectX 10 in 2006).

This flavor is only available as a window player/editor application.


## OpenGL
This build was initially started to add support for Virtual Reality Headsets. It was merged back into 10.8, adding a bunch of unique features.
It needs a computer with OpenGL 4.3 and is unlikely to run on older Windows version (anything before Windows 10 may work but is not tested nor supported).
Now that the BGFX flavor exists, there isn't that much uses of this flavor anymore.
It should be considered as deprecated and not used in any situation.

The following features were introduced by the OpenGL version:
- Virtual Reality support via OpenVR (removed in 10.9.0, VR is now provided by OpenXR in the BGFX flavor),
- Real stereo rendering for 3D-stereo TVs or anaglyph glasses,
- Hardware Multisample Antialiasing (MSAA) with support for antialiasing of pre-rendered and dynamically rendered parts,
- Hardware Adaptive sync (a way of synchronizing between CPU and GPU, which leads to slightly better synchronization when using the 'Adaptive Sync' settings in the video preferences),
- Enhanced texture compression (gives more performance with lower quality impact).

This flavor is available for Windows as a window player/editor application, and as a standalone player for macOS and Linux.


<sub><sup>[Information applicable to version 10.9.0 Beta]</sup></sub>
