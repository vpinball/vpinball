# Using an Open Pinball Device in VPX

[Open Pinball Device](http://mjrnet.org/pinscape/OpenPinballDevice/OpenPinballDeviceHID.htm) 
is a new kind of USB HID interface designed specifically for virtual pinball cabinet I/O
controllers.  

OPD is essentially brand new as of this writing (September 2024, during the development
phase of Visual Pinball 10.8.1), and it's still in the experimental stages.  Its goal is
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

For the "axis" inputs - plunger and nudge - you connect an axis to an Open Pinball Device
by selecting **OpenPinDev** in the drop-down list for the axis.  For example, if you want
VP to read nudge input from your Open Pinball Device, set the Nudge X and Y axis drop
lists to **OpenPinDev**.  When you assign an axis to **OpenPinDev**, there's no need to
select *which* axis to assign (the way that you have to select among X, Y, RX, RY, etc for
a joystick), since the pinball device labels each of its inputs with the specific pin cab
function it performs.

The Open Pinball Device interface defines two kinds of buttons: "pinball" buttons and
"generic" buttons.

The "pinball" buttons are a set of pre-assigned buttons that cover the specific special
pinball functions that are common to most pinball simulators, such as "Start", "Left
Flipper", and "Coin In".  Consult your I/O controller's documentation for a list of
available buttons and instructions on how to assign physical buttons to the pinball
functions.  For a physical button assigned to an OPD pinball button function, there's
nothing to configure in VP, because VP knows exactly which VP function to invoke when you
push that button.

The "generic" buttons are an extra set of up to 32 buttons that are simply given numbers,
#1 to #32, with no special pre-assigned meanings.  In VP, these work exactly like joystick
buttons.  In the VP Keys dialog, you assign these using the same numbering assignments as
joystick buttons.  Note that VP doesn't have a way to distinguish between "Joystick button
#1" and "Open Pinball Device button #1", so if you're wiring your button inputs through
multiple physical devices, you'll have to take care to assign a unique number to each
button across your whole system.


## How to mix Open Pinball Devices and traditional joystick/game pad devices

VP lets you assign each type of input independently, so you can freely mix Open Pinball
Devices with other devices that implement the traditional joystick interface.

For example, suppose that your plunger is connected to an Open Pinball Device unit, but
your accelerometer is connected to a separate device that only has a joystick interface.
In the VP Keys setup dialog, you'd assign the Plunger Position and Plunger Speed inputs
to **OpenPinDev**, and assign the Nudge X/Y axes to **X** and **Y**, or whichever
joystick axes that your other device uses for its accelerometer reports.


## How to handle multiple Open Pinball Device units

If you have multiple physical devices that all implement the OPD interface, Visual Pinball
simply treats them "additively": it literally adds up the readings on each input across
devices.  That means there's no need to select *which* device to use for an axis that you
assign as **OpenPinDev**, since adding up all of the sources should automatically find the
one that's actually sending data on each input.

The reason we can use this simple strategy is that all of the pinball inputs are naturally
singular.  A pin cab just naturally has one accelerometer, one plunger, and one of each
button.  There's no use case where you'd expect to have multiple nudge accelerometers or
multiples of the other inputs.  All of the units that *don't* have accelerometers
attached will send zeroes on the accelerometer inputs, so adding those zeroes into the
total across devices won't change the numbers coming in from the one unit where the
accelerometer actually is attached.

Note that this doesn't mean that you can't have additional accelerometers in your system
for *other purposes* besides pinball nudging, since those will presumably be connected to
other kinds of controllers that aren't presenting themselves as pinball device inputs.
We're just saying that there's no reason you should have two *pinball nudge sensor*
accelerometers in a single cabinet.
