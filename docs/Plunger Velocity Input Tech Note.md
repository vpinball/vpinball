# Plunger Velocity Input Technical Note

Visual Pinball and other simulators can read an external analog sensor
that tracks the position of a mechanical plunger assembly, using the
input in the simulation to control a simulated ball shooter.

This technical note describes an extension to the traditional plunger
sensor input system that includes not only the instantaneous position
of the mechanical plunger, but also the instantaneous speed at which
it's moving.  The simulator can use this information to improve the
simulation accuracy.

## Why it's *useful* to calculate velocity externally

Put simply, it's because the ball shooter moves faster than USB
reports.

Pinball simulators like VP receive plunger position input via USB HID
joystick axis reports.  These run at about 10ms polling intervals.  A
standard pinball ball shooter assembly traverses its whole range of
travel in about 30ms when the player pulls back the plunger and
releases it to let the main spring drive it forward.  This means that
the simulator on the PC only sees one or two position reports across
the whole travel range.  What's more, the plunger bounces back a
significant fraction of its travel length when it hits the barrel
spring, so with USB samples at only 10ms intervals, it's highly likely
that the host will see "aliasing", where it *appears* that the plunger
has moved forward since the last reading, but where the plunger
actually made it all the way to the barrel spring and bounced back
part of the way.  If the simulator attempts to calculate the speed by
taking the difference of the two positions and dividing by time, it's
likely to calculate a speed that's much too low (or even in the wrong
direction) because it's unaware that the plunger has completed two
whole legs of travel in opposite directions between the two readings.

These timing characteristics make it impossible for the simulator to
accurately calculate the plunger's instantaneous speed based on the
position reports.

VP's plunger code has some heuristics that attempt to work around this
by detecting patterns of motion that resemble release events, and
synthetically calculating the speed that would result *if* a release
is indeed under way.  Like most heuristics, this works well enough in
typical cases, but it's easy to fool, and it can produce anomalies
when it guesses wrong.


## Why it's *possible* to calculate the velocity externally

Plunger position sensors are never connected directly to a PC.
They're instead always connected to some kind of external processor,
typically a microcontroller (Arduino, KL25Z, Raspberry Pi Pico) that
has the kind of electronic interfaces that sensors can attach to.  The
external processor is what connects to the PC via USB.

If VP can't calculate the plunger speed accurately, why would we
expect the intermediary processor to do a better job of it?  Simple:
the microcontroller isn't constrained by the USB HID polling cycle.
It can sample the sensor as many times per USB polling cycle as it
wants, up to the sensor's native rate.  Remember that the problem VP
has is time resolution: it simply doesn't have access to enough
samples over the typical 30ms release-motion travel time to get a
meaningful first derivative.  In contrast, the microcontroller can
collect plenty of samples over this same time period, thanks to its
higher speed access to the sensor.

Nearly all of the commonly used position sensors can be sampled at
intervals of around 2.5ms or faster, which is fast enough for accurate
speed readings throughout a ball shooter release motion.  The slowest
commonly used sensor is the VCNL4010 used in the later VirtuaPin
plunger kits, and that samples at 4ms intervals, which is still fast
enough for decently accurate speed calculations.  The most commonly
used sensors - slide potentiometers - can be sampled at ADC rates,
which can run as fast as a few microseconds per reading.


## How I/O controllers can supply speed readings

I/O controllers traditionally send plunger position reports to the PC
via a joystick axis.  This is most commonly the main joystick Z axis,
but VP can use any axis reported by the device (joystick/gamepad axes
via SDL, or the named inputs of an Open Pinball Device).

The speed reading can be passed to the PC in the same manner,
alongside the position report, by placing it on any other joystick
axis not already used for plunger position or accelerometer readings.
It seems pleasingly symmetrical to adopt RZ as the conventional
default for the speed data, mirroring the convention of using Z for
the position report.

Note that the speed reading is **not** a replacement for the position
report.  The simulator still needs the position reports; the speed is
only a supplement.

VP expects the speed in plunger travel lengths per second (p.u./s,
1 = full travel range, positive when retracting).  The device value is
converted with the mapping's **Unit scale** (Pinscape preset: 12.5 p.u./s).


## How VP consumes the speed readings

The speed input is mapped in the in-game menu: **Plunger Settings** >
plunger sensor > **Velocity sensor**.

Position: VP rebuilds a continuous 1 ms position estimate from the
timestamped position reports (Kalman estimator), and moves the on-screen
plunger toward it through the table's **Mech Strength** spring.  The
position is never integrated from the speed readings, since that would
accumulate the measurement error.

Hit speed: the speed applied to the ball is taken, in order of preference,
from:

* the external speed reading, when a Velocity sensor is mapped;
* otherwise, an estimate from the position history (see below).

The hit speed (p.u./s) is rescaled to the range between the rest position
and full retraction (x (1 - rest position)), converted to table units with
the plunger travel length, and scaled by the table's **Release Speed**/100
and the plunger mass.  It is used for the ball impulse in the collision,
and also drives the on-screen tip while a release is under way, so that
the tip reaches the ball in time and bounces back like a released plunger.
Script/keyboard firing (`Fire`) keeps using its own internal speed.


### Release estimate without a speed input

Without a speed input, VP looks at the positions of the last 100 ms for
the furthest pull-back (apex).  If the plunger is now more than 10% of its
travel forward of that apex, VP assumes a free release from the apex and
uses the keyboard-plunger launch model:
speed = -(apex - rest) x 100/13 (positions as fractions of the travel,
same scale as the keyboard plunger launch).
Otherwise the hit speed is zero and the impulse comes from the simulated
tip speed.  A mapped speed input replaces this estimate entirely.

VP up to 10.8: VP instead detected "firing events" from two fast forward
readings and then ran the on-screen plunger through a full scripted
firing stroke.  This could be fooled by fast manual movements, leaving the
on-screen plunger out of sync with the real one; the speed input
suppressed these events.  This mode no longer exists for mechanical
plungers.

## References

[Improving the Virtual Pin Cab Input Model](http://mjrnet.org/pinscape/OpenPinballDevice/NewPinCabInput.htm):
A more complete presentation of the ideas here, along with related
changes for accelerometer (nudge) input, written for simulator
developers and device developers.
