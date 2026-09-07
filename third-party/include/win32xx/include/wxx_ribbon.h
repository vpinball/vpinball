// Win32++   Version 10.3.0
// Release Date: 4th September 2026
//
//      David Nash
//      email: dnash@bigpond.net.au
//      url: https://sourceforge.net/projects/win32-framework
//           https://github.com/DavidNash2024/Win32xx
//
//
// Copyright (c) 2005-2026  David Nash
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


//////////////////////////////////////////////////////////
// wxx_ribbon.h
//  Declaration of the following classes:
//  CRibbon, CRibbonFrameT, CRibbonFrame, CRibbonDockFrame
//  CRibbonMDIFrame and CRibbonMDIDockFrame.
//

#ifndef WIN32XX_RIBBON_H_
#define WIN32XX_RIBBON_H_


// Notes :
//  To compile this code you will need a Microsoft compiler, Visual Studio
//  Community 2019 (or later).

#include <uiribbon.h>                   // Contained within the Windows 7 SDK.
#include <uiribbonpropertyhelpers.h>    // Provides a collection of helper functions.
#include <wrl/client.h>                 // Provides the ComPtr smart pointer.
#include <wrl/implements.h>             // Provides the RuntimeClass template.

#include "wxx_frame.h"
#include "wxx_dockframe.h"

namespace Win32xx
{
    ///////////////////////////////////////////////////////////
    // The CRibbonT class template is used to add the Ribbon to
    // a window. The ribbon user interface typically replaces
    // the menu and toolbar used by a frame window.
    template <class T>
    class CRibbonT : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags
        <Microsoft::WRL::ClassicCom>, IUIApplication, IUICommandHandler>
    {
    public:
        CRibbonT() : m_pWnd(nullptr) {}
        void SetWindow(T* pWnd) { m_pWnd = pWnd; }

        // IUIApplication methods.
        virtual STDMETHODIMP OnCreateUICommand(UINT32 commandId, __in UI_COMMANDTYPE typeID,
            __deref_out IUICommandHandler** ppCommandHandler) override;
        virtual STDMETHODIMP OnDestroyUICommand(UINT32 commandId, __in UI_COMMANDTYPE typeID,
            __in_opt IUICommandHandler* pCommandHandler) override;
        virtual STDMETHODIMP OnViewChanged(UINT32 viewId, __in UI_VIEWTYPE typeId,
            __in IUnknown* pView, UI_VIEWVERB verb, INT32 reasonCode) override;

        // IUICommandHandler methods.
        virtual STDMETHODIMP Execute(UINT32 commandId, UI_EXECUTIONVERB verb,
            __in_opt const PROPERTYKEY* key, __in_opt const PROPVARIANT* value,
            __in_opt IUISimplePropertySet* pCommandExecutionProperties) override;
        virtual STDMETHODIMP UpdateProperty(UINT32 commandId, __in REFPROPERTYKEY key,
            __in_opt const PROPVARIANT* currentValue, __out PROPVARIANT* newValue) override;

        // Other methods.
        STDMETHODIMP CreateRibbon(HWND wnd);
        void DestroyRibbon();
        Microsoft::WRL::ComPtr<IUIFramework> GetRibbonFramework() const;
        STDMETHODIMP_(UINT32) GetRibbonHeight() const;

    private:
        Microsoft::WRL::ComPtr<IUIFramework> m_pFramework = nullptr;
        T* m_pWnd;
    };

    ///////////////////////////////////////////////////
    // Declaration of the CRibbonFrameT class template.
    //

    // The CRibbonFrameT is the base class for frames that support the Ribbon
    // Framework. The T parameter can be either CFrame, CDockFrame, CMDIFrame
    // or CMDIDockFrame.
    template <class T>
    class CRibbonFrameT : public T
    {
    public:
        // A nested class for the MRU item properties.
        class CRecentFiles : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags
        <Microsoft::WRL::ClassicCom>, IUISimplePropertySet>
        {
        public:
            CRecentFiles(PWSTR pFullPath);

            // IUISimplePropertySet methods.
            STDMETHODIMP GetValue(__in REFPROPERTYKEY key, __out PROPVARIANT* value) override;

        private:
            WCHAR m_displayName[MAX_PATH];
            WCHAR m_fullPath[MAX_PATH];
        };

        CRibbonFrameT();
        virtual ~CRibbonFrameT() override = default;

