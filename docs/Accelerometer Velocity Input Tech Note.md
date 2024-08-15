# Accelerometer Velocity Input Technical Note

Visual Pinball can simulate nudging based on input from an
accelerometer, via the USB HID joystick interface.  This is a key
feature for virtual pin cab users, since it seamlessly replicates the
kind of physical interaction you can engage in with a mechanical
pinball machine.

This technical note describes a new enhancement to VP's accelerometer
input model that's designed to improve the playing experience and the
realism of the simulation.  To use the new system, the USB device
supplying the accelerometer input must also be equipped with the new
mechanism.  This is purely a supplement to the traditional input
model, so it doesn't affect users with existing devices; it's just a
new alternative available to users with devices that provide the new
input data.  The new system is easy to implement on the device side;
details for device developers are included later in this tech note.


## Background: The accelerometer device-level interface

VP doesn't accept accelerometer input directly, since there aren't
any accelerometers that you can directly connect to a PC.  Instead,
the accelerometer itself is connected to some kind of intermediary
device, typically a microcontroller (such as an Arduino, KL25Z, 
or Raspberry Pi Pico), which in turn connects to the PC via USB.
VP's access to the accelerometer is by way of the USB interace,
specifically the USB HID joystick interface.  VP can be configured
to accept two axes of accelerometer input, representing the X and Y
axes in the horizontal plane of the virtual playfield, via any pair
of standard gamepad joystick axes.

This approach was invented by a couple of early "virtual pinball
controller" products, and was copied by all of the later commercial
and open-source controllers, since it's easy to implement in a
microcontroller, and is already supported in the simulators.


## The traditional simulator input model: Raw accelerations

