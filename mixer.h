#pragma once
#ifndef __MIXER_H__
#define __MIXER_H__

BOOL mixer_init(const HWND wnd);
void mixer_shutdown();

void mixer_get_volume();

void mixer_update();

void mixer_draw();

#endif/* !__MIXER_H__ */
