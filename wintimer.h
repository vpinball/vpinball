#pragma once
#ifndef __WINTIMER_H__
#define __WINTIMER_H__

inline void SLEEP( const int x ) { Sleep((x > 0) ? x : 1); }
inline U64 get_tick_count() {__asm rdtsc};

extern void wintimer_init();

extern U32 msec();
extern U64 usec();

#endif /* !__WINTIMER_H__ */
