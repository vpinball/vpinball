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

#ifndef _WIN32XX_WEBBROWSER_H_
#define _WIN32XX_WEBBROWSER_H_

#include "wxx_appcore0.h"
#include <exdisp.h>
#include <ocidl.h>

#ifdef _MSC_VER
  #pragma warning (disable : 4091)  // temporarily disable C4091 warning
#endif 
 
#include <shlobj.h>

#ifdef _MSC_VER
  #pragma warning (default : 4091)  // re-enable C4091 warning
#endif 


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
        virtual void Activate(BOOL fFocus);
        virtual void CreateControl(BSTR bstrClsid);
        virtual void CreateControl(REFCLSID clsid);
        virtual void Remove();
        virtual void SetParent(HWND hWndParent);
        virtual void SetLocation(int x, int y, int width, int height);
        virtual void SetVisible(BOOL IsVisible);
        virtual void SetStatusWindow(HWND hWndStatus);
        virtual void TranslateKey(MSG msg);
        IDispatch* GetDispatch();
        IUnknown* GetUnknown();

        // IUnknown Methods
        STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject);
        STDMETHODIMP_(ULONG) AddRef();
        STDMETHODIMP_(ULONG) Release();

        // IOleClientSite Methods
        STDMETHODIMP SaveObject();
        STDMETHODIMP GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, LPMONIKER* ppMk);
        STDMETHODIMP GetContainer(LPOLECONTAINER* ppContainer);
        STDMETHODIMP ShowObject();
        STDMETHODIMP OnShowWindow(BOOL Show);
        STDMETHODIMP RequestNewObjectLayout();

        // IOleWindow Methods
        STDMETHODIMP GetWindow(HWND* phwnd);
        STDMETHODIMP ContextSensitiveHelp(BOOL EnterMode);

        // IOleInPlaceSite Methods
        STDMETHODIMP CanInPlaceActivate();
        STDMETHODIMP OnInPlaceActivate();
        STDMETHODIMP OnUIActivate();
        STDMETHODIMP GetWindowContext(IOleInPlaceFrame** ppFrame, IOleInPlaceUIWindow** ppDoc, LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo);
        STDMETHODIMP Scroll(SIZE scrollExtent);
        STDMETHODIMP OnUIDeactivate(BOOL fUndoable);
        STDMETHODIMP OnInPlaceDeactivate();
        STDMETHODIMP DiscardUndoState();
        STDMETHODIMP DeactivateAndUndo();
        STDMETHODIMP OnPosRectChange(LPCRECT lprcPosRect);

        // IOleInPlaceUIWindow Methods
        STDMETHODIMP GetBorder(LPRECT lprectBorder);
        STDMETHODIMP RequestBorderSpace(LPCBORDERWIDTHS lpborderwidths);
        STDMETHODIMP SetBorderSpace(LPCBORDERWIDTHS lpborderwidths);
        STDMETHODIMP SetActiveObject(IOleInPlaceActiveObject* pActiveObject, LPCOLESTR lpszObjName);

        // IOleInPlaceFrame Methods
        STDMETHODIMP InsertMenus(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths);
        STDMETHODIMP SetMenu(HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject);
        STDMETHODIMP RemoveMenus(HMENU hmenuShared);
        STDMETHODIMP SetStatusText(LPCOLESTR pszStatusText);
        STDMETHODIMP EnableModeless(BOOL Enable);
        STDMETHODIMP TranslateAccelerator(LPMSG lpmsg, WORD wID);

        // IOleControlSite Methods
        STDMETHODIMP OnControlInfoChanged();
        STDMETHODIMP LockInPlaceActive(BOOL Lock);
        STDMETHODIMP GetExtendedControl(IDispatch** ppDisp);
        STDMETHODIMP TransformCoords(POINTL* pptlHimetric, POINTF* pptfContainer, DWORD dwFlags);
        STDMETHODIMP TranslateAccelerator(LPMSG pMsg, DWORD grfModifiers);
        STDMETHODIMP OnFocus(BOOL fGotFocus);
        STDMETHODIMP ShowPropertyFrame();

        // IDispatch Methods
        STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR** rgszNames, unsigned int cNames, LCID lcid, DISPID* rgdispid);
        STDMETHODIMP GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo** pptinfo);
        STDMETHODIMP GetTypeInfoCount(unsigned int* pctinfo);
        STDMETHODIMP Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexecinfo, unsigned int* puArgErr);

    private:
         ULONG       m_cRefs;       // ref count
         HWND        m_hWndAX;      // window handle of the container
         HWND        m_hWndStatus;  // status window handle
         IUnknown*   m_pUnk;        // IUnknown of contained object
         CRect       m_rcControl;   // size of control
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
        CAXWindow& GetAXWindow() const { return m_AXContainer; }
        BOOL GetBusy() const;
        LPDISPATCH GetContainer() const;
        BOOL GetFullScreen() const;
        long GetHeight() const;
        IWebBrowser2* GetIWebBrowser2() const { return m_pIWebBrowser2; }
        long GetLeft() const;
        CString GetLocationName() const;
        CString GetLocationURL() const;
        BOOL GetOffline() const;
        READYSTATE GetReadyState() const;
        BOOL GetRegisterAsBrowser() const;
        BOOL GetTheaterMode() const;
        long GetTop() const;
        BOOL GetTopLevelContainer() const;
        CString GetType() const;
        BOOL GetVisible() const;
        long GetWidth() const;
        void SetFullScreen(BOOL IsFullScreen);
        void SetHeight(long Height);
        void SetLeft(long LeftEdge);
        void SetOffline(BOOL IsOffline);
        void SetRegisterAsBrowser(BOOL IsBrowser);
        void SetTheaterMode(BOOL IsTheaterMode);
        void SetTop(long TopEdge);
        void SetVisible(BOOL IsVisible);
        void SetWidth(long Width);

        // Operations
        void AddWebBrowserControl();
        void ExecWB(OLECMDID cmdID, OLECMDEXECOPT cmdexecopt, VARIANT* pvaIn, VARIANT* pvaOut);
        VARIANT GetProperty( LPCTSTR pszProperty);
        void GoBack();
        void GoForward();
        void GoHome();
        void GoSearch();
        void Navigate(LPCTSTR pszURL, DWORD dwFlags = 0, LPCTSTR pszTargetFrameName = NULL,
                        LPCTSTR pszHeaders = NULL, LPVOID pvPostData = NULL, DWORD dwPostDataLen = 0);
        void Navigate2(LPITEMIDLIST pIDL, DWORD dwFlags = 0, LPCTSTR pszTargetFrameName = NULL);
        void Navigate2(LPCTSTR pszURL, DWORD dwFlags = 0, LPCTSTR pszTargetFrameName = NULL,
                         LPCTSTR pszHeaders = NULL, LPVOID lpvPostData = NULL, DWORD dwPostDataLen = 0);
        void PutProperty(LPCTSTR pszPropertyName, const VARIANT& vtValue);
        void PutProperty(LPCTSTR pszPropertyName, double dValue);
        void PutProperty(LPCTSTR pszPropertyName, long lValue);
        void PutProperty(LPCTSTR pszPropertyName, LPCTSTR lpszValue);
        void PutProperty(LPCTSTR pszPropertyName, short nValue);
        void Refresh();
        void Refresh2(int nLevel);
        void Stop();

    protected:
        // Override these as required
        virtual void OnAttach();
        virtual int  OnCreate(CREATESTRUCT& cs);
        virtual void OnDestroy();
        virtual void OnSize(int width, int height);

        // Not intended to be overridden
        LRESULT WndProcDefault(UINT uMsg, WPARAM wParam, LPARAM lParam);

    private:
        UINT    GetPidlLength(LPITEMIDLIST pidl);
        mutable CAXWindow     m_AXContainer;    // The ActiveX Container
        IWebBrowser2* m_pIWebBrowser2;  // Interface to the ActiveX web browser control
    };

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

