# bgfx-imgui

This code is a port of a [Gist](https://gist.github.com/RichardGale/6e2b74bc42b3005e08397236e4be0fd0) by Richard Gale.

It remains mostly intact other than a few fixes to build with the latest release of `bgfx` and all input handling has been removed so that `imgui_impl_sdl.h/cpp` implementations may be used instead.

`vs_ocornut_imgui.bin.h` and `fs_ocornut_imgui.bin.h` are copied from the [examples/common/imgui](https://github.com/bkaradzic/bgfx/tree/master/examples/common/imgui) of the [bgfx](https://github.com/bkaradzic/bgfx) repo (to make the use of `bgfx::EmbeddedShader` easier).
