#pragma once
#ifndef __SLINTF_H__
#define __SLINTF_H__

extern void slintf_init( void );

extern void slintf_popup_console( void ); // noop if aleady popped up
extern void slintf_close_console( void ); // noop if already closed

extern void slintf( const char *str, ... );

#endif/* !__SLINTF_H__ */
