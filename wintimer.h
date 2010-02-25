#pragma once
#ifndef __WINTIMER_H__
#define __WINTIMER_H__

static inline void SLEEP( int x ) { if( x ) Sleep( x ); else Sleep( 1 ); }

extern void wintimer_init( void );

extern U32 msec( void );
extern U64 usec( void );

#endif /* !__WINTIMER_H__ */
