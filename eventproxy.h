#pragma once

class Ball;

class EventProxyBase
{
public:
   virtual HRESULT FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) = 0;

   void FireVoidEvent(const int dispid)
   {
      DISPPARAMS dispparams = { nullptr, nullptr, 0, 0 };

      FireDispID(dispid, &dispparams);
   }

   void FireVoidEventParm(const int dispid, const float parm)
   {
      CComVariant rgvar[1] = { CComVariant(parm) };
      DISPPARAMS dispparams = { rgvar, nullptr, 1, 0 };

      FireDispID(dispid, &dispparams);
   }

   void FireVoidEventParm(const int dispid, const int parm)
   {
      CComVariant rgvar[1] = { CComVariant(parm) };
      DISPPARAMS dispparams = { rgvar, nullptr, 1, 0 };

      FireDispID(dispid, &dispparams);
   }

   /*void FireVoidEventParm(const int dispid, const unsigned int parm)
   {
      CComVariant rgvar[1] = { CComVariant(parm) };
      DISPPARAMS dispparams  = { rgvar, nullptr, 1, 0 };

      FireDispID(dispid, &dispparams);
   }*/

   /*void FireVoidEventParm(const int dispid, const char* parm)
   {
      CComVariant rgvar[1] = { CComVariant(parm) };
      DISPPARAMS dispparams = { rgvar, nullptr, 1, 0 };

      FireDispID(dispid, &dispparams);
   }*/
};

template <class T, const IID* psrcid>
class EventProxy : public EventProxyBase, public IConnectionPointImpl<T, psrcid, CComDynamicUnkArray>
{
public:
   EventProxy() {}

   virtual ~EventProxy() {}

   void FireVoidGroupEvent(const int dispid)
   {
      T* const pT = (T*)this;
      for (size_t i = 0; i < pT->m_vEventCollection.size(); ++i)
      {
         Collection * const pcollection = pT->m_vEventCollection[i];

         if (pcollection!=nullptr)
         {
#ifndef __STANDALONE__
            CComVariant rgvar[1] = { CComVariant((long)pT->m_viEventCollection[i]) };
#else
            CComVariant rgvar[1] = { CComVariant(pT->m_viEventCollection[i]) };
#endif
            DISPPARAMS dispparams = { rgvar, nullptr, 1, 0 };

#ifndef __STANDALONE__
            pcollection->FireDispID(dispid, &dispparams);
#else
            ((EventProxyBase*)pcollection)->FireDispID(dispid, &dispparams);
#endif
         }
      }

      if (pT->m_singleEvents)
         FireVoidEvent(dispid);
   }

   HRESULT FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) override
   {
      T* const pT = (T*)this;
      pT->Lock();
      IUnknown** pp = IConnectionPointImpl<T, psrcid, CComDynamicUnkArray>::m_vec.begin();
      while (pp < IConnectionPointImpl<T, psrcid, CComDynamicUnkArray>::m_vec.end())
      {
         if (*pp != nullptr)
         {
            IDispatch* const pDispatch = reinterpret_cast<IDispatch*>(*pp);
            pDispatch->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, nullptr, nullptr, nullptr);
         }
         ++pp;
      }
      pT->Unlock();

      return S_OK;
   }
};
