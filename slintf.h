#pragma once
#ifndef __SLINTF_H__
#define __SLINTF_H__

#ifdef SLINTF
void slintf_init();

void slintf_popup_console(); // noop if aleady popped up
void slintf_close_console(); // noop if already closed
#endif

void slintf(const char *str, ...);

#endif/* !__SLINTF_H__ */
