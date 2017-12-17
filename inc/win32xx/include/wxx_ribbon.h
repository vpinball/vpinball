// Win32++   Version 8.5
// Release Date: 1st December 2017
//
//      David Nash
//      email: dnash@bigpond.net.au
//      url: https://sourceforge.net/projects/win32-framework
//
//
// Copyright (c) 2005-2017  David Nash
//
// Permission is hereby granted, free of charge, to
// any person obtaining a copy of this software and
// associated documentation files (the "Software"),
// to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify,
// merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom
// the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice
// shall be included in all copies or substantial portions
// of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF
// ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
// TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
// SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
// ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
// OR OTHER DEALINGS IN THE SOFTWARE.
//
////////////////////////////////////////////////////////


///////////////////////////////////////////////////////
// wxx_ribbon.h
//  Declaration of the following classes:
//  CRibbon, CRibbonFrameT, CRibbonFrame, CRibbonDockFrame
//  CRibbonMDIFrame and CRibbonMDIDockFrame.
//

#ifndef _WIN32XX_RIBBON_H_
#define _WIN32XX_RIBBON_H_


// Notes :
//  1) To compile code using the Ribbon UI you will need a Microsoft compiler.
//     Visual Studio Community 2013 (or later) is recommended, but older
//     Microsoft compilers can be used with the Windows 7 SDK.
//  2) The Ribbon UI only runs on Windows 7 or later operating systems.If the
//     code is run on an earlier operating system, it reverts back to a menu
//     and toolbar.

#include <UIRibbon.h>                   // Contained within the Windows 7 SDK
#include <UIRibbonPropertyHelpers.h>

#include "wxx_frame.h"
#include "wxx_dockframe.h"

namespace Win32xx
{
    //////////////////////////////////////////////
    // The CRibbon class is used to add the Ribbon framework to a window.
    class CRibbon : public IUICommandHandler, public IUIApplication
    {
    public:
        CRibbon() : m_cRef(0), m_pRibbonFramework(NULL) {}
        ~CRibbon();

        // IUnknown methods.
        STDMETHOD_(ULONG, AddRef());
        STDMETHOD_(ULONG, Release());
        STDMETHOD(QueryInterface(REFIID iid, void** ppv));

        // IUIApplication methods
        STDMETHOD(OnCreateUICommand)(UINT nCmdID, __in UI_COMMANDTYPE typeID,
            __deref_out IUICommandHandler** ppCommandHandler);

        STDMETHOD(OnDestroyUICommand)(UINT32 commandId, __in UI_COMMANDTYPE typeID,
            __in_opt IUICommandHandler* commandHandler);

        STDMETHOD(OnViewChanged)(UINT viewId, __in UI_VIEWTYPE typeId, __in IUnknown* pView,
            UI_VIEWVERB verb, INT uReasonCode);

        // IUICommandHandle methods
        STDMETHODIMP Execute(UINT nCmdID, UI_EXECUTIONVERB verb, __in_opt const PROPERTYKEY* key, __in_opt const PROPVARIANT* ppropvarValue,
                                          __in_opt IUISimplePropertySet* pCommandExecutionProperties);

        STDMETHODIMP UpdateProperty(UINT nCmdID, __in REFPROPERTYKEY key, __in_opt const PROPVARIANT* ppropvarCurrentValue,
                                                 __out PROPVARIANT* ppropvarNewValue);

        bool virtual CreateRibbon(HWND hWnd);
        void virtual DestroyRibbon();
        IUIFramework* GetRibbonFramework() const { return m_pRibbonFramework; }
        UINT GetRibbonHeight() const;

    private:
        IUIFramework* m_pRibbonFramework;
        LONG m_cRef;                            // Reference count.

    };

    //////////////////////////////////////////////
    // Declaration of the CRibbonFrameT class template
    //

    // The CRibbonFrameT is the base class for frames that support the Ribbon Framework.
    // The T parameter can be either CWnd or CDocker.
    template <class T>
    class CRibbonFrameT : public T, public CRibbon
    {
    public:
        // A nested class for the MRU item properties
        class CRecentFiles : public IUISimplePropertySet
        {
        public:
            CRecentFiles(PWSTR wszFullPath);
            ~CRecentFiles() {}

