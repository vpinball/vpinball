# Lights

## Table of Contents
1. [Lightmaps](#Lightmaps)
2. [Fading](#Fading)
3. [Shadows](#Shadows)
4. [Reflections](#Reflections)

## Lightmaps
VPX renders lights using 'lightmaps' that is to say it renders a flat surface where the light illuminates.
This surface may be either an opaque surface for 'Classic' lights, or a transparent additive surface for 'Halo' lights.

You may also choose to use custom lightmaps which are made using Flasher or Primitive objects, with their 'lightmap' property pointing to a light object.
Their opacity will then be modulated by the light intensity ratio.

## Fading
When a light is switched on or off, it fades up and down. VPX is able to simulate 3 behaviors:
- Incandescent bulb, for which a complete physyical model is performed, simulating the bulb's filament temperature (which can be accessed from script, for example for dynamic tinting),
- LED bulb, which switch instantaneously to the requested state (no fading),
- Linear, which performs linear interpolation at the selected speed.

## Shadows
VPX only supports dynamic shadows between balls and lightmaps (from light objects or flasher/primitives used as lightmap).
These shadows are computed using raytracing between the ball and the light source as defined in the editor (using the defined surface offseted by the given z property).

![Light position](img/Lights-Position.webp)

The softness of the shadows depends on the distance between ball and bulb, the radius of the ball, and the radius of the bulb (which is fixed to a value of 0.1 inches).

## Reflections
Light reflections on balls are directly rendered if the corresponding checkbox is enabled in the editor.
Note that even a light set to 'None' mode for rendering (no lightmap) can have this checkbox enabled and therefore have its reflection on balls.

<sub><sup>[Informations applicable to version 10.8 Beta]</sup></sub>