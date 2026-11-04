// license:GPLv3+

#pragma once

#include <map>

#include "common.h"
#include "B2SDataModel.h"

namespace B2S {

class B2SServer;

// B2S Pro external pivot groups and position grids: several marker snippits authored at different angles
// (ExternalPivotGroup) or positions (ExternalGridName) stand for the poses of one representative picture,
// which moves to a marker's pose when the table script writes that marker's name as an illumination group
// (B2SSetData "name=value" with a non zero value). A single image pivot (PivotAnimation without a trigger
// kind) is a pivot group of one picture with two named poses (PivotDownTrigger / PivotUpTrigger).
// Ports of B2S Pro's ExternalPivotState and ExternalPositionGridState.
class B2SProExternalPivotGroup final
{
public:
   void Register(B2SBulb* picture, const string& targetName, bool representative, float angle, int durationMs);
   // Polls the named poses' script writes and moves the representative on a new non zero write
   void Update(B2SServer* server);

private:
   struct Target
   {
      float angle = 0.f;
      uint64_t lastStamp = 0;
      bool primed = false;
   };
   B2SBulb* m_representative = nullptr;
   int m_durationMs = 80;
   std::map<string, Target> m_targets; // Lower case names
};

class B2SProPositionGrid final
{
public:
   void Register(B2SBulb* picture, const string& targetName, bool representative, int durationMs);
   void Update(B2SServer* server, float elapsedInS);

private:
   void MoveTo(const string& targetName);
   void FlushPendingTarget();
   struct Target
   {
      B2SBulb* picture = nullptr;
      uint64_t lastStamp = 0;
      bool primed = false;
   };
   B2SBulb* m_representative = nullptr;
   int m_durationMs = 45;
   std::map<string, Target> m_targets; // Lower case names
   string m_pendingTargetName;
   float m_flushRemainingMs = 0.f;
   bool m_flushArmed = false;
};

}
