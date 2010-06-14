#pragma once
#ifndef __MIXER_H__
#define __MIXER_H__

int  mixer_init    ( HWND wnd );
void mixer_shutdown();

void mixer_display_volume(); // signal to display the volume control on the screen
void mixer_volume( F32 volume );
void mixer_volmod( F32 volmod );
F32  mixer_get_volume();

void mixer_update();

void mixer_draw();
void mixer_erase();

#endif/* !__MIXER_H__ */