            // IUnknown methods.
            STDMETHODIMP_(ULONG) AddRef();
            STDMETHODIMP_(ULONG) Release();
            STDMETHODIMP QueryInterface(REFIID iid, void** ppv);

            // IUISimplePropertySet methods
            STDMETHODIMP GetValue(__in REFPROPERTYKEY key, __out PROPVARIANT *value);

        private:
            LONG m_cRef;                        // Reference count.
            WCHAR m_wszDisplayName[MAX_PATH];
            WCHAR m_wszFullPath[MAX_PATH];
        };

        typedef Shared_Ptr<CRecentFiles> RecentFilesPtr;

        CRibbonFrameT() {}
        virtual ~CRibbonFrameT() {}
        virtual CRect GetViewRect() const;
        virtual int  OnCreate(CREATESTRUCT& cs);
        virtual void OnDestroy();
        virtual STDMETHODIMP OnViewChanged(UINT32 viewId, UI_VIEWTYPE typeId, IUnknown* pView, UI_VIEWVERB verb, INT32 uReasonCode);
        virtual HRESULT PopulateRibbonRecentItems(__deref_out PROPVARIANT* pvarValue);
        virtual void UpdateMRUMenu();

    private:
        std::vector<RecentFilesPtr> m_vRecentFiles;
    };


    // This class provides an SDI frame with a Ribbon Framework
    class CRibbonFrame : public CRibbonFrameT<CFrame>
    {
    public:
        CRibbonFrame() {}
        virtual ~CRibbonFrame() {}
    };


    // This class provide an SDI frame with a Ribbon Framework that supports docking.
    class CRibbonDockFrame : public CRibbonFrameT<CDockFrame>
    {
    public:
        CRibbonDockFrame() {}
        virtual ~CRibbonDockFrame() {}
    };


    // This class provides a MDI frame with a Ribbon Framework.
    class CRibbonMDIFrame : public CRibbonFrameT<CMDIFrame>
    {
    public:
        CRibbonMDIFrame() {}
        virtual ~CRibbonMDIFrame() {}
    };


    // This class provides a MDI frame with a Ribbon Framework that supports docking.
    class CRibbonMDIDockFrame : public CRibbonFrameT<CMDIDockFrame>
    {
    public:
        CRibbonMDIDockFrame() {}
        virtual ~CRibbonMDIDockFrame() {}
    };

}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


namespace Win32xx
{
    //////////////////////////////////////////////
    // Definitions for the CRibbon class
    //

    inline CRibbon::~CRibbon()
    {
        // Reference count must be 0 or we have a leak!
        assert(m_cRef == 0);
    }

    //////////////////////////////////
    // IUnknown method implementations.


    inline STDMETHODIMP_(ULONG) CRibbon::AddRef()
    {
        return InterlockedIncrement(&m_cRef);
    }

    inline STDMETHODIMP_(ULONG) CRibbon::Release()
    {
        LONG cRef = InterlockedDecrement(&m_cRef);
        return cRef;
    }


    // Responds to execute events on Commands bound to the Command handler.
    inline STDMETHODIMP CRibbon::Execute(UINT nCmdID, UI_EXECUTIONVERB verb, __in_opt const PROPERTYKEY* key, __in_opt const PROPVARIANT* ppropvarValue,
                                          __in_opt IUISimplePropertySet* pCommandExecutionProperties)
    {
        UNREFERENCED_PARAMETER (nCmdID);
        UNREFERENCED_PARAMETER (verb);
        UNREFERENCED_PARAMETER (key);
        UNREFERENCED_PARAMETER (ppropvarValue);
        UNREFERENCED_PARAMETER (pCommandExecutionProperties);

        return E_NOTIMPL;
    }


