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
   virtual void GetDialogPanes(Vector<PropertyPane> *pvproppane) = 0;
   virtual ItemTypeEnum GetItemType() = 0;

   virtual void Delete() = 0;
   virtual void Uncreate() = 0;

   virtual void FlipY(Vertex2D * const pvCenter);
   virtual void FlipX(Vertex2D * const pvCenter);
   virtual void Rotate(float ang, Vertex2D *pvCenter, const bool useElementCenter=false);
   virtual void Scale(float scalex, float scaley, Vertex2D *pvCenter, const bool useElementsCenter=false);
   virtual void Translate(Vertex2D *pvOffset);

   // So objects don't have to implement all the transformation functions themselves
   virtual void GetCenter(Vertex2D * const pv) const = 0;
   virtual void GetScale(float *sx, float *sy)
   {
      *sx = 1.0f;
      *sy = 1.0f;
   }
   virtual float GetRotate()
   {
      return 0.0f;
   }

   virtual void PutCenter(const Vertex2D * const pv) = 0;

   virtual void SetDefaultPhysics(bool fromMouseClick){}

   virtual IEditable *GetIEditable() = 0;

   BOOL LoadToken(int id, BiffReader *pbr);
   HRESULT SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey);

   virtual int GetSelectLevel() { return 1; }
   virtual bool LoadMesh(){ return false; }
   virtual void ExportMesh(){};
   virtual void UpdatePropertyPanes(){};
   virtual void AddPoint(int x, int y, const bool smooth=false){};

   POINT m_ptLast;

   SelectState m_selectstate;

   int m_menuid;  // context menu to use

   int layerIndex;

   bool m_fDragging;
   bool m_fMarkedForUndo;
   bool m_fLocked; // Can not be dragged in the editor
};
