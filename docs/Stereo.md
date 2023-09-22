# Stereo Rendering

Visual Pinball X supports rendering to a stereo output which can be one of:
- a VR Headset
- a 3D TV using passive/active glasses,
- a traditional screen, using anaglyph glasses.

Two rendering modes are available:
- Fake Stereo: a single image is rendered from a single point of view, then a 'fake' view for each eye is built from the depth informations. This mode needs a less powerful setup but has graphic glitches (like not properly handling transparency).
- Real Stereo: one image per eye is rendered and presented to the user. For the time being this mode is only supported by the OpenGL flavor of VPX.

## Anaglyph Stereo

[Anaglyph](https://en.wikipedia.org/wiki/Anaglyph_3D) is a clever and cheap way of presenting stereo rendering to a user by using color filters. The player looks at an image through glasses with 2 different colored filters, leading his eyes to perceive 2 different colored images that are then merged by his brain into a single 3D perceived scene.

### Pros and cons

The method is great since it is cheap, widely available, and can be used without a headset cutting the player from his surroundings.

It also suffers from some well-known and documented drawbacks:
- "ghosting" can be perceived when an eye partly perceive the image targeted at the other eye,
- "retinal rivalry" may happen when an object is only perceived by one eye leading the brain to reject it and not merge the images,
- darkening of the scene is expected since each eye will only receive a share of the luminance they would receive without the color filters,
- biased colors are expected since each eye perceive a subset of the original color spectrum (which are then combined back by the brain, not always leading to the original color spectrum),
- incorrect perceived colors may happen since the brain has to merge 2 images with different color spectrum, and may balance them differently depending on the viewer,
- some eye strain is also expected as the consequence of requesting the brain to merge the 2 different images, and because accomodation (focal length) and convergence (eye focus) do not perfectly match.

Still, for the application of playing virtual pinball, especially using a virtual cabinet, this technique is well suited since our use case has:
- consistency between displayed scene depth and display position, making convergence vs accomodation problem limited,
- limited depth budget needs: nearest from farthest points have a limited distance, making image merging easier to viewer,
- mostly static scene, making the merging less demanding and eye-straining.

### Anaglyph composition filters

To improve rendering and limit the aforementioned drawbacks, VPX uses filters which can be selected through the [Live UI](LiveUI.md) settings (allowing direct visual feedback). The proposed filters are the following:
- "None": no filtering, just apply the glasses colors in the most basic way (for example, full red to left, full green+blue to right). This filter does not uses the calibration data except for identifying glasses. This filter is kept for reference only and should not be used.
- "Dubois": apply [Dubois](https://www.site.uottawa.ca/~edubois/anaglyph/) method for composing the anaglyph. This method may work well if your glasses / display / eyes characteristics are similar to the one Dubois used when optimizing his filter. This filter does not uses the calibration data except for identifying glasses.
- "Deghost": apply [vectorform.com](http://iaian7.com/quartz/AnaglyphCompositing) method for anaglyph composition. Like Dubois method, this filters uses prefitted filters and will only work well if your glasses / display / eyes have similar characteristics as the ones of the original author.  This filter does not uses the calibration data except for identifying glasses.
- "Luminance": apply [Anaglyph Stereo without Ghosting](https://www.visus.uni-stuttgart.de/en/research/computer-graphics/anaglyph-stereo/anaglyph-stereo-without-ghosting/) method for anaglyph composition. This filters first needs to be calibrated to measure your glasses / display / eyes characteristics. With these data, it will optimize the rendering to your own precise characteristics in order to avoid ghosting while trying to preserve the overall brightness.
- "Dyn.Desat" a.k.a. "Luminance with Dynamic Desaturation": this filter is an improved version of the "Luminance" filter adding dynamic desaturation to avoid retinal rivalry. Retinal rivalry happens when an eye perceive an object the other don't, making the merging difficult/impossible to the brain. The solution applied here is to dynamically desaturate the parts of the image where this problem arise. For example, a pure red cube may not be seen through perfect red/cyan glasses without facing retinal rivalry since the right eye would filter it completely. In this situation, the filter will partly desaturate the red cube into a brown cube to solve the problem.

| Filter    | Calibration | Ghosting | Retinal Rivalry | Brightness | Colors |
| --------- | ----------- | -------- | --------------- | ---------- | ------ |
| None      | No<sup>(1)</sup> | High | High | Low | Good |
| Dubois    | No<sup>(1)</sup> | Low | Moderate | Medium | Bad |
| Deghost   | No<sup>(1)</sup> | Low | Moderate | Medium | Medium |
| Luminance | Yes  | Very Low | High | Good | Good |
| Dyn.Desat | Yes  | Very Low | Very Low | Good | Medium |

<sup>(1)</sup> Calibration is only used to identify the pair of filters of the glasses against a set of standard filters: red/cyan, green/magenta or amber/blue.

### Calibration process

The calibration process consists in measuring the perceived luminance through the anaglyph glasses of each eye for each color channel. This luminance depends on your display settings and characteristics, as well as your glasses and eyes. The process used is the one described in ["Face-based Luminance Matching for Perceptual Colormap Generation"](https://people.cs.uchicago.edu/~glk/pubs/pdf/Kindlmann-FaceBasedLuminanceMatching-VIS-2002.pdf).

This calibration is needed to get good result when using the "Luminance" and "Dyn.Desat" filters.

The calibration is only available from the [Live UI](LiveUI.md) (in the Settings / Video Options menu). Once started, you will have to close one eye, and adjust the brightness of an image until the 2 presented faces are equally bright (which happens when your focus naturally switch from one to the other). The Left/Right Shift keys are used to adjust the brightness, and the Left/Right Control keys are used to move between calibration steps.

Once the calibration is performed, VPX will search to fit its model against the measured values. While doing so, the screen color will change and flash until finding a suitable equilibrium.

If the measured values are not valid, VPX will not find an equilibrium point. In this situation you may redo the calibration or press the "Reset" button to get back to the default values.

### View setup

When using stereo rendering on a virtual cabinet, a key point for the result to look "good" and for the brain to gently merge the 2 colored images is that the view setup should have as little differences as possible from a real pinball setup.

In VPX 10.8, the [View setup](<View Setup.md>) has been updated to allow this. For virtual cabinet, a new 'Window' mode has been created and should be used for stereo rendering.

When seting up the view, the cabinet screen (a.k.a. the 'window') is unlikely to have the same size and form factor as the simulated pinball glass. In this situation some scaling of the real pinball table is applied to make it fit inside the screen. When doing so, searching to maintain the overall proportions - that is to say having the same, or at least similar, scaling factor on the x, y and z axis - should lead to better results.



<sub><sup>[Informations applicable to version 10.8 Beta]</sup></sub>
