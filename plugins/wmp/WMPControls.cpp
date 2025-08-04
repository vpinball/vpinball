#include "WMPControls.h"
#include "WMPCore.h"

namespace WMP {

WMPControls::WMPControls(WMPCore* pCore) : m_pCore(pCore)
{
   m_pCore->AddRef();
}

WMPControls::~WMPControls()
{
   assert(m_refCount == 0);

   m_pCore->Release();
}

void WMPControls::Play()
{
   m_pCore->GetAudioPlayer()->Play();
}

void WMPControls::Stop()
{
   m_pCore->GetAudioPlayer()->Stop();
}

void WMPControls::Pause()
{
   m_pCore->GetAudioPlayer()->Pause();
}

double WMPControls::GetCurrentPosition() const
{
   return m_pCore->GetAudioPlayer()->GetPosition();
}

void WMPControls::SetCurrentPosition(double position)
{
   m_pCore->GetAudioPlayer()->SetPosition(position);
}

}
