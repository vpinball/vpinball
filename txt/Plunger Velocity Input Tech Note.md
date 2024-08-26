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
but it's configurable in VP to any of 8 axes (X, Y, Z, RX, RY, RZ,
or either of two "slider" controls).

The speed reading can be passed to the PC in the same manner,
alongside the position report, by placing it on any other joystick
axis not already used for plunger position or accelerometer readings.
It seems pleasingly symmetrical to adopt RZ as the conventional
default for the speed data, mirroring the convention of using Z for
the position report.

Note that the speed reading is **not** a replacement for the position
report.  The simulator still needs the position reports; the speed is
only a supplement.


## How VP consumes the new speed readings

VP has a new control in the "Keys" setup dialog that lets you select
a joystick axis to assign as the Plunger Speed input.

VP continues to use the USB position reports to update the on-screen
plunger animation and the "hit plunger" physics object, exactly as in
previous versions before speed reports were added.  In fact, VP still
uses *only* the USB position reports to track the position *and speed*
of the "hit plunger" object, so that code is essentially unchanged.

The reason **not** to use the speed reading to update the position is
that we already know the **actual** instantaneous position, from the
USB reports.  Calculating the position from the velocity would
effectively integrate the velocity readings over time, which would
magnify any measurement error component by adding the error into the
running total (the position) on every time step.  In contrast, the
position reading is always an instantaneous "state of the world"
reading, so any measurement error it contains is a one-off that's
discarded on the next time step.

The place where the external speed reading is applied is in the
collision processing, to calculate the impulse imparted to the ball.
Here, it's the external speed that's the instantaneous "state"
reading, so we use this in preference to the internal speed
calculation, which is imperfectly calculated from the first
derivative of position readings over time.


### Firing events are suppressed when speed readings are available

"Firing events" are the heuristic that VP uses to improve its speed
calculations for fast-moving plunger events.  VP declares a firing
event when it detects two sequential readings that indicate that the
plunger has moved forward at a high enough speed that it's likely
being driven forward by the main spring.  A typical mechanical
plunger takes about 30ms to travel all the way forward after being
pulled back and released, so VP has good odds of seeing at
least two USB reports over a firing motion, allowing it to detect
most of these events.

Two reports isn't enough to accurately calculate speed, for the reasons
discussed above - the aliasing problem at the bounce-back point, and
the absence of precise delta-time information.  But two readings *are* enough to 
tell that some kind of high-speed action occurred, even if we can't determine the speed
with any precision.  That's why this is a heuristic.  VP assumes from the
apparent rapid motion that the user did in fact pull back the plunger
and release it.  So it tries to guess at the actual speed the plunger
reached *over the course of the forward travel* by looking back over
recent reports to see how far back the plunger was at its maximum
excursion.  It's likely that the *retraction* phase, where the user
was manually operating the plunger, was slow enough that VP has fairly
detailed readings on the position, so it can figure the apex position
(or something close) by looking at recent readings and taking the peak
value.  VP then assumes that the plunger actually was released from
this position, so it calculates the speed that would occur in an ideal
system under these conditions.  It then uses that speed to figure the
ball collision impulse.

The whole point of this heuristic is to get a better estimate of the
speed for times when VP knows it can't properly calculate the speed
by taking the first derivative of the position readings.  This is why
the new system disables the heuristic when external speed readings are
available.  The external speed readings are presumed to reflect the
reality of the mechanical system more accurately than the heuristic
can, so the external speed is used in preference to the heuristically
synthesized speed.

In addition, it's desirable to disable the firing-event heuristic when
it's not needed because the heuristic can be fooled pretty easily by
actions that aren't actually release motions, but which the heuristic
interprets as such anyway.  If you move the plunger rapidly forward by
hand and then pull it back, for example, you'll trigger a firing event
even though you didn't actually release the plunger.  That'll make the
on-screen plunger go through the full firing travel distance even
though the actual plunger has already been pulled back, so the
on-screen motion will diverge from the real motion.  Disabling the
heuristic when it's not needed keeps it from producing weird results
when the user accidentally fools it.

## References

[Improving the Virtual Pin Cab Input Model](http://mjrnet.org/pinscape/OpenPinballDevice/NewPinCabInput.htm):
A more complete presentation of the ideas here, along with related
changes for accelerometer (nudge) input, aimed at both simulator
developers and device developers.
