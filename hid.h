#pragma once
#ifndef __HID_H__
#define __HID_H__

//#define		HID_OUTPUT_START		0x01
#define		HID_OUTPUT_KNOCKER		0x02
#define		HID_OUTPUT_PLUNGER		0x04
//#define		HID_OUTPUT_EXIT			0x08
//#define		HID_OUTPUT_COINMETER	0x10

void hid_init();
void hid_shutdown();
void hid_set_output(const U08 mask, const bool on);

void hid_update(const U32 cur_time_msec);

void hid_knock(const int count = 1); // Do a single knock

#endif/* !__HID_H__ */
