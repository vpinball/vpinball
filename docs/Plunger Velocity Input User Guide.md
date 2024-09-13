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

Use the "Keys" dialog to set up the plunger.  The procedure is the
same as the usual plunger setup, except that you select a Plunger
Speed axis in addition to the normal Plunger Position axis.

* In the **Plunger** drop list, select the joystick axis assigned to
the **plunger position** reading on your I/O controller.

* In the **Speed** drop list immediately below that, select the joystick
axis assigned to the **plunger speed** reading on your I/O controller.

* Check the **Reverse axis** box if necessary (if you discover that
the on-screen plunger moves in the opposite direction you expect in
response to inputs on the physical plunger).

* Remember that you must **close all tables in the editor** after changing
any of these settings before they'll go into effect in the player.