        // IUIApplication methods.
        virtual STDMETHODIMP OnCreateUICommand(UINT32 commandId, __in UI_COMMANDTYPE typeID,
            __deref_out IUICommandHandler** ppCommandHandler);
        virtual STDMETHODIMP OnDestroyUICommand(UINT32, __in UI_COMMANDTYPE,
            __in_opt IUICommandHandler*);
        virtual STDMETHODIMP OnViewChanged(UINT32, UI_VIEWTYPE, IUIApplication::IUnknown*,
            UI_VIEWVERB, INT32);

        // IUICommandHandler methods.
        virtual STDMETHODIMP Execute(UINT32, UI_EXECUTIONVERB, const PROPERTYKEY*,
            const PROPVARIANT*, IUISimplePropertySet*);
        virtual STDMETHODIMP UpdateProperty(UINT32, __in REFPROPERTYKEY,
            __in_opt const PROPVARIANT*, __out PROPVARIANT*);

        // Other methods.
        STDMETHODIMP CreateRibbon(HWND wnd);
        void DestroyRibbon();
        Microsoft::WRL::ComPtr<CRibbonT<CRibbonFrameT<T>>> GetRibbon() const;
        Microsoft::WRL::ComPtr<IUIFramework> GetRibbonFramework() const;
        STDMETHODIMP_(UINT32) GetRibbonHeight() const;

    protected:
        virtual CRect GetViewRect() const override;
        virtual int  OnCreate(CREATESTRUCT& cs) override;
        virtual void OnDestroy() override;
        virtual HRESULT PopulateRibbonRecentItems(PROPVARIANT* value);
        virtual void UpdateMRUMenu() override;

    private:
        CRibbonFrameT(const CRibbonFrameT&) = delete;
        CRibbonFrameT& operator=(const CRibbonFrameT&) = delete;

        std::vector<Microsoft::WRL::ComPtr<CRecentFiles>> m_recentFiles;
        Microsoft::WRL::ComPtr<CRibbonT<CRibbonFrameT<T>>> m_ribbon;
    };

    /////////////////////////////////////////////////
    // This class provides an SDI frame with a Ribbon
    // Framework.
    class CRibbonFrame : public CRibbonFrameT<CFrame>
    {
    public:
        CRibbonFrame() = default;
        virtual ~CRibbonFrame() override = default;

    private:
        CRibbonFrame(const CRibbonFrame&) = delete;
        CRibbonFrame& operator=(const CRibbonFrame&) = delete;
    };

    /////////////////////////////////////////////////////
    // CRibbonDockFrame manages a frame that supports the
    // ribbon user interface and docking.
    class CRibbonDockFrame : public CRibbonFrameT<CDockFrame>
    {
    public:
        CRibbonDockFrame() = default;
        virtual ~CRibbonDockFrame() override = default;

    private:
        CRibbonDockFrame(const CRibbonDockFrame&) = delete;
        CRibbonDockFrame& operator=(const CRibbonDockFrame&) = delete;
    };

    /////////////////////////////////////////////////////////////
    // CRibbonMDIFrame manages a frame that supports the Multiple
    // Document Interface (MDI) and the Ribbon user interface.
    class CRibbonMDIFrame : public CRibbonFrameT<CMDIFrame>
    {
    public:
        CRibbonMDIFrame() = default;
        virtual ~CRibbonMDIFrame() override = default;

    private:
        CRibbonMDIFrame(const CRibbonMDIFrame&) = delete;
        CRibbonMDIFrame& operator=(const CRibbonMDIFrame&) = delete;
    };

    ////////////////////////////////////////////////////////////////
    // CRibbonMDIDockFrame manages a frame that supports the
    // Multiple Document Interface (MDI), the Ribbon user interface,
    // and docking.
    class CRibbonMDIDockFrame : public CRibbonFrameT<CMDIDockFrame>
    {
    public:
        CRibbonMDIDockFrame() = default;
        virtual ~CRibbonMDIDockFrame() override = default;

    private:
        CRibbonMDIDockFrame(const CRibbonMDIDockFrame&) = delete;
        CRibbonMDIDockFrame& operator=(const CRibbonMDIDockFrame&) = delete;
    };

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

namespace Win32xx
{
    ///////////////////////////////////////////////
    // Definitions for the CRibbonT class template.
    //

    // Responds to execute events on commands bound to the Command handler.
    template <class T>
    inline STDMETHODIMP CRibbonT<T>::Execute(UINT32 commandId,
        UI_EXECUTIONVERB verb, __in_opt const PROPERTYKEY* key,
        __in_opt const PROPVARIANT* propValue,
        __in_opt IUISimplePropertySet* pCommandExecutionProperties)
    {
        if (!m_pWnd)
            return E_FAIL;

        return m_pWnd->Execute(commandId, verb, key, propValue, pCommandExecutionProperties);
    }

