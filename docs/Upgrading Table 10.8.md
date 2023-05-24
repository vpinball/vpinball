# Upgrading a table to 10.8

VPX 10.8 tries to be entirely backward compatible. Therefore, existing tables should look exactly the same as with 10.7, excepted for the 3 following points:
- parts below the playfield used to be invisible before 10.8 unless they were tagged as 'Active', now they will be visible,
- reflections used to be rendered differently, so they will likely look somewhat different than before,
- before 10.8, scripts were not allowed to adjust static parts at all, now this is possible during table initialization, therefore tables that would tweak static part and rely on the fact the changes to not be processed will need to be updated.

Beside these eventually needed updates, 10.8 offers new features that can be leveraged by updating the table. The main ones are described below.

## Table of Contents
1. [Ball Shadows](#ball-shadows)
2. [Ball Reflections](#ball-reflections)
3. [Playfield transparency](#playfield-transparency)
4. [Playfield Cutouts and Lower Playfield](#playfield-cutouts-and-lower-playfield)
5. [Reflections](#reflections)
6. [Better light fading](better-light-fading)
7. [Simplified scripting](#simplified-scripting)
8. [Better statics](#better-statics)
9. [Additional DMD](additional-dmd)

## Ball Shadows
Dynamic raytraced ball shadows were added to lights. To enable them, just enable the corresponding checkbox in the light properties.

![Option](img/Upgrade10.8-BallShadowsOption.png)

![No Shadows](img/Upgrade10.8-BallNoShadows.png)
![Shadows](img/Upgrade10.8-BallShadows.png)

Left: no shadow / Right: with ball shadow

## Ball Reflections
Ball used to be rendered using a 'sphere map' that have the drawback of being view independent, therefore sometime looking wrong when the viewer moved (for example in VR or with headtracking). They can now used an 'equirectangular map', that is to say the same type of texture as for the environment lighting.

![Option](img/Upgrade10.8-BallMapOption.png)

![Sphere Map](img/Upgrade10.8-BallSphereMap.png)
![Equirectangular Map](img/Upgrade10.8-BallEquirectangular.png)

Left: sphere mapping / Right: view dependent equirectangular mapping

## Playfield Transparency
Before 10.8, playfield did only support binary transparency, that is to say either opaque or fully transparent based on the alpha threshold value defined on the playfield image. Now, alpha blended transparency is fully supported allowing better rendering of tranparent parts like inserts.

There is nothing special to do to use it. With this feature, the alpha mask property of images should always be set to 1. It is mainly kept for backward compatibility.

![Option](img/Upgrade10.8-AlphaMask.png)

If you are using a custom 'playfield_mesh' named primitive for the playfield visual, all the properties are now processed like for other primitives. Especially the 'Disable Lighting From Below' is now supported.

## Playfield Cutouts and lower playfield
Before 10.8, parts under the playfield needed to be created separately, using an 'Active' material. This restriction does not apply any more. Therefore, you can create a playfield primitive with its nice beveled cutouts, name it 'playfield_mesh' to get good looking cutouts.

The same applies to lower playfield. These used to need an active material and could not be marked as static. This does not apply anymore.

## Reflections
Reflections used to be limited to playfield, with no VR nor camera mode support, not taking in account the actual playfield geometry (parts above a hole or outside of the playfield would reflect just like if there was a playfield beneath them).

In 10.8, reflections are done using 'reflection probe', that is to say rendered using 2 'passes': the first one evaluate the reflected incoming light, the second actually draw the object using this incoming light.
This offers teh following benefits:
- it supports VR (and also camera mode & LiveUI),
- it allowed to process the playfield like any [other parts](playfield-transparency),
- it allows to use reflections on any part,
- reflections are rendered with the object they are applied to, therefore taking in account its geometry: no more reflections in the playfield hole,
- while implementing this feature, an additional 'roughness' parameter was added to allow blurred reflections.

If the table uses the default playfield (i.e. does not have a 'playfield_mesh' named primitive), VPX will handle the reflection probe based on user and table setting.
For backward compatibility, when loading an old table with a custom 'playfield_mesh' primitive, VPX will create a default playfield renderprobe and apply it to this primitive.

To define a reflection probe, in the new 'Reflection Probe manager, you set up the reflection plane by giving its normal and position along this normal.

![Option](img/Upgrade10.8-ReflectionsOption.png)

The default table offers an example of a table with 3 reflection probe: one for the playfield, and one for each of the cabinet side.

![Example](img/Upgrade10.8-ReflectionsExample.png)

## Better light fading

Light fading was updated and now offers 3 modes:
- None: no fading,
- Linear: linear interpolation, like in previous versions of VPX,
- Incandescent: fading according to a physical model of a buld (compute the evolution of the temperature of the filament over time).

![Option](img/Upgrade10.8-LightOption.png)

If you are using a custom fader (implemented in your script), then the 'None' fading mode is your friend. Otherwise, the 'Incandescent' mode should give better results than the 'linear' mode.

![Fading](img/Upgrade10.8-LightFading.gif)

## Simplified Scripting
A new event called `Animate` was added to most animated parts. It allows to simplify the script written to sync custom visual parts with animated one.

For example, the following code from VPX 10.7:
```
Sub Table1_KeyDown(ByVal keycode)
	If keycode = LeftFlipperKey Then
		LeftFlipper.TimerEnabled = True 'This line is only for ninuzzu's flipper shadows!
		LeftFlipper.RotateToEnd
	End If
End Sub

Sub LeftFlipper_Init()
    LeftFlipper.TimerInterval = 10
End Sub

Sub LeftFlipper_Timer()
    FlipperLSh.RotZ = LeftFlipper.CurrentAngle
    If LeftFlipper.CurrentAngle = LeftFlipper.StartAngle Then
        LeftFlipper.TimerEnabled = False
    End If
End Sub
```

can be replaced by:
```
Sub Table1_KeyDown(ByVal keycode)
	If keycode = LeftFlipperKey Then
		LeftFlipper.RotateToEnd
	End If
End Sub

Sub LeftFlipper_Animate()
    FlipperLSh.RotZ = LeftFlipper.CurrentAngle
End Sub
```

Custom fading of lights and flashers can also be largely simplified by using the new [light fading](#better-light-fading), leveraging the new 'lightmap' property of flashers and/or using the 'Animate' event .


## Better Statics
VPX 10.8 allows the script to update 'static' parts properties during initialization, that is to say in the main script part and in the 'Init' events of the table and parts. This can be leveraged to adjust visiblity of static parts depending on the running situation or selected options.

For example:
```
' Detect if VPX is rendering in VR and adjust part visibility of parts in VR_Cab collection
Dim VRThings
If RenderingMode = 2 Then
	For Each VRThings In VR_Cab
		VRThings.visible = 1
	Next
Else
	For Each VRThings In VR_Cab
		VRThings.visible = 0
	Next
End If
```

## Additional DMD

VPX 10.8 added the ability to setup a custom DMD to any flasher object.
This allows to have multiple DMDs, for example to add an ingame option menu, or for an additional DMD (like for Monopoly pinball).

<sub><sup>[Informations applicable to version 10.8 Beta]</sup></sub>