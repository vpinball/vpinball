#pragma once
#ifndef __PLUMB_H__
#define __PLUMB_H__

void plumb_set_sensitivity( F32 sens );
void nudge_set_sensitivity( F32 sens );

F32 nudge_get_sensitivity();

void plumb_update(const U32 curr_time_msec);

// This function is detructive .. it will return 1 once per tilt event
int  plumb_tilted();

void plumb_draw();
void plumb_erase();

#endif/* !__PLUMB_H__ */
