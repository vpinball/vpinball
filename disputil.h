typedef void (*EventListCallback)(char *,int,int,long);

void EnumEventsFromDispatch(IDispatch *pdisp, EventListCallback Callback, LPARAM lparam);