    inline STDMETHODIMP CRibbon::QueryInterface(REFIID iid, void** ppv)
    {
        if (iid == __uuidof(IUnknown))
        {
            *ppv = static_cast<IUnknown*>(static_cast<IUIApplication*>(this));
        }
        else if (iid == __uuidof(IUICommandHandler))
        {
            *ppv = static_cast<IUICommandHandler*>(this);
        }
        else if (iid == __uuidof(IUIApplication))
        {
            *ppv = static_cast<IUIApplication*>(this);
        }
        else
        {
            *ppv = NULL;
            return E_NOINTERFACE;
        }

        AddRef();
        return S_OK;
    }


    // Called by the Ribbon framework for each command specified in markup, to bind the Command to an IUICommandHandler.
    inline STDMETHODIMP CRibbon::OnCreateUICommand(UINT nCmdID, __in UI_COMMANDTYPE typeID,
                                                 __deref_out IUICommandHandler** ppCommandHandler)
    {
        UNREFERENCED_PARAMETER(typeID);
        UNREFERENCED_PARAMETER(nCmdID);

        // By default we use the single command handler provided as part of CRibbon.
        // Override this function to account for multiple command handlers.

        return QueryInterface(IID_PPV_ARGS(ppCommandHandler));
    }


    // Called when the state of the Ribbon changes, for example, created, destroyed, or resized.
    inline STDMETHODIMP CRibbon::OnViewChanged(UINT viewId, __in UI_VIEWTYPE typeId, __in IUnknown* pView,
                                             UI_VIEWVERB verb, INT uReasonCode)
    {
        UNREFERENCED_PARAMETER(viewId);
        UNREFERENCED_PARAMETER(typeId);
        UNREFERENCED_PARAMETER(pView);
        UNREFERENCED_PARAMETER(verb);
        UNREFERENCED_PARAMETER(uReasonCode);

        return E_NOTIMPL;
    }


    // Called by the Ribbon framework for each command at the time of ribbon destruction.
    inline STDMETHODIMP CRibbon::OnDestroyUICommand(UINT32 nCmdID, __in UI_COMMANDTYPE typeID,
                                                  __in_opt IUICommandHandler* commandHandler)
    {
        UNREFERENCED_PARAMETER(commandHandler);
        UNREFERENCED_PARAMETER(typeID);
        UNREFERENCED_PARAMETER(nCmdID);

        return E_NOTIMPL;
    }


    // Called by the Ribbon framework when a command property (PKEY) needs to be updated.
    inline STDMETHODIMP CRibbon::UpdateProperty(UINT nCmdID, __in REFPROPERTYKEY key, __in_opt const PROPVARIANT* ppropvarCurrentValue,
                                                 __out PROPVARIANT* ppropvarNewValue)
    {
        UNREFERENCED_PARAMETER(nCmdID);
        UNREFERENCED_PARAMETER(key);
        UNREFERENCED_PARAMETER(ppropvarCurrentValue);
        UNREFERENCED_PARAMETER(ppropvarNewValue);

        return E_NOTIMPL;
    }


    // Creates the ribbon.
    inline bool CRibbon::CreateRibbon(HWND hWnd)
    {
        ::CoInitialize(NULL);

        // Instantiate the Ribbon framework object.
        ::CoCreateInstance(CLSID_UIRibbonFramework, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pRibbonFramework));

        // Connect the host application to the Ribbon framework.

        assert(m_pRibbonFramework);
        HRESULT hr = m_pRibbonFramework->Initialize(hWnd, this);
        if (FAILED(hr))
        {
            return false;
        }

        // Load the binary markup. APPLICATION_RIBBON is the default name generated by uicc.
        hr = m_pRibbonFramework->LoadUI(GetModuleHandle(NULL), L"APPLICATION_RIBBON");
        if (FAILED(hr))
        {
            return false;
        }

