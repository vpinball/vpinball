#pragma once

class Kerning {
public:
   Kerning();
   ~Kerning();

   int GetHash();

   char GetFirstCharacter() { return m_firstCharacter; }
   void SetFirstCharacter(char firstCharacter) { m_firstCharacter = firstCharacter; }
   char GetSecondCharacter() { return m_secondCharacter; }
   void SetSecondCharacter(char secondCharacter) { m_secondCharacter = secondCharacter; }
   int GetAmount() { return m_amount; }
   void SetAmount(int amount) { m_amount = amount; }

private:
   char m_firstCharacter;
   char m_secondCharacter;
   int m_amount;
};