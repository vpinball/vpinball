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

#ifndef _WIN32XX_WEBBROWSER_H_
#define _WIN32XX_WEBBROWSER_H_

#include "wxx_appcore0.h"
#include <ExDisp.h>
#include <OCidl.h>

#ifdef _MSC_VER
  #pragma warning ( push )
  #pragma warning ( disable : 4091 )  // temporarily disable C4091 warning
#endif // _MSC_VER

#include <ShlObj.h>

#ifdef _MSC_VER
  #pragma warning ( pop )
#endif // _MSC_VER

#if defined (_MSC_VER) && (_MSC_VER >= 1920)   // >= VS2019
#pragma warning ( push )
#pragma warning ( disable : 26812 )            // enum type is unscoped.
#endif // (_MSC_VER) && (_MSC_VER >= 1920)


namespace Win32xx
{
    ////////////////////////////////////////////////////////////////////////
    // This class implements a COM container which hosts an ActiveX control,
    // such as internet explorer.
    class CAXHost : public IOleInPlaceFrame, public IOleClientSite,
                    public IOleInPlaceSite, public IOleControlSite,
                    public IDispatch
    {
    public:
        CAXHost();
        virtual ~CAXHost();

        // CAXHost methods
        virtual STDMETHODIMP Activate();
        virtual STDMETHODIMP CreateControl(BSTR clsidName, void** ppUnk);
        virtual STDMETHODIMP CreateControl(REFCLSID clsid, void** ppUnk);
        virtual STDMETHODIMP Remove();
        virtual STDMETHODIMP SetParent(HWND hWndParent);
        virtual STDMETHODIMP SetLocation(int x, int y, int width, int height);
        virtual STDMETHODIMP SetVisible(BOOL isVisible);

        // IDispatch Methods
        virtual STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR** names,
                                unsigned int namesCount, LCID lcid, DISPID* pID);
        virtual STDMETHODIMP GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo** pptinfo);
        virtual STDMETHODIMP GetTypeInfoCount(unsigned int* pctinfo);
        virtual STDMETHODIMP Invoke(DISPID dispID, REFIID riid, LCID lcid, WORD flags,
                                    DISPPARAMS* pParams, VARIANT* result, EXCEPINFO* pExecInfo,
                                    unsigned int* pArgErr);

        // IOleClientSite Methods
        virtual STDMETHODIMP GetContainer(LPOLECONTAINER* ppContainer);
        virtual STDMETHODIMP GetMoniker(DWORD assign, DWORD whichMoniker, LPMONIKER* ppMk);
        virtual STDMETHODIMP OnShowWindow(BOOL show);
        virtual STDMETHODIMP RequestNewObjectLayout();
        virtual STDMETHODIMP SaveObject();
        virtual STDMETHODIMP ShowObject();

        // IOleControlSite Methods
        virtual STDMETHODIMP GetExtendedControl(IDispatch** ppDisp);
        virtual STDMETHODIMP LockInPlaceActive(BOOL lock);
        virtual STDMETHODIMP OnControlInfoChanged();
        virtual STDMETHODIMP OnFocus(BOOL gotFocus);
        virtual STDMETHODIMP ShowPropertyFrame();
        virtual STDMETHODIMP TransformCoords(POINTL* pHimetric, POINTF* pContainer, DWORD flags);
        virtual STDMETHODIMP TranslateAccelerator(LPMSG pMsg, DWORD modifiers);

        // IOleInPlaceFrame Methods
        virtual STDMETHODIMP EnableModeless(BOOL enable);
        virtual STDMETHODIMP InsertMenus(HMENU shared, LPOLEMENUGROUPWIDTHS pMenuWidths);
        virtual STDMETHODIMP RemoveMenus(HMENU shared);
        virtual STDMETHODIMP SetMenu(HMENU shared, HOLEMENU holemenu, HWND activeObject);
        virtual STDMETHODIMP SetStatusText(LPCOLESTR pStatusText);
        virtual STDMETHODIMP TranslateAccelerator(LPMSG pMsg, WORD id);

        // IOleInPlaceSite Methods
        virtual STDMETHODIMP CanInPlaceActivate();
        virtual STDMETHODIMP DeactivateAndUndo();
        virtual STDMETHODIMP DiscardUndoState();
        virtual STDMETHODIMP GetWindowContext(IOleInPlaceFrame** ppFrame,
                                  IOleInPlaceUIWindow** ppDoc, LPRECT pRect,
                                  LPRECT pClipRect, LPOLEINPLACEFRAMEINFO pFrameInfo);
        virtual STDMETHODIMP OnInPlaceActivate();
        virtual STDMETHODIMP OnInPlaceDeactivate();
        virtual STDMETHODIMP OnPosRectChange(LPCRECT pRect);
        virtual STDMETHODIMP OnUIActivate();
        virtual STDMETHODIMP OnUIDeactivate(BOOL undoable);
        virtual STDMETHODIMP Scroll(SIZE scrollExtent);

        // IOleInPlaceUIWindow Methods
        virtual STDMETHODIMP GetBorder(LPRECT pBorderRect);
        virtual STDMETHODIMP RequestBorderSpace(LPCBORDERWIDTHS pBorderWidths);
        virtual STDMETHODIMP SetBorderSpace(LPCBORDERWIDTHS pBorderWidths);
        virtual STDMETHODIMP SetActiveObject(IOleInPlaceActiveObject* pActiveObject,
                                             LPCOLESTR pObjName);

        // IOleWindow Methods
        virtual STDMETHODIMP ContextSensitiveHelp(BOOL enterMode);
        virtual STDMETHODIMP GetWindow(HWND* pHwnd);

        // IUnknown Methods
        virtual STDMETHODIMP_(ULONG) AddRef();
        virtual STDMETHODIMP QueryInterface(REFIID riid, void** ppObject);
        virtual STDMETHODIMP_(ULONG) Release();

    private:
         HWND        m_hwnd;        // window handle of the container
         IUnknown*   m_pUnk;        // IUnknown of contained object
         CRect       m_controlRect; // size of control
    };


    ////////////////////////////////////////////////////////
    // This class provides the functionality of a WebBrower,
    // using the IWebBrower2 interface.
    class CWebBrowser : public CWnd
    {
    public:
        CWebBrowser();
        virtual ~CWebBrowser();

        // Accessors
        virtual CAXHost* GetAXHost() { return &m_axHost; }

        LPDISPATCH GetApplication() const;
        BOOL    GetBusy() const;
        LPDISPATCH GetContainer() const;
        LPDISPATCH GetDocument() const;
        BOOL    GetFullScreen() const;
        long    GetHeight() const;
        IWebBrowser2* GetIWebBrowser2() const { return m_pIWebBrowser2; }
        long    GetLeft() const;
        CString GetLocationName() const;
        CString GetLocationURL() const;
        BOOL    GetOffline() const;
        LPDISPATCH GetParent() const;
        VARIANT GetProperty(LPCTSTR propertyName) const;
        READYSTATE GetReadyState() const;
        BOOL    GetRegisterAsBrowser() const;
        BOOL    GetTheaterMode() const;
        long    GetTop() const;
        BOOL    GetTopLevelContainer() const;
        CString GetType() const;
        BOOL    GetVisible() const;
        long    GetWidth() const;
        HRESULT SetFullScreen(BOOL isFullScreen);
        HRESULT SetHeight(long height);
        HRESULT SetLeft(long leftEdge);
        HRESULT SetOffline(BOOL isOffline);
        HRESULT SetRegisterAsBrowser(BOOL isBrowser);
        HRESULT SetTheaterMode(BOOL isTheaterMode);
        HRESULT SetTop(long topEdge);
        HRESULT SetVisible(BOOL isVisible);
        HRESULT SetWidth(long width);

        // Operations
        HRESULT AddWebBrowserControl();
        HRESULT ExecWB(OLECMDID cmdID, OLECMDEXECOPT cmdExecOpt, VARIANT* pIn, VARIANT* pOut);
        HRESULT GoBack();
        HRESULT GoForward();
        HRESULT GoHome();
        HRESULT GoSearch();
        HRESULT Navigate(LPCTSTR URL, DWORD flags = 0, LPCTSTR targetFrameName = NULL,
                        LPCTSTR headers = NULL, LPVOID pPostData = NULL, DWORD postDataLen = 0);
        HRESULT Navigate2(LPITEMIDLIST pIDL, DWORD flags = 0, LPCTSTR targetFrameName = NULL);
        HRESULT Navigate2(LPCTSTR URL, DWORD flags = 0, LPCTSTR targetFrameName = NULL,
                         LPCTSTR headers = NULL, LPVOID pPostData = NULL, DWORD postDataLen = 0);
        HRESULT PutProperty(LPCTSTR propertyName, const VARIANT& value);
        HRESULT PutProperty(LPCTSTR propertyName, double value);
        HRESULT PutProperty(LPCTSTR propertyName, long value);
        HRESULT PutProperty(LPCTSTR propertyName, LPCTSTR value);
        HRESULT PutProperty(LPCTSTR propertyName, short value);
        HRESULT Refresh();
        HRESULT Refresh2(int evel);
        HRESULT Stop();

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
        CAXHost  m_axHost;              // The ActiveX host
        IWebBrowser2* m_pIWebBrowser2;  // Interface to the ActiveX web browser control
    };

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

