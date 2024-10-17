#pragma once

//#define		USHOCK_OUTPUT_START		0x01
#define		USHOCK_OUTPUT_KNOCKER		0x02
#define		USHOCK_OUTPUT_PLUNGER		0x04
//#define		USHOCK_OUTPUT_EXIT			0x08
//#define		USHOCK_OUTPUT_COINMETER	0x10

void ushock_init();
void ushock_shutdown();
void ushock_set_output(const U08 mask, const bool on);

void ushock_update(const U32 cur_time_msec);

void ushock_knock(const int count = 1); // Do a single knock
