// license:GPLv3+

#include "plugins/MsgPlugin.h"
#include "plugins/ControllerPlugin.h"

#include <functional>
#include <cassert>

#include "common.h"

#include "resources/ResourceDef.h"
#include "resources/AssetSrc.h"
#include "resources/Font.h"
#include "actors/Actor.h"
#include "actors/Actions.h"
#include "actors/Label.h"
#include "actors/Frame.h"
#include "resources/Bitmap.h"
#include "actors/Image.h"
#include "actors/Video.h"
#include "actors/Group.h"
#include "actors/AnimatedActor.h"
#include "UltraDMD.h"
#include "FlexDMD.h"


namespace Flex {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Resource & resource Identifiers

PSC_CLASS_START(FlexDMD_FontDef, FontDef)
PSC_CLASS_END()

PSC_CLASS_START(FlexDMD_VideoDef, VideoDef)
PSC_CLASS_END()

PSC_CLASS_START(FlexDMD_ImageSequenceDef, ImageSequenceDef)
PSC_CLASS_END()

PSC_CLASS_START(FlexDMD_AssetSrc, AssetSrc)
PSC_CLASS_END()

#define PSC_VAR_FlexDMD_Font(variant) PSC_VAR_object(Font, variant)
#define PSC_VAR_SET_FlexDMD_Font(variant, value) PSC_VAR_SET_object(Font, variant, value)
PSC_CLASS_START(FlexDMD_Font, Font)
PSC_CLASS_END()

///////////////////////////////////////////////////////////////////////////////////////////////////
// All actors

#define PSC_VAR_FlexDMD_Alignment(variant) PSC_VAR_enum(Alignment, variant)
#define PSC_VAR_SET_FlexDMD_Alignment(variant, value) PSC_VAR_SET_enum(Alignment, variant, value)
PSC_CLASS_ALIAS(FlexDMD_Alignment, int32)

#define PSC_VAR_FlexDMD_Scaling(variant) PSC_VAR_enum(Scaling, variant)
#define PSC_VAR_SET_FlexDMD_Scaling(variant, value) PSC_VAR_SET_enum(Scaling, variant, value)
PSC_CLASS_ALIAS(FlexDMD_Scaling, int32)

#define PSC_VAR_FlexDMD_Actor(variant) PSC_VAR_object(Actor, variant)
#define PSC_VAR_SET_FlexDMD_ActionFactory(variant, value) PSC_VAR_SET_object(ActionFactory, variant, value)
#define PSC_VAR_FlexDMD_Action(variant) PSC_VAR_object(Action, variant)
PSC_CLASS_START(FlexDMD_Actor, Actor)
   PSC_PROP_RW(string, Name)
   PSC_PROP_RW(float, X)
   PSC_PROP_RW(float, Y)
   PSC_PROP_RW(int, Width)
   PSC_PROP_RW(int, Height)
   PSC_PROP_RW(bool, Visible)
   PSC_PROP_RW(bool, FillParent)
   PSC_PROP_RW(bool, ClearBackground)
   PSC_FUNCTION4(void, SetBounds, float, float, int, int)
   PSC_FUNCTION2(void, SetPosition, float, float)
   PSC_FUNCTION3(void, SetAlignedPosition, float, float, FlexDMD_Alignment)
   PSC_FUNCTION2(void, SetSize, int, int)
   PSC_PROP_RW(int, PrefWidth)
   PSC_PROP_RW(int, PrefHeight)
   PSC_FUNCTION0(void, Pack)
   PSC_FUNCTION0(void, Remove)
   PSC_PROP_R(FlexDMD_ActionFactory, ActionFactory)
   PSC_FUNCTION1(void, AddAction, FlexDMD_Action)
   PSC_FUNCTION0(void, ClearActions)
PSC_CLASS_END()

#define PSC_VAR_FlexDMD_Label(variant) PSC_VAR_object(Label, variant)
#define PSC_VAR_SET_FlexDMD_Label(variant, value) PSC_VAR_SET_object(Label, variant, value)
PSC_CLASS_START(FlexDMD_Label, Label)
   PSC_INHERIT_CLASS(FlexDMD_Actor)
   PSC_PROP_RW(bool, AutoPack)
   PSC_PROP_RW(FlexDMD_Alignment, Alignment)
   PSC_PROP_RW(FlexDMD_Font, Font)
   PSC_PROP_RW(string, Text)
PSC_CLASS_END()

#define PSC_VAR_FlexDMD_Frame(variant) PSC_VAR_object(Frame, variant)
#define PSC_VAR_SET_FlexDMD_Frame(variant, value) PSC_VAR_SET_object(Frame, variant, value)
PSC_CLASS_START(FlexDMD_Frame, Frame)
   PSC_INHERIT_CLASS(FlexDMD_Actor)
   PSC_PROP_RW(int32, Thickness)
   PSC_PROP_RW(int32, BorderColor)
   PSC_PROP_RW(bool, Fill)
   PSC_PROP_RW(int32, FillColor)
PSC_CLASS_END()

#define PSC_VAR_FlexDMD_Bitmap(variant) PSC_VAR_object(Bitmap, variant)
#define PSC_VAR_SET_FlexDMD_Bitmap(variant, value) PSC_VAR_SET_object(Bitmap, variant, value)
PSC_CLASS_START(FlexDMD_Bitmap, Bitmap)
PSC_CLASS_END()

#define PSC_VAR_FlexDMD_Image(variant) PSC_VAR_object(Image, variant)
#define PSC_VAR_SET_FlexDMD_Image(variant, value) PSC_VAR_SET_object(Image, variant, value)
PSC_CLASS_START(FlexDMD_Image, Image)
   PSC_INHERIT_CLASS(FlexDMD_Actor)
   PSC_PROP_RW(FlexDMD_Bitmap, Bitmap)
   PSC_PROP_RW(FlexDMD_Scaling, Scaling)
   PSC_PROP_RW(FlexDMD_Alignment, Alignment)
PSC_CLASS_END()

#define PSC_VAR_SET_FlexDMD_AnimatedActor(variant, value) PSC_VAR_SET_object(AnimatedActor, variant, value)
PSC_CLASS_START(FlexDMD_AnimatedActor, AnimatedActor)
   PSC_INHERIT_CLASS(FlexDMD_Actor)
   PSC_PROP_R(float, Length)
   PSC_PROP_RW(bool, Loop)
   PSC_PROP_RW(bool, Paused)
   PSC_PROP_RW(float, PlaySpeed)
   PSC_PROP_RW(FlexDMD_Scaling, Scaling)
   PSC_PROP_RW(FlexDMD_Alignment, Alignment)
   PSC_FUNCTION1(void, Seek, float)
PSC_CLASS_END()

#define PSC_VAR_FlexDMD_Group(variant) PSC_VAR_object(Group, variant)
#define PSC_VAR_SET_FlexDMD_Group(variant, value) PSC_VAR_SET_object(Group, variant, value)
PSC_CLASS_START(FlexDMD_Group, Group)
   PSC_INHERIT_CLASS(FlexDMD_Actor)
   PSC_PROP_RW(bool, Clip)
   PSC_PROP_R(int32, ChildCount)
   PSC_FUNCTION1(bool, HasChild, string)
   PSC_FUNCTION1(FlexDMD_Group, GetGroup, string)
   PSC_FUNCTION1(FlexDMD_Frame, GetFrame, string)
   PSC_FUNCTION1(FlexDMD_Label, GetLabel, string)
   PSC_FUNCTION1(FlexDMD_AnimatedActor, GetVideo, string)
   PSC_FUNCTION1(FlexDMD_Image, GetImage, string)
   PSC_FUNCTION0(void, RemoveAll)
   PSC_FUNCTION1(void, AddActor, FlexDMD_Actor)
   PSC_FUNCTION1(void, RemoveActor, FlexDMD_Actor)
PSC_CLASS_END()


///////////////////////////////////////////////////////////////////////////////////////////////////
// All actions and ActionFactory

PSC_CLASS_START(FlexDMD_Action, Action)
   PSC_FUNCTION1(bool, Update, float)
PSC_CLASS_END()

PSC_CLASS_START(FlexDMD_AddChildAction, AddChildAction)
   PSC_INHERIT_CLASS(FlexDMD_Action)
PSC_CLASS_END()

PSC_CLASS_START(FlexDMD_BlinkAction, BlinkAction)
   PSC_INHERIT_CLASS(FlexDMD_Action)
PSC_CLASS_END()

PSC_CLASS_START(FlexDMD_DelayedAction, DelayedAction)
   PSC_INHERIT_CLASS(FlexDMD_Action)
PSC_CLASS_END()

#define PSC_VAR_FlexDMD_Interpolation(variant) PSC_VAR_enum(Interpolation, variant)
#define PSC_VAR_SET_FlexDMD_Interpolation(variant, value) PSC_VAR_SET_enum(Interpolation, variant, value)
PSC_CLASS_ALIAS(FlexDMD_Interpolation, int32)

PSC_CLASS_START(FlexDMD_MoveToAction, MoveToAction)
   PSC_INHERIT_CLASS(FlexDMD_Action)
   PSC_PROP_RW(FlexDMD_Interpolation, Ease)
PSC_CLASS_END()

#define PSC_VAR_SET_FlexDMD_ParallelAction(variant, value) PSC_VAR_SET_object(ParallelAction, variant, value)
PSC_CLASS_START(FlexDMD_ParallelAction, ParallelAction)
   PSC_INHERIT_CLASS(FlexDMD_Action)
   PSC_FUNCTION1(void, Add, FlexDMD_Action) // Returning this is not yet implemented
PSC_CLASS_END()

PSC_CLASS_START(FlexDMD_RemoveFromParentAction, RemoveFromParentAction)
   PSC_INHERIT_CLASS(FlexDMD_Action)
PSC_CLASS_END()

PSC_CLASS_START(FlexDMD_RepeatAction, RepeatAction)
   PSC_INHERIT_CLASS(FlexDMD_Action)
PSC_CLASS_END()

PSC_CLASS_START(FlexDMD_SeekAction, SeekAction)
   PSC_INHERIT_CLASS(FlexDMD_Action)
PSC_CLASS_END()

#define PSC_VAR_SET_FlexDMD_SequenceAction(variant, value) PSC_VAR_SET_object(SequenceAction, variant, value)
PSC_CLASS_START(FlexDMD_SequenceAction, SequenceAction)
   PSC_INHERIT_CLASS(FlexDMD_Action)
   PSC_FUNCTION1(void, Add, FlexDMD_Action) // Returning this is not yet implemented
PSC_CLASS_END()

PSC_CLASS_START(FlexDMD_ShowAction, ShowAction)
   PSC_INHERIT_CLASS(FlexDMD_Action)
PSC_CLASS_END()

PSC_CLASS_START(FlexDMD_WaitAction, WaitAction)
   PSC_INHERIT_CLASS(FlexDMD_Action)
PSC_CLASS_END()

#define PSC_VAR_SET_FlexDMD_WaitAction(variant, value) PSC_VAR_SET_object(WaitAction, variant, value)
#define PSC_VAR_SET_FlexDMD_DelayedAction(variant, value) PSC_VAR_SET_object(DelayedAction, variant, value)
#define PSC_VAR_SET_FlexDMD_RepeatAction(variant, value) PSC_VAR_SET_object(RepeatAction, variant, value)
#define PSC_VAR_SET_FlexDMD_BlinkAction(variant, value) PSC_VAR_SET_object(BlinkAction, variant, value)
#define PSC_VAR_SET_FlexDMD_ShowAction(variant, value) PSC_VAR_SET_object(ShowAction, variant, value)
#define PSC_VAR_SET_FlexDMD_AddChildAction(variant, value) PSC_VAR_SET_object(AddChildAction, variant, value)
#define PSC_VAR_SET_FlexDMD_RemoveFromParentAction(variant, value) PSC_VAR_SET_object(RemoveFromParentAction, variant, value)
#define PSC_VAR_SET_FlexDMD_SeekAction(variant, value) PSC_VAR_SET_object(SeekAction, variant, value)
#define PSC_VAR_SET_FlexDMD_MoveToAction(variant, value) PSC_VAR_SET_object(MoveToAction, variant, value)
PSC_CLASS_START(FlexDMD_ActionFactory, ActionFactory)
   PSC_FUNCTION1(FlexDMD_WaitAction, Wait, float)
   PSC_FUNCTION2(FlexDMD_DelayedAction, Delayed, float, FlexDMD_Action)
   PSC_FUNCTION0(FlexDMD_ParallelAction, Parallel)
   PSC_FUNCTION0(FlexDMD_SequenceAction, Sequence)
   PSC_FUNCTION2(FlexDMD_RepeatAction, Repeat, FlexDMD_Action, int)
   PSC_FUNCTION3(FlexDMD_BlinkAction, Blink, float, float, int)
   PSC_FUNCTION1(FlexDMD_ShowAction, Show, bool)
   PSC_FUNCTION1(FlexDMD_AddChildAction, AddTo, FlexDMD_Group)
   PSC_FUNCTION0(FlexDMD_RemoveFromParentAction, RemoveFromParent)
   PSC_FUNCTION1(FlexDMD_AddChildAction, AddChild, FlexDMD_Actor)
   PSC_FUNCTION1(FlexDMD_AddChildAction, RemoveChild, FlexDMD_Actor)
   PSC_FUNCTION1(FlexDMD_SeekAction, Seek, float)
   PSC_FUNCTION3(FlexDMD_MoveToAction, MoveTo, float, float, float)
PSC_CLASS_END()


///////////////////////////////////////////////////////////////////////////////////////////////////
// Main classes

#define PSC_VAR_SET_FlexDMD_UltraDMD(variant, value) PSC_VAR_SET_object(UltraDMD, variant, value)
PSC_CLASS_START(FlexDMD_UltraDMD, UltraDMD)
   PSC_FUNCTION0(void, LoadSetup)
   PSC_FUNCTION0(void, Init)
   PSC_FUNCTION0(void, Uninit)
   PSC_FUNCTION0(int32, GetMajorVersion)
   PSC_FUNCTION0(int32, GetMinorVersion)
   PSC_FUNCTION0(int32, GetBuildNumber)
   PSC_FUNCTION1(bool, SetVisibleVirtualDMD, bool)
   PSC_FUNCTION1(bool, SetFlipY, bool)
   PSC_FUNCTION0(bool, IsRendering)
   PSC_FUNCTION0(void, CancelRendering)
   PSC_FUNCTION1(void, CancelRenderingWithId, string)
   PSC_FUNCTION0(void, Clear)
   PSC_FUNCTION1(void, SetProjectFolder, string)
   PSC_FUNCTION1(void, SetVideoStretchMode, int)
   PSC_FUNCTION3(void, SetScoreboardBackgroundImage, string, int, int)
   PSC_FUNCTION3(int32, CreateAnimationFromImages, int, bool, string)
   PSC_FUNCTION3(int32, RegisterVideo, int, bool, string)
   PSC_FUNCTION0(void, DisplayVersionInfo)
   PSC_FUNCTION8(void, DisplayScoreboard, int, int, int, int, int, int, string, string)
   PSC_FUNCTION8(void, DisplayScoreboard00, int, int, int, int, int, int, string, string)
   PSC_FUNCTION8(void, DisplayScene00, string, string, int, string, int, int, int, int)
   PSC_FUNCTION10(void, DisplayScene00Ex, string, string, int, int, string, int, int, int, int, int)
   PSC_FUNCTION12(void, DisplayScene00ExWithId, string, bool, string, string, int, int, string, int, int, int, int, int)
   PSC_FUNCTION3(void, ModifyScene00, string, string, string)
   PSC_FUNCTION4(void, ModifyScene00Ex, string, string, string, int)
   PSC_FUNCTION8(void, DisplayScene01, string, string, string, int, int, int, int, int)
   PSC_FUNCTION3(void, DisplayText, string, int, int)
   PSC_FUNCTION6(void, ScrollingCredits, string, string, int, int, int, int)
PSC_CLASS_END()

#define PSC_VAR_FlexDMD_RenderMode(variant) PSC_VAR_enum(RenderMode, variant)
#define PSC_VAR_SET_FlexDMD_RenderMode(variant, value) PSC_VAR_SET_enum(RenderMode, variant, value)
PSC_CLASS_ALIAS(FlexDMD_RenderMode, int32)

PSC_ARRAY1(FlexDMD_ByteArray, uint8, 0)
#define PSC_VAR_SET_FlexDMD_ByteArray(variant, value) PSC_VAR_SET_array1(FlexDMD_ByteArray, variant, value)

PSC_ARRAY1(FlexDMD_ShortArray, int16, 0)
#define PSC_VAR_SET_FlexDMD_ShortArray(variant, value) PSC_VAR_SET_array1(FlexDMD_ShortArray, variant, value)
#define PSC_VAR_FlexDMD_ShortArray(variant) PSC_VAR_array1(uint16_t, variant)

PSC_ARRAY1(FlexDMD_IntArray, int32, 0)
#define PSC_VAR_SET_FlexDMD_IntArray(variant, value) PSC_VAR_SET_array1(FlexDMD_IntArray, variant, value)

PSC_CLASS_START(FlexDMD_FlexDMD, FlexDMD)
   PSC_PROP_R(int32, Version)
   PSC_PROP_RW(int32, RuntimeVersion)
   PSC_PROP_RW(bool, Run)
   PSC_PROP_RW(bool, Show)
   PSC_PROP_RW(string, GameName)
   PSC_PROP_RW(int32, Width)
   PSC_PROP_RW(int32, Height)
   PSC_PROP_RW(uint, Color)
   PSC_PROP_RW(FlexDMD_RenderMode, RenderMode)
   PSC_PROP_RW(string, ProjectFolder)
   PSC_PROP_RW(string, TableFile)
   PSC_PROP_RW(bool, Clear)
   PSC_PROP_R(FlexDMD_IntArray, DmdColoredPixels)
   PSC_PROP_R(FlexDMD_ByteArray, DmdPixels)
   PSC_PROP_W(FlexDMD_ShortArray, Segments)
   PSC_PROP_W(FlexDMD_ShortArray, Segments)
   PSC_PROP_R(FlexDMD_Group, Stage)
   PSC_FUNCTION0(void, LockRenderThread)
   PSC_FUNCTION0(void, UnlockRenderThread)
   PSC_FUNCTION1(FlexDMD_Group, NewGroup, string)
   PSC_FUNCTION1(FlexDMD_Frame, NewFrame, string)
   PSC_FUNCTION3(FlexDMD_Label, NewLabel, string, FlexDMD_Font, string)
   PSC_FUNCTION2(FlexDMD_AnimatedActor, NewVideo, string, string)
   PSC_FUNCTION2(FlexDMD_Image, NewImage, string, string)
   PSC_FUNCTION4(FlexDMD_Font, NewFont, string, uint, uint, int)
   PSC_FUNCTION0(FlexDMD_UltraDMD, NewUltraDMD)
PSC_CLASS_END()


///////////////////////////////////////////////////////////////////////////////////////////////////
// Plugin interface

static const MsgPluginAPI* msgApi = nullptr;
static VPXPluginAPI* vpxApi = nullptr;
static ScriptablePluginAPI* scriptApi = nullptr;

static uint32_t endpointId, nextDmdId;

static std::vector<FlexDMD*> flexDmds;


///////////////////////////////////////////////////////////////////////////////////////////////////
// Alphanumeric segment displays

static bool hasAlpha = false;
static unsigned int onSegSrcChangedId, getSegSrcId;
static float segLuminances[16 * 128] = {};

static SegDisplayFrame GetState(const CtlResId id)
{
   // FIXME implement new output ids
   if (id.endpointId != endpointId)
      return { 0, nullptr };
   uint32_t flexId = id.resId >> 8;
   uint32_t subId = id.resId & 0x0FF;
   static int sizes[17][14] = {
      {}, // RenderMode_DMD_GRAY_2
      {}, // RenderMode_DMD_GRAY_4
      {}, // RenderMode_DMD_RGB
      { 16, 16 }, // RenderMode_SEG_2x16Alpha
      { 20, 20 }, // RenderMode_SEG_2x20Alpha
      { 7, 7, 7, 7 }, // RenderMode_SEG_2x7Alpha_2x7Num
      { 7, 7, 7, 7, 1, 1, 1, 1 }, // RenderMode_SEG_2x7Alpha_2x7Num_4x1Num
      { 7, 7, 7, 7, 1, 1, 1, 1 }, // RenderMode_SEG_2x7Num_2x7Num_4x1Num
      { 7, 7, 7, 7, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, // RenderMode_SEG_2x7Num_2x7Num_10x1Num
      { 7, 7, 7, 7, 1, 1, 1, 1 }, // RenderMode_SEG_2x7Num_2x7Num_4x1Num_gen7
      { 7, 7, 7, 7, 1, 1, 1, 1 }, // RenderMode_SEG_2x7Num10_2x7Num10_4x1Num
      { 6, 6, 6, 6, 1, 1, 1, 1 }, // RenderMode_SEG_2x6Num_2x6Num_4x1Num
      { 6, 6, 6, 6, 1, 1, 1, 1 }, // RenderMode_SEG_2x6Num10_2x6Num10_4x1Num
      { 7, 7, 7, 7 }, // RenderMode_SEG_4x7Num10
      { 6, 6, 6, 6, 1, 1, 1, 1 }, // RenderMode_SEG_6x4Num_4x1Num
      { 7, 7, 1, 1, 1, 1, 16 }, // RenderMode_SEG_2x7Num_4x1Num_1x16Alpha,
      { 16, 16, 7 }, // RenderMode_SEG_1x16Alpha_1x16Num_1x7Num
   };
   for (FlexDMD* pFlex : flexDmds)
   {
      if ((pFlex->GetShow()) && (pFlex->GetId() == flexId))
      {
         int pos = 0;
         float* lum = segLuminances;
         for (uint32_t i = 0; i < subId; i++)
         {
            pos += sizes[pFlex->GetRenderMode()][i];
            lum += sizes[pFlex->GetRenderMode()][i] * 16;
         }
         for (int i = 0; i < sizes[pFlex->GetRenderMode()][subId]; i++)
         {
            uint16_t v = pFlex->GetSegFrame()[pos + i];
            for (int j = 0; j < 16; j++, v >>= 1)
               lum[i * 16 + j] = (v & 1) ? 1.f : 0.f;
         }
         return { pFlex->GetFrameId(), lum };
      }
   }
   return { 0, nullptr };
}


static void AddSegSrc(GetSegSrcMsg& msg, uint32_t flexId, int displayIndex, int nDisplays, unsigned int nElements, SegElementType type)
{
   if (msg.count < msg.maxEntryCount)
   {
      msg.entries[msg.count] = {};
      msg.entries[msg.count].id = { endpointId, flexId << 8 | displayIndex };
      msg.entries[msg.count].groupId = { endpointId, flexId };
      msg.entries[msg.count].hardware = CTLPI_SEG_HARDWARE_UNKNOWN;
      msg.entries[msg.count].nElements = nElements;
      for (unsigned int j = 0; j < nElements; j++)
         msg.entries[msg.count].elementType[j] = type;
      msg.entries[msg.count].GetState = GetState;
   }
   msg.count++;
}

static void onGetSegSrc(const unsigned int eventId, void* userData, void* msgData)
{
   GetSegSrcMsg& msg = *static_cast<GetSegSrcMsg*>(msgData);
   for (const FlexDMD* pFlex : flexDmds)
   {
      if (pFlex->GetShow())
      {
         switch (pFlex->GetRenderMode())
         {
            case RenderMode_SEG_2x16Alpha:
               AddSegSrc(msg, pFlex->GetId(), 0, 2, 16, CTLPI_SEG_LAYOUT_14D);
               AddSegSrc(msg, pFlex->GetId(), 1, 2, 16, CTLPI_SEG_LAYOUT_14D);
               break;
            case RenderMode_SEG_2x20Alpha:
               AddSegSrc(msg, pFlex->GetId(), 0, 2, 20, CTLPI_SEG_LAYOUT_14D);
               AddSegSrc(msg, pFlex->GetId(), 1, 2, 20, CTLPI_SEG_LAYOUT_14D);
               break;
            case RenderMode_SEG_2x7Alpha_2x7Num:
               AddSegSrc(msg, pFlex->GetId(), 0, 4, 7, CTLPI_SEG_LAYOUT_14D);
               AddSegSrc(msg, pFlex->GetId(), 1, 4, 7, CTLPI_SEG_LAYOUT_14D);
               AddSegSrc(msg, pFlex->GetId(), 2, 4, 7, CTLPI_SEG_LAYOUT_7C);
               AddSegSrc(msg, pFlex->GetId(), 3, 4, 7, CTLPI_SEG_LAYOUT_7C);
               break;
            case RenderMode_SEG_2x7Alpha_2x7Num_4x1Num:
               AddSegSrc(msg, pFlex->GetId(), 0, 6, 7, CTLPI_SEG_LAYOUT_14D);
               AddSegSrc(msg, pFlex->GetId(), 1, 6, 7, CTLPI_SEG_LAYOUT_14D);
               AddSegSrc(msg, pFlex->GetId(), 2, 6, 7, CTLPI_SEG_LAYOUT_7C);
               AddSegSrc(msg, pFlex->GetId(), 3, 6, 7, CTLPI_SEG_LAYOUT_7C);
               AddSegSrc(msg, pFlex->GetId(), 4, 6, 2, CTLPI_SEG_LAYOUT_7C);
               AddSegSrc(msg, pFlex->GetId(), 5, 6, 2, CTLPI_SEG_LAYOUT_7C);
               break;
            case RenderMode_SEG_2x7Num_2x7Num_4x1Num:
               AddSegSrc(msg, pFlex->GetId(), 0, 6, 7, CTLPI_SEG_LAYOUT_7C);
               AddSegSrc(msg, pFlex->GetId(), 1, 6, 7, CTLPI_SEG_LAYOUT_7C);
               AddSegSrc(msg, pFlex->GetId(), 2, 6, 7, CTLPI_SEG_LAYOUT_7C);
               AddSegSrc(msg, pFlex->GetId(), 3, 6, 7, CTLPI_SEG_LAYOUT_7C);
               AddSegSrc(msg, pFlex->GetId(), 4, 6, 2, CTLPI_SEG_LAYOUT_7C);
               AddSegSrc(msg, pFlex->GetId(), 5, 6, 2, CTLPI_SEG_LAYOUT_7C);
               break;
            case RenderMode_SEG_2x7Num_2x7Num_10x1Num:
               AddSegSrc(msg, pFlex->GetId(), 0, 9, 7, CTLPI_SEG_LAYOUT_7C);
               AddSegSrc(msg, pFlex->GetId(), 1, 9, 7, CTLPI_SEG_LAYOUT_7C);
               AddSegSrc(msg, pFlex->GetId(), 2, 9, 7, CTLPI_SEG_LAYOUT_7C);
               AddSegSrc(msg, pFlex->GetId(), 3, 9, 7, CTLPI_SEG_LAYOUT_7C);
               AddSegSrc(msg, pFlex->GetId(), 4, 9, 2, CTLPI_SEG_LAYOUT_7C);
               AddSegSrc(msg, pFlex->GetId(), 5, 9, 2, CTLPI_SEG_LAYOUT_7C);
               AddSegSrc(msg, pFlex->GetId(), 6, 9, 2, CTLPI_SEG_LAYOUT_7C);
               AddSegSrc(msg, pFlex->GetId(), 7, 9, 2, CTLPI_SEG_LAYOUT_7C);
               AddSegSrc(msg, pFlex->GetId(), 8, 9, 2, CTLPI_SEG_LAYOUT_7C);
               break;
            case RenderMode_SEG_2x7Num_2x7Num_4x1Num_gen7:
               AddSegSrc(msg, pFlex->GetId(), 0, 6, 7, CTLPI_SEG_LAYOUT_7C);
               AddSegSrc(msg, pFlex->GetId(), 1, 6, 7, CTLPI_SEG_LAYOUT_7C);
               AddSegSrc(msg, pFlex->GetId(), 2, 6, 7, CTLPI_SEG_LAYOUT_7C);
               AddSegSrc(msg, pFlex->GetId(), 3, 6, 7, CTLPI_SEG_LAYOUT_7C);
               AddSegSrc(msg, pFlex->GetId(), 4, 6, 2, CTLPI_SEG_LAYOUT_7C);
               AddSegSrc(msg, pFlex->GetId(), 5, 6, 2, CTLPI_SEG_LAYOUT_7C);
               break;
            case RenderMode_SEG_2x7Num10_2x7Num10_4x1Num:
               AddSegSrc(msg, pFlex->GetId(), 0, 6, 7, CTLPI_SEG_LAYOUT_9C);
               AddSegSrc(msg, pFlex->GetId(), 1, 6, 7, CTLPI_SEG_LAYOUT_9C);
               AddSegSrc(msg, pFlex->GetId(), 2, 6, 7, CTLPI_SEG_LAYOUT_9C);
               AddSegSrc(msg, pFlex->GetId(), 3, 6, 7, CTLPI_SEG_LAYOUT_9C);
               AddSegSrc(msg, pFlex->GetId(), 4, 6, 2, CTLPI_SEG_LAYOUT_7C);
               AddSegSrc(msg, pFlex->GetId(), 5, 6, 2, CTLPI_SEG_LAYOUT_7C);
               break;
            case RenderMode_SEG_2x6Num_2x6Num_4x1Num:
               AddSegSrc(msg, pFlex->GetId(), 0, 6, 6, CTLPI_SEG_LAYOUT_7C);
               AddSegSrc(msg, pFlex->GetId(), 1, 6, 6, CTLPI_SEG_LAYOUT_7C);
               AddSegSrc(msg, pFlex->GetId(), 2, 6, 6, CTLPI_SEG_LAYOUT_7C);
               AddSegSrc(msg, pFlex->GetId(), 3, 6, 6, CTLPI_SEG_LAYOUT_7C);
               AddSegSrc(msg, pFlex->GetId(), 4, 6, 2, CTLPI_SEG_LAYOUT_7C);
               AddSegSrc(msg, pFlex->GetId(), 5, 6, 2, CTLPI_SEG_LAYOUT_7C);
               break;
            case RenderMode_SEG_2x6Num10_2x6Num10_4x1Num:
               AddSegSrc(msg, pFlex->GetId(), 0, 6, 6, CTLPI_SEG_LAYOUT_9C);
               AddSegSrc(msg, pFlex->GetId(), 1, 6, 6, CTLPI_SEG_LAYOUT_9C);
               AddSegSrc(msg, pFlex->GetId(), 2, 6, 6, CTLPI_SEG_LAYOUT_9C);
               AddSegSrc(msg, pFlex->GetId(), 3, 6, 6, CTLPI_SEG_LAYOUT_9C);
               AddSegSrc(msg, pFlex->GetId(), 4, 6, 2, CTLPI_SEG_LAYOUT_7C);
               AddSegSrc(msg, pFlex->GetId(), 5, 6, 2, CTLPI_SEG_LAYOUT_7C);
               break;
            case RenderMode_SEG_4x7Num10:
               AddSegSrc(msg, pFlex->GetId(), 0, 4, 7, CTLPI_SEG_LAYOUT_9C);
               AddSegSrc(msg, pFlex->GetId(), 1, 4, 7, CTLPI_SEG_LAYOUT_9C);
               AddSegSrc(msg, pFlex->GetId(), 2, 4, 7, CTLPI_SEG_LAYOUT_9C);
               AddSegSrc(msg, pFlex->GetId(), 3, 4, 7, CTLPI_SEG_LAYOUT_9C);
               break;
            case RenderMode_SEG_6x4Num_4x1Num:
               AddSegSrc(msg, pFlex->GetId(), 0, 6, 4, CTLPI_SEG_LAYOUT_7C);
               AddSegSrc(msg, pFlex->GetId(), 1, 6, 4, CTLPI_SEG_LAYOUT_7C);
               AddSegSrc(msg, pFlex->GetId(), 2, 6, 4, CTLPI_SEG_LAYOUT_7C);
               AddSegSrc(msg, pFlex->GetId(), 3, 6, 4, CTLPI_SEG_LAYOUT_7C);
               AddSegSrc(msg, pFlex->GetId(), 4, 6, 4, CTLPI_SEG_LAYOUT_7C);
               AddSegSrc(msg, pFlex->GetId(), 5, 6, 4, CTLPI_SEG_LAYOUT_7C);
               AddSegSrc(msg, pFlex->GetId(), 4, 6, 2, CTLPI_SEG_LAYOUT_7C);
               AddSegSrc(msg, pFlex->GetId(), 5, 6, 2, CTLPI_SEG_LAYOUT_7C);
               break;
            case RenderMode_SEG_2x7Num_4x1Num_1x16Alpha:
               AddSegSrc(msg, pFlex->GetId(), 0, 5, 7, CTLPI_SEG_LAYOUT_7C);
               AddSegSrc(msg, pFlex->GetId(), 1, 5, 7, CTLPI_SEG_LAYOUT_7C);
               AddSegSrc(msg, pFlex->GetId(), 2, 5, 2, CTLPI_SEG_LAYOUT_7C);
               AddSegSrc(msg, pFlex->GetId(), 3, 5, 2, CTLPI_SEG_LAYOUT_7C);
               AddSegSrc(msg, pFlex->GetId(), 4, 5, 16, CTLPI_SEG_LAYOUT_14D);
               break;
            case RenderMode_SEG_1x16Alpha_1x16Num_1x7Num:
               AddSegSrc(msg, pFlex->GetId(), 0, 3, 16, CTLPI_SEG_LAYOUT_14D);
               AddSegSrc(msg, pFlex->GetId(), 1, 3, 16, CTLPI_SEG_LAYOUT_7C);
               AddSegSrc(msg, pFlex->GetId(), 2, 3, 7, CTLPI_SEG_LAYOUT_7C);
               break;
            default:
               break;
         }
      }
   }
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// DMD & Displays

static bool hasDMD = false;
static unsigned int getDmdSrcId, onDmdSrcChangeId;

static DisplayFrame GetRenderFrame(const CtlResId id)
{
   for (FlexDMD* pFlex : flexDmds)
   {
      if ((endpointId == id.endpointId) && (pFlex->GetId() == id.resId))
      {
         pFlex->Render();
         if (pFlex->GetRenderMode() == RenderMode_DMD_RGB)
            return { pFlex->GetFrameId(), pFlex->UpdateRGBFrame() };
         else if ((pFlex->GetRenderMode() == RenderMode_DMD_GRAY_2) || (pFlex->GetRenderMode() == RenderMode_DMD_GRAY_4))
            return { pFlex->GetFrameId(), pFlex->UpdateLumFP32Frame() };
         return { 0, nullptr };
      }
   }
   return { 0, nullptr };
}

static void onGetRenderDMDSrc(const unsigned int eventId, void* userData, void* msgData)
{
   GetDisplaySrcMsg& msg = *static_cast<GetDisplaySrcMsg*>(msgData);
   for (const FlexDMD* pFlex : flexDmds)
   {
      if (pFlex->GetShow() && ((pFlex->GetRenderMode() == RenderMode_DMD_GRAY_2) || (pFlex->GetRenderMode() == RenderMode_DMD_GRAY_4) || (pFlex->GetRenderMode() == RenderMode_DMD_RGB)))
      {
         if (msg.count < msg.maxEntryCount)
         {
            msg.entries[msg.count] = {};
            msg.entries[msg.count].id = { endpointId, pFlex->GetId() };
            msg.entries[msg.count].groupId = { endpointId, pFlex->GetId() };
            msg.entries[msg.count].width = pFlex->GetWidth();
            msg.entries[msg.count].height = pFlex->GetHeight();
            msg.entries[msg.count].frameFormat = (pFlex->GetRenderMode() == RenderMode_DMD_RGB) ? CTLPI_DISPLAY_FORMAT_SRGB888 : CTLPI_DISPLAY_FORMAT_LUM32F;
            msg.entries[msg.count].GetRenderFrame = &GetRenderFrame;
            // TODO we should also provide identify frame to allow colorization/upscaling/pup events/...
         }
         msg.count++;
      }
   }
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Main plugin

PSC_ERROR_IMPLEMENT(scriptApi); // Implement script error

LPI_IMPLEMENT_CPP // Implement shared log support

static void OnShowChanged(FlexDMD* pFlexI)
{
   bool hadDMD = hasDMD;
   bool hadAlpha = hasAlpha;
   hasDMD = false;
   hasAlpha = false;
   for (const FlexDMD* pFlex : flexDmds)
   {
      if (pFlex->GetShow())
      {
         if ((pFlex->GetRenderMode() == RenderMode_DMD_GRAY_2) || (pFlex->GetRenderMode() == RenderMode_DMD_GRAY_4) || (pFlex->GetRenderMode() == RenderMode_DMD_RGB))
            hasDMD = true;
         else
            hasAlpha = true;
      }
   }
   if (hasDMD != hadDMD)
   {
      if (hasDMD)
         msgApi->SubscribeMsg(endpointId, getDmdSrcId, onGetRenderDMDSrc, nullptr);
      else
         msgApi->UnsubscribeMsg(getDmdSrcId, onGetRenderDMDSrc, nullptr);
   }
   msgApi->BroadcastMsg(endpointId, onDmdSrcChangeId, nullptr);
   if (hasAlpha != hadAlpha)
   {
      if (hasAlpha)
         msgApi->SubscribeMsg(endpointId, getSegSrcId, onGetSegSrc, nullptr);
      else
         msgApi->UnsubscribeMsg(getSegSrcId, onGetSegSrc, nullptr);
   }
   msgApi->BroadcastMsg(endpointId, onSegSrcChangedId, nullptr);
}

static void OnFlexDestroyed(FlexDMD* pFlex)
{
   bool showChanged = pFlex->GetShow();
   std::erase(flexDmds, pFlex);
   if (showChanged)
      OnShowChanged(pFlex);
}

}

using namespace Flex;

MSGPI_EXPORT void MSGPIAPI FlexDMDPluginLoad(const uint32_t sessionId, const MsgPluginAPI* api)
{
   msgApi = api;
   endpointId = sessionId;

   // Setup login
   LPISetup(endpointId, msgApi);

   // Contribute DMDs and segment displays when show is true
   onDmdSrcChangeId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_ON_SRC_CHG_MSG);
   getDmdSrcId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_GET_SRC_MSG);
   onSegSrcChangedId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_SEG_ON_SRC_CHG_MSG);
   getSegSrcId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_SEG_GET_SRC_MSG);

   unsigned int getVpxApiId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API);
   msgApi->BroadcastMsg(endpointId, getVpxApiId, &vpxApi);
   msgApi->ReleaseMsgID(getVpxApiId);

   // Contribute our API to the script engine
   const unsigned int getScriptApiId = msgApi->GetMsgID(SCRIPTPI_NAMESPACE, SCRIPTPI_MSG_GET_API);
   msgApi->BroadcastMsg(endpointId, getScriptApiId, &scriptApi);
   msgApi->ReleaseMsgID(getScriptApiId);

   auto regLambda = [&](ScriptClassDef* scd) { scriptApi->RegisterScriptClass(scd); };
   auto aliasLambda = [&](const char* name, const char* aliasedType) { scriptApi->RegisterScriptTypeAlias(name, aliasedType); };
   auto arrayLambda = [&](ScriptArrayDef* sad) { scriptApi->RegisterScriptArrayType(sad); };

   RegisterFlexDMD_Alignment(aliasLambda);
   RegisterFlexDMD_Interpolation(aliasLambda);

   RegisterFlexDMD_Actor(regLambda);
   RegisterFlexDMD_AnimatedActor(regLambda);
   RegisterFlexDMD_Label(regLambda);
   RegisterFlexDMD_Frame(regLambda);
   RegisterFlexDMD_Image(regLambda);
   RegisterFlexDMD_Group(regLambda);

   RegisterFlexDMD_Bitmap(regLambda);
   RegisterFlexDMD_FontDef(regLambda);
   RegisterFlexDMD_Font(regLambda);

   RegisterFlexDMD_Action(regLambda);
   RegisterFlexDMD_AddChildAction(regLambda);
   RegisterFlexDMD_BlinkAction(regLambda);
   RegisterFlexDMD_DelayedAction(regLambda);
   RegisterFlexDMD_MoveToAction(regLambda);
   RegisterFlexDMD_ParallelAction(regLambda);
   RegisterFlexDMD_RemoveFromParentAction(regLambda);
   RegisterFlexDMD_RepeatAction(regLambda);
   RegisterFlexDMD_SeekAction(regLambda);
   RegisterFlexDMD_SequenceAction(regLambda);
   RegisterFlexDMD_ShowAction(regLambda);
   RegisterFlexDMD_WaitAction(regLambda);
   RegisterFlexDMD_ActionFactory(regLambda);

   RegisterFlexDMD_RenderMode(aliasLambda);
   RegisterFlexDMD_Scaling(aliasLambda);
   RegisterFlexDMD_ByteArray(arrayLambda);
   RegisterFlexDMD_ShortArray(arrayLambda);
   RegisterFlexDMD_IntArray(arrayLambda);
   RegisterFlexDMD_UltraDMD(regLambda);
   RegisterFlexDMD_FlexDMD(regLambda);

   scriptApi->SubmitTypeLibrary(endpointId);

   nextDmdId = 0;
   FlexDMD_FlexDMD_SCD->CreateObject = []()
   {
      FlexDMD* pFlex = new FlexDMD(vpxApi);
      pFlex->SetId(nextDmdId);
      pFlex->SetOnDMDChangedHandler(OnShowChanged);
      pFlex->SetOnDestroyHandler(OnFlexDestroyed);
      nextDmdId++;
      flexDmds.push_back(pFlex);
      return static_cast<void*>(pFlex);
   };
   FlexDMD_UltraDMD_SCD->CreateObject = []()
   {
      FlexDMD* pFlex = (FlexDMD*)FlexDMD_FlexDMD_SCD->CreateObject();
      UltraDMD* pUDMD = new UltraDMD(pFlex);
      pFlex->Release();
      return static_cast<void*>(pUDMD);
   };

   scriptApi->SetCOMObjectOverride("FlexDMD.FlexDMD", FlexDMD_FlexDMD_SCD);
   scriptApi->SetCOMObjectOverride("UltraDMD.DMDObject", FlexDMD_UltraDMD_SCD);
}

