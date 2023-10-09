#pragma once

class CollectData
{
public:
   CollectData(const int state, const int types);
   CollectData(const int state, const int types, bool earlyoffmode);

   int GetState() const { return m_state; }
   void SetState(const int state) { m_state = state; }
   int GetTypes() const { return m_types; }
   void SetTypes(const int types) { m_types = types; }
   bool IsEarlyOffMode() const { return m_earlyoffmode; }
   void SetEarlyOffMode(const bool earlyoffmode) { m_earlyoffmode = earlyoffmode; }

private:
   int m_state;
   int m_types;
   bool m_earlyoffmode;
};