namespace Win32xx
{

    /////////////////////////////////////////
    // Definitions for the CAXHost class
    //

    inline CAXHost::CAXHost() : m_hwnd(NULL), m_pUnk(NULL)
    {
    }

    inline CAXHost::~CAXHost()
    {
        if (m_pUnk)
            m_pUnk->Release();
    }

    // Activates the hosted control, along with any of its UI tools.
    inline STDMETHODIMP CAXHost::Activate()
    {
        HRESULT hr = E_FAIL;
        if (m_pUnk)
        {
            IOleObject* pObject;
            VERIFY(SUCCEEDED(hr = m_pUnk->QueryInterface(IID_IOleObject, reinterpret_cast<void**>(&pObject))));
            if (pObject)
            {
                VERIFY(SUCCEEDED(hr = pObject->DoVerb(OLEIVERB_UIACTIVATE, NULL, this, 0, m_hwnd, &m_controlRect)));
                pObject->Release();
            }
        }

        return hr;
    }

    // Creates the hosted control using its CLSID name, and adds it to the
    // CAXHost container.
    inline STDMETHODIMP CAXHost::CreateControl(BSTR clsidName, void** ppUnk)
    {
        CLSID   clsid;
        HRESULT hr;
        VERIFY(SUCCEEDED(hr = CLSIDFromString(clsidName, &clsid)));
        VERIFY(SUCCEEDED(hr = CreateControl(clsid, ppUnk)));

        return hr;
    }

    // Creates the hosted control using its CLSID, and adds it to the
    // CAXHost container.
    inline STDMETHODIMP CAXHost::CreateControl(REFCLSID clsid, void** ppUnk)
    {
        HRESULT hr = E_FAIL;

        VERIFY(SUCCEEDED(hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER, IID_IUnknown, ppUnk)));
        m_pUnk = reinterpret_cast<IUnknown*>(*ppUnk);
        if (m_pUnk)
        {
            IOleObject* pObject;
            VERIFY(SUCCEEDED(hr = m_pUnk->QueryInterface(IID_IOleObject, reinterpret_cast<void**>(&pObject))));
            if (pObject)
            {
                VERIFY(SUCCEEDED(hr = pObject->SetClientSite(this)));
                pObject->Release();

                IPersistStreamInit* ppsi;
                VERIFY(SUCCEEDED(hr = m_pUnk->QueryInterface(IID_IPersistStreamInit, reinterpret_cast<void**>(&ppsi))));
                if (ppsi)
                {
                    VERIFY(SUCCEEDED(hr = ppsi->InitNew()));
                    ppsi->Release();
                }
            }
        }