MSGPI_EXPORT void MSGPIAPI FlexDMDPluginUnload()
{
   // All FlexDMD must be destroyed before unloading the plugin
   assert(!hasDMD);
   assert(!hasAlpha);

   auto regLambda = [&](ScriptClassDef* scd) { scriptApi->UnregisterScriptClass(scd); };
   auto aliasLambda = [&](const char* name) { scriptApi->UnregisterScriptTypeAlias(name); };
   auto arrayLambda = [&](ScriptArrayDef* sad) { scriptApi->UnregisterScriptArrayType(sad); };

   scriptApi->SetCOMObjectOverride("UltraDMD.DMDObject", nullptr);
   scriptApi->SetCOMObjectOverride("FlexDMD.FlexDMD", nullptr);

   UnregisterFlexDMD_Alignment(aliasLambda);
   UnregisterFlexDMD_Interpolation(aliasLambda);

   UnregisterFlexDMD_Actor(regLambda);
   UnregisterFlexDMD_AnimatedActor(regLambda);
   UnregisterFlexDMD_Label(regLambda);
   UnregisterFlexDMD_Frame(regLambda);
   UnregisterFlexDMD_Image(regLambda);
   UnregisterFlexDMD_Group(regLambda);

   UnregisterFlexDMD_Bitmap(regLambda);
   UnregisterFlexDMD_FontDef(regLambda);
   UnregisterFlexDMD_Font(regLambda);

   UnregisterFlexDMD_Action(regLambda);
   UnregisterFlexDMD_AddChildAction(regLambda);
   UnregisterFlexDMD_BlinkAction(regLambda);
   UnregisterFlexDMD_DelayedAction(regLambda);
   UnregisterFlexDMD_MoveToAction(regLambda);
   UnregisterFlexDMD_ParallelAction(regLambda);
   UnregisterFlexDMD_RemoveFromParentAction(regLambda);
   UnregisterFlexDMD_RepeatAction(regLambda);
   UnregisterFlexDMD_SeekAction(regLambda);
   UnregisterFlexDMD_SequenceAction(regLambda);
   UnregisterFlexDMD_ShowAction(regLambda);
   UnregisterFlexDMD_WaitAction(regLambda);
   UnregisterFlexDMD_ActionFactory(regLambda);

   UnregisterFlexDMD_RenderMode(aliasLambda);
   UnregisterFlexDMD_Scaling(aliasLambda);
   UnregisterFlexDMD_ByteArray(arrayLambda);
   UnregisterFlexDMD_ShortArray(arrayLambda);
   UnregisterFlexDMD_IntArray(arrayLambda);
   UnregisterFlexDMD_UltraDMD(regLambda);
   UnregisterFlexDMD_FlexDMD(regLambda);

   msgApi->ReleaseMsgID(onSegSrcChangedId);
   msgApi->ReleaseMsgID(getSegSrcId);
   msgApi->ReleaseMsgID(onDmdSrcChangeId);
   msgApi->ReleaseMsgID(getDmdSrcId);

   scriptApi = nullptr;
   msgApi = nullptr;
   vpxApi = nullptr;
}