    // Called by the Ribbon framework for each command specified in markup,
    // to bind the Command to an IUICommandHandler.
    template <class T>
    inline STDMETHODIMP CRibbonT<T>::OnCreateUICommand(UINT32 commandId,
        __in UI_COMMANDTYPE typeID, __deref_out IUICommandHandler** ppCommandHandler)
    {
        if (!m_pWnd)
            return E_FAIL;

        return m_pWnd->OnCreateUICommand(commandId, typeID, ppCommandHandler);
    }

    // Called when the state of the Ribbon changes, for example, created,
    // destroyed, or resized.
    template <class T>
    inline STDMETHODIMP CRibbonT<T>::OnViewChanged(UINT32 viewId,
        __in UI_VIEWTYPE typeId, __in IUnknown* pView,
        UI_VIEWVERB verb, INT32 reasonCode)
    {
        if (!m_pWnd)
            return E_FAIL;

        return m_pWnd->OnViewChanged(viewId, typeId, pView, verb, reasonCode);
    }

    // Called by the Ribbon framework for each command at the time of ribbon
    // destruction.
    template <class T>
    inline STDMETHODIMP CRibbonT<T>::OnDestroyUICommand(UINT32 commandId,
        __in UI_COMMANDTYPE typeID, __in_opt IUICommandHandler* pCommandHandler)
    {
        if (!m_pWnd)
            return E_FAIL;

        return m_pWnd->OnDestroyUICommand(commandId, typeID, pCommandHandler);
    }

    // Called by the Ribbon framework when a command property (PKEY) needs to
    // be updated.
    template <class T>
    inline STDMETHODIMP CRibbonT<T>::UpdateProperty(UINT32 commandId,
        __in REFPROPERTYKEY key, __in_opt const PROPVARIANT* currentValue,
        __out PROPVARIANT* newValue)
    {
        if (!m_pWnd)
            return E_FAIL;

        return m_pWnd->UpdateProperty(commandId, key, currentValue, newValue);
    }

    // Creates the ribbon.
    template <class T>
    inline STDMETHODIMP CRibbonT<T>::CreateRibbon(HWND wnd)
    {
        // Instantiate the Ribbon framework.
        HRESULT hr;
        if (SUCCEEDED(hr = ::CoCreateInstance(CLSID_UIRibbonFramework, nullptr,
            CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pFramework))))
        {
            // Initialize the Ribbon framework.
            if (SUCCEEDED(hr = m_pFramework->Initialize(wnd, this)))
            {
                // Load the binary markup resource.
                hr = m_pFramework->LoadUI(GetModuleHandle(0), L"APPLICATION_RIBBON");
            }
        }

        return hr;
    }

    // Destroys the ribbon.
    template <class T>
    inline void CRibbonT<T>::DestroyRibbon()
    {
        if (m_pFramework)
        {
            m_pFramework->Destroy();
        }

        // Set to nullptr to trigger Release() immediately.
        m_pFramework = nullptr;
    }

    // Retrieves a pointer to IUIFramework interface for the Windows Ribbon framework,
    // assigned when the ribbon is created.
    template <class T>
    inline Microsoft::WRL::ComPtr<IUIFramework> CRibbonT<T>::GetRibbonFramework() const
    {
        return m_pFramework;
    }

    template <class T>
    inline STDMETHODIMP_(UINT32) CRibbonT<T>::GetRibbonHeight() const
    {
        Microsoft::WRL::ComPtr<IUIRibbon> pRibbon;
        UINT32 ribbonHeight = 0;

        if (m_pFramework && SUCCEEDED(m_pFramework->GetView(0, IID_PPV_ARGS(&pRibbon))))
        {
            if (FAILED(pRibbon->GetHeight(&ribbonHeight)))
                ribbonHeight = 0;
        }

        return ribbonHeight;
    }


    ////////////////////////////////////////////////////
    // Definitions for the CRibbonFrameT class template.
    //

    // Constructor for the CRibbonFrameT class template. Sets the ribbon's
    // window to this frame.
    template <class T>
    inline CRibbonFrameT<T>::CRibbonFrameT()
    {
        m_ribbon = Microsoft::WRL::Make<CRibbonT<CRibbonFrameT<T>>>();
        if (m_ribbon)
        {
            m_ribbon->SetWindow(this);
        }
    }

