// license:GPLv3+

#pragma once

// these are used for file I/O and must not be changed/reordered!
enum ItemTypeEnum : unsigned int
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
   eItemBall,
   eItemPartGroup,
   eItemTypeCount,
   eItemInvalid = 0xffffffff // Force enum to be 32 bits
};


class Sur;
class PinTable;
class IEditable;
class TimerDataRoot;
struct PropertyPane;
class VPinball;


enum SelectState
{
   eNotSelected,
   eSelected,
   eMultiSelected
};

INT_PTR CALLBACK RotateProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ScaleProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK TranslateProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

class BaseProperty // not everything in here is used in all of the derived classes, but it simplifies the UI code!
{
public:
    string m_szImage;
    string m_szMaterial;
    string m_szPhysicsMaterial;
    float m_elasticity;
    float m_friction;
    float m_scatter;
    float m_threshold;
    bool  m_collidable;
    bool  m_hitEvent = false;
    bool  m_overwritePhysics = true;
    bool  m_reflectionEnabled = true;
    bool  m_visible = true;
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
   virtual void EditMenu(CMenu &menu);
   virtual void DoCommand(int icmd, int x, int y);
   virtual void SetObjectPos();

   virtual void SetSelectFormat(Sur *psur);
   virtual void SetMultiSelectFormat(Sur *psur);
   virtual void SetLockedFormat(Sur *psur);

   virtual PinTable *GetPTable() = 0;
   virtual const PinTable *GetPTable() const = 0;

   virtual HRESULT GetTypeName(BSTR *pVal) const;
   void GetTypeNameForType(const ItemTypeEnum type, WCHAR * const buf) const;

   virtual IDispatch *GetDispatch() = 0;
   virtual const IDispatch *GetDispatch() const = 0;
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
      return {1.f, 1.f};
   }
   virtual float GetRotate() const
   {
      return 0.0f;
   }

   virtual void PutCenter(const Vertex2D& pv) = 0;

   virtual void SetDefaultPhysics(const bool fromMouseClick) {}

   virtual IEditable *GetIEditable() = 0;
   virtual const IEditable *GetIEditable() const = 0;

   bool LoadToken(const int id, BiffReader * const pbr) override;
   HRESULT SaveData(IStream *pstm, HCRYPTHASH hcrypthash);

   virtual int GetSelectLevel() const { return 1; }
   virtual bool LoadMeshDialog() { return false; }
   virtual void ExportMeshDialog() {}
   virtual void AddPoint(int x, int y, const bool smooth) {}
   virtual void UpdateStatusBarInfo();

   SelectState m_selectstate = eNotSelected;
   int m_menuid = -1; // context menu to use
   bool m_dragging = false;
   bool m_locked = false; // Can not be dragged in the editor
   bool m_isVisible = true; // UI visibility (not the same as rendering visibility which is a member of part data)

protected:
   VPinball *m_vpinball = nullptr;

private:
   bool m_markedForUndo = false; // Flag set when dragged to enable undo
   POINT m_ptLast {}; // Last point when dragging
};
