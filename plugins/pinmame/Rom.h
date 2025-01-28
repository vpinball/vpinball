#pragma once

#include "common.h"
#include "ScriptablePlugin.h"

class Rom
{
public:
   Rom() = default;
   ~Rom() = default;

   PSC_IMPLEMENT_REFCOUNT()

   // FIXME implement
   string GetName() const { PSC_NOT_IMPLEMENTED("GetName is not implemented"); return ""; }
   long GetState() const { PSC_NOT_IMPLEMENTED("GetState is not implemented"); return 0; }
   string GetStateDescription() const { PSC_NOT_IMPLEMENTED("GetStateDescription is not implemented"); return ""; }
   void Audit(bool fStrict = false) { PSC_NOT_IMPLEMENTED("Audit is not implemented"); }
   long GetLength() const { PSC_NOT_IMPLEMENTED("GetLength is not implemented"); return 0; }
   long GetExpLength() const { PSC_NOT_IMPLEMENTED("GetExpLength is not implemented"); return 0; }
   long GetChecksum() const { PSC_NOT_IMPLEMENTED("GetChecksum is not implemented"); return 0; }
   long GetExpChecksum() const { PSC_NOT_IMPLEMENTED("GetExpChecksum is not implemented"); return 0; }
   long GetFlags() const { PSC_NOT_IMPLEMENTED("GetFlags is not implemented"); return 0; }
};