    // Creates the ribbon.
    template <class T>
    inline HRESULT CRibbonFrameT<T>::CreateRibbon(HWND wnd)
    {
        return m_ribbon ? m_ribbon->CreateRibbon(wnd) : E_FAIL;
    }

    // Destroys the ribbon.
    template <class T>
    inline void CRibbonFrameT<T>::DestroyRibbon()
    {
        if (m_ribbon) m_ribbon->DestroyRibbon();
    }

    // Retrieves a pointer to both the IUIApplication and IUICommandHandler
    // interfaces for the Windows Ribbon framework.
    template <class T>
    inline Microsoft::WRL::ComPtr<CRibbonT<CRibbonFrameT<T>>> CRibbonFrameT<T>::GetRibbon() const
    {
        return m_ribbon;
    }

    // Retrieves a pointer to IUIFramework interface for the Windows Ribbon framework,
    // assigned when the ribbon is created.
    template <class T>
    inline Microsoft::WRL::ComPtr<IUIFramework> CRibbonFrameT<T>::GetRibbonFramework() const
    {
        return m_ribbon ? m_ribbon->GetRibbonFramework() : nullptr;
    }

    // Retrieves the height of the ribbon.
    template <class T>
    inline UINT32 CRibbonFrameT<T>::GetRibbonHeight() const
    {
        return m_ribbon ? m_ribbon->GetRibbonHeight() : 0;
    }

    // Responds to execute events on commands bound to the Command handler.
    template <class T>
    inline STDMETHODIMP CRibbonFrameT<T>::Execute(UINT32, UI_EXECUTIONVERB, const PROPERTYKEY*,
        const PROPVARIANT*, IUISimplePropertySet*)
    {
        return E_NOTIMPL;
    }

