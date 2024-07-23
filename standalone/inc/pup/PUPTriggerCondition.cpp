#include "stdafx.h"

#include "PUPTriggerCondition.h"

#include "PUPManager.h"

PUPTriggerCondition::PUPTriggerCondition(const string& szId, int value)
{
   m_szId = szId;
   m_value = value;
}

vector<PUPTriggerCondition*> PUPTriggerCondition::CreateFromCSV(const string& line)
{
   vector<PUPTriggerCondition*> conditions;

   std::stringstream ss(line);
   std::string token;
   while (std::getline(ss, token, ',')) {
       auto pos = token.find('=');
       string id = trim_string(token.substr(0, pos));
       int value = (pos == string::npos)
           ? 1
           : string_to_int(token.substr(pos + 1));
       conditions.push_back(new PUPTriggerCondition(id, value));
   }
   return conditions;
}

bool PUPTriggerCondition::Evaluate(PUPManager* pManager, const PUPTriggerData& data, bool& idMatch)
{
   idMatch = m_szId == data.type + std::to_string(data.number);

   return pManager->GetTriggerValue(m_szId) == m_value;
}

string PUPTriggerCondition::ToString() const
{
   return m_szId + "=" + std::to_string(m_value);
}