#include "WMPSettings.h"
#include "WMPCore.h"
#include <algorithm>

namespace WMP {

WMPSettings::WMPSettings(WMPCore* pCore) : m_pCore(pCore)
{
   m_pCore->AddRef();
}

WMPSettings::~WMPSettings()
{
   assert(m_refCount == 0);

   m_pCore->Release();
}

bool WMPSettings::GetAutoStart() const
{
   return m_pCore->m_autoStart;
}

void WMPSettings::SetAutoStart(bool autoStart)
{
   m_pCore->m_autoStart = autoStart;
}

bool WMPSettings::GetMute() const
{
   return m_pCore->m_mute;
}

void WMPSettings::SetMute(bool mute)
{
   m_pCore->m_mute = mute;
   m_pCore->GetAudioPlayer()->UpdateVolume(m_pCore->m_volume, m_pCore->m_mute);
}

long WMPSettings::GetVolume() const
{
   return m_pCore->m_volume;
}

void WMPSettings::SetVolume(long volume)
{
   m_pCore->m_volume = volume;
   m_pCore->GetAudioPlayer()->UpdateVolume(m_pCore->m_volume, m_pCore->m_mute);
}

}
