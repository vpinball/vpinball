// license:GPLv3+

#pragma once

class PinTable;
class IEditable;
class WinEditor;

// Warning: these are Win32 ui part id, but also used to identify table parts (see IEditable)
// These are used for file I/O and must not be changed/reordered!
enum ItemTypeEnum : uint32_t
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
   eItemInvalid = 0xFFFFFFFFu
};

// ISelect is the interface for parts contributing to the Win32 editor (mouse, property sheet, ...)
class ISelect
{
public:
   ISelect();

   // Report a change that would need the Win32 UI to be redrawn
   virtual void SetDirtyDraw();

   virtual void MoveOffset(const float dx, const float dy) { } // Implement in child class to enable dragging

   virtual PinTable *GetPTable() = 0;
   virtual const PinTable *GetPTable() const = 0;

   virtual HRESULT GetTypeName(BSTR *pVal) const;
   wstring GetTypeNameForType(const ItemTypeEnum type) const;

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

   virtual IEditable *GetIEditable() = 0;
   virtual const IEditable *GetIEditable() const = 0;

   virtual int GetSelectLevel() const { return 1; }
   virtual void UpdateStatusBarInfo();

protected:
   WinEditor *m_vpinball = nullptr;
};
