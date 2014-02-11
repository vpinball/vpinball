#pragma once
#ifndef __WINTIMER_H__
#define __WINTIMER_H__

void wintimer_init();

U32 msec();
unsigned long long usec();

void uSleep(const unsigned long long u);

#endif /* !__WINTIMER_H__ */
