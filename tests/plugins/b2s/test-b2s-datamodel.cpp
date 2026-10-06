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

   // Bulbs are ZOrder sorted: the bulb without ZOrder comes first
   REQUIRE(table->m_backglassIlluminations.size() == 2);
   CHECK(table->m_backglassIlluminations[0]->m_romId == 10);
   CHECK(table->m_backglassIlluminations[0]->m_romIdType == B2SRomIDType::Solenoid);
   CHECK(table->m_backglassIlluminations[0]->m_romInverted);
   CHECK(table->m_backglassIlluminations[1]->m_b2sId == 5);
   CHECK(table->m_backglassIlluminations[1]->m_b2sValue == 2);
   CHECK(table->m_backglassIlluminations[1]->m_name == "groupA");
   CHECK(table->m_backglassIlluminations[1]->m_zOrder == 3);

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

   // IDJoin is parsed into ROM event triggers: lamp 10 and inverted solenoid 3
   REQUIRE(anim.GetRomTriggers().size() == 2);
   CHECK(anim.GetRomTriggers()[0].romIdType == B2SRomIDType::Lamp);
   CHECK(anim.GetRomTriggers()[0].romId == 10);
   CHECK(!anim.GetRomTriggers()[0].inverted);
   CHECK(anim.GetRomTriggers()[1].romIdType == B2SRomIDType::Solenoid);
   CHECK(anim.GetRomTriggers()[1].romId == 3);
   CHECK(anim.GetRomTriggers()[1].inverted);
}

namespace
{
struct AnimFx
{
   std::unordered_map<string, float> groups;
   std::unordered_map<string, int> locks;
   vector<int> pulses;
   bool scoreDisplaysHidden = false;
   int allOffCount = 0;

   B2SAnimationEffects Make()
   {
      B2SAnimationEffects fx;
      fx.setGroup = [this](const string& group, bool on) { groups[group] = on ? 1.f : 0.f; };
      fx.getGroup = [this](const string& group) -> float
      {
         const auto it = groups.find(group);
         return it == groups.end() ? 0.f : it->second;
      };
      fx.lockGroup = [this](const string& group) { locks[group]++; };
      fx.unlockGroup = [this](const string& group)
      {
         if (const auto it = locks.find(group); it != locks.end())
         {
            if (it->second > 1)
               it->second--;
            else
               locks.erase(it);
         }
      };
      fx.pulseSwitch = [this](int switchId) { pulses.push_back(switchId); };
      fx.setScoreDisplaysHidden = [this](bool hidden) { scoreDisplaysHidden = hidden; };
      fx.allLightsOff = [this]()
      {
         allOffCount++;
         for (auto& [key, value] : groups)
            value = 0.f;
      };
      fx.snapshotAllLights = [this]() { return groups; };
      fx.restoreAllLights = [this](const std::unordered_map<string, float>& snapshot) { groups = snapshot; };
      return fx;
   }
};
}