    // Get the frame's client area.
    template <class T>
    inline CRect CRibbonFrameT<T>::GetViewRect() const
    {
        CRect clientRect = T::GetClientRect();
        clientRect.top += GetRibbonHeight();

        if (T::GetStatusBar().IsWindow() && T::GetStatusBar().IsWindowVisible())
            clientRect = T::ExcludeChildRect(clientRect, T::GetStatusBar());

        if (T::GetReBar().IsWindow() && T::GetReBar().IsWindowVisible())
            clientRect = T::ExcludeChildRect(clientRect, T::GetReBar());
        else if (T::GetToolBar().IsWindow() && T::GetToolBar().IsWindowVisible())
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
        if (GetWinVersion() >= 2601) // Windows 7 or newer
        {
            if (SUCCEEDED(CreateRibbon(*this)))
            {
                T::UseReBar(FALSE);     // Don't use a ReBar.
                T::UseToolBar(FALSE);   // Don't use a ToolBar.
            }
            else
            {
                TRACE("\n*** WARNING Failed to create Ribbon. ***\n\n");
                DestroyRibbon();
            }
        }

        T::OnCreate(cs);
        if (GetRibbonFramework())
        {
            T::SetMenu(nullptr);
            T::SetFrameMenu(0);
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

    // Called for each Command specified in the Windows Ribbon framework markup to
    // bind the Command to an IUICommandHandler.
    template <class T>
    inline STDMETHODIMP CRibbonFrameT<T>::OnCreateUICommand(UINT32,
        __in UI_COMMANDTYPE, __deref_out IUICommandHandler** ppCommandHandler)
    {
        if (!m_ribbon) return E_FAIL;

        // Pass the request back to our WRL Ribbon instance
        return m_ribbon.CopyTo(ppCommandHandler);
    }

    // // Called for each Command specified in the Windows Ribbon framework markup
    // when the application window is destroyed.
    template <class T>
    inline STDMETHODIMP CRibbonFrameT<T>::OnDestroyUICommand(UINT32, __in UI_COMMANDTYPE,
        __in_opt IUICommandHandler*)
    {
        return E_NOTIMPL;
    }

    // OnViewChanged is called when the ribbon has changed.
    template <class T>
    inline STDMETHODIMP CRibbonFrameT<T>::OnViewChanged(UINT32,
        UI_VIEWTYPE typeId, IUnknown*, UI_VIEWVERB verb, INT32)
    {
        HRESULT result = E_NOTIMPL;

        // Checks to see if the view that was changed was a Ribbon view.
        if (UI_VIEWTYPE_RIBBON == typeId)
        {
            switch (verb)
            {
            case UI_VIEWVERB_CREATE:    // The view was newly created.
            case UI_VIEWVERB_DESTROY:   // The view was destroyed.
                result = S_OK;
                break;
            case UI_VIEWVERB_SIZE:      // Ribbon size has changed.
                T::RecalcLayout();
                break;
            case UI_VIEWVERB_ERROR:
                result = E_FAIL;
                break;

            default: break;
            }
        }

        return result;
    }

    // Populates the ribbon's recent items list.
    template <class T>
    inline HRESULT CRibbonFrameT<T>::PopulateRibbonRecentItems(PROPVARIANT* pvarValue)
    {
        std::vector<CString> fileNames = T::GetMRUEntries();
        HRESULT result = E_FAIL;
        SAFEARRAY* psa = ::SafeArrayCreateVector(VT_UNKNOWN, 0, static_cast<ULONG>(fileNames.size()));
        m_recentFiles.clear();

        if (psa == nullptr)
            return E_OUTOFMEMORY;

        LONG currentFile = 0;
        HRESULT hr = S_OK;

        for (const CString& fileName : fileNames)
        {
            WCHAR curFileName[MAX_PATH] = {};
            StrCopyW(curFileName, TtoW(fileName), MAX_PATH);

            m_recentFiles.push_back(Microsoft::WRL::Make<CRecentFiles>(curFileName));
            IUnknown* pUnk = m_recentFiles.back().Get();
            hr = ::SafeArrayPutElement(psa, &currentFile, static_cast<void*>(pUnk));
            if (FAILED(hr))
            {
                ::SafeArrayDestroy(psa);
                return hr;
            }

            ++currentFile;
        }

        SAFEARRAYBOUND sab = { static_cast<ULONG>(currentFile), 0 };
        hr = ::SafeArrayRedim(psa, &sab);
        if (FAILED(hr))
        {
            ::SafeArrayDestroy(psa);
            return hr;
        }

        result = ::UIInitPropertyFromIUnknownArray(UI_PKEY_RecentItems, psa, pvarValue);

        ::SafeArrayDestroy(psa);  // Calls release for each element in the array.
        return result;
    }

    // Updates the frame's MRU when the Ribbon Framework isn't used.
    template <class T>
    inline void CRibbonFrameT<T>::UpdateMRUMenu()
    {
        // Update the MRU menu when the ribbon isn't used.
        if (GetRibbonFramework() == nullptr)
            T::UpdateMRUMenu();
    }

    // Called by the Ribbon framework when a command property (PKEY) needs to
    // be updated.
    template <class T>
    inline STDMETHODIMP CRibbonFrameT<T>::UpdateProperty(UINT32, __in REFPROPERTYKEY,
        __in_opt const PROPVARIANT*, __out PROPVARIANT*)
    {
        return E_NOTIMPL;
    }


    ////////////////////////////////////////////////
    // Declaration of the nested CRecentFiles class.
    //

    // Constructor for the CRecentFiles class. Initializes the display name and
    // full path for a recent file.
    template <class T>
    inline CRibbonFrameT<T>::CRecentFiles::CRecentFiles(PWSTR fullPath)
    {
        SHFILEINFOW sfi = {};
        DWORD_PTR ptr = 0;
        m_fullPath[0] = L'\0';
        m_displayName[0] = L'\0';

        StrCopyW(m_fullPath, fullPath, MAX_PATH);
        ptr = ::SHGetFileInfoW(fullPath, FILE_ATTRIBUTE_NORMAL, &sfi,
            sizeof(sfi), SHGFI_DISPLAYNAME | SHGFI_USEFILEATTRIBUTES);

        if (ptr != 0)
        {
            StrCopyW(m_displayName, sfi.szDisplayName, MAX_PATH);
        }
        else // Provide a reasonable fall back.
        {
            StrCopyW(m_displayName, m_fullPath, MAX_PATH);
        }

    }

        //Retrieves the value identified by a property key.
    template <class T>
    inline STDMETHODIMP CRibbonFrameT<T>::CRecentFiles::GetValue(
        __in REFPROPERTYKEY key, __out PROPVARIANT *ppropvar)
    {
        HRESULT result = HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);

        if (key == UI_PKEY_Label)
        {
            result = ::UIInitPropertyFromString(key, m_displayName, ppropvar);
        }
        else if (key == UI_PKEY_LabelDescription)
        {
            result = ::UIInitPropertyFromString(key, m_displayName, ppropvar);
        }

        return result;
    }


} // namespace Win32xx

#endif  // WIN32XX_RIBBON_H_

