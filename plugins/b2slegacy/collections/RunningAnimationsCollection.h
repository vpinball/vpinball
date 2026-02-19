#pragma once


namespace B2SLegacy {

class RunningAnimationsCollection final : public vector<string>
{
public:
   static RunningAnimationsCollection* GetInstance();

   void Add(const string& item);
   bool Remove(const string& item);
   bool Contains(const string& item) const;

private:
   static RunningAnimationsCollection* m_pInstance;
};

}
