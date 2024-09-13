// Win32++   Version 10.0.0
// Release Date: 9th September 2024
//
//      David Nash
//      email: dnash@bigpond.net.au
//      url: https://sourceforge.net/projects/win32-framework
//           https://github.com/DavidNash2024/Win32xx
//
//
// Copyright (c) 2005-2024  David Nash
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


// Acknowledgements:
// ================
// A special thanks to James Brown and Jeff Glatt for publishing tutorials
// on docking and making these freely available. This code uses some of the
// ideas presented in those tutorials.
//
// Catch22 Docking Tutorials by James Brown:
// http://www.catch22.net/tuts
//
// Docking Toolbars in Plain C by Jeff Glatt
// http://www.codeproject.com/Articles/10224/Docking-Toolbars-in-Plain-C


///////////////////////////////////////////////////////
// wxx_docking.h
//  Declaration of the CDocker class

#ifndef _WIN32XX_DOCKING_H_
#define _WIN32XX_DOCKING_H_


#include "wxx_wincore.h"
#include "wxx_toolbar.h"
#include "wxx_tab.h"
#include "wxx_regkey.h"
#include "wxx_themes.h"
#include "default_resource.h"


namespace Win32xx
{
    // Docking Styles
    const int DS_DOCKED_LEFT         = 0x00001; // Dock the child left
    const int DS_DOCKED_RIGHT        = 0x00002; // Dock the child right
    const int DS_DOCKED_TOP          = 0x00004; // Dock the child top
    const int DS_DOCKED_BOTTOM       = 0x00008; // Dock the child bottom
    const int DS_NO_DOCKCHILD_LEFT   = 0x00010; // Prevent a child docking left
    const int DS_NO_DOCKCHILD_RIGHT  = 0x00020; // Prevent a child docking right
    const int DS_NO_DOCKCHILD_TOP    = 0x00040; // Prevent a child docking at the top
    const int DS_NO_DOCKCHILD_BOTTOM = 0x00080; // Prevent a child docking at the bottom
    const int DS_NO_RESIZE           = 0x00100; // Prevent resizing
    const int DS_NO_CAPTION          = 0x00200; // Prevent display of caption when docked
    const int DS_NO_CLOSE            = 0x00400; // Prevent closing of a docker while docked
    const int DS_NO_UNDOCK           = 0x00800; // Prevent manual undocking of a docker
    const int DS_CLIENTEDGE          = 0x01000; // Has a 3D border when docked
    const int DS_DOCKED_CONTAINER    = 0x04000; // Dock a container within a container
    const int DS_DOCKED_LEFTMOST     = 0x10000; // Leftmost outer docking
    const int DS_DOCKED_RIGHTMOST    = 0x20000; // Rightmost outer docking
    const int DS_DOCKED_TOPMOST      = 0x40000; // Topmost outer docking
    const int DS_DOCKED_BOTTOMMOST   = 0x80000; // Bottommost outer docking

    // Class declarations
    class CDockContainer;
    class CDocker;

    struct ContainerInfo
    {
        CString tabText;
        int tabImage;
        CDockContainer* pContainer;
        ContainerInfo() : tabImage(0), pContainer(0) {}
    };

    //////////////////////////////////////////////////////////////////////
    // A container (CDockContainer) is a specialized CTab window and
    // also the view of a docker (CDocker). Each container tab displays
    // a view window, and can display an optional toolbar. A container
    // can contain other containers.
    //
    // Each container is the view window for a docker. Containers support
    // container-in-container docking. When containers are docked within
    // each other, one container becomes the container parent. The container
    // parent provides a tab for the view window for each of the containers
    // docked within it. The view for each container along with possibly
    // its toolbar, is contained within the container parent's view page.
    //
    // When a container is docked within another container, it is hidden,
    // along with its parent docker window. The container parent becomes
    // the parent window for the view windows of these child containers.
    // GetContainerParent returns a pointer to the container parent.
    //
    // When a container docked within another container is undocked, the
    // undocked container window and its parent docker window become visible.
    // The container also resumes being the parent window of its view
    // window.
    class CDockContainer : public CTab
    {
    public:

        // Nested class. This is the Wnd for the window displayed over the client area
        // of the tab control.  The toolbar and view window are child windows of the
        // viewpage window. Only the ViewPage of the parent CDockContainer is displayed.
        // Its contents are updated with the view window of the active container
        // whenever a different tab is selected.
        class CViewPage : public CWnd
        {
        public:
            CViewPage();
            virtual ~CViewPage() override {}

            CDockContainer* GetContainer() const;
            CWnd* GetTabCtrl() const { return m_pTab; }
            CToolBar& GetToolBar() const { return *m_pToolBar; }
            CWnd* GetView() const { return m_pView; }
            void SetContainer(CDockContainer* pContainer) { m_pContainer = pContainer; }
            void SetToolBar(CToolBar& toolBar) { m_pToolBar = &toolBar; }
            void SetView(CWnd& wndView);
            void RecalcLayout() const;

        protected:
            virtual BOOL OnCommand(WPARAM wparam, LPARAM lparam) override;
            virtual int OnCreate(CREATESTRUCT& cs) override;
            virtual LRESULT OnNotify(WPARAM wparam, LPARAM lparam) override;
            virtual void PreRegisterClass(WNDCLASS& wc) override;
            LRESULT WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam) override;

        private:
            CViewPage(const CViewPage&) = delete;
            CViewPage& operator=(const CViewPage&) = delete;

            CToolBar m_toolBar;
            CToolBar* m_pToolBar;
            CDockContainer* m_pContainer;
            CString m_tooltip;
            CWnd* m_pView;
            CWnd* m_pTab;
        };

    public:
        CDockContainer();
        virtual ~CDockContainer() override;

        virtual void AddContainer(CDockContainer* pContainer, BOOL insert = FALSE, BOOL selectPage = TRUE);
        virtual void AddToolBarButton(UINT id, BOOL isEnabled = TRUE);
        virtual void CreateToolBar();
        virtual void DrawTabs(CDC& dc) override;
        virtual void RecalcLayout() override;
        virtual void RemoveContainer(CDockContainer* pWnd, BOOL updateParent = TRUE);
        virtual void SelectPage(int page) override;
        virtual void SwapTabs(int tab1, int tab2) override;
        virtual void SetTabsFontSize(int fontSize = 9) override;
        virtual void SetTabsIconSize(int iconSize = 16) override;
        virtual void UpdateTabs() override;

        // Accessors and mutators
        CDockContainer* GetActiveContainer() const;
        CWnd* GetActiveView() const;
        const std::vector<ContainerInfo>& GetAllContainers() const {return m_pContainerParent->m_allInfo; }
        CDockContainer* GetContainerParent() const { return m_pContainerParent; }
        CDockContainer* GetContainerFromIndex(size_t index) const;
        CDockContainer* GetContainerFromView(CWnd* pView) const;
        int GetContainerIndex(CDockContainer* pContainer) const;
        const CString& GetDockCaption() const { return m_caption; }
        CDocker* GetDocker() const            { return m_pDocker; }
        SIZE GetMaxTabTextSize() const;
        HICON GetTabIcon() const { return m_tabIcon; }
        const CString& GetTabText() const     { return m_tabText; }
        CToolBar& GetToolBar()  const         { return GetViewPage().GetToolBar(); }
        std::vector<UINT>& GetToolBarData()   { return m_toolBarData; }
        CWnd* GetView() const                 { return GetViewPage().GetView(); }
        CViewPage& GetViewPage() const { return *m_pViewPage; }
        void SetActiveContainer(CDockContainer* pContainer);
        void SetDocker(CDocker* pDocker)      { m_pDocker = pDocker; }
        void SetDockCaption(LPCTSTR caption);
        void SetHideSingleTab(BOOL hideSingle);
        void SetTabIcon(HICON tabIcon);
        void SetTabIcon(UINT iconID);
        void SetTabSize() override;
        void SetTabText(LPCTSTR text);
        void SetToolBar(CToolBar& toolBar) const   { GetViewPage().SetToolBar(toolBar); }
        void SetToolBarImages(COLORREF mask, UINT normalID, UINT hotID, UINT disabledID);
        void SetView(CWnd& wnd) const;

    protected:
        virtual void OnAttach() override;
        virtual LRESULT OnLButtonDown(UINT msg, WPARAM wparam, LPARAM lparam) override;
        virtual LRESULT OnLButtonUp(UINT msg, WPARAM wparam, LPARAM lparam) override;
        virtual LRESULT OnMouseLeave(UINT msg, WPARAM wparam, LPARAM lparam) override;
        virtual LRESULT OnMouseMove(UINT msg, WPARAM wparam, LPARAM lparam) override;
        virtual LRESULT OnNotifyReflect(WPARAM wparam, LPARAM lparam) override;
        virtual LRESULT OnSetFocus(UINT msg, WPARAM wparam, LPARAM lparam) override;
        virtual LRESULT OnSize(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnTCNSelChange(LPNMHDR pNMHDR) override;
        virtual LRESULT OnDpiChangedBeforeParent(UINT, WPARAM, LPARAM);
        virtual void PreCreate(CREATESTRUCT& cs) override;
        virtual void SetTBImageList(CToolBar& toolBar, UINT id, COLORREF mask);
        virtual void SetTBImageListDis(CToolBar& toolBar, UINT id, COLORREF mask);
        virtual void SetTBImageListHot(CToolBar& toolBar, UINT id, COLORREF mask);
        virtual void SetupToolBar();

        LRESULT WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam) override;

    private:
        CDockContainer(const CDockContainer&) = delete;
        CDockContainer& operator=(const CDockContainer&) = delete;

        int GetDockTabImageID(int tab) const;
        CString GetDockTabText(int tab) const;
        CSize GetTBImageSize(CBitmap* pBitmap) const;

        std::vector<ContainerInfo>& GetAll() const {return m_pContainerParent->m_allInfo;}
        std::vector<ContainerInfo> m_allInfo;          // vector of ContainerInfo structs
        std::vector<UINT> m_toolBarData;               // vector of resource IDs for ToolBar buttons
        CString m_tabText;
        CString m_caption;

