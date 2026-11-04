// license:GPLv3+

#pragma once

#include <deque>
#include <random>
#include <set>

#include "common.h"
#include "B2SDataModel.h"

namespace B2S {

// B2S Pro motion path sequence group (a trough): image snippits sharing a MotionPathSequenceGroup, ordered by
// MotionPathSequenceOrder, fill their slots one entry at a time from a feeder shown at the path start by the
// first member, and leave along their exit points one at a time (survivors shifting down a slot) or all
// together (drain all), or, for gravity troughs, are released into the physics where they sit and come back
// from wherever they settled on the next entry. Port of B2S Pro's MotionPathSequenceState, driven per frame:
// the members' travel completions are polled instead of raised as events.
class B2SProSequenceGroup final
{
public:
   explicit B2SProSequenceGroup(const string& name)
      : m_name(name)
   {
   }

   void Register(B2SBulb* member);
   const string& Name() const { return m_name; }
   bool Contains(const B2SBulb* bulb) const { return std::find(m_members.begin(), m_members.end(), bulb) != m_members.end(); }

   // Trigger requests (deduplicated per frame by the caller)
   void RequestStart();
   void RequestRemove();
   // Once per frame after the members advanced: consumes their completions and runs the pending operations
   void Update();

private:
   bool QueueEnabled() const;
   bool DrainAllEnabled() const;
   bool GravityDropEnabled() const;
   void QueueOperation(bool remove);
   bool StartEntry();
   bool StartGravityDrop();
   void AssignNextOpenEntrySlot(B2SBulb* capturedMember);
   static bool ActivateGravityDrop(B2SBulb* member);
   static void DeactivateGravityDrop(B2SBulb* member);
   bool StartRemoval();
   bool StartAllRemovals();
   void BeginEntry(B2SBulb* member, bool fromCurrentPosition);
   void BeginSourceRespawn();
   void CompleteSourceRespawn();
   void ProcessPendingOperations();
   void StartCompactionShifts(B2SBulb* removingMember);
   void NormalizeCompactedSlots(size_t remainingCount);
   void UpdateSourceAppearance();
   void EntryCompleted(B2SBulb* member);
   void ApplyEntryCollision(B2SBulb* incomingMember);
   void RemovalCompleted(B2SBulb* member);
   void DrainAllRemovalCompleted(B2SBulb* member);
   static VPXTexture ArtworkOf(const B2SBulb* bulb) { return bulb->m_proRt.displayImage != nullptr ? bulb->m_proRt.displayImage : bulb->m_image; }

   const string m_name;
   vector<B2SBulb*> m_members; // Ordered
   std::set<B2SBulb*> m_activeEntries;
   std::set<B2SBulb*> m_occupied;
   std::deque<bool> m_pendingOperations; // true = remove
   vector<B2SProPoint> m_compactionTargets;
   std::set<B2SBulb*> m_activeDrainAllRemovals;
   B2SBulb* m_activeRemoval = nullptr;
   B2SBulb* m_launchingMember = nullptr;
   B2SBulb* m_sourceAnchor = nullptr;
   bool m_sourceReady = true;
   std::set<B2SBulb*> m_availableGravityMembers; // Released into the physics, waiting to be called back
   bool m_gravityReuseEnabled = false;
   std::mt19937 m_random { std::random_device {}() };
};

}
