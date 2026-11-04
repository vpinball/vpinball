// license:GPLv3+

#include <algorithm>
#include <cmath>

#include "B2SProSequence.h"

namespace B2S {

static double RouteLength(const vector<B2SProPoint>& route)
{
   double total = 0.0;
   for (size_t i = 0; i + 1 < route.size(); i++)
      total += std::sqrt(static_cast<double>(route[i + 1].x - route[i].x) * (route[i + 1].x - route[i].x) + static_cast<double>(route[i + 1].y - route[i].y) * (route[i + 1].y - route[i].y));
   return total;
}

static void AddDistinctPoint(vector<B2SProPoint>& route, const B2SProPoint& point)
{
   if (route.empty())
   {
      route.push_back(point);
      return;
   }
   const B2SProPoint& previous = route.back();
   const double dx = point.x - previous.x, dy = point.y - previous.y;
   if (dx * dx + dy * dy > 0.0001)
      route.push_back(point);
}

void B2SProSequenceGroup::Register(B2SBulb* member)
{
   if (member == nullptr || Contains(member))
      return;
   // A stable golden angle phase keeps striped trough balls from stopping in lockstep
   if (member->m_pro.motionPathRollEnabled)
      member->m_proRt.rollAngle = static_cast<float>(std::fmod((std::max(1, member->m_pro.motionPathSequenceOrder) - 1) * 137.507764, 360.0));
   m_members.push_back(member);
   std::sort(m_members.begin(), m_members.end(),
      [](const B2SBulb* a, const B2SBulb* b)
      {
         if (a->m_pro.motionPathSequenceOrder != b->m_pro.motionPathSequenceOrder)
            return a->m_pro.motionPathSequenceOrder < b->m_pro.motionPathSequenceOrder;
         return string_to_lower(a->m_name) < string_to_lower(b->m_name);
      });
   // The first ordered member supplies the artwork and rectangle of the feeder shown at the path start
   for (B2SBulb* m : m_members)
      m->ProSetSequenceSourceAnchor(false);
   m_sourceAnchor = m_members.empty() ? nullptr : m_members[0];
   if (m_sourceAnchor != nullptr)
   {
      m_sourceAnchor->ProSetSequenceSourceAnchor(true);
      UpdateSourceAppearance();
      m_sourceAnchor->m_proRt.seqSourceVisible = m_sourceReady;
   }
}

bool B2SProSequenceGroup::QueueEnabled() const
{
   return std::any_of(m_members.begin(), m_members.end(), [](const B2SBulb* m) { return m->m_pro.motionPathQueueTriggers; });
}

bool B2SProSequenceGroup::DrainAllEnabled() const
{
   return std::any_of(m_members.begin(), m_members.end(), [](const B2SBulb* m) { return m->m_pro.motionPathDrainAll; });
}

void B2SProSequenceGroup::QueueOperation(const bool remove)
{
   if (QueueEnabled() && m_pendingOperations.size() < 32)
      m_pendingOperations.push_back(remove);
}

void B2SProSequenceGroup::RequestStart()
{
   if (!m_pendingOperations.empty())
   {
      QueueOperation(false);
      ProcessPendingOperations();
      return;
   }
   if (m_activeRemoval != nullptr || !m_activeDrainAllRemovals.empty())
   {
      QueueOperation(false);
      return;
   }
   if (!m_sourceReady)
   {
      if (QueueEnabled())
         QueueOperation(false);
      else if (m_launchingMember != nullptr && m_activeEntries.count(m_launchingMember) != 0)
         BeginEntry(m_launchingMember, false);
      return;
   }
   StartEntry();
}

void B2SProSequenceGroup::RequestRemove()
{
   if (!m_pendingOperations.empty() || !m_activeEntries.empty())
   {
      QueueOperation(true);
      ProcessPendingOperations();
      return;
   }
   if (!m_activeDrainAllRemovals.empty())
   {
      if (QueueEnabled())
         QueueOperation(true);
      return;
   }
   if (m_activeRemoval != nullptr)
   {
      if (QueueEnabled())
         QueueOperation(true);
      else
         m_activeRemoval->ProStartMotionPath(true, false);
      return;
   }
   if (DrainAllEnabled())
      StartAllRemovals();
   else
      StartRemoval();
}

bool B2SProSequenceGroup::StartEntry()
{
   if (!m_sourceReady || m_activeRemoval != nullptr || !m_activeDrainAllRemovals.empty())
      return false;
   B2SBulb* memberToStart = nullptr;
   for (B2SBulb* member : m_members)
      if ((memberToStart == nullptr || member->m_pro.motionPathSequenceOrder < memberToStart->m_pro.motionPathSequenceOrder) && m_occupied.count(member) == 0 && m_activeEntries.count(member) == 0)
         memberToStart = member;
   if (memberToStart == nullptr)
      return false;
   m_activeEntries.insert(memberToStart);
   BeginEntry(memberToStart, false);
   return true;
}

bool B2SProSequenceGroup::StartRemoval()
{
   if (m_activeRemoval != nullptr || !m_activeDrainAllRemovals.empty() || !m_activeEntries.empty())
      return false;
   for (B2SBulb* member : m_members)
      if (m_occupied.count(member) != 0 && member->HasProExitPath())
      {
         m_activeRemoval = member;
         break;
      }
   if (m_activeRemoval == nullptr)
      return false;
   StartCompactionShifts(m_activeRemoval);
   m_activeRemoval->ProStartMotionPath(true, false);
   return true;
}

bool B2SProSequenceGroup::StartAllRemovals()
{
   if (m_activeRemoval != nullptr || !m_activeDrainAllRemovals.empty() || !m_activeEntries.empty())
      return false;
   vector<B2SBulb*> removals;
   for (B2SBulb* member : m_members)
      if (m_occupied.count(member) != 0 && member->HasProExitPath())
         removals.push_back(member);
   if (removals.empty())
      return false;
   std::sort(removals.begin(), removals.end(), [](const B2SBulb* a, const B2SBulb* b) { return a->m_pro.motionPathSequenceOrder < b->m_pro.motionPathSequenceOrder; });
   for (B2SBulb* member : removals)
      m_activeDrainAllRemovals.insert(member);
   B2SBulb* const lead = removals[0];
   const vector<B2SProPoint> leadRoute = lead->ProExitRuntimeRoute();
   const double leadLength = RouteLength(leadRoute);
   const double leadSpeed = leadLength > 0.001 ? leadLength / std::max(250, lead->m_pro.motionPathExitDuration) : 0.0;
   if (leadSpeed <= 0.000001)
   {
      for (B2SBulb* member : removals)
         member->ProStartMotionPath(true, false);
      return true;
   }
   // Each ball follows the balls ahead of it to the lead's exit route, at the lead's speed (a train)
   for (size_t index = 0; index < removals.size(); index++)
   {
      B2SBulb* const member = removals[index];
      vector<B2SProPoint> trainRoute { member->ProMotionCenter() };
      for (size_t preceding = index; preceding-- > 0;)
         AddDistinctPoint(trainRoute, removals[preceding]->ProMotionCenter());
      for (size_t i = 1; i < leadRoute.size(); i++)
         AddDistinctPoint(trainRoute, leadRoute[i]);
      const int memberDuration = static_cast<int>(std::lround(RouteLength(trainRoute) / leadSpeed));
      member->ProStartMotionPathDrainAll(trainRoute, memberDuration);
   }
   return true;
}

void B2SProSequenceGroup::BeginEntry(B2SBulb* member, const bool fromCurrentPosition)
{
   if (member == nullptr)
      return;
   m_launchingMember = member;
   m_sourceReady = false;
   if (m_sourceAnchor != nullptr)
   {
      m_sourceAnchor->m_proRt.seqSourceImage = ArtworkOf(member);
      m_sourceAnchor->m_proRt.seqSourceVisible = false;
   }
   member->ProStartMotionPath(false, fromCurrentPosition);
}

void B2SProSequenceGroup::BeginSourceRespawn()
{
   m_launchingMember = nullptr;
   if (m_sourceAnchor != nullptr)
   {
      UpdateSourceAppearance();
      if (m_sourceAnchor->ProStartSequenceSourceRespawn())
         return; // Completion is consumed in Update
   }
   CompleteSourceRespawn();
}

void B2SProSequenceGroup::CompleteSourceRespawn()
{
   m_sourceReady = true;
   if (m_sourceAnchor != nullptr && m_sourceAnchor->m_proRt.seqSourceAnchor)
      m_sourceAnchor->m_proRt.seqSourceVisible = true;
   ProcessPendingOperations();
}

void B2SProSequenceGroup::ProcessPendingOperations()
{
   while (!m_pendingOperations.empty())
   {
      const bool remove = m_pendingOperations.front();
      if (remove)
      {
         if (m_activeRemoval != nullptr || !m_activeDrainAllRemovals.empty() || !m_activeEntries.empty())
            return;
         m_pendingOperations.pop_front();
         if (DrainAllEnabled() ? StartAllRemovals() : StartRemoval())
            return;
      }
      else
      {
         if (m_activeRemoval != nullptr || !m_activeDrainAllRemovals.empty() || !m_sourceReady)
            return;
         if (StartEntry())
            m_pendingOperations.pop_front();
         return;
      }
   }
}

void B2SProSequenceGroup::StartCompactionShifts(B2SBulb* removingMember)
{
   m_compactionTargets.clear();
   vector<B2SBulb*> occupiedInOrder;
   for (B2SBulb* member : m_members)
      if (m_occupied.count(member) != 0)
         occupiedInOrder.push_back(member);
   if (occupiedInOrder.empty() || occupiedInOrder[0] != removingMember)
      return;
   for (size_t index = 0; index + 1 < occupiedInOrder.size(); index++)
   {
      m_compactionTargets.push_back(occupiedInOrder[index]->ProMotionCenter());
      occupiedInOrder[index + 1]->ProStartMotionPathShift(m_compactionTargets[index], removingMember->m_pro.motionPathExitDuration, static_cast<int>(index) * 48);
   }
}

void B2SProSequenceGroup::NormalizeCompactedSlots(const size_t remainingCount)
{
   // The survivors rolled into their new slots: swap their artwork and roll angles down to the lower numbered
   // logical members so the next entry uses the right open slot
   vector<VPXTexture> survivorImages, unusedImages;
   vector<float> survivorRolls, unusedRolls;
   for (B2SBulb* member : m_members)
   {
      if (m_occupied.count(member) != 0)
      {
         survivorImages.push_back(ArtworkOf(member));
         survivorRolls.push_back(member->m_proRt.rollAngle);
      }
      else
      {
         unusedImages.push_back(ArtworkOf(member));
         unusedRolls.push_back(member->m_proRt.rollAngle);
      }
   }
   m_occupied.clear();
   for (size_t index = 0; index < m_members.size(); index++)
   {
      B2SBulb* const member = m_members[index];
      if (index < remainingCount && index < survivorRolls.size())
      {
         member->m_proRt.displayImage = survivorImages[index];
         member->m_proRt.rollAngle = survivorRolls[index];
      }
      else if (index >= remainingCount && index - remainingCount < unusedRolls.size())
      {
         member->m_proRt.displayImage = unusedImages[index - remainingCount];
         member->m_proRt.rollAngle = unusedRolls[index - remainingCount];
      }
      member->ProSetMotionPathPosition(member->ProMotionCenter()); // Cancels every survivor shift
      if (index < remainingCount)
      {
         if (index < m_compactionTargets.size())
            member->ProSetMotionPathPosition(m_compactionTargets[index]);
         member->m_brightness = 1.f;
         m_occupied.insert(member);
      }
      else
         member->m_brightness = 0.f;
   }
   m_compactionTargets.clear();
   UpdateSourceAppearance();
}

void B2SProSequenceGroup::UpdateSourceAppearance()
{
   if (m_sourceAnchor == nullptr)
      return;
   for (B2SBulb* member : m_members)
      if (m_occupied.count(member) == 0 && m_activeEntries.count(member) == 0 && member != m_activeRemoval)
      {
         m_sourceAnchor->m_proRt.seqSourceImage = ArtworkOf(member);
         return;
      }
}

void B2SProSequenceGroup::EntryCompleted(B2SBulb* member)
{
   if (m_activeEntries.erase(member) != 0)
   {
      ApplyEntryCollision(member);
      m_occupied.insert(member);
      if (member == m_launchingMember)
         BeginSourceRespawn();
   }
   ProcessPendingOperations();
}

void B2SProSequenceGroup::ApplyEntryCollision(B2SBulb* incomingMember)
{
   if (incomingMember == nullptr || m_occupied.empty())
      return;
   vector<B2SBulb*> impacted;
   for (B2SBulb* member : m_members)
      if (m_occupied.count(member) != 0)
         impacted.push_back(member);
   std::sort(impacted.begin(), impacted.end(), [](const B2SBulb* a, const B2SBulb* b) { return b->m_pro.motionPathSequenceOrder < a->m_pro.motionPathSequenceOrder; });
   if (impacted.empty())
      return;
   const B2SProPoint incomingCenter = incomingMember->ProMotionCenter();
   const B2SProPoint nearestCenter = impacted[0]->ProMotionCenter();
   float dirX = nearestCenter.x - incomingCenter.x, dirY = nearestCenter.y - incomingCenter.y;
   const double length = std::sqrt(static_cast<double>(dirX) * dirX + static_cast<double>(dirY) * dirY);
   if (length <= 0.001)
      return;
   dirX = static_cast<float>(dirX / length);
   dirY = static_cast<float>(dirY / length);
   const float ballDiameter = std::max(1.f, static_cast<float>(std::min(incomingMember->m_width, incomingMember->m_height)));
   float impactAmplitude = incomingMember->m_proRt.motionArrivalSpeed * 4.5f;
   impactAmplitude = std::max(ballDiameter * 0.012f, std::min(ballDiameter * 0.04f, impactAmplitude)) * 30.f;
   incomingMember->ProStartTroughImpulse(-dirX, -dirY, impactAmplitude * 0.35f, 0.f);
   float transferred = impactAmplitude;
   for (size_t index = 0; index < impacted.size(); index++)
   {
      impacted[index]->ProStartTroughImpulse(dirX, dirY, transferred, static_cast<float>(index) * 18.f);
      transferred *= 0.58f;
      if (transferred < 0.25f)
         break;
   }
}

void B2SProSequenceGroup::RemovalCompleted(B2SBulb* member)
{
   if (member == m_activeRemoval)
   {
      m_occupied.erase(member);
      member->m_brightness = 0.f;
      NormalizeCompactedSlots(m_occupied.size());
      m_activeRemoval = nullptr;
   }
   ProcessPendingOperations();
}

void B2SProSequenceGroup::DrainAllRemovalCompleted(B2SBulb* member)
{
   if (m_activeDrainAllRemovals.erase(member) == 0)
      return;
   m_occupied.erase(member);
   member->m_brightness = 0.f;
   if (m_activeDrainAllRemovals.empty())
   {
      m_compactionTargets.clear();
      UpdateSourceAppearance();
      ProcessPendingOperations();
   }
}

void B2SProSequenceGroup::Update()
{
   // Launch segment completions (the feeder respawns once the launched ball left the start)
   for (B2SBulb* member : vector<B2SBulb*>(m_activeEntries.begin(), m_activeEntries.end()))
   {
      if (member == m_launchingMember && member->m_proRt.motionLaunchReported && member->m_proRt.motionRunning)
      {
         // Reported once: the flag stays set while travelling, the launching member is cleared by the respawn
         BeginSourceRespawn();
      }
   }
   // Travel completions
   for (B2SBulb* member : vector<B2SBulb*>(m_activeEntries.begin(), m_activeEntries.end()))
      if (member->m_proRt.motionCompleted)
      {
         member->m_proRt.motionCompleted = false;
         EntryCompleted(member);
      }
   if (m_activeRemoval != nullptr && m_activeRemoval->m_proRt.motionCompleted)
   {
      m_activeRemoval->m_proRt.motionCompleted = false;
      RemovalCompleted(m_activeRemoval);
   }
   for (B2SBulb* member : vector<B2SBulb*>(m_activeDrainAllRemovals.begin(), m_activeDrainAllRemovals.end()))
      if (member->m_proRt.motionCompleted)
      {
         member->m_proRt.motionCompleted = false;
         DrainAllRemovalCompleted(member);
      }
   if (m_sourceAnchor != nullptr && m_sourceAnchor->m_proRt.seqRespawnJustCompleted)
   {
      m_sourceAnchor->m_proRt.seqRespawnJustCompleted = false;
      CompleteSourceRespawn();
   }
}

}
