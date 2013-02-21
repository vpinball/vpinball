#pragma once
#ifndef __SLINTF_H__
#define __SLINTF_H__

extern void slintf_init();

extern void slintf_popup_console(); // noop if aleady popped up
extern void slintf_close_console(); // noop if already closed

extern void slintf( const char *str, ... );

#endif/* !__SLINTF_H__ */
