#include "stdafx.h"

void EventProxyBase::FireBallEvent(int dispid, Ball *pball)
	{
	VARIANT rgvar[1];

	// arguments are reversed when they're put on the stack.
	V_VT(&rgvar[0]) = VT_DISPATCH;
	V_DISPATCH(&rgvar[0]) = (IDispatch *)pball->m_pballex;

	DISPPARAMS dispparams  = {
		rgvar,
		NULL,
		1,
		0
		};

	FireDispID(dispid, &dispparams);
	}