        CViewPage m_viewPage;
        CViewPage* m_pViewPage;
        int m_currentPage;
        CDocker* m_pDocker;
        CDockContainer* m_pContainerParent;
        HICON m_tabIcon;
        int m_pressedTab;
        BOOL m_isHideSingleTab;
        CPoint m_oldMousePos;
    };

    struct DragPos
    {
        NMHDR hdr;
        POINT pos;
        UINT dockZone;
        CDocker* pDocker;
    };


    /////////////////////////////////////////////////////////////////////////////////
    // A CDocker window allows other CDocker windows to be "docked" inside it.
    // A CDocker can dock on the top, left, right or bottom side of a parent CDocker.
    // There is no theoretical limit to the number of CDockers within CDockers.
    //
    // Each docker has a view window. This view window can be a dock container or any
    // other child window.
    //
    // The docker class also provides the following windows
    //  * A dock client window with provides the caption for the docker.
    //  * A splitter bar that allows dockers to be resized.
    //  * Dock targets that are used to facilitate docking.
    //  * Dock hint windows that display a blue tint over area which will
    //    be covered if the docker is docked.
    class CDocker : public CWnd
    {
    public:

        // A nested class for the splitter bar that separates the docked panes.
        class CDockBar : public CWnd
        {
        public:
            CDockBar();
            virtual ~CDockBar() override;

            CBrush GetBrushBkgnd() const     {return m_brBackground;}
            CDocker& GetDocker() const       {assert (m_pDocker); return *m_pDocker;}
            int GetWidth() const             {return m_dockBarWidth;}
            void SetColor(COLORREF color);
            void SetDocker(CDocker* pDocker) {m_pDocker = pDocker;}
            void SetWidth(int width)         {m_dockBarWidth = width;}

        protected:
            virtual void OnDraw(CDC& dc) override;
            virtual LRESULT OnLButtonDown(UINT msg, WPARAM wparam, LPARAM lparam);
            virtual LRESULT OnLButtonUp(UINT msg, WPARAM wparam, LPARAM lparam);
            virtual LRESULT OnMouseMove(UINT msg, WPARAM wparam, LPARAM lparam);
            virtual LRESULT OnSetCursor(UINT msg, WPARAM wparam, LPARAM lparam);
            virtual void PreCreate(CREATESTRUCT& cs) override;
            virtual void PreRegisterClass(WNDCLASS& wc) override;
            virtual LRESULT WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam) override;

        private:
            CDockBar(const CDockBar&) = delete;
            CDockBar& operator=(const CDockBar&) = delete;

            void SendNotify(UINT messageID);

            CDocker* m_pDocker;
            DragPos m_dragPos;
            CBrush m_brBackground;
            int m_dockBarWidth;
        };

        // A nested class for the window inside a CDocker that includes all of this docked client.
        // It's the remaining part of the CDocker that doesn't belong to the CDocker's children.
        // The docker's view window is a child window of CDockClient.
        class CDockClient : public CWnd
        {
        public:
            CDockClient();
            virtual ~CDockClient() override {}

            void Draw3DBorder(const RECT& rect);
            void DrawCaption();
            void DrawCloseButton(CDC& drawDC);
            CRect GetCloseRect() const;
            const CString& GetCaption() const     { return m_caption; }
            CWnd& GetView() const                 { assert (m_pView); return *m_pView; }
            void SetDocker(CDocker* pDocker)      { m_pDocker = pDocker;}
            void SetCaption(LPCTSTR caption)      { m_caption = caption; }
            void SetCaptionColors(COLORREF foregnd1, COLORREF backgnd1, COLORREF foreGnd2, COLORREF backGnd2, COLORREF penColor);
            void SetView(CWnd& view);

        protected:
            virtual int     OnCreate(CREATESTRUCT&) override;
            virtual LRESULT OnLButtonDown(UINT msg, WPARAM wparam, LPARAM lparam);
            virtual LRESULT OnLButtonUp(UINT msg, WPARAM wparam, LPARAM lparam);
            virtual LRESULT OnMouseMove(UINT msg, WPARAM wparam, LPARAM lparam);
            virtual LRESULT OnNCCalcSize(UINT msg, WPARAM wparam, LPARAM lparam);
            virtual LRESULT OnNCHitTest(UINT msg, WPARAM wparam, LPARAM lparam);
            virtual LRESULT OnNCLButtonDblClk(UINT msg, WPARAM wparam, LPARAM lparam);
            virtual LRESULT OnNCLButtonDown(UINT msg, WPARAM wparam, LPARAM lparam);
            virtual LRESULT OnNCMouseLeave(UINT msg, WPARAM wparam, LPARAM lparam);
            virtual LRESULT OnNCMouseMove(UINT msg, WPARAM wparam, LPARAM lparam);
            virtual LRESULT OnNCPaint(UINT msg, WPARAM wparam, LPARAM lparam);
            virtual LRESULT OnWindowPosChanged(UINT msg, WPARAM wparam, LPARAM lparam);
            virtual void    PreRegisterClass(WNDCLASS& wc) override;
            virtual void    PreCreate(CREATESTRUCT& cs) override;
            LRESULT WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam) override;

        private:
            CDockClient(const CDockClient&) = delete;
            CDockClient& operator=(const CDockClient&) = delete;

            LRESULT MouseMove(UINT msg, WPARAM wparam, LPARAM lparam);
            void SendNotify(UINT messageID);

            CString m_caption;
            CPoint m_oldPoint;
            CDocker* m_pDocker;
            CWnd* m_pView;
            BOOL m_isClosePressed;
            BOOL m_isCaptionPressed;
            BOOL m_isTracking;
            COLORREF m_foregnd1;
            COLORREF m_backgnd1;
            COLORREF m_foregnd2;
            COLORREF m_backgnd2;
            COLORREF m_penColor;
        };

        // This nested class is used to indicate where a window could dock by
        // displaying a blue partially transparent window.
        class CDockHint : public CWnd
        {
        public:
            CDockHint();
            virtual ~CDockHint() override;

            RECT CalcHintRectContainer(CDocker* pDockTarget);
            RECT CalcHintRectInner(CDocker* pDockTarget, CDocker* pDockDrag, UINT dockSide);
            RECT CalcHintRectOuter(CDocker* pDockDrag, UINT dockSide);
            void DisplayHint(CDocker* pDockTarget, CDocker* pDockDrag, UINT dockSide);

        protected:
            virtual void PreCreate(CREATESTRUCT& cs) override;
            void PreRegisterClass(WNDCLASS& wc) override;

        private:
            CDockHint(const CDockHint&) = delete;
            CDockHint& operator=(const CDockHint&) = delete;

            CBrush m_brush;
            CBitmap m_bmBlueTint;
        };

        // This nested class is the base class for drawing dock targets.
        class CTarget : public CWnd
        {
        public:
            CTarget() {}
            virtual ~CTarget() override {}

        protected:
            virtual void OnDraw(CDC& dc) override;
            virtual void PreCreate(CREATESTRUCT& cs) override;

            CBitmap m_image;

        private:
            CTarget(const CTarget&) = delete;
            CTarget& operator=(const CTarget&) = delete;

        };

        // This nested class draws the a set of dock targets at the centre of
        // the docker. The dock within docker target is only enabled for a
        // dock container.
        class CTargetCentre : public CTarget
        {
        public:
            CTargetCentre();
            virtual ~CTargetCentre() override;

            BOOL CheckTarget(DragPos* pDragPos);
            BOOL IsOverContainer() const { return m_isOverContainer; }

        protected:
            virtual int OnCreate(CREATESTRUCT& cs) override;
            virtual void OnDraw(CDC& dc) override;

        private:
            CTargetCentre(const CTargetCentre&) = delete;
            CTargetCentre& operator=(const CTargetCentre&) = delete;

            BOOL m_isOverContainer;
            CDocker* m_pOldDockTarget;
        };

        // This nested class is draws the left dock target.
        class CTargetLeft : public CTarget
        {
        public:
            CTargetLeft();
            BOOL CheckTarget(DragPos* pDragPos);

        private:
            CTargetLeft(const CTargetLeft&) = delete;
            CTargetLeft& operator=(const CTargetLeft&) = delete;
        };

        // This nested class is draws the top dock target.
        class CTargetTop : public CTarget
        {
        public:
            CTargetTop();
            BOOL CheckTarget(DragPos* pDragPos);

        private:
            CTargetTop(const CTargetTop&) = delete;
            CTargetTop& operator=(const CTargetTop&) = delete;
        };

        // This nested class is draws the right dock target.
        class CTargetRight : public CTarget
        {
        public:
            CTargetRight();
            BOOL CheckTarget(DragPos* pDragPos);

        private:
            CTargetRight(const CTargetRight&) = delete;
            CTargetRight& operator=(const CTargetRight&) = delete;
        };

        // This nested class is draws the bottom dock target.
        class CTargetBottom : public CTarget
        {
        public:
            CTargetBottom();
            BOOL CheckTarget(DragPos* pDragPos);

        private:
            CTargetBottom(const CTargetBottom&) = delete;
            CTargetBottom& operator=(const CTargetBottom&) = delete;
        };

        // These classes can access private members of CDocker.
        friend class CTargetCentre;
        friend class CTargetLeft;
        friend class CTargetTop;
        friend class CTargetRight;
        friend class CTargetBottom;
        friend class CDockClient;
        friend class CDockContainer;

    public:
        CDocker();
        virtual ~CDocker();

        // Operations
        virtual CDocker* AddDockedChild(CDocker* pDocker, DWORD dockStyle, int dockSize, int dockID = 0);
        virtual CDocker* AddDockedChild(DockPtr docker, DWORD dockStyle, int dockSize, int dockID = 0);
        virtual CDocker* AddUndockedChild(CDocker* pDocker, DWORD dockStyle,
                                          int dockSize, const RECT& rc, int dockID = 0);
        virtual CDocker* AddUndockedChild(DockPtr docker, DWORD dockStyle, int dockSize, const RECT& rc, int dockID = 0);
        virtual void CloseAllDockers();
        virtual void Dock(CDocker* pDocker, UINT dockSide);
        virtual void DockInContainer(CDocker* pDocker, DWORD dockStyle, BOOL selectPage = TRUE);
        virtual void DpiUpdateDockerSizes();
        virtual CRect GetViewRect() const { return GetClientRect(); }
        virtual void Hide();
        virtual BOOL LoadContainerRegistrySettings(LPCTSTR registryKeyName);
        virtual BOOL LoadDockRegistrySettings(LPCTSTR registryKeyName);
        virtual void RecalcDockChildLayout(CRect& rc);
        virtual void RecalcDockLayout();
        virtual BOOL SaveDockRegistrySettings(LPCTSTR registryKeyName);
        virtual void SaveContainerRegistrySettings(CRegKey& dockKey, CDockContainer* pContainer, UINT& container);
        virtual void Undock(CPoint pt, BOOL showUndocked = TRUE);
        virtual void UndockContainer(CDockContainer* pContainer, CPoint pt, BOOL showUndocked);
        virtual void UndockContainerGroup();
        virtual BOOL VerifyDockers();

        // Accessors and mutators
        const std::vector<DockPtr>& GetAllDockChildren() const    {return GetDockAncestor()->m_allDockChildren;}
        const std::vector<CDocker*>& GetDockChildren() const      {return m_dockChildren;}
        const std::vector<CDocker*>& GetAllDockers()  const       {return m_allDockers;}

        CDocker* GetActiveDocker() const;
        CWnd*    GetActiveView() const;
        int GetBarWidth() const                     {return DpiScaleInt(GetDockBar().GetWidth());}
        const CString& GetCaption() const           {return GetDockClient().GetCaption();}
        CDockContainer* GetContainer() const;
        CDocker* GetDockAncestor() const;
        CDockBar& GetDockBar() const { return *m_pDockBar; }
        CDockClient& GetDockClient() const { return *m_pDockClient; }
        CDockHint& GetDockHint() const { return *m_pDockAncestor->m_pDockHint; }
        int GetDockID() const                       {return m_dockID;}
        CDocker* GetDockParent() const              {return m_pDockParent;}
        CDocker* GetDockFromID(int dockID) const;
        CDocker* GetDockFromView(CWnd* pView) const;
        int GetDockSize() const;
        DWORD GetDockStyle() const                  {return m_dockStyle;}
        CTabbedMDI* GetTabbedMDI() const;
        int GetTextHeight();
        CDocker* GetTopmostDocker() const;
        CWnd& GetView() const                       { return GetDockClient().GetView(); }
        BOOL IsChildOfDocker(HWND wnd) const;
        BOOL IsDocked() const;
        BOOL IsRelated(HWND wnd) const;
        BOOL IsUndocked() const;
        BOOL IsUndockable() const;
        void SetBarColor(COLORREF color) const {GetDockBar().SetColor(color);}
        void SetBarWidth(int width) const {GetDockBar().SetWidth(width);}
        void SetCaption(LPCTSTR caption);
        void SetCaptionColors(COLORREF foregnd1, COLORREF backgnd1, COLORREF foreGnd2,
                              COLORREF backGnd2, COLORREF penColor = RGB(160, 150, 140)) const;
        void SetCaptionHeight(int height);
        void SetDefaultCaptionHeight();
        void SetDockBar(CDockBar& dockBar) { m_pDockBar = &dockBar; }
        void SetDockClient(CDockClient& dockClient);
        void SetDockHint(CDockHint& dockHint) { m_pDockHint = &dockHint; }
        void SetDockStyle(DWORD dockStyle);
        void SetDockSize(int dockSize);
        void SetView(CWnd& view);

    protected:
        virtual DockPtr NewDockerFromID(int dockID);
        virtual void OnClose() override;
        virtual int  OnCreate(CREATESTRUCT& cs) override;
        virtual void OnDestroy() override;
        virtual LRESULT OnBarEnd(DragPos* pDragPos);
        virtual LRESULT OnBarMove(DragPos* pDragPos);
        virtual LRESULT OnBarStart(DragPos* pDragPos);
        virtual LRESULT OnDockEnd(DragPos* pDragPos);
        virtual LRESULT OnDockMove(DragPos* pDragPos);
        virtual LRESULT OnDockStart(DragPos* pDragPos);
        virtual LRESULT OnNotify(WPARAM wparam, LPARAM lparam) override;
        virtual void PreCreate(CREATESTRUCT& cs) override;
        virtual void PreRegisterClass(WNDCLASS& wc) override;

        // Not intended to be overwritten
        LRESULT WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam) override;

        // Message handlers
        virtual LRESULT OnActivate(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnDockActivated(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnDockDestroyed(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnDpiChanged(UINT, WPARAM, LPARAM);
        virtual LRESULT OnDpiChangedBeforeParent(UINT, WPARAM, LPARAM);
        virtual LRESULT OnExitSizeMove(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnGetDpiScaledSize(UINT, WPARAM, LPARAM);
        virtual LRESULT OnMouseActivate(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnNCLButtonDblClk(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnSettingChange(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnSize(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnSysColorChange(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnSysCommand(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnWindowPosChanged(UINT msg, WPARAM wparam, LPARAM lparam);

    private:
        CDocker(const CDocker&) = delete;
        CDocker& operator=(const CDocker&) = delete;
        std::vector <DockPtr> & GetAllChildren() const {return GetDockAncestor()->m_allDockChildren;}
        virtual CDocker* GetDockUnderDragPoint(POINT pt);
        void CheckAllTargets(DragPos* pDragPos) const;
        void CloseAllTargets() const;
        void DockOuter(CDocker* pDocker, DWORD dockStyle);
        void DrawAllCaptions() const;
        void ConvertToChild(HWND parent) const;
        void ConvertToPopup(const RECT& rc, BOOL showUndocked);
        int  GetMonitorDpi(HWND wnd);
        void MoveDockChildren(CDocker* pDockTarget);
        void PromoteFirstChild();
        void ResizeDockers(DragPos* pDragPos);
        CDocker* SeparateFromDock();
        void SendNotify(UINT messageID);
        void SetUndockPosition(CPoint pt, BOOL showUndocked);
        std::vector<CDocker*> SortDockers();
        static BOOL CALLBACK EnumWindowsProc(HWND top, LPARAM lparam);

        CDockBar        m_dockBar;
        CDockBar*       m_pDockBar;
        CDockClient     m_dockClient;
        CDockClient*    m_pDockClient;
        CDockHint       m_dockHint;
        CDockHint*      m_pDockHint;

        CTargetCentre   m_targetCentre;
        CTargetLeft     m_targetLeft;
        CTargetTop      m_targetTop;
        CTargetRight    m_targetRight;
        CPoint          m_oldPoint;
        CTargetBottom   m_targetBottom;
        CDocker*        m_pDockParent;
        CDocker*        m_pDockAncestor;

        std::vector <CDocker*> m_dockChildren;     // Docker's immediate children
        std::vector <DockPtr> m_allDockChildren;   // All descendants of the DockAncestor (only used by the DockAncestor)
        std::vector <CDocker*> m_allDockers;       // DockAncestor + all descendants (only used by the DockAncestor)

        CRect m_barRect;
        CRect m_childRect;
        BOOL m_isUndocking;
        BOOL m_isClosing;
        BOOL m_isDragging;
        int m_dockStartSize;
        int m_dockID;
        int m_ncHeight;
        int m_newDpi;
        int m_oldDpi;
        DWORD m_dockZone;
        DWORD m_dockStyle;
        HWND m_dockUnderPoint;
        CPoint m_dockPoint;

    }; // class CDocker

    struct DockInfo
    {
        DWORD dockStyle;
        int dockSize;
        int dockID;
        int dockParentID;
        bool isInAncestor;
        RECT rect;
    };

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

namespace Win32xx
{

    /////////////////////////////////////////////////////////////
    // Definitions for the CDockBar class nested within CDocker
    //
    inline CDocker::CDockBar::CDockBar() : m_pDocker(nullptr), m_dockBarWidth(4)
    {
        m_dragPos = {};
    }

    inline CDocker::CDockBar::~CDockBar()
    {
    }

    inline void CDocker::CDockBar::OnDraw(CDC& dc)
    {
        CRect rcClient = GetClientRect();
        dc.SelectObject(m_brBackground);
        dc.PatBlt(0, 0, rcClient.Width(), rcClient.Height(), PATCOPY);
    }

    inline LRESULT CDocker::CDockBar::OnLButtonDown(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        if (!(m_pDocker->GetDockStyle() & DS_NO_RESIZE))
        {
            SendNotify(UWN_BARSTART);
            SetCapture();
        }

        return FinalWindowProc(msg, wparam, lparam);
    }

    inline LRESULT CDocker::CDockBar::OnLButtonUp(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        if (!(m_pDocker->GetDockStyle() & DS_NO_RESIZE) && (GetCapture() == *this))
        {
            SendNotify(UWN_BAREND);
            ReleaseCapture();
        }

        return FinalWindowProc(msg, wparam, lparam);
    }

    inline LRESULT CDocker::CDockBar::OnMouseMove(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        if (!(m_pDocker->GetDockStyle() & DS_NO_RESIZE) && (GetCapture() == *this))
        {
            SendNotify(UWN_BARMOVE);
        }

        return FinalWindowProc(msg, wparam, lparam);
    }

    inline LRESULT CDocker::CDockBar::OnSetCursor(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        if (!(m_pDocker->GetDockStyle() & DS_NO_RESIZE))
        {
            HCURSOR cursor;
            DWORD side = GetDocker().GetDockStyle() & 0xF;
            if ((side == DS_DOCKED_LEFT) || (side == DS_DOCKED_RIGHT))
                cursor = GetApp()->LoadCursor(IDW_SPLITH);
            else
                cursor = GetApp()->LoadCursor(IDW_SPLITV);

            if (cursor) SetCursor(cursor);
            else TRACE("\n*** WARNING Missing cursor resource for slider bar. ***\n");

            return TRUE;
        }
        else
            SetCursor(LoadCursor(nullptr, IDC_ARROW));

        return FinalWindowProc(msg, wparam, lparam);
    }

    inline void CDocker::CDockBar::PreCreate(CREATESTRUCT& cs)
    {
        // Create a child window, initially hidden.
        cs.style = WS_CHILD;
    }

    inline void CDocker::CDockBar::PreRegisterClass(WNDCLASS& wc)
    {
        wc.lpszClassName = _T("Win32++ Bar");
        wc.hbrBackground = m_brBackground;
    }

    inline void CDocker::CDockBar::SendNotify(UINT messageID)
    {
        // Send a splitter bar notification to the parent.
        m_dragPos.hdr.code = messageID;
        m_dragPos.hdr.hwndFrom = GetHwnd();
        m_dragPos.pos = GetCursorPos();
        if (GetDocker().GetDockStyle() & DS_CLIENTEDGE)
            m_dragPos.pos.x += 1;

        m_dragPos.pDocker = m_pDocker;
        LPARAM lparam = reinterpret_cast<LPARAM>(&m_dragPos);
        GetParent().SendMessage(WM_NOTIFY, 0, lparam);
    }

    inline void CDocker::CDockBar::SetColor(COLORREF color)
    {
        // Useful colors:
        // GetSysColor(COLOR_BTNFACE)   // Default Gray
        // RGB(196, 215, 250)           // Default Blue

        m_brBackground.CreateSolidBrush(color);
    }

    inline LRESULT CDocker::CDockBar::WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        switch (msg)
        {
        case WM_SETCURSOR:      return OnSetCursor(msg, wparam, lparam);
        case WM_ERASEBKGND:     return 0;
        case WM_LBUTTONDOWN:    return OnLButtonDown(msg, wparam, lparam);
        case WM_LBUTTONUP:      return OnLButtonUp(msg, wparam, lparam);
        case WM_MOUSEMOVE:      return OnMouseMove(msg, wparam, lparam);
        }

        // Pass unhandled messages on for default processing.
        return CWnd::WndProcDefault(msg, wparam, lparam);
    }


    ////////////////////////////////////////////////////////////////
    // Definitions for the CDockClient class nested within CDocker
    //
    inline CDocker::CDockClient::CDockClient() : m_pDocker(nullptr), m_pView(nullptr), m_isClosePressed(FALSE),
                        m_isCaptionPressed(FALSE), m_isTracking(FALSE)
    {
        m_foregnd1 = RGB(32,32,32);
        m_backgnd1 = RGB(190,207,227);
        m_foregnd2 = GetSysColor(COLOR_BTNTEXT);
        m_backgnd2 = GetSysColor(COLOR_BTNFACE);
        m_penColor = RGB(160, 150, 140);
    }

    inline void CDocker::CDockClient::Draw3DBorder(const RECT& rect)
    {
        // Imitates the drawing of the WS_EX_CLIENTEDGE extended style.
        // This draws a 2 pixel border around the specified RECT.
        CWindowDC dc(*this);
        CRect rcw = rect;
        dc.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));
        dc.MoveTo(0, rcw.Height());
        dc.LineTo(0, 0);
        dc.LineTo(rcw.Width(), 0);
        dc.CreatePen(PS_SOLID,1, GetSysColor(COLOR_3DDKSHADOW));
        dc.MoveTo(1, rcw.Height()-2);
        dc.LineTo(1, 1);
        dc.LineTo(rcw.Width()-2, 1);
        dc.CreatePen(PS_SOLID,1, GetSysColor(COLOR_3DHILIGHT));
        dc.MoveTo(rcw.Width()-1, 0);
        dc.LineTo(rcw.Width()-1, rcw.Height()-1);
        dc.LineTo(0, rcw.Height()-1);
        dc.CreatePen(PS_SOLID,1, GetSysColor(COLOR_3DLIGHT));
        dc.MoveTo(rcw.Width()-2, 1);
        dc.LineTo(rcw.Width()-2, rcw.Height()-2);
        dc.LineTo(1, rcw.Height()-2);
    }

    inline void CDocker::CDockClient::DrawCaption()
    {
        assert(m_pDocker);
        if (!(m_pDocker->GetDockStyle() & DS_NO_CAPTION))
        {
            if (m_pDocker->IsWindow() && m_pDocker->IsUndockable())
            {
                // Acquire the DC for our NonClient painting.
                CWindowDC dc(*this);

                // Create and set up our memory DC.
                CRect rc = GetWindowRect();
                CMemDC memDC(dc);
                int rcAdjust = (GetExStyle() & WS_EX_CLIENTEDGE) ? 2 : 0;
                int width = std::max(rc.Width() - rcAdjust, 0);

                int height = m_pDocker->m_ncHeight + rcAdjust;
                memDC.CreateCompatibleBitmap(dc, width, height);

                // Set the font for the title.
                int dpi = GetWindowDpi(*this);
                NONCLIENTMETRICS info = GetNonClientMetrics();
                LOGFONT lf = info.lfStatusFont;
                lf.lfHeight = -MulDiv(9, dpi, POINTS_PER_INCH);
                memDC.CreateFontIndirect(lf);

                // Set the Colors.
                if (m_pDocker->GetActiveDocker() == m_pDocker)
                {
                    memDC.SetTextColor(m_foregnd1);
                    memDC.CreateSolidBrush(m_backgnd1);
                    memDC.SetBkColor(m_backgnd1);
                }
                else
                {
                    memDC.SetTextColor(m_foregnd2);
                    memDC.CreateSolidBrush(m_backgnd2);
                    memDC.SetBkColor(m_backgnd2);
                }

                // Draw the rectangle.
                memDC.CreatePen(PS_SOLID, 1, m_penColor);
                memDC.Rectangle(rcAdjust, rcAdjust, rc.Width() - rcAdjust, m_pDocker->m_ncHeight + rcAdjust);

                // Display the caption.
                int cxSmallIcon = ::GetSystemMetrics(SM_CXSMICON) * GetWindowDpi(*this) / GetWindowDpi(HWND_DESKTOP);
                int cx = (m_pDocker->GetDockStyle() & DS_NO_CLOSE) ? 0 : cxSmallIcon;
                CRect rcText(4 + rcAdjust, rcAdjust, rc.Width() - 4 - cx - rcAdjust, m_pDocker->m_ncHeight + rcAdjust);
                memDC.DrawText(m_caption, m_caption.GetLength(), rcText, DT_LEFT | DT_VCENTER |
                    DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

                // Draw the close button.
                if (!(m_pDocker->GetDockStyle() & DS_NO_CLOSE))
                    DrawCloseButton(memDC);

                // Draw the 3D border.
                if (GetExStyle() & WS_EX_CLIENTEDGE)
                    Draw3DBorder(rc);

                // Copy the Memory DC to the window's DC.
                dc.BitBlt(rcAdjust, rcAdjust, width, height, memDC, rcAdjust, rcAdjust, SRCCOPY);
            }
        }
    }

    inline void CDocker::CDockClient::DrawCloseButton(CDC& drawDC)
    {
        if (m_pDocker->IsUndockable() && !(m_pDocker->GetDockStyle() & DS_NO_CAPTION))
        {
            CDockContainer* pContainer = m_pDocker->GetContainer();
            if ((m_pDocker->IsDocked()) || ((m_pDocker == m_pDocker->GetDockAncestor()) && pContainer && pContainer->GetItemCount() > 0))
            {
                // Determine the close button's drawing position relative to the window.
                CRect rcClose = GetCloseRect();
                UINT uState = GetCloseRect().PtInRect(GetCursorPos()) ? m_isClosePressed && IsLeftButtonDown() ? 2U : 1U : 0U;
                VERIFY(ScreenToClient(rcClose));

                if (GetExStyle() & WS_EX_CLIENTEDGE)
                {
                    rcClose.OffsetRect(2, m_pDocker->m_ncHeight + 2);
                    if (GetWindowRect().Height() < (m_pDocker->m_ncHeight + 4))
                        rcClose.OffsetRect(-2, -2);
                }
                else
                    rcClose.OffsetRect(0, m_pDocker->m_ncHeight - 2);

                // Draw the outer highlight for the close button.
                if (!IsRectEmpty(&rcClose))
                {
                    // Use the Marlett font to draw special characters.
                    CFont marlett;
                    marlett.CreatePointFont(100, _T("Marlett"));
                    drawDC.SetBkMode(TRANSPARENT);
                    LOGFONT lf = DpiScaleLogfont(marlett.GetLogFont(), 10);
                    drawDC.CreateFontIndirect(lf);

                    COLORREF grey(RGB(232, 228, 220));
                    COLORREF black(RGB(0, 0, 0));
                    COLORREF white(RGB(255, 255, 255));

                    switch (uState)
                    {
                    case 0:
                    {
                        // Draw a grey box for the normal button using two special characters.
                        drawDC.SetTextColor(grey);
                        drawDC.TextOut(rcClose.left, rcClose.top, _T("\x63"), 1);
                        drawDC.TextOut(rcClose.left, rcClose.top, _T("\x64"), 1);
                        break;
                    }
                    case 1:
                    {
                        // Draw popped up button, black on right and bottom.
                        drawDC.SetTextColor(white);
                        drawDC.TextOut(rcClose.left, rcClose.top, _T("\x63"), 1);
                        drawDC.SetTextColor(black);
                        drawDC.TextOut(rcClose.left, rcClose.top, _T("\x64"), 1);
                        break;
                    }
                    case 2:
                    {
                        // Draw pressed button, black on left and top.
                        drawDC.SetTextColor(black);
                        drawDC.TextOut(rcClose.left, rcClose.top, _T("\x63"), 1);
                        drawDC.SetTextColor(white);
                        drawDC.TextOut(rcClose.left, rcClose.top, _T("\x64"), 1);
                        break;
                    }
                    }

                    // Draw the close button (a Marlett "r" looks like "X").
                    drawDC.SetTextColor(black);
                    drawDC.TextOut(rcClose.left, rcClose.top, _T("\x72"), 1);
                }
            }
        }
    }

    // Calculate the close rect position in screen co-ordinates.
    inline CRect CDocker::CDockClient::GetCloseRect() const
    {
        CRect rcClose;
        int gap = DpiScaleInt(2);
        CRect rc = GetWindowRect();
        int cx = GetSystemMetrics(SM_CXSMICON) * GetWindowDpi(*this) / GetWindowDpi(HWND_DESKTOP);
        int cy = GetSystemMetrics(SM_CYSMICON) * GetWindowDpi(*this) / GetWindowDpi(HWND_DESKTOP);

        rcClose.top = gap + rc.top + (m_pDocker->m_ncHeight - cy) / 2;
        rcClose.bottom = rc.top + cy;
        rcClose.right = rc.right - gap;
        rcClose.left = rcClose.right - cx;

        if (GetExStyle() & WS_EX_LAYOUTRTL)
        {
            rcClose.left = rc.left + gap;
            rcClose.right = rcClose.left + cx;
        }

        return rcClose;
    }

    inline LRESULT CDocker::CDockClient::MouseMove(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        if (!m_isTracking)
        {
            TRACKMOUSEEVENT TrackMouseEventStruct{};
            TrackMouseEventStruct.cbSize = sizeof(TrackMouseEventStruct);
            TrackMouseEventStruct.dwFlags = TME_LEAVE | TME_NONCLIENT;
            TrackMouseEventStruct.hwndTrack = *this;
            _TrackMouseEvent(&TrackMouseEventStruct);
            m_isTracking = TRUE;
        }

        if ((m_pDocker != nullptr) && !(m_pDocker->GetDockStyle() & DS_NO_CAPTION))
        {
            if (m_pDocker->IsDocked())
            {
                // Discard phantom mouse move messages.
                if ((m_oldPoint.x == GET_X_LPARAM(lparam)) && (m_oldPoint.y == GET_Y_LPARAM(lparam)))
                    return 0;

                if (IsLeftButtonDown() && (wparam == HTCAPTION) && (m_isCaptionPressed))
                {
                    assert(m_pDocker);
                    if (!(m_pDocker->GetDockStyle() & DS_NO_UNDOCK))
                        m_pDocker->Undock(GetCursorPos());
                }

                // Update the close button.
                if ((m_pDocker != nullptr) && !(m_pDocker->GetDockStyle() & DS_NO_CLOSE))
                {
                    CWindowDC dc(*this);
                    DrawCloseButton(dc);
                }
            }
            else if (m_pDocker->IsUndockable() && !(m_pDocker->GetDockStyle() & DS_NO_UNDOCK))
            {
                // Discard phantom mouse move messages.
                if ((m_oldPoint.x == GET_X_LPARAM(lparam)) && (m_oldPoint.y == GET_Y_LPARAM(lparam)))
                    return 0;

                if (IsLeftButtonDown() && (wparam == HTCAPTION) && (m_isCaptionPressed))
                    m_pDocker->GetDockAncestor()->UndockContainerGroup();

                // Update the close button.
                if ((m_pDocker != nullptr) && !(m_pDocker->GetDockStyle() & DS_NO_CLOSE))
                {
                    CWindowDC dc(*this);
                    DrawCloseButton(dc);
                }
            }

            m_isCaptionPressed = FALSE;
        }

        return FinalWindowProc(msg, wparam, lparam);
    }

    inline int CDocker::CDockClient::OnCreate(CREATESTRUCT&)
    {
        assert(m_pDocker);
        assert(m_pDocker->IsWindow());

        // Save the initial DPI when the docker is created.
        m_pDocker->m_oldDpi = GetWindowDpi(*m_pDocker);
        return 0;
    }

    inline LRESULT CDocker::CDockClient::OnLButtonDown(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        m_isClosePressed = FALSE;
        ReleaseCapture();
        CWindowDC dc(*this);
        DrawCloseButton(dc);

        return FinalWindowProc(msg, wparam, lparam);
    }

    inline LRESULT CDocker::CDockClient::OnLButtonUp(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        ReleaseCapture();

        if ((m_pDocker != nullptr) && !(m_pDocker->GetDockStyle() & DS_NO_CLOSE))
        {
            CWindowDC dc(*this);
            DrawCloseButton(dc);
            dc.Destroy();  // Destroy the dc before destroying its window.

            if ((m_pDocker != nullptr) && !(m_pDocker->GetDockStyle() & (DS_NO_CAPTION|DS_NO_CLOSE)))
            {
                m_isCaptionPressed = FALSE;
                if (m_isClosePressed && GetCloseRect().PtInRect(GetCursorPos()))
                {
                    // Destroy the docker.
                    if (m_pDocker->GetContainer())
                    {
                        CDockContainer* pContainer = m_pDocker->GetContainer()->GetActiveContainer();
                        if (pContainer)
                        {
                            CDockContainer* pParentC = pContainer->GetContainerParent();
                            CDocker* pDocker = m_pDocker->GetDockFromView(pContainer);
                            int tab = pParentC->GetContainerIndex(pContainer);

                            assert(pDocker);
                            if (pDocker)
                                pDocker->SendMessage(WM_CLOSE);

                            if (pContainer != pParentC)
                            {
                                if (pParentC->GetItemCount() == 0)
                                    pParentC->GetDocker()->RecalcDockLayout();
                                else
                                {
                                    pParentC->SelectPage(std::max(tab - 1, 0));
                                    pParentC->RecalcLayout();
                                }
                            }

                            if (m_pDocker->IsWindow())
                                m_pDocker->RecalcDockLayout();
                        }
                    }
                    else
                    {
                        m_pDocker->SendMessage(WM_CLOSE);
                    }
                }
            }
        }

        return CWnd::WndProcDefault(msg, wparam, lparam);
    }

    inline LRESULT CDocker::CDockClient::OnMouseMove(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        return MouseMove(msg, wparam, lparam);
    }

    // Sets the non-client area (and hence sets the client area).
    // This function modifies lparam.
    inline LRESULT CDocker::CDockClient::OnNCCalcSize(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        if ((m_pDocker != nullptr) && !(m_pDocker->GetDockStyle() & DS_NO_CAPTION))
        {
            if (m_pDocker->IsUndockable())
            {
                LPRECT rc = (LPRECT)lparam;
                rc->top += m_pDocker->m_ncHeight;
            }
        }

        return CWnd::WndProcDefault(msg, wparam, lparam);
    }

    // Identify which part of the non-client area the cursor is over.
    inline LRESULT CDocker::CDockClient::OnNCHitTest(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        if ((m_pDocker != nullptr) && !(m_pDocker->GetDockStyle() & DS_NO_CAPTION))
        {
            if (m_pDocker->IsUndockable())
            {
                CPoint pt(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));

                // Indicate if the point is in the close button.
                if (GetCloseRect().PtInRect(pt))
                    return HTCLOSE;

                VERIFY(ScreenToClient(pt));

                // Indicate if the point is in the caption.
                if (pt.y < 0)
                    return HTCAPTION;
            }
        }
        return CWnd::WndProcDefault(msg, wparam, lparam);
    }

    inline LRESULT CDocker::CDockClient::OnNCLButtonDblClk(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        if ((m_pDocker != nullptr) && !(m_pDocker->GetDockStyle() & DS_NO_CAPTION))
        {
            if ((HTCLOSE == wparam) && !(m_pDocker->GetDockStyle() & DS_NO_CLOSE))
            {
                m_isClosePressed = TRUE;
                SetCapture();
            }

            m_isCaptionPressed = TRUE;
            m_oldPoint.x = GET_X_LPARAM(lparam);
            m_oldPoint.y = GET_Y_LPARAM(lparam);
            if (m_pDocker->IsUndockable())
            {
                // Give the view window focus unless its child already has it.
                if (!GetView().IsChild(GetFocus()))
                    m_pDocker->GetView().SetFocus();

                // Update the close button.
                if (!(m_pDocker->GetDockStyle() & DS_NO_CLOSE))
                {
                    CWindowDC dc(*this);
                    DrawCloseButton(dc);
                }

                return 0;
            }
        }
        return CWnd::WndProcDefault(msg, wparam, lparam);
    }

    inline LRESULT CDocker::CDockClient::OnNCLButtonDown(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        if ((m_pDocker != nullptr) && !(m_pDocker->GetDockStyle() & DS_NO_CAPTION))
        {
            if ((HTCLOSE == wparam) && !(m_pDocker->GetDockStyle() & DS_NO_CLOSE))
            {
                m_isClosePressed = TRUE;
                SetCapture();
            }

            m_isCaptionPressed = TRUE;
            m_oldPoint.x = GET_X_LPARAM(lparam);
            m_oldPoint.y = GET_Y_LPARAM(lparam);
            if (m_pDocker->IsUndockable())
            {
                // Give the view window focus unless its child already has it.
                if (!GetView().IsChild(GetFocus()))
                    m_pDocker->GetView().SetFocus();

                // Update the close button.
                if (!(m_pDocker->GetDockStyle() & DS_NO_CLOSE))
                {
                    CWindowDC dc(*this);
                    DrawCloseButton(dc);
                }

                return 0;
            }
        }

        return CWnd::WndProcDefault(msg, wparam, lparam);
    }

    inline LRESULT CDocker::CDockClient::OnNCMouseLeave(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        m_isTracking = FALSE;
        CWindowDC dc(*this);
        if ((0 != m_pDocker) && !(m_pDocker->GetDockStyle() & (DS_NO_CAPTION|DS_NO_CLOSE)) && m_pDocker->IsUndockable())
            DrawCloseButton(dc);

        m_isTracking = FALSE;

        return FinalWindowProc(msg, wparam, lparam);
    }

    inline LRESULT CDocker::CDockClient::OnNCMouseMove(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        return MouseMove(msg, wparam, lparam);
    }

    inline LRESULT CDocker::CDockClient::OnNCPaint(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        if ((m_pDocker != nullptr) && !(m_pDocker->GetDockStyle() & DS_NO_CAPTION))
        {
            if (m_pDocker->IsUndockable())
            {
                DefWindowProc(WM_NCPAINT, wparam, lparam);
                DrawCaption();
                return 0;
            }
        }
        return FinalWindowProc(msg, wparam, lparam);
    }

    // Called after the window is resized.
    inline LRESULT CDocker::CDockClient::OnWindowPosChanged(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        // Reposition the View window to cover the DockClient's client area.
        CRect rc = GetClientRect();
        VERIFY(GetView().SetWindowPos(HWND_TOP, rc, SWP_SHOWWINDOW));

        return FinalWindowProc(msg, wparam, lparam);
    }

    inline void CDocker::CDockClient::PreRegisterClass(WNDCLASS& wc)
    {
        wc.lpszClassName = _T("Win32++ DockClient");
        wc.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
    }

    inline void CDocker::CDockClient::PreCreate(CREATESTRUCT& cs)
    {
        assert(m_pDocker);
        if (!m_pDocker) return;

        DWORD style = m_pDocker->GetDockStyle();
        if (style & DS_CLIENTEDGE)
            cs.dwExStyle = WS_EX_CLIENTEDGE;

        if (m_pDocker->GetExStyle() & WS_EX_LAYOUTRTL)
            cs.dwExStyle |= WS_EX_LAYOUTRTL;
    }

    // Sends custom notification messages to the parent window.
    inline void CDocker::CDockClient::SendNotify(UINT messageID)
    {
        // Fill the DragPos structure with data.
        DragPos dragPos{};
        dragPos.hdr.code = messageID;
        dragPos.hdr.hwndFrom = GetHwnd();
        dragPos.pos = GetCursorPos();
        dragPos.pDocker = m_pDocker;

        // Send a DragPos notification to the docker.
        LPARAM lparam = reinterpret_cast<LPARAM>(&dragPos);
        GetParent().SendMessage(WM_NOTIFY, 0, lparam);
    }

    // Sets the caption's foreground and background colors.
    // foregnd1 specifies the foreground color(focused).
    // backgnd1 specifies the background color(focused).
    // foregnd2 specifies the foreground color(not focused).
    // backgnd2 specifies the background color(not focused).
    // penColor specifies the pen color used for drawing the outline.
    inline void CDocker::CDockClient::SetCaptionColors(COLORREF foregnd1, COLORREF backgnd1, COLORREF foregnd2, COLORREF backgnd2, COLORREF penColor)
    {
        m_foregnd1 = foregnd1;
        m_backgnd1 = backgnd1;
        m_foregnd2 = foregnd2;
        m_backgnd2 = backgnd2;
        m_penColor = penColor;
    }

    inline void CDocker::CDockClient::SetView(CWnd& view)
    {
        if (m_pView != &view)
        {
            // Hide the existing view window (if any).
            if (m_pView && m_pView->IsWindow())
            {
                // We can't change docker view if it is a DockContainer.
                // Use CDockContainer::SetView to change the DockContainer's view instead.
                assert(m_pView->SendMessage(UWM_GETCDOCKCONTAINER) == 0);

                m_pView->ShowWindow(SW_HIDE);
            }

            // Assign the view window.
            m_pView = &view;

            if (IsWindow())
            {
                // The docker is already created, so create and position the new view too.

                if (!GetView().IsWindow())
                    GetView().Create(*this);
                else
                {
                    GetView().SetParent(*this);
                }

                CRect rc = GetClientRect();
                VERIFY(GetView().SetWindowPos(HWND_TOP, rc, SWP_SHOWWINDOW));
            }
        }
    }

    inline LRESULT CDocker::CDockClient::WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        switch (msg)
        {
        case WM_LBUTTONUP:          return OnLButtonUp(msg, wparam, lparam);
        case WM_MOUSEMOVE:          return OnMouseMove(msg, wparam, lparam);
        case WM_NCCALCSIZE:         return OnNCCalcSize(msg, wparam, lparam);
        case WM_NCHITTEST:          return OnNCHitTest(msg, wparam, lparam);
        case WM_NCLBUTTONDBLCLK:    return OnNCLButtonDblClk(msg, wparam, lparam);
        case WM_NCLBUTTONDOWN:      return OnNCLButtonDown(msg, wparam, lparam);
        case WM_NCMOUSEMOVE:        return OnNCMouseMove(msg, wparam, lparam);
        case WM_NCPAINT:            return OnNCPaint(msg, wparam, lparam);
        case WM_NCMOUSELEAVE:       return OnNCMouseLeave(msg, wparam, lparam);
        case WM_NOTIFY:
        {
            // Perform default handling for WM_NOTIFY.
            LRESULT result = CWnd::WndProcDefault(msg, wparam, lparam);

            // Also forward WM_NOTIFY to the docker.
            if (result == 0)
                result = m_pDocker->SendMessage(msg, wparam, lparam);

            return result;
        }
        case WM_WINDOWPOSCHANGED:   return OnWindowPosChanged(msg, wparam, lparam);
        }

        return CWnd::WndProcDefault(msg, wparam, lparam);
    }


    //////////////////////////////////////////////////////////////
    // Definitions for the CDockHint class nested within CDocker
    //
    inline CDocker::CDockHint::CDockHint()
    {
        m_brush.CreateSolidBrush(RGB(0, 150, 255));
    }

    inline CDocker::CDockHint::~CDockHint()
    {
    }

    inline RECT CDocker::CDockHint::CalcHintRectContainer(CDocker* pDockTarget)
    {
        // Calculate the hint window's position for container docking.
        CRect rcHint = pDockTarget->GetDockClient().GetWindowRect();
        if (pDockTarget->GetDockClient().GetExStyle() & WS_EX_CLIENTEDGE)
            rcHint.InflateRect(-2, -2);
        VERIFY(pDockTarget->ScreenToClient(rcHint));

        return rcHint;
    }

    inline RECT CDocker::CDockHint::CalcHintRectInner(CDocker* pDockTarget, CDocker* pDockDrag, UINT dockSide)
    {
        assert(pDockTarget);
        assert(pDockDrag);

        if (!pDockTarget || !pDockDrag) return CRect(0, 0, 0, 0);

        bool isRTL = false;
        isRTL = ((pDockTarget->GetExStyle() & WS_EX_LAYOUTRTL)) != 0;

        // Calculate the hint window's position for inner docking.
        CDockClient* pDockClient = &pDockTarget->GetDockClient();
        CRect rcHint = pDockClient->GetWindowRect();

        if (pDockClient->GetExStyle() & WS_EX_CLIENTEDGE)
            rcHint.InflateRect(-2, -2);
        VERIFY(pDockTarget->ScreenToClient(rcHint));

        int Width;
        CRect rcDockDrag = pDockDrag->GetWindowRect();
        CRect rcDockTarget = pDockClient->GetWindowRect();
        if ((dockSide  == DS_DOCKED_LEFT) || (dockSide  == DS_DOCKED_RIGHT))
        {
            Width = rcDockDrag.Width();
            if (Width >= (rcDockTarget.Width() - pDockDrag->GetBarWidth()))
                Width = std::max(rcDockTarget.Width()/2 - pDockDrag->GetBarWidth(), pDockDrag->GetBarWidth());
        }
        else
        {
            Width = rcDockDrag.Height();
            if (Width >= (rcDockTarget.Height() - pDockDrag->GetBarWidth()))
                Width = std::max(rcDockTarget.Height()/2 - pDockDrag->GetBarWidth(), pDockDrag->GetBarWidth());
        }
        switch (dockSide)
        {
        case DS_DOCKED_LEFT:
            if (isRTL)
                rcHint.left = rcHint.right - Width;
            else
                rcHint.right = rcHint.left + Width;

            break;
        case DS_DOCKED_RIGHT:
            if (isRTL)
                rcHint.right = rcHint.left + Width;
            else
                rcHint.left = rcHint.right - Width;

            break;
        case DS_DOCKED_TOP:
            rcHint.bottom = rcHint.top + Width;
            break;
        case DS_DOCKED_BOTTOM:
            rcHint.top = rcHint.bottom - Width;
            break;
        }

        return rcHint;
    }

    // Calculate the hint window's position for outer docking.
    inline RECT CDocker::CDockHint::CalcHintRectOuter(CDocker* pDockDrag, UINT dockSide)
    {
        assert(pDockDrag);
        if (!pDockDrag) return CRect(0, 0, 0, 0);

        CDocker* pDockTarget = pDockDrag->GetDockAncestor();
        CRect rcHint = pDockTarget->GetViewRect();
        if (pDockTarget->GetDockClient().GetExStyle() & WS_EX_CLIENTEDGE)
            rcHint.InflateRect(-2, -2);

        CRect rcDockDrag = pDockDrag->GetWindowRect();
        int barWidth = pDockDrag->GetBarWidth();
        CRect rcDockClient = pDockTarget->GetDockClient().GetClientRect();

        // Limit the docked size to half the parent's size if it won't fit inside parent.
        int width;
        if ((dockSide == DS_DOCKED_LEFTMOST) || (dockSide  == DS_DOCKED_RIGHTMOST))
        {
            width = rcDockDrag.Width();
            if (width >= rcDockClient.Width() - barWidth)
                width = std::max(rcDockClient.Width()/2 - barWidth, barWidth);
        }
        else
        {
            width = rcDockDrag.Height();
            if (width >= rcDockClient.Height() - barWidth)
                width = std::max(rcDockClient.Height()/2 - barWidth, barWidth);
        }

        bool isRTL = false;
        isRTL = ((pDockTarget->GetExStyle() & WS_EX_LAYOUTRTL)) != 0;

        // Adjust the hint rect for the dock side and right to left (RTL).
        switch (dockSide)
        {
        case DS_DOCKED_LEFTMOST:
            if (isRTL)
                rcHint.left = rcHint.right - width;
            else
                rcHint.right = rcHint.left + width;
            break;
        case DS_DOCKED_RIGHTMOST:
            if (isRTL)
                rcHint.right = rcHint.left + width;
            else
                rcHint.left = rcHint.right - width;
            break;
        case DS_DOCKED_TOPMOST:
            rcHint.bottom = rcHint.top + width;
            break;
        case DS_DOCKED_BOTTOMMOST:
            rcHint.top = rcHint.bottom - width;
            break;
        }

        return rcHint;
    }

    inline void CDocker::CDockHint::DisplayHint(CDocker* pDockTarget, CDocker* pDockDrag, UINT dockSide)
    {
        assert(pDockTarget);
        assert(pDockDrag);

        // Retrieve the hint rect.
        CRect rcHint;
        if (dockSide & 0xF)
            rcHint = CalcHintRectInner(pDockTarget, pDockDrag, dockSide);
        else if (dockSide & 0xF0000)
            rcHint = CalcHintRectOuter(pDockDrag, dockSide);
        else if (dockSide & DS_DOCKED_CONTAINER)
            rcHint = CalcHintRectContainer(pDockTarget);
        else
            return;

        // Create the dock hint window if required.
        if (!IsWindow())
            Create();

        // Adjust hint shape for container in container docking using a region.
        if ((dockSide & DS_DOCKED_CONTAINER) && rcHint.Height() > 50)
        {
            CDockContainer* pDragged = pDockDrag->GetContainer();
            CDockContainer* pTarget = pDockTarget->GetContainer();
            assert(pTarget);

            if (pDragged != nullptr)
            {
                // Create the region.
                CRgn rgn;
                int gap = DpiScaleInt(8);
                int tabHeight = pDragged->GetTabHeight();
                CSize imageSize = pDragged->GetImages().GetIconSize();
                CSize textSize1 = pDragged->GetMaxTabTextSize();
                CSize textSize2 = pTarget->GetMaxTabTextSize();
                int tabWidth = imageSize.cx + std::max(textSize1.cx, textSize2.cx) + gap;
                rgn.CreateRectRgn(0, 0, rcHint.Width(), rcHint.Height() - tabHeight);
                assert(rgn.GetHandle());
                CRgn rgn2;
                gap = DpiScaleInt(5);
                rgn2.CreateRectRgn(gap, rcHint.Height() - tabHeight, tabWidth, rcHint.Height());
                rgn.CombineRgn(rgn2, RGN_OR);

                // Assign the region.
                SetWindowRgn(rgn, TRUE);
            }
        }
        else
            // Remove any assigned region.
            SetWindowRgn(nullptr, TRUE);

        // Position the hint window.
        VERIFY(pDockTarget->ClientToScreen(rcHint));
        VERIFY(SetWindowPos(HWND_TOP, rcHint, SWP_SHOWWINDOW));

        // Call SetLayeredWindowAttributes to specify the hint window's transparency.
        VERIFY(SetLayeredWindowAttributes(*this, 0, 92, LWA_ALPHA));
    }

    inline void CDocker::CDockHint::PreCreate(CREATESTRUCT& cs)
    {
        // WS_POPUP creates a window without a caption.
        cs.style = WS_POPUP;

        // WS_EX_TOOLWINDOW prevents the window from being displayed on the taskbar.
        // WS_EX_LAYERED makes the window layered for transparency.
        cs.dwExStyle = WS_EX_LAYERED | WS_EX_TOOLWINDOW;
    }

    inline void CDocker::CDockHint::PreRegisterClass(WNDCLASS& wc)
    {
        wc.lpszClassName = _T("Win32++ DockHint");

        // Assign the hint window's color.
        wc.hbrBackground = m_brush;
    }


    ////////////////////////////////////////////////////////////////
    // Definitions for the CTarget class nested within CDocker.
    // CTarget is the base class for a number of CTargetXXX classes.

    inline void CDocker::CTarget::OnDraw(CDC& dc)
    {
        if (m_image.GetHandle() != nullptr)
        {
            CBitmap image = DpiScaleUpBitmap(m_image);
            CSize imageSize = image.GetSize();
            CRect rc = GetClientRect();

            // Draw the dock target.
            // Support dock targets split across different monitors.
            CMemDC memDC(dc);
            memDC.CreateCompatibleBitmap(dc, imageSize.cx, imageSize.cy);
            memDC.DrawBitmap(0, 0, imageSize.cx, imageSize.cy, image, RGB(255, 0, 255));
            dc.StretchBlt(0, 0, rc.Width(), rc.Height(), memDC, 0, 0, imageSize.cx, imageSize.cy, SRCCOPY);
        }
        else
            TRACE("\n*** WARNING: Missing docking resource. ***\n");
    }

    inline void CDocker::CTarget::PreCreate(CREATESTRUCT& cs)
    {
        cs.style = WS_POPUP;
        cs.dwExStyle = WS_EX_TOPMOST | WS_EX_TOOLWINDOW;
        cs.lpszClass = _T("Win32++ DockTargeting");
    }


    /////////////////////////////////////////////////////////////////
    // Definitions for the CTargetBottom class nested within CDocker.
    //

    // Constructor.
    inline CDocker::CTargetBottom::CTargetBottom()
    {
        m_image.LoadBitmap(IDW_SDBOTTOM);
    }

    inline BOOL CDocker::CTargetBottom::CheckTarget(DragPos* pDragPos)
    {
        CDocker* pDockDrag = pDragPos->pDocker;
        assert(pDockDrag);
        if (!pDockDrag) return FALSE;

        CPoint pt = pDragPos->pos;
        CDocker* pDockTarget = pDockDrag->GetDockUnderDragPoint(pt)->GetTopmostDocker();
        if (pDockTarget != pDockDrag->GetDockAncestor())
        {
            Destroy();
            return FALSE;
        }

        if (pDockTarget->GetDockStyle() & DS_NO_DOCKCHILD_BOTTOM)
            return FALSE;

        CBitmap image = pDockTarget->DpiScaleUpBitmap(CBitmap(IDW_SDBOTTOM));
        int cxImage = image.GetSize().cx;
        int cyImage = image.GetSize().cy;

        if (!IsWindow())
        {
            Create();
            CRect rc = pDockTarget->GetViewRect();
            VERIFY(pDockTarget->ClientToScreen(rc));
            int xMid = rc.left + (rc.Width() - cxImage) / 2;
            VERIFY(SetWindowPos(HWND_TOPMOST, xMid, rc.bottom - DpiScaleInt(8) - cyImage, cxImage, cyImage, SWP_NOACTIVATE | SWP_SHOWWINDOW));
        }

        CRect rcBottom(0, 0, cxImage, cyImage);
        VERIFY(ScreenToClient(pt));

        // Test if our cursor is in one of the docking zones.
        if (rcBottom.PtInRect(pt))
        {
            pDockTarget->GetDockHint().DisplayHint(pDockTarget, pDockDrag, DS_DOCKED_BOTTOMMOST);
            pDockDrag->m_dockZone = DS_DOCKED_BOTTOMMOST;
            return TRUE;
        }

        return FALSE;
    }


    ////////////////////////////////////////////////////////////////
    // Definitions for the CTargetCentre class nested within CDocker
    //

    // Constructor.
    inline CDocker::CTargetCentre::CTargetCentre() : m_isOverContainer(FALSE), m_pOldDockTarget(0)
    {
        m_image.LoadBitmap(IDW_SDCENTER);
    }

    inline CDocker::CTargetCentre::~CTargetCentre()
    {
    }

    inline BOOL CDocker::CTargetCentre::CheckTarget(DragPos* pDragPos)
    {
        CDocker* pDockDrag = pDragPos->pDocker;
        assert(::SendMessage(*pDockDrag, UWM_GETCDOCKER, 0, 0));

        CDocker* pDockTarget = pDockDrag->GetDockUnderDragPoint(pDragPos->pos);
        if (pDockTarget == nullptr)
            return FALSE;

        m_image = pDockTarget->DpiScaleUpBitmap(CBitmap(IDW_SDCENTER));
        CSize imageSize = m_image.GetSize();

        if (!IsWindow())
            Create();

        m_isOverContainer = pDockTarget->GetView().SendMessage(UWM_GETCDOCKCONTAINER) ? TRUE : FALSE;

        // Redraw the target if the dock target changes.
        if (m_pOldDockTarget != pDockTarget)    Invalidate();
        m_pOldDockTarget = pDockTarget;

        int cxImage = imageSize.cx;
        int cyImage = imageSize.cy;

        CRect rcTarget = pDockTarget->GetDockClient().GetWindowRect();
        int xMid = rcTarget.left + (rcTarget.Width() - cxImage) / 2;
        int yMid = rcTarget.top + (rcTarget.Height() - cyImage) / 2;
        VERIFY(SetWindowPos(HWND_TOPMOST, xMid, yMid, cxImage, cyImage, SWP_NOACTIVATE | SWP_SHOWWINDOW));

        // The IDW_SDCENTER bitmap should be square.
        int p1 = cxImage / 3 - 1;
        int p2 = (2 * cxImage) / 3 - 1;
        int p3 = cxImage - 1;

        // Create the docking zone rectangles.
        CPoint pt = pDragPos->pos;
        VERIFY(ScreenToClient(pt));
        CRect rcLeft(0, p1, p1, p2);
        CRect rcTop(p1, 0, p2, p1);
        CRect rcRight(p2 + 1, p1, p3, p2 + 1);
        CRect rcBottom(p1, p2 + 1, p2 + 1, p3);
        CRect rcMiddle(p1 + 1, p1 + 1, p2, p2);

        // Test if our cursor is in one of the docking zones.
        if ((rcLeft.PtInRect(pt)) && !(pDockTarget->GetDockStyle() & DS_NO_DOCKCHILD_LEFT))
        {
            pDockTarget->GetDockHint().DisplayHint(pDockTarget, pDockDrag, DS_DOCKED_LEFT);
            pDockDrag->m_dockZone = DS_DOCKED_LEFT;
            return TRUE;
        }
        else if ((rcTop.PtInRect(pt)) && !(pDockTarget->GetDockStyle() & DS_NO_DOCKCHILD_TOP))
        {
            pDockTarget->GetDockHint().DisplayHint(pDockTarget, pDockDrag, DS_DOCKED_TOP);
            pDockDrag->m_dockZone = DS_DOCKED_TOP;
            return TRUE;
        }
        else if ((rcRight.PtInRect(pt)) && !(pDockTarget->GetDockStyle() & DS_NO_DOCKCHILD_RIGHT))
        {
            pDockTarget->GetDockHint().DisplayHint(pDockTarget, pDockDrag, DS_DOCKED_RIGHT);
            pDockDrag->m_dockZone = DS_DOCKED_RIGHT;
            return TRUE;
        }
        else if ((rcBottom.PtInRect(pt)) && !(pDockTarget->GetDockStyle() & DS_NO_DOCKCHILD_BOTTOM))
        {
            pDockTarget->GetDockHint().DisplayHint(pDockTarget, pDockDrag, DS_DOCKED_BOTTOM);
            pDockDrag->m_dockZone = DS_DOCKED_BOTTOM;
            return TRUE;
        }
        else if ((rcMiddle.PtInRect(pt)) && (IsOverContainer()))
        {
            pDockTarget->GetDockHint().DisplayHint(pDockTarget, pDockDrag, DS_DOCKED_CONTAINER);
            pDockDrag->m_dockZone = DS_DOCKED_CONTAINER;
            return TRUE;
        }
        else
            return FALSE;
    }

    inline int CDocker::CTargetCentre::OnCreate(CREATESTRUCT&)
    {
        // Calculate the points for the region.
        int side = m_image.GetSize().cx;
        int p1 = side / 4;
        int p2 = side / 3;
        int p3 = 2 * p2 - 1;
        int p4 = p1 * 3;
        int p5 = side;

        // Use a region to create an irregularly shaped window.
        POINT ptArray[16] = { {0, p2},  {p1, p2}, {p2, p1}, {p2, 0},
                              {p3, 0},  {p3, p1}, {p4, p2}, {p5, p2},
                              {p5, p3}, {p4, p3}, {p3, p4}, {p3, p5},
                              {p2, p5}, {p2, p4}, {p1, p3}, {0, p3} };

        CRgn rgnPoly;
        rgnPoly.CreatePolygonRgn(ptArray, 16, WINDING);
        SetWindowRgn(rgnPoly, FALSE);
        return 0;
    }

    inline void CDocker::CTargetCentre::OnDraw(CDC& dc)
    {
        if (m_image.GetHandle() != nullptr)
        {
            // Load the target bitmaps.
            CBitmap bmLeft = DpiScaleUpBitmap(CBitmap(IDW_SDLEFT));
            CBitmap bmRight = DpiScaleUpBitmap(CBitmap(IDW_SDRIGHT));
            CBitmap bmTop = DpiScaleUpBitmap(CBitmap(IDW_SDTOP));
            CBitmap bmBottom = DpiScaleUpBitmap(CBitmap(IDW_SDBOTTOM));

            // Gray out invalid dock targets.
            DWORD style = m_pOldDockTarget->GetDockStyle();
            if (style & DS_NO_DOCKCHILD_LEFT)  bmLeft.TintBitmap(150, 150, 150);
            if (style & DS_NO_DOCKCHILD_TOP)   bmTop.TintBitmap(150, 150, 150);
            if (style & DS_NO_DOCKCHILD_RIGHT) bmRight.TintBitmap(150, 150, 150);
            if (style & DS_NO_DOCKCHILD_BOTTOM) bmBottom.TintBitmap(150, 150, 150);

            // Get the dock target sizes.
            CSize szBig = m_image.GetSize();
            CSize szLeft = bmLeft.GetSize();
            CSize szTop = bmTop.GetSize();
            CSize szRight = bmRight.GetSize();
            CSize szBottom = bmBottom.GetSize();

            // Draw the dock targets.
            // Support dock targets split across different monitors.
            CRect rc = GetClientRect();
            CMemDC memDC(dc);
            memDC.CreateCompatibleBitmap(dc, szBig.cx, szBig.cy);

            memDC.DrawBitmap(0, 0, szBig.cx, szBig.cy, m_image, RGB(255, 0, 255));
            int midleft = (szBig.cy - szLeft.cy) / 2;
            int midright = szBig.cx - szRight.cx;
            memDC.DrawBitmap(0, midleft, szLeft.cx, szLeft.cy, bmLeft, RGB(255, 0, 255));
            memDC.DrawBitmap(midleft, 0, szTop.cx, szTop.cy, bmTop, RGB(255, 0, 255));
            memDC.DrawBitmap(midright, midleft, szRight.cx, szRight.cy, bmRight, RGB(255, 0, 255));
            memDC.DrawBitmap(midleft, midright, szBottom.cx, szBottom.cy, bmBottom, RGB(255, 0, 255));

            CBitmap bmMiddle = DpiScaleUpBitmap(CBitmap(IDW_SDMIDDLE));
            CSize szMiddle = bmMiddle.GetSize();
            int xMid = (szBig.cx - szMiddle.cx) / 2;
            int yMid = (szBig.cy - szMiddle.cy) / 2;
            CBrush grey(RGB(224, 224, 224));
            dc.FillRect(rc, grey);
            int height3 = rc.Height() / 3;
            int width3 = rc.Width() /3;
            dc.StretchBlt(0, height3,width3, height3, memDC, 0, midleft, szLeft.cx, szLeft.cy, SRCCOPY);
            dc.StretchBlt(width3, 0, width3, height3, memDC, midleft, 0, szTop.cx, szTop.cy, SRCCOPY);
            dc.StretchBlt(2* width3, height3, width3, height3, memDC, midright, midleft, szRight.cx, szRight.cy, SRCCOPY);
            dc.StretchBlt(width3, 2* height3, width3, height3, memDC, midleft, midright, szBottom.cx, szBottom.cy, SRCCOPY);

            if (IsOverContainer())
            {
                memDC.DrawBitmap(xMid, yMid, szMiddle.cx, szMiddle.cy, bmMiddle, RGB(255, 0, 255));
                dc.StretchBlt(rc.Width()/3, rc.Height() / 3, (rc.Width() / 3), (rc.Height() / 3), memDC, xMid, yMid, szMiddle.cx, szMiddle.cy, SRCCOPY);
            }
        }
    }


    ////////////////////////////////////////////////////////////////
    // Definitions for the CTargetLeft class nested within CDocker.
    //

    // Constructor.
    inline CDocker::CTargetLeft::CTargetLeft()
    {
        m_image.LoadBitmap(IDW_SDLEFT);
    }

    inline BOOL CDocker::CTargetLeft::CheckTarget(DragPos* pDragPos)
    {
        CDocker* pDockDrag = pDragPos->pDocker;
        assert(pDockDrag);
        if (!pDockDrag) return FALSE;

        CPoint pt = pDragPos->pos;
        CDocker* pDockTarget = pDockDrag->GetDockUnderDragPoint(pt)->GetTopmostDocker();
        if (pDockTarget != pDockDrag->GetDockAncestor())
        {
            Destroy();
            return FALSE;
        }

        if (pDockTarget->GetDockStyle() & DS_NO_DOCKCHILD_LEFT)
            return FALSE;

        CBitmap image = pDockTarget->DpiScaleUpBitmap(CBitmap(IDW_SDLEFT));
        int cxImage = image.GetSize().cx;
        int cyImage = image.GetSize().cy;

        if (!IsWindow())
        {
            Create();
            CRect rc = pDockTarget->GetViewRect();
            VERIFY(pDockTarget->ClientToScreen(rc));
            int yMid = rc.top + (rc.Height() - cyImage) / 2;
            VERIFY(SetWindowPos(HWND_TOPMOST, rc.left + DpiScaleInt(8), yMid, cxImage, cyImage, SWP_NOACTIVATE | SWP_SHOWWINDOW));
        }

        CRect rcLeft(0, 0, cxImage, cyImage);
        VERIFY(ScreenToClient(pt));

        // Test if our cursor is in one of the docking zones.
        if (rcLeft.PtInRect(pt))
        {
            pDockTarget->GetDockHint().DisplayHint(pDockTarget, pDockDrag, DS_DOCKED_LEFTMOST);
            pDockDrag->m_dockZone = DS_DOCKED_LEFTMOST;
            return TRUE;
        }

        return FALSE;
    }


    ////////////////////////////////////////////////////////////////
    // Definitions for the CTargetRight class nested within CDocker.
    //

    // Constructor.
    inline CDocker::CTargetRight::CTargetRight()
    {
        m_image.LoadBitmap(IDW_SDRIGHT);
    }

    inline BOOL CDocker::CTargetRight::CheckTarget(DragPos* pDragPos)
    {
        CDocker* pDockDrag = pDragPos->pDocker;
        assert(pDockDrag);
        if (!pDockDrag) return FALSE;

        CPoint pt = pDragPos->pos;
        CDocker* pDockTarget = pDockDrag->GetDockUnderDragPoint(pt)->GetTopmostDocker();
        if (pDockTarget != pDockDrag->GetDockAncestor())
        {
            Destroy();
            return FALSE;
        }

        if (pDockTarget->GetDockStyle() & DS_NO_DOCKCHILD_RIGHT)
            return FALSE;

        CBitmap image = pDockTarget->DpiScaleUpBitmap(CBitmap(IDW_SDRIGHT));
        int cxImage = image.GetSize().cx;
        int cyImage = image.GetSize().cy;

        if (!IsWindow())
        {
            Create();
            CRect rc = pDockTarget->GetViewRect();
            VERIFY(pDockTarget->ClientToScreen(rc));
            int yMid = rc.top + (rc.Height() - cyImage) / 2;
            VERIFY(SetWindowPos(HWND_TOPMOST, rc.right - DpiScaleInt(8) - cxImage, yMid, cxImage, cyImage, SWP_NOACTIVATE | SWP_SHOWWINDOW));
        }

        CRect rcRight(0, 0, cxImage, cyImage);
        VERIFY(ScreenToClient(pt));

        // Test if our cursor is in one of the docking zones.
        if (rcRight.PtInRect(pt))
        {
            pDockTarget->GetDockHint().DisplayHint(pDockTarget, pDockDrag, DS_DOCKED_RIGHTMOST);
            pDockDrag->m_dockZone = DS_DOCKED_RIGHTMOST;
            return TRUE;
        }

        return FALSE;
    }


    ////////////////////////////////////////////////////////////////
    // Definitions for the CTargetTop class nested within CDocker.
    //

    // Constructor.
    inline CDocker::CTargetTop::CTargetTop()
    {
        m_image.LoadBitmap(IDW_SDTOP);
    }

    inline BOOL CDocker::CTargetTop::CheckTarget(DragPos* pDragPos)
    {
        CDocker* pDockDrag = pDragPos->pDocker;
        assert(pDockDrag);
        if (!pDockDrag) return FALSE;

        CPoint pt = pDragPos->pos;
        CDocker* pDockTarget = pDockDrag->GetDockUnderDragPoint(pt)->GetTopmostDocker();
        if (pDockTarget != pDockDrag->GetDockAncestor())
        {
            Destroy();
            return FALSE;
        }

        if (pDockTarget->GetDockStyle() & DS_NO_DOCKCHILD_TOP)
            return FALSE;

        CBitmap image = pDockTarget->DpiScaleUpBitmap(CBitmap(IDW_SDTOP));
        int cxImage = image.GetSize().cx;
        int cyImage = image.GetSize().cy;

        if (!IsWindow())
        {
            Create();
            CRect rc = pDockTarget->GetViewRect();
            VERIFY(pDockTarget->ClientToScreen(rc));
            int xMid = rc.left + (rc.Width() - cxImage) / 2;
            VERIFY(SetWindowPos(HWND_TOPMOST, xMid, rc.top + DpiScaleInt(8), cxImage, cyImage, SWP_NOACTIVATE | SWP_SHOWWINDOW));
        }

        CRect rcTop(0, 0, cxImage, cyImage);
        VERIFY(ScreenToClient(pt));

        // Test if our cursor is in one of the docking zones.
        if (rcTop.PtInRect(pt))
        {
            pDockTarget->GetDockHint().DisplayHint(pDockTarget, pDockDrag, DS_DOCKED_TOPMOST);
            pDockDrag->m_dockZone = DS_DOCKED_TOPMOST;
            return TRUE;
        }

        return FALSE;
    }


    /////////////////////////////////////////
    // Definitions for the CDocker class.
    //

    // Constructor.
    inline CDocker::CDocker() : m_pDockParent(nullptr), m_pDockAncestor(nullptr),
                    m_isUndocking(FALSE), m_isClosing(FALSE), m_isDragging(FALSE),
                    m_dockStartSize(0), m_dockID(0), m_ncHeight(0),
                    m_newDpi(USER_DEFAULT_SCREEN_DPI), m_oldDpi(USER_DEFAULT_SCREEN_DPI),
                    m_dockZone(0), m_dockStyle(0), m_dockUnderPoint(0)
    {
        // Set the initial defaults.
        SetDockBar(m_dockBar);
        SetDockClient(m_dockClient);
        SetDockHint(m_dockHint);

        m_pDockAncestor = this;         // Assume this docker is the DockAncestor for now.
        m_allDockers.push_back(this);
        GetDockClient().SetDocker(this);
    }

    // Destructor.
    inline CDocker::~CDocker()
    {
    }

    // This function creates the docker, and adds it to the docker hierarchy as docked.
    inline CDocker* CDocker::AddDockedChild(CDocker* pDocker, DWORD dockStyle, int dockSize, int dockID /* = 0*/)
    {
        // Create the new docker window as a child of this docker.
        // This permanently sets the frame window as the docker window's owner,
        // even when its parent is subsequently changed.

        assert(pDocker);
        return AddDockedChild(DockPtr(pDocker), dockStyle, dockSize, dockID);
    }

    // This function creates the docker, and adds it to the docker hierarchy as docked.
    inline CDocker* CDocker::AddDockedChild(DockPtr docker, DWORD dockStyle, int dockSize, int dockID /* = 0*/)
    {
        // Create the new docker window as a child of this docker.
        // This permanently sets the frame window as the docker window's owner,
        // even when its parent is subsequently changed.

        CDocker* pDocker = docker.get();
        assert(pDocker);
        if (!pDocker)
            return nullptr;

        // Store the docker's unique pointer in the DockAncestor's vector for later deletion.
        GetAllChildren().push_back(std::move(docker));
        GetDockAncestor()->m_allDockers.push_back(pDocker);

        pDocker->SetDockStyle(dockStyle);
        pDocker->m_dockID = dockID;
        pDocker->m_pDockAncestor = GetDockAncestor();
        pDocker->m_pDockParent = this;
        HWND frame = GetDockAncestor()->GetAncestor();
        pDocker->Create(frame);
        pDocker->SetParent(*this);

        // Dock the docker window.
        if (dockStyle & DS_DOCKED_CONTAINER)
            DockInContainer(pDocker, dockStyle);
        else
            Dock(pDocker, dockStyle);

        pDocker->SetDockSize(dockSize);

        // Issue TRACE warnings for any missing resources.
        HMODULE module = GetApp()->GetResourceHandle();

        if (!(dockStyle & DS_NO_RESIZE))
        {
            if (!FindResource(module, MAKEINTRESOURCE(IDW_SPLITH), RT_GROUP_CURSOR))
                TRACE("*** WARNING: Horizontal cursor resource missing. ***\n");
            if (!FindResource(module, MAKEINTRESOURCE(IDW_SPLITV), RT_GROUP_CURSOR))
                TRACE("*** WARNING: Vertical cursor resource missing. ***\n");
        }

        if (!(dockStyle & DS_NO_UNDOCK))
        {
            if (!FindResource(module, MAKEINTRESOURCE(IDW_SDCENTER), RT_BITMAP))
                TRACE("*** WARNING: Docking center bitmap resource missing. ***\n");
            if (!FindResource(module, MAKEINTRESOURCE(IDW_SDLEFT), RT_BITMAP))
                TRACE("*** WARNING: Docking left bitmap resource missing. ***\n");
            if (!FindResource(module, MAKEINTRESOURCE(IDW_SDRIGHT), RT_BITMAP))
                TRACE("*** WARNING: Docking right bitmap resource missing. ***\n");
            if (!FindResource(module, MAKEINTRESOURCE(IDW_SDTOP), RT_BITMAP))
                TRACE("*** WARNING: Docking top bitmap resource missing. ***\n");
            if (!FindResource(module, MAKEINTRESOURCE(IDW_SDBOTTOM), RT_BITMAP))
                TRACE("*** WARNING: Docking center bottom resource missing. ***\n");
        }

        if (dockStyle & DS_DOCKED_CONTAINER)
        {
            if (!FindResource(module, MAKEINTRESOURCE(IDW_SDMIDDLE), RT_BITMAP))
                TRACE("*** WARNING Docking container bitmap resource missing. ***\n");
        }

        RecalcDockLayout();
        return pDocker;
    }

    // This function creates the docker, and adds it to the docker hierarchy as undocked.
    inline CDocker* CDocker::AddUndockedChild(CDocker* pDocker, DWORD dockStyle, int dockSize, const RECT& rc, int dockID /* = 0*/)
    {
        assert(pDocker);
        return AddUndockedChild(DockPtr(pDocker), dockStyle, dockSize, rc, dockID);
    }

    // This function creates the docker, and adds it to the docker hierarchy as undocked.
    inline CDocker* CDocker::AddUndockedChild(DockPtr docker, DWORD dockStyle, int dockSize, const RECT& rc, int dockID /* = 0*/)
    {
        CDocker* pDocker = docker.get();
        assert(pDocker);
        if (!pDocker)
            return nullptr;

        // Store the Docker's unique pointer in the DockAncestor's vector for later deletion.
        GetAllChildren().push_back(std::move(docker));
        GetDockAncestor()->m_allDockers.push_back(pDocker);

        pDocker->SetDockSize(dockSize);
        pDocker->SetDockStyle(dockStyle & 0XFFFFFF0);
        pDocker->m_isUndocking = TRUE;    // IsUndocked reports FALSE.
        pDocker->m_dockID = dockID;
        pDocker->m_pDockAncestor = GetDockAncestor();

        // Initially create the as a child window of the frame.
        // This makes the frame window the owner of our docker.
        HWND frame = GetDockAncestor()->GetAncestor();
        pDocker->Create(frame);
        pDocker->SetParent(*this);

        // Change the Docker to a POPUP window.
        DWORD style = WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_VISIBLE;
        pDocker->SetStyle(style);
        pDocker->SetRedraw(FALSE);
        pDocker->SetParent(0);
        VERIFY(pDocker->SetWindowPos(HWND_TOP, rc, SWP_SHOWWINDOW | SWP_FRAMECHANGED));
        pDocker->RecalcDockLayout();
        pDocker->SetWindowText(pDocker->GetCaption().c_str());
        pDocker->SetRedraw(TRUE);

        pDocker->RedrawWindow(RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
        pDocker->m_isUndocking = FALSE;  // IsUndocked now reports TRUE.

        return pDocker;
    }

    // Calls CheckTarget for each possible target zone.
    inline void CDocker::CheckAllTargets(DragPos* pDragPos) const
    {
        if (!GetDockAncestor()->m_targetCentre.CheckTarget(pDragPos))
        {
            if (!GetDockAncestor()->m_targetLeft.CheckTarget(pDragPos))
            {
                if (!GetDockAncestor()->m_targetTop.CheckTarget(pDragPos))
                {
                    if (!GetDockAncestor()->m_targetRight.CheckTarget(pDragPos))
                    {
                        if (!GetDockAncestor()->m_targetBottom.CheckTarget(pDragPos))
                        {
                            // Not in a docking zone, so clean up.
                            CDocker* pDockDrag = pDragPos->pDocker;
                            if (pDockDrag)
                            {
                                GetDockHint().Destroy();
                                pDockDrag->m_dockZone = 0;
                            }
                        }
                    }
                }
            }
        }
    }

    // Closes all the child dockers of this dock ancestor.
    inline void CDocker::CloseAllDockers()
    {
        assert(this == GetDockAncestor());  // Must call CloseAllDockers from the DockAncestor.

        for (const DockPtr& ptr : GetAllChildren())
        {
            // The CDocker is destroyed when the window is destroyed.
            ptr->m_isClosing = TRUE;
            ptr->Destroy();    // Destroy the window.
        }

        m_dockChildren.clear();

        // Delete any child containers this container might have.
        if (GetContainer())
        {
            std::vector<ContainerInfo> AllContainers = GetContainer()->GetAllContainers();
            for (const ContainerInfo& ci : AllContainers)
            {
                if (GetContainer() != ci.pContainer)
                    GetContainer()->RemoveContainer(ci.pContainer, FALSE);
            }
        }

        RecalcDockLayout();
    }

    // Close all dock target windows.
    inline void CDocker::CloseAllTargets() const
    {
        GetDockAncestor()->m_targetCentre.Destroy();
        GetDockAncestor()->m_targetLeft.Destroy();
        GetDockAncestor()->m_targetTop.Destroy();
        GetDockAncestor()->m_targetRight.Destroy();
        GetDockAncestor()->m_targetBottom.Destroy();
    }

    inline void CDocker::ConvertToChild(HWND parent) const
    {
        DWORD style = WS_CHILD | WS_VISIBLE;
        SetStyle(style);
        SetParent(parent);
        GetDockBar().SetParent(parent);
    }

    // Change the window to an "undocked" style.
    inline void CDocker::ConvertToPopup(const RECT& rc, BOOL showUndocked)
    {
        ShowWindow(SW_HIDE);
        DWORD style = WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME;
        SetStyle(style);

        // Change the window's parent and reposition it.
        if (GetDockBar().IsWindow())
            GetDockBar().ShowWindow(SW_HIDE);

        VERIFY(SetWindowPos(HWND_TOP, 0, 0, 0, 0, SWP_NOSENDCHANGING | SWP_HIDEWINDOW | SWP_NOREDRAW));
        m_pDockParent = 0;
        SetParent(0);

        DWORD styleShow = showUndocked ? SWP_SHOWWINDOW : 0U;
        VERIFY(SetWindowPos(HWND_TOP, rc, styleShow | SWP_FRAMECHANGED | SWP_NOOWNERZORDER));
        VERIFY(GetDockClient().SetWindowPos(HWND_TOP, GetClientRect(), SWP_SHOWWINDOW));
        SetWindowText(GetCaption().c_str());
    }

    // Docks the specified docker inside this docker.
    inline void CDocker::Dock(CDocker* pDocker, UINT dockStyle)
    {
        assert(pDocker);
        if (!pDocker) return;

        pDocker->m_pDockParent = this;
        pDocker->SetDockStyle(dockStyle);
        m_dockChildren.push_back(pDocker);
        pDocker->ConvertToChild(*this);

        // Limit the docked size to half the parent's size if it won't fit inside parent.
        if (((dockStyle & 0xF)  == DS_DOCKED_LEFT) || ((dockStyle &0xF)  == DS_DOCKED_RIGHT))
        {
            int width = GetDockClient().GetWindowRect().Width();
            int barWidth = pDocker->GetBarWidth();
            if (pDocker->m_dockStartSize >= (width - barWidth))
                pDocker->SetDockSize(std::max(width/2 - barWidth, barWidth));
        }
        else
        {
            int height = GetDockClient().GetWindowRect().Height();
            int barWidth = pDocker->GetBarWidth();
            if (pDocker->m_dockStartSize >= (height - barWidth))
                pDocker->SetDockSize(std::max(height/2 - barWidth, barWidth));
        }

        // Redraw the docked windows.
        if (GetAncestor().IsWindowVisible())
        {
            GetTopmostDocker()->SetForegroundWindow();

            // Give the view window focus unless its child already has it.
            if (!pDocker->GetView().IsChild(GetFocus()))
                pDocker->GetView().SetFocus();

            GetTopmostDocker()->SetRedraw(FALSE);
            pDocker->SetDefaultCaptionHeight();
            RecalcDockLayout();
            GetTopmostDocker()->SetRedraw(TRUE);
            GetTopmostDocker()->RedrawWindow();

            // Update the Dock captions.
            GetDockAncestor()->PostMessage(UWM_DOCKACTIVATE);
        }
    }

    // Add a container to an existing container.
    inline void CDocker::DockInContainer(CDocker* pDocker, DWORD dockStyle, BOOL selectPage)
    {
        if ((dockStyle & DS_DOCKED_CONTAINER) && GetContainer())
        {
            // Transfer any dock children to this docker.
            pDocker->MoveDockChildren(this);

            // Transfer container children to the target container.
            CDockContainer* pContainer = GetContainer();
            CDockContainer* pContainerSource = pDocker->GetContainer();
            std::vector<ContainerInfo> allContainers = pContainerSource->GetAllContainers();
            for (const ContainerInfo& ci : allContainers)
            {
                CDockContainer* pContainerChild = ci.pContainer;
                if (pContainerChild != pContainerSource)
                {
                    // Remove child container from pContainerSource.
                    pContainerChild->ShowWindow(SW_HIDE);
                    pContainerSource->RemoveContainer(pContainerChild);
                    CDocker* pDockChild = GetDockFromView(pContainerChild);
                    assert(pDockChild);
                    if (pDockChild)
                    {
                        pDockChild->SetParent(*this);
                        pDockChild->m_pDockParent = this;
                    }
                }
            }

            pDocker->m_pDockParent = this;
            pDocker->ShowWindow(SW_HIDE);
            pDocker->SetStyle(WS_CHILD);
            pDocker->SetDockStyle(dockStyle);
            pDocker->SetParent(*this);
            pDocker->GetDockBar().SetParent(*GetDockAncestor());

            // Insert the containers in reverse order.
            for (ContainerInfo& ci : allContainers)
            {
                pContainer->AddContainer(ci.pContainer, TRUE, selectPage);
            }
        }

        // Redraw the docked windows.
        pDocker->RecalcDockLayout();
    }

    // Docks the specified docker inside the dock ancestor.
    inline void CDocker::DockOuter(CDocker* pDocker, DWORD dockStyle)
    {
        assert(pDocker);
        if (!pDocker) return;

        pDocker->m_pDockParent = GetDockAncestor();

        DWORD outerDocking = dockStyle & 0xF0000;
        DWORD dockSide = outerDocking / 0x10000;
        dockStyle &= 0xFFF0FFFF;
        dockStyle |= dockSide;

        // Set the dock styles
        DWORD style = WS_CHILD | WS_VISIBLE;
        pDocker->SetStyle(style);
        pDocker->ShowWindow(SW_HIDE);
        pDocker->SetDockStyle(dockStyle);

        // Set the docking relationships.
        auto it = GetDockAncestor()->m_dockChildren.begin();
        GetDockAncestor()->m_dockChildren.insert(it, pDocker);
        pDocker->SetParent(*GetDockAncestor());
        pDocker->GetDockBar().SetParent(*GetDockAncestor());

        // Limit the docked size to half the parent's size if it won't fit inside parent.
        if (((dockStyle & 0xF)  == DS_DOCKED_LEFT) || ((dockStyle &0xF)  == DS_DOCKED_RIGHT))
        {
            int width = GetDockAncestor()->GetDockClient().GetWindowRect().Width();
            int barWidth = pDocker->GetBarWidth();
            if (pDocker->m_dockStartSize >= (width - barWidth))
                pDocker->SetDockSize(std::max(width/2 - barWidth, barWidth));
        }
        else
        {
            int height = GetDockAncestor()->GetDockClient().GetWindowRect().Height();
            int barWidth = pDocker->GetBarWidth();
            if (pDocker->m_dockStartSize >= (height - barWidth))
                pDocker->SetDockSize(std::max(height/2 - barWidth, barWidth));
        }

        // Redraw the docked windows.
        if (GetAncestor().IsWindowVisible())
        {
            GetTopmostDocker()->SetForegroundWindow();

            // Give the view window focus unless its child already has it.
            if (!pDocker->GetView().IsChild(GetFocus()))
                pDocker->GetView().SetFocus();

            GetTopmostDocker()->SetRedraw(FALSE);
            pDocker->SetDefaultCaptionHeight();
            RecalcDockLayout();
            GetTopmostDocker()->SetRedraw(TRUE);
            GetTopmostDocker()->RedrawWindow();

            // Update the Dock captions.
            GetDockAncestor()->PostMessage(UWM_DOCKACTIVATE);
        }
    }

    // Updates the view for all dockers in this dock family.
    // Call this when the DPI changes.
    inline void CDocker::DpiUpdateDockerSizes()
    {
        std::vector<CDocker*> v = GetAllDockers();
        for (CDocker* docker : v)
        {
            if (docker->IsWindow() && (docker->GetTopmostDocker() == this))
            {
                // Reset the docker size.
                int size = (docker->GetDockSize() * GetWindowDpi(*GetTopmostDocker())) / GetTopmostDocker()->m_oldDpi;
                docker->SetDockSize(size);
            }
        }

        GetTopmostDocker()->m_oldDpi = GetWindowDpi(*GetTopmostDocker());
        RecalcDockLayout();
    }

    // Draws all the captions.
    inline void CDocker::DrawAllCaptions() const
    {
        for (CDocker* pDocker : GetAllDockers())
        {
            if (pDocker->IsDocked() || pDocker == GetDockAncestor())
                pDocker->GetDockClient().DrawCaption();
        }
    }

    // Static callback function to enumerate top level dockers excluding
    // the one being dragged. Top level windows are enumerated in Z order.
    inline BOOL CALLBACK CDocker::EnumWindowsProc(HWND top, LPARAM lparam)
    {
        CDocker* pThis = reinterpret_cast<CDocker*>(lparam);
        assert(dynamic_cast<CDocker*>(pThis));
        if (!pThis) return FALSE;

        CPoint pt = pThis->m_dockPoint;

        // Update top if the DockAncestor is a child of the top level window.
        if (::IsChild(top, pThis->GetDockAncestor()->GetHwnd()))
            top = pThis->GetDockAncestor()->GetHwnd();

        // Assign this docker's m_dockUnderPoint.
        if (pThis->IsRelated(top) && top != pThis->GetHwnd())
        {
            CRect rc;
            VERIFY(::GetWindowRect(top, &rc));
            if (rc.PtInRect(pt))
            {
                pThis->m_dockUnderPoint = top;
                return FALSE;   // Stop enumerating.
            }
        }

        return TRUE;    // Continue enumerating.
    }

    // Returns the docker that has a child window with keyboard focus.
    // The active docker's caption is drawn with a different color.
    inline CDocker* CDocker::GetActiveDocker() const
    {
        CDocker* pDockActive = nullptr;
        HWND test = ::GetFocus();

        while (test != nullptr)
        {
            HWND parent = ::GetParent(test);
            if (parent == test) break;      // Might be owned window, not parent.
            test = parent;

            CDocker* pDock = reinterpret_cast<CDocker*>(::SendMessage(test, UWM_GETCDOCKER, 0, 0));
            if (pDock)
            {
                pDockActive = pDock;
                break;
            }
        }

        return pDockActive;
    }

    // Returns a Docker's active view. Returns the container's active view for a docker with
    // a DockContainer, returns the the active MDI child for a docker with a TabbedMDI,
    // or the docker's view for a simple docker. Can return nullptr.
    inline CWnd* CDocker::GetActiveView() const
    {
        CWnd* pView;
        if (GetContainer())
            pView = GetContainer()->GetActiveView();
        else if (GetTabbedMDI())
            pView = GetTabbedMDI()->GetActiveMDIChild();
        else
            pView = &GetView();

        return pView;
    }

    // Returns a pointer to the view if it is a CDockContainer.
    // Returns nullptr if the view is not a CDockContainer.
    // Returns nullptr if the docker or it's view are not created.
    inline CDockContainer* CDocker::GetContainer() const
    {
        // returns nullptr if not a CDockContainer*
        if (IsWindow() && GetView().IsWindow())
            return reinterpret_cast<CDockContainer*>(GetView().SendMessage(UWM_GETCDOCKCONTAINER));

        return nullptr;
    }

    // The GetDockAncestor function retrieves the pointer to the
    // ancestor (root docker parent) of the docker.
    inline CDocker* CDocker::GetDockAncestor() const
    {
        return m_pDockAncestor;
    }

    // Returns the docker that has the specified Dock ID.
    inline CDocker* CDocker::GetDockFromID(int dockID) const
    {
        if (GetDockAncestor())
        {
            for (const DockPtr& ptr : GetAllChildren())
            {
                if (dockID == ptr->GetDockID())
                    return ptr.get();
            }
        }

        return 0;
    }

    // Returns the child docker that has the specified view.
    inline CDocker* CDocker::GetDockFromView(CWnd* pView) const
    {
        CDocker* pDocker = nullptr;
        for (const DockPtr& dockPtr : GetAllChildren())
        {
            if (&dockPtr->GetView() == pView)
                pDocker = dockPtr.get();
        }

        if (&GetDockAncestor()->GetView() == pView)
            pDocker = GetDockAncestor();

        return pDocker;
    }

    // Returns the size of the docker to be used if it is redocked.
    // Note: This function returns 0 if the docker has the DS_DOCKED_CONTAINER style.
    inline int CDocker::GetDockSize() const
    {
        return m_dockStartSize;
    }

    // Retrieves the Docker whose view window contains the specified point.
    // Used when dragging undocked dockers over other dockers to provide
    // the docker that needs to display the dock targets and dock hints.
    inline CDocker* CDocker::GetDockUnderDragPoint(POINT pt)
    {
        // Step 1: Find the top level Docker under the point.
        // EnumWindows assigns the Docker under the point to m_dockUnderPoint.

        m_dockUnderPoint = nullptr;
        m_dockPoint = pt;
        EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(this));

        // Step 2: Find the docker child whose view window has the point.
        CDocker* pDockTarget = nullptr;

        HWND dockTest = m_dockUnderPoint;
        HWND dockParent = m_dockUnderPoint;

        if (m_dockUnderPoint != nullptr)
        {
            while (IsRelated(dockTest))
            {
                dockParent = dockTest;
                CPoint ptLocal = pt;
                VERIFY(::ScreenToClient(dockParent, &ptLocal));
                dockTest = ::ChildWindowFromPoint(dockParent, ptLocal);
                if (dockTest == dockParent) break;
            }

            CDocker* pDockParent = reinterpret_cast<CDocker*>(::SendMessage(dockParent, UWM_GETCDOCKER, 0, 0));
            assert(pDockParent);

            CRect rc = pDockParent->GetDockClient().GetWindowRect();
            if (rc.PtInRect(pt))
                pDockTarget = pDockParent;
        }

        return pDockTarget;
    }

    // Retrieve the DPI of the monitor the window is on. This usually provides
    // the same value as the GetWindowDpi function, but not when the user
    // changes DPI settings manually and a WM_SETTINGCHANGE message is sent.
    // This function uses the GetDpiForMonitor function.
    inline int CDocker::GetMonitorDpi(HWND wnd)
    {
        // Retrieve window's default dpi as a fallback.
        CClientDC clientDC(wnd);
        int dpi = GetDeviceCaps(clientDC, LOGPIXELSX);

        // Retrieve the monitor's dpi if we can.
        using GETDPIFORMONITOR = HRESULT (WINAPI*)(HMONITOR hmonitor, int dpiType, UINT* dpiX, UINT* dpiY);
        HMODULE shcore = GetModuleHandle(_T("shcore"));
        if (shcore)
        {
            GETDPIFORMONITOR pGetDpiForMonitor = reinterpret_cast<GETDPIFORMONITOR>(
                reinterpret_cast<void*>(::GetProcAddress(shcore, "GetDpiForMonitor")));

            if (pGetDpiForMonitor)
            {
                HMONITOR hMonitor = MonitorFromWindow(wnd, MONITOR_DEFAULTTOPRIMARY);
                UINT dpiX;
                UINT dpiY;
                HRESULT hr = pGetDpiForMonitor(hMonitor, 0, &dpiX, &dpiY);
                if (SUCCEEDED(hr))
                {
                    dpi = static_cast<int>(dpiX);
                }
            }
        }

        return dpi;
    }

    // Returns the pointer to the view window if it is a CTabbedMDI.
    // Returns nullptr if not a CTabbedMDI.
    inline CTabbedMDI* CDocker::GetTabbedMDI() const
    {
        // Returns nullptr if not a CTabbedMDI*.
        if (IsWindow())
            return reinterpret_cast<CTabbedMDI*>(GetView().SendMessage(UWM_GETCTABBEDMDI));

        return nullptr;
    }

    // Retrieves the text height for the caption.
    inline int CDocker::GetTextHeight()
    {
        int dpi = GetWindowDpi(*this);
        NONCLIENTMETRICS info = GetNonClientMetrics();
        LOGFONT lf = info.lfStatusFont;
        lf.lfHeight = -MulDiv(9, dpi, POINTS_PER_INCH);

        CClientDC dc(*this);
        dc.CreateFontIndirect(lf);
        CSize textSize = dc.GetTextExtentPoint32(_T("Text"), lstrlen(_T("Text")));
        return textSize.cy;
    }

    // Returns the docker's parent at the top of the Z order.
    // Could be the dock ancestor or an undocked docker.
    inline CDocker* CDocker::GetTopmostDocker() const
    {
        CDocker* pDockTopLevel = (CDocker* const)this;

        while (pDockTopLevel->GetDockParent())
        {
            assert(pDockTopLevel != pDockTopLevel->GetDockParent());
            pDockTopLevel = pDockTopLevel->GetDockParent();
        }

        return pDockTopLevel;
    }

    // Undocks a docker (if needed) and hides it.
    // To unhide the docker, dock it.
    inline void CDocker::Hide()
    {
        if (IsDocked())
        {
            CDockContainer* pContainer = GetContainer();

            if (pContainer)
            {
                if (pContainer == pContainer->GetContainerParent())
                    UndockContainer(pContainer, GetCursorPos(), FALSE);
                else
                    pContainer->GetContainerParent()->RemoveContainer(pContainer, FALSE);
            }
            else
            {
                CDocker* pDockUndockedFrom = SeparateFromDock();
                pDockUndockedFrom->RecalcDockLayout();
            }
        }

        ShowWindow(SW_HIDE);
        RecalcDockLayout();
    }

    // Returns true if the specified window is a decendant of this docker.
    inline BOOL CDocker::IsChildOfDocker(HWND wnd) const
    {
        while ((wnd != nullptr) && (wnd != *GetDockAncestor()))
        {
            if ( wnd == *this ) return TRUE;
            wnd = ::GetParent(wnd);
        }

        return FALSE;
    }

    // Returns TRUE if this docker is docked.
    inline BOOL CDocker::IsDocked() const
    {
        return (((m_dockStyle&0xF) || (m_dockStyle & DS_DOCKED_CONTAINER)) && !m_isUndocking); // Boolean expression
    }

    // Returns TRUE if the wnd is a docker within this dock family.
    inline BOOL CDocker::IsRelated(HWND wnd) const
    {
        if (*GetDockAncestor() == wnd) return TRUE;

        for (const DockPtr& dockPtr : GetAllChildren())
        {
            if (dockPtr.get()->GetHwnd() == wnd) return TRUE;
        }

        return FALSE;
    }

    // Returns TRUE if the docker is docked, or is a dock ancestor that
    // has a CDockContainer with tabs.
    inline BOOL CDocker::IsUndockable() const
    {
        BOOL isUndockable = IsDocked();

        CDockContainer* pContainer = GetContainer();
        if (this == GetDockAncestor())
        {
            if (pContainer && (pContainer->GetItemCount() > 0))
                isUndockable = TRUE;
        }

        return isUndockable;
    }

    // Returns TRUE if the docker is undocked.
    inline BOOL CDocker::IsUndocked() const
    {
        return (!((m_dockStyle&0xF)|| (m_dockStyle & DS_DOCKED_CONTAINER)) && !m_isUndocking); // Boolean expression
    }

    // Loads the information for CDockContainers from the registry.
    inline BOOL CDocker::LoadContainerRegistrySettings(LPCTSTR registryKeyName)
    {
        BOOL isLoaded = FALSE;
        if (registryKeyName)
        {
            // Load Dock container tab order and active container.
            const CString dockSettings = _T("\\Dock Settings");
            const CString dockKeyName = _T("Software\\") + CString(registryKeyName) + dockSettings;
            CRegKey settingsKey;

            try
            {
                if (ERROR_SUCCESS != settingsKey.Open(HKEY_CURRENT_USER, dockKeyName, KEY_READ))
                    throw CUserException();

                UINT container = 0;
                CString dockContainerName;
                dockContainerName.Format(_T("DockContainer%u"), container);
                CRegKey containerKey;

                while (ERROR_SUCCESS == containerKey.Open(settingsKey, dockContainerName, KEY_READ))
                {
                    // Load tab order
                    isLoaded = TRUE;
                    UINT tabNumber = 0;
                    DWORD tabID;
                    std::vector<UINT> tabOrder;
                    CString tabKeyName;
                    tabKeyName.Format(_T("Tab%u"), tabNumber);
                    while (ERROR_SUCCESS == containerKey.QueryDWORDValue(tabKeyName, tabID))
                    {
                        tabOrder.push_back(tabID);
                        tabKeyName.Format(_T("Tab%u"), ++tabNumber);
                    }

                    // Set tab order.
                    DWORD parentID;
                    if (ERROR_SUCCESS == containerKey.QueryDWORDValue(_T("Parent Container"), parentID))
                    {
                        CDocker* pDocker = GetDockFromID(static_cast<int>(parentID));
                        if (!pDocker)
                            pDocker = this;

                        CDockContainer* pParentContainer = pDocker->GetContainer();
                        if (!pParentContainer)
                            throw CUserException();

                        for (UINT tab = 0; tab < tabOrder.size(); ++tab)
                        {
                            CDocker* pOldDocker = GetDockFromView(pParentContainer->GetContainerFromIndex(tab));
                            if (!pOldDocker)
                                throw CUserException();

                            UINT oldID = static_cast<UINT>(pOldDocker->GetDockID());

                            auto it = std::find(tabOrder.begin(), tabOrder.end(), oldID);
                            UINT oldTab = static_cast<UINT>(it - tabOrder.begin());

                            if (tab >= pParentContainer->GetAllContainers().size())
                                throw CUserException();

                            if (oldTab >= pParentContainer->GetAllContainers().size())
                                throw CUserException();

                            if (tab != oldTab)
                                pParentContainer->SwapTabs(static_cast<int>(tab), static_cast<int>(oldTab));
                        }
                    }

                    // Set the active container.
                    DWORD activeContainer;
                    if (ERROR_SUCCESS == containerKey.QueryDWORDValue(_T("Active Container"), activeContainer))
                    {
                        CDocker* pDocker = GetDockFromID(static_cast<int>(activeContainer));
                        if (pDocker)
                        {
                            CDockContainer* pContainer = pDocker->GetContainer();
                            if (!pContainer)
                                throw CUserException();

                            int page = pContainer->GetContainerIndex(pContainer);
                            if (page >= 0)
                                pContainer->SelectPage(page);
                        }
                    }

                    dockContainerName.Format(_T("DockContainer%u"), ++container);
                }
            }

            catch (const CUserException&)
            {
                TRACE("*** WARNING: Failed to load dock containers from registry. ***\n");
                CloseAllDockers();

                // Delete the bad key from the registry.
                const CString appKeyName = _T("Software\\") + CString(registryKeyName);
                CRegKey appKey;
                if (ERROR_SUCCESS == appKey.Open(HKEY_CURRENT_USER, appKeyName, KEY_READ))
                    appKey.RecurseDeleteKey(dockSettings);
            }
        }

        return isLoaded;
    }

    // Recreates the docker layout based on information stored in the registry.
    // Assumes the DockAncestor window is already created.
    inline BOOL CDocker::LoadDockRegistrySettings(LPCTSTR registryKeyName)
    {
        BOOL isLoaded = FALSE;

        if (registryKeyName)
        {
            std::vector<DockInfo> dockList;
            const CString dockSettings = _T("\\Dock Settings");
            const CString dockKeyName = _T("Software\\") + CString(registryKeyName) + dockSettings;
            CRegKey settingsKey;

            try
            {
                if (ERROR_SUCCESS != settingsKey.Open(HKEY_CURRENT_USER, dockKeyName, KEY_READ))
                    throw CUserException();

                DWORD bufferSize = sizeof(DockInfo);
                DockInfo info;
                int i = 0;
                CString dockChildName;
                dockChildName.Format(_T("DockChild%d"), i);

                // Fill the DockList vector from the registry.
                while (ERROR_SUCCESS == settingsKey.QueryBinaryValue(dockChildName, &info, &bufferSize))
                {
                    dockList.push_back(info);
                    i++;
                    dockChildName.Format(_T("DockChild%d"), i);
                }

                if (dockList.size() > 0)
                    isLoaded = TRUE;

                // Add the dock ancestor's style.
                DWORD style;
                if (ERROR_SUCCESS != settingsKey.QueryDWORDValue(_T("DockAncestor"), style))
                    throw CUserException();

                SetDockStyle(style);

                // Add dockers without parents first.
                for (const DockInfo& di : dockList)
                {
                    if ((di.dockParentID == 0) || (di.isInAncestor))
                    {
                        DockPtr docker = NewDockerFromID(di.dockID);
                        CDocker* pDocker = docker.get();
                        if (!pDocker)
                            throw CUserException();

                        if ((di.dockStyle & 0xF) || (di.isInAncestor))
                            AddDockedChild(std::move(docker), di.dockStyle, di.dockSize, di.dockID);
                        else
                            AddUndockedChild(std::move(docker), di.dockStyle, di.dockSize, di.rect, di.dockID);
                    }
                }

                // Remove dockers without parents from dockList.
                auto it = dockList.begin();
                while (it != dockList.end())
                {
                    if (((*it).dockParentID == 0) || ((*it).isInAncestor))
                        it = dockList.erase(it);
                    else
                        ++it;
                }

                // Add remaining dockers
                while (dockList.size() > 0)
                {
                    bool found = false;
                    for (it = dockList.begin(); it != dockList.end(); ++it)
                    {
                        DockInfo di = *it;
                        CDocker* pDockParent = GetDockFromID(di.dockParentID);

                        if (pDockParent != nullptr)
                        {
                            DockPtr docker = NewDockerFromID(di.dockID);
                            if (docker.get() == nullptr)
                                throw CUserException();

                            pDockParent->AddDockedChild(std::move(docker), di.dockStyle, di.dockSize, di.dockID);
                            found = true;
                            dockList.erase(it);
                            break;
                        }
                    }

                    if (!found)
                        throw CUserException();

                    if (!VerifyDockers())
                        throw CUserException();
                }
            }

            catch (const CUserException&)
            {
                TRACE("*** WARNING: Failed to load dockers from registry. ***\n");
                isLoaded = FALSE;
                CloseAllDockers();

                // Delete the bad key from the registry.
                const CString appKeyName = _T("Software\\") + CString(registryKeyName);
                CRegKey appKey;
                if (ERROR_SUCCESS == appKey.Open(HKEY_CURRENT_USER, appKeyName, KEY_READ))
                    appKey.RecurseDeleteKey(dockSettings);
            }
        }

        if (isLoaded)
            LoadContainerRegistrySettings(registryKeyName);

        return isLoaded;
    }

    // Used internally by Dock and Undock.
    inline void CDocker::MoveDockChildren(CDocker* pDockTarget)
    {
        assert(pDockTarget);
        if (!pDockTarget)  return;

        // Transfer any dock children from the current docker to the target docker.
        for (CDocker* pDocker : m_dockChildren)
        {
            pDockTarget->m_dockChildren.push_back(pDocker);
            pDocker->m_pDockParent = pDockTarget;
            pDocker->SetParent(*pDockTarget);
            pDocker->GetDockBar().SetParent(*pDockTarget);
        }
        m_dockChildren.clear();
    }

    // Used in LoadRegistrySettings. Creates a new Docker from the specified ID.
    // Returns a unique_ptr<CDocker>.
    inline DockPtr CDocker::NewDockerFromID(int /*id*/)
    {
        // Override this function to create the Docker objects as shown below.

        DockPtr docker;
    /*  switch(id)
        {
        case ID_CLASSES:
            docker = std::make_unique<CDockClasses>();
            break;
        case ID_FILES:
            docker = std::make_unique<CDockFiles>();
            break;
        default:
            TRACE("Unknown Dock ID\n");
            break;
        } */

        return docker;
    }

    // Called when the this docker is activated.
    inline LRESULT CDocker::OnActivate(UINT, WPARAM wparam, LPARAM)
    {
        if ((this != GetDockAncestor()) && IsUndocked())
        {
            GetDockAncestor()->PostMessage(UWM_DOCKACTIVATE);

            // Give the view window focus unless its child already has it.
            if ((wparam != WA_INACTIVE) && !GetView().IsChild(GetFocus()))
                GetView().SetFocus();
        }

        return 0;
    }

    // Called when splitter bar move has ended.
    inline LRESULT CDocker::OnBarEnd(DragPos* pDragPos)
    {
        POINT pt = pDragPos->pos;
        VERIFY(ScreenToClient(pt));

        ResizeDockers(pDragPos);
        return 0;
    }

    // Called when the splitter bar is moved.
    inline LRESULT CDocker::OnBarMove(DragPos* pDragPos)
    {
        CPoint pt = pDragPos->pos;
        VERIFY(ScreenToClient(pt));

        if (pt != m_oldPoint)
        {
            ResizeDockers(pDragPos);
            m_oldPoint = pt;
        }

        return 0;
    }

    // Begin moving the splitter bar.
    inline LRESULT CDocker::OnBarStart(DragPos* pDragPos)
    {
        CPoint pt = pDragPos->pos;
        VERIFY(ScreenToClient(pt));
        m_oldPoint = pt;

        return 0;
    }

    // Called when the close button is pressed.
    inline void CDocker::OnClose()
    {
        m_isClosing = TRUE;

        if (IsDocked())
        {
            // Undock the docker and hide it.
            Hide();
        }

        // Destroy the docker and its children.
        Destroy();
    }

    // Called when this docker is created.
    inline int CDocker::OnCreate(CREATESTRUCT&)
    {
        if (GetParent().GetExStyle() & WS_EX_LAYOUTRTL)
        {
            DWORD exStyle = static_cast<DWORD>(GetExStyle());
            SetExStyle(exStyle | WS_EX_LAYOUTRTL);
        }

        // Create the various child windows.
        GetDockClient().Create(*this);

        assert(&GetView());         // Use SetView in the docker's constructor to set the view window.
        GetView().Create(GetDockClient());

        // Create the slider bar belonging to this docker.
        GetDockBar().SetDocker(this);
        if (GetDockAncestor() != this)
            GetDockBar().Create(*GetDockAncestor());

        // Now remove the WS_POPUP style. It was required to allow this window
        // to be owned by the frame window.
        SetStyle(WS_CHILD);
        SetParent(GetParent());     // Reinstate the window's parent.

        // Set the default color for the splitter bar if it hasn't already been set.
        if (!GetDockBar().GetBrushBkgnd().GetHandle())
        {
            COLORREF rgbColour = GetSysColor(COLOR_BTNFACE);
            HWND frame = GetDockAncestor()->GetAncestor();

            ReBarTheme* pTheme = reinterpret_cast<ReBarTheme*>(::SendMessage(frame, UWM_GETRBTHEME, 0, 0));

            if (pTheme && pTheme->UseThemes && pTheme->clrBkgnd2 != 0)
                rgbColour = pTheme->clrBkgnd2;

            SetBarColor(rgbColour);
        }

        // Set the caption height based on text height.
        SetDefaultCaptionHeight();

        CDockContainer* pContainer = GetContainer();
        if (pContainer)
        {
            SetCaption(pContainer->GetDockCaption().c_str());
        }

        return 0;
    }

    // Called when the window is about to be destroyed.
    inline void CDocker::OnDestroy()
    {
        // Destroy any dock children first
        for (CDocker* pDocker : m_dockChildren)
        {
            pDocker->Destroy();
        }

        CDockContainer* pContainer = GetContainer();
        if (pContainer && IsUndocked())
        {
            if (pContainer->GetAllContainers().size() > 1)
            {
                // This container has children, so destroy them now.
                for (const ContainerInfo& ci : pContainer->GetAllContainers())
                {
                    if (ci.pContainer != pContainer)
                    {
                        // Reset container parent before destroying the dock window.
                        CDocker* pDocker = GetDockFromView(ci.pContainer);
                        assert(pDocker);
                        if (pDocker)
                        {
                            if (pContainer->IsWindow())
                                pContainer->SetParent(pDocker->GetDockClient());

                            pDocker->Destroy();
                        }
                    }
                }
            }
        }

        GetDockBar().Destroy();

        if (this == GetDockAncestor())
        {
            m_dockChildren.clear();
            m_allDockers.clear();
            m_allDockChildren.clear();
        }
        else
        {
            // Post a docker destroyed message.
            if (GetDockAncestor()->IsWindow())
                GetDockAncestor()->PostMessage(UWM_DOCKDESTROYED, reinterpret_cast<WPARAM>(this), 0);
        }
    }

    // Called in response to a UWM_DOCKACTIVATE message.
    inline LRESULT CDocker::OnDockActivated(UINT, WPARAM, LPARAM)
    {
        // Redraw captions to take account of focus change
        if (this == GetDockAncestor())
            DrawAllCaptions();

        return 0;
    }

    // Called when this docker is destroyed.
    inline LRESULT CDocker::OnDockDestroyed(UINT, WPARAM wparam, LPARAM)
    {
        CDocker* pDocker = reinterpret_cast<CDocker*>(wparam);

        assert( this == GetDockAncestor() );
        for (auto it = GetAllChildren().begin(); it != GetAllChildren().end(); ++it)
        {
            if ((*it).get() == pDocker)
            {
                GetAllChildren().erase(it);
                break;
            }
        }

        std::vector<CDocker*>& dockers = GetDockAncestor()->m_allDockers;
        for (auto it = dockers.begin(); it != dockers.end(); ++it)
        {
            if ((*it) == pDocker)
            {
                dockers.erase(it);
                break;
            }
        }

        return 0;
    }

    // Completes the docking. Docks the window on the dock target (if any).
    inline LRESULT CDocker::OnDockEnd(DragPos* pDragPos)
    {
        CDocker* pDocker = pDragPos->pDocker;
        assert(pDocker);
        if (!pDocker) return 0;

        UINT dockZone = pDragPos->dockZone;
        CRect rc = pDocker->GetWindowRect();

        switch (dockZone)
        {
        case DS_DOCKED_LEFT:
        case DS_DOCKED_RIGHT:
            pDocker->SetDockSize(rc.Width());
            Dock(pDocker, pDocker->GetDockStyle() | dockZone);
            break;
        case DS_DOCKED_TOP:
        case DS_DOCKED_BOTTOM:
            pDocker->SetDockSize(rc.Height());
            Dock(pDocker, pDocker->GetDockStyle() | dockZone);
            break;
        case DS_DOCKED_CONTAINER:
        {
            CDockContainer* pContainer = GetContainer();
            assert(pContainer);
            CDockContainer* pActive = nullptr;
            if (pDocker->GetContainer())
                pActive = pDocker->GetContainer()->GetActiveContainer();

            DockInContainer(pDocker, pDocker->GetDockStyle() | dockZone, FALSE);
            if (pActive)
                pContainer->SetActiveContainer(pActive);
            else
                pContainer->SelectPage(0);
        }
        break;
        case DS_DOCKED_LEFTMOST:
        case DS_DOCKED_RIGHTMOST:
            pDocker->SetDockSize(rc.Width());
            DockOuter(pDocker, pDocker->GetDockStyle() | dockZone);
            break;
        case DS_DOCKED_TOPMOST:
        case DS_DOCKED_BOTTOMMOST:
            pDocker->SetDockSize(rc.Height());
            DockOuter(pDocker, pDocker->GetDockStyle() | dockZone);
            break;
        }

        GetDockHint().Destroy();
        CloseAllTargets();

        return 0;
    }

    // Performs the docking move.
    inline LRESULT CDocker::OnDockMove(DragPos* pDragPos)
    {
        CheckAllTargets(pDragPos);
        return 0;
    }

    // Starts the undocking.
    inline LRESULT CDocker::OnDockStart(DragPos* pDragPos)
    {
        if (IsDocked() && !(GetDockStyle() & DS_NO_UNDOCK))
        {
            Undock(GetCursorPos());
            WPARAM wparam = static_cast<WPARAM>(HTCAPTION);
            LPARAM lparam = MAKELPARAM(pDragPos->pos.x, pDragPos->pos.y);
            SendMessage(WM_NCLBUTTONDOWN, wparam, lparam);
        }

        return 0;
    }

    // Called in response to a WM_DPICHANGED message that is sent to a top-level
    // window when the DPI changes.
    // Only top-level windows receive a WM_DPICHANGED message, so this message is
    // handled when an undocked docker is moved between monitors.
    inline LRESULT CDocker::OnDpiChanged(UINT, WPARAM, LPARAM lparam)
    {
        if (IsUndocked())   // Ignore dockers currently being undocked.
        {
            // An undocked docker has moved to a different monitor.
            LPRECT prc = reinterpret_cast<LPRECT>(lparam);
            SetRedraw(FALSE);
            SetWindowPos(HWND_TOP, *prc, SWP_SHOWWINDOW);
            SetRedraw(TRUE);
            RedrawWindow();
        }

        return 0;
    }

    // Called in response to a WM_DPICHANGED_BEFOREPARENT message that is sent to child
    // windows after a DPI change. A WM_DPICHANGED_BEFOREPARENT is only received when the
    // application is DPI_AWARENESS_PER_MONITOR_AWARE.
    inline LRESULT CDocker::OnDpiChangedBeforeParent(UINT, WPARAM, LPARAM)
    {
        SetDefaultCaptionHeight();

        // Reset the docker size.
        if (GetDockAncestor() != GetTopmostDocker())
            m_dockStartSize = m_dockStartSize * GetTopmostDocker()->m_newDpi / GetTopmostDocker()->m_oldDpi;

        return 0;
    }

    // Called when docker resizing is complete.
    inline LRESULT CDocker::OnExitSizeMove(UINT, WPARAM, LPARAM)
    {
        m_isDragging = FALSE;
        SendNotify(UWN_DOCKEND);

        return 0;
    }

    // Called before the window's DPI change is processed.
    inline LRESULT CDocker::OnGetDpiScaledSize(UINT, WPARAM wparam, LPARAM)
    {
        // Update the grab point with the DPI changes.
        m_oldDpi = GetWindowDpi(*this);
        m_newDpi = static_cast<int>(wparam);

        // Return FALSE to indicate computed size isn't modified.
        return FALSE;
    }

    // Called when the window is activated with a mouse click.
    inline LRESULT CDocker::OnMouseActivate(UINT, WPARAM, LPARAM)
    {
        CPoint pt = GetCursorPos();

        if (PtInRect(GetDockClient().GetWindowRect(), pt)) // only for this docker.
        {
            GetDockAncestor()->PostMessage(UWM_DOCKACTIVATE);
        }

        return 0;  // Return 0 to stop propagating this message to parent windows.
    }

    // Called with a left mouse button double click on the non-client area.
    inline LRESULT CDocker::OnNCLButtonDblClk(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        m_isDragging = FALSE;
        return FinalWindowProc(msg, wparam, lparam);
    }

    // Process docker notifications,
    inline LRESULT CDocker::OnNotify(WPARAM, LPARAM lparam)
    {
        DragPos* pdp = (DragPos*)lparam;

        if (IsWindowVisible())
        {
            LPNMHDR pHeader = reinterpret_cast<LPNMHDR>(lparam);
            switch (pHeader->code)
            {
            case UWN_BARSTART:      return OnBarStart(pdp);
            case UWN_BARMOVE:       return OnBarMove(pdp);
            case UWN_BAREND:        return OnBarEnd(pdp);
            case UWN_DOCKSTART:     return OnDockStart(pdp);
            case UWN_DOCKMOVE:      return OnDockMove(pdp);
            case UWN_DOCKEND:       return OnDockEnd(pdp);
            }
        }

        return 0;
    }

    // Called when the SystemParametersInfo function changes a system-wide
    // setting or when policy settings (including display settings) have changed.
    inline LRESULT CDocker::OnSettingChange(UINT, WPARAM, LPARAM)
    {
        if (this == GetTopmostDocker())
        {
            SetRedraw(FALSE);
            int newDPI = GetMonitorDpi(*this);
            int oldDPI = GetWindowDpi(*this);

            if (newDPI != oldDPI)
            {
                m_newDpi = newDPI;
                m_oldDpi = newDPI;

                for (const DockPtr& ptr : GetAllDockChildren())
                {
                    if (ptr->IsWindow() && IsChildOfDocker(ptr->GetHwnd()))
                    {
                        int size = (ptr->m_dockStartSize * newDPI) / oldDPI;
                        ptr->SetDockSize(size);
                    }
                }

                CRect rc = GetWindowRect();
                int width = (rc.Width() * newDPI) / oldDPI;
                int height = (rc.Height() * newDPI) / oldDPI;
                rc.right = rc.left + width;
                rc.bottom = rc.top + height;
                SetWindowPos(HWND_TOP, rc, SWP_FRAMECHANGED | SWP_SHOWWINDOW);
            }

            RecalcDockLayout();
            SetRedraw(TRUE);
            RedrawWindow();
        }
        return 0;
    }

    // Called when the docker is resized.
    inline LRESULT CDocker::OnSize(UINT, WPARAM, LPARAM)
    {
        if (this == GetTopmostDocker())
        {
            // Reposition the dock children.
            if (IsUndocked() && IsWindowVisible() && !m_isClosing)
                RecalcDockLayout();
        }

        return 0;
    }

    // Called when the system colors are changed.
    inline LRESULT CDocker::OnSysColorChange(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        if (this == GetDockAncestor())
        {
            COLORREF color = GetSysColor(COLOR_BTNFACE);
            HWND frame = GetDockAncestor()->GetAncestor();

            ReBarTheme* pTheme = reinterpret_cast<ReBarTheme*>(::SendMessage(frame, UWM_GETRBTHEME, 0, 0));

            if (pTheme && pTheme->UseThemes && pTheme->clrBand2 != 0)
                color = pTheme->clrBkgnd2;
            else
                color = GetSysColor(COLOR_BTNFACE);

            // Set the splitter bar color for each docker descendant.
            for (const DockPtr& dockPtr : GetAllChildren())
                dockPtr->SetBarColor(color);

            // Set the splitter bar color for the docker ancestor.
            SetBarColor(color);
        }

        return FinalWindowProc(msg, wparam, lparam);
    }

    // Called in response to a system command (docker window is moved or closed).
    inline LRESULT CDocker::OnSysCommand(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        // The "wparam & 0xFFF0" below is a requirement mentioned in the
        // description of WM_SYSCOMMAND in the Windows API documentation.

        switch(wparam & 0xFFF0)
        {
        case SC_MOVE:
            // An undocked docker is being moved.
            {
                UINT isEnabled = FALSE;
                m_isDragging = TRUE;
                SetCursor(LoadCursor(nullptr, IDC_ARROW));
                VERIFY(::SystemParametersInfo(SPI_GETDRAGFULLWINDOWS, 0, &isEnabled, 0));

                // Turn on DragFullWindows for this move.
                VERIFY(::SystemParametersInfo(SPI_SETDRAGFULLWINDOWS, TRUE, 0, 0));

                // Process this message.
                DefWindowProc(WM_SYSCOMMAND, wparam, lparam);

                // Return DragFullWindows to its previous state.
                VERIFY(::SystemParametersInfo(SPI_SETDRAGFULLWINDOWS, isEnabled, 0, 0));
                return 0;
            }
        default:
            break;
        }
        return FinalWindowProc(msg, wparam, lparam);
    }

    // Called after the docker is moved or resized.
    inline LRESULT CDocker::OnWindowPosChanged(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        if (m_isDragging)
        {
            // Send a Move notification to the parent.
            if (IsLeftButtonDown())
            {
                LPWINDOWPOS wPos = (LPWINDOWPOS)lparam;
                if (!(wPos->flags & SWP_NOMOVE))
                    SendNotify(UWN_DOCKMOVE);
            }
            else
            {
                CloseAllTargets();
            }
        }

        return FinalWindowProc(msg, wparam, lparam);
    }

    // Set the CREATESTURCT parameters before the window is created
    inline void CDocker::PreCreate(CREATESTRUCT& cs)
    {
        // Call base clase to set defaults.
        CWnd::PreCreate(cs);

        // Specify the WS_POPUP style to have this window owned.
        if (this != GetDockAncestor())
            cs.style |= WS_POPUP;

        cs.style |= WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
        cs.dwExStyle |= WS_EX_TOOLWINDOW;
    }

    inline void CDocker::PreRegisterClass(WNDCLASS& wc)
    {
        wc.lpszClassName = _T("Win32++ Docker");
        wc.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
    }

    // One of the steps required for undocking.
    inline void CDocker::PromoteFirstChild()
    {
        // Promote our first child to replace ourself.
        if (m_pDockParent)
        {
            std::vector<CDocker*>& children = m_pDockParent->m_dockChildren;

            for (auto it = children.begin(); it != children.end(); ++it)
            {
                if ((*it) == this)
                {
                    if (m_dockChildren.size() > 0)
                        // Swap our first child for ourself as a child of the parent.
                        (*it) = m_dockChildren[0];
                    else
                        // Remove ourself as a child of the parent.
                        children.erase(it);

                    // Done.
                    break;
                }
            }
        }

        // Transfer styles and data and children to the child docker.
        CDocker* pDockFirstChild = nullptr;
        if (m_dockChildren.size() > 0)
        {
            pDockFirstChild = m_dockChildren[0];
            pDockFirstChild->m_dockStyle = (pDockFirstChild->m_dockStyle & 0xFFFFFFF0) | (m_dockStyle & 0xF);
            pDockFirstChild->m_dockStartSize = m_dockStartSize;

            if (m_pDockParent)
            {
                pDockFirstChild->m_pDockParent = m_pDockParent;
                pDockFirstChild->SetParent(*m_pDockParent);
                pDockFirstChild->GetDockBar().SetParent(*m_pDockParent);
            }
            else
            {
                for (auto it = m_dockChildren.begin() + 1; it != m_dockChildren.end(); ++it)
                    (*it)->ShowWindow(SW_HIDE);

                pDockFirstChild->ConvertToPopup(GetWindowRect(), TRUE);
                pDockFirstChild->GetDockBar().ShowWindow(SW_HIDE);
            }

            m_dockChildren.erase(m_dockChildren.begin());
            MoveDockChildren(pDockFirstChild);
        }
    }

    // Called by RecalcDockLayout to reposition child dockers.
    inline void CDocker::RecalcDockChildLayout(CRect& rc)
    {
        // This function positions the Docker's dock children, the Dockers client area
        //  and draws the dockbar bars.

        // Notes:
        // 1) This function is called recursively.
        // 2) The client area and child dockers are positioned simultaneously with
        //      DeferWindowPos to avoid drawing errors in complex docker arrangements.
        // 3) The docker's client area contains the docker's caption (if any) and the docker's view window.

        // Note: All top level dockers are undocked, including the dock ancestor.

        bool isRTL = false;
        isRTL = ((GetExStyle() & WS_EX_LAYOUTRTL)) != 0;

        if (IsDocked())
        {
            rc.OffsetRect(-rc.left, -rc.top);
        }

        HDWP hdwp = BeginDeferWindowPos(static_cast<int>(m_dockChildren.size()) +2);

        // Step 1: Calculate the position of each Docker child, DockBar, and Client window.
        // The client area = the docker rect minus the area of dock children and the dock bar (splitter bar).
        for (CDocker* pDocker : m_dockChildren)
        {
            CRect rcChild = rc;
            int dockSize = pDocker->m_dockStartSize;
            int minSize = 30;

            // Calculate the size of the Docker children.
            switch (pDocker->GetDockStyle() & 0xF)
            {
            case DS_DOCKED_LEFT:
                if (isRTL)
                {
                    rcChild.left = rcChild.right - dockSize;
                    rcChild.left = std::min(rcChild.left, rc.right - minSize);
                    rcChild.left = std::max(rcChild.left, rc.left + minSize);
                }
                else
                {
                    rcChild.right = rcChild.left + dockSize;
                    rcChild.right = std::max(rcChild.right, rc.left + minSize);
                    rcChild.right = std::min(rcChild.right, rc.right - minSize);
                }
                break;
            case DS_DOCKED_RIGHT:
                if (isRTL)
                {
                    rcChild.right = rcChild.left + dockSize;
                    rcChild.right = std::max(rcChild.right, rc.left + minSize);
                    rcChild.right = std::min(rcChild.right, rc.right - minSize);
                }
                else
                {
                    rcChild.left = rcChild.right - dockSize;
                    rcChild.left = std::min(rcChild.left, rc.right - minSize);
                    rcChild.left = std::max(rcChild.left, rc.left + minSize);
                }

                break;
            case DS_DOCKED_TOP:
                rcChild.bottom = rcChild.top + dockSize;
                rcChild.bottom = std::max(rcChild.bottom, rc.top + minSize);
                rcChild.bottom = std::min(rcChild.bottom, rc.bottom - minSize);
                break;
            case DS_DOCKED_BOTTOM:
                rcChild.top = rcChild.bottom - dockSize;
                rcChild.top = std::min(rcChild.top, rc.bottom - minSize);
                rcChild.top = std::max(rcChild.top, rc.top + minSize);

                break;
            }

            if (pDocker->IsDocked())
            {
                // Position this docker's children.
                hdwp = pDocker->DeferWindowPos(hdwp, 0, rcChild, SWP_SHOWWINDOW|SWP_FRAMECHANGED);
                pDocker->m_childRect = rcChild;

                rc.SubtractRect(rc, rcChild);

                // Calculate the dimensions of the splitter bar.
                CRect barRect = rc;
                DWORD dockSide = pDocker->GetDockStyle() & 0xF;

                if (DS_DOCKED_LEFT   == dockSide)
                {
                    if (isRTL) barRect.left   = barRect.right - pDocker->GetBarWidth();
                    else     barRect.right  = barRect.left + pDocker->GetBarWidth();
                }

                if (DS_DOCKED_RIGHT  == dockSide)
                {
                    if (isRTL) barRect.right  = barRect.left + pDocker->GetBarWidth();
                    else     barRect.left   = barRect.right - pDocker->GetBarWidth();
                }

                if (DS_DOCKED_TOP    == dockSide) barRect.bottom = barRect.top + pDocker->GetBarWidth();
                if (DS_DOCKED_BOTTOM == dockSide) barRect.top    = barRect.bottom - pDocker->GetBarWidth();

                // Save the splitter bar position. We will reposition it later.
                pDocker->m_barRect = barRect;
                rc.SubtractRect(rc, barRect);
            }
        }

        // Step 2: Position the Dock client and dock bar.
        if (GetDockClient().IsWindow())
            hdwp = GetDockClient().DeferWindowPos(hdwp, 0, rc, SWP_SHOWWINDOW |SWP_FRAMECHANGED);
        VERIFY(EndDeferWindowPos(hdwp));

        // Position the dockbar. Only docked dockers have a dock bar.
        if (IsDocked())
        {
            CRect barRect;
            barRect.IntersectRect(m_barRect, GetDockParent()->GetViewRect());

            // The SWP_NOCOPYBITS forces a redraw of the dock bar.
            VERIFY(GetDockBar().SetWindowPos(HWND_TOP, barRect, SWP_SHOWWINDOW|SWP_FRAMECHANGED|SWP_NOCOPYBITS));
        }

        // Step 3: Now recurse through the docker's children. They might have children of their own.
        for (CDocker* pDocker : m_dockChildren)
        {
            pDocker->RecalcDockChildLayout(pDocker->m_childRect);
        }
    }

    // Repositions the dock children of a top level docker.
    inline void CDocker::RecalcDockLayout()
    {
        if (GetDockAncestor()->IsWindow())
        {
            CRect rc = GetTopmostDocker()->GetViewRect();
            GetTopmostDocker()->RecalcDockChildLayout(rc);
            GetTopmostDocker()->UpdateWindow();
        }
    }

    // Called when the docker's splitter bar is dragged.
    inline void CDocker::ResizeDockers(DragPos* pDragPos)
    {
        assert(pDragPos);
        if (!pDragPos) return;

        POINT pt = pDragPos->pos;
        VERIFY(ScreenToClient(pt));
        int posX = pt.x;
        int posY = pt.y;

        CDocker* pDocker = pDragPos->pDocker;
        assert(pDocker);
        if (!pDocker) return;

        RECT rcDock = pDocker->GetWindowRect();
        VERIFY(ScreenToClient(rcDock));

        int barWidth = pDocker->GetDockBar().GetWidth();
        int dockSize;

        BOOL isRTL = false;
        isRTL = ((GetExStyle() & WS_EX_LAYOUTRTL)) != 0;

        switch (pDocker->GetDockStyle() & 0xF)
        {
        case DS_DOCKED_LEFT:
            if (isRTL) dockSize = rcDock.right - std::max(posX, barWidth / 2) - (barWidth / 2);
            else     dockSize = std::max(posX, barWidth / 2) - rcDock.left - (barWidth / 2);

            dockSize = std::max(-barWidth, dockSize);
            pDocker->SetDockSize(dockSize);
            break;
        case DS_DOCKED_RIGHT:
            if (isRTL)  dockSize = std::max(posX, barWidth / 2) - rcDock.left - (barWidth / 2);
            else      dockSize = rcDock.right - std::max(posX, barWidth / 2) - (barWidth / 2);

            dockSize = std::max(-barWidth, dockSize);
            pDocker->SetDockSize(dockSize);
            break;
        case DS_DOCKED_TOP:
            dockSize = std::max(posY, barWidth / 2) - rcDock.top - (barWidth / 2);
            dockSize = std::max(-barWidth, dockSize);
            pDocker->SetDockSize(dockSize);
            break;
        case DS_DOCKED_BOTTOM:
            dockSize = rcDock.bottom - std::max(posY, barWidth / 2) - (barWidth / 2);
            dockSize = std::max(-barWidth, dockSize);
            pDocker->SetDockSize(dockSize);
            break;
        }

        RecalcDockLayout();
    }

    // Stores the docking container configuration in the registry.
    inline void CDocker::SaveContainerRegistrySettings(CRegKey& dockKey, CDockContainer* pContainer, UINT& container)
    {
        CRegKey containerKey;
        CString dockContainerName;
        dockContainerName.Format(_T("DockContainer%u"), container++);
        if (ERROR_SUCCESS != dockKey.Create(dockKey, dockContainerName))
            throw CUserException();

        if (ERROR_SUCCESS != containerKey.Open(dockKey, dockContainerName))
            throw CUserException();

        // Store the container group's parent.
        CDocker* pDocker = GetDockFromView(pContainer);
        if (pDocker == nullptr)
            throw CUserException();
        DWORD id = static_cast<DWORD>(pDocker->GetDockID());
        if (ERROR_SUCCESS != containerKey.SetDWORDValue(_T("Parent Container"), id))
            throw CUserException();

        // Store the active (selected) container.
        pDocker = GetDockFromView(pContainer->GetActiveContainer());
        if (pDocker == nullptr)
            id = 0;
        else
            id = static_cast<DWORD>(pDocker->GetDockID());

        if (ERROR_SUCCESS != containerKey.SetDWORDValue(_T("Active Container"), id))
            throw CUserException();

        // Store the tab order.
        for (size_t u2 = 0; u2 < pContainer->GetAllContainers().size(); ++u2)
        {
            dockContainerName = _T("Tab");
            dockContainerName << u2;
            CDockContainer* pTab = pContainer->GetContainerFromIndex(u2);
            if (pTab == nullptr)
                throw CUserException();
            pDocker = GetDockFromView(pTab);
            if (pDocker == nullptr)
                throw CUserException();
            DWORD tabID = static_cast<DWORD>(pDocker->GetDockID());

            if (ERROR_SUCCESS != containerKey.SetDWORDValue(dockContainerName, tabID))
                throw CUserException();
        }
    }

    // Stores the docking configuration in the registry.
    // NOTE: This function assumes that each docker has a unique DockID.
    inline BOOL CDocker::SaveDockRegistrySettings(LPCTSTR registryKeyName)
    {
        std::vector<CDocker*> sortedDockers = SortDockers();
        std::vector<DockInfo> allDockInfo;

        if (registryKeyName)
        {
            CRegKey appKey;
            CRegKey dockKey;
            const CString appKeyName = _T("Software\\") + CString(registryKeyName);
            const CString dockKeyName = _T("Dock Settings");
            try
            {
                if (!VerifyDockers())
                    throw CUserException();

                // Create the App's registry key.
                if (ERROR_SUCCESS != appKey.Create(HKEY_CURRENT_USER, appKeyName))
                    throw CUserException();

                if (ERROR_SUCCESS != appKey.Open(HKEY_CURRENT_USER, appKeyName))
                    throw CUserException();

                // Remove Old Docking info ...
                appKey.RecurseDeleteKey(dockKeyName);

                // Fill the DockInfo vector with the docking information.
                for (CDocker* pDocker : sortedDockers)
                {
                    DockInfo di{};
                    if (!pDocker->IsWindow())
                        throw CUserException();

                    di.dockID    = pDocker->GetDockID();
                    di.dockStyle = pDocker->GetDockStyle();
                    di.dockSize  = pDocker->GetDockSize();
                    di.rect      = pDocker->GetWindowRect();
                    if (pDocker->GetDockParent())
                        di.dockParentID = pDocker->GetDockParent()->GetDockID();

                    di.isInAncestor = (pDocker->GetDockParent() == GetDockAncestor());

                    allDockInfo.push_back(di);
                }

                if (ERROR_SUCCESS != dockKey.Create(appKey, dockKeyName))
                    throw CUserException();

                if (ERROR_SUCCESS != dockKey.Open(appKey, dockKeyName))
                    throw CUserException();

                // Add the dock settings information to the registry.
                for (size_t t = 0; t < allDockInfo.size(); ++t)
                {
                    DockInfo di = allDockInfo[t];
                    CString dockChildName;
                    dockChildName << _T("DockChild") << t;
                    if (ERROR_SUCCESS != dockKey.SetBinaryValue(dockChildName, &di, sizeof(DockInfo)))
                        throw CUserException();
                }

                // Add dock container info to the registry.
                UINT container = 0;

                if (GetContainer())
                    SaveContainerRegistrySettings(dockKey, GetContainer(), container);

                for (CDocker* pDocker : sortedDockers)
                {
                    CDockContainer* pContainer = pDocker->GetContainer();

                    if (pContainer && ( !(pDocker->GetDockStyle() & DS_DOCKED_CONTAINER) ))
                    {
                        SaveContainerRegistrySettings(dockKey, pContainer, container);
                    }
                }

                // Add the dock ancestor's style to the registry.
                DWORD style = GetDockStyle();
                if (ERROR_SUCCESS != dockKey.SetDWORDValue(_T("DockAncestor"), style))
                    throw CUserException();
            }

            catch (const CUserException&)
            {
                TRACE("*** WARNING: Failed to save dock settings in registry. ***\n");

                // Roll back the registry changes by deleting the subkeys.
                if (appKey.GetKey())
                {
                    appKey.RecurseDeleteKey(dockKeyName);
                }

                return FALSE;
            }
        }

        return TRUE;
    }

    // Sends a docking notification to the docker below the cursor.
    inline void CDocker::SendNotify(UINT messageID)
    {
        DragPos dragPos{};
        dragPos.hdr.code = messageID;
        dragPos.hdr.hwndFrom = GetHwnd();
        dragPos.pos = GetCursorPos();
        dragPos.dockZone = m_dockZone;
        dragPos.pDocker = this;
        m_dockZone = 0;

        CDocker* pDocker = GetDockUnderDragPoint(dragPos.pos);

        if (pDocker)
            pDocker->SendMessage(WM_NOTIFY, 0, reinterpret_cast<LPARAM>(&dragPos));
        else
        {
            if (GetDockHint().IsWindow())
                GetDockHint().Destroy();

            CloseAllTargets();
        }
    }

    // This performs some of the tasks required for undocking.
    // It is also used when a docker is hidden.
    inline CDocker* CDocker::SeparateFromDock()
    {
        CDocker* pDockUndockedFrom = GetDockParent();
        if (!pDockUndockedFrom && (m_dockChildren.size() > 0))
            pDockUndockedFrom = m_dockChildren[0];

        PromoteFirstChild();
        m_pDockParent = nullptr;

        if (GetDockBar().IsWindow())
            GetDockBar().ShowWindow(SW_HIDE);

        m_dockStyle = m_dockStyle & 0xFFFFFFF0;
        m_dockStyle &= ~DS_DOCKED_CONTAINER;

        return pDockUndockedFrom;
    }

    // Sets the caption text.
    inline void CDocker::SetCaption(LPCTSTR caption)
    {
        GetDockClient().SetCaption(caption);

        if (IsWindow() && (this != GetDockAncestor()))
            SetWindowText(caption);
    }

    // Sets the caption's foreground and background colors.
    // foregnd1 specifies the foreground color(focused).
    // backgnd1 specifies the background color(focused).
    // foregnd2 specifies the foreground color(not focused).
    // backgnd2 specifies the background color(not focused).
    // penColor specifies the pen color used for drawing the outline.
    inline void CDocker::SetCaptionColors(COLORREF foregnd1, COLORREF backgnd1, COLORREF foreGnd2, COLORREF backGnd2, COLORREF penColor /*= RGB(160, 150, 140)*/) const
    {
        GetDockClient().SetCaptionColors(foregnd1, backgnd1, foreGnd2, backGnd2, penColor);
    }

    // Sets the height of the caption
    inline void CDocker::SetCaptionHeight(int height)
    {
        m_ncHeight = height;
    }

    // Sets the caption height based on the current text height.
    inline void CDocker::SetDefaultCaptionHeight()
    {
        SetCaptionHeight(std::max(20, GetTextHeight() + 5));
    }

    // Sets the dock client window for this docker.
    inline void CDocker::SetDockClient(CDockClient& dockClient)
    {
        m_pDockClient = &dockClient;
        m_pDockClient->SetDocker(this);
    }

    // Sets the size of a docked docker
    inline void CDocker::SetDockSize(int dockSize)
    {
        m_dockStartSize = dockSize;
    }

    // Sets the docker's style from one or more of the following:
    // DS_DOCKED_LEFT,DS_DOCKED_RIGHT, DS_DOCKED_TOP, DS_DOCKED_BOTTOM,
    // DS_NO_DOCKCHILD_LEFT, DS_NO_DOCKCHILD_RIGHT, DS_NO_DOCKCHILD_TOP,
    // DS_NO_DOCKCHILD_BOTTOM, DS_NO_RESIZE, DS_NO_CAPTION, DS_NO_CLOSE,
    // DS_NO_UNDOCK, DS_CLIENTEDGE, DS_DOCKED_CONTAINER,
    // DS_DOCKED_LEFTMOST, DS_DOCKED_RIGHTMOST, DS_DOCKED_TOPMOST,
    // DS_DOCKED_BOTTOMMOST.
    inline void CDocker::SetDockStyle(DWORD dockStyle)
    {
        if (IsWindow())
        {
            if ((dockStyle & DS_CLIENTEDGE) != (m_dockStyle & DS_CLIENTEDGE))
            {
                if (dockStyle & DS_CLIENTEDGE)
                {
                    DWORD exStyle = GetDockClient().GetExStyle() | WS_EX_CLIENTEDGE;
                    GetDockClient().SetExStyle(exStyle);
                    GetDockClient().RedrawWindow(RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_FRAME);
                }
                else
                {
                    DWORD exStyle = GetDockClient().GetExStyle();
                    exStyle &= ~WS_EX_CLIENTEDGE;
                    GetDockClient().SetExStyle(exStyle);
                    GetDockClient().RedrawWindow(RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_FRAME);
                }
            }
        }

        m_dockStyle = dockStyle;
        if (IsWindow())
        {
            RecalcDockLayout();
        }
    }

    // Assigns the view window to the docker.
    // The docker's view can be changed during runtime.
    // The docker's view can be a DockContainer or another child window.
    // Note: DockContainers have their own view which is not set here.
    inline void CDocker::SetView(CWnd& view)
    {
        GetDockClient().SetView(view);
        CDockContainer* pContainer = GetContainer();
        if (pContainer)
        {
            SetCaption(pContainer->GetDockCaption().c_str());
            pContainer->SetDocker(this);
        }
    }

    // Position and draw the undocked window, unless it is about to be closed.
    inline void CDocker::SetUndockPosition(CPoint pt, BOOL showUndocked)
    {
        m_isUndocking = TRUE;
        CRect rc;
        rc = GetDockClient().GetWindowRect();
        CRect testRect = rc;
        testRect.bottom = std::min(testRect.bottom, testRect.top + m_ncHeight);
        if ( !testRect.PtInRect(pt))
            rc.SetRect(pt.x - rc.Width()/2, pt.y - m_ncHeight/2, pt.x + rc.Width()/2, pt.y - m_ncHeight/2 + rc.Height());

        if (!m_isClosing)
            ConvertToPopup(rc, showUndocked);

        m_isUndocking = FALSE;

        // Send the undock notification to the frame.
        NMHDR nmhdr{};
        nmhdr.hwndFrom = GetHwnd();
        nmhdr.code = UWN_UNDOCKED;
        nmhdr.idFrom = static_cast<UINT_PTR>(m_dockID);
        HWND frame = GetDockAncestor()->GetAncestor();
        assert(frame);
        WPARAM wparam = static_cast<WPARAM>(m_dockID);
        LPARAM lparam = reinterpret_cast<LPARAM>(&nmhdr);
        ::SendMessage(frame, WM_NOTIFY, wparam, lparam);

        // Initiate the window move.
        SetCursorPos(pt.x, pt.y);
        VERIFY(ScreenToClient(pt));
        wparam = static_cast<WPARAM>(SC_MOVE | 0x0002);
        lparam = MAKELPARAM(pt.x, pt.y);
        PostMessage(WM_SYSCOMMAND, wparam, lparam);
    }

    // Returns a vector of sorted dockers, used by SaveRegistrySettings.
    inline std::vector<CDocker*> CDocker::SortDockers()
    {
        std::vector<CDocker*> vSorted;

        // Add undocked top level dockers
        for (const DockPtr& ptr : GetAllChildren())
        {
            if (!ptr->GetDockParent())
                vSorted.push_back(ptr.get());
        }

        // Add dock ancestor's children.
        vSorted.insert(vSorted.end(), GetDockAncestor()->GetDockChildren().begin(), GetDockAncestor()->GetDockChildren().end());

        // Add other dock children.
        int index = 0;
        auto itSort = vSorted.begin();
        while (itSort != vSorted.end())
        {
            vSorted.insert(vSorted.end(), (*itSort)->GetDockChildren().begin(), (*itSort)->GetDockChildren().end());
            itSort = vSorted.begin() + (++index);
        }

        // Add dockers docked in containers.
        std::vector<CDocker*> vDockContainers;
        for (CDocker* pDocker : vSorted)
        {
            if (pDocker->GetContainer())
                vDockContainers.push_back(pDocker);
        }

        for (CDocker* pDocker : vDockContainers)
        {
            CDockContainer* pContainer = pDocker->GetContainer();

            for (size_t i = 0; i < pContainer->GetAllContainers().size(); ++i)
            {
                CDockContainer* pChild = pContainer->GetContainerFromIndex(i);

                if (pChild != pContainer)
                {
                    CDocker* pDocker1 = GetDockFromView(pChild);
                    assert(pDocker1);
                    vSorted.push_back(pDocker1);
                }
            }
        }

        if (GetContainer())
        {
            CDockContainer* pContainer = GetContainer();

            for (size_t i = 0; i < pContainer->GetAllContainers().size(); ++i)
            {
                CDockContainer* pChild = pContainer->GetContainerFromIndex(i);

                if (pChild != pContainer)
                {
                    CDocker* pDocker = GetDockFromView(pChild);
                    assert(pDocker);
                    vSorted.push_back(pDocker);
                }
            }
        }

        return vSorted;
    }

    // Undocks a docker.
    // Called when the user undocks a docker, or when a docker is closed.
    inline void CDocker::Undock(CPoint pt, BOOL showUndocked)
    {
        CDocker* pDockUndockedFrom = SeparateFromDock();

        // Position and draw the undocked window, unless it is about to be closed.
        SetUndockPosition(pt, showUndocked);

        // Give the view window focus unless its child already has it.
        if (!GetView().IsChild(GetFocus()))
            GetView().SetFocus();

        RecalcDockLayout();
        if ((pDockUndockedFrom) && (pDockUndockedFrom->GetTopmostDocker() != GetTopmostDocker()))
            pDockUndockedFrom->RecalcDockLayout();
    }

    // Undocks a CDockContainer.
    // Called when the user undocks a container, or when a container is closed.
    inline void CDocker::UndockContainer(CDockContainer* pContainer, CPoint pt, BOOL showUndocked)
    {
        assert(pContainer);
        if (!pContainer) return;

        assert(this == GetDockFromView(pContainer->GetContainerParent()));

        if (GetDockFromView(pContainer) == GetDockAncestor()) return;

        CDocker* pDockUndockedFrom = this;
        if (&GetView() == pContainer)
        {
            // The parent container is being undocked, so we need
            // to transfer our child containers to a different docker.

            // Choose a new docker from among the dockers for child containers.
            CDocker* pDockNew = nullptr;
            CDocker* pDockOld = GetDockFromView(pContainer);
            assert(pDockOld);
            std::vector<ContainerInfo> AllContainers = pContainer->GetAllContainers();
            auto it = AllContainers.begin();
            while ((pDockNew == nullptr) && (it != AllContainers.end()))
            {
                if ((*it).pContainer != pContainer)
                {
                    pDockNew = (*it).pContainer->GetDocker();
                    assert(pDockNew);
                }

                ++it;
            }

            if (pDockNew)
            {
                // Move containers from the old docker to the new docker.
                pDockNew->m_isUndocking = TRUE;  // IsUndocked will report FALSE.
                CDockContainer* pContainerNew = pDockNew->GetContainer();
                assert(pContainerNew);
                for (const ContainerInfo& ci : AllContainers)
                {
                    if (ci.pContainer != pContainer)
                    {
                        CDockContainer* pChildContainer = ci.pContainer;
                        pContainer->RemoveContainer(pChildContainer);
                        if (pContainerNew != pChildContainer)
                        {
                            pContainerNew->AddContainer(pChildContainer);
                            CDocker* pDocker = GetDockFromView(pChildContainer);
                            assert(pDocker);
                            pDocker->SetParent(*pDockNew);
                            pDocker->m_pDockParent = pDockNew;
                        }
                    }
                }

                // Now transfer the old docker's settings to the new one.
                pDockUndockedFrom = pDockNew;
                pDockNew->m_dockStyle       = pDockOld->m_dockStyle;
                pDockNew->m_dockStartSize   = pDockOld->m_dockStartSize;
                if (pDockOld->IsDocked())
                {
                    pDockNew->m_pDockParent     = pDockOld->m_pDockParent;
                    pDockNew->SetParent(pDockOld->GetParent());
                }
                else
                {
                    CRect rc = pDockOld->GetWindowRect();
                    pDockNew->ShowWindow(SW_HIDE);
                    DWORD style = WS_POPUP| WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_VISIBLE;
                    pDockNew->SetStyle(style);
                    pDockNew->m_pDockParent = nullptr;
                    pDockNew->SetParent(nullptr);
                    VERIFY(pDockNew->SetWindowPos(HWND_TOP, rc, SWP_SHOWWINDOW|SWP_FRAMECHANGED| SWP_NOOWNERZORDER));
                }
                pDockNew->GetDockBar().SetParent(pDockOld->GetParent());
                pDockNew->GetView().SetFocus();

                // Transfer the Dock children to the new docker.
                pDockOld->MoveDockChildren(pDockNew);

                // Insert pDockNew into its DockParent's DockChildren vector.
                if (pDockNew->m_pDockParent)
                {
                    std::vector<CDocker*>& children = pDockNew->m_pDockParent->m_dockChildren;
                    for (auto p = children.begin(); p != children.end(); ++p)
                    {
                        if (*p == this)
                        {
                            children.insert(p, pDockNew);
                            break;
                        }
                    }
                }

                pDockNew->m_isUndocking = FALSE;  // IsUndocked reports undocked state.
            }
        }
        else
        {
            // This is a child container, so simply remove it from the parent.
            CDockContainer* pContainerParent = GetContainer();
            assert(pContainerParent);
            if (pContainerParent)
            {
                pContainerParent->RemoveContainer(pContainer, showUndocked);
                pContainerParent->SetTabSize();
                pContainerParent->SetFocus();
                if (pContainerParent->GetViewPage().IsWindow())
                    pContainerParent->GetViewPage().SetParent(*pContainerParent);
            }
        }

        // Finally do the actual undocking.
        CDocker* pDocker = GetDockFromView(pContainer);
        assert(pDocker);
        if (!pDocker) return;

        CRect rc = GetDockClient().GetWindowRect();
        VERIFY(ScreenToClient(rc));
        VERIFY(pDocker->GetDockClient().SetWindowPos(HWND_TOP, rc, SWP_SHOWWINDOW));
        pDocker->Undock(pt, showUndocked);
        pDocker->GetDockBar().SetParent(*GetDockAncestor());
        pDockUndockedFrom->ShowWindow();
        pDockUndockedFrom->RecalcDockLayout();
        pDocker->BringWindowToTop();
    }

    // Undocks a CDockContainer group.
    // Called when the user undocks a container group from a CDdockFrame.
    inline void CDocker::UndockContainerGroup()
    {
        CDockContainer* pContainer = GetContainer();
        assert(pContainer);
        if (pContainer)
        {
            CDockContainer* pParent = pContainer->GetContainerParent();
            CDockContainer* pActive = pContainer->GetActiveContainer();
            if (pActive)
            {
                assert(pParent->GetAllContainers().size() > 0);
                size_t lastTab = pParent->GetAllContainers().size() - 1;
                CDockContainer* pContainerLast = pContainer->GetContainerFromIndex(lastTab);
                GetDockAncestor()->UndockContainer(pContainerLast, GetCursorPos(), FALSE);

                while (pParent->GetAllContainers().size() > 0)
                {
                    lastTab = pParent->GetAllContainers().size() - 1;
                    CDockContainer* pContainerNext = pContainer->GetContainerFromIndex(lastTab);

                    CDocker* pDocker = pContainerNext->GetDocker();
                    pDocker->Hide();
                    pContainerLast->GetDocker()->DockInContainer(pDocker, pDocker->GetDockStyle());
                }

                pContainerLast->SetActiveContainer(pActive);
                pContainerLast->GetDocker()->ShowWindow(SW_SHOW);
                pContainerLast->RedrawWindow();
                GetDockAncestor()->RecalcDockLayout();
            }
        }
    }

    // A diagnostic routine that verifies the integrity of the docking layout.
    inline BOOL CDocker::VerifyDockers()
    {
        BOOL isVerified = TRUE;

        // Check dock ancestor
        for (const DockPtr& dockPtr : GetAllChildren())
        {
            if (GetDockAncestor() != dockPtr->m_pDockAncestor)
                isVerified = FALSE;
        }

        // Check presence of dock parent.
        for (const DockPtr& dockPtr : GetAllChildren())
        {
            if (dockPtr->IsUndocked() && dockPtr->m_pDockParent != nullptr)
                isVerified = FALSE;

            if (dockPtr->IsDocked() && dockPtr->m_pDockParent == nullptr)
                isVerified = FALSE;
        }

        // Check dock parent/child relationship.
        for (const DockPtr& dockPtr : GetAllChildren())
        {
            for (CDocker* pDocker : dockPtr->m_dockChildren)
            {
                if (pDocker->m_pDockParent != dockPtr.get())
                    isVerified = FALSE;

                if (pDocker->GetParent() != dockPtr.get()->GetHwnd())
                    isVerified = FALSE;
            }
        }

        // Check dock parent chain.
        for (const DockPtr& dockPtr : GetAllChildren())
        {
            CDocker* pDockTopLevel = dockPtr->GetTopmostDocker();
            if (pDockTopLevel->IsDocked())
                isVerified = FALSE;
        }

        assert(isVerified);
        return isVerified;
    }

    inline LRESULT CDocker::WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        switch (msg)
        {
        case WM_ACTIVATE:               return OnActivate(msg, wparam, lparam);
        case WM_DPICHANGED:             return OnDpiChanged(msg, wparam, lparam);
        case WM_DPICHANGED_BEFOREPARENT: return OnDpiChangedBeforeParent(msg, wparam, lparam);
        case WM_EXITSIZEMOVE:           return OnExitSizeMove(msg, wparam, lparam);
        case WM_GETDPISCALEDSIZE:       return OnGetDpiScaledSize(msg, wparam, lparam);
        case WM_MOUSEACTIVATE:          return OnMouseActivate(msg, wparam, lparam);
        case WM_NCLBUTTONDBLCLK:        return OnNCLButtonDblClk(msg, wparam, lparam);
        case WM_SETTINGCHANGE:          return OnSettingChange(msg, wparam, lparam);
        case WM_SIZE:                   return OnSize(msg, wparam, lparam);
        case WM_SYSCOLORCHANGE:         return OnSysColorChange(msg, wparam, lparam);
        case WM_SYSCOMMAND:             return OnSysCommand(msg, wparam, lparam);
        case WM_WINDOWPOSCHANGED:       return OnWindowPosChanged(msg, wparam, lparam);

            // Messages defined by Win32++
        case UWM_DOCKACTIVATE:          return OnDockActivated(msg, wparam, lparam);
        case UWM_DOCKDESTROYED:         return OnDockDestroyed(msg, wparam, lparam);
        case UWM_GETCDOCKER:            return reinterpret_cast<LRESULT>(this);
        }

        return CWnd::WndProcDefault(msg, wparam, lparam);
    }


    ///////////////////////////////////////////
    // Declaration of the CDockContainer class.

    // Constructor.
    inline CDockContainer::CDockContainer() : m_currentPage(0), m_pDocker(nullptr),
                                         m_pContainerParent(nullptr), m_tabIcon(nullptr),
                                         m_pressedTab(-1), m_isHideSingleTab(FALSE)
    {
        m_pViewPage = &m_viewPage;
        m_pContainerParent = this;
        m_viewPage.SetContainer(this);
    }

    // Destructor.
    inline CDockContainer::~CDockContainer()
    {
    }

    // Adds a container to the group. Set Insert to TRUE to insert the container
    // as the first tab, or FALSE to add it as the last tab.
    inline void CDockContainer::AddContainer(CDockContainer* pContainer, BOOL insert /* = FALSE */, BOOL selectPage)
    {
        assert(pContainer);
        assert(this == m_pContainerParent); // Must be performed by parent container.
        if (!pContainer || !m_pContainerParent) return;

        ContainerInfo ci;
        ci.pContainer = pContainer;
        ci.tabText = pContainer->GetTabText();
        ci.tabImage = GetImages().Add( pContainer->GetTabIcon() );
        int newPage = 0;
        if (insert)
        {
            m_allInfo.insert(m_allInfo.begin(), ci);
        }
        else
        {
            newPage = static_cast<int>(m_allInfo.size());
            m_allInfo.push_back(ci);
        }

        if (IsWindow())
        {
            TCITEM tie{};
            tie.mask = TCIF_TEXT | TCIF_IMAGE;
            tie.iImage = ci.tabImage;
            size_t newPageIndex = static_cast<size_t>(newPage);
            tie.pszText = const_cast<LPTSTR>(m_allInfo[newPageIndex].tabText.c_str());
            InsertItem(newPage, &tie);

            if (selectPage)
                SelectPage(newPage);

            SetTabSize();
        }

        pContainer->m_pContainerParent = this;
        if (pContainer->IsWindow())
        {
            // Set the parent container relationships.
            pContainer->GetViewPage().SetParent(*this);
            pContainer->GetViewPage().ShowWindow(SW_HIDE);
            RecalcLayout();
        }
    }

    // Adds Resource IDs to toolbar buttons.
    // A resource ID of 0 is a separator.
    inline void CDockContainer::AddToolBarButton(UINT id, BOOL isEnabled /* = TRUE */)
    {
        GetToolBarData().push_back(id);
        GetToolBar().AddButton(id, isEnabled);
    }

    // Creates the toolbar within the viewpage.
    inline void CDockContainer::CreateToolBar()
    {
        // Create the toolbar.
        if (GetViewPage().IsWindow())
        {
            m_toolBarData.clear();
            GetToolBar().Create(GetViewPage());
            DWORD style = GetToolBar().GetStyle();
            style |= CCS_NODIVIDER;
            GetToolBar().SetStyle(style);
            SetupToolBar();
            if (GetToolBarData().size() > 0)
            {
                // Load the default images if no images are loaded.
                if (!GetToolBar().SendMessage(TB_GETIMAGELIST, 0, 0))
                    SetToolBarImages(RGB(192, 192, 192), IDW_MAIN, 0, 0);

                GetToolBar().Autosize();
            }
            else
                GetToolBar().ShowWindow(SW_HIDE);
        }
    }

    // Draw the tabs.
    inline void CDockContainer::DrawTabs(CDC& dc)
    {
        // Draw the tab buttons:
        for (int i = 0; i < GetItemCount(); ++i)
        {
            CRect rcItem;
            GetItemRect(i, rcItem);
            if (!rcItem.IsRectEmpty())
            {
                if (i == GetCurSel())
                {
                    dc.CreateSolidBrush(RGB(248, 248, 248));
                    dc.SetBkColor(RGB(248, 248, 248));
                }
                else
                {
                    dc.CreateSolidBrush(RGB(200, 200, 200));
                    dc.SetBkColor(RGB(200, 200, 200));
                }

                dc.CreatePen(PS_SOLID, 1, RGB(160, 160, 160));
                dc.RoundRect(rcItem.left, rcItem.top, rcItem.right + 1, rcItem.bottom, 6, 6);

                CSize szImage = GetImages().GetIconSize();
                int padding = DpiScaleInt(4);

                if (rcItem.Width() >= szImage.cx + 2 * padding)
                {
                    CString str = GetDockTabText(i);
                    int image = GetDockTabImageID(i);
                    int yOffset = (rcItem.Height() - szImage.cy) / 2;

                    // Draw the icon.
                    int drawleft = rcItem.left + padding;
                    int drawtop = rcItem.top + yOffset;
                    GetImages().Draw(dc, image, CPoint(drawleft, drawtop), ILD_NORMAL);

                    // Calculate the size of the text.
                    CRect rcText = rcItem;

                    if (image >= 0)
                        rcText.left += szImage.cx + padding;

                    rcText.left += padding;

                    // Draw the text.
                    dc.SelectObject(GetTabFont());
                    dc.DrawText(str, -1, rcText, DT_LEFT | DT_VCENTER |
                        DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
                }
            }
        }
    }

    // Returns a pointer to the container at the specified tab number.
    inline CDockContainer* CDockContainer::GetContainerFromIndex(size_t index) const
    {
        CDockContainer* pContainer = nullptr;
        if (index < m_allInfo.size())
            pContainer = m_allInfo[index].pContainer;

        return pContainer;
    }

    // Returns a pointer to the currently active container.
    inline CDockContainer* CDockContainer::GetActiveContainer() const
    {
        assert(m_pContainerParent);
        if (!m_pContainerParent) return nullptr;

        if (m_pContainerParent->m_allInfo.size() > 0)
        {
            size_t index = static_cast<size_t>(m_pContainerParent->m_currentPage);
            return m_pContainerParent->m_allInfo[index].pContainer;
        }
        else
            return 0;
    }

    // Returns a pointer to the active view window, or nullptr if there is no active view.
    inline CWnd* CDockContainer::GetActiveView() const
    {
        if (GetActiveContainer())
            return GetActiveContainer()->GetView();
        else
            return 0;
    }

    // Returns a pointer to the container with the specified view.
    inline CDockContainer* CDockContainer::GetContainerFromView(CWnd* pView) const
    {
        assert(pView);

        CDockContainer* pViewContainer = nullptr;
        for (const ContainerInfo& ci : GetAll())
        {
            CDockContainer* pContainer = ci.pContainer;
            if (pContainer->GetView() == pView)
            {
                pViewContainer = pContainer;
                break;
            }
        }

        return pViewContainer;
    }

    // Returns the tab index of the specified container.
    inline int CDockContainer::GetContainerIndex(CDockContainer* pContainer) const
    {
        assert(pContainer);
        int index = -1;

        for (auto it = GetAll().begin(); it != GetAll().end(); ++it)
        {
            if ((*it).pContainer == pContainer)
            {
                index = static_cast<int>(std::distance(GetAll().begin(), it));
                break;
            }
        }

        return index;
    }

    // Returns the size (width and height) of the caption text.
    inline SIZE CDockContainer::GetMaxTabTextSize() const
    {
        CSize sz;

        for (auto it = m_allInfo.begin(); it != m_allInfo.end(); ++it)
        {
            CSize tempSize;
            CClientDC dc(*this);
            dc.SelectObject(GetTabFont());
            tempSize = dc.GetTextExtentPoint32(it->tabText, lstrlen(it->tabText));
            if (tempSize.cx > sz.cx)
                sz = tempSize;
        }

        return sz;
    }

    // Returns a container tab's image index.
    inline int CDockContainer::GetDockTabImageID(int tab) const
    {
        size_t index = static_cast<size_t>(tab);
        assert (index < GetAllContainers().size());
        return GetAllContainers()[index].tabImage;
    }

    // Returns a container tab's text.
    inline CString CDockContainer::GetDockTabText(int tab) const
    {
        size_t index = static_cast<size_t>(tab);
        assert (index < GetAllContainers().size());
        return GetAllContainers()[index].tabText;
    }

    // Returns the size of a bitmap image.
    inline CSize CDockContainer::GetTBImageSize(CBitmap* pBitmap) const
    {
        assert(pBitmap);
        if (!pBitmap) return CSize(0, 0);

        assert(pBitmap->GetHandle());
        BITMAP data = pBitmap->GetBitmapData();
        int cy = data.bmHeight;
        int dataHeight = data.bmHeight;
        int cx = std::max(dataHeight, 16);

        return CSize(cx, cy);
    }

    // Called when a HWND is attached to this CWnd.
    inline void CDockContainer::OnAttach()
    {
        m_allInfo.clear();

        // The parent window must be a docker.
        m_pDocker = reinterpret_cast<CDocker*>((GetParent().GetParent().SendMessage(UWM_GETCDOCKER)));
        assert(dynamic_cast<CDocker*>(m_pDocker));

        // Create and assign the tab's image list.
        int iconHeight = DpiScaleInt(16);
        iconHeight = iconHeight - iconHeight % 8;
        GetImages().Create(iconHeight, iconHeight, ILC_MASK | ILC_COLOR32, 0, 0);

        // Update the font and icons in the tabs.
        UpdateTabs();

        // Add a tab for this container except for the DockAncestor.
        if (!GetDocker() || GetDocker()->GetDockAncestor() != GetDocker())
        {
            assert(GetView());          // Use SetView in the constructor to set the view window.

            ContainerInfo ci;
            ci.pContainer = this;
            ci.tabText = GetTabText();
            ci.tabImage = GetImages().Add(GetTabIcon());
            m_allInfo.push_back(ci);

            // Create the page window.
            GetViewPage().Create(*this);

            // Create the toolbar.
            CreateToolBar();
        }

        SetFixedWidth(TRUE);
        SetOwnerDraw(TRUE);

        // Add tabs for each container.
        for (size_t i = 0; i < m_allInfo.size(); ++i)
        {
            // Add tabs for each view.
            TCITEM tie{};
            tie.mask = TCIF_TEXT | TCIF_IMAGE;
            tie.iImage = m_allInfo[i].tabImage;
            tie.pszText = const_cast<LPTSTR>(m_allInfo[i].tabText.c_str());
            InsertItem(static_cast<int>(i), &tie);
        }
    }

    // Called in response to a WM_DPICHANGED_BEFOREPARENT message that is sent to child
    // windows after a DPI change. A WM_DPICHANGED_BEFOREPARENT is only received when the
    // application is DPI_AWARENESS_PER_MONITOR_AWARE.
    inline LRESULT CDockContainer::OnDpiChangedBeforeParent(UINT, WPARAM, LPARAM)
    {
        UpdateTabs();

        // Destroy and recreate the toolbar.
        GetViewPage().GetToolBar().Destroy();
        CreateToolBar();
        RecalcLayout();

        return 0;
    }

    // Called when the left mouse button is pressed.
    // Overrides CTab::OnLButtonDown.
    inline LRESULT CDockContainer::OnLButtonDown(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        CPoint pt(lparam);
        TCHITTESTINFO info{};
        info.pt = pt;
        m_pressedTab = HitTest(info);

        return FinalWindowProc(msg, wparam, lparam);
    }

    // Called when the left mouse button is released.
    // Overrides CTab::OnLButtonUp and takes no action.
    inline LRESULT CDockContainer::OnLButtonUp(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        return FinalWindowProc(msg, wparam, lparam);
    }

    // Called when the mouse cursor is moved over the window.
    // Overrides CTab::OnMouseLeave.
    inline LRESULT CDockContainer::OnMouseLeave(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        if (IsLeftButtonDown() && (m_pressedTab >= 0))
        {
            if (GetDocker() && !(GetDocker()->GetDockStyle() & DS_NO_UNDOCK))
            {
                CDockContainer* pContainer = GetActiveContainer();
                GetDocker()->UndockContainer(pContainer, GetCursorPos(), TRUE);
            }
        }

        m_pressedTab = -1;
        return CTab::OnMouseLeave(msg, wparam, lparam);
    }

    // Called when the mouse cursor is moved over the window.
    inline LRESULT CDockContainer::OnMouseMove(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        CPoint pt;
        pt.x = GET_X_LPARAM(lparam);
        pt.y = GET_Y_LPARAM(lparam);

        // Skip if mouse hasn't moved
        if ((pt.x == m_oldMousePos.x) && (pt.y == m_oldMousePos.y))
            return FALSE;

        m_oldMousePos.x = pt.x;
        m_oldMousePos.y = pt.y;

        if (IsLeftButtonDown())
        {
            TCHITTESTINFO info{};
            info.pt = CPoint(lparam);
            int tab = HitTest(info);
            if (tab >= 0 && m_pressedTab >= 0)
            {
                if (tab != m_pressedTab)
                {
                    SwapTabs(tab, m_pressedTab);
                    m_pressedTab = tab;
                    SelectPage(tab);
                }
            }
        }

        return CTab::OnMouseMove(msg, wparam, lparam);
    }

    // Process notifications (WM_NOTIFY) reflected back from the parent.
    inline LRESULT CDockContainer::OnNotifyReflect(WPARAM, LPARAM lparam)
    {
        LPNMHDR pHeader = (LPNMHDR)lparam;
        switch (pHeader->code)
        {
        case TCN_SELCHANGE: return OnTCNSelChange(pHeader);
        }

        return 0;
    }

    // Called when the window gets keyboard focus. We set the keyboard focus
    // to the active view window.
    inline LRESULT CDockContainer::OnSetFocus(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        // Sets the focus to the active view (or its child).
        HWND prevFocus = reinterpret_cast<HWND>(wparam);
        if (GetActiveView() && GetActiveView()->IsWindow())
        {
            if (GetActiveView()->IsChild(prevFocus))
            {
                // Return focus back to the child of the active view that had it before.
                ::SetFocus(prevFocus);
            }
            else
                GetActiveView()->SetFocus();
        }

        return FinalWindowProc(msg, wparam, lparam);
    }

    // Called when the window is resized.
    inline LRESULT CDockContainer::OnSize(UINT, WPARAM, LPARAM)
    {
        RecalcLayout();
        return 0;
    }

    // Called when the currently selected tab has changed.
    inline LRESULT CDockContainer::OnTCNSelChange(LPNMHDR)
    {
        // Display the newly selected tab page.
        int page = GetCurSel();
        SelectPage(page);

        return 0;
    }

    // Called prior to window creation to set the CREATESTRUCT parameters.
    inline void CDockContainer::PreCreate(CREATESTRUCT& cs)
    {
        // For Tabs on the bottom, add the TCS_BOTTOM style.
        CTab::PreCreate(cs);
        cs.style |= TCS_BOTTOM;
    }

    // Repositions the child windows when the window is resized.
    inline void CDockContainer::RecalcLayout()
    {
        CDockContainer* parent = GetContainerParent();

        // Set the tab sizes.
        parent->SetTabSize();

        // Position the View over the tab control's display area.
        CRect rc = parent->GetClientRect();
        parent->AdjustRect(FALSE, &rc);

        if (parent->m_allInfo.size() > 0)
        {
            size_t pageIndex = static_cast<size_t>(parent->m_currentPage);
            CDockContainer* pContainer = parent->m_allInfo[pageIndex].pContainer;

            if (pContainer->GetViewPage().IsWindow())
            {
                VERIFY(pContainer->GetViewPage().SetWindowPos(HWND_TOP, rc, SWP_SHOWWINDOW));
                pContainer->GetViewPage().RecalcLayout();
            }
        }

        parent->RedrawWindow(RDW_INVALIDATE | RDW_NOCHILDREN);
    }

    // Removes the specified child container from this container group.
    inline void CDockContainer::RemoveContainer(CDockContainer* pWnd, BOOL updateParent)
    {
        assert(pWnd);
        if (!pWnd) return;

        // Must be called from the container parent.
        assert(this == GetContainerParent());

        // Remove the tab.
        int tab = GetContainerIndex(pWnd);
        DeleteItem(tab);

        // Remove the ContainerInfo entry.
        int image = -1;
        for (auto it = m_allInfo.begin(); it != m_allInfo.end(); ++it)
        {
            if (it->pContainer == pWnd)
            {
                image = (*it).tabImage;
                if (image >= 0)
                    RemoveImage(image);

                m_allInfo.erase(it);
                break;
            }
        }

        // Set the parent container relationships.
        pWnd->GetViewPage().SetParent(*pWnd);
        pWnd->m_pContainerParent = pWnd;

        // Display next lowest page.
        m_currentPage = std::max(tab - 1, 0);
        if (IsWindow() && updateParent)
        {
            if (GetItemCount() > 0)
                SelectPage(m_currentPage);
            else
                m_pDocker->SetCaption(_T(""));

            RecalcLayout();
        }
    }

    // Activates the specified page number.
    inline void CDockContainer::SelectPage(int page)
    {
        if (this != m_pContainerParent)
            m_pContainerParent->SelectPage(page);
        else
        {
            if ((page >= 0) && (page < static_cast<int>(m_allInfo.size())))
            {
                if (GetCurSel() != page)
                    SetCurSel(page);

                // Create the new container window if required.
                size_t pageIndex = static_cast<size_t>(page);
                if (!m_allInfo[pageIndex].pContainer->IsWindow())
                {
                    CDockContainer* pContainer = m_allInfo[pageIndex].pContainer;
                    pContainer->Create(GetParent());
                    pContainer->GetViewPage().SetParent(*this);
                }

                // Determine the size of the tab page's view area.
                CRect rc = GetClientRect();
                AdjustRect(FALSE, &rc);

                // Swap the pages over.
                CDockContainer* pNewContainer = m_allInfo[pageIndex].pContainer;
                for (const ContainerInfo& ci : m_allInfo)
                {
                    ci.pContainer->GetViewPage().ShowWindow(SW_HIDE);
                }

                VERIFY(pNewContainer->GetViewPage().SetWindowPos(HWND_TOP, rc, SWP_SHOWWINDOW));
                pNewContainer->GetViewPage().GetView()->SetFocus();

                // Adjust the docking caption.
                if (GetDocker())
                {
                    GetDocker()->SetCaption(pNewContainer->GetDockCaption());
                    GetDocker()->RedrawWindow();
                }

                m_currentPage = page;
            }
        }
    }

    // Sets the active container.
    inline void CDockContainer::SetActiveContainer(CDockContainer* pContainer)
    {
        int page = GetContainerIndex(pContainer);
        assert (0 <= page);
        SelectPage(page);
    }

    // Sets the caption of the docker whose view is this container.
    inline void CDockContainer::SetDockCaption(LPCTSTR caption)
    {
        m_caption = caption;
        if (IsWindow() && this == GetActiveContainer())
        {
            GetContainerParent()->GetDocker()->SetCaption(caption);
            GetContainerParent()->GetDocker()->RedrawWindow();
        }
    }

    // Shows or hides the tab if it has only one page.
    inline void CDockContainer::SetHideSingleTab(BOOL hideSingle)
    // Only display tabs if there are two or more.
    {
        m_isHideSingleTab = hideSingle;
        RecalcLayout();
    }

    // Sets the icon for this container's tab.
    inline void CDockContainer::SetTabIcon(HICON tabIcon)
    {
        m_tabIcon = tabIcon;
        int index = GetContainerIndex(this);
        if (index >= 0)
        {
            int image = GetContainerParent()->GetImages().Replace(index, tabIcon);
            GetContainerParent()->m_allInfo[index].tabImage = image;
            GetContainerParent()->SetTabSize();
            GetContainerParent()->UpdateTabs();
        }
    }

    // Sets the icon for this container's tab.
    inline void CDockContainer::SetTabIcon(UINT iconID)
    {
        HICON icon = reinterpret_cast<HICON>(GetApp()->LoadImage(iconID, IMAGE_ICON, 0, 0, LR_SHARED));
        SetTabIcon(icon);
    }

    // Updates the tabs font based on the window's DPI.
    inline void CDockContainer::SetTabsFontSize(int fontSize)
    {
        // Set the font used in the tabs.
        CFont font;
        NONCLIENTMETRICS info = GetNonClientMetrics();
        int dpi = GetWindowDpi(*this);
        LOGFONT lf = info.lfStatusFont;
        lf.lfHeight = -MulDiv(fontSize, dpi, POINTS_PER_INCH);
        font.CreateFontIndirect(lf);
        SetTabFont(font);
        RecalcLayout();
    }

    // Updates the tabs icon size based on the window's DPI.
    inline void CDockContainer::SetTabsIconSize(int iconSize)
    {
        int iconHeight = GetContainerParent()->DpiScaleInt(iconSize);
        iconHeight = iconHeight - iconHeight % 8;

        if (this == GetContainerParent())
        {
            std::vector<ContainerInfo>& v = GetAll();
            GetImages().Create(iconHeight, iconHeight, ILC_MASK | ILC_COLOR32, 0, 0);
            for (size_t i = 0; i < v.size(); ++i)
            {
                // Set the icons for the container parent.
                CDockContainer* pContainer = GetContainerFromIndex(i);
                v[i].tabImage = GetImages().Add(pContainer->GetTabIcon());
            }
        }
        else
        {
            // Set the icons for the container children, used if the container is undocked.
            GetImages().Create(iconHeight, iconHeight, ILC_MASK | ILC_COLOR32, 0, 0);
            GetImages().Add(GetTabIcon());
        }
        RecalcLayout();
    }

    // Sets the size of the tabs to accommodate the tab's icon and text.
    inline void CDockContainer::SetTabSize()
    {
        CRect rc = GetClientRect();
        AdjustRect(FALSE, &rc);
        if (rc.Width() < 0)
            rc.SetRectEmpty();

        int itemWidth = 0;
        int itemHeight = 1;

        CSize szImage = GetImages().GetIconSize();
        int padding = DpiScaleInt(4);
        szImage.cx = szImage.cx + 2 * padding;

        if ((m_allInfo.size() > 0) && ((GetItemCount() != 1) || !m_isHideSingleTab))
        {
            int imageX = szImage.cx;
            int imageY = szImage.cy;
            int tabTextWidth = GetMaxTabTextSize().cx;
            itemWidth = std::min(imageX + tabTextWidth + padding, (rc.Width() - 2) / static_cast<int>(m_allInfo.size()));
            itemHeight = std::max(imageY, GetTextHeight()) + padding;
        }

        SetItemSize(itemWidth, itemHeight);
    }

    // Sets the tab text for this container's tab.
    inline void CDockContainer::SetTabText(LPCTSTR text)
    {
        m_tabText = text;
        int index = GetContainerIndex(this);
        if (index >= 0)
        {
            GetContainerParent()->m_allInfo[index].tabText = text;
            GetContainerParent()->SetTabSize();
        }
    }

    // Sets the Image List for toolbars.
    // A Disabled image list is created from ToolBarID if one doesn't already exist.
    inline void CDockContainer::SetTBImageList(CToolBar& toolBar, UINT id, COLORREF mask)
    {
        // Get the image size.
        CBitmap bm(id);

        // Assert if we failed to load the bitmap.
        assert(bm.GetHandle() != nullptr);

        // Scale the bitmap to the window's DPI.
        CBitmap dpiImage = DpiScaleUpBitmap(bm);
        CSize sz = GetTBImageSize(&dpiImage);

        // Set the toolbar's image list.
        CImageList imageList;
        imageList.Create(sz.cx, sz.cy, ILC_COLOR32 | ILC_MASK, 0, 0);
        imageList.Add(dpiImage, mask);
        toolBar.SetImageList(imageList);
    }

    // Sets the disabled Image List for toolbars.
    inline void CDockContainer::SetTBImageListDis(CToolBar& toolBar, UINT id, COLORREF mask)
    {
        // Get the image size.
        CBitmap bm(id);

        // Assert if we failed to load the bitmap.
        assert(bm.GetHandle() != nullptr);

        // Scale the bitmap to the window's DPI.
        CBitmap dpiImage = DpiScaleUpBitmap(bm);
        CSize sz = GetTBImageSize(&dpiImage);

        // Set the toolbar's image list.
        CImageList imageList;
        imageList.Create(sz.cx, sz.cy, ILC_COLOR32 | ILC_MASK, 0, 0);
        imageList.Add(dpiImage, mask);
        toolBar.SetDisableImageList(imageList);
    }

    // Sets the Hot Image List for additional toolbars.
    inline void CDockContainer::SetTBImageListHot(CToolBar& toolBar, UINT id, COLORREF mask)
    {
        // Get the image size.
        CBitmap bm(id);

        // Assert if we failed to load the bitmap.
        assert(bm.GetHandle() != nullptr);

        // Scale the bitmap to the window's DPI.
        CBitmap dpiImage = DpiScaleUpBitmap(bm);
        CSize sz = GetTBImageSize(&dpiImage);

        // Set the toolbar's image list.
        CImageList imageList;
        imageList.Create(sz.cx, sz.cy, ILC_COLOR32 | ILC_MASK, 0, 0);
        imageList.Add(dpiImage, mask);
        toolBar.SetHotImageList(imageList);
    }

    // Either sets the imagelist or adds/replaces bitmap depending on ComCtl32.dll version
    // Assumes the width of the button image = height, minimum width = 16
    // The color mask is ignored for 32bit bitmaps, but is required for 24bit bitmaps
    // The color mask is often gray RGB(192,192,192) or magenta (255,0,255)
    // The hot and disabled bitmap resources can be 0
    // A disabled image list is created from the normal image list if one isn't provided.
    inline void CDockContainer::SetToolBarImages(COLORREF mask, UINT normalID, UINT hotID, UINT disabledID)
    {
        // Set the normal button images.
        SetTBImageList(GetToolBar(), normalID, mask);

        // Set the hot button images.
        if (hotID != 0)
            SetTBImageListHot(GetToolBar(), hotID, mask);
        else
            SetTBImageListHot(GetToolBar(), normalID, mask);

        // Set the disabled button images.
        if (disabledID != 0)
            SetTBImageListDis(GetToolBar(), disabledID, mask);
        else
        {
            CImageList toolBarDisabledImages;
            toolBarDisabledImages.CreateDisabledImageList(GetToolBar().GetImageList());
            GetToolBar().SetDisableImageList(toolBarDisabledImages);
        }
    }

    // Use this function to set the Resource IDs for the toolbar(s).
    inline void CDockContainer::SetupToolBar()
    {
/*      // Set the Resource IDs for the toolbar buttons
        AddToolBarButton( IDM_FILE_NEW   );
        AddToolBarButton( IDM_FILE_OPEN  );
        AddToolBarButton( IDM_FILE_SAVE  );
        AddToolBarButton( 0 );              // Separator
        AddToolBarButton( IDM_EDIT_CUT   );
        AddToolBarButton( IDM_EDIT_COPY  );
        AddToolBarButton( IDM_EDIT_PASTE );
        AddToolBarButton( 0 );              // Separator
        AddToolBarButton( IDM_FILE_PRINT );
        AddToolBarButton( 0 );              // Separator
        AddToolBarButton( IDM_HELP_ABOUT );
*/
    }

    // Sets or changes the container's view window.
    // The view window can be any resizeable child window.
    inline void CDockContainer::SetView(CWnd& wnd) const
    {
        GetViewPage().SetView(wnd);
    }

    // Swaps the positions of the specified tabs.
    inline void CDockContainer::SwapTabs(int tab1, int tab2)
    {
        size_t tab1Index = static_cast<size_t>(tab1);
        size_t tab2Index = static_cast<size_t>(tab2);
        assert (tab1Index < GetContainerParent()->m_allInfo.size());
        assert (tab2Index < GetContainerParent()->m_allInfo.size());

        if (tab1 != tab2)
        {
            ContainerInfo info1 = GetContainerParent()->m_allInfo[tab1Index];
            ContainerInfo info2 = GetContainerParent()->m_allInfo[tab2Index];

            TCITEM Item1{};
            Item1.mask = TCIF_IMAGE | TCIF_PARAM | TCIF_RTLREADING | TCIF_STATE | TCIF_TEXT;
            Item1.cchTextMax = info1.tabText.GetLength()+1;
            Item1.pszText = const_cast<LPTSTR>(info1.tabText.c_str());
            GetItem(tab1, &Item1);

            TCITEM Item2{};
             Item2.mask = TCIF_IMAGE | TCIF_PARAM | TCIF_RTLREADING | TCIF_STATE | TCIF_TEXT;
            Item2.cchTextMax = info2.tabText.GetLength()+1;
            Item2.pszText = const_cast<LPTSTR>(info2.tabText.c_str());
            GetItem(tab2, &Item2);

            SetItem(tab1, &Item2);
            SetItem(tab2, &Item1);
            GetContainerParent()->m_allInfo[tab1Index] = info2;
            GetContainerParent()->m_allInfo[tab2Index] = info1;
        }
    }

    // Updates the font and icons in the tabs.
    inline void CDockContainer::UpdateTabs()
    {
        SetTabsFontSize();
        SetTabsIconSize();
    }

    // Process the window's messages.
    inline LRESULT CDockContainer::WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        switch (msg)
        {
        case WM_LBUTTONDOWN:    return OnLButtonDown(msg, wparam, lparam);
        case WM_LBUTTONUP:      return OnLButtonUp(msg, wparam, lparam);
        case WM_MOUSELEAVE:     return OnMouseLeave(msg, wparam, lparam);
        case WM_MOUSEMOVE:      return OnMouseMove(msg, wparam, lparam);
        case WM_SETFOCUS:       return OnSetFocus(msg, wparam, lparam);
        case WM_SIZE:           return OnSize(msg, wparam, lparam);
        case WM_DPICHANGED_BEFOREPARENT: return OnDpiChangedBeforeParent(msg, wparam, lparam);
        case UWM_GETCDOCKCONTAINER: return reinterpret_cast<LRESULT>(this);
        }

        // pass unhandled messages on to CTab for processing
        return CTab::WndProcDefault(msg, wparam, lparam);
    }


    ///////////////////////////////////////////
    // Declaration of the nested CViewPage class
    // This is the class for the window displayed over the client area
    // of the tab control.  The toolbar and view window are child windows of the
    // viewpage window. Only the ViewPage of the parent CDockContainer is displayed.
    // Its contents are updated with the view window of the active container
    // whenever a different tab is selected.

    inline CDockContainer::CViewPage::CViewPage() : m_pContainer(nullptr), m_pView(nullptr), m_pTab(nullptr)
    {
        m_pToolBar = &m_toolBar;
    }


    // Returns a pointer to the parent dock container.
    inline CDockContainer* CDockContainer::CViewPage::GetContainer() const
    {
        assert(m_pContainer);
        return m_pContainer;
    }

    // Forwards command messages (WM_COMMAND) to the active container.
    inline BOOL CDockContainer::CViewPage::OnCommand(WPARAM wparam, LPARAM lparam)
    {
        BOOL isHandled = FALSE;
        if (GetContainer()->GetActiveContainer() && GetContainer()->GetActiveContainer()->IsWindow())
            isHandled = GetContainer()->GetActiveContainer()->SendMessage(WM_COMMAND, wparam, lparam) ? TRUE : FALSE;

        return isHandled;
    }

    // Called during window creation. Creates the child view window.
    inline int CDockContainer::CViewPage::OnCreate(CREATESTRUCT&)
    {
        if (m_pView)
            m_pView->Create(*this);

        return 0;
    }

    // Process WM_NOTIFY notifications from the child view window.
    inline LRESULT CDockContainer::CViewPage::OnNotify(WPARAM wparam, LPARAM lparam)
    {
        LPNMHDR pHeader = reinterpret_cast<LPNMHDR>(lparam);
        switch (pHeader->code)
        {

        // Display tooltips for the toolbar.
        case TTN_GETDISPINFO:
            {
                int index =  GetToolBar().HitTest();
                LPNMTTDISPINFO lpDispInfo = (LPNMTTDISPINFO)lparam;
                if (index >= 0)
                {
                    UINT id = GetToolBar().GetCommandID(index);
                    if (id > 0)
                    {
                        m_tooltip = LoadString(id);
                        lpDispInfo->lpszText = const_cast<LPTSTR>(m_tooltip.c_str());
                    }
                    else
                        m_tooltip = _T("");
                }
            }
            break;
        case NM_CUSTOMDRAW:
            {
                if (pHeader->hwndFrom == GetToolBar())
                {
                    // Pass Toolbar's custom draw up to CFrame.
                    return GetAncestor().SendMessage(WM_NOTIFY, wparam, lparam);
                }
            }
            break;
        } // switch LPNMHDR

        return 0;
    }

    // Called before window creation. Sets the WNDCLASS parameters.
    inline void CDockContainer::CViewPage::PreRegisterClass(WNDCLASS& wc)
    {
        wc.lpszClassName = _T("Win32++ TabPage");
        wc.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
    }

    // Called when the window is resized. Repositions the toolbar and view window.
    inline void CDockContainer::CViewPage::RecalcLayout() const
    {
        CRect rc = GetClientRect();
        if (GetToolBar().IsWindow() && GetToolBar().IsWindowVisible())
        {
            GetToolBar().Autosize();
            CRect rcToolBar = GetToolBar().GetClientRect();
            rc.top += rcToolBar.Height();
        }

        if (GetView())
            VERIFY(GetView()->SetWindowPos(HWND_TOP, rc, SWP_SHOWWINDOW));
    }

    // Sets or changes the View window displayed within the container.
    inline void CDockContainer::CViewPage::SetView(CWnd& wndView)
    {
        if (m_pView != &wndView)
        {
            // Hide the existing view window (if any).
            if (m_pView && m_pView->IsWindow())
                m_pView->ShowWindow(SW_HIDE);

            // Assign the view window.
            m_pView = &wndView;

            if (IsWindow())
            {
                if (!GetView()->IsWindow())
                    GetView()->Create(*this);
                else
                {
                    GetView()->SetParent(*this);
                    GetView()->ShowWindow();
                }

                // The new view must not be a dockcontainer.
                assert(GetView()->SendMessage(UWM_GETCDOCKCONTAINER) == 0);

                RecalcLayout();
            }
        }
    }

    // Process the window's messages.
    inline LRESULT CDockContainer::CViewPage::WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        switch (msg)
        {
        case WM_SIZE:
            RecalcLayout();
            break;
        }

        // pass unhandled messages on for default processing.
        return CWnd::WndProcDefault(msg, wparam, lparam);
    }

} // namespace Win32xx

#endif // _WIN32XX_DOCKING_H_
