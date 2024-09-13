#pragma once

//#define		HID_OUTPUT_START		0x01
#define		HID_OUTPUT_KNOCKER		0x02
#define		HID_OUTPUT_PLUNGER		0x04
//#define		HID_OUTPUT_EXIT			0x08
//#define		HID_OUTPUT_COINMETER	0x10

void ushock_output_init();
void ushock_output_shutdown();
void ushock_output_set(const U08 mask, const bool on);

void ushock_output_update(const U32 cur_time_msec);

void ushock_output_knock(const int count = 1); // Do a single knock
