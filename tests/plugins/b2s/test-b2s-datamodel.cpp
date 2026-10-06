// license:GPLv3+

#include "core/stdafx.h"
#include "../../vpx-test.h"
#include "doctest.h"

// B2S plugin sources are compiled directly into the test binary (see VPX_TEST_SOURCES)
#include "b2s/B2SDataModel.h"

// Stubs for the plugin-side symbols normally provided by B2SPlugin.cpp
namespace B2S
{
void LPILog_CPP(const char*, int, const unsigned int, const std::string&) { }
void LPILog_CPP(const char*, int, const unsigned int, const std::stringstream&) { }
VPXTexture CreateTexture(uint8_t*, int) { return nullptr; }
void UpdateTexture(VPXTexture*, int, int, VPXTextureFormat, const void*) { }
VPXTextureInfo* GetTextureInfo(VPXTexture) { return nullptr; }
void DeleteTexture(VPXTexture) { }
}

using namespace B2S;

static std::shared_ptr<B2STable> LoadTable(const char* xml)
{
   tinyxml2::XMLDocument doc;
   REQUIRE(doc.Parse(xml) == tinyxml2::XML_SUCCESS);
   const tinyxml2::XMLElement* root = doc.FirstChildElement("DirectB2SData");
   REQUIRE(root != nullptr);
   return std::make_shared<B2STable>(*root);
}

TEST_CASE("B2S score display digit numbering")
{
   // Auto digits are numbered sequentially over all Score nodes in file order,
   // whatever their parent (backglass or DMD) and type (reel or LED)
   const auto table = LoadTable(R"(
      <DirectB2SData>
         <Scores ReelRollingInterval="50">
            <Score ID="1" Parent="Backglass" Digits="4" ReelType="reel_0" LocX="0" LocY="0" Width="100" Height="20"/>
            <Score ID="2" Parent="Backglass" Digits="2" B2SStartDigit="29" B2SScoreType="2" ReelType="dream7 7"/>
            <Score ID="3" Parent="DMD" Digits="3" ReelType="reel2_00"/>
         </Scores>
      </DirectB2SData>)");

   REQUIRE(table->m_backglassScores.m_scores.size() == 2);
   REQUIRE(table->m_dmdScores.m_scores.size() == 1);

   CHECK(table->m_backglassScores.m_scores[0].m_resolvedStartDigit == 1);
   CHECK(table->m_backglassScores.m_scores[1].m_resolvedStartDigit == 29);
   CHECK(table->m_dmdScores.m_scores[0].m_resolvedStartDigit == 7); // 5 after display 1 + 2 from display 2

   const B2SScore* display = table->FindScoreDisplay(1);
   REQUIRE(display != nullptr);
   CHECK(display->m_resolvedStartDigit == 1);
   CHECK(table->FindScoreDisplay(3)->m_resolvedStartDigit == 7);
   CHECK(table->FindScoreDisplay(42) == nullptr);
}

TEST_CASE("B2S score distribution over display digits")
{
   const auto table = LoadTable(R"(
      <DirectB2SData>
         <Scores>
            <Score ID="1" Parent="Backglass" Digits="4" ReelType="reel_0"/>
            <Score ID="2" Parent="Backglass" Digits="2" B2SStartDigit="29" ReelType="dream7 7"/>
         </Scores>
      </DirectB2SData>)");

   const B2SScore* reels = table->FindScoreDisplay(1);
   const B2SScore* dream7 = table->FindScoreDisplay(2);
   REQUIRE(reels != nullptr);
   REQUIRE(dream7 != nullptr);

   // Reels are zero padded on the left
   vector<int> digits = reels->DistributeScore(42);
   REQUIRE(digits.size() == 4);
   CHECK(digits == vector<int> { 0, 0, 4, 2 });

   // Rightmost digits are kept when the score does not fit
   digits = reels->DistributeScore(12345);
   CHECK(digits == vector<int> { 2, 3, 4, 5 });

   digits = reels->DistributeScore(0);
   CHECK(digits == vector<int> { 0, 0, 0, 0 });

   // LED displays are blank padded on the left (-1 means blank)
   digits = dream7->DistributeScore(7);
   REQUIRE(digits.size() == 2);
   CHECK(digits == vector<int> { -1, 7 });
}

TEST_CASE("B2S bulb and animation parsing")
{
   const auto table = LoadTable(R"(
      <DirectB2SData>
         <Illumination>
            <Bulb ID="1" Parent="Backglass" Name="groupA" B2SID="5" B2SValue="2" RomInverted="1" ZOrder="3"/>
            <Bulb ID="2" Parent="Backglass" Name="groupA" RomID="10" RomIDType="2" RomInverted="1"/>
         </Illumination>
         <Animations>
            <Animation Name="flash" Parent="Backglass" Interval="25" Loops="2" IDJoin="L10,IS3" RandomStart="0">
               <AnimationStep Step="1" On="groupA" WaitLoopsAfterOn="1" Off="groupB" PulseSwitch="44"/>
            </Animation>
         </Animations>
      </DirectB2SData>)");

   REQUIRE(table->m_backglassIlluminations.size() == 2);
   CHECK(table->m_backglassIlluminations[0]->m_b2sId == 5);
   CHECK(table->m_backglassIlluminations[0]->m_b2sValue == 2);
   CHECK(table->m_backglassIlluminations[0]->m_name == "groupA");
   CHECK(table->m_backglassIlluminations[0]->m_zOrder == 3);
   CHECK(table->m_backglassIlluminations[1]->m_romId == 10);
   CHECK(table->m_backglassIlluminations[1]->m_romIdType == B2SRomIDType::Solenoid);
   CHECK(table->m_backglassIlluminations[1]->m_romInverted);

   REQUIRE(table->m_backglassAnimations.size() == 1);
   const B2SAnimation& anim = table->m_backglassAnimations[0];
   CHECK(anim.m_name == "flash");
   CHECK(anim.m_interval == 25);
   CHECK(anim.m_loops == 2);
   CHECK(anim.m_idJoin == "L10,IS3");
   REQUIRE(anim.m_animationSteps.size() == 1);
   CHECK(anim.m_animationSteps[0].m_on == vector<string> { "groupA" });
   CHECK(anim.m_animationSteps[0].m_off == vector<string> { "groupB" });
   CHECK(anim.m_animationSteps[0].m_pulseSwitch == 44);
}
