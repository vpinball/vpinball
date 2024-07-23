# Visual Pinball X Flavors

VPX comes in 2 flavors: either a DirectX 9 build or an OpenGL build. The main differences are described here.

A preliminary standalone player is also available that runs on a lot of platforms: Linux, Mac, iPhone, Android, RaspBerry,...

## DirectX 9
This is the original flavor of VPX which has been developped during the last 20+ years.

It runs on all Windows builds starting from Windows XP and is the reference build.

The following features are only offered by the DirectX 9 version:
- 10 bit rendering (note that VPX does not support HDR output, so this limits the gradient banding effects but does not change the brightness range)
- Requesting a maximum frame latency to be honored by the GPU driver.

## OpenGL
This build was initially started to add support for Virtual Reality Headsets. It was merged back into 10.8, adding a bunch of unique features. It needs a computer with OpenGL 4.3 and is unlikely to run on older Windows version (anything before Windows 10 may work but is not tested nor supported).

The following features are only offered by the OpenGL version:
- Virtual Reality support (via OpenVR),
- Real stereo rendering for 3D-stereo TVs or anaglyph glasses,
- Hardware Multisample Antialiasing (MSAA) with support for antialiasing of pre-rendered and dynamically rendered parts,
- Hardware Adaptive sync (a way of synchronizing between CPU and GPU, which leads to slightly better synchronization when using the 'Adaptive Sync' settings in the video preferences),
- Enhanced texture compression (gives more performance with lower quality impact).

## Standalone player
The standalone player is a preliminary build meant to allow playing VPX tables on a large set of platforms, but without the windows UI. It is targeted at devices devices supporting OpenGL 4.1 (desktop) or OpenGL ES 3.0 (mobile and sbc) with GLSL 3.0.

<sub><sup>[Information applicable to version 10.8 Beta]</sup></sub>