namespace Win32xx
{

    /////////////////////////////////////////
    // Definitions for the CAXWindow class
    //

    inline CAXWindow::CAXWindow() : m_cRefs(1), m_hWndAX(NULL), m_hWndStatus(0), m_pUnk(NULL)
    {
    }


    inline CAXWindow::~CAXWindow()
    {
        if (m_pUnk)
            m_pUnk->Release();

        Release();
    }


    inline void CAXWindow::CreateControl(BSTR bstrClsid)
    {
        CLSID   clsid;
        if (NOERROR == CLSIDFromString(bstrClsid, &clsid))
            CreateControl(clsid);
    }


    inline void CAXWindow::Activate(BOOL fFocus)
    {
        if (!m_pUnk)
            return;

        if (fFocus)
        {
            IOleObject* pioo;
            HRESULT hr = m_pUnk->QueryInterface(IID_IOleObject, (void**)&pioo);
            if (FAILED(hr))
                return;

            pioo->DoVerb(OLEIVERB_UIACTIVATE, NULL, this, 0, m_hWndAX, &m_rcControl);
            pioo->Release();
        }
    }


    inline void CAXWindow::CreateControl(REFCLSID clsid)
    {
        if (S_OK != CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER, IID_IUnknown, (void**)&m_pUnk))
            return;

