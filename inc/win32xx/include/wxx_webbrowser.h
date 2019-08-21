// Win32++   Version 8.7.0
// Release Date: 12th August 2019
//
//      David Nash
//      email: dnash@bigpond.net.au
//      url: https://sourceforge.net/projects/win32-framework
//
//
// Copyright (c) 2005-2019  David Nash
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

#ifndef _WIN32XX_WEBBROWSER_H_
#define _WIN32XX_WEBBROWSER_H_

#include "wxx_appcore0.h"
#include <exdisp.h>
#include <ocidl.h>

#ifdef _MSC_VER
  #pragma warning ( push )
  #pragma warning ( disable : 4091 )  // temporarily disable C4091 warning
#endif // _MSC_VER

#include <shlobj.h>

#ifdef _MSC_VER
  #pragma warning ( pop )
#endif // _MSC_VER


namespace Win32xx
{
    ///////////////////////////////////////////////////
    // Declaration of the CAXWindow class
    // This class implements an ActiveX control container.
    class CAXWindow : public IOleClientSite, public IOleInPlaceSite, public IOleInPlaceFrame,
                            public IOleControlSite, public IDispatch
    {
    public:
        CAXWindow();
        virtual ~CAXWindow();
        virtual void Activate(BOOL focus);
        virtual void CreateControl(BSTR clsidName);
        virtual void CreateControl(REFCLSID clsid);
        virtual void Remove();
        virtual void SetParent(HWND hWndParent);
        virtual void SetLocation(int x, int y, int width, int height);
        virtual void SetVisible(BOOL isVisible);
        virtual void SetStatusWindow(HWND status);
        virtual void TranslateKey(MSG msg);
        IDispatch* GetDispatch();
        IUnknown* GetUnknown();

        // IUnknown Methods
        STDMETHODIMP QueryInterface(REFIID riid, void** ppObject);
        STDMETHODIMP_(ULONG) AddRef();
        STDMETHODIMP_(ULONG) Release();

        // IOleClientSite Methods
        STDMETHODIMP SaveObject();
        STDMETHODIMP GetMoniker(DWORD assign, DWORD whichMoniker, LPMONIKER* ppMk);
        STDMETHODIMP GetContainer(LPOLECONTAINER* ppContainer);
        STDMETHODIMP ShowObject();
        STDMETHODIMP OnShowWindow(BOOL show);
        STDMETHODIMP RequestNewObjectLayout();

        // IOleWindow Methods
        STDMETHODIMP GetWindow(HWND* pHwnd);
        STDMETHODIMP ContextSensitiveHelp(BOOL enterMode);

        // IOleInPlaceSite Methods
        STDMETHODIMP CanInPlaceActivate();
        STDMETHODIMP OnInPlaceActivate();
        STDMETHODIMP OnUIActivate();
        STDMETHODIMP GetWindowContext(IOleInPlaceFrame** ppFrame, IOleInPlaceUIWindow** ppDoc, LPRECT pRect, LPRECT pClipRect, LPOLEINPLACEFRAMEINFO pFrameInfo);
        STDMETHODIMP Scroll(SIZE scrollExtent);
        STDMETHODIMP OnUIDeactivate(BOOL undoable);
        STDMETHODIMP OnInPlaceDeactivate();
        STDMETHODIMP DiscardUndoState();
        STDMETHODIMP DeactivateAndUndo();
        STDMETHODIMP OnPosRectChange(LPCRECT pRect);

        // IOleInPlaceUIWindow Methods
        STDMETHODIMP GetBorder(LPRECT pBorderRect);
        STDMETHODIMP RequestBorderSpace(LPCBORDERWIDTHS pBorderWidths);
        STDMETHODIMP SetBorderSpace(LPCBORDERWIDTHS pBorderWidths);
        STDMETHODIMP SetActiveObject(IOleInPlaceActiveObject* pActiveObject, LPCOLESTR pObjName);

        // IOleInPlaceFrame Methods
        STDMETHODIMP InsertMenus(HMENU shared, LPOLEMENUGROUPWIDTHS pMenuWidths);
        STDMETHODIMP SetMenu(HMENU shared, HOLEMENU holemenu, HWND activeObject);
        STDMETHODIMP RemoveMenus(HMENU shared);
        STDMETHODIMP SetStatusText(LPCOLESTR pStatusText);
        STDMETHODIMP EnableModeless(BOOL enable);
        STDMETHODIMP TranslateAccelerator(LPMSG pMsg, WORD id);

        // IOleControlSite Methods
        STDMETHODIMP OnControlInfoChanged();
        STDMETHODIMP LockInPlaceActive(BOOL lock);
        STDMETHODIMP GetExtendedControl(IDispatch** ppDisp);
        STDMETHODIMP TransformCoords(POINTL* pHimetric, POINTF* pContainer, DWORD flags);
        STDMETHODIMP TranslateAccelerator(LPMSG pMsg, DWORD modifiers);
        STDMETHODIMP OnFocus(BOOL gotFocus);
        STDMETHODIMP ShowPropertyFrame();

        // IDispatch Methods
        STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR** pNames, unsigned int namesCount, LCID lcid, DISPID* pID);
        STDMETHODIMP GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo** pptinfo);
        STDMETHODIMP GetTypeInfoCount(unsigned int* pctinfo);
        STDMETHODIMP Invoke(DISPID dispID, REFIID riid, LCID lcid, WORD flags, DISPPARAMS* pParams, VARIANT* result, EXCEPINFO* pExecInfo, unsigned int* pArgErr);

    private:
         ULONG       m_count;       // ref count
         HWND        m_ax;          // window handle of the container
         HWND        m_status;      // status window handle
         IUnknown*   m_pUnk;        // IUnknown of contained object
         CRect       m_controlRect; // size of control
    };


    ///////////////////////////////////////////////
    // Declaration of the CWebBrowser class
    // This class provides the functionality of a WebBrower, using the IWebBrower2 interface.
    class CWebBrowser : public CWnd
    {
    public:
        CWebBrowser();
        virtual ~CWebBrowser();

        //Attributes
        LPDISPATCH GetApplication() const;
        CAXWindow& GetAXWindow() const { return m_axContainer; }
        BOOL GetBusy() const;
        LPDISPATCH GetContainer() const;
        LPDISPATCH GetDocument() const;
        BOOL GetFullScreen() const;
        long GetHeight() const;
        IWebBrowser2* GetIWebBrowser2() const { return m_pIWebBrowser2; }
        long GetLeft() const;
        CString GetLocationName() const;
        CString GetLocationURL() const;
        BOOL GetOffline() const;
        LPDISPATCH GetParent() const;
        READYSTATE GetReadyState() const;
        BOOL GetRegisterAsBrowser() const;
        BOOL GetTheaterMode() const;
        long GetTop() const;
        BOOL GetTopLevelContainer() const;
        CString GetType() const;
        BOOL GetVisible() const;
        long GetWidth() const;
        void SetFullScreen(BOOL isFullScreen);
        void SetHeight(long height);
        void SetLeft(long leftEdge);
        void SetOffline(BOOL isOffline);
        void SetRegisterAsBrowser(BOOL isBrowser);
        void SetTheaterMode(BOOL isTheaterMode);
        void SetTop(long topEdge);
        void SetVisible(BOOL isVisible);
        void SetWidth(long width);

        // Operations
        void AddWebBrowserControl();
        void ExecWB(OLECMDID cmdID, OLECMDEXECOPT cmdExecOpt, VARIANT* pIn, VARIANT* pOut);
        VARIANT GetProperty( LPCTSTR pProperty);
        void GoBack();
        void GoForward();
        void GoHome();
        void GoSearch();
        void Navigate(LPCTSTR pURL, DWORD flags = 0, LPCTSTR pTargetFrameName = NULL,
                        LPCTSTR pHeaders = NULL, LPVOID pPostData = NULL, DWORD postDataLen = 0);
        void Navigate2(LPITEMIDLIST pIDL, DWORD flags = 0, LPCTSTR pTargetFrameName = NULL);
        void Navigate2(LPCTSTR pURL, DWORD flags = 0, LPCTSTR pTargetFrameName = NULL,
                         LPCTSTR pHeaders = NULL, LPVOID pPostData = NULL, DWORD postDataLen = 0);
        void PutProperty(LPCTSTR pPropertyName, const VARIANT& value);
        void PutProperty(LPCTSTR pPropertyName, double value);
        void PutProperty(LPCTSTR pPropertyName, long value);
        void PutProperty(LPCTSTR pPropertyName, LPCTSTR pValue);
        void PutProperty(LPCTSTR pPropertyName, short value);
        void Refresh();
        void Refresh2(int evel);
        void Stop();

    protected:
        // Override these as required
        virtual void OnAttach();
        virtual int  OnCreate(CREATESTRUCT& cs);
        virtual void OnDestroy();
        virtual void OnSize(int width, int height);

        // Not intended to be overridden
        LRESULT WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam);

    private:
        UINT    GetPidlLength(LPITEMIDLIST pidl);
        mutable CAXWindow  m_axContainer;    // The ActiveX Container
        IWebBrowser2* m_pIWebBrowser2;  // Interface to the ActiveX web browser control
    };

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

