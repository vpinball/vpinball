# Open Pinball Device - VP Setup Guide

This note describes how to set up an Open Pinball Device interface
in VP.

The Open Pinball Device is a new USB HID interface created specifically for
virtual pin cab use.  It's designed to offer an alternative to the
traditional ad hoc gamepad/joystick interface that pin cab I/O controllers
have used up until now.  The point of the new interface is to eliminate the
conflicts that the joystick setup can sometimes cause with non-pinball video
games, by defining an interface that explicitly identifies itself as a
pinball controller, which non-pinball video games won't confuse with other
types of input devices.  This is how USB's HID system was supposed to work;
the traditional approach of impersonating a joystick was expedient for the
early device developers, but it violates the basic HID principle that
devices should identify themselves according to their function, which is why
some people run into conflicts; some games take devices at their word and
expect joysticks to actually be *joysticks*.  Defining a pinball-specific
interface also makes it possible for simulators to make the configuration
process a lot more automatic, since the data inputs in the pinball interface
are specifically identified by their pinball functions: accelerometer,
plunger, buttons.  The simulator doesn't have to depend upon you to manually
map all of the inputs from "X" and "RX" and so forth, because they're
already labeled according to their actual function - again, as the creators
of HID always intended.

For technical documentation on the interface design, see the
[Open Pinball Device HID Specification](http://mjrnet.org/pinscape/OpenPinballDevice/OpenPinballDeviceHID.htm).


## When to use

You can only use the Open Pinball Device interface in Visual Pinball if
your pin cab I/O controller device implements it (and you've enabled it
in the controller's configuration, if necessary).  Compatible devices
include:

* Pinscape Pico: To enable the feature, set the property **openPinballDevice.enable**
to true in your configuration.  (Note that this feature is **not** currently
available in the original KL25Z Pinscape - it's only in the Pico version.)


## VP Setup

Use the "Keys" dialog to set up the controller inputs.

* If your device has an accelerometer, set the **X Axis (L/R)** and **Y Axis (L/R)**
to **OpenPinDev**, and check the box **Enable Analog Nudge**.

* If your I/O controller supports velocity-based nudge input, check the box **Analog Nudge Input is Velocity**.
(Pinscape Pico supports this feature; for other controllers, check the controller's documentation.)

* If your device has a plunger sensor, set the **Plunger** and **Pl. Speed**
entries to **OpenPinDev**.


## Mixing inputs from other devices

If you have multiple I/O controllers, some with Open Pinball Device support and some without,
it's easy to combine them in the VP setup.  Simply select the axis assignments according to
which device has which type of sensor input.  For example, if your accelerometer is attached
to the Open Pinball Device controller, set the nudge X and Y axes to **OpenPinDev**; if the
plunger is on a separate device that uses the traditional joystick Z axis input, set the
plunger axis to **Z**.  VP will only use the Open Pinball Device inputs on the axes that
you assign to it, so you can still easily map any of the axes to other inputs.