        assert(m_pUnk);

        IOleObject* pioo;
        HRESULT hr = m_pUnk->QueryInterface(IID_IOleObject, (void**)&pioo);
        if (FAILED(hr))
            return;

        pioo->SetClientSite(this);
        pioo->Release();

        IPersistStreamInit* ppsi;
        hr = m_pUnk->QueryInterface(IID_IPersistStreamInit, (void**)&ppsi);
        if (SUCCEEDED(hr))
        {
            ppsi->InitNew();
            ppsi->Release();
        }
    }


    inline STDMETHODIMP_(ULONG) CAXWindow::AddRef()
    {
        return ++m_cRefs;
    }


    inline STDMETHODIMP CAXWindow::CanInPlaceActivate()
    {
        return S_OK;
    }


    inline STDMETHODIMP CAXWindow::ContextSensitiveHelp(BOOL EnterMode)
    {
        UNREFERENCED_PARAMETER(EnterMode);
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


    inline STDMETHODIMP CAXWindow::EnableModeless(BOOL Enable)
    {
        UNREFERENCED_PARAMETER(Enable);
        return E_NOTIMPL;
    }


    inline STDMETHODIMP CAXWindow::GetBorder(LPRECT lprectBorder)
    {
        UNREFERENCED_PARAMETER(lprectBorder);
        return E_NOTIMPL;
    }


    inline STDMETHODIMP CAXWindow::GetContainer(LPOLECONTAINER* ppContainer)
    {
        UNREFERENCED_PARAMETER(ppContainer);
        return E_NOINTERFACE;
    }


    inline IDispatch* CAXWindow::GetDispatch()
    {
        if (!m_pUnk)
            return NULL;

        IDispatch*  pdisp;

        m_pUnk->QueryInterface(IID_IDispatch, (void**)&pdisp);
        return pdisp;
    }


    inline STDMETHODIMP CAXWindow::GetExtendedControl(IDispatch** ppDisp)
    {
        if (ppDisp == NULL)
            return E_INVALIDARG;

        *ppDisp = (IDispatch*)this;
        (*ppDisp)->AddRef();

        return S_OK;
    }


    inline STDMETHODIMP CAXWindow::GetIDsOfNames(REFIID riid, OLECHAR** rgszNames, unsigned int cNames, LCID lcid, DISPID* rgdispid)
    {
        UNREFERENCED_PARAMETER((IID)riid);      // IID cast required for the MinGW compiler
        UNREFERENCED_PARAMETER(rgszNames);
        UNREFERENCED_PARAMETER(cNames);
        UNREFERENCED_PARAMETER(lcid);

        *rgdispid = DISPID_UNKNOWN;
        return DISP_E_UNKNOWNNAME;
    }


    inline STDMETHODIMP CAXWindow::GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, LPMONIKER* ppMk)
    {
        UNREFERENCED_PARAMETER(dwAssign);
        UNREFERENCED_PARAMETER(dwWhichMoniker);
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


    inline STDMETHODIMP CAXWindow::GetWindow(HWND* lphwnd)
    {
        if (!IsWindow(m_hWndAX))
            return S_FALSE;

        *lphwnd = m_hWndAX;
        return S_OK;
    }


    inline STDMETHODIMP CAXWindow::GetWindowContext (IOleInPlaceFrame** ppFrame, IOleInPlaceUIWindow** ppIIPUIWin,
                                      LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo)
    {
        *ppFrame = (IOleInPlaceFrame*)this;
        *ppIIPUIWin = NULL;

        RECT rect;
        GetClientRect(m_hWndAX, &rect);
        lprcPosRect->left       = 0;
        lprcPosRect->top        = 0;
        lprcPosRect->right      = rect.right;
        lprcPosRect->bottom     = rect.bottom;

        CopyRect(lprcClipRect, lprcPosRect);

        lpFrameInfo->cb             = sizeof(OLEINPLACEFRAMEINFO);
        lpFrameInfo->fMDIApp        = FALSE;
        lpFrameInfo->hwndFrame      = m_hWndAX;
        lpFrameInfo->haccel         = 0;
        lpFrameInfo->cAccelEntries  = 0;

        (*ppFrame)->AddRef();
        return S_OK;
    }


    inline STDMETHODIMP CAXWindow::InsertMenus(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths)
    {
        UNREFERENCED_PARAMETER(hmenuShared);
        UNREFERENCED_PARAMETER(lpMenuWidths);
        return E_NOTIMPL;
    }


    inline STDMETHODIMP CAXWindow::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexecinfo, unsigned int* puArgErr)
    {
        UNREFERENCED_PARAMETER(dispid);
        UNREFERENCED_PARAMETER((IID)riid);      // IID cast required for the MinGW compiler
        UNREFERENCED_PARAMETER(lcid);
        UNREFERENCED_PARAMETER(wFlags);
        UNREFERENCED_PARAMETER(pdispparams);
        UNREFERENCED_PARAMETER(pvarResult);
        UNREFERENCED_PARAMETER(pexecinfo);
        UNREFERENCED_PARAMETER(puArgErr);
        return DISP_E_MEMBERNOTFOUND;
    }


    inline STDMETHODIMP CAXWindow::LockInPlaceActive(BOOL Lock)
    {
        UNREFERENCED_PARAMETER(Lock);
        return E_NOTIMPL;
    }


    inline STDMETHODIMP CAXWindow::OnControlInfoChanged()
    {
        return E_NOTIMPL;
    }


    inline STDMETHODIMP CAXWindow::OnFocus(BOOL fGotFocus)
    {
        UNREFERENCED_PARAMETER(fGotFocus);
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


    inline STDMETHODIMP CAXWindow::OnPosRectChange(LPCRECT lprcPosRect)
    {
        UNREFERENCED_PARAMETER(lprcPosRect);
        return S_OK;
    }


    inline STDMETHODIMP CAXWindow::OnShowWindow(BOOL Show)
    {
        UNREFERENCED_PARAMETER(Show);
        return S_OK;
    }


    inline STDMETHODIMP CAXWindow::OnUIActivate()
    {
        return S_OK;
    }


    inline STDMETHODIMP CAXWindow::OnUIDeactivate(BOOL fUndoable)
    {
        UNREFERENCED_PARAMETER(fUndoable);
        return E_NOTIMPL;
    }


    inline STDMETHODIMP CAXWindow::QueryInterface(REFIID riid, void** ppvObject)
    {
        if (!ppvObject)
            return E_POINTER;

        if (IsEqualIID(riid, IID_IOleClientSite))
            *ppvObject = (IOleClientSite*)this;
        else if (IsEqualIID(riid, IID_IOleInPlaceSite))
            *ppvObject = (IOleInPlaceSite*)this;
        else if (IsEqualIID(riid, IID_IOleInPlaceFrame))
            *ppvObject = (IOleInPlaceFrame*)this;
        else if (IsEqualIID(riid, IID_IOleInPlaceUIWindow))
            *ppvObject = (IOleInPlaceUIWindow*)this;
        else if (IsEqualIID(riid, IID_IOleControlSite))
            *ppvObject = (IOleControlSite*)this;
        else if (IsEqualIID(riid, IID_IOleWindow))
            *ppvObject = this;
        else if (IsEqualIID(riid, IID_IDispatch))
            *ppvObject = (IDispatch*)this;
        else if (IsEqualIID(riid, IID_IUnknown))
            *ppvObject = this;
        else
        {
            *ppvObject = NULL;
            return E_NOINTERFACE;
        }

        AddRef();
        return S_OK;
    }


    inline STDMETHODIMP_(ULONG) CAXWindow::Release()
    {
        return --m_cRefs;
    }


    inline void CAXWindow::Remove()
    {
        if (!m_pUnk)
            return;

        IOleObject* pioo;
        HRESULT hr = m_pUnk->QueryInterface(IID_IOleObject, (void**)&pioo);
        if (SUCCEEDED(hr))
        {
            pioo->Close(OLECLOSE_NOSAVE);
            pioo->SetClientSite(NULL);
            pioo->Release();
        }

        IOleInPlaceObject* pipo;
        hr = m_pUnk->QueryInterface(IID_IOleInPlaceObject, (void**)&pipo);
        if (SUCCEEDED(hr))
        {
            pipo->UIDeactivate();
            pipo->InPlaceDeactivate();
            pipo->Release();
        }

    }


    inline STDMETHODIMP CAXWindow::RemoveMenus(HMENU hmenuShared)
    {
        UNREFERENCED_PARAMETER(hmenuShared);
        return E_NOTIMPL;
    }


    inline STDMETHODIMP CAXWindow::RequestBorderSpace(LPCBORDERWIDTHS lpborderwidths)
    {
        UNREFERENCED_PARAMETER(lpborderwidths);
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


    inline STDMETHODIMP CAXWindow::SetActiveObject(IOleInPlaceActiveObject* pActiveObject, LPCOLESTR lpszObjName)
    {
        UNREFERENCED_PARAMETER(pActiveObject);
        UNREFERENCED_PARAMETER(lpszObjName);
        return E_NOTIMPL;
    }


    inline STDMETHODIMP CAXWindow::SetBorderSpace(LPCBORDERWIDTHS lpborderwidths)
    {
        UNREFERENCED_PARAMETER(lpborderwidths);
        return E_NOTIMPL;
    }


    inline void CAXWindow::SetLocation(int x, int y, int width, int height)
    {
        m_rcControl.SetRect(x, y, x + width, y + height);

        if (!m_pUnk)
            return;

        IOleInPlaceObject* pipo;
        HRESULT hr = m_pUnk->QueryInterface(IID_IOleInPlaceObject, (void**)&pipo);
        if (FAILED(hr))
            return;

        pipo->SetObjectRects(&m_rcControl, &m_rcControl);
        pipo->Release();
    }


    inline STDMETHODIMP CAXWindow::SetMenu(HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject)
    {
        UNREFERENCED_PARAMETER(hmenuShared);
        UNREFERENCED_PARAMETER(holemenu);
        UNREFERENCED_PARAMETER(hwndActiveObject);
        return E_NOTIMPL;
    }


    inline void CAXWindow::SetParent(HWND hWndParent)
    {
        m_hWndAX = hWndParent;
    }


    inline STDMETHODIMP CAXWindow::SetStatusText(LPCOLESTR pszStatusText)
    {
        if (NULL == pszStatusText)
            return E_POINTER;

    #ifndef UNICODE
        char status[MAX_PATH];
        // Convert the Wide string to char
        WideCharToMultiByte(CP_ACP, 0, pszStatusText, -1, status, MAX_PATH, NULL, NULL);

        if (IsWindow(m_hWndStatus))
            SendMessage(m_hWndStatus, SB_SETTEXT, 0, reinterpret_cast<LPARAM>(status));
    #else
        if (IsWindow(m_hWndStatus))
            SendMessage(m_hWndStatus, SB_SETTEXT, 0, reinterpret_cast<LPARAM>(pszStatusText));
    #endif

        return S_OK;
    }


    inline void CAXWindow::SetStatusWindow(HWND hWndStatus)
    {
        m_hWndStatus = hWndStatus;
    }


    inline void CAXWindow::SetVisible(BOOL IsVisible)
    {
        if (!m_pUnk)
            return;

        IOleObject* pioo;
        HRESULT hr = m_pUnk->QueryInterface(IID_IOleObject, (void**)&pioo);
        if (FAILED(hr))
            return;

        if (IsVisible)
        {
            pioo->DoVerb(OLEIVERB_INPLACEACTIVATE, NULL, this, 0, m_hWndAX, &m_rcControl);
            pioo->DoVerb(OLEIVERB_SHOW, NULL, this, 0, m_hWndAX, &m_rcControl);
        }
        else
            pioo->DoVerb(OLEIVERB_HIDE, NULL, this, 0, m_hWndAX, NULL);

        pioo->Release();
    }


    inline STDMETHODIMP CAXWindow::ShowObject()
    {
        return S_OK;
    }


    inline STDMETHODIMP CAXWindow::ShowPropertyFrame()
    {
        return E_NOTIMPL;
    }


    inline STDMETHODIMP CAXWindow::TransformCoords(POINTL* pptlHimetric, POINTF* pptfContainer, DWORD dwFlags)
    {
        UNREFERENCED_PARAMETER(pptlHimetric);
        UNREFERENCED_PARAMETER(pptfContainer);
        UNREFERENCED_PARAMETER(dwFlags);
        return E_NOTIMPL;
    }


    inline STDMETHODIMP CAXWindow::TranslateAccelerator(LPMSG lpmsg, WORD wID)
    {
        UNREFERENCED_PARAMETER(lpmsg);
        UNREFERENCED_PARAMETER(wID);
        return S_OK;
    }


    inline STDMETHODIMP CAXWindow::TranslateAccelerator(LPMSG pMsg, DWORD grfModifiers)
    {
        UNREFERENCED_PARAMETER(pMsg);
        UNREFERENCED_PARAMETER(grfModifiers);
        return S_FALSE;
    }


    inline void CAXWindow::TranslateKey(MSG msg)
    {
        if (!m_pUnk)
            return;

        IOleInPlaceActiveObject* pao;
        HRESULT hr = m_pUnk->QueryInterface(IID_IOleInPlaceActiveObject, (void**)&pao);
        if (FAILED(hr))
            return;

        pao->TranslateAccelerator(&msg);
        pao->Release();
    }


    ////////////////////////////////////////
    // Definitions for the CWebBrowser class
    //

    inline CWebBrowser::CWebBrowser() : m_pIWebBrowser2(0)
    {
        OleInitialize(NULL);
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
    inline void CWebBrowser::AddWebBrowserControl()
    {
        GetAXWindow().CreateControl(CLSID_WebBrowser);
        GetAXWindow().SetParent(*this);
        GetAXWindow().SetVisible(TRUE);
        GetAXWindow().Activate(TRUE);

        IUnknown* pUnk = GetAXWindow().GetUnknown();
        if(pUnk)
        {
            // Store the pointer to the WebBrowser control
            HRESULT hr = pUnk->QueryInterface(IID_IWebBrowser2, (void**)&m_pIWebBrowser2);
            pUnk->Release();

            // Navigate to an empty page
            if (SUCCEEDED(hr))
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
    inline LRESULT CWebBrowser::WndProcDefault(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch(uMsg)
        {
        case WM_SIZE:
            OnSize(LOWORD(lParam), HIWORD(lParam));
            break;
        }

        return CWnd::WndProcDefault(uMsg, wParam, lParam);
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
        return static_cast<BOOL>(Value);
    }


    // Retrieves an object reference to a container.
    inline LPDISPATCH CWebBrowser::GetContainer() const
    {
        LPDISPATCH Value;
        GetIWebBrowser2()->get_Container(&Value);
        return Value;
    }


    // Retrieves a value that indicates whether Internet Explorer is in full-screen mode or normal window mode.
    inline BOOL CWebBrowser::GetFullScreen() const
    {
        VARIANT_BOOL Value = VARIANT_FALSE;
        GetIWebBrowser2()->get_FullScreen(&Value);
        return static_cast<BOOL>(Value);
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
        while(pidl && pidl->mkid.cb)
        {
            cbPidl += pidl->mkid.cb;

            // Walk to next item
            BYTE* ptr = (BYTE*)pidl;
            ptr += pidl->mkid.cb;
            pidl = (LPITEMIDLIST)ptr;
        }

        return cbPidl;
    }


    // Retrieves a value that indicates whether the object is operating in offline mode.
    inline BOOL CWebBrowser::GetOffline() const
    {
        VARIANT_BOOL Value = VARIANT_FALSE;
        GetIWebBrowser2()->get_Offline(&Value);
        return static_cast<BOOL>(Value);
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
#if defined(__BORLANDC__) && (__BORLANDC__ < 0x600)
        GetIWebBrowser2()->get_RegisterAsBrowser(&Value);
#endif
        return static_cast<BOOL>(Value);
    }


    // Retrieves the theater mode state of the object.
    inline BOOL CWebBrowser::GetTheaterMode() const
    {
        VARIANT_BOOL Value = VARIANT_FALSE;
        GetIWebBrowser2()->get_TheaterMode(&Value);
        return static_cast<BOOL>(Value);
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
        return static_cast<BOOL>(Value);
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
        return static_cast<BOOL>(Value);
    }


    // Retrieves the width of the object.
    inline long CWebBrowser::GetWidth() const
    {
        long lValue;
        GetIWebBrowser2()->get_Width(&lValue);
        return lValue;
    }


    // Sets a value that indicates whether Internet Explorer is in full-screen mode or normal window mode.
    inline void CWebBrowser::SetFullScreen(BOOL IsFullScreen)
    {
        VARIANT_BOOL vBool = IsFullScreen ? VARIANT_TRUE : VARIANT_FALSE;
        GetIWebBrowser2()->put_FullScreen(vBool);
    }


    // Sets the height of the object.
    inline void CWebBrowser::SetHeight(long Height)
    {
        GetIWebBrowser2()->put_Height(Height);
    }


    // Sets the coordinate of the left edge of the object.
    inline void CWebBrowser::SetLeft(long LeftEdge)
    {
        GetIWebBrowser2()->put_Left(LeftEdge);
    }


    // Sets a value that indicates whether the object is operating in offline mode.
    inline void CWebBrowser::SetOffline(BOOL IsOffline)
    {
        VARIANT_BOOL vBool = IsOffline ? VARIANT_TRUE : VARIANT_FALSE;
        GetIWebBrowser2()->put_Offline(vBool);
    }


    // Sets a value that indicates whether the object is registered as a top-level browser window.
    inline void CWebBrowser::SetRegisterAsBrowser(BOOL IsBrowser)
    {
        VARIANT_BOOL vBool = IsBrowser ? VARIANT_TRUE : VARIANT_FALSE;
        GetIWebBrowser2()->put_RegisterAsBrowser(vBool);
    }


    // Sets the theatre mode state of the object.
    inline void CWebBrowser::SetTheaterMode(BOOL IsTheaterMode)
    {
        VARIANT_BOOL vBool = IsTheaterMode ? VARIANT_TRUE : VARIANT_FALSE;
        GetIWebBrowser2()->put_TheaterMode(vBool);
    }


    // Sets the coordinate of the top edge of the object.
    inline void CWebBrowser::SetTop(long TopEdge)

    {
        GetIWebBrowser2()->put_Top(TopEdge);
    }


    // Sets a value that indicates whether the object is visible or hidden.
    inline void CWebBrowser::SetVisible(BOOL IsVisible)
    {
        VARIANT_BOOL vBool = IsVisible ? VARIANT_TRUE : VARIANT_FALSE;
        GetIWebBrowser2()->put_Visible(vBool);
    }


    // Sets the width of the object.
    inline void CWebBrowser::SetWidth(long Width)

    {
        GetIWebBrowser2()->put_Width(Width);
    }


    // Executes a command using the IOleCommandTarget interface.
    inline void CWebBrowser::ExecWB(OLECMDID cmdID, OLECMDEXECOPT cmdexecopt, VARIANT* pvaIn, VARIANT* pvaOut)
    {
        GetIWebBrowser2()->ExecWB(cmdID, cmdexecopt, pvaIn, pvaOut);
    }


    // Gets the value associated with the specified property name.
    inline VARIANT CWebBrowser::GetProperty( LPCTSTR pszProperty )
    {
        VARIANT v;
        GetIWebBrowser2()->GetProperty( TtoBSTR(pszProperty), &v );
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
    inline void CWebBrowser::Navigate(LPCTSTR pszURL,   DWORD dwFlags /*= 0*/, LPCTSTR pszTargetFrameName /*= NULL*/,
                    LPCTSTR pszHeaders /*= NULL*/, LPVOID pvPostData /*= NULL*/,   DWORD dwPostDataLen /*= 0*/)
    {
        VARIANT Flags;
        Flags.vt = VT_I4;
        Flags.lVal = dwFlags;

        VARIANT TargetFrameName;
        TargetFrameName.vt = VT_BSTR;
        TargetFrameName.bstrVal = SysAllocString(TtoW(pszTargetFrameName));

        SAFEARRAY* psa = SafeArrayCreateVector(VT_UI1, 0, dwPostDataLen);
        CopyMemory(psa->pvData, pvPostData, dwPostDataLen);
        VARIANT PostData;
        PostData.vt = VT_ARRAY|VT_UI1;
        PostData.parray = psa;

        VARIANT Headers;
        Headers.vt = VT_BSTR;
        Headers.bstrVal = SysAllocString(TtoW(pszHeaders));
        BSTR url = SysAllocString(TtoW(pszURL));

        if (url)
            GetIWebBrowser2()->Navigate(url, &Flags, &TargetFrameName, &PostData, &Headers);

        VariantClear(&Flags);
        VariantClear(&TargetFrameName);
        VariantClear(&PostData);
        VariantClear(&Headers);
    }


    // Navigates the browser to a location specified by a pointer to an item identifier list (PIDL) for an entity in the Microsoft Windows Shell namespace.
    inline void CWebBrowser::Navigate2(LPITEMIDLIST pIDL, DWORD dwFlags /*= 0*/, LPCTSTR pszTargetFrameName /*= NULL*/)
    {
        UINT cb = GetPidlLength(pIDL);
        LPSAFEARRAY psa = SafeArrayCreateVector(VT_UI1, 0, cb);
        VARIANT PIDL;
        PIDL.vt = VT_ARRAY|VT_UI1;
        PIDL.parray = psa;
        CopyMemory(psa->pvData, pIDL, cb);

        VARIANT Flags;
        Flags.vt = VT_I4;
        Flags.lVal = dwFlags;

        VARIANT TargetFrameName;
        TargetFrameName.vt = VT_BSTR;
        TargetFrameName.bstrVal = SysAllocString(TtoW(pszTargetFrameName));

        GetIWebBrowser2()->Navigate2(&PIDL, &Flags, &TargetFrameName, 0, 0);

        VariantClear(&PIDL);
        VariantClear(&Flags);
        VariantClear(&TargetFrameName);
    }


    // Navigates the browser to a location that is expressed as a URL.
    inline void CWebBrowser::Navigate2(LPCTSTR pszURL, DWORD dwFlags /*= 0*/, LPCTSTR pszTargetFrameName /*= NULL*/,
                     LPCTSTR pszHeaders /*= NULL*/,   LPVOID pvPostData /*= NULL*/, DWORD dwPostDataLen /*= 0*/)
    {
        VARIANT URL;
        URL.vt = VT_BSTR;
        URL.bstrVal = SysAllocString(TtoW(pszURL));

        VARIANT Flags;
        Flags.vt = VT_I4;
        Flags.lVal = dwFlags;

        VARIANT TargetFrameName;
        TargetFrameName.vt = VT_BSTR;
        TargetFrameName.bstrVal = SysAllocString(TtoW(pszTargetFrameName));

        // Store the pidl in a SafeArray, and assign the SafeArray to a VARIANT
        SAFEARRAY* psa = SafeArrayCreateVector(VT_UI1, 0, dwPostDataLen);
        CopyMemory(psa->pvData, pvPostData, dwPostDataLen);
        VARIANT PostData;
        PostData.vt = VT_ARRAY|VT_UI1;
        PostData.parray = psa;

        VARIANT Headers;
        Headers.vt = VT_BSTR;
        Headers.bstrVal = SysAllocString(TtoW(pszHeaders));

        GetIWebBrowser2()->Navigate2(&URL, &Flags, &TargetFrameName, &PostData, &Headers);

        VariantClear(&URL);
        VariantClear(&Flags);
        VariantClear(&TargetFrameName);
        VariantClear(&PostData);
        VariantClear(&Headers);
    }


    // Sets the value of a property associated with the object.
    inline void CWebBrowser::PutProperty(LPCTSTR pszProperty, const VARIANT& vtValue)
    {
        GetIWebBrowser2()->PutProperty(TtoBSTR(pszProperty), vtValue);
    }


    // Sets the value of a property associated with the object.
    inline void CWebBrowser::PutProperty(LPCTSTR pszPropertyName, double dValue)
    {
        VARIANT v;
        v.vt = VT_I4;
        v.dblVal = dValue;
        GetIWebBrowser2()->PutProperty(TtoBSTR(pszPropertyName), v);
        VariantClear(&v);
    }


    // Sets the value of a property associated with the object.
    inline void CWebBrowser::PutProperty(LPCTSTR pszPropertyName, long lValue)
    {
        VARIANT v;
        v.vt = VT_I4;
        v.lVal= lValue;
        GetIWebBrowser2()->PutProperty(TtoBSTR(pszPropertyName), v);
        VariantClear(&v);
    }


    // Sets the value of a property associated with the object.
    inline void CWebBrowser::PutProperty(LPCTSTR pszPropertyName, LPCTSTR lpszValue)
    {
        VARIANT v;
        v.vt = VT_BSTR;
        v.bstrVal= SysAllocString(TtoW(lpszValue));
        GetIWebBrowser2()->PutProperty(TtoBSTR(pszPropertyName), v);
        VariantClear(&v);
    }


    // Sets the value of a property associated with the object.
    inline void CWebBrowser::PutProperty(LPCTSTR pszPropertyName, short nValue)
    {
        VARIANT v;
        v.vt = VT_I4;
        v.iVal = nValue;
        GetIWebBrowser2()->PutProperty(TtoBSTR(pszPropertyName), v);
        VariantClear(&v);
    }


    // Reloads the file that is currently displayed in the object.
    inline void CWebBrowser::Refresh()
    {
        GetIWebBrowser2()->Refresh();
    }


    // Reloads the file that is currently displayed with the specified refresh level.
    inline void CWebBrowser::Refresh2(int nLevel)
    {
        VARIANT v;
        v.vt = VT_I4;
        v.intVal = nLevel;
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

