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
acceleration input, but you won't need to use those in the VP setup.

This **only** works with devices that actually calculate and send
velocity data.  **Don't** enable velocity inputs in VP with devices that
only send accelerations, because that will just make VP misinterpret
the data and will yield unrealistic results.


## VP setup

Use the "Keys" dialog to set up the accelerometer.  The setup for
velocity input is almost the same as for any other accelerometer.  The
only differences are:

* Check **Analog Nudge Input is Velocity**

* **Un**check **Enable Nudge Filter** (this filter is designed for acceleration
inputs only, so it should always be **disabled** for velocity-based nudge inputs)

* Select the X and Y axis assignments where you've assigned the **velocity** 
readings in your USB device configuration.  

  * For Pinscape on KL25Z, these are **RX** and **RY**
  * For Pinscape Pico, these are the axes you've assigned to **nudge.vx** and **nudge.vy**

Everything else is just the standard setup procedure for any type of nudge device:

* Check the box **Enable Analog Nudge**

* Set the **X and Y Gain** percentages.  You'll have to experiment
with these to find the right scale for your system, since it depends
on the device, and also on how strong an effect you like.  The Nudge
Test Table is an easy way to check the effect and adjust the settings
interactively.

* **Board Mounted Face Up** and **Accelerometer Rotation** are used
to translate the device's coordinate system into VP's model, so these
depend on what kind of device you're using and how it's physically
oriented in the cabinet.  You'll have to consult your device's
documentation for advice (or just experiment with the Nudge Test
Table until side-to-side and front-to-back nudges move things in
the right directions).