        return hr;
    }

    // Sets CAXHost's reference count.
    inline STDMETHODIMP_(ULONG) CAXHost::AddRef()
    {
        // Automatic deletion is not required.
        return 1;
    }

    // Determines whether or not the container can activate the object in place.
    inline STDMETHODIMP CAXHost::CanInPlaceActivate()
    {
        return S_OK;
    }

    // Determines whether context-sensitive help mode should be entered during
    // an in-place activation session.
    inline STDMETHODIMP CAXHost::ContextSensitiveHelp(BOOL)
    {
        return E_NOTIMPL;
    }

    // Causes the container to end the in-place session, deactivate the object,
    // and revert to its own saved undo state.
    inline STDMETHODIMP CAXHost::DeactivateAndUndo()
    {
        return E_NOTIMPL;
    }

    // Tells the container that the object no longer has any undo state and that
    // the container should not call IOleInPlaceObject::ReActivateAndUndo.
    inline STDMETHODIMP CAXHost::DiscardUndoState()
    {
        return E_NOTIMPL;
    }

    // Enables or disables a frame's modeless dialog boxes.
    inline STDMETHODIMP CAXHost::EnableModeless(BOOL)
    {
        return E_NOTIMPL;
    }

    // Returns a RECT structure in which the object can put toolbars
    // and similar controls while active in place.
    inline STDMETHODIMP CAXHost::GetBorder(LPRECT)
    {
        return E_NOTIMPL;
    }

    // Returns a pointer to the container's IOleContainer interface.
    inline STDMETHODIMP CAXHost::GetContainer(LPOLECONTAINER*)
    {
        return E_NOINTERFACE;
    }

    // Requests an IDispatch pointer to the extended control that
    // the container uses to wrap the real control.
    // Call Release on this pointer when it is no longer required.
    inline STDMETHODIMP CAXHost::GetExtendedControl(IDispatch** ppDisp)
    {
        if (ppDisp == NULL)
            return E_INVALIDARG;

        *ppDisp = static_cast<IDispatch*>(this);
        (*ppDisp)->AddRef();

        return S_OK;
    }

    // Maps a single member and an optional set of argument names to
    // a corresponding set of integer DISPIDs, which can be used on
    // subsequent calls to IDispatch::Invoke.
    inline STDMETHODIMP CAXHost::GetIDsOfNames(REFIID, OLECHAR**, unsigned int, LCID, DISPID* pID)
    {
        *pID = DISPID_UNKNOWN;
        return DISP_E_UNKNOWNNAME;
    }

    // Retrieves an embedded object's moniker, which the caller
    // can use to link to the object.
    inline STDMETHODIMP CAXHost::GetMoniker(DWORD, DWORD, LPMONIKER*)
    {
        return E_NOTIMPL;
    }

    // Retrieves the type information for an object, which can then
    // be used to get the type information for the IDispatch interface.
    inline STDMETHODIMP CAXHost::GetTypeInfo(unsigned int, LCID, ITypeInfo**)
    {
        return E_NOTIMPL;
    }

    // Retrieves the number of type information interfaces that the IDispatch
    // an object provides (either 0 or 1).
    inline STDMETHODIMP CAXHost::GetTypeInfoCount(unsigned int*)
    {
        return E_NOTIMPL;
    }

    // Returns the window handle assigned to the CAXHost container.
    inline STDMETHODIMP CAXHost::GetWindow(HWND* pHwnd)
    {
        if (!IsWindow(m_hwnd))
            return S_FALSE;

        *pHwnd = m_hwnd;
        return S_OK;
    }

    // Enables the in - place object to retrieve the window interfaces that
    // form the window object hierarchy, and the position in the parent
    // window where the object's in-place activation window should be placed.
    // Call Release on ppFrame when it is no longer required.
    inline STDMETHODIMP CAXHost::GetWindowContext (IOleInPlaceFrame** ppFrame, IOleInPlaceUIWindow** ppIIPUIWin,
                                      LPRECT pRect, LPRECT pClipRect, LPOLEINPLACEFRAMEINFO pFrameInfo)
    {
        *ppFrame = (IOleInPlaceFrame*)this;
        *ppIIPUIWin = NULL;

        RECT rect;
        VERIFY(::GetClientRect(m_hwnd, &rect));
        pRect->left       = 0;
        pRect->top        = 0;
        pRect->right      = rect.right;
        pRect->bottom     = rect.bottom;

        CopyRect(pClipRect, pRect);

        pFrameInfo->cb             = sizeof(OLEINPLACEFRAMEINFO);
        pFrameInfo->fMDIApp        = FALSE;
        pFrameInfo->hwndFrame      = m_hwnd;
        pFrameInfo->haccel         = 0;
        pFrameInfo->cAccelEntries  = 0;

        (*ppFrame)->AddRef();
        return S_OK;
    }

    // Allows the container to insert its menu groups into the composite menu
    // to be used during the in-place session.
    inline STDMETHODIMP CAXHost::InsertMenus(HMENU, LPOLEMENUGROUPWIDTHS)
    {
        return E_NOTIMPL;
    }

    // Provides access to properties and methods exposed by the container.
    inline STDMETHODIMP CAXHost::Invoke(DISPID, REFIID, LCID, WORD, DISPPARAMS*, VARIANT*, EXCEPINFO*, unsigned int*)
    {
        return DISP_E_MEMBERNOTFOUND;
    }

    // Indicates whether or not a control should remain in-place active.
    inline STDMETHODIMP CAXHost::LockInPlaceActive(BOOL)
    {
        return E_NOTIMPL;
    }

    // Informs the container that the control's CONTROLINFO structure
    // has changed and that the container should call the control's
    // IOleControl::GetControlInfo for an update.
    inline STDMETHODIMP CAXHost::OnControlInfoChanged()
    {
        return E_NOTIMPL;
    }

    // Indicates whether the control managed by this control site has
    // gained or lost the focus, according to the fGotFocus parameter.
    inline STDMETHODIMP CAXHost::OnFocus(BOOL)
    {
        return E_NOTIMPL;
    }

    // Notifies the container that one of its objects is being activated
    // in place.
    inline STDMETHODIMP CAXHost::OnInPlaceActivate()
    {
        return S_OK;
    }

    // Notifies the container that the object is no longer active in place.
    inline STDMETHODIMP CAXHost::OnInPlaceDeactivate()
    {
        return S_OK;
    }

    // Indicates the object's extents have changed.
    inline STDMETHODIMP CAXHost::OnPosRectChange(LPCRECT)
    {
        return S_OK;
    }

    // Notifies a container when an embedded object's window is
    // about to become visible or invisible.
    inline STDMETHODIMP CAXHost::OnShowWindow(BOOL)
    {
        return S_OK;
    }

    // Notifies the container that the object is about to be activated
    // in place and that the object is going to replace the container's
    // main menu with an in-place composite menu.
    inline STDMETHODIMP CAXHost::OnUIActivate()
    {
        return S_OK;
    }

    // Notifies the container on deactivation that it should reinstall
    // its user interface and take focus, and whether or not the object
    // has an undoable state.
    inline STDMETHODIMP CAXHost::OnUIDeactivate(BOOL)
    {
        return E_NOTIMPL;
    }

    // Returns a pointer to the specified interface on an object.
    // Call Release on this pointer when it is no longer required.
    inline STDMETHODIMP CAXHost::QueryInterface(REFIID riid, void** ppObject)
    {
        if (!ppObject)
            return E_POINTER;

        if (IsEqualIID(riid, IID_IOleInPlaceFrame))
            *ppObject = static_cast<IOleInPlaceFrame*>(this);
        else if (IsEqualIID(riid, IID_IOleInPlaceSite))
            *ppObject = static_cast<IOleInPlaceSite*>(this);
        else if (IsEqualIID(riid, IID_IOleClientSite))
            *ppObject = static_cast<IOleClientSite*>(this);
        else if (IsEqualIID(riid, IID_IOleControlSite))
            *ppObject = static_cast<IOleControlSite*>(this);
        else if (IsEqualIID(riid, IID_IDispatch))
            *ppObject = static_cast<IDispatch*>(this);
        else if (IsEqualIID(riid, IID_IUnknown))
            *ppObject = static_cast<IUnknown*>(static_cast<IOleInPlaceFrame*>(this));
        else
        {
            *ppObject = NULL;
            return E_NOINTERFACE;
        }

        AddRef();
        return S_OK;
    }

    // Sets CAXHost's reference count.
    inline STDMETHODIMP_(ULONG) CAXHost::Release()
    {
        // Automatic deletion is not required.
        return 1;
    }

    // Removes the hosted control from the CAXHost container.
    inline STDMETHODIMP CAXHost::Remove()
    {
        HRESULT hr = E_FAIL;

        if (m_pUnk)
        {
            IOleInPlaceObject* pipo;
            VERIFY(SUCCEEDED(hr = m_pUnk->QueryInterface(IID_IOleInPlaceObject, reinterpret_cast<void**>(&pipo))));
            if (pipo)
            {
                VERIFY(SUCCEEDED(hr = pipo->UIDeactivate()));
                VERIFY(SUCCEEDED(hr = pipo->InPlaceDeactivate()));
                pipo->Release();
            }

            IOleObject* pObject;
            VERIFY(SUCCEEDED(hr = m_pUnk->QueryInterface(IID_IOleObject, reinterpret_cast<void**>(&pObject))));
            if (pObject)
            {
                VERIFY(SUCCEEDED(hr = pObject->Close(OLECLOSE_NOSAVE)));
                VERIFY(SUCCEEDED(hr = pObject->SetClientSite(NULL)));
                pObject->Release();
            }
        }

        return hr;
    }

    // Gives the container a chance to remove its menu elements from the
    // in-place composite menu.
    inline STDMETHODIMP CAXHost::RemoveMenus(HMENU)
    {
        return E_NOTIMPL;
    }

    // Determines if there is available space for tools to be installed around
    // the object's window frame while the object is active in place.
    inline STDMETHODIMP CAXHost::RequestBorderSpace(LPCBORDERWIDTHS)
    {
        return E_NOTIMPL;
    }

    // Asks container to allocate more or less space for displaying an
    // embedded object.
    inline STDMETHODIMP CAXHost::RequestNewObjectLayout()
    {
        return E_NOTIMPL;
    }

    // Saves the object associated with the client site.
    inline STDMETHODIMP CAXHost::SaveObject()
    {
        return E_NOTIMPL;
    }

    // Tells the container to scroll the view of the object by a specified
    // number of pixels.
    inline STDMETHODIMP CAXHost::Scroll(SIZE)
    {
        return E_NOTIMPL;
    }

    // Provides a direct channel of communication between the object and each
    // of the frame and document windows.
    inline STDMETHODIMP CAXHost::SetActiveObject(IOleInPlaceActiveObject*, LPCOLESTR)
    {
        return E_NOTIMPL;
    }

    // Allocates space for the border requested in the call to
    // IOleInPlaceUIWindow::RequestBorderSpace.
    inline STDMETHODIMP CAXHost::SetBorderSpace(LPCBORDERWIDTHS)
    {
        return E_NOTIMPL;
    }

    // Displays the specified portion of the hosted control.
    inline STDMETHODIMP CAXHost::SetLocation(int x, int y, int width, int height)
    {
        m_controlRect.SetRect(x, y, x + width, y + height);

        HRESULT hr = E_FAIL;
        if (m_pUnk)
        {
            IOleInPlaceObject* pipo;
            VERIFY(SUCCEEDED(hr = m_pUnk->QueryInterface(IID_IOleInPlaceObject, reinterpret_cast<void**>(&pipo))));
            if (pipo)
            {
                VERIFY(SUCCEEDED(hr = pipo->SetObjectRects(&m_controlRect, &m_controlRect)));
                pipo->Release();
            }
        }

        return hr;
    }

    // Installs the composite menu in the window frame containing the object
    // being activated in place.
    inline STDMETHODIMP CAXHost::SetMenu(HMENU, HOLEMENU, HWND)
    {
        return E_NOTIMPL;
    }

    // Assigns the specified window handle to the CAXHost container.
    inline STDMETHODIMP CAXHost::SetParent(HWND hWndParent)
    {
        m_hwnd = hWndParent;
        return S_OK;
    }

    // Sets and displays status text about the in-place object in the
    // container's frame window status line.
    inline STDMETHODIMP CAXHost::SetStatusText(LPCOLESTR pStatusText)
    {
        if (NULL == pStatusText)
            return E_POINTER;

        return S_OK;
    }

    // Shows or hides the hosted control.
    inline STDMETHODIMP CAXHost::SetVisible(BOOL isVisible)
    {
        HRESULT hr = E_FAIL;
        if (m_pUnk)
        {
            IOleObject* pObject;
            VERIFY(SUCCEEDED(hr = m_pUnk->QueryInterface(IID_IOleObject, reinterpret_cast<void**>(&pObject))));
            if (pObject)
            {
                if (isVisible)
                {
                    VERIFY(SUCCEEDED(hr = pObject->DoVerb(OLEIVERB_INPLACEACTIVATE, NULL, this, 0, m_hwnd, &m_controlRect)));
                    VERIFY(SUCCEEDED(hr = pObject->DoVerb(OLEIVERB_SHOW, NULL, this, 0, m_hwnd, &m_controlRect)));
                }
                else
                    VERIFY(SUCCEEDED(hr = pObject->DoVerb(OLEIVERB_HIDE, NULL, this, 0, m_hwnd, NULL)));

                pObject->Release();
            }
        }

        return hr;
    }

    // Tells the container to position the object so it is visible to the user.
    // This method ensures that the container itself is visible and not
    // minimized.
    inline STDMETHODIMP CAXHost::ShowObject()
    {
        return S_OK;
    }

    // Instructs a container to display a property sheet for the control
    // embedded in this site.
    inline STDMETHODIMP CAXHost::ShowPropertyFrame()
    {
        return E_NOTIMPL;
    }

    // Converts between a POINTL structure expressed in HIMETRIC units
    // (as is standard in OLE) and a POINTF structure expressed in units
    // specified by the container.
    inline STDMETHODIMP CAXHost::TransformCoords(POINTL*, POINTF*, DWORD)
    {
        return E_NOTIMPL;
    }

    // Translates accelerator keystrokes intended for the container's frame
    // while an object is active in place.
    inline STDMETHODIMP CAXHost::TranslateAccelerator(LPMSG, WORD)
    {
        return S_OK;
    }

    // Instructs the control site to process the keystroke described in
    // pMsg modified by the flags in modifiers.
    inline STDMETHODIMP CAXHost::TranslateAccelerator(LPMSG, DWORD)
    {
        return S_FALSE;
    }


    ////////////////////////////////////////
    // Definitions for the CWebBrowser class
    //

    inline CWebBrowser::CWebBrowser() : m_pIWebBrowser2(0)
    {
    }

    inline CWebBrowser::~CWebBrowser()
    {
        if (m_pIWebBrowser2)
            m_pIWebBrowser2->Release();
    }

    // Adds the IWebBrowser interface to the ActiveX host.
    // Refer to IID_IWebBrowser2 in the Windows API documentation for more information.
    inline HRESULT CWebBrowser::AddWebBrowserControl()
    {
        IUnknown* pUnk = NULL;
        VERIFY(SUCCEEDED(GetAXHost()->CreateControl(CLSID_WebBrowser, (void**)&pUnk)));
        VERIFY(SUCCEEDED(GetAXHost()->SetParent(*this)));
        VERIFY(SUCCEEDED(GetAXHost()->SetVisible(TRUE)));
        VERIFY(SUCCEEDED(GetAXHost()->Activate()));

        HRESULT hr = E_FAIL;
        if (pUnk)
        {
            // Store the pointer to the WebBrowser control
            VERIFY(SUCCEEDED(hr = pUnk->QueryInterface(IID_IWebBrowser2, reinterpret_cast<void**>(&m_pIWebBrowser2))));

            // Navigate to an empty page
            VERIFY(SUCCEEDED(hr = Navigate(_T("about:blank"))));
        }

        return hr;
    }

    // Called when the WebBrowser window's HWND is attached this object.
    inline void CWebBrowser::OnAttach()
    {
        if (m_pIWebBrowser2 == NULL)
            VERIFY(SUCCEEDED(AddWebBrowserControl()));
    }

    // Called when the web browser window is created.
    inline int CWebBrowser::OnCreate(CREATESTRUCT&)
    {
        VERIFY(SUCCEEDED(AddWebBrowserControl()));
        return 0;
    }

    // Called when the window is destroyed.
    inline void CWebBrowser::OnDestroy()
    {
        VERIFY(SUCCEEDED(m_pIWebBrowser2->Stop()));
        VERIFY(SUCCEEDED(GetAXHost()->Remove()));
    }

    // Called when the window is resized.
    inline void CWebBrowser::OnSize(int width, int height)
    {
        // position the container
        VERIFY(SUCCEEDED(GetAXHost()->SetLocation(0, 0, width, height)));
    }

    // Provides default message processing for the web browser window.
    inline LRESULT CWebBrowser::WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        switch (msg)
        {
        case WM_SIZE:
            OnSize(LOWORD(lparam), HIWORD(lparam));
            break;
        }

        return CWnd::WndProcDefault(msg, wparam, lparam);
    }

    //////////////////////////////////////////////////
    // Wrappers for the IWebBrowser2 interface

    // Retrieves a pointer to the IDispatch interface for the the application that is hosting the WebBrowser Control.
    inline LPDISPATCH CWebBrowser::GetApplication() const
    {
        LPDISPATCH pDispatch = NULL;
        GetIWebBrowser2()->get_Application(&pDispatch);
        return pDispatch;
    }

    // Retrieves a value that indicates whether the object is engaged in a navigation or downloading operation.
    inline BOOL CWebBrowser::GetBusy() const
    {
        VARIANT_BOOL isBusy = VARIANT_FALSE;
        GetIWebBrowser2()->get_Busy(&isBusy);
        return (isBusy != 0);
    }

    // Retrieves a pointer to the IDispatch interface to a container. This property returns the same pointer
    // as GetParent.
    inline LPDISPATCH CWebBrowser::GetContainer() const
    {
        LPDISPATCH pDispatch = NULL;
        GetIWebBrowser2()->get_Container(&pDispatch);
        return pDispatch;
    }

    // Retrieves a pointer to the IDispatch interface of the active document object. Call QueryInterface on the
    // IDispatch received from this property get the Component Object Model (COM) interfaces IHTMLDocument,
    // IHTMLDocument2, and IHTMLDocument3.
    inline LPDISPATCH CWebBrowser::GetDocument() const
    {
        LPDISPATCH Value = NULL;
        GetIWebBrowser2()->get_Document(&Value);
        return Value;
    }

    // Retrieves a value that indicates whether Internet Explorer is in full-screen mode or normal window mode.
    inline BOOL CWebBrowser::GetFullScreen() const
    {
        VARIANT_BOOL value = VARIANT_FALSE;
        GetIWebBrowser2()->get_FullScreen(&value);
        return (value != 0);
    }

    // Retrieves the height of the object.
    inline long CWebBrowser::GetHeight() const
    {
        long height = 0;
        GetIWebBrowser2()->get_Height(&height);
        return height;
    }

    // Retrieves the coordinate of the left edge of the object.
    inline long CWebBrowser::GetLeft() const
    {
        long left = 0;
        GetIWebBrowser2()->get_Left(&left);
        return left;
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

    // Retrieves a value that indicates whether the object is operating in offline mode.
    inline BOOL CWebBrowser::GetOffline() const
    {
        VARIANT_BOOL isOffLine = VARIANT_FALSE;
        GetIWebBrowser2()->get_Offline(&isOffLine);
        return (isOffLine != 0);
    }

    // Retrieves a pointer to the IDispatch interface of the object that is the
    // container of the WebBrowser control. If the WebBrowser control is in a frame,
    // this method returns the automation interface of the document object in the
    // containing window. Otherwise, it delegates to the top-level control, if there is one.
    inline LPDISPATCH CWebBrowser::GetParent() const
    {
        LPDISPATCH pDispatch = NULL;
        GetIWebBrowser2()->get_Parent(&pDispatch);
        return pDispatch;
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

    // Retrieves the ready state of the object.
    inline READYSTATE CWebBrowser::GetReadyState() const
    {
        READYSTATE rs = READYSTATE_UNINITIALIZED;
        GetIWebBrowser2()->get_ReadyState(&rs);
        return rs;
    }

    // Retrieves a value that indicates whether the object is registered as a top-level browser window.
    inline BOOL CWebBrowser::GetRegisterAsBrowser() const
    {
        VARIANT_BOOL isTopLevel = VARIANT_FALSE;
#if !defined(__BORLANDC__) || (__BORLANDC__ >= 0x600)
        GetIWebBrowser2()->get_RegisterAsBrowser(&isTopLevel);
#endif
        return (isTopLevel != 0);
    }

    // Retrieves the theater mode state of the object.
    inline BOOL CWebBrowser::GetTheaterMode() const
    {
        VARIANT_BOOL isTheater = VARIANT_FALSE;
        GetIWebBrowser2()->get_TheaterMode(&isTheater);
        return (isTheater != 0);
    }

    // Retrieves the coordinate of the top edge of the object.
    inline long CWebBrowser::GetTop() const
    {
        long top;
        GetIWebBrowser2()->get_Top(&top);
        return top;
    }

    //Returns TRUE  if the object is a top-level container.
    inline BOOL CWebBrowser::GetTopLevelContainer() const
    {
        VARIANT_BOOL isTop = VARIANT_FALSE;
        GetIWebBrowser2()->get_TopLevelContainer(&isTop);
        return (isTop != 0);
    }

    // Retrieves the user type name of the contained document object.
    inline CString CWebBrowser::GetType() const
    {
        BSTR bstr;
        GetIWebBrowser2()->get_Type(&bstr);
        CString str(bstr);
        SysFreeString(bstr);
        return str;
    }

    // Retrieves a value that indicates whether the object is visible or hidden.
    inline BOOL CWebBrowser::GetVisible() const
    {
        VARIANT_BOOL isVisible = VARIANT_FALSE;
        GetIWebBrowser2()->get_Visible(&isVisible);
        return (isVisible != 0);
    }

    // Retrieves the width of the object.
    inline long CWebBrowser::GetWidth() const
    {
        long width = 0;
        GetIWebBrowser2()->get_Width(&width);
        return width;
    }

    // Sets a value that indicates whether Internet Explorer is in full-screen mode or normal window mode.
    inline HRESULT CWebBrowser::SetFullScreen(BOOL isFullScreen)
    {
        VARIANT_BOOL isFS = isFullScreen ? VARIANT_TRUE : VARIANT_FALSE;
        return GetIWebBrowser2()->put_FullScreen(isFS);
    }

    // Sets the height of the object.
    inline HRESULT CWebBrowser::SetHeight(long height)
    {
        return GetIWebBrowser2()->put_Height(height);
    }

    // Sets the coordinate of the left edge of the object.
    inline HRESULT CWebBrowser::SetLeft(long leftEdge)
    {
        return GetIWebBrowser2()->put_Left(leftEdge);
    }

    // Sets a value that indicates whether the object is operating in offline mode.
    inline HRESULT CWebBrowser::SetOffline(BOOL isOffline)
    {
        VARIANT_BOOL isOL = isOffline ? VARIANT_TRUE : VARIANT_FALSE;
        return GetIWebBrowser2()->put_Offline(isOL);
    }

    // Sets a value that indicates whether the object is registered as a top-level browser window.
    inline HRESULT CWebBrowser::SetRegisterAsBrowser(BOOL isBrowser)
    {
        VARIANT_BOOL isB = isBrowser ? VARIANT_TRUE : VARIANT_FALSE;
        return GetIWebBrowser2()->put_RegisterAsBrowser(isB);
    }

    // Sets the theatre mode state of the object.
    inline HRESULT CWebBrowser::SetTheaterMode(BOOL isTheaterMode)
    {
        VARIANT_BOOL isTM = isTheaterMode ? VARIANT_TRUE : VARIANT_FALSE;
        return GetIWebBrowser2()->put_TheaterMode(isTM);
    }

    // Sets the coordinate of the top edge of the object.
    inline HRESULT CWebBrowser::SetTop(long topEdge)
    {
        return GetIWebBrowser2()->put_Top(topEdge);
    }

    // Sets a value that indicates whether the object is visible or hidden.
    inline HRESULT CWebBrowser::SetVisible(BOOL isVisible)
    {
        VARIANT_BOOL isV = isVisible ? VARIANT_TRUE : VARIANT_FALSE;
        return GetIWebBrowser2()->put_Visible(isV);
    }

    // Sets the width of the object.
    inline HRESULT CWebBrowser::SetWidth(long width)
    {
        return GetIWebBrowser2()->put_Width(width);
    }

    // Executes a command using the IOleCommandTarget interface.
    inline HRESULT CWebBrowser::ExecWB(OLECMDID cmdID, OLECMDEXECOPT cmdExecOpt, VARIANT* in, VARIANT* out)
    {
        return GetIWebBrowser2()->ExecWB(cmdID, cmdExecOpt, in, out);
    }

    // Gets the value associated with the specified property name.
    inline VARIANT CWebBrowser::GetProperty( LPCTSTR propertyName ) const
    {
        VARIANT v;
        GetIWebBrowser2()->GetProperty( TtoBSTR(propertyName), &v );
        return v;
    }

    // Navigates backward one item in the history list.
    inline HRESULT CWebBrowser::GoBack()
    {
        return GetIWebBrowser2()->GoBack();
    }

    // Navigates forward one item in the history list.
    inline HRESULT CWebBrowser::GoForward()
    {
        return GetIWebBrowser2()->GoForward();
    }

    // Navigates to the current home or start page.
    inline HRESULT CWebBrowser::GoHome()
    {
        return GetIWebBrowser2()->GoHome();
    }

    // Navigates to the current search page.
    inline HRESULT CWebBrowser::GoSearch()
    {
        return GetIWebBrowser2()->GoSearch();
    }

    // Navigates to a resource identified by a URL or to a file identified by a full path.
    inline HRESULT CWebBrowser::Navigate(LPCTSTR URL,   DWORD flags /*= 0*/, LPCTSTR targetFrameName /*= NULL*/,
                    LPCTSTR headers /*= NULL*/, LPVOID pPostData /*= NULL*/,   DWORD postDataLen /*= 0*/)
    {
        VARIANT flagsVariant;
        flagsVariant.vt = VT_I4;
        flagsVariant.lVal = flags;

        VARIANT targetVariant;
        targetVariant.vt = VT_BSTR;
        targetVariant.bstrVal = SysAllocString(TtoW(targetFrameName));

        SAFEARRAY* psa = SafeArrayCreateVector(VT_UI1, 0, postDataLen);
        CopyMemory(psa->pvData, pPostData, postDataLen);
        VARIANT dataVariant;
        dataVariant.vt = VT_ARRAY|VT_UI1;
        dataVariant.parray = psa;

        VARIANT headersVariant;
        headersVariant.vt = VT_BSTR;
        headersVariant.bstrVal = SysAllocString(TtoW(headers));
        BSTR url = SysAllocString(TtoW(URL));
        HRESULT hr = E_FAIL;
        if (url)
            hr = GetIWebBrowser2()->Navigate(url, &flagsVariant, &targetVariant, &dataVariant, &headersVariant);

        VariantClear(&flagsVariant);
        VariantClear(&targetVariant);
        VariantClear(&dataVariant);
        VariantClear(&headersVariant);

        return hr;
    }

    // Navigates the browser to a location specified by a pointer to an item identifier list (PIDL) for an entity in the Microsoft Windows Shell namespace.
    inline HRESULT CWebBrowser::Navigate2(LPITEMIDLIST pIDL, DWORD flags /*= 0*/, LPCTSTR targetFrameName /*= NULL*/)
    {
        UINT cb = GetPidlLength(pIDL);
        LPSAFEARRAY pSA = SafeArrayCreateVector(VT_UI1, 0, cb);
        VARIANT pidlVariant;
        pidlVariant.vt = VT_ARRAY|VT_UI1;
        pidlVariant.parray = pSA;
        CopyMemory(pSA->pvData, pIDL, cb);

        VARIANT flagsVariant;
        flagsVariant.vt = VT_I4;
        flagsVariant.lVal = flags;

        VARIANT targetVariant;
        targetVariant.vt = VT_BSTR;
        targetVariant.bstrVal = SysAllocString(TtoW(targetFrameName));

        HRESULT hr = GetIWebBrowser2()->Navigate2(&pidlVariant, &flagsVariant, &targetVariant, 0, 0);

        VariantClear(&pidlVariant);
        VariantClear(&flagsVariant);
        VariantClear(&targetVariant);

        return hr;
    }

    // Navigates the browser to a location that is expressed as a URL.
    inline HRESULT CWebBrowser::Navigate2(LPCTSTR URL, DWORD flags /*= 0*/, LPCTSTR targetFrameName /*= NULL*/,
                     LPCTSTR headers /*= NULL*/,   LPVOID pPostData /*= NULL*/, DWORD postDataLen /*= 0*/)
    {
        VARIANT urlVariant;
        urlVariant.vt = VT_BSTR;
        urlVariant.bstrVal = SysAllocString(TtoW(URL));

        VARIANT flagsVariant;
        flagsVariant.vt = VT_I4;
        flagsVariant.lVal = flags;

        VARIANT TargetVariant;
        TargetVariant.vt = VT_BSTR;
        TargetVariant.bstrVal = SysAllocString(TtoW(targetFrameName));

        // Store the pidl in a SafeArray, and assign the SafeArray to a VARIANT
        SAFEARRAY* psa = SafeArrayCreateVector(VT_UI1, 0, postDataLen);
        CopyMemory(psa->pvData, pPostData, postDataLen);
        VARIANT dataVariant;
        dataVariant.vt = VT_ARRAY|VT_UI1;
        dataVariant.parray = psa;

        VARIANT headersVariant;
        headersVariant.vt = VT_BSTR;
        headersVariant.bstrVal = SysAllocString(TtoW(headers));

        HRESULT hr = GetIWebBrowser2()->Navigate2(&urlVariant, &flagsVariant, &TargetVariant, &dataVariant, &headersVariant);

        VariantClear(&urlVariant);
        VariantClear(&flagsVariant);
        VariantClear(&TargetVariant);
        VariantClear(&dataVariant);
        VariantClear(&headersVariant);

        return hr;
    }

    // Sets the value of a property associated with the object.
    inline HRESULT CWebBrowser::PutProperty(LPCTSTR propertyName, const VARIANT& value)
    {
        return GetIWebBrowser2()->PutProperty(TtoBSTR(propertyName), value);
    }

    // Sets the value of a property associated with the object.
    inline HRESULT CWebBrowser::PutProperty(LPCTSTR propertyName, double value)
    {
        VARIANT v;
        v.vt = VT_I4;
        v.dblVal = value;
        HRESULT hr = GetIWebBrowser2()->PutProperty(TtoBSTR(propertyName), v);
        VariantClear(&v);
        return hr;
    }

    // Sets the value of a property associated with the object.
    inline HRESULT CWebBrowser::PutProperty(LPCTSTR propertyName, long value)
    {
        VARIANT v;
        v.vt = VT_I4;
        v.lVal= value;
        HRESULT hr = GetIWebBrowser2()->PutProperty(TtoBSTR(propertyName), v);
        VariantClear(&v);
        return hr;
    }

    // Sets the value of a property associated with the object.
    inline HRESULT CWebBrowser::PutProperty(LPCTSTR propertyName, LPCTSTR value)
    {
        VARIANT v;
        v.vt = VT_BSTR;
        v.bstrVal= SysAllocString(TtoW(value));
        HRESULT hr = GetIWebBrowser2()->PutProperty(TtoBSTR(propertyName), v);
        VariantClear(&v);
        return hr;
    }

    // Sets the value of a property associated with the object.
    inline HRESULT CWebBrowser::PutProperty(LPCTSTR propertyName, short value)
    {
        VARIANT v;
        v.vt = VT_I4;
        v.iVal = value;
        HRESULT hr = GetIWebBrowser2()->PutProperty(TtoBSTR(propertyName), v);
        VariantClear(&v);
        return hr;
    }

    // Reloads the file that is currently displayed in the object.
    inline HRESULT CWebBrowser::Refresh()
    {
        return GetIWebBrowser2()->Refresh();
    }

    // Reloads the file that is currently displayed with the specified refresh level.
    inline HRESULT CWebBrowser::Refresh2(int level)
    {
        VARIANT v;
        v.vt = VT_I4;
        v.intVal = level;
        HRESULT hr = GetIWebBrowser2()->Refresh2(&v);
        VariantClear(&v);
        return hr;
    }

    // Cancels a pending navigation or download, and stops dynamic page elements,
    // such as background sounds and animations.
    inline HRESULT CWebBrowser::Stop()
    {
        return GetIWebBrowser2()->Stop();
    }

}

#if defined (_MSC_VER) && (_MSC_VER >= 1920)
#pragma warning ( pop )
#endif // (_MSC_VER) && (_MSC_VER >= 1920)

#endif  // _WIN32XX_WEBBROWSER_H_

