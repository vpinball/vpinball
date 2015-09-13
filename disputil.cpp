#include "stdafx.h"
#include "disputil.h"

void EnumEventsFromDispatch(IDispatch *pdisp, EventListCallback Callback, LPARAM lparam)
{
   IProvideClassInfo* pClassInfo;
   pdisp->QueryInterface(IID_IProvideClassInfo, (void **)&pClassInfo);

   if (pClassInfo)
   {
      ITypeInfo *pti;

      pClassInfo->GetClassInfo(&pti);

      if (!pti) return;

      TYPEATTR *pta;

      pti->GetTypeAttr(&pta);

      for (int i = 0; i < pta->cImplTypes; i++)
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

               // Get Name
               {
                  BSTR *rgstr = (BSTR *)CoTaskMemAlloc(6 * sizeof(BSTR *));

                  unsigned int cnames;
                  /*const HRESULT hr =*/ ptiChild->GetNames(pfd->memid, rgstr, 6, &cnames);

                  // Add enum string to combo control
                  char szT[512];
                  WideCharToMultiByte(CP_ACP, 0, rgstr[0], -1, szT, 512, NULL, NULL);
                  (*Callback)(szT, l, pfd->memid, lparam);

                  for (unsigned int i2 = 0; i2 < cnames; i2++)
                  {
                     SysFreeString(rgstr[i2]);
                  }

                  CoTaskMemFree(rgstr);
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