TEST_CASE("B2S animation engine steps, loops and stop behaviours")
{
   const auto table = LoadTable(R"(
      <DirectB2SData>
         <Animations>
            <Animation Name="flash" Parent="Backglass" Interval="100" Loops="2">
               <AnimationStep Step="1" On="grpA" WaitLoopsAfterOn="1" Off="grpA" WaitLoopsAfterOff="1"/>
            </Animation>
            <Animation Name="autostart" Parent="Backglass" Interval="50" Loops="0" StartAnimationAtBackglassStartup="1">
               <AnimationStep Step="1" On="grpB" WaitLoopsAfterOn="1"/>
            </Animation>
            <Animation Name="empty" Parent="Backglass" Interval="0"/>
            <Animation Name="nosteps" Parent="Backglass" Interval="50"/>
         </Animations>
      </DirectB2SData>)");

   // Animations without interval or steps are dropped
   REQUIRE(table->m_backglassAnimations.size() == 2);

   AnimFx animFx;
   const B2SAnimationEffects fx = animFx.Make();

   B2SAnimation& anim = table->m_backglassAnimations[0];
   CHECK(!anim.IsRunning());

   // Start requests are consumed by the update loop; the first step happens after one interval
   anim.Start();
   anim.Update(0.05f, fx);
   CHECK(anim.IsRunning());
   CHECK(animFx.groups["grpA"] == 0.f);
   anim.Update(0.05f, fx);
   CHECK(animFx.groups["grpA"] == 1.f); // On grpA (tick 1)
   anim.Update(0.1f, fx);
   CHECK(animFx.groups["grpA"] == 0.f); // Off grpA (tick 2), loop 1 done
   anim.Update(0.1f, fx);
   CHECK(animFx.groups["grpA"] == 1.f); // On grpA (tick 3)
   CHECK(anim.IsRunning());
   anim.Update(0.1f, fx); // Off grpA (tick 4), loop 2 done => end of animation
   CHECK(!anim.IsRunning());
   // Default LightsStateAtAnimationEnd is InvolvedLightsOff
   CHECK(animFx.groups["grpA"] == 0.f);

   // Starting an already running animation is ignored
   anim.Start();
   anim.Update(0.1f, fx);
   CHECK(anim.IsRunning());
   CHECK(animFx.groups["grpA"] == 1.f);
   anim.Start(); // ignored
   anim.Update(0.1f, fx);
   CHECK(animFx.groups["grpA"] == 0.f); // Off, not restarted

   // Stop immediately (default stop behaviour)
   anim.Start();
   anim.Update(0.05f, fx);
   anim.Stop();
   anim.Update(0.05f, fx);
   CHECK(!anim.IsRunning());

   // Autostart animation runs on its first update
   B2SAnimation& autoAnim = table->m_backglassAnimations[1];
   autoAnim.Update(0.05f, fx);
   CHECK(autoAnim.IsRunning());
   CHECK(animFx.groups["grpB"] == 1.f);
   autoAnim.Stop();
   autoAnim.Update(0.05f, fx);
   CHECK(!autoAnim.IsRunning());
}

TEST_CASE("B2S animation reverse, reset and pulse switch")
{
   const auto table = LoadTable(R"(
      <DirectB2SData>
         <Animations>
            <Animation Name="blink" Parent="Backglass" Interval="100" Loops="1"
                       LightsStateAtAnimationEnd="3" LockInvolvedLamps="1" HideScoreDisplays="1">
               <AnimationStep Step="1" On="grpA" WaitLoopsAfterOn="1" Off="grpA" WaitLoopsAfterOff="1"/>
               <AnimationStep Step="2" On="grpB" WaitLoopsAfterOn="1" Off="grpB" WaitLoopsAfterOff="1"/>
               <AnimationStep Step="3" PulseSwitch="44"/>
            </Animation>
         </Animations>
      </DirectB2SData>)");

   REQUIRE(table->m_backglassAnimations.size() == 1);
   B2SAnimation& anim = table->m_backglassAnimations[0];

   AnimFx animFx;
   animFx.groups["grpA"] = 1.f; // light initially on
   const B2SAnimationEffects fx = animFx.Make();

   // The first step is evaluated on the same update that starts the animation
   anim.Start();
   anim.Update(0.1f, fx);
   CHECK(anim.IsRunning());
   CHECK(animFx.groups["grpA"] == 1.f); // step 1 on
   CHECK(animFx.locks["grpA"] == 1);
   CHECK(animFx.locks["grpB"] == 1);
   CHECK(animFx.scoreDisplaysHidden);
   anim.Update(0.1f, fx);
   CHECK(animFx.groups["grpA"] == 0.f); // step 1 off
   anim.Update(0.1f, fx);
   CHECK(animFx.groups["grpB"] == 1.f); // step 2 on
   anim.Update(0.1f, fx);
   CHECK(animFx.groups["grpB"] == 0.f); // step 2 off
   anim.Update(0.1f, fx);
   CHECK(animFx.pulses == vector<int> { 44 }); // step 3 pulses the switch then ends (1 loop)
   CHECK(!anim.IsRunning());
   CHECK(animFx.groups["grpA"] == 1.f); // LightsReseted restores the snapshot taken at start
   CHECK(animFx.groups["grpB"] == 0.f);
   CHECK(animFx.locks.empty());
   CHECK(!animFx.scoreDisplaysHidden);

   // Reverse run replays the on/off pairs back to front, each reversed on its on counterpart
   animFx.pulses.clear();
   anim.Start(true);
   anim.Update(0.1f, fx);
   CHECK(anim.IsRunning());
   CHECK(animFx.pulses == vector<int> { 44 }); // the last entry action (pulse) is hit first
   CHECK(animFx.groups["grpB"] == 1.f); // step 2 off replays its on counterpart
   anim.Update(0.1f, fx);
   CHECK(animFx.groups["grpB"] == 0.f); // step 2 on replays its off counterpart
   anim.Update(0.1f, fx);
   CHECK(animFx.groups["grpA"] == 1.f); // step 1 off replays on
   anim.Update(0.1f, fx);
   CHECK(animFx.groups["grpA"] == 1.f); // step 1 on replays off, then the run ends and restores the snapshot
   CHECK(!anim.IsRunning());
}

