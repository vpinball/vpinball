#pragma once

#ifdef SLINTF
void slintf_init();

void slintf_popup_console(); // noop if aleady popped up
void slintf_close_console(); // noop if already closed

void slintf(const char *str, ...);
#else
#define slintf(...) ((void)0)
#endif
