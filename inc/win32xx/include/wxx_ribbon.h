// Win32++   Version 9.0
// Release Date: 30th April 2022
//
//      David Nash
//      email: dnash@bigpond.net.au
//      url: https://sourceforge.net/projects/win32-framework
//
//
// Copyright (c) 2005-2022  David Nash
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
    ////////////////////////////////////////////////////////////
    // The CRibbon class is used to add the Ribbon to a window.
    // The ribbon user interface typically replaces the menu and
    // toolbar used by a frame window.
    class CRibbon : public IUICommandHandler, public IUIApplication
    {
    public:
        CRibbon();
        virtual ~CRibbon();

        // IUIApplication methods
        virtual STDMETHODIMP OnCreateUICommand(UINT32 nCmdID, __in UI_COMMANDTYPE typeID,
            __deref_out IUICommandHandler** ppCommandHandler);
        virtual STDMETHODIMP OnDestroyUICommand(UINT32 commandId, __in UI_COMMANDTYPE typeID,
            __in_opt IUICommandHandler* commandHandler);
        virtual STDMETHODIMP OnViewChanged(UINT32 viewId, __in UI_VIEWTYPE typeId, __in IUnknown* pView,
            UI_VIEWVERB verb, INT uReasonCode);

        // IUICommandHandle methods
        virtual STDMETHODIMP Execute(UINT32 nCmdID, UI_EXECUTIONVERB verb, __in_opt const PROPERTYKEY* key, __in_opt const PROPVARIANT* value,
                                          __in_opt IUISimplePropertySet* pCommandExecutionProperties);
        virtual STDMETHODIMP UpdateProperty(UINT32 nCmdID, __in REFPROPERTYKEY key, __in_opt const PROPVARIANT* currentValue,
                                                 __out PROPVARIANT* newValue);
        virtual STDMETHODIMP CreateRibbon(HWND wnd);
        virtual STDMETHODIMP DestroyRibbon();

        // IUnknown methods.
        STDMETHODIMP_(ULONG) AddRef();
        STDMETHODIMP_(ULONG) Release();
        STDMETHODIMP QueryInterface(REFIID iid, void** ppObject);

        // Other
        STDMETHODIMP_(IUIFramework*) GetRibbonFramework() const { return m_pRibbonFramework; }
        STDMETHODIMP_(UINT32) GetRibbonHeight() const;

    private:
        CRibbon(const CRibbon&);              // Disable copy construction
        CRibbon& operator = (const CRibbon&); // Disable assignment operator

        IUIFramework* m_pRibbonFramework;
        LONG m_count;                            // Reference count.

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
            CRecentFiles(PWSTR pFullPath);
            virtual ~CRecentFiles() {}

            // IUnknown methods.
            STDMETHODIMP_(ULONG) AddRef();
            STDMETHODIMP_(ULONG) Release();
            STDMETHODIMP QueryInterface(REFIID iid, void** ppObject);

            // IUISimplePropertySet methods
            STDMETHODIMP GetValue(__in REFPROPERTYKEY key, __out PROPVARIANT* value);

        private:
            LONG m_count;                        // Reference count.
            WCHAR m_displayName[MAX_PATH];
            WCHAR m_fullPath[MAX_PATH];
        };

        // Note: Modern C++ compilers can use this typedef instead.
        // typedef std::shared_ptr<CRecentFiles> RecentFilesPtr;
        typedef Shared_Ptr<CRecentFiles> RecentFilesPtr;

        CRibbonFrameT() {}
        virtual ~CRibbonFrameT() {}
        virtual CRect GetViewRect() const;
        virtual int  OnCreate(CREATESTRUCT& cs);
        virtual void OnDestroy();
        virtual STDMETHODIMP OnViewChanged(UINT32 viewId, UI_VIEWTYPE typeId, IUnknown* pView, UI_VIEWVERB verb, INT32 reasonCode);
        virtual HRESULT PopulateRibbonRecentItems(PROPVARIANT* value);
        virtual void UpdateMRUMenu();

    private:
        CRibbonFrameT(const CRibbonFrameT&);              // Disable copy construction
        CRibbonFrameT& operator = (const CRibbonFrameT&); // Disable assignment operator

        std::vector<RecentFilesPtr> m_recentFiles;
    };


    // This class provides an SDI frame with a Ribbon Framework
    class CRibbonFrame : public CRibbonFrameT<CFrame>
    {
    public:
        CRibbonFrame() {}
        virtual ~CRibbonFrame() {}
    };

    ////////////////////////////////////////////////////
    // CRibbonDockFrame manages a frame which supports the
    // ribbon user interface and docking.
    class CRibbonDockFrame : public CRibbonFrameT<CDockFrame>
    {
    public:
        CRibbonDockFrame() {}
        virtual ~CRibbonDockFrame() {}
    };

    //////////////////////////////////////////////////////////////
    // CRibbonMDIFrame manages a frame which supports the Multiple
    // Document Interface (MDI) and the Ribbon user interface.
    class CRibbonMDIFrame : public CRibbonFrameT<CMDIFrame>
    {
    public:
        CRibbonMDIFrame() {}
        virtual ~CRibbonMDIFrame() {}
    };

    ////////////////////////////////////////////////////////////////
    // CRibbonMDIDockFrame manages a frame which supports the
    // Multiple Document Interface (MDI), the Ribbon user interface,
    // and docking.
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

    inline CRibbon::CRibbon() : m_pRibbonFramework(NULL), m_count(0)
    {
    }

    inline CRibbon::~CRibbon()
    {
    }

    //////////////////////////////////
    // IUnknown method implementations.


    inline STDMETHODIMP_(ULONG) CRibbon::AddRef()
    {
        return InterlockedIncrement(&m_count);
    }

    inline STDMETHODIMP_(ULONG) CRibbon::Release()
    {
        return InterlockedDecrement(&m_count);
    }

    // Responds to execute events on Commands bound to the Command handler.
    inline STDMETHODIMP CRibbon::Execute(UINT32, UI_EXECUTIONVERB, __in_opt const PROPERTYKEY*, __in_opt const PROPVARIANT*,
                                          __in_opt IUISimplePropertySet*)
    {
        return E_NOTIMPL;
    }

    inline STDMETHODIMP CRibbon::QueryInterface(REFIID iid, void** ppObject)
    {
        if (iid == __uuidof(IUnknown))
        {
            *ppObject = static_cast<IUnknown*>(static_cast<IUIApplication*>(this));
        }
        else if (iid == __uuidof(IUICommandHandler))
        {
            *ppObject = static_cast<IUICommandHandler*>(this);
        }
        else if (iid == __uuidof(IUIApplication))
        {
            *ppObject = static_cast<IUIApplication*>(this);
        }
        else
        {
            *ppObject = NULL;
            return E_NOINTERFACE;
        }

        AddRef();
        return S_OK;
    }


    // Called by the Ribbon framework for each command specified in markup, to bind the Command to an IUICommandHandler.
    inline STDMETHODIMP CRibbon::OnCreateUICommand(UINT32, __in UI_COMMANDTYPE,
                                                 __deref_out IUICommandHandler** ppCommandHandler)
    {
        // By default we use the single command handler provided as part of CRibbon.
        // Override this function to account for multiple command handlers.

        return QueryInterface(IID_PPV_ARGS(ppCommandHandler));
    }

    // Called when the state of the Ribbon changes, for example, created, destroyed, or resized.
    inline STDMETHODIMP CRibbon::OnViewChanged(UINT32, __in UI_VIEWTYPE, __in IUnknown*,
                                             UI_VIEWVERB, INT)
    {
        return E_NOTIMPL;
    }

    // Called by the Ribbon framework for each command at the time of ribbon destruction.
    inline STDMETHODIMP CRibbon::OnDestroyUICommand(UINT32, __in UI_COMMANDTYPE,
                                                  __in_opt IUICommandHandler*)
    {
        return E_NOTIMPL;
    }

    // Called by the Ribbon framework when a command property (PKEY) needs to be updated.
    inline STDMETHODIMP CRibbon::UpdateProperty(UINT32, __in REFPROPERTYKEY, __in_opt const PROPVARIANT*,
                                                 __out PROPVARIANT*)
    {
        return E_NOTIMPL;
    }

    // Creates the ribbon.
    inline STDMETHODIMP CRibbon::CreateRibbon(HWND wnd)
    {
        HRESULT hr;
        // Instantiate the Ribbon framework object.
        if (SUCCEEDED(hr = ::CoCreateInstance(CLSID_UIRibbonFramework, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pRibbonFramework))))
        {
            // Connect the host application to the Ribbon framework.
            assert(m_pRibbonFramework);
            if (SUCCEEDED(hr = m_pRibbonFramework->Initialize(wnd, this)))
            {
                // Load the binary markup. APPLICATION_RIBBON is the default name generated by uicc.
                hr = m_pRibbonFramework->LoadUI(GetModuleHandle(0), L"APPLICATION_RIBBON");
            }
        }

        return hr;
    }

    // Destroys the ribbon.
    inline STDMETHODIMP CRibbon::DestroyRibbon()
    {
        HRESULT hr = S_OK;
        if (m_pRibbonFramework)
        {
            hr = m_pRibbonFramework->Destroy();
            m_pRibbonFramework->Release();
            m_pRibbonFramework = NULL;
        }

        return hr;
    }

    // Retrieves the height of the ribbon.
    inline STDMETHODIMP_(UINT32) CRibbon::GetRibbonHeight() const
    {
        HRESULT result = E_FAIL;
        IUIRibbon* pRibbon = NULL;
        UINT32 ribbonHeight = 0;

        if (GetRibbonFramework())
        {
            result = GetRibbonFramework()->GetView(0, IID_PPV_ARGS(&pRibbon));
            if (SUCCEEDED(result))
            {
                // Call to the framework to determine the desired height of the Ribbon.
                result = pRibbon->GetHeight(&ribbonHeight);
                pRibbon->Release();
            }
        }

        return ribbonHeight;
    }


    //////////////////////////////////////////////
    // Definitions for the CRibbonFrameT class template
    //

    // Get the frame's client area
    template <class T>
    inline CRect CRibbonFrameT<T>::GetViewRect() const
    {
        CRect clientRect = T::GetClientRect();

        clientRect.top += GetRibbonHeight();

        if (T::GetStatusBar().IsWindow() && T::GetStatusBar().IsWindowVisible())
            clientRect = T::ExcludeChildRect(clientRect, T::GetStatusBar());

        if (T::GetReBar().IsWindow() && T::GetReBar().IsWindowVisible())
            clientRect = T::ExcludeChildRect(clientRect, T::GetReBar());
        else
            if (T::GetToolBar().IsWindow() && T::GetToolBar().IsWindowVisible())
                clientRect = T::ExcludeChildRect(clientRect, T::GetToolBar());

        return clientRect;
    }

    // OnCreate is called automatically during window creation when a
    // WM_CREATE message received.
    // Tasks such as setting the icon, creating child windows, or anything
    // associated with creating windows are normally performed here.
    template <class T>
    inline int CRibbonFrameT<T>::OnCreate(CREATESTRUCT& cs)
    {
        if (GetWinVersion() >= 2601)    // WinVersion >= Windows 7
        {
            if (SUCCEEDED(CreateRibbon(*this)))
            {
                T::UseReBar(FALSE);     // Don't use a ReBar
                T::UseToolBar(FALSE);   // Don't use a ToolBar
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
            T::SetMenu(0);              // Disable the window menu
            T::SetFrameMenu(reinterpret_cast<HMENU>(0));
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
    inline STDMETHODIMP CRibbonFrameT<T>::OnViewChanged(UINT32, UI_VIEWTYPE typeId, IUIApplication::IUnknown*, UI_VIEWVERB verb, INT32)
    {
        HRESULT result = E_NOTIMPL;

        // Checks to see if the view that was changed was a Ribbon view.
        if (UI_VIEWTYPE_RIBBON == typeId)
        {
            switch (verb)
            {
            case UI_VIEWVERB_CREATE:    // The view was newly created.
                result = S_OK;
                break;
            case UI_VIEWVERB_SIZE:      // Ribbon size has changed.
                T::RecalcLayout();
                break;
            case UI_VIEWVERB_DESTROY:   // The view was destroyed.
                result = S_OK;
                break;
            case UI_VIEWVERB_ERROR:
                result = E_FAIL;
                break;
            }
        }

        return result;
    }

    // Populates the ribbon's recent items list.
    template <class T>
    inline HRESULT CRibbonFrameT<T>::PopulateRibbonRecentItems(PROPVARIANT* pvarValue)
    {
        LONG currentFile = 0;
        std::vector<CString> fileNames = T::GetMRUEntries();
        std::vector<CString>::const_iterator iter;
        ULONG_PTR fileCount = fileNames.size();
        HRESULT result = E_FAIL;
        SAFEARRAY* psa = SafeArrayCreateVector(VT_UNKNOWN, 0, (ULONG)fileCount);
        m_recentFiles.clear();

        if (psa != NULL)
        {
            for (iter = fileNames.begin(); iter != fileNames.end(); ++iter)
            {
                WCHAR curFileName[MAX_PATH] = {0};
                StrCopyW(curFileName, TtoW(*iter), MAX_PATH);

                RecentFilesPtr pRecentFiles(new CRecentFiles(curFileName));
                m_recentFiles.push_back(pRecentFiles);
                result = SafeArrayPutElement(psa, &currentFile, static_cast<void*>(pRecentFiles.get()));
                ++currentFile;
            }

            SAFEARRAYBOUND sab = {static_cast<ULONG>(currentFile), 0};
            SafeArrayRedim(psa, &sab);
            result = UIInitPropertyFromIUnknownArray(UI_PKEY_RecentItems, psa, pvarValue);

            SafeArrayDestroy(psa);  // Calls release for each element in the array.
        }

        return result;
    }

    // Updates the frame's MRU when the Ribbon Framework isn't used.
    template <class T>
    inline void CRibbonFrameT<T>::UpdateMRUMenu()
    {
        // Suppress UpdateMRUMenu when ribbon is used.
        if (GetRibbonFramework() != 0) return;

        T::UpdateMRUMenu();
    }


    ////////////////////////////////////////////////////////
    // Declaration of the nested CRecentFiles class
    //
    template <class T>
    inline CRibbonFrameT<T>::CRecentFiles::CRecentFiles(PWSTR fullPath) : m_count(0)
    {
        SHFILEINFOW sfi;
        ZeroMemory(&sfi, sizeof(sfi));
        DWORD_PTR ptr = NULL;
        m_fullPath[0] = L'\0';
        m_displayName[0] = L'\0';

        StrCopyW(m_fullPath, fullPath, MAX_PATH);
        ptr = ::SHGetFileInfoW(fullPath, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(sfi), SHGFI_DISPLAYNAME | SHGFI_USEFILEATTRIBUTES);
        if (ptr != NULL)
        {
            StrCopyW(m_displayName, sfi.szDisplayName, MAX_PATH);
        }
        else // Provide a reasonable fall back.
        {
            StrCopyW(m_displayName, m_fullPath, MAX_PATH);
        }

    }

    template <class T>
    inline STDMETHODIMP_(ULONG) CRibbonFrameT<T>::CRecentFiles::AddRef()
    {
        return InterlockedIncrement(&m_count);
    }

    template <class T>
    inline STDMETHODIMP_(ULONG) CRibbonFrameT<T>::CRecentFiles::Release()
    {
        return InterlockedDecrement(&m_count);
    }

    template <class T>
    inline STDMETHODIMP CRibbonFrameT<T>::CRecentFiles::QueryInterface(REFIID iid, void** ppObject)
    {
        if (!ppObject)
        {
            return E_POINTER;
        }

        if (iid == __uuidof(IUnknown))
        {
            *ppObject = static_cast<IUnknown*>(this);
        }
        else if (iid == __uuidof(IUISimplePropertySet))
        {
            *ppObject = static_cast<IUISimplePropertySet*>(this);
        }
        else
        {
            *ppObject = NULL;
            return E_NOINTERFACE;
        }

        AddRef();
        return S_OK;
    }

    // IUISimplePropertySet methods.
    template <class T>
    inline STDMETHODIMP CRibbonFrameT<T>::CRecentFiles::GetValue(__in REFPROPERTYKEY key, __out PROPVARIANT *ppropvar)
    {
        HRESULT result = HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);

        if (key == UI_PKEY_Label)
        {
            result = UIInitPropertyFromString(key, m_displayName, ppropvar);
        }
        else if (key == UI_PKEY_LabelDescription)
        {
            result = UIInitPropertyFromString(key, m_displayName, ppropvar);
        }

        return result;
    }


} // namespace Win32xx

#endif  // _WIN32XX_RIBBON_H_

