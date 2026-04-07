// license:GPLv3+

#pragma once

class Sur;
class PinTable;
class IEditable;
class TimerDataRoot;
struct PropertyPane;
class WinEditor;

// Warning: these are Win32 ui part id, but also used to identify table parts (see IEditable)
// These are used for file I/O and must not be changed/reordered!
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
   eItemBall,
   eItemPartGroup,
   eItemTypeCount,
   eItemInvalid = 0xffffffff // Force enum to be 32 bits
};

// ISelect is the interface for parts contributing to the Win32 editor (mouse, property sheet, ...)
class ISelect
{
public:
   ISelect();

   // Report a change that would need the Win32 UI to be redrawn
   virtual void SetDirtyDraw();

   // Win32 editor rendering and picking
   virtual void SetSelectFormat(Sur *psur);
   virtual void SetMultiSelectFormat(Sur *psur);
   virtual void SetLockedFormat(Sur *psur);
   // This function draws the shape of the object with a solid fill, called before the grid lines are drawn on the map
   virtual void UIRenderPass1(Sur *const psur) = 0;
   // This function draws the shape of the object with a black outline (no solid fill), called after the grid lines have been drawn on the map.
   virtual void UIRenderPass2(Sur *const psur) = 0;
   virtual void RenderBlueprint(Sur *psur, const bool solid);

   virtual void OnLButtonDown(int x, int y);
   virtual void OnLButtonUp(int x, int y);

   virtual void MoveOffset(const float dx, const float dy) { } // Implement in child class to enable dragging
   virtual void EditMenu(CMenu &menu) { }
   virtual void DoCommand(int icmd, int x, int y);
   virtual void SetObjectPos();

   virtual PinTable *GetPTable() = 0;
   virtual const PinTable *GetPTable() const = 0;

   virtual HRESULT GetTypeName(BSTR *pVal) const;
   wstring GetTypeNameForType(const ItemTypeEnum type) const;

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

   virtual int GetSelectLevel() const { return 1; }
   virtual bool LoadMeshDialog() { return false; }
   virtual void ExportMeshDialog() {}
   virtual void AddPoint(int x, int y, const bool smooth) {}
   virtual void UpdateStatusBarInfo();

   virtual bool IsUILocked() const = 0;
   virtual void SetUILock(bool lock) = 0;
   virtual bool IsUIVisible() const = 0;
   virtual void SetUIVisible(bool visible) = 0;
   bool IsVisible(IEditable *editable) const; // UI visibility, applying PartGroup visibility (i.e. a part is visible if it is flagged as such, and its parents are also visibles)

   enum class SelectState
   {
      NotSelected,
      Selected,
      MultiSelected
   };
   SelectState m_selectstate = SelectState::NotSelected;

   int m_menuid = -1; // context menu to use

   bool m_dragging = false;

   bool m_markedForUndo = false; // Flag set when dragged to enable undo

protected:
   WinEditor *m_vpinball = nullptr;
};