In the traditional USB HID joystick interface that VP and other
simulators implement, the microcontroller simply passes along the
acceleration samples read from the accelerometer, with little or no
processing.  The device typically scales the readings to fit the USB
HID joystick axis range, and provides some kind of automatic
calibration to subtract out the physical tilt bias that's almost
always present (since it's nearly impossible to install an
accelerometer sensor such that it's perfectly level).  Otherwise,
the readings sent to the simulator are just the readings taken
from the accelerometer: a series of discrete-time digitized
acceleration samples.

Internally, VP applies these instantaneous accelerations to the
simulation by accelerating each moving object by the amount read from
the accelerometer, for one time step in the simulation.  The
expression of this concept in the C++ code is equally straightforward:
VP literally adds the instantaneous acceleration on each time step to
each movable object's velocity.  

To be correct in a physics sense, the instantaneous acceleration must
be multiplied by the amount of time in a time step, to yield the
incremental velocity for one time step.  The VP code has a comment
that this is missing and should be added someday, but it's really not
missing after all: it's just hidden in plain sight, by virtue of being
rolled into the user-adjustable "Gain" setting.  Each acceleration
reading from the USB device is multiplied by the Gain to yield the
amount added to the velocities, so the Gain implicitly contains the
time step factor.  If you've ever wondered why you needed to reduce
the gain by a factor of 10 between VP9 and VP10, it's because VP10's
time step is 1/10 of VP9's, thus the component of the gain that
represents the time step had to be reduced by 10x.


## The new alternative simulator input model: Integrated velocities

The new input model passes **velocities**, instead of accelerations,
through the USB interface.

Readers will recall from high-school physics classes that velocity is
the integral over time of acceleration.  To perform a numerical
integration over time with a series of discrete-time samples, such as
our accelerometer readings, you just add up the samples, multiplying
each by the time step per sample.

To implement the new "integrated velocity" model, the microcontroller
collects the individual acceleration samples from the accelerometer as
usual, and then adds them up, scaling by the time per sample, to
obtain the velocity at each step.  It reports the integrated velocity
in each USB HID joystick report in place of (or alongside) the
instantaneous acceleration reading.

Applying the velocity-based inputs in VP is about as straightforward
to implement as the original acceleration-based model, at least in
terms of the C++ code, although perhaps a little harder to
conceptualize.  The key to thinking about this is to think of each
moving object's current velocity as the sum of two components: its
velocity relative to the inertial rest frame, and its velocity
relative to the playfield.  The second type comes from the nudge
input, because the nudge input represents the motion of the playfield
relative to the inertial rest frame.  Since VP's convention is that
the playfield defines the coordinate system, adding velocity to the
playfield is a matter of adding the opposite velocity to all of the
moving objects, so that the playfield remains at rest relative to the
coordinate system.  

To express all of this in C++ code, we add a new pair of variables
representing the current X/Y "nudge velocity".  What these really
represent physically is the velocity of the playfield relative to the
inertial rest frame.  On each time step, we calculate the new velocity
for each moving object by adding the **difference** between the **new
instantaneous accelerometer velocity reading** and the **old nudge
velocity** variables.  We then update the nudge velocity variables to
equal the new instantaneous device reading.  This calculation effectively
gives every moving object on every time step a total velocity equal to its
intrinsic velocity (relative to the inertial rest frame) plus the
current instantaneous playfield velocity.  There's no need for the
moving objects to maintain two separate velocities - it's all done
with the simple accounting trick - so nothing else in the simulation
is affected.  In particular, hit testing and collision processing
are completely unchanged.

Note that VPX already has a similar concept for its internal *simulated*
nudging system, which applies a nudge from scripting or by keyboard
activation by modeling an imaginary cabinet as a damped oscillator.  VPX
calculates the motion of this imaginary damped oscillator, and applies
the differential velocity to the ball speeds on each physics time step.
The new accelerometer input handling applies the externally supplied
velocity input the same way.


## Compatibility with games *not* using the new model

For compatibility with other pinball games that only accept the
traditional acceleration-based input, the microcontroller can report
**both** the accelerations and the velocities, using different pairs
of axes for each type of value.  For example, it could report the
instantaneous accelerations on the traditional joystick X and Y axes,
and simultaneously report the velocities on joystick RX/RY.  For each
simulator, the user selects the appropriate pair of axes in the game's
joystick setup dialog, according to what kind of input the simulator
accepts.  This allows the microcontroller to work with multiple
pinball simulators without any need for "mode switching" when changing
games.  The device simply reports both kinds of data, and the
simulator reads the type it's interested in and ignores the rest.



## What's wrong with the traditional model

The motivation for this new feature is that the original
acceleration-based model has some significant, inherent limitations
that negatively affect its realism.

The fundamental, unfixable problem with the traditional model is that
a PC-based simulator doesn't have synchronous access to the
accelerometer.  The access is mediated through the USB HID mechanism
(and, in the case of VP, further mediated by the DirectInput layer in
Windows).  USB HID doesn't provide reliable message delivery or a
consistent time base.  This means that the simulator and the
accelerometer can't synchronize their discrete time steps.  Both work
in terms of discrete time, but *different* discrete time cycles.  As a
result, VP is effectively resampling a discrete-time signal
asynchronously, which injects a great deal of noise and distortion
into the signal.  

It's easiest to understand this in practical terms with an example.
Suppose that the accelerometer emits this series of acceleration
samples:

```7 100 200 1000 300 -50 -200 -600 -400 -60 10 -7 15 8 -8...```

The problem occurs when VP *reads* these sample.  VP doesn't have
synchronous access to the sample stream, so when it reads a new
sample, it just gets whichever sample came through the USB connection
most recently.  To illustrate, let's suppose that VP's input reading
cycle lines up with the samples as shown below, where "*" is a VP
read operation:

```
7 100 200 1000 -500 -100 600 -200 -60 10 -7 15 8 -8...
* *   *   * *        *   * *       *   * *    *  *  *...
```

We're fine for the first few samples, but look at what happens when we
get the "1000" sample: VP reads it *twice* - and then skips the next
sample entirely.  So that extra-large acceleration of 1000 units gets
applied to the simulation for two time steps in a row, yielding twice
the amount of acceleration that it should in the simulation.  And the
next gets missed entirely, so we lose the considerable negative
acceleration it would have applied.

Since over- and under-sampling is inherently stochastic (in that it
arises from the mutually asynchronous time bases of the simulator and
accelerometer), we could equally well see the opposite problem with
this same data stream, where VP never sees *any* instances of the
"1000" reading.  That would be unfortunate for the player because it
means that her attempt to keep the ball from draining with that
particularly hard nudge is completely lost in the simulation.  This
makes the strength of each nudge as reflected in the simulation appear
to be rather random to users.  They'll be frustrated trying to find a
Gain setting that gives them the effect they want, because there's so
little consistency in how VP interprets a given physical nudge.  If
you happen to hit a streak where VP is missing all of the peak
numbers, you'll start cranking the gain up super high because you're
annoyed at how little effect it's having, and then suddenly the
slightest touch makes the ball jump halfway across the screen.

This might sound contrived, but in the actual testing I've done, this
is exactly what the situation looks like.  The over- and
under-counting isn't even rare.  The actual occurrence I've observed
is something like 10%.  This really shouldn't be surprising when you
consider that VP and the accelerometer are operating on similar
discrete time scales.  The two cycles are constantly going in and out
of phase with each other, so things will frequently line up where a VP
cycle misses a whole accelerometer cycle or overlaps two or more.  And
we know from much discussion on the forums that users in practice do
find Gain tuning to be frustratingly random.  The problem isn't just
academic.

The over- and under-counting causes a serious additional problem,
apart from the obvious inconsistency in responsiveness to nudging.  If
you stop to think about the big picture of what VP's really doing here -
it's adding each discrete acceleration sample into each moving
object's velocity on each time step - you see that we have an almost
inherently divergent calculation.  **If** the acceleration inputs
happen to all add up to zero over time, the sum stays finite.  But if
there's even a slight bias in the readings, it will accumulate and
steadily grow over time until the velocities overflow the "floats"
they're stored in.  And this is indeed exactly what happens in
practice.  It's the whole reason that we had to add the "Nudge
Filter" a long time ago to suppress the divergent accumulated
accelerations.  That filter suppresses the infinities, but it adds
artifacts of its own, since it only has the flawed USB data stream
to work with.

## Why device-side velocity integration fixes this

In a nutshell, it's because the device has synchronous access to the
acceleration data.  For the same reason that VP **can't** accurately
integrate the accelerations over time, the device **can**.

Perhaps even more importantly, the velocity model eliminates all of
the problems that arise from the incongruous sampling rates.  The VP
side no longer has to integrate the samples, so if it misses one, it
doesn't matter!  VP still gets the correct instantaneous velocity
reading every time it looks.  If VP misses six samples in a row, it
makes no difference to the running total, because VP isn't calculating
a running total: when VP finally gets around to asking for another
sample after missing six, it instantly gets the exactly correct
current velocity, and everything in the simulation is instantly
brought up to date with the correct current velocity.  If VP looks
too frequently, and keeps getting the same sample six times in a
row, it *still* doesn't matter, because it's actually *correct*
to keep applying that same current velocity to the simulation as
long as it's current on the USB connection.  Again, there's no
running total to miscalculate because there is no running
total.  There's only what the USB device is reporting right now.

As an added bonus, the pinball I/O controller not only can do the
calculation *correctly*, but it can also do it at a higher level of
detail than the simulator can.  The simulator is limited to the USB
polling rate.  The I/O controller, in contrast, has direct access to
the sensor, so it can easily read the sensor at its highest native
rate.  Most of the current consumer accelerometer chips have maximum
sampling rates around 1000 Hz - about 10x faster than the HID polling
cycle.  Sampling at the higher rate provides a more accurate
reconstruction of the actual motion.  The simulator still benefits
from this higher sampling rate on the device side, even though the
simulator is still limited to the slower USB rate, because the
simulator is only interested in the cumulative (integrated) effect of
the readings - the velocity.  The velocity reported
at each USB polling cycle reflects the integration of the many samples
collected since the last polling cycle, so the ability to take many
sensor samples between USB reports yields better quality numbers for
the USB reports.


## Device-side filtering

In practice, the device will still need to apply some correction to
the integrated velocity data, because the consumer-grade
accelerometers we use in pin cabs are very noisy.  The device is in a
much better position to do this than the simulator, though, since it
has a more accurate view of the raw data stream.

To be clear, VP doesn't need or use any of the filtering we're about
to discuss.  VP simply uses the velocities reported, and leaves any
desired filtering to the device, so that filtering can be customized
to the characteristics of the accelerometer being used.  The notes
below are only meant as an aid to device implementers who wish to add
velocity-based nudging to their devices.


### DC removal filtering

Accelerometers almost always have some fixed bias on each axis.  Part
of this comes from the near impossibility of installing the device
perfectly level.  Any slight tilt to one of the horizontal axes will
show up in the readings as a constant acceleration from the component
of the Earth's gravity along the tilted axis.  In addition, most
sensors have some inherent internal DC offset arising from their
mechanical and electronic construction.  This internal bias can't be
eliminated the end user, no matter how perfectly they nail the
positioning.

Almost all of the current pinball I/O controllers already do some kind
of "calibration" or "centering" to remove the bias, by setting the
zero point to an average of readings taken at startup or on an ongoing
basis.  I've found that the bias tends to be fairly dynamic, probably
because the actual cabinet position changes a bit as the machine gets
jostled during active play, so it works best to adjust the centering
point continuously.  One good approach is to use a digital DC removal
filter, with an adaptation time of perhaps 200 to 500 ms.  Algorithms
for such filters are published widely on the Web.


### Noise filtering

Consumer-grade accelerometers tend to be noisy enough that some kind of
noise filtering is called for.

Many older pin controllers use a simple-minded dead-zone approach,
where they simply ignore readings within a certain range of the center
point.  I don't like the dead-zone model because it has such
non-linear behavior; a momentary bit of noise that happens to exceed
the cutoff will cause a sudden burst of reported activity when there
isn't any actually occurring.

The original KL25Z Pinscape used a sort of hybrid of the dead-zone
filter that gradually attenuates readings near the zero point, using a
log-like curve, rather than blocking everything within the center
range.  Like dead-zone filters, this approach is non-linear, but it
doesn't have any abrupt cut-off points, which I think makes its
operation less noticeable and therefore more pleasing.

Pinscape Pico uses hysteresis filtering (which I sometimes call
"jitter filtering", after the filter used in the KL25Z Pinscape
plunger sensor reader).  This type of filter uses a sliding window of
a user-configurable size.  When a reading is within the same window
bounds as the prior reading, the filter output is the midpoint of the
current window.  When a reading is outside of the current window, the
window is repositioned just far enough to incorporate the new reading,
and then that new midpoint is returned as the result.  This type of
filter has the advantage that it responds instantly, with no latency,
to large changes that exceed the window, and minimally attenuates the
signal.  It also moves fairly smoothly when brief blasts of noise
occur, unlike dead zone filters.  When the window size is set to, say,
three or four sigmas worth of noise averaging, the filter is very
stable during periods of actual mechanical quiet.

Some of the accelerometer manufacturers' application notes discuss
more sophisticated band-pass filters, with particular attention to the
dynamic characteristics of the individual sensor models.  MEMs
accelerometers apparently tend to have some specific noise modes that
arise from mechanical resonances unique to each sensor's geometry,
and this seems to call for filters designed to remove those frequencies
specifically.


### Friction filtering

As discussed earlier, one major problem that manifests in VP with its
traditional acceleration-based input model is the accumulation over
time of excess velocity, from the accumulation of unbalanced errors in
the input signal.  Much of the unbalanced error accumulation is due to
the signal corruption that occurs when VP asynchronously resamples the
discrete-time accelerometer input, as already discussed.  But even
with the microcontroller's direct access to the sensor, there might
still be some unbalanced bias that accumulates over time.

A simple technique to eliminate this is what I call a "friction"
filter.  This filter applies a slight attenuation to the integrated
velocity on every time step (i.e., every new acceleration sample).
This is actually the same basic idea as the "Nudge filter" in VP, but
that filter has a lot of added complexity that was necessary because
of VP's more limited view of the underlying data and faster error
accumulation.  The device-side equivalent doesn't need the same
complexity and can be a lot gentler, with much less in the way of
visible artifacts.

The nice thing about a friction filter is that it actually *looks*
like friction from a user's perspective, which makes it look natural
in the simulation rather than like an unwanted artifact.  If you model
the effect of the filter on a ball rolling on a flat, level,
frictionless surface, and you give the accelerometer a hard nudge that
leaves some residual unbalanced velocity in the simulation, the
friction filter will appear to make the ball slow and roll to a stop
after a few half-life periods.  It looks just like a real ball would
on a real surface that actually has some friction.  Now, the
accelerometer is actually measuring the motion of the pin cab, not the
ball, and the pin cab doesn't have this particular drift-to-a-stop
behavior (at least, not on such a long time scale).  But from the
user's perspective, the visible effect is on the way the balls move in
the simulation, so it's a happy coincidence that the filter produces
natural-looking effects in that context.

In the Pinscape Pico setup, I've found that a friction filter with a
half-life of around 2 seconds works well.  ("Half-life" means the time
it takes to attenuate the velocity by 50%, in the absence
of further accelerations.)  The implementation of a friction filter
is simple: just multiply the current velocity by a fixed factor on
each time step, equal to `pow(0.5, halfLife/sampleRate)`, where
`halfLife` is the 50% attenuation time in seconds, and `sampleRate`
is the number of samples per second.

## References

[Improving the Virtual Pin Cab Input Model](http://mjrnet.org/pinscape/OpenPinballDevice/NewPinCabInput.htm):
A more complete presentation of the ideas here, along with related
changes for plunger input, aimed at both simulator developers and device developers.
