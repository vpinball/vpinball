// license:GPLv3+

#pragma once

#include "core/def.h"


// Reports messages through a Win32 message box, optionally parented to the given window
class Win32DialogSink final : public UserMessageSink
{
public:
   explicit Win32DialogSink(HWND parent = nullptr)
      : m_parent(parent)
   {
   }
   void Notify(MsgSeverity severity, const string& title, const string& message) override;
   bool Confirm(const string& title, const string& message, bool fallback) override;

private:
   const HWND m_parent;
};
