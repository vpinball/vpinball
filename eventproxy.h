class Ball;

class EventProxyBase
	{
public:
	virtual HRESULT FireDispID(DISPID dispid, DISPPARAMS *pdispparams) = 0;

	void FireVoidEvent(int dispid)
		{
		DISPPARAMS dispparams  = {
			NULL,
			NULL,
			0,
			0
			};
		
		FireDispID(dispid, &dispparams);
		}

	void FireBallEvent(int dispid, Ball *pball);
	};

template <class T, const IID* psrcid>
class EventProxy : public EventProxyBase, public IConnectionPointImpl<T, psrcid, CComDynamicUnkArray>
{
public:
	EventProxy()
		{
		/*HRESULT hr = CREATEINSTANCE(ChartEventInfo,&m_peventinfo);
		if (m_peventinfo)
			m_peventinfo->AddRef();
		m_fEventsEnabled = fTrue;
		m_fDblClick = fFalse;
		m_xPrevious = -1;
		m_yPrevious = -1;
		m_grbitActiveEvents = 0;*/
		}
		
	virtual ~EventProxy()
		{
		//if (m_peventinfo)
			//m_peventinfo->Release();
		}

	void FireVoidGroupEvent(int dispid)
		{
		T* pT = (T*)this;
		//BOOL m_fFireObjectEvent = fTrue;
		int i;
		for (i=0;i<pT->m_vEventCollection.Size();i++)
			{
			Collection *pcollection = pT->m_vEventCollection.ElementAt(i);
			//if (pcollection->m_fFireEvents)
				{
				CComVariant rgvar[1] = {CComVariant((int)pT->m_viEventCollection.ElementAt(i))};
					
				DISPPARAMS dispparams  = {
					rgvar,
					NULL,
					1,
					0
					};

				pcollection->FireDispID(dispid, &dispparams);
				}

			/*if (pcollection->m_fStopSingleEvents)
				{
				m_fFireObjectEvent = fFalse;
				}*/
			}

		if (pT->m_fSingleEvents)
			{
			FireVoidEvent(dispid);
			}
		/*if (!pT->m_pcollection)
			{
			FireVoidEvent(dispid);
			}
		else
			{
			CComVariant rgvar[1] = {CComVariant(pT->m_iCollection)};
	
			DISPPARAMS dispparams  = {
				rgvar,
				NULL,
				1,
				0
				};

			pT->m_pcollection->FireDispID(dispid, &dispparams);
			}*/
		}

	virtual HRESULT FireDispID(DISPID dispid, DISPPARAMS *pdispparams)
		{
		T* pT = (T*)this;
		pT->Lock();
		IUnknown** pp = m_vec.begin();
		while (pp < m_vec.end())
			{
			if (*pp != NULL)
				{   
				IDispatch* pDispatch = reinterpret_cast<IDispatch*>(*pp);
				pDispatch->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, NULL, NULL, NULL);  
				}
			pp++;
			}
		pT->Unlock();
		
		//m_peventinfo->DisassociateEventObj();
		
		return S_OK;
		}	
};