#pragma once

#include "PUPManager.h"

class PUPTriggerCondition
{
public:
   ~PUPTriggerCondition() { };

   static vector<PUPTriggerCondition*> CreateFromCSV(const string& line);
   bool Evaluate(PUPManager* pManager, const PUPTriggerData& data, bool& idMatch);
   string ToString() const;

private:
   PUPTriggerCondition(const string& szId, int value);

   string m_szId;
   int m_value;
};
