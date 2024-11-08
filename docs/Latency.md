# Latency

One very impacting thing when emulating Pinball is latency. Therefore, VPX has been optimized for very low latency, but each setup is different and the following informations are here to help to understand, measure and adjust settings accordingly.

Latency is the delay between a stimumus and its consequence. For VPX, there are 3 types of latency to consider:
- **Input latency**: the delay between when an input event happens and when it is taken in account by VPX.
- **Physics/Emulation latency**: the delay between the instant when something happens (balls roll over a switch for example) and the emulation is aware of it.
- **Visual latency**: the delay between when the game is in a state and when this state is displayed on the screen.

Each of these latencies present different challenges which are described below. The informations given here are mostly relevant to BGFX variant as it leverages BGFX multithreading capabilities for better latency results, whereas OpenGL and DX9 variants have higher latencies due to their single threaded nature.


## Input latency

Input is poled continuously, targeting a resolution below 1ms, and directly fed to VPX engine which is ran at a 1ms step precision. The average and maximum input latency is displayed when pressing F11 (delay between last poll and when it is taken in account by the engine, excluding the OS/USB/hardware latencies).


## Physics/Emulation latency

When a physic state change causes a switch to change its position, its state must be adjusted, the emulated controller must detect it and react accordingly, then the corresponding reaction must be applied back to the physics engine.

For example in Williams' WPC Fliptronic, when the user presses a flipper button, it is detected by the WPC CPU which in turns activates the flipper solenoid, the overall process taking about 0.5 to 1ms.

To be as close as possible to this behavior, PinMame controller emulation and physics emulation are run continuously, targeting a maximum of 0.5ms of sync difference between them.

An optimized emulation of the flipper solenoids nicknamed 'FastFlips' is also implemented for most tables. It lowers the latency even below this value by immediatly performing what the emulated CPU controller will do. It is really needed when running DX9/OpenGL variants where input is (mostly) sampled at the framerate (usually every 16.6ms when running on a 60Hz display).


## Visual latency

Visual latency impacts player reaction, as reacting to things that haven't been displayed yet is difficult. Therefore, it directly matters to keep it as low as possible. This latency is somewhat complex as it implies the following steps:
- First, VPX creates a list of commands to render the scene, it captures the state of the game at this point in time,
- Then, VPX submits it to the GPU, eventually waiting for the GPU to be in a state where it can accept these commands (GPU have a limited number of 'frames in flight', that is to say frames pending to be rendered),
- The GPU performs the rendering, eventually waiting for a backbuffer to be available to render on it (one of the buffers used for rendering),
- This rendered backbuffer must be composed with other parts of the desktop (or just push forward when the window covers the full screen),
- The resulting image (VPX + desktop) must be sent to the display, eventually syncing on the display 'VSync' to avoid tearing,
- The display must display it

All these steps take time and, in the end, the displayed image correspond to a game state that happened quite some time ago.


There is no way to display the visual latency directly inside VPX. To know your visual latency, you may install the opensource application [PresentMon](https://github.com/GameTechDev/PresentMon/releases). You may then ran it in a command prompt and it will display the overall framerate and latency.

To optimize this value, you will likely want to run the BGFX variant, using a window that covers the entire screen (or fullscreen mode which does the same). In this situation, PresentMon should mention that 'PresentMode' is 'Hardware: Independent Flip' and the latency should be a bit more than your display rate (for example, on a 144Hz display, that is to say 1/144 = 7ms per frame, latency should be around 8ms). Note that all hardwares do not support these optimized latency modes.


## Overall scheme

The scheme below illustrates these elements with a flipper state change:
1. move fingers, press button, mechanical contact, electric signal available to pinball input device (mostly instantaneous)
2. pinball input device process, then communicates with computer through USB
3. computer input API process and makes this data available to VPX
4. VPX process input and report it to script which updates a switch state table for PinMame
5. PinMame reads switch state table and emulate Controller CPU, in turns toggling a virtual solenoid output
6. VPX synchronize with PinMame and report solenoid state change to script which request physic engine to move flipper
7. VPX's physics engine actually move the flipper

Note that with 'FastFlips' steps 5 and 6 are skipped but all the others remain.

![Latency](img/Latency.svg)


<sub><sup>[Information applicable to version 10.8.1 Beta]</sup></sub>
