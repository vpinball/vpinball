#pragma once

class Ball;

class EventProxyBase
{
public:
   virtual HRESULT FireDispID(DISPID dispid, DISPPARAMS *pdispparams) = 0;

   void FireVoidEvent(int dispid)
   {
      DISPPARAMS dispparams = { NULL, NULL, 0, 0 };

      FireDispID(dispid, &dispparams);
   }

   void FireVoidEventParm(int dispid, float parm)
   {
      CComVariant rgvar[1] = { CComVariant(parm) };
      DISPPARAMS dispparams = { rgvar, NULL, 1, 0 };

      FireDispID(dispid, &dispparams);
   }

   void FireVoidEventParm(int dispid, int parm)
   {
      CComVariant rgvar[1] = { CComVariant(parm) };
      DISPPARAMS dispparams = { rgvar, NULL, 1, 0 };

      FireDispID(dispid, &dispparams);
   }

   /*	void FireVoidEventParm(int dispid, unsigned int parm)
      {
      CComVariant rgvar[1] = {  CComVariant(parm)};
      DISPPARAMS dispparams  = {rgvar,NULL,1,0};

      FireDispID(dispid, &dispparams);
      }
      */

   void FireVoidEventParm(int dispid, char* parm)
   {
      CComVariant rgvar[1] = { CComVariant(parm) };
      DISPPARAMS dispparams = { rgvar, NULL, 1, 0 };

      FireDispID(dispid, &dispparams);
   }
};

template <class T, const IID* psrcid>
class EventProxy : public EventProxyBase, public IConnectionPointImpl<T, psrcid, CComDynamicUnkArray>
{
public:
   EventProxy() {}

   virtual ~EventProxy() {}

   void FireVoidGroupEvent(int dispid)
   {
      T* const pT = (T*)this;
      for (int i = 0; i < pT->m_vEventCollection.Size(); ++i)
      {
         Collection * const pcollection = pT->m_vEventCollection.ElementAt(i);

         /* this seems to break the COM interface because the Hit-Event of a collection needs a long parameter and not size_t
      CComVariant rgvar[1] = {CComVariant((size_t)pT->m_viEventCollection.ElementAt(i))};
      */
         CComVariant rgvar[1] = { CComVariant((long)pT->m_viEventCollection.ElementAt(i)) };

         DISPPARAMS dispparams = { rgvar, NULL, 1, 0 };

         pcollection->FireDispID(dispid, &dispparams);
      }

      if (pT->m_fSingleEvents)
         FireVoidEvent(dispid);
   }

   virtual HRESULT FireDispID(DISPID dispid, DISPPARAMS *pdispparams)
   {
      T* const pT = (T*)this;
      pT->Lock();
      IUnknown** pp = m_vec.begin();
      while (pp < m_vec.end())
      {
         if (*pp != NULL)
         {
            IDispatch* const pDispatch = reinterpret_cast<IDispatch*>(*pp);
            pDispatch->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, NULL, NULL, NULL);
         }
         ++pp;
      }
      pT->Unlock();

      return S_OK;
   }
};
