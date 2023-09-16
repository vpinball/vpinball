#pragma once

#include "Group.h"
#include "Label.h"

class ScoreBoard : public Group
{
public:
   ScoreBoard(FlexDMD* pFlexDMD, Font* pScoreFont, Font* pHighlightFont, Font* pTextFont);
   ~ScoreBoard();

   void SetBackground(Actor* background);
   void SetNPlayers(int nPlayers);
   void SetFonts(Font* scoreFont, Font* highlightFont, Font* textFont);
   void SetHighlightedPlayer(int player);
   void SetScore(LONG score1, LONG score2, LONG score3, LONG score4);

   virtual void Update(float delta);
   virtual void Draw(Graphics* graphics);

   Font* m_pScoreFont;
   Font* m_pHighlightFont;
   Font* m_pTextFont;
   Label* m_pLowerLeft;
   Label* m_pLowerRight;
   Label* m_pScores[4];
   Actor* m_pBackground;
   int m_highlightedPlayer;
};