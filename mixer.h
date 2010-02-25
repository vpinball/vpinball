#pragma once
#ifndef __MIXER_H__
#define __MIXER_H__

int  mixer_init    ( HWND wnd );
void mixer_shutdown( void );

void mixer_display_volume( void ); // signal to display the volume control on the screen
void mixer_volume( F32 volume );
void mixer_volmod( F32 volmod );
F32  mixer_get_volume( void );

void mixer_update( void );

void mixer_draw( void );
void mixer_erase( void );

#endif/* !__MIXER_H__ */
