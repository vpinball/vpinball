#pragma once

typedef void (*EventListCallback)(const char *,int,int,LPARAM);

void EnumEventsFromDispatch(IDispatch *pdisp, EventListCallback Callback, LPARAM lparam);