namespace Win32xx
{

    /////////////////////////////////////////
    // Definitions for the CAXWindow class
    //

    inline CAXWindow::CAXWindow() : m_count(1), m_ax(NULL), m_status(0), m_pUnk(NULL)
    {
    }

    inline CAXWindow::~CAXWindow()
    {
        if (m_pUnk)
            m_pUnk->Release();

        Release();
    }

    inline void CAXWindow::CreateControl(BSTR clsidName)
    {
        CLSID   clsid;
        if (NOERROR == CLSIDFromString(clsidName, &clsid))
            CreateControl(clsid);
    }

    inline void CAXWindow::Activate(BOOL focus)
    {
        if (!m_pUnk)
            return;

        if (focus)
        {
            IOleObject* pObject;
            HRESULT result = m_pUnk->QueryInterface(IID_IOleObject, reinterpret_cast<void**>(&pObject));
            if (FAILED(result))
                return;

            pObject->DoVerb(OLEIVERB_UIACTIVATE, NULL, this, 0, m_ax, &m_controlRect);
            pObject->Release();
        }
    }

    inline void CAXWindow::CreateControl(REFCLSID clsid)
    {
        if (S_OK != CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER, IID_IUnknown, reinterpret_cast<void**>(&m_pUnk)))
            return;

        assert(m_pUnk);

        IOleObject* pObject;
        HRESULT result = m_pUnk->QueryInterface(IID_IOleObject, reinterpret_cast<void**>(&pObject));
        if (FAILED(result))
            return;

        pObject->SetClientSite(this);
        pObject->Release();

