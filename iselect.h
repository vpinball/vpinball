#pragma once

// NB: these are used for file I/O and must not be changed/reordered!
enum ItemTypeEnum
{
   eItemSurface,
   eItemFlipper,
   eItemTimer,
   eItemPlunger,
   eItemTextbox,
   eItemBumper,
   eItemTrigger,
   eItemLight,
   eItemKicker,
   eItemDecal,
   eItemGate,
   eItemSpinner,
   eItemRamp,
   eItemTable,
   eItemLightCenter,
   eItemDragPoint,
   eItemCollection,
   eItemDispReel,
   eItemLightSeq,
   eItemPrimitive,
   eItemFlasher,
   eItemRubber,
   eItemHitTarget,
   //eItemLightSeqCenter,
   eItemTypeCount,
   eItemInvalid = 0xffffffff // Force enum to be 32 bits
};

class Sur;

class PinTable;

class IEditable;

class TimerDataRoot;

struct PropertyPane;

enum SelectState
{
   eNotSelected,
   eSelected,
   eMultiSelected
};

INT_PTR CALLBACK RotateProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ScaleProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK TranslateProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

class BaseProperty
{
public:
    char  m_szImage[MAXTOKEN];
    char  m_szMaterial[MAXNAMEBUFFER];
    char  m_szPhysicsMaterial[MAXNAMEBUFFER];
    float m_elasticity;
    float m_friction;
    float m_scatter;
    float m_threshold;
    bool  m_collidable;
    bool  m_hitEvent;
    bool  m_overwritePhysics;
    bool  m_reflectionEnabled;
    bool  m_visible;
};

// ISelect is the subclass for anything that can be manipulated with the mouse.
// and that has a property sheet.

class ISelect : public ILoadable
{
public:
   ISelect();

   virtual void OnLButtonDown(int x, int y);
   virtual void OnLButtonUp(int x, int y);
   virtual void OnRButtonDown(int x, int y, HWND hwnd);
   virtual void OnRButtonUp(int x, int y);
   virtual void OnMouseMove(int x, int y);

   // Things to override
   virtual void MoveOffset(const float dx, const float dy);
   virtual void EditMenu(HMENU hmenu);
   virtual void DoCommand(int icmd, int x, int y);
   virtual void SetObjectPos();

   virtual void SetSelectFormat(Sur *psur);
   virtual void SetMultiSelectFormat(Sur *psur);
   virtual void SetLockedFormat(Sur *psur);

   virtual PinTable *GetPTable() = 0;

   virtual HRESULT GetTypeName(BSTR *pVal);
   static void GetTypeNameForType(ItemTypeEnum type, WCHAR * buf);

   virtual IDispatch *GetDispatch() = 0;
   virtual void GetDialogPanes(vector<PropertyPane*> &pvproppane) = 0;
   virtual ItemTypeEnum GetItemType() const = 0;

   virtual void Delete() = 0;
   virtual void Uncreate() = 0;

   virtual void FlipY(const Vertex2D& pvCenter);
   virtual void FlipX(const Vertex2D& pvCenter);
   virtual void Rotate(const float ang, const Vertex2D& pvCenter, const bool useElementCenter);
   virtual void Scale(const float scalex, const float scaley, const Vertex2D& pvCenter, const bool useElementCenter);
   virtual void Translate(const Vertex2D &pvOffset);

   // So objects don't have to implement all the transformation functions themselves
   virtual Vertex2D GetCenter() const = 0;
   virtual Vertex2D GetScale() const
   {
      return Vertex2D(1.f, 1.f);
   }
   virtual float GetRotate() const
   {
      return 0.0f;
   }

   virtual void PutCenter(const Vertex2D& pv) = 0;

   virtual void SetDefaultPhysics(bool fromMouseClick) {}

   virtual IEditable *GetIEditable() = 0;

   virtual bool LoadToken(const int id, BiffReader * const pbr);
   HRESULT SaveData(IStream *pstm, HCRYPTHASH hcrypthash);

   virtual int GetSelectLevel() { return 1; }
   virtual bool LoadMesh() { return false; }
   virtual void ExportMesh() {}
   virtual void UpdatePropertyPanes() {}
   virtual void AddPoint(int x, int y, const bool smooth) {}

   POINT m_ptLast;

   SelectState m_selectstate;

   int m_menuid; // context menu to use

   char m_layerIndex;

   bool m_dragging;
   bool m_markedForUndo;
   bool m_locked; // Can not be dragged in the editor
};
