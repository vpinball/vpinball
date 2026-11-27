# Velocity-based Input for Accelerometers - VP Setup Guide

This note describes how to set up a **velocity-based accelerometer**
in VP.

Traditionally, VP and other simulators accepted accelerometer input in
"raw" form, reading the instantaneous accelerations reported by the
sensor.  Newer VP versions support an alternative model where your I/O
controller device uses the acceleration readings to calculate the
moment-to-moment velocity of the cabinet's motion, and passes the
velocity to VP instead of the raw acceleration data.  The advantage of
this approach is that the device can do this calculation much more
accurately than the simulator can, thanks to its direct, high-speed
access to the sensor readings.  The velocity is ultimately the only
number that matters in the simulation, so better accuracy in the
velocity calculation should yield a more natural effect in the
simulation.


## When to use

Use the velocity input option **only** with devices that provide
velocity readings, and **only** if you've configured the option on the
device.  Compatible devices include:

* Pinscape Controller for KL25Z: To enable the feature in Pinscape, use the
Pinscape Config Tool to set the joystick axis assignments to "X/Y/Z + RX/RY/RZ".
This will report the traditional raw accelerations on X/Y, and the velocity
on RX/RY.

* Pinscape Pico.  To enable the feature on the Pico, configure a pair of joystick
axes as **nudge.vx** and **nudge.vy**.  Note that you can *also* configure
a separate pair of axes as acceleration readings, **nudge.x** and **nudge.y**,
if you're also using other pinball games that only accept the traditional
acceleration input.  VP can optionally use both (see below).

This **only** works with devices that actually calculate and send
velocity data.  **Don't** enable velocity inputs in VP with devices that
only send accelerations, because that will just make VP misinterpret
the data and will yield unrealistic results.


## VP setup

The accelerometer is set up in the in-game menu (while playing):
**Nudge & Tilt Settings** > **Add a new nudge sensor** (or select an existing sensor).
Adding the sensor enables it; changes apply immediately.

* **Nudge Sensor Type**: **Intent Sensor**, or **Cabinet Sensor** for a
high-frequency, low-latency, noise-free sensor in a real cabinet.  (Velocity
axes are not available for the **Game Controller** type.)

* **Side velocity sensor** / **Front velocity sensor**: select the axes that
carry the **velocity** readings.

  * For Pinscape on KL25Z, these are **RX** and **RY**
  * For Pinscape Pico, these are the axes you've assigned to **nudge.vx** and **nudge.vy**

* **Sensor unit**: velocities must be in m/s.  For Pinscape, select
**0.2048 m/s [Pinscape nudge velocity]**; otherwise set **Unit scale**
(the default scale is 1).

* Optional: also map **Side/Front acceleration sensor** (unit: 1/2/4/8 Gravity
or a custom scale to m/s^2).  With both mapped, VP calibrates the gain
between the two channels and combines them.

* **Nudge Strength** (0-200%, default 100%) scales the effect of this sensor.
**Cabinet Weight** (Cabinet Sensor only, default 113 kg) converts the
measured acceleration to the force applied to the simulated cabinet.

* Orientation: map the device axis that moves with side-to-side nudges to
**Side**, and the front-to-back one to **Front**.  Enable **Reversed axis**
if a direction is inverted.  The sensor page plots the readings live,
which helps to check directions and scale.