        IPersistStreamInit* ppsi;
        result = m_pUnk->QueryInterface(IID_IPersistStreamInit, reinterpret_cast<void**>(&ppsi));
        if (SUCCEEDED(result))
        {
            ppsi->InitNew();
            ppsi->Release();
        }
    }

    inline STDMETHODIMP_(ULONG) CAXWindow::AddRef()
    {
        return ++m_count;
    }

    inline STDMETHODIMP CAXWindow::CanInPlaceActivate()
    {
        return S_OK;
    }

    inline STDMETHODIMP CAXWindow::ContextSensitiveHelp(BOOL enterMode)
    {
        UNREFERENCED_PARAMETER(enterMode);
        return E_NOTIMPL;
    }

    inline STDMETHODIMP CAXWindow::DeactivateAndUndo()
    {
        return E_NOTIMPL;
    }

    inline STDMETHODIMP CAXWindow::DiscardUndoState()
    {
        return E_NOTIMPL;
    }

    inline STDMETHODIMP CAXWindow::EnableModeless(BOOL enable)
    {
        UNREFERENCED_PARAMETER(enable);
        return E_NOTIMPL;
    }

    inline STDMETHODIMP CAXWindow::GetBorder(LPRECT pBorderRect)
    {
        UNREFERENCED_PARAMETER(pBorderRect);
        return E_NOTIMPL;
    }

    inline STDMETHODIMP CAXWindow::GetContainer(LPOLECONTAINER* ppContainer)
    {
        UNREFERENCED_PARAMETER(ppContainer);
        return E_NOINTERFACE;
    }

    // Returns the IDispatch COM interface.
    // The caller should release the IDispatch pointer.
    inline IDispatch* CAXWindow::GetDispatch()
    {
        if (!m_pUnk)
            return NULL;

        IDispatch*  pDisp;

        m_pUnk->QueryInterface(IID_IDispatch, reinterpret_cast<void**>(&pDisp));
        return pDisp;
    }

    inline STDMETHODIMP CAXWindow::GetExtendedControl(IDispatch** ppDisp)
    {
        if (ppDisp == NULL)
            return E_INVALIDARG;

        *ppDisp = static_cast<IDispatch*>(this);
        (*ppDisp)->AddRef();

        return S_OK;
    }

    inline STDMETHODIMP CAXWindow::GetIDsOfNames(REFIID riid, OLECHAR** pNames, unsigned int namesCount, LCID lcid, DISPID* pID)
    {
        UNREFERENCED_PARAMETER((IID)riid);      // IID cast required for the MinGW compiler
        UNREFERENCED_PARAMETER(pNames);
        UNREFERENCED_PARAMETER(namesCount);
        UNREFERENCED_PARAMETER(lcid);

        *pID = DISPID_UNKNOWN;
        return DISP_E_UNKNOWNNAME;
    }

    inline STDMETHODIMP CAXWindow::GetMoniker(DWORD assign, DWORD whichMoniker, LPMONIKER* ppMk)
    {
        UNREFERENCED_PARAMETER(assign);
        UNREFERENCED_PARAMETER(whichMoniker);
        UNREFERENCED_PARAMETER(ppMk);
        return E_NOTIMPL;
    }

    inline STDMETHODIMP CAXWindow::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo** pptinfo)
    {
        UNREFERENCED_PARAMETER(itinfo);
        UNREFERENCED_PARAMETER(lcid);
        UNREFERENCED_PARAMETER(pptinfo);
        return E_NOTIMPL;
    }

    inline STDMETHODIMP CAXWindow::GetTypeInfoCount(unsigned int* pctinfo)
    {
        UNREFERENCED_PARAMETER(pctinfo);
        return E_NOTIMPL;
    }

    inline IUnknown* CAXWindow::GetUnknown()
    {
        if (!m_pUnk)
            return NULL;

        m_pUnk->AddRef();
        return m_pUnk;
    }

    inline STDMETHODIMP CAXWindow::GetWindow(HWND* pHwnd)
    {
        if (!IsWindow(m_ax))
            return S_FALSE;

        *pHwnd = m_ax;
        return S_OK;
    }

    inline STDMETHODIMP CAXWindow::GetWindowContext (IOleInPlaceFrame** ppFrame, IOleInPlaceUIWindow** ppIIPUIWin,
                                      LPRECT pRect, LPRECT pClipRect, LPOLEINPLACEFRAMEINFO pFrameInfo)
    {
        *ppFrame = (IOleInPlaceFrame*)this;
        *ppIIPUIWin = NULL;

        RECT rect;
        GetClientRect(m_ax, &rect);
        pRect->left       = 0;
        pRect->top        = 0;
        pRect->right      = rect.right;
        pRect->bottom     = rect.bottom;

        CopyRect(pClipRect, pRect);

        pFrameInfo->cb             = sizeof(OLEINPLACEFRAMEINFO);
        pFrameInfo->fMDIApp        = FALSE;
        pFrameInfo->hwndFrame      = m_ax;
        pFrameInfo->haccel         = 0;
        pFrameInfo->cAccelEntries  = 0;

        (*ppFrame)->AddRef();
        return S_OK;
    }

    inline STDMETHODIMP CAXWindow::InsertMenus(HMENU shared, LPOLEMENUGROUPWIDTHS pMenuWidths)
    {
        UNREFERENCED_PARAMETER(shared);
        UNREFERENCED_PARAMETER(pMenuWidths);
        return E_NOTIMPL;
    }

    inline STDMETHODIMP CAXWindow::Invoke(DISPID dispID, REFIID riid, LCID lcid, WORD flags, DISPPARAMS* pParams, VARIANT* result, EXCEPINFO* pExecInfo, unsigned int* pArgErr)
    {
        UNREFERENCED_PARAMETER(dispID);
        UNREFERENCED_PARAMETER((IID)riid);      // IID cast required for the MinGW compiler
        UNREFERENCED_PARAMETER(lcid);
        UNREFERENCED_PARAMETER(flags);
        UNREFERENCED_PARAMETER(pParams);
        UNREFERENCED_PARAMETER(result);
        UNREFERENCED_PARAMETER(pExecInfo);
        UNREFERENCED_PARAMETER(pArgErr);
        return DISP_E_MEMBERNOTFOUND;
    }

    inline STDMETHODIMP CAXWindow::LockInPlaceActive(BOOL lock)
    {
        UNREFERENCED_PARAMETER(lock);
        return E_NOTIMPL;
    }

    inline STDMETHODIMP CAXWindow::OnControlInfoChanged()
    {
        return E_NOTIMPL;
    }

    inline STDMETHODIMP CAXWindow::OnFocus(BOOL gotFocus)
    {
        UNREFERENCED_PARAMETER(gotFocus);
        return E_NOTIMPL;
    }

    inline STDMETHODIMP CAXWindow::OnInPlaceActivate()
    {
        return S_OK;
    }

    inline STDMETHODIMP CAXWindow::OnInPlaceDeactivate()
    {
        return S_OK;
    }

    inline STDMETHODIMP CAXWindow::OnPosRectChange(LPCRECT pPosRect)
    {
        UNREFERENCED_PARAMETER(pPosRect);
        return S_OK;
    }

    inline STDMETHODIMP CAXWindow::OnShowWindow(BOOL show)
    {
        UNREFERENCED_PARAMETER(show);
        return S_OK;
    }

    inline STDMETHODIMP CAXWindow::OnUIActivate()
    {
        return S_OK;
    }

    inline STDMETHODIMP CAXWindow::OnUIDeactivate(BOOL undoable)
    {
        UNREFERENCED_PARAMETER(undoable);
        return E_NOTIMPL;
    }

    inline STDMETHODIMP CAXWindow::QueryInterface(REFIID riid, void** ppObject)
    {
        if (!ppObject)
            return E_POINTER;

        if (IsEqualIID(riid, IID_IOleClientSite))
            *ppObject = static_cast<IOleClientSite*>(this);
        else if (IsEqualIID(riid, IID_IOleInPlaceSite))
            *ppObject = static_cast<IOleInPlaceSite*>(this);
        else if (IsEqualIID(riid, IID_IOleInPlaceFrame))
            *ppObject = static_cast<IOleInPlaceFrame*>(this);
        else if (IsEqualIID(riid, IID_IOleInPlaceUIWindow))
            *ppObject = static_cast<IOleInPlaceUIWindow*>(this);
        else if (IsEqualIID(riid, IID_IOleControlSite))
            *ppObject = static_cast<IOleControlSite*>(this);
        else if (IsEqualIID(riid, IID_IOleWindow))
            *ppObject = this;
        else if (IsEqualIID(riid, IID_IDispatch))
            *ppObject = static_cast<IDispatch*>(this);
        else if (IsEqualIID(riid, IID_IUnknown))
            *ppObject = this;
        else
        {
            *ppObject = NULL;
            return E_NOINTERFACE;
        }

        AddRef();
        return S_OK;
    }

    inline STDMETHODIMP_(ULONG) CAXWindow::Release()
    {
        return --m_count;
    }

    inline void CAXWindow::Remove()
    {
        if (!m_pUnk)
            return;

        IOleObject* pObject;
        HRESULT result = m_pUnk->QueryInterface(IID_IOleObject, reinterpret_cast<void**>(&pObject));
        if (SUCCEEDED(result))
        {
            pObject->Close(OLECLOSE_NOSAVE);
            pObject->SetClientSite(NULL);
            pObject->Release();
        }

        IOleInPlaceObject* pipo;
        result = m_pUnk->QueryInterface(IID_IOleInPlaceObject, reinterpret_cast<void**>(&pipo));
        if (SUCCEEDED(result))
        {
            pipo->UIDeactivate();
            pipo->InPlaceDeactivate();
            pipo->Release();
        }

    }

    inline STDMETHODIMP CAXWindow::RemoveMenus(HMENU shared)
    {
        UNREFERENCED_PARAMETER(shared);
        return E_NOTIMPL;
    }

    inline STDMETHODIMP CAXWindow::RequestBorderSpace(LPCBORDERWIDTHS pBorderWidths)
    {
        UNREFERENCED_PARAMETER(pBorderWidths);
        return E_NOTIMPL;
    }

    inline STDMETHODIMP CAXWindow::RequestNewObjectLayout()
    {
        return E_NOTIMPL;
    }

    inline STDMETHODIMP CAXWindow::SaveObject()
    {
        return E_NOTIMPL;
    }

    inline STDMETHODIMP CAXWindow::Scroll(SIZE scrollExtent)
    {
        UNREFERENCED_PARAMETER(scrollExtent);
        return E_NOTIMPL;
    }

    inline STDMETHODIMP CAXWindow::SetActiveObject(IOleInPlaceActiveObject* pActiveObject, LPCOLESTR pObjName)
    {
        UNREFERENCED_PARAMETER(pActiveObject);
        UNREFERENCED_PARAMETER(pObjName);
        return E_NOTIMPL;
    }

    inline STDMETHODIMP CAXWindow::SetBorderSpace(LPCBORDERWIDTHS pBorderWidths)
    {
        UNREFERENCED_PARAMETER(pBorderWidths);
        return E_NOTIMPL;
    }

    inline void CAXWindow::SetLocation(int x, int y, int width, int height)
    {
        m_controlRect.SetRect(x, y, x + width, y + height);

        if (!m_pUnk)
            return;

        IOleInPlaceObject* pipo;
        HRESULT result = m_pUnk->QueryInterface(IID_IOleInPlaceObject, reinterpret_cast<void**>(&pipo));
        if (FAILED(result))
            return;

        pipo->SetObjectRects(&m_controlRect, &m_controlRect);
        pipo->Release();
    }

    inline STDMETHODIMP CAXWindow::SetMenu(HMENU shared, HOLEMENU holemenu, HWND activeObject)
    {
        UNREFERENCED_PARAMETER(shared);
        UNREFERENCED_PARAMETER(holemenu);
        UNREFERENCED_PARAMETER(activeObject);
        return E_NOTIMPL;
    }

    inline void CAXWindow::SetParent(HWND hWndParent)
    {
        m_ax = hWndParent;
    }

    inline STDMETHODIMP CAXWindow::SetStatusText(LPCOLESTR pStatusText)
    {
        if (NULL == pStatusText)
            return E_POINTER;

#ifndef UNICODE
        char status[MAX_PATH];
        // Convert the Wide string to char
        WideCharToMultiByte(CP_ACP, 0, pStatusText, -1, status, MAX_PATH, NULL, NULL);

        if (IsWindow(m_status))
            SendMessage(m_status, SB_SETTEXT, 0, (LPARAM)status);
#else
        if (IsWindow(m_status))
            SendMessage(m_status, SB_SETTEXT, 0, (LPARAM)pStatusText);
#endif

        return S_OK;
    }

    inline void CAXWindow::SetStatusWindow(HWND status)
    {
        m_status = status;
    }

    inline void CAXWindow::SetVisible(BOOL isVisible)
    {
        if (!m_pUnk)
            return;

        IOleObject* pObject;
        HRESULT result = m_pUnk->QueryInterface(IID_IOleObject, reinterpret_cast<void**>(&pObject));
        if (FAILED(result))
            return;

        if (isVisible)
        {
            pObject->DoVerb(OLEIVERB_INPLACEACTIVATE, NULL, this, 0, m_ax, &m_controlRect);
            pObject->DoVerb(OLEIVERB_SHOW, NULL, this, 0, m_ax, &m_controlRect);
        }
        else
            pObject->DoVerb(OLEIVERB_HIDE, NULL, this, 0, m_ax, NULL);

        pObject->Release();
    }

    inline STDMETHODIMP CAXWindow::ShowObject()
    {
        return S_OK;
    }

    inline STDMETHODIMP CAXWindow::ShowPropertyFrame()
    {
        return E_NOTIMPL;
    }

    inline STDMETHODIMP CAXWindow::TransformCoords(POINTL* pHimetric, POINTF* pContainer, DWORD flags)
    {
        UNREFERENCED_PARAMETER(pHimetric);
        UNREFERENCED_PARAMETER(pContainer);
        UNREFERENCED_PARAMETER(flags);
        return E_NOTIMPL;
    }

    inline STDMETHODIMP CAXWindow::TranslateAccelerator(LPMSG pMsg, WORD id)
    {
        UNREFERENCED_PARAMETER(pMsg);
        UNREFERENCED_PARAMETER(id);
        return S_OK;
    }

    inline STDMETHODIMP CAXWindow::TranslateAccelerator(LPMSG pMsg, DWORD modifiers)
    {
        UNREFERENCED_PARAMETER(pMsg);
        UNREFERENCED_PARAMETER(modifiers);
        return S_FALSE;
    }

    inline void CAXWindow::TranslateKey(MSG msg)
    {
        if (!m_pUnk)
            return;

        IOleInPlaceActiveObject* pao;
        HRESULT result = m_pUnk->QueryInterface(IID_IOleInPlaceActiveObject, reinterpret_cast<void**>(&pao));
        if (FAILED(result))
            return;

        pao->TranslateAccelerator(&msg);
        pao->Release();
    }


    ////////////////////////////////////////
    // Definitions for the CWebBrowser class
    //

    inline CWebBrowser::CWebBrowser() : m_pIWebBrowser2(0)
    {
        HRESULT hr = OleInitialize(NULL);
        if (FAILED(hr))
            throw CWinException(g_msgOleInitialize);
    }

    inline CWebBrowser::~CWebBrowser()
    {
        if (m_pIWebBrowser2)
        {
            m_pIWebBrowser2->Stop();
            m_pIWebBrowser2->Release();
        }

        OleUninitialize();
    }

    // Adds the IWebBrowser interface to the ActiveX container window.
    // Refer to IID_IWebBrowser2 in the Windows API documentation for more information.
    inline void CWebBrowser::AddWebBrowserControl()
    {
        GetAXWindow().CreateControl(CLSID_WebBrowser);
        GetAXWindow().SetParent(*this);
        GetAXWindow().SetVisible(TRUE);
        GetAXWindow().Activate(TRUE);

        IUnknown* pUnk = GetAXWindow().GetUnknown();
        if (pUnk)
        {
            // Store the pointer to the WebBrowser control
            HRESULT result = pUnk->QueryInterface(IID_IWebBrowser2, reinterpret_cast<void**>(&m_pIWebBrowser2));
            pUnk->Release();

            // Navigate to an empty page
            if (SUCCEEDED(result))
            {
                Navigate(_T("about:blank"));
            }
        }
    }

    // Called when the WebBrowser window is created and the HWND is attached this object.
    inline void CWebBrowser::OnAttach()
    {
        if (NULL == m_pIWebBrowser2)
            AddWebBrowserControl();
    }

    // Called when the web browser window is created.
    inline int CWebBrowser::OnCreate(CREATESTRUCT& cs)
    {
        UNREFERENCED_PARAMETER(cs);
        AddWebBrowserControl();
        return 0;
    }

    // Called when the window is destroyed.
    inline void CWebBrowser::OnDestroy()
    {
        GetAXWindow().Remove();
    }

    // Called when the window is resized.
    inline void CWebBrowser::OnSize(int width, int height)
    {
        // position the container
        GetAXWindow().SetLocation(0, 0, width, height);
    }

    // Provides default message processing for the web browser window.
    inline LRESULT CWebBrowser::WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        switch(msg)
        {
        case WM_SIZE:
            OnSize(LOWORD(lparam), HIWORD(lparam));
            break;
        }

        return CWnd::WndProcDefault(msg, wparam, lparam);
    }

    // Retrieves the automation object for the application that is hosting the WebBrowser Control.
    inline LPDISPATCH CWebBrowser::GetApplication() const
    {
        LPDISPATCH Value;
        GetIWebBrowser2()->get_Application(&Value);
        return Value;
    }

    // Retrieves a value that indicates whether the object is engaged in a navigation or downloading operation.
    inline BOOL CWebBrowser::GetBusy() const
    {
        VARIANT_BOOL Value = VARIANT_FALSE;
        GetIWebBrowser2()->get_Busy(&Value);
        return (Value != 0);
    }

    // Retrieves an object reference to a container.
    inline LPDISPATCH CWebBrowser::GetContainer() const
    {
        LPDISPATCH Value;
        GetIWebBrowser2()->get_Container(&Value);
        return Value;
    }

    // Retrieves an object reference to a document.
    inline LPDISPATCH CWebBrowser::GetDocument() const
    {
        LPDISPATCH Value;
        GetIWebBrowser2()->get_Document(&Value);
        return Value;
    }

    // Retrieves a value that indicates whether Internet Explorer is in full-screen mode or normal window mode.
    inline BOOL CWebBrowser::GetFullScreen() const
    {
        VARIANT_BOOL Value = VARIANT_FALSE;
        GetIWebBrowser2()->get_FullScreen(&Value);
        return (Value != 0);
    }

    // Retrieves the height of the object.
    inline long CWebBrowser::GetHeight() const
    {
        long lValue;
        GetIWebBrowser2()->get_Height(&lValue);
        return lValue;
    }

    // Retrieves the coordinate of the left edge of the object.
    inline long CWebBrowser::GetLeft() const
    {
        long lValue;
        GetIWebBrowser2()->get_Left(&lValue);
        return lValue;
    }

    // Retrieves the path or title of the resource that is currently displayed.
    inline CString CWebBrowser::GetLocationName() const
    {
        BSTR bstr;
        GetIWebBrowser2()->get_LocationName(&bstr);
        CString str(bstr);
        SysFreeString(bstr);
        return str;
    }

    // Retrieves the URL of the resource that is currently displayed.
    inline CString CWebBrowser::GetLocationURL() const
    {
        BSTR bstr;
        GetIWebBrowser2()->get_LocationURL(&bstr);
        CString str(bstr);
        SysFreeString(bstr);
        return str;
    }

    // Retrieves the accumulated  length of the ITEMIDLIST.
    inline UINT CWebBrowser::GetPidlLength(LPITEMIDLIST pidl)
    {
        assert(pidl);
        UINT cbPidl = sizeof(pidl->mkid.cb);
        while (pidl && pidl->mkid.cb)
        {
            cbPidl += pidl->mkid.cb;

            // Walk to next item
            BYTE* ptr = reinterpret_cast<BYTE*>(pidl);
            ptr += pidl->mkid.cb;
            pidl = reinterpret_cast<LPITEMIDLIST>(ptr);
        }

        return cbPidl;
    }

    // Retrieves a value that indicates whether the object is operating in offline mode.
    inline BOOL CWebBrowser::GetOffline() const
    {
        VARIANT_BOOL Value = VARIANT_FALSE;
        GetIWebBrowser2()->get_Offline(&Value);
        return (Value != 0);
    }

    // Retrieves the ready state of the object.
    inline READYSTATE CWebBrowser::GetReadyState() const
    {
        READYSTATE rsValue;
        GetIWebBrowser2()->get_ReadyState(&rsValue);
        return rsValue;
    }

    // Retrieves a value that indicates whether the object is registered as a top-level browser window.
    inline BOOL CWebBrowser::GetRegisterAsBrowser() const
    {
        VARIANT_BOOL Value = VARIANT_FALSE;
#if !defined(__BORLANDC__) || (__BORLANDC__ >= 0x600)
        GetIWebBrowser2()->get_RegisterAsBrowser(&Value);
#endif
        return (Value != 0);
    }

    // Retrieves the theater mode state of the object.
    inline BOOL CWebBrowser::GetTheaterMode() const
    {
        VARIANT_BOOL Value = VARIANT_FALSE;
        GetIWebBrowser2()->get_TheaterMode(&Value);
        return (Value != 0);
    }

    // Retrieves the coordinate of the top edge of the object.
    inline long CWebBrowser::GetTop() const
    {
        long lValue;
        GetIWebBrowser2()->get_Top(&lValue);
        return lValue;
    }

    //Returns TRUE  if the object is a top-level container.
    inline BOOL CWebBrowser::GetTopLevelContainer() const
    {
        VARIANT_BOOL Value = VARIANT_FALSE;
        GetIWebBrowser2()->get_TopLevelContainer(&Value);
        return (Value != 0);
    }

    inline LPDISPATCH CWebBrowser::GetParent() const
    {
        LPDISPATCH pDispatch = NULL;
        GetIWebBrowser2()->get_Parent(&pDispatch);
        return pDispatch;
    }

    // Retrieves the user type name of the contained document object.
    inline CString CWebBrowser::GetType() const
    {
        BSTR bString;
        GetIWebBrowser2()->get_Type(&bString);
        CString str(bString);
        SysFreeString(bString);
        return str;
    }

    // Retrieves a value that indicates whether the object is visible or hidden.
    inline BOOL CWebBrowser::GetVisible() const
    {
        VARIANT_BOOL Value = VARIANT_FALSE;
        GetIWebBrowser2()->get_Visible(&Value);
        return (Value != 0);
    }

    // Retrieves the width of the object.
    inline long CWebBrowser::GetWidth() const
    {
        long lValue;
        GetIWebBrowser2()->get_Width(&lValue);
        return lValue;
    }

    // Sets a value that indicates whether Internet Explorer is in full-screen mode or normal window mode.
    inline void CWebBrowser::SetFullScreen(BOOL isFullScreen)
    {
        VARIANT_BOOL vBool = isFullScreen ? VARIANT_TRUE : VARIANT_FALSE;
        GetIWebBrowser2()->put_FullScreen(vBool);
    }

    // Sets the height of the object.
    inline void CWebBrowser::SetHeight(long height)
    {
        GetIWebBrowser2()->put_Height(height);
    }

    // Sets the coordinate of the left edge of the object.
    inline void CWebBrowser::SetLeft(long leftEdge)
    {
        GetIWebBrowser2()->put_Left(leftEdge);
    }

    // Sets a value that indicates whether the object is operating in offline mode.
    inline void CWebBrowser::SetOffline(BOOL isOffline)
    {
        VARIANT_BOOL vBool = isOffline ? VARIANT_TRUE : VARIANT_FALSE;
        GetIWebBrowser2()->put_Offline(vBool);
    }

    // Sets a value that indicates whether the object is registered as a top-level browser window.
    inline void CWebBrowser::SetRegisterAsBrowser(BOOL isBrowser)
    {
        VARIANT_BOOL vBool = isBrowser ? VARIANT_TRUE : VARIANT_FALSE;
        GetIWebBrowser2()->put_RegisterAsBrowser(vBool);
    }

    // Sets the theatre mode state of the object.
    inline void CWebBrowser::SetTheaterMode(BOOL isTheaterMode)
    {
        VARIANT_BOOL vBool = isTheaterMode ? VARIANT_TRUE : VARIANT_FALSE;
        GetIWebBrowser2()->put_TheaterMode(vBool);
    }

    // Sets the coordinate of the top edge of the object.
    inline void CWebBrowser::SetTop(long topEdge)
    {
        GetIWebBrowser2()->put_Top(topEdge);
    }

    // Sets a value that indicates whether the object is visible or hidden.
    inline void CWebBrowser::SetVisible(BOOL isVisible)
    {
        VARIANT_BOOL vBool = isVisible ? VARIANT_TRUE : VARIANT_FALSE;
        GetIWebBrowser2()->put_Visible(vBool);
    }

    // Sets the width of the object.
    inline void CWebBrowser::SetWidth(long width)
    {
        GetIWebBrowser2()->put_Width(width);
    }

    // Executes a command using the IOleCommandTarget interface.
    inline void CWebBrowser::ExecWB(OLECMDID cmdID, OLECMDEXECOPT cmdExecOpt, VARIANT* in, VARIANT* out)
    {
        GetIWebBrowser2()->ExecWB(cmdID, cmdExecOpt, in, out);
    }

    // Gets the value associated with the specified property name.
    inline VARIANT CWebBrowser::GetProperty( LPCTSTR pProperty )
    {
        VARIANT v;
        GetIWebBrowser2()->GetProperty( TtoBSTR(pProperty), &v );
        return v;
    }

    // Navigates backward one item in the history list.
    inline void CWebBrowser::GoBack()
    {
        GetIWebBrowser2()->GoBack();
    }

    // Navigates forward one item in the history list.
    inline void CWebBrowser::GoForward()
    {
        GetIWebBrowser2()->GoForward();
    }

    // Navigates to the current home or start page.
    inline void CWebBrowser::GoHome()
    {
        GetIWebBrowser2()->GoHome();
    }

    // Navigates to the current search page.
    inline void CWebBrowser::GoSearch()
    {
        GetIWebBrowser2()->GoSearch();
    }

    // Navigates to a resource identified by a URL or to a file identified by a full path.
    inline void CWebBrowser::Navigate(LPCTSTR pURL,   DWORD flags /*= 0*/, LPCTSTR pTargetFrameName /*= NULL*/,
                    LPCTSTR pHeaders /*= NULL*/, LPVOID pPostData /*= NULL*/,   DWORD postDataLen /*= 0*/)
    {
        VARIANT Flags;
        Flags.vt = VT_I4;
        Flags.lVal = flags;

        VARIANT TargetFrameName;
        TargetFrameName.vt = VT_BSTR;
        TargetFrameName.bstrVal = SysAllocString(TtoW(pTargetFrameName));

        SAFEARRAY* psa = SafeArrayCreateVector(VT_UI1, 0, postDataLen);
        CopyMemory(psa->pvData, pPostData, postDataLen);
        VARIANT PostData;
        PostData.vt = VT_ARRAY|VT_UI1;
        PostData.parray = psa;

        VARIANT Headers;
        Headers.vt = VT_BSTR;
        Headers.bstrVal = SysAllocString(TtoW(pHeaders));
        BSTR url = SysAllocString(TtoW(pURL));

        if (url)
            GetIWebBrowser2()->Navigate(url, &Flags, &TargetFrameName, &PostData, &Headers);

        VariantClear(&Flags);
        VariantClear(&TargetFrameName);
        VariantClear(&PostData);
        VariantClear(&Headers);
    }

    // Navigates the browser to a location specified by a pointer to an item identifier list (PIDL) for an entity in the Microsoft Windows Shell namespace.
    inline void CWebBrowser::Navigate2(LPITEMIDLIST pIDL, DWORD flags /*= 0*/, LPCTSTR pTargetFrameName /*= NULL*/)
    {
        UINT cb = GetPidlLength(pIDL);
        LPSAFEARRAY psa = SafeArrayCreateVector(VT_UI1, 0, cb);
        VARIANT PIDL;
        PIDL.vt = VT_ARRAY|VT_UI1;
        PIDL.parray = psa;
        CopyMemory(psa->pvData, pIDL, cb);

        VARIANT Flags;
        Flags.vt = VT_I4;
        Flags.lVal = flags;

        VARIANT TargetFrameName;
        TargetFrameName.vt = VT_BSTR;
        TargetFrameName.bstrVal = SysAllocString(TtoW(pTargetFrameName));

        GetIWebBrowser2()->Navigate2(&PIDL, &Flags, &TargetFrameName, 0, 0);

        VariantClear(&PIDL);
        VariantClear(&Flags);
        VariantClear(&TargetFrameName);
    }

    // Navigates the browser to a location that is expressed as a URL.
    inline void CWebBrowser::Navigate2(LPCTSTR pURL, DWORD flags /*= 0*/, LPCTSTR pTargetFrameName /*= NULL*/,
                     LPCTSTR pHeaders /*= NULL*/,   LPVOID pPostData /*= NULL*/, DWORD postDataLen /*= 0*/)
    {
        VARIANT URL;
        URL.vt = VT_BSTR;
        URL.bstrVal = SysAllocString(TtoW(pURL));

        VARIANT Flags;
        Flags.vt = VT_I4;
        Flags.lVal = flags;

        VARIANT TargetFrameName;
        TargetFrameName.vt = VT_BSTR;
        TargetFrameName.bstrVal = SysAllocString(TtoW(pTargetFrameName));

        // Store the pidl in a SafeArray, and assign the SafeArray to a VARIANT
        SAFEARRAY* psa = SafeArrayCreateVector(VT_UI1, 0, postDataLen);
        CopyMemory(psa->pvData, pPostData, postDataLen);
        VARIANT PostData;
        PostData.vt = VT_ARRAY|VT_UI1;
        PostData.parray = psa;

        VARIANT Headers;
        Headers.vt = VT_BSTR;
        Headers.bstrVal = SysAllocString(TtoW(pHeaders));

        GetIWebBrowser2()->Navigate2(&URL, &Flags, &TargetFrameName, &PostData, &Headers);

        VariantClear(&URL);
        VariantClear(&Flags);
        VariantClear(&TargetFrameName);
        VariantClear(&PostData);
        VariantClear(&Headers);
    }

    // Sets the value of a property associated with the object.
    inline void CWebBrowser::PutProperty(LPCTSTR pProperty, const VARIANT& value)
    {
        GetIWebBrowser2()->PutProperty(TtoBSTR(pProperty), value);
    }

    // Sets the value of a property associated with the object.
    inline void CWebBrowser::PutProperty(LPCTSTR pPropertyName, double value)
    {
        VARIANT v;
        v.vt = VT_I4;
        v.dblVal = value;
        GetIWebBrowser2()->PutProperty(TtoBSTR(pPropertyName), v);
        VariantClear(&v);
    }

    // Sets the value of a property associated with the object.
    inline void CWebBrowser::PutProperty(LPCTSTR pPropertyName, long value)
    {
        VARIANT v;
        v.vt = VT_I4;
        v.lVal= value;
        GetIWebBrowser2()->PutProperty(TtoBSTR(pPropertyName), v);
        VariantClear(&v);
    }

    // Sets the value of a property associated with the object.
    inline void CWebBrowser::PutProperty(LPCTSTR pPropertyName, LPCTSTR pValue)
    {
        VARIANT v;
        v.vt = VT_BSTR;
        v.bstrVal= SysAllocString(TtoW(pValue));
        GetIWebBrowser2()->PutProperty(TtoBSTR(pPropertyName), v);
        VariantClear(&v);
    }

    // Sets the value of a property associated with the object.
    inline void CWebBrowser::PutProperty(LPCTSTR pPropertyName, short value)
    {
        VARIANT v;
        v.vt = VT_I4;
        v.iVal = value;
        GetIWebBrowser2()->PutProperty(TtoBSTR(pPropertyName), v);
        VariantClear(&v);
    }

    // Reloads the file that is currently displayed in the object.
    inline void CWebBrowser::Refresh()
    {
        GetIWebBrowser2()->Refresh();
    }

    // Reloads the file that is currently displayed with the specified refresh level.
    inline void CWebBrowser::Refresh2(int level)
    {
        VARIANT v;
        v.vt = VT_I4;
        v.intVal = level;
        GetIWebBrowser2()->Refresh2(&v);
        VariantClear(&v);
    }

    // Cancels a pending navigation or download, and stops dynamic page elements, such as background sounds and animations.
    inline void CWebBrowser::Stop()
    {
        GetIWebBrowser2()->Stop();
    }


}

#endif  // _WIN32XX_WEBBROWSER_H_

