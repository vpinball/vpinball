#include "core/stdafx.h"

#include "Scriptable.h"


vector<wstring> IScriptable::GetMethodNames()
{
   vector<wstring> methods;
#ifndef __STANDALONE__
   IDispatch *pdisp = GetDispatch();
   if (!pdisp)
      return methods;

   ITypeInfo *pti = nullptr;
   // Get the TypeInfo directly from the Dispatch pointer
   if (SUCCEEDED(pdisp->GetTypeInfo(0, LOCALE_USER_DEFAULT, &pti)))
   {
      TYPEATTR *pta;
      if (SUCCEEDED(pti->GetTypeAttr(&pta)))
      {
         // Iterate through all functions in the interface
         for (unsigned int i = 0; i < pta->cFuncs; i++)
         {
            FUNCDESC *pfd;
            if (SUCCEEDED(pti->GetFuncDesc(i, &pfd)))
            {
               // Filter for methods (FUNC_PUREVIRTUAL or FUNC_DISPATCH)
               // and ignore property accessors (INVOKE_PROPERTYGET/PUT) if desired
               if (pfd->invkind & INVOKE_FUNC)
               {
                  BSTR rgstr[1];
                  unsigned int cnames;
                  if (SUCCEEDED(pti->GetNames(pfd->memid, rgstr, 1, &cnames)) && cnames > 0)
                  {
                     methods.push_back(wstring(rgstr[0], SysStringLen(rgstr[0])));
                     SysFreeString(rgstr[0]);
                  }
               }
               pti->ReleaseFuncDesc(pfd);
            }
         }
         pti->ReleaseTypeAttr(pta);
      }
      pti->Release();
   }
#endif
   return methods;
}

vector<wstring> IScriptable::GetEventNames()
{
   vector<wstring> events;
#ifndef __STANDALONE__
   IDispatch * pdisp = GetDispatch();

   // Enum Events From Dispatch
   IProvideClassInfo *pClassInfo;
   pdisp->QueryInterface(IID_IProvideClassInfo, (void **)&pClassInfo);

   if (pClassInfo)
   {
      ITypeInfo *pti;
      pClassInfo->GetClassInfo(&pti);
      if (pti)
      {
         TYPEATTR *pta;
         pti->GetTypeAttr(&pta);
         for (unsigned int i = 0; i < pta->cImplTypes; i++)
         {
            HREFTYPE href;
            ITypeInfo *ptiChild;
            TYPEATTR *ptaChild;

            int impltype;
            pti->GetImplTypeFlags(i, &impltype);
            if (impltype & IMPLTYPEFLAG_FSOURCE)
            {
               pti->GetRefTypeOfImplType(i, &href);
               pti->GetRefTypeInfo(href, &ptiChild);
               ptiChild->GetTypeAttr(&ptaChild);
               for (int l = 0; l < ptaChild->cFuncs; l++)
               {
                  FUNCDESC *pfd;
                  ptiChild->GetFuncDesc(l, &pfd);
                  {
                     BSTR rgstr[6];
                     unsigned int cnames;
                     ptiChild->GetNames(pfd->memid, rgstr, std::size(rgstr), &cnames);
                     events.push_back(wstring(rgstr[0], SysStringLen(rgstr[0])));
                     for (unsigned int i2 = 0; i2 < cnames; i2++)
                        SysFreeString(rgstr[i2]);
                  }
                  ptiChild->ReleaseFuncDesc(pfd);
               }
               ptiChild->ReleaseTypeAttr(ptaChild);
               ptiChild->Release();
            }
         }
         pti->ReleaseTypeAttr(pta);
         pti->Release();
         pClassInfo->Release();
      }
   }
#endif
   return events;
}