TEST_CASE("B2S animation ROM triggers (IDJoin)")
{
   const auto table = LoadTable(R"(
      <DirectB2SData>
         <Animations>
            <Animation Name="lamped" Parent="Backglass" Interval="100" Loops="0" IDJoin="L10,S7">
               <AnimationStep Step="1" On="grpA" WaitLoopsAfterOn="1"/>
            </Animation>
         </Animations>
      </DirectB2SData>)");

   REQUIRE(table->m_backglassAnimations.size() == 1);
   B2SAnimation& anim = table->m_backglassAnimations[0];
   REQUIRE(anim.GetRomTriggers().size() == 2);
   CHECK(anim.GetRomTriggers()[0].romIdType == B2SRomIDType::Lamp);
   CHECK(anim.GetRomTriggers()[0].romId == 10);
   CHECK(anim.GetRomTriggers()[1].romIdType == B2SRomIDType::Solenoid);
   CHECK(anim.GetRomTriggers()[1].romId == 7);

   // Bind fake ROM state readers
   float lamp10 = 0.f, sol7 = 0.f;
   anim.BindRomTriggers(
      [&lamp10, &sol7](B2SRomIDType type, int id, bool, float* target) -> std::function<void()>
      {
         const float* src = (type == B2SRomIDType::Lamp) ? &lamp10 : &sol7;
         return [src, target]() { *target = *src; };
      });

   AnimFx animFx;
   const B2SAnimationEffects fx = animFx.Make();

   // Rising edge on lamp 10 starts the animation, falling edge stops it
   lamp10 = 1.f;
   anim.Update(0.f, fx);
   CHECK(anim.IsRunning());
   anim.Update(0.1f, fx);
   CHECK(animFx.groups["grpA"] == 1.f);
   lamp10 = 0.f;
   anim.Update(0.05f, fx);
   CHECK(!anim.IsRunning()); // stop on falling edge (default: stop immediately)

   // The solenoid trigger works the same
   sol7 = 1.f;
   anim.Update(0.f, fx);
   CHECK(anim.IsRunning());
   anim.Stop();
   anim.Update(0.f, fx);
   CHECK(!anim.IsRunning());
}

TEST_CASE("B2S bulbs are sorted by ZOrder")
{
   const auto table = LoadTable(R"(
      <DirectB2SData>
         <Illumination>
            <Bulb Name="b1" Parent="Backglass" ZOrder="2"/>
            <Bulb Name="b2" Parent="Backglass"/>
            <Bulb Name="b3" Parent="Backglass" ZOrder="1"/>
            <Bulb Name="b4" Parent="DMD" ZOrder="1"/>
            <Bulb Name="b5" Parent="DMD"/>
         </Illumination>
      </DirectB2SData>)");

   // Bulbs without ZOrder keep file order, then ZOrder ascending (drawn on top)
   REQUIRE(table->m_backglassIlluminations.size() == 3);
   CHECK(table->m_backglassIlluminations[0]->m_name == "b2");
   CHECK(table->m_backglassIlluminations[1]->m_name == "b3");
   CHECK(table->m_backglassIlluminations[2]->m_name == "b1");
   REQUIRE(table->m_dmdIlluminations.size() == 2);
   CHECK(table->m_dmdIlluminations[0]->m_name == "b5");
   CHECK(table->m_dmdIlluminations[1]->m_name == "b4");
}
