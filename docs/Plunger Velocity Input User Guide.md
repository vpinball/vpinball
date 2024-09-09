# Plunger Velocity Input - VP Setup Guide

This note describes how to set up **plunger velocity input** in VP.

Traditionally, I/O controller devices that include mechanical plunger
input reported only the position of the plunger to simulators like VP.
Newer versions of VP support an additional input that lets the I/O
controller also report the *speed* of the plunger from moment to
moment.  The benefit is that the I/O controller has high-bandwidth
access to the plunger sensor, which allows it to accurately calculate
the speed of the plunger at any given moment.  VP can't get a good fix
on the speed during fast motion, such as when you pull back and
release the plunger, because the USB reports are simply too slow.
Since the I/O controller *can* read the speed accurately, it can pass
the information on to VP alongside the position reading, improving the
quality of VP's simulation by giving it accurate speed data.

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
same as the usual plunger setup, except that you also select a Plunger
Speed axis, in addition to the normal Plunger Position axis.

* In the **Plunger** drop list, select the joystick axis assigned to
the **plunger position** reading on your I/O controller.

* In the **Speed** drop list immediately below that, select the joystick
axis assigned to the **plunger speed** reading on your I/O controller.

* Check the **Reverse axis** box if necessary (if you discover that
the on-screen plunger moves in the opposite direction you expect in
response to inputs on the physical plunger).

* Remember that you must **close all tables in the editor** after changing
any of these settings before they'll go into effect in the player.