        return true;
    }


    // Destroys the ribbon.
    inline void CRibbon::DestroyRibbon()
    {
        if (m_pRibbonFramework)
        {
            m_pRibbonFramework->Destroy();
            m_pRibbonFramework->Release();
            m_pRibbonFramework = NULL;
        }

        ::CoUninitialize();
    }


    // Retrieves the height of the ribbon.
    inline UINT CRibbon::GetRibbonHeight() const
    {
        HRESULT hr = E_FAIL;
        IUIRibbon* pRibbon = NULL;
        UINT uRibbonHeight = 0;

        if (GetRibbonFramework())
        {
            hr = GetRibbonFramework()->GetView(0, IID_PPV_ARGS(&pRibbon));
            if (SUCCEEDED(hr))
            {
                // Call to the framework to determine the desired height of the Ribbon.
                hr = pRibbon->GetHeight(&uRibbonHeight);
                pRibbon->Release();
            }
        }

        return uRibbonHeight;
    }


    //////////////////////////////////////////////
    // Definitions for the CRibbonFrameT class template
    //

    // Get the frame's client area
    template <class T>
    inline CRect CRibbonFrameT<T>::GetViewRect() const
    {
        CRect rcClient = GetClientRect();

        rcClient.top += GetRibbonHeight();

        if (GetStatusBar().IsWindow() && GetStatusBar().IsWindowVisible())
            rcClient = ExcludeChildRect(rcClient, GetStatusBar());

        if (GetReBar().IsWindow() && GetReBar().IsWindowVisible())
            rcClient = ExcludeChildRect(rcClient, GetReBar());
        else
            if (GetToolBar().IsWindow() && GetToolBar().IsWindowVisible())
                rcClient = ExcludeChildRect(rcClient, GetToolBar());

        return rcClient;
    }


    // OnCreate is called automatically during window creation when a
    // WM_CREATE message received.
    // Tasks such as setting the icon, creating child windows, or anything
    // associated with creating windows are normally performed here.
    template <class T>
    inline int CRibbonFrameT<T>::OnCreate(CREATESTRUCT& cs)
    {
        UNREFERENCED_PARAMETER(cs);

        if (GetWinVersion() >= 2601)    // WinVersion >= Windows 7
        {
            if (CreateRibbon(*this))
            {
                SetUseReBar(FALSE);     // Don't use a ReBar
                SetUseToolBar(FALSE);   // Don't use a ToolBar
            }
            else
            {
                TRACE("Failed to create Ribbon\n");
                DestroyRibbon();
            }
        }

        T::OnCreate(cs);
        if (GetRibbonFramework())
        {
            SetMenu(NULL);              // Disable the window menu
            SetFrameMenu(reinterpret_cast<HMENU>(0));
        }

        return 0;
    }


    // Called when the ribbon frame is destroyed.
    template <class T>
    inline void CRibbonFrameT<T>::OnDestroy()
    {
        DestroyRibbon();
        T::OnDestroy();
    }


    // Called when the ribbon's view has changed.
    template <class T>
    inline STDMETHODIMP CRibbonFrameT<T>::OnViewChanged(UINT32 viewId, UI_VIEWTYPE typeId, IUnknown* pView, UI_VIEWVERB verb, INT32 uReasonCode)
    {
        UNREFERENCED_PARAMETER(viewId);
        UNREFERENCED_PARAMETER(pView);
        UNREFERENCED_PARAMETER(uReasonCode);

        HRESULT hr = E_NOTIMPL;

        // Checks to see if the view that was changed was a Ribbon view.
        if (UI_VIEWTYPE_RIBBON == typeId)
        {
            switch (verb)
            {
            case UI_VIEWVERB_CREATE:    // The view was newly created.
                hr = S_OK;
                break;
            case UI_VIEWVERB_SIZE:      // Ribbon size has changed
                RecalcLayout();
                break;
            case UI_VIEWVERB_DESTROY:   // The view was destroyed.
                hr = S_OK;
                break;
            }
        }

        return hr;
    }


    // Populates the ribbon's recent items list.
    template <class T>
    inline HRESULT CRibbonFrameT<T>::PopulateRibbonRecentItems(__deref_out PROPVARIANT* pvarValue)
    {
        LONG iCurrentFile = 0;
        std::vector<CString> FileNames = GetMRUEntries();
        std::vector<CString>::const_iterator iter;
        int iFileCount = FileNames.size();
        HRESULT hr = E_FAIL;
        SAFEARRAY* psa = SafeArrayCreateVector(VT_UNKNOWN, 0, iFileCount);
        m_vRecentFiles.clear();

        if (psa != NULL)
        {
            for (iter = FileNames.begin(); iter != FileNames.end(); ++iter)
            {
                CString strCurrentFile = (*iter);
                WCHAR wszCurrentFile[MAX_PATH] = {0L};
                lstrcpynW(wszCurrentFile, TtoW(strCurrentFile), MAX_PATH);

                CRecentFiles* pRecentFiles = new CRecentFiles(wszCurrentFile);
                m_vRecentFiles.push_back(RecentFilesPtr(pRecentFiles));
                hr = SafeArrayPutElement(psa, &iCurrentFile, static_cast<void*>(pRecentFiles));
                ++iCurrentFile;
            }

            SAFEARRAYBOUND sab = {static_cast<ULONG>(iCurrentFile), 0};
            SafeArrayRedim(psa, &sab);
            hr = UIInitPropertyFromIUnknownArray(UI_PKEY_RecentItems, psa, pvarValue);

            SafeArrayDestroy(psa);  // Calls release for each element in the array
        }

        return hr;
    }


    // Updates the frame's MRU when the Ribbon Framework isn't used.
    template <class T>
    inline void CRibbonFrameT<T>::UpdateMRUMenu()
    {
        // Suppress UpdateMRUMenu when ribbon is used
        if (GetRibbonFramework() != 0) return;

        T::UpdateMRUMenu();
    }


    ////////////////////////////////////////////////////////
    // Declaration of the nested CRecentFiles class
    //
    template <class T>
    inline CRibbonFrameT<T>::CRecentFiles::CRecentFiles(PWSTR wszFullPath) : m_cRef(0)
    {
        SHFILEINFOW sfi;
        DWORD_PTR dwPtr = NULL;
        m_wszFullPath[0] = L'\0';
        m_wszDisplayName[0] = L'\0';

        if (NULL != lstrcpynW(m_wszFullPath, wszFullPath, MAX_PATH))
        {
            dwPtr = ::SHGetFileInfoW(wszFullPath, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(sfi), SHGFI_DISPLAYNAME | SHGFI_USEFILEATTRIBUTES);

            if (dwPtr != NULL)
            {
                lstrcpynW(m_wszDisplayName, sfi.szDisplayName, MAX_PATH);
            }
            else // Provide a reasonable fall back.
            {
                lstrcpynW(m_wszDisplayName, m_wszFullPath, MAX_PATH);
            }
        }
    }

    template <class T>
    inline STDMETHODIMP_(ULONG) CRibbonFrameT<T>::CRecentFiles::AddRef()
    {
        return InterlockedIncrement(&m_cRef);
    }

    template <class T>
    inline STDMETHODIMP_(ULONG) CRibbonFrameT<T>::CRecentFiles::Release()
    {
        return InterlockedDecrement(&m_cRef);
    }

    template <class T>
    inline STDMETHODIMP CRibbonFrameT<T>::CRecentFiles::QueryInterface(REFIID iid, void** ppv)
    {
        if (!ppv)
        {
            return E_POINTER;
        }

        if (iid == __uuidof(IUnknown))
        {
            *ppv = static_cast<IUnknown*>(this);
        }
        else if (iid == __uuidof(IUISimplePropertySet))
        {
            *ppv = static_cast<IUISimplePropertySet*>(this);
        }
        else
        {
            *ppv = NULL;
            return E_NOINTERFACE;
        }

        AddRef();
        return S_OK;
    }

    // IUISimplePropertySet methods.
    template <class T>
    inline STDMETHODIMP CRibbonFrameT<T>::CRecentFiles::GetValue(__in REFPROPERTYKEY key, __out PROPVARIANT *ppropvar)
    {
        HRESULT hr = HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);

        if (key == UI_PKEY_Label)
        {
            hr = UIInitPropertyFromString(key, m_wszDisplayName, ppropvar);
        }
        else if (key == UI_PKEY_LabelDescription)
        {
            hr = UIInitPropertyFromString(key, m_wszDisplayName, ppropvar);
        }

        return hr;
    }


} // namespace Win32xx

#endif  // _WIN32XX_RIBBON_H_

