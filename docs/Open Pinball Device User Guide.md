# Using an Open Pinball Device in VPX

[Open Pinball Device](http://mjrnet.org/pinscape/OpenPinballDevice/OpenPinballDeviceHID.htm) 
is a new kind of USB HID interface designed specifically for virtual pinball cabinet I/O
controllers.  

OPD is essentially brand new as of this writing (September 2024, during the development
phase of Visual Pinball 10.9.0), and it's still in the experimental stages.  Its goal is
to provide a better alternative to the traditional joystick/game controller emulation that
all pin cab I/O controllers up to now have used.  "Better" in two main ways...

First, pretending to be a joystick can sometimes create compatibility problems for people
who play other kinds of video games on their PCs apart from pinball simulators.  This
isn't really the fault of the other games, either: USB HID devices are supposed to use
interface abstractions that match the physical functions they perform, and pin cab I/O
controllers that pretend to be joysticks are in flagrant violation of that principle.  So
the first benefit of using an interface that's explicitly for pinball devices is that it
won't confuse non-pinball video games that are looking for joysticks.  

The second benefit is that it can reduce the amount of manual configuration work required
to set up a pinball simulator like VP, because a simulator can know for sure that an Open
Pinball Device is actually a pin cab controller, and not some random joystick.  It can
also know that the nudge input from the device is actually a nudge input, the plunger
input is actually a plunger input, and so on, without requiring the user to map all of
that out via a dialog box.


## Compatible devices

As of this writing, the only pin cab I/O controller that implements the Open Pinball
Device interface is Pinscape Pico, which is still in development.

The interface is fully documented in a public, open-source specification
([http://mjrnet.org/pinscape/OpenPinballDevice/OpenPinballDeviceHID.htm](http://mjrnet.org/pinscape/OpenPinballDevice/OpenPinballDeviceHID.htm)),
so the hope is that other open-source and commercial devices adopt it and that it eventually becomes
widely implemented in devices.  Write to your device developer and ask them to add support
today!


## How to set up an Open Pinball Device in VP

An Open Pinball Device can supply nudge, plunger, and button inputs to the PC.  VP lets
you configure any combination of these as Open Pinball Device inputs.  You can freely mix
inputs from OPD controllers with inputs from traditional joystick/game pad interfaces.

When VP detects an Open Pinball Device, it offers to apply a default layout: the pinball
buttons, plunger position and speed, and nudge speed X/Y.  The offer can be disabled with
`Input/Device.OpenPinDev.NoAutoLayout`.

To map inputs manually, use the in-game menu.  Each OPD input is listed by name:
**Plunger Position**, **Plunger Speed**, **Nudge X/Y Acceleration** and **Nudge X/Y Speed**
(device "OpenPinDev").  Select them in **Plunger Settings** (plunger sensor page) and
**Nudge & Tilt Settings** (nudge sensor page).  Buttons are mapped in
**Input Settings** > **Action mappings**.

Open Pinball Device support is not available in the mobile and web builds.  On Linux and
macOS, the device signature string cannot be read, so any HID Pinball Device with the
matching report layout is accepted as struct version 1.0.

The Open Pinball Device interface defines two kinds of buttons: "pinball" buttons and
"generic" buttons.

The "pinball" buttons are a set of pre-assigned buttons that cover the specific special
pinball functions that are common to most pinball simulators, such as "Start", "Left
Flipper", and "Coin In".  Consult your I/O controller's documentation for a list of
available buttons and instructions on how to assign physical buttons to the pinball
functions.  The default layout maps each OPD pinball button to the matching VP action.

The "generic" buttons are an extra set of up to 32 buttons that are simply given numbers,
#1 to #32, with no special pre-assigned meanings.  In VP, they appear as **Button #1** to
**Button #32** of the OpenPinDev device and are mapped like any other button in
**Input Settings** > **Action mappings**.  They are distinct from joystick buttons with the
same number.


## How to mix Open Pinball Devices and traditional joystick/game pad devices

VP lets you assign each type of input independently, so you can freely mix Open Pinball
Devices with other devices that implement the traditional joystick interface.

For example, suppose that your plunger is connected to an Open Pinball Device unit, but
your accelerometer is connected to a separate device that only has a joystick interface.
In **Plunger Settings**, you'd map the plunger sensor's Position and Velocity sensors to
**Plunger Position** and **Plunger Speed** (OpenPinDev), and in **Nudge & Tilt Settings**
map the nudge sensor axes to whichever joystick axes your other device uses for its
accelerometer reports.


## Multiple Open Pinball Device units

Only one Open Pinball Device unit is currently supported.  With several units, their
inputs are not merged:

* all units share one VP device id ("OpenPinDev");
* for each axis (plunger, nudge), the last value that changed wins, whichever unit sent it;
* buttons are forwarded as press/release events, so a release on one unit releases the
  action even if the same button is still held on another unit.

If you have several OPD controllers, enable the Open Pinball Device interface on one of
them only, or make sure each sensor and button is reported by a single unit.

TODO: merge multiple OPD units (per-sensor selection, buttons ORed across units).
