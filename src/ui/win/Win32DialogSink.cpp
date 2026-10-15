// license:GPLv3+

#include "core/stdafx.h"
#include "ui/win/Win32DialogSink.h"


void Win32DialogSink::Notify(const MsgSeverity severity, const string& title, const string& message)
{
   const UINT icon = severity == MsgSeverity::Info ? MB_ICONINFORMATION : severity == MsgSeverity::Warning ? MB_ICONWARNING : MB_ICONERROR;
   ::MessageBoxW(m_parent, MakeWString(message).c_str(), MakeWString(title).c_str(), MB_OK | icon); // Messages are UTF-8
}

bool Win32DialogSink::Confirm(const string& title, const string& message, const bool fallback)
{
   return ::MessageBoxW(m_parent, MakeWString(message).c_str(), MakeWString(title).c_str(), MB_YESNO | MB_ICONQUESTION | (fallback ? MB_DEFBUTTON1 : MB_DEFBUTTON2)) == IDYES;
}
