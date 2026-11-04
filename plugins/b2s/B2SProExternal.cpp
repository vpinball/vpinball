// license:GPLv3+

#include <algorithm>

#include "B2SProExternal.h"
#include "B2SServer.h"

namespace B2S {

// A marker's script write: true on a new write (stamp changed) with a non zero value. The write seen at
// registration primes the stamp so that only later writes move the representative
static bool PolledWrite(B2SServer* server, const string& name, uint64_t& lastStamp, bool& primed)
{
   float value = 0.f;
   uint64_t stamp = 0;
   if (server == nullptr || !server->GetScriptedGroupState(name, value, &stamp))
      return false;
   if (!primed)
   {
      primed = true;
      lastStamp = stamp;
      return false;
   }
   if (stamp == lastStamp)
      return false;
   lastStamp = stamp;
   return value != 0.f;
}

void B2SProExternalPivotGroup::Register(B2SBulb* picture, const string& targetName, const bool representative, const float angle, const int durationMs)
{
   if (picture == nullptr || targetName.empty())
      return;
   m_targets[string_to_lower(targetName)].angle = angle;
   picture->m_brightness = 0.f; // Markers are hidden, the representative shows on its first move
   if (representative)
   {
      m_representative = picture;
      picture->m_proRt.externalPivot = true;
      picture->m_proRt.angle = angle;
   }
   m_durationMs = std::clamp(durationMs, 10, 1000);
}

void B2SProExternalPivotGroup::Update(B2SServer* server)
{
   if (m_representative == nullptr)
      return;
   for (auto& [name, target] : m_targets)
      if (PolledWrite(server, name, target.lastStamp, target.primed))
         m_representative->ProSetPivotRotationTarget(target.angle, m_durationMs);
}

void B2SProPositionGrid::Register(B2SBulb* picture, const string& targetName, const bool representative, const int durationMs)
{
   if (picture == nullptr || targetName.empty())
      return;
   m_targets[string_to_lower(targetName)].picture = picture;
   if (picture != m_representative)
      picture->m_brightness = 0.f;
   if (representative)
      m_representative = picture;
   m_durationMs = std::clamp(durationMs, 10, 1000);
}

void B2SProPositionGrid::MoveTo(const string& targetName)
{
   if (m_representative == nullptr || m_targets.find(targetName) == m_targets.end())
      return;
   // A short look ahead window stays open beyond the movement: natural gaps between source markers must
   // not be mistaken for the end of the motion, or the motion alternates between curves and catch up steps
   m_flushRemainingMs = static_cast<float>(std::clamp(m_durationMs * 2, 20, 2000));
   m_flushArmed = true;
   if (!m_pendingTargetName.empty() && m_targets.find(m_pendingTargetName) != m_targets.end())
      m_representative->ProStartExternalGridShift(m_targets[m_pendingTargetName].picture->ProMotionCenter(), m_durationMs, m_targets[targetName].picture->ProMotionCenter());
   m_pendingTargetName = targetName;
}

void B2SProPositionGrid::FlushPendingTarget()
{
   m_flushArmed = false;
   if (m_representative == nullptr || m_pendingTargetName.empty() || m_targets.find(m_pendingTargetName) == m_targets.end())
      return;
   const B2SProPoint target = m_targets[m_pendingTargetName].picture->ProMotionCenter();
   m_pendingTargetName.clear();
   m_representative->ProStartExternalGridShift(target, m_durationMs, target);
}

void B2SProPositionGrid::Update(B2SServer* server, const float elapsedInS)
{
   if (m_representative == nullptr)
      return;
   for (auto& [name, target] : m_targets)
      if (PolledWrite(server, name, target.lastStamp, target.primed))
         MoveTo(name);
   if (m_flushArmed)
   {
      m_flushRemainingMs -= elapsedInS * 1000.f;
      if (m_flushRemainingMs <= 0.f)
         FlushPendingTarget();
   }
}

}
