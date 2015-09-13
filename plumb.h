#pragma once
#ifndef __PLUMB_H__
#define __PLUMB_H__

void plumb_set_sensitivity(const F32 sens);
void nudge_set_sensitivity(const F32 sens);

F32 nudge_get_sensitivity();

void plumb_update(const U32 curr_time_msec, const float getx, const float gety);

// This function is detructive .. it will return 1 once per tilt event
int  plumb_tilted();

void plumb_draw();

#endif/* !__PLUMB_H__ */
