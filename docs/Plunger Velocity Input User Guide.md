# Plunger Velocity Input - VP Setup Guide

This note describes how to set up **plunger velocity input** in VP.

Traditionally, pin cab I/O controller devices with plunger input only
reported the current *position* of the plunger to the PC.  Newer
versions of VP support an additional input that lets the I/O
controller also report the *speed* of the plunger.  This is beneficial
because the I/O controller can measure the plunger speed much more
accurately than VP can, because the controller has direct access to
the sensor that lets it take position readings at high frequency.  VP
only sees the position intermittently, at the slower USB HID rate.
The new plunger speed input lets the controller send these more
accurate readings to VP along with the position, which in turn lets VP
simulate the plunger physics more realistically.


## When to use

Use the plunger velocity input **only** with devices that provide
this information, and **only** if you've configured the option on
the device.  Compatible devices include:

* Pinscape Controller on KL25Z: In the Pinscape Config Tool, set the
joystick axis configuration to "X/Y/Z + RX/RY/RZ".  This reports
the plunger position on the **Z** axis and the plunger speed on **RZ**.

* Pinscape Pico.  To enable the feature, configure a joystick axis
of your choice as **plunger.z** (position), and a second axis as **plunger.speed**.


## VP setup

The plunger is set up in the in-game menu (while playing):
**Plunger Settings** > **Add a new plunger sensor** (or select an existing sensor).

* **Position sensor**: select the axis that carries the **plunger position**.

* **Velocity sensor**: select the axis that carries the **plunger speed**.

* **Sensor unit** (velocity sensor): the speed must be expressed in plunger
travel lengths per second (p.u./s, 1 = full travel range).  For Pinscape, select
**12.5 p.u/s [Pinscape plunger velocity]**.  For other devices, set **Unit scale**
so that the value is in p.u./s (the default scale is 1).

* **Reversed axis**: enable it if the on-screen plunger moves in the opposite
direction to the physical plunger.

* Changes apply immediately and are saved as application-wide settings.

* The table's plunger must have **Mech Plunger** enabled to follow the
analog input.

