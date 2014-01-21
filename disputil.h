#pragma once
typedef void (*EventListCallback)(char *,int,int,LPARAM);

void EnumEventsFromDispatch(IDispatch *pdisp, EventListCallback Callback, LPARAM lparam);
