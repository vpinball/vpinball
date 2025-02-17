#include "ScoreBoard.h"

ScoreBoard::ScoreBoard(FlexDMD* pFlexDMD, Font* pScoreFont, Font* pHighlightFont, Font* pTextFont) : Group(pFlexDMD, string())
{
   m_pScoreFont = pScoreFont;
   m_pScoreFont->AddRef();

   m_pHighlightFont = pHighlightFont;
   m_pHighlightFont->AddRef();

   m_pTextFont = pTextFont;
   m_pTextFont->AddRef();
   
   m_pLowerLeft = new Label(pFlexDMD, pTextFont, string(), string());
   AddActor(m_pLowerLeft);

   m_pLowerRight = new Label(pFlexDMD, pTextFont, string(), string());
   AddActor(m_pLowerRight);

   for (int i = 0; i < 4; i++) {
      m_pScores[i] = new Label(pFlexDMD, pScoreFont, "0"s, string());
      AddActor(m_pScores[i]);
   }
   m_pBackground = nullptr;
   m_highlightedPlayer = 0;
}

ScoreBoard::~ScoreBoard()
{
   m_pLowerLeft->Remove();
   m_pLowerRight->Remove();

   for (int i = 0; i < 4; i++)
      m_pScores[i]->Remove();

   m_pScoreFont->Release();
   m_pHighlightFont->Release();
   m_pTextFont->Release();
}

void ScoreBoard::SetBackground(Actor* pBackground)
{
   if (m_pBackground) 
      RemoveActor(m_pBackground);
   m_pBackground = pBackground;
   if (m_pBackground)
      AddActorAt(m_pBackground, 0);
}

void ScoreBoard::SetNPlayers(int nPlayers)
{
   for (int i = 0; i < 4; i++)
      m_pScores[i]->SetVisible(i < nPlayers);
}

void ScoreBoard::SetFonts(Font* scoreFont, Font* highlightFont, Font* textFont)
{
   m_pScoreFont->Release();
   m_pScoreFont = scoreFont;
   m_pScoreFont->AddRef();

   m_pHighlightFont->Release();
   m_pHighlightFont = highlightFont;
   m_pHighlightFont->AddRef();

   m_pTextFont->Release();
   m_pTextFont = textFont;
   m_pTextFont->AddRef();
   
   SetHighlightedPlayer(m_highlightedPlayer);
   m_pLowerLeft->SetFont(textFont);
   m_pLowerRight->SetFont(textFont);
}

void ScoreBoard::SetHighlightedPlayer(int player)
{
   m_highlightedPlayer = player;
   for (int i = 0; i < 4; i++) {
      if (i == player - 1)
         m_pScores[i]->SetFont(m_pHighlightFont);
      else
         m_pScores[i]->SetFont(m_pScoreFont);
    }
}

void ScoreBoard::SetScore(int score1, int score2, int score3, int score4)
{
   m_pScores[0]->SetText(std::to_string(score1));
   m_pScores[1]->SetText(std::to_string(score2));
   m_pScores[2]->SetText(std::to_string(score3));
   m_pScores[3]->SetText(std::to_string(score4));
}

void ScoreBoard::Update(float delta)
{
    Group::Update(delta);
    SetBounds(0, 0, GetParent()->GetWidth(), GetParent()->GetHeight());
    float yText = GetHeight() - m_pTextFont->GetBitmapFont()->GetBaseHeight() - 1;
    // float yLine2 = 1 + m_pHighlightFont->m_pBitmapFont->m_baseHeight + (GetHeight() - 2 -  m_pTextFont->m_pBitmapFont->m_baseHeight - 2 * m_pHighlightFont->m_pBitmapFont->m_baseHeight ) / 2;
    float yLine2 = (GetHeight() - m_pTextFont->GetBitmapFont()->GetBaseHeight()) / 2.0f;
    float dec = (float)(m_pHighlightFont->GetBitmapFont()->GetBaseHeight() - m_pScoreFont->GetBitmapFont()->GetBaseHeight()) / 2.0f;
    // float yLine2 = (1 + yText) * 0.5f;
    m_pScores[0]->Pack();
    m_pScores[1]->Pack();
    m_pScores[2]->Pack();
    m_pScores[3]->Pack();
    m_pLowerLeft->Pack();
    m_pLowerRight->Pack();
    m_pScores[0]->SetAlignedPosition(1, 1 + (m_highlightedPlayer == 1 ? 0 : dec), Alignment_TopLeft);
    m_pScores[1]->SetAlignedPosition(GetWidth() - 1, 1 + (m_highlightedPlayer == 2 ? 0 : dec), Alignment_TopRight);
    m_pScores[2]->SetAlignedPosition(1, yLine2 + (m_highlightedPlayer == 3 ? 0 : dec), Alignment_TopLeft);
    m_pScores[3]->SetAlignedPosition(GetWidth() - 1, yLine2 + (m_highlightedPlayer == 4 ? 0 : dec), Alignment_TopRight);
    m_pLowerLeft->SetAlignedPosition(1, yText, Alignment_TopLeft);
    m_pLowerRight->SetAlignedPosition(GetWidth() - 1, yText, Alignment_TopRight);
}

void ScoreBoard::Draw(VP::SurfaceGraphics* pGraphics)
{
   if (GetVisible()) {
      pGraphics->SetColor(RGB(0, 0, 0));
      pGraphics->Clear();
      if (m_pBackground)
         m_pBackground->SetSize(GetWidth(), GetHeight());
      Group::Draw(pGraphics);
   }
}
