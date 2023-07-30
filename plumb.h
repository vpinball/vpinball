#pragma once

void plumb_set_sensitivity(const F32 sens);
void nudge_set_sensitivity(const F32 sens);

F32 nudge_get_sensitivity();

void plumb_update(const U32 curr_time_msec, const float getx, const float gety);

// This function is destructive .. it will return true once per tilt event
bool plumb_tilted();

void plumb_draw();
