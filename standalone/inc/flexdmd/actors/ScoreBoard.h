#pragma once

#include "Group.h"
#include "Label.h"

class ScoreBoard : public Group
{
public:
   ScoreBoard(FlexDMD* pFlexDMD, Font* pScoreFont, Font* pHighlightFont, Font* pTextFont);
   ~ScoreBoard();

   Label* GetLowerLeft() { return m_pLowerLeft; }
   Label* GetLowerRight() { return m_pLowerRight; }
   void SetBackground(Actor* background);
   void SetNPlayers(int nPlayers);
   void SetFonts(Font* scoreFont, Font* highlightFont, Font* textFont);
   void SetHighlightedPlayer(int player);
   void SetScore(LONG score1, LONG score2, LONG score3, LONG score4);
   void Update(float delta) override;
   void Draw(VP::SurfaceGraphics* pGraphics) override;

private:
   Font* m_pScoreFont;
   Font* m_pHighlightFont;
   Font* m_pTextFont;
   Label* m_pLowerLeft;
   Label* m_pLowerRight;
   Label* m_pScores[4];
   Actor* m_pBackground;
   int m_highlightedPlayer;
};