// license:GPLv3+

#include "common.h"

#include "B2SServer.h"

namespace B2S {
   
B2SServer::B2SServer()
{
   
}

B2SServer::~B2SServer()
{
   if (m_onDestroyHandler)
      m_onDestroyHandler(this);
}

void B2SServer::B2SSetData(int id, int value)
{
   m_states[id] = static_cast<float>(value);
}

void B2SServer::B2SSetData(const std::string& group, int value)
{

}

float B2SServer::GetState(int b2sId) const
{
   const auto it = m_states.find(b2sId);
   return it == m_states.end() ? 0.f : it->second;
}

// Scores

void B2SServer::B2SSetScorePlayer(int playerno, int score) { m_playerScores[playerno] = score; }

int B2SServer::GetPlayerScore(int player) const
{
   const auto it = m_playerScores.find(player);
   return it == m_playerScores.end() ? 0.f : it->second;
}

void B2SServer::B2SSetScoreDigit(int digit, int value) { m_scoreDigits[digit] = static_cast<float>(value); }

float B2SServer::GetScoreDigit(int b2sId) const
{
   const auto it = m_scoreDigits.find(b2sId);
   return it == m_scoreDigits.end() ? 0.f : it->second;
}

}