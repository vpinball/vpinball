# Velocity-based Input for Accelerometers - VP Setup Guide

This note describes how to set up a **velocity-based accelerometer**
in VP.

Traditionally, VP and other simulators accepted accelerometer input in
"raw" form, reading the instantaneous accelerations reported by the
sensor.  Newer VP versions support an alternative model where your I/O
controller device calculates the velocity of the cabinet implied by
recent acceleration readings, and passes the velocity readings to VP
*instead of* the raw acceleration readings.  The advantage of this
approach is that the I/O controller has high-speed and high-precision
access to the accelerometer data, so it can perform the
integration-over-time calculation much more accurately than VP can
within the limitations of the USB connection.  This in principle can
provide more life-like nudging behavior in VP games.


## When to use

Use the velocity input option **only** with devices that provide
velocity readings, and **only** if you've configured the option on the
device.  Compatible devices include:

* Pinscape Pico.  To enable the feature on the Pico, configure a pair of joystick
axes as **nudge.vx** and **nudge.vy**.  Note that you can *also* configure
a separate pair of axes as acceleration readings, **nudge.x** and **nudge.y**,
if you're also using other pinball games that only accept the traditional
acceleration input.


## VP setup

Use the "Keys" dialog to set up the accelerometer.  The setup for
velocity input is almost the same as for any other accelerometer.  The
only difference is that you check the box labeled "Treat Input as
Velocity".

* Select the X and Y axis assignments where you've assigned the
velocity readings in your USB device configuration.  For Pinscape Pico,
these are the axes you've assigned to **nudge.vx** and **nudge.vy**.

* Check the box **Enable Analog Nudge**

* Check the box **Normal Mounting Orientation**

* Set the **X and Y Gain** percentages.  You'll have to experiment
with these to find the appropriate scale for your system.  The
Nudge Test Table is an easy way to check the effect.

* Check **Treat Input as Velocity**

* Remember that you must **close all tables in the editor** after changing
any of these settings before they'll go into effect in the player.


