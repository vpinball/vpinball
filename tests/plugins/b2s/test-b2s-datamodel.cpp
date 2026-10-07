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

   // No display is hidden without a DisplayState=1 attribute
   CHECK(!display->IsHidden());
}

TEST_CASE("B2S hidden score displays")
{
   const auto table = LoadTable(R"(
      <DirectB2SData>
         <Scores>
            <Score ID="1" Parent="Backglass" Digits="4" ReelType="reel_0" DisplayState="1"/>
            <Score ID="2" Parent="Backglass" Digits="2" ReelType="dream7 7" DisplayState="0"/>
            <Score ID="3" Parent="DMD" Digits="3" ReelType="reel_00" DisplayState="1"/>
         </Scores>
      </DirectB2SData>)");

   // DisplayState=1 marks the display as initially hidden
   CHECK(table->FindScoreDisplay(1)->IsHidden());
   CHECK(!table->FindScoreDisplay(2)->IsHidden());
   CHECK(table->FindScoreDisplay(3)->IsHidden());
}

TEST_CASE("B2S LED display segment types")
{
   const auto table = LoadTable(R"(
      <DirectB2SData>
         <Scores>
            <Score ID="1" Parent="Backglass" Digits="4" ReelType="Dream7LED7"/>
            <Score ID="2" Parent="Backglass" Digits="4" ReelType="Dream7LED9"/>
            <Score ID="3" Parent="Backglass" Digits="4" ReelType="Dream7LED14"/>
            <Score ID="4" Parent="Backglass" Digits="4" ReelType="RenderedLED8"/>
            <Score ID="5" Parent="Backglass" Digits="4" ReelType="RenderedLED14" ReelLitColor="255.128.64" ReelDarkColor="32.16.8"/>
            <Score ID="6" Parent="Backglass" Digits="4" ReelType="reel_0"/>
         </Scores>
      </DirectB2SData>)");

   // The ReelType suffix selects the segment layout; Dream7LEDx and RenderedLEDx pick the renderer
   CHECK(table->FindScoreDisplay(1)->m_scoreType == B2SScoreRenderer::Dream7);
   CHECK(table->FindScoreDisplay(1)->m_ledSegments == 7);
   CHECK(table->FindScoreDisplay(2)->m_ledSegments == 10);
   CHECK(table->FindScoreDisplay(3)->m_ledSegments == 14);
   CHECK(table->FindScoreDisplay(4)->m_scoreType == B2SScoreRenderer::RenderedLED);
   CHECK(table->FindScoreDisplay(4)->m_ledSegments == 7);
   CHECK(table->FindScoreDisplay(5)->m_ledSegments == 14);
   CHECK(table->FindScoreDisplay(6)->m_scoreType == B2SScoreRenderer::Reel);
   CHECK(table->FindScoreDisplay(6)->m_ledSegments == 0);

   // Lit/dark segment colors are parsed for the styled segment rendering
   const B2SScore* styled = table->FindScoreDisplay(5);
   CHECK(styled->m_reelLitColor.x == doctest::Approx(1.f));
   CHECK(styled->m_reelLitColor.y == doctest::Approx(128.f / 255.f));
   CHECK(styled->m_reelLitColor.z == doctest::Approx(64.f / 255.f));
   CHECK(styled->m_reelDarkColor.z == doctest::Approx(8.f / 255.f));
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
   CHECK(dream7->m_ledSegments == 7);

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

TEST_CASE("B2S self rotating images")
{
   const auto table = LoadTable(R"(
      <DirectB2SData>
         <Illumination>
            <Bulb Name="cw" Parent="Backglass" SnippitType="1" SnippitRotatingSteps="4" SnippitRotatingInterval="100"
                  eSnippitRotationDirection="0" SnippitRotatingStopBehaviour="1"/>
            <Bulb Name="ccw" Parent="Backglass" SnippitType="1" SnippitRotatingSteps="4" SnippitRotatingInterval="100"
                  eSnippitRotationDirection="1" SnippitRotatingStopBehaviour="1"/>
            <Bulb Name="angle" Parent="Backglass" SnippitType="1" SnippitRotatingAngle="45" SnippitRotatingInterval="100"
                  SnippitRotatingStopBehaviour="1"/>
            <Bulb Name="both" Parent="Backglass" SnippitType="1" SnippitRotatingSteps="8" SnippitRotatingAngle="45" SnippitRotatingInterval="100"
                  SnippitRotatingStopBehaviour="1"/>
            <Bulb Name="spin" Parent="Backglass" SnippitType="1" SnippitRotatingSteps="4" SnippitRotatingInterval="100"
                  SnippitRotatingStopBehaviour="0"/>
            <Bulb Name="tillend" Parent="Backglass" SnippitType="1" SnippitRotatingSteps="4" SnippitRotatingInterval="100"
                  SnippitRotatingStopBehaviour="2"/>
            <Bulb Name="nosteps" Parent="Backglass" SnippitType="1" SnippitRotatingInterval="100"/>
         </Illumination>
      </DirectB2SData>)");

   REQUIRE(table->m_backglassIlluminations.size() == 7);
   const auto bulb = [&table](const char* name) -> B2SBulb*
   {
      for (auto& b : table->m_backglassIlluminations)
         if (b->m_name == name)
            return b.get();
      return nullptr;
   };

   // Parsing: steps can be given directly or through the per-step angle; steps wins if both are present
   CHECK(bulb("cw")->m_snippitType == B2SSnippitType::SelfRotatingImage);
   CHECK(bulb("cw")->m_snippitRotatingSteps == 4);
   CHECK(bulb("cw")->m_snippitRotatingInterval == 100);
   CHECK(bulb("cw")->m_snippitRotatingDirection == B2SSnippitRotationDirection::Clockwise);
   CHECK(bulb("angle")->m_snippitRotatingSteps == 8); // 360 / 45
   CHECK(bulb("both")->m_snippitRotatingSteps == 8); // SnippitRotatingSteps takes precedence

   // Clockwise rotation advances 90 degrees per 100ms interval step
   B2SBulb* cw = bulb("cw");
   cw->UpdateRotation(0.05f);
   CHECK(cw->GetRotationAngle() == doctest::Approx(0.f));
   cw->StartRotation();
   cw->UpdateRotation(0.05f);
   CHECK(cw->IsRotating());
   CHECK(cw->GetRotationAngle() == doctest::Approx(45.f));
   cw->UpdateRotation(0.05f);
   CHECK(cw->GetRotationAngle() == doctest::Approx(90.f));

   // Immediate stop freezes the angle
   cw->StopRotation();
   cw->UpdateRotation(0.05f);
   CHECK(!cw->IsRotating());
   CHECK(cw->GetRotationAngle() == doctest::Approx(90.f));

   // Anti-clockwise rotates the other way (360-45)
   B2SBulb* ccw = bulb("ccw");
   ccw->StartRotation();
   ccw->UpdateRotation(0.05f);
   CHECK(ccw->GetRotationAngle() == doctest::Approx(315.f));

   // Rotation wraps at 360
   ccw->UpdateRotation(0.3f);
   CHECK(ccw->GetRotationAngle() == doctest::Approx(45.f));

   // Spin off keeps rotating while slowing down, then stops
   B2SBulb* spin = bulb("spin");
   spin->StartRotation();
   spin->UpdateRotation(0.05f);
   spin->StopRotation();
   spin->UpdateRotation(0.05f);
   CHECK(spin->IsRotating()); // still spinning down
   for (int i = 0; i < 40; i++)
      spin->UpdateRotation(0.1f);
   CHECK(!spin->IsRotating());

   // Run till end completes the revolution and stops on the first step
   B2SBulb* tillEnd = bulb("tillend");
   tillEnd->StartRotation();
   tillEnd->UpdateRotation(0.05f);
   CHECK(tillEnd->GetRotationAngle() == doctest::Approx(45.f));
   tillEnd->StopRotation();
   tillEnd->UpdateRotation(0.05f);
   CHECK(tillEnd->IsRotating()); // keeps rotating until the revolution is over
   CHECK(tillEnd->GetRotationAngle() == doctest::Approx(90.f));
   for (int i = 0; i < 8; i++)
      tillEnd->UpdateRotation(0.1f);
   CHECK(!tillEnd->IsRotating());
   CHECK(tillEnd->GetRotationAngle() == doctest::Approx(0.f));

   // A rotating snippit without steps or interval never rotates
   B2SBulb* noSteps = bulb("nosteps");
   noSteps->StartRotation();
   noSteps->UpdateRotation(0.5f);
   CHECK(!noSteps->IsRotating());
}

TEST_CASE("B2S sounds and reel sound names")
{
   const auto table = LoadTable(R"(
      <DirectB2SData>
         <Sounds>
            <Sound Name="click" Stream="QUJD"/>
            <Sound Name="legacy" Value="REVG"/>
         </Sounds>
         <Scores>
            <Score ID="1" Parent="Backglass" Digits="3" ReelType="reel_0" Sound="roll" Sound1="a" Sound3="c"/>
         </Scores>
      </DirectB2SData>)");

   // Sounds are stored as base64 WAV streams in the 'Stream' attribute ('Value' in older files)
   REQUIRE(table->m_sounds.size() == 2);
   CHECK(table->m_sounds[0].m_name == "click");
   REQUIRE(table->m_sounds[0].m_wav != nullptr);
   CHECK(table->m_sounds[0].m_wav->size() == 3);
   CHECK(table->m_sounds[0].m_wav->at(0) == 'A');
   REQUIRE(table->m_sounds[1].m_wav != nullptr);
   CHECK(table->m_sounds[1].m_wav->size() == 3);
   CHECK(table->m_sounds[1].m_wav->at(0) == 'D');

   // Per digit reel sounds (Sound1..SoundN attributes)
   const B2SScore* score = table->FindScoreDisplay(1);
   REQUIRE(score != nullptr);
   CHECK(score->m_soundName == "roll");
   REQUIRE(score->m_soundNames.size() == 3);
   CHECK(score->m_soundNames[0] == "a");
   CHECK(score->m_soundNames[1] == "");
   CHECK(score->m_soundNames[2] == "c");
}

TEST_CASE("B2S WAV decoding")
{
   WavData wavData;
   CHECK(!DecodeWav({}, wavData));
   CHECK(!DecodeWav({ 'N', 'O', 'P', 'E' }, wavData));

   // Build a minimal 16-bit PCM mono WAV in memory
   const auto buildWav = [](int format, int channels, int bits, const vector<uint8_t>& data)
   {
      vector<uint8_t> wav;
      const auto tag = [&wav](const char* s) { wav.insert(wav.end(), s, s + 4); };
      const auto u32 = [&wav](uint32_t v) { wav.insert(wav.end(), { (uint8_t)v, (uint8_t)(v >> 8), (uint8_t)(v >> 16), (uint8_t)(v >> 24) }); };
      const auto u16 = [&wav](uint16_t v) { wav.insert(wav.end(), { (uint8_t)v, (uint8_t)(v >> 8) }); };
      tag("RIFF");
      u32(0);
      tag("WAVE");
      tag("fmt ");
      u32(16);
      u16((uint16_t)format);
      u16((uint16_t)channels);
      u32(22050);
      u32(22050 * channels * (bits / 8));
      u16((uint16_t)(channels * (bits / 8)));
      u16((uint16_t)bits);
      tag("data");
      u32((uint32_t)data.size());
      wav.insert(wav.end(), data.begin(), data.end());
      return wav;
   };

   // 16-bit PCM passes through
   REQUIRE(DecodeWav(buildWav(1, 1, 16, { 0x34, 0x12, 0xCD, 0xAB }), wavData));
   CHECK(wavData.channels == 1);
   CHECK(wavData.sampleRate == 22050.);
   CHECK(!wavData.isFloat);
   REQUIRE(wavData.pcm.size() == 4);
   CHECK(wavData.pcm[0] == 0x34);
   CHECK(wavData.pcm[1] == 0x12);

   // 8-bit unsigned PCM is converted to signed 16-bit
   REQUIRE(DecodeWav(buildWav(1, 1, 8, { 0, 128, 255 }), wavData));
   REQUIRE(wavData.pcm.size() == 6);
   const int16_t* s16 = reinterpret_cast<const int16_t*>(wavData.pcm.data());
   CHECK(s16[0] == -32768);
   CHECK(s16[1] == 0);
   CHECK(s16[2] == 32512);

   // 32-bit float passes through
   const float fsample = 0.5f;
   vector<uint8_t> fdata(4);
   memcpy(fdata.data(), &fsample, 4);
   REQUIRE(DecodeWav(buildWav(3, 2, 32, fdata), wavData));
   CHECK(wavData.isFloat);
   CHECK(wavData.channels == 2);

   // Unsupported formats are rejected
   CHECK(!DecodeWav(buildWav(85, 1, 16, { 0, 0 }), wavData));
   CHECK(!DecodeWav(buildWav(1, 4, 16, { 0, 0 }), wavData));
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

TEST_CASE("B2S reel image lookup and rolling metadata")
{
   const auto table = LoadTable(R"(
      <DirectB2SData>
         <Reels>
            <Images>
               <Image Name="reel_00" Image="" CountOfIntermediates="2" IntermediateImage1="" IntermediateImage2=""/>
               <Image Name="reel_05" Image=""/>
               <Image Name="reel_5" Image=""/>
               <Image Name="reel_Empty" Image=""/>
            </Images>
            <IlluminatedImages>
               <IlluminatedImage Name="reel_00" Image=""/>
               <Set ID="2">
                  <IlluminatedImage Name="reel_05" Image=""/>
               </Set>
            </IlluminatedImages>
         </Reels>
         <Scores ReelRollingInterval="80" ReelRollingDirection="Down" ReelCountOfIntermediates="3">
            <Score ID="1" Parent="Backglass" Digits="3" ReelType="reel_00" B2SStartDigit="5"
                   ReelIlluB2SID="12" ReelIlluB2SValue="1" ReelIlluImageSet="2" ReelIlluIntensity="30"/>
         </Scores>
      </DirectB2SData>)");

   // Scores level rolling attributes
   CHECK(table->m_backglassScores.m_reelRollingInterval == 80);
   CHECK(table->m_backglassScores.m_reelRollingDirection == B2SReelRollingDirection::Down);
   CHECK(table->m_backglassScores.m_reelCountOfIntermediates == 3);

   // Score level reel illumination attributes
   const B2SScore* score = table->FindScoreDisplay(1);
   REQUIRE(score != nullptr);
   CHECK(score->m_reelIlluB2SID == 12);
   CHECK(score->m_reelIlluB2SValue == 1);
   CHECK(score->m_reelIlluImageSet == 2);
   CHECK(score->m_reelIlluIntensity == 30);

   // Reel images: "reel_00" is a 2 digit field (reel_05), "reel_0" a 1 digit field (reel_5), -1 is the empty image
   const B2SReelImage* img = table->m_reels.GetImage("reel_00", 0);
   REQUIRE(img != nullptr);
   CHECK(img->m_countOfIntermediate == 2);
   REQUIRE(img->m_intermediates.size() == 2);
   CHECK(table->m_reels.GetImage("reel_00", 5)->m_name == "reel_05");
   CHECK(table->m_reels.GetImage("reel_0", 5)->m_name == "reel_5");
   CHECK(table->m_reels.GetImage("reel_00", -1)->m_name == "reel_Empty");
   CHECK(table->m_reels.GetImage("reel_00", 9) == nullptr);

   // Illuminated images: flat set when no image set is selected, _setId suffixed names otherwise
   CHECK(table->m_reels.GetImage("reel_00", 0, true)->m_name == "reel_00");
   CHECK(table->m_reels.GetImage("reel_00", 5, true, 2)->m_name == "reel_05_2");
   CHECK(table->m_reels.GetImage("reel_00", 5, true) == nullptr); // no flat reel_05 illuminated image
   CHECK(table->m_reels.GetImage("reel_00", 5, true, 1) == nullptr); // no set 1
}

TEST_CASE("B2S reel digit rolling")
{
   B2SReelDigit digit;

   // Non animated changes are instant
   digit.SetTarget(3, false);
   digit.Update(1.f, 100, 2, true);
   CHECK(digit.Current() == 3);
   CHECK(!digit.IsRolling());

   // Animated changes roll one digit per rolling interval, spread over intermediate + settle ticks.
   // interval 100ms with 2 intermediates: 25ms sub ticks, 5 ticks per digit step (int1, int2, advance, settle, settle)
   digit.SetTarget(7, true);
   CHECK(digit.IsRolling());
   CHECK(digit.Current() == 3);
   int advances = 0;
   digit.Update(0.025f, 100, 2, true);
   CHECK(digit.Intermediate() == 1);
   digit.Update(0.025f, 100, 2, true);
   CHECK(digit.Intermediate() == 2);
   CHECK(digit.Current() == 3);
   advances += digit.Update(0.025f, 100, 2, true) ? 1 : 0;
   CHECK(digit.Current() == 4); // the digit advances after the intermediates
   CHECK(digit.Intermediate() == 0);
   CHECK(advances == 1); // each advance triggers a reel sound
   advances += digit.Update(0.05f, 100, 2, true) ? 1 : 0; // 2 settle ticks
   CHECK(digit.Current() == 4);

   // Roll the remaining steps (5 ticks per digit)
   for (int i = 0; i < 15; i++)
      advances += digit.Update(0.025f, 100, 2, true) ? 1 : 0;
   CHECK(digit.Current() == 7);
   CHECK(!digit.IsRolling());
   CHECK(advances == 4); // 3 -> 7 is 4 digit advances

   // Rolling down goes the other way and wraps through 0
   digit.SetTarget(5, false);
   digit.Update(0.f, 100, 0, false);
   digit.SetTarget(3, true);
   for (int i = 0; i < 20 && digit.IsRolling(); i++)
      digit.Update(0.05f, 100, 0, false);
   CHECK(digit.Current() == 3);
   digit.SetTarget(1, false);
   digit.Update(0.f, 100, 0, false);
   digit.SetTarget(8, true);
   for (int i = 0; i < 20 && digit.IsRolling(); i++)
      digit.Update(0.05f, 100, 0, false);
   CHECK(digit.Current() == 8); // 1 -> 0 -> 9 -> 8

   // Rolling up wraps through 9, and the wrap is pending until the digit passes it
   digit.SetTarget(8, false);
   digit.Update(0.f, 100, 0, true);
   digit.SetTarget(2, true);
   CHECK(digit.HasPendingWrap(true));
   int ticks = 0;
   while (digit.IsRolling() && ticks < 100)
   {
      digit.Update(0.05f, 100, 0, true);
      ticks++;
      if (digit.Current() <= 2)
         break;
   }
   CHECK(digit.Current() == 0);
   CHECK(digit.IsRolling());
   CHECK(!digit.HasPendingWrap(true)); // wrapped, more significant digits may roll now
   while (digit.IsRolling() && ticks < 100)
   {
      digit.Update(0.05f, 100, 0, true);
      ticks++;
   }
   CHECK(digit.Current() == 2);

   // Rolling to a blank digit is instant
   digit.SetTarget(-1, true);
   CHECK(!digit.IsRolling());
   CHECK(digit.Current() == -1);

   // A rolling interval below 10ms falls back to the default, it does not snap
   digit.SetTarget(0, false);
   digit.Update(0.f, 0, 0, true);
   digit.SetTarget(4, true);
   CHECK(digit.IsRolling());
   digit.Update(0.f, 0, 0, true);
   CHECK(digit.IsRolling());
}

TEST_CASE("B2S bulbs baked into the background image")
{
   const auto table = LoadTable(R"(
      <DirectB2SData>
         <Images>
            <BackglassOnImage Value="" RomID="4" RomIDType="1"/>
         </Images>
         <Illumination>
            <Bulb Name="baked" Parent="Backglass" RomID="4" RomIDType="1"/>
            <Bulb Name="inverted" Parent="Backglass" RomID="4" RomIDType="1" RomInverted="1"/>
            <Bulb Name="othertype" Parent="Backglass" RomID="4" RomIDType="2"/>
            <Bulb Name="otherid" Parent="Backglass" RomID="5" RomIDType="1"/>
            <Bulb Name="b2sid" Parent="Backglass" B2SID="4"/>
            <Bulb Name="anim" Parent="Backglass" RomID="4" RomIDType="1"/>
            <Bulb Name="dmd" Parent="DMD" RomID="4" RomIDType="1"/>
            <Bulb Name="norom" Parent="Backglass"/>
         </Illumination>
         <Animations>
            <Animation Name="a" Parent="Backglass" Interval="50">
               <AnimationStep Step="1" On="anim" WaitLoopsAfterOn="1"/>
            </Animation>
         </Animations>
      </DirectB2SData>)");

   const auto bulb = [&table](const char* name) -> B2SBulb*
   {
      for (auto& b : table->m_backglassIlluminations)
         if (b->m_name == name)
            return b.get();
      return nullptr;
   };

   // Bulbs sharing the BackglassOnImage ROM channel are baked into the lit background
   CHECK(bulb("baked")->m_bakedIntoBackground);
   CHECK(bulb("b2sid")->m_bakedIntoBackground); // a B2SID bulb matches a lamp ROM channel

   // Different type, inverted or other channel: drawn as a normal bulb
   CHECK(!bulb("inverted")->m_bakedIntoBackground);
   CHECK(!bulb("othertype")->m_bakedIntoBackground);
   CHECK(!bulb("otherid")->m_bakedIntoBackground);
   CHECK(!bulb("norom")->m_bakedIntoBackground);

   // A bulb driven by an animation keeps drawing on top (SetThruAnimation)
   CHECK(!bulb("anim")->m_bakedIntoBackground);

   // DMD bulbs are never baked into the backglass image
   REQUIRE(table->m_dmdIlluminations.size() == 1);
   CHECK(!table->m_dmdIlluminations[0]->m_bakedIntoBackground);
}

TEST_CASE("B2S dual backglass mode")
{
   const auto table = LoadTable(R"(
      <DirectB2SData>
         <DualBackglass Value="1"/>
         <Illumination>
            <Bulb Name="both" Parent="Backglass" DualMode="0"/>
            <Bulb Name="auth" Parent="Backglass" DualMode="1"/>
            <Bulb Name="fant" Parent="Backglass" DualMode="2"/>
            <Bulb Name="def" Parent="Backglass"/>
         </Illumination>
         <Animations>
            <Animation Name="shared" Parent="Backglass" Interval="50" DualMode="0">
               <AnimationStep Step="1" On="both" WaitLoopsAfterOn="1"/>
            </Animation>
            <Animation Name="authOnly" Parent="Backglass" Interval="50" DualMode="1">
               <AnimationStep Step="1" On="auth" WaitLoopsAfterOn="1"/>
            </Animation>
            <Animation Name="fantOnly" Parent="Backglass" Interval="50" DualMode="2">
               <AnimationStep Step="1" On="fant" WaitLoopsAfterOn="1"/>
            </Animation>
         </Animations>
      </DirectB2SData>)");

   CHECK(table->m_dualBackglass);

   // Bulb dual mode parsing (Both is the default)
   CHECK(table->m_backglassIlluminations[0]->m_dualMode == B2SDualMode::Both);
   CHECK(table->m_backglassIlluminations[1]->m_dualMode == B2SDualMode::Authentic);
   CHECK(table->m_backglassIlluminations[2]->m_dualMode == B2SDualMode::Fantasy);
   CHECK(table->m_backglassIlluminations[3]->m_dualMode == B2SDualMode::Both);

   CHECK(table->m_backglassAnimations[0].m_dualMode == B2SDualMode::Both);
   CHECK(table->m_backglassAnimations[1].m_dualMode == B2SDualMode::Authentic);
   CHECK(table->m_backglassAnimations[2].m_dualMode == B2SDualMode::Fantasy);

   // Animations only start in their declared mode
   AnimFx animFx;
   B2SAnimationEffects fx = animFx.Make();

   fx.dualMode = B2SDualMode::Fantasy;
   for (auto& anim : table->m_backglassAnimations)
      anim.Start();
   for (auto& anim : table->m_backglassAnimations)
      anim.Update(0.f, fx);
   CHECK(table->m_backglassAnimations[0].IsRunning()); // Both runs in any mode
   CHECK(!table->m_backglassAnimations[1].IsRunning()); // Authentic dropped in Fantasy
   CHECK(table->m_backglassAnimations[2].IsRunning()); // Fantasy runs
   for (auto& anim : table->m_backglassAnimations)
      anim.Stop();
   for (auto& anim : table->m_backglassAnimations)
      anim.Update(0.f, fx);

   // Without a dual backglass (Both = unfiltered), every animation may start
   fx.dualMode = B2SDualMode::Both;
   for (auto& anim : table->m_backglassAnimations)
      anim.Start();
   for (auto& anim : table->m_backglassAnimations)
      anim.Update(0.f, fx);
   CHECK(table->m_backglassAnimations[1].IsRunning());
   CHECK(table->m_backglassAnimations[2].IsRunning());
}

TEST_CASE("B2S animation slowdown setting parsing")
{
   CHECK(B2SAnimationSlowDown(""s, "flash"s) == 1);
   CHECK(B2SAnimationSlowDown("flash"s, "flash"s) == 1); // entry without = is ignored
   CHECK(B2SAnimationSlowDown("flash=3"s, "flash"s) == 3);
   CHECK(B2SAnimationSlowDown("flash=3"s, "other"s) == 1);
   CHECK(B2SAnimationSlowDown(" flash = 2 ; diver=4 "s, "flash"s) == 2);
   CHECK(B2SAnimationSlowDown(" flash = 2 ; diver=4 "s, "diver"s) == 4);
   CHECK(B2SAnimationSlowDown("flash=0"s, "flash"s) == 1); // clamped to a positive factor
}

TEST_CASE("b2s LED segment masks")
{
   // Character masks follow the Dream7 DisplayCharacter tables in CTLPI bit order

   // 7 segments: a..g = bits 0..6
   CHECK(B2SSegmentCharMask('8', 7) == 0x7F);
   CHECK(B2SSegmentCharMask('0', 7) == 0x3F);
   CHECK(B2SSegmentCharMask('1', 7) == 0x06);
   CHECK(B2SSegmentCharMask('E', 7) == 0x79);
   CHECK(B2SSegmentCharMask(' ', 7) == 0x00);

   // 14 segments: a..f = bits 0..5, g1 = 6, h = 8, i = 9, j = 10, g2 = 11, k = 12, l = 13, m = 14
   // Dream7 '0' = abcdefjk (slashed zero)
   CHECK(B2SSegmentCharMask('0', 14) == 0x143F);
   // Dream7 '8' = abcdefg1g2
   CHECK(B2SSegmentCharMask('8', 14) == 0x087F);
   // Dream7 'B' = abcdg2il
   CHECK(B2SSegmentCharMask('B', 14) == 0x280F);
   // Dream7 'W' = bcefkm
   CHECK(B2SSegmentCharMask('W', 14) == 0x5036);
   // Dream7 'I' = adil
   CHECK(B2SSegmentCharMask('I', 14) == 0x2209);
   CHECK(B2SSegmentCharMask('!', 14) == 0x0000);

   // 10 segments: i/l center verticals at bits 8/9 (matching the Gottlieb 9-seg '1' = 0x300)
   CHECK(B2SSegmentCharMask('1', 10) == 0x0300);
   CHECK(B2SSegmentCharMask('8', 10) == 0x007F);

   // Bit code translation from the Dream7 segment order to CTLPI bit order
   // 7 segments: identity on 8 bits
   CHECK(B2SSegmentTranslateBitCode(0xFF, 7) == 0x00FF);
   // 10 segments: g2 shares bit 6 with g1, i/l stay at 8/9
   CHECK(B2SSegmentTranslateBitCode(0x3FF, 10) == 0x037F);
   // 14 segments Dream7 order a,b,c,d,e,f,g1,dot,h,i,j,g2,m,l,k,dot2
   // Dream7 bit7 (dot) lands on the dp bit 15, Dream7 bits 12/13/14 (m/l/k) map to 14/13/12,
   // and no Dream7 bit drives the CTLPI comma (bit 7)
   CHECK(B2SSegmentTranslateBitCode(0xFFFF, 14) == 0xFF7F);
   CHECK(B2SSegmentTranslateBitCode(0x0080, 14) == 0x8000);
   CHECK(B2SSegmentTranslateBitCode(0x1000, 14) == 0x4000);
   CHECK(B2SSegmentTranslateBitCode(0x4000, 14) == 0x1000);
}
