// license:GPLv3+

#include "MsgPlugin.h"
#include "ControllerPlugin.h"

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

PSC_CLASS_START(FontDef)
PSC_CLASS_END(FontDef)

PSC_CLASS_START(VideoDef)
PSC_CLASS_END(VideoDef)

PSC_CLASS_START(ImageSequenceDef)
PSC_CLASS_END(ImageSequenceDef)

PSC_CLASS_START(AssetSrc)
PSC_CLASS_END(AssetSrc)

#define PSC_VAR_Font(variant) PSC_VAR_object(Font, variant)
#define PSC_VAR_SET_Font(variant, value) PSC_VAR_SET_object(Font, variant, value)
PSC_CLASS_START(Font)
PSC_CLASS_END(Font)

///////////////////////////////////////////////////////////////////////////////////////////////////
// All actors

#define PSC_VAR_Alignment(variant) PSC_VAR_enum(Alignment, variant)
#define PSC_VAR_SET_Alignment(variant, value) PSC_VAR_SET_enum(Alignment, variant, value)
PSC_CLASS_ALIAS(Alignment, int32)

#define PSC_VAR_Scaling(variant) PSC_VAR_enum(Scaling, variant)
#define PSC_VAR_SET_Scaling(variant, value) PSC_VAR_SET_enum(Scaling, variant, value)
PSC_CLASS_ALIAS(Scaling, int32)

#define PSC_VAR_Actor(variant) PSC_VAR_object(Actor, variant)
#define PSC_VAR_SET_ActionFactory(variant, value) PSC_VAR_SET_object(ActionFactory, variant, value)
#define PSC_VAR_Action(variant) PSC_VAR_object(Action, variant)
PSC_CLASS_START(Actor)
   PSC_PROP_RW(Actor, string, Name)
   PSC_PROP_RW(Actor, float, X)
   PSC_PROP_RW(Actor, float, Y)
   PSC_PROP_RW(Actor, float, Width)
   PSC_PROP_RW(Actor, float, Height)
   PSC_PROP_RW(Actor, bool, Visible)
   PSC_PROP_RW(Actor, bool, FillParent)
   PSC_PROP_RW(Actor, bool, ClearBackground)
   PSC_FUNCTION4(Actor, void, SetBounds, float, float, float, float)
   PSC_FUNCTION2(Actor, void, SetPosition, float, float)
   PSC_FUNCTION3(Actor, void, SetAlignedPosition, float, float, Alignment)
   PSC_FUNCTION2(Actor, void, SetSize, float, float)
   PSC_PROP_RW(Actor, float, PrefWidth)
   PSC_PROP_RW(Actor, float, PrefHeight)
   PSC_FUNCTION0(Actor, void, Pack)
   PSC_FUNCTION0(Actor, void, Remove)
   PSC_PROP_R(Actor, ActionFactory, ActionFactory)
   PSC_FUNCTION1(Actor, void, AddAction, Action)
   PSC_FUNCTION0(Actor, void, ClearActions)
PSC_CLASS_END(Actor)

#define PSC_VAR_Label(variant) PSC_VAR_object(Label, variant)
#define PSC_VAR_SET_Label(variant, value) PSC_VAR_SET_object(Label, variant, value)
PSC_CLASS_START(Label)
   PSC_INHERIT_CLASS(Label, Actor)
   PSC_PROP_RW(Label, bool, AutoPack)
   PSC_PROP_RW(Label, Alignment, Alignment)
   PSC_PROP_RW(Label, Font, Font)
   PSC_PROP_RW(Label, string, Text)
PSC_CLASS_END(Label)

#define PSC_VAR_Frame(variant) PSC_VAR_object(Frame, variant)
#define PSC_VAR_SET_Frame(variant, value) PSC_VAR_SET_object(Frame, variant, value)
PSC_CLASS_START(Frame)
   PSC_INHERIT_CLASS(Frame, Actor)
   PSC_PROP_RW(Frame, int, Thickness)
   PSC_PROP_RW(Frame, int, BorderColor)
   PSC_PROP_RW(Frame, bool, Fill)
   PSC_PROP_RW(Frame, int, FillColor)
PSC_CLASS_END(Frame)

#define PSC_VAR_Bitmap(variant) PSC_VAR_object(Bitmap, variant)
#define PSC_VAR_SET_Bitmap(variant, value) PSC_VAR_SET_object(Bitmap, variant, value)
PSC_CLASS_START(Bitmap)
PSC_CLASS_END(Bitmap)

#define PSC_VAR_Image(variant) PSC_VAR_object(Image, variant)
#define PSC_VAR_SET_Image(variant, value) PSC_VAR_SET_object(Image, variant, value)
PSC_CLASS_START(Image)
   PSC_INHERIT_CLASS(Image, Actor)
   PSC_PROP_RW(Image, Bitmap, Bitmap)
   PSC_PROP_RW(Image, Scaling, Scaling)
   PSC_PROP_RW(Image, Alignment, Alignment)
PSC_CLASS_END(Image)

#define PSC_VAR_SET_AnimatedActor(variant, value) PSC_VAR_SET_object(AnimatedActor, variant, value)
PSC_CLASS_START(AnimatedActor)
   PSC_INHERIT_CLASS(AnimatedActor, Actor)
   PSC_PROP_R(AnimatedActor, float, Length)
   PSC_PROP_RW(AnimatedActor, bool, Loop)
   PSC_PROP_RW(AnimatedActor, bool, Paused)
   PSC_PROP_RW(AnimatedActor, float, PlaySpeed)
   PSC_PROP_RW(AnimatedActor, Scaling, Scaling)
   PSC_PROP_RW(AnimatedActor, Alignment, Alignment)
   PSC_FUNCTION1(AnimatedActor, void, Seek, float)
PSC_CLASS_END(AnimatedActor)

#define PSC_VAR_Group(variant) PSC_VAR_object(Group, variant)
#define PSC_VAR_SET_Group(variant, value) PSC_VAR_SET_object(Group, variant, value)
PSC_CLASS_START(Group)
   PSC_INHERIT_CLASS(Group, Actor)
   PSC_PROP_RW(Group, bool, Clip)
   PSC_PROP_R(Group, int, ChildCount)
   PSC_FUNCTION1(Group, bool, HasChild, string)
   PSC_FUNCTION1(Group, Group, GetGroup, string)
   PSC_FUNCTION1(Group, Frame, GetFrame, string)
   PSC_FUNCTION1(Group, Label, GetLabel, string)
   PSC_FUNCTION1(Group, AnimatedActor, GetVideo, string)
   PSC_FUNCTION1(Group, Image, GetImage, string)
   PSC_FUNCTION0(Group, void, RemoveAll)
   PSC_FUNCTION1(Group, void, AddActor, Actor)
   PSC_FUNCTION1(Group, void, RemoveActor, Actor)
PSC_CLASS_END(Group)


///////////////////////////////////////////////////////////////////////////////////////////////////
// All actions and ActionFactory

PSC_CLASS_START(Action)
   PSC_FUNCTION1(Action, bool, Update, float)
PSC_CLASS_END(Action)

PSC_CLASS_START(AddChildAction)
   PSC_INHERIT_CLASS(AddChildAction, Action)
PSC_CLASS_END(AddChildAction)

PSC_CLASS_START(BlinkAction)
   PSC_INHERIT_CLASS(BlinkAction, Action)
PSC_CLASS_END(BlinkAction)

PSC_CLASS_START(DelayedAction)
   PSC_INHERIT_CLASS(DelayedAction, Action)
PSC_CLASS_END(DelayedAction)

#define PSC_VAR_Interpolation(variant) PSC_VAR_enum(Interpolation, variant)
#define PSC_VAR_SET_Interpolation(variant, value) PSC_VAR_SET_enum(Interpolation, variant, value)
PSC_CLASS_ALIAS(Interpolation, int32)

PSC_CLASS_START(MoveToAction)
   PSC_INHERIT_CLASS(MoveToAction, Action)
   PSC_PROP_RW(MoveToAction, Interpolation, Ease)
PSC_CLASS_END(MoveToAction)

#define PSC_VAR_SET_ParallelAction(variant, value) PSC_VAR_SET_object(ParallelAction, variant, value)
PSC_CLASS_START(ParallelAction)
   PSC_INHERIT_CLASS(ParallelAction, Action)
   PSC_FUNCTION1(ParallelAction, void, Add, Action) // Returning this is not yet implemented
PSC_CLASS_END(ParallelAction)

PSC_CLASS_START(RemoveFromParentAction)
   PSC_INHERIT_CLASS(RemoveFromParentAction, Action)
PSC_CLASS_END(RemoveFromParentAction)

PSC_CLASS_START(RepeatAction)
   PSC_INHERIT_CLASS(RepeatAction, Action)
PSC_CLASS_END(RepeatAction)

PSC_CLASS_START(SeekAction)
   PSC_INHERIT_CLASS(SeekAction, Action)
PSC_CLASS_END(SeekAction)

#define PSC_VAR_SET_SequenceAction(variant, value) PSC_VAR_SET_object(SequenceAction, variant, value)
PSC_CLASS_START(SequenceAction)
   PSC_INHERIT_CLASS(SequenceAction, Action)
   PSC_FUNCTION1(SequenceAction, void, Add, Action) // Returning this is not yet implemented
PSC_CLASS_END(SequenceAction)

PSC_CLASS_START(ShowAction)
   PSC_INHERIT_CLASS(ShowAction, Action)
PSC_CLASS_END(ShowAction)

PSC_CLASS_START(WaitAction)
   PSC_INHERIT_CLASS(WaitAction, Action)
PSC_CLASS_END(WaitAction)

#define PSC_VAR_SET_WaitAction(variant, value) PSC_VAR_SET_object(WaitAction, variant, value)
#define PSC_VAR_SET_DelayedAction(variant, value) PSC_VAR_SET_object(DelayedAction, variant, value)
#define PSC_VAR_SET_RepeatAction(variant, value) PSC_VAR_SET_object(RepeatAction, variant, value)
#define PSC_VAR_SET_BlinkAction(variant, value) PSC_VAR_SET_object(BlinkAction, variant, value)
#define PSC_VAR_SET_ShowAction(variant, value) PSC_VAR_SET_object(ShowAction, variant, value)
#define PSC_VAR_SET_AddChildAction(variant, value) PSC_VAR_SET_object(AddChildAction, variant, value)
#define PSC_VAR_SET_RemoveFromParentAction(variant, value) PSC_VAR_SET_object(RemoveFromParentAction, variant, value)
#define PSC_VAR_SET_SeekAction(variant, value) PSC_VAR_SET_object(SeekAction, variant, value)
#define PSC_VAR_SET_MoveToAction(variant, value) PSC_VAR_SET_object(MoveToAction, variant, value)
PSC_CLASS_START(ActionFactory)
   PSC_FUNCTION1(ActionFactory, WaitAction, Wait, float)
   PSC_FUNCTION2(ActionFactory, DelayedAction, Delayed, float, Action)
   PSC_FUNCTION0(ActionFactory, ParallelAction, Parallel)
   PSC_FUNCTION0(ActionFactory, SequenceAction, Sequence)
   PSC_FUNCTION2(ActionFactory, RepeatAction, Repeat, Action, int)
   PSC_FUNCTION3(ActionFactory, BlinkAction, Blink, float, float, int)
   PSC_FUNCTION1(ActionFactory, ShowAction, Show, bool)
   PSC_FUNCTION1(ActionFactory, AddChildAction, AddTo, Group)
   PSC_FUNCTION0(ActionFactory, RemoveFromParentAction, RemoveFromParent)
   PSC_FUNCTION1(ActionFactory, AddChildAction, AddChild, Actor)
   PSC_FUNCTION1(ActionFactory, AddChildAction, RemoveChild, Actor)
   PSC_FUNCTION1(ActionFactory, SeekAction, Seek, float)
   PSC_FUNCTION3(ActionFactory, MoveToAction, MoveTo, float, float, float)
PSC_CLASS_END(ActionFactory)


///////////////////////////////////////////////////////////////////////////////////////////////////
// Main classes

#define PSC_VAR_SET_UltraDMD(variant, value) PSC_VAR_SET_object(UltraDMD, variant, value)
PSC_CLASS_START(UltraDMD)
   PSC_FUNCTION0(UltraDMD, void, LoadSetup)
   PSC_FUNCTION0(UltraDMD, void, Init)
   PSC_FUNCTION0(UltraDMD, void, Uninit)
   PSC_FUNCTION0(UltraDMD, int, GetMajorVersion)
   PSC_FUNCTION0(UltraDMD, int, GetMinorVersion)
   PSC_FUNCTION0(UltraDMD, int, GetBuildNumber)
   PSC_FUNCTION1(UltraDMD, bool, SetVisibleVirtualDMD, bool)
   PSC_FUNCTION1(UltraDMD, bool, SetFlipY, bool)
   PSC_FUNCTION0(UltraDMD, bool, IsRendering)
   PSC_FUNCTION0(UltraDMD, void, CancelRendering)
   PSC_FUNCTION1(UltraDMD, void, CancelRenderingWithId, string)
   PSC_FUNCTION0(UltraDMD, void, Clear)
   PSC_FUNCTION1(UltraDMD, void, SetProjectFolder, string)
   PSC_FUNCTION1(UltraDMD, void, SetVideoStretchMode, int)
   PSC_FUNCTION3(UltraDMD, void, SetScoreboardBackgroundImage, string, int, int)
   PSC_FUNCTION3(UltraDMD, int, CreateAnimationFromImages, int, bool, string)
   PSC_FUNCTION3(UltraDMD, int, RegisterVideo, int, bool, string)
   PSC_FUNCTION0(UltraDMD, void, DisplayVersionInfo)
   PSC_FUNCTION8(UltraDMD, void, DisplayScoreboard, int, int, int, int, int, int, string, string)
   PSC_FUNCTION8(UltraDMD, void, DisplayScoreboard00, int, int, int, int, int, int, string, string)
   PSC_FUNCTION8(UltraDMD, void, DisplayScene00, string, string, int, string, int, int, int, int)
   PSC_FUNCTION10(UltraDMD, void, DisplayScene00Ex, string, string, int, int, string, int, int, int, int, int)
   PSC_FUNCTION12(UltraDMD, void, DisplayScene00ExWithId, string, bool, string, string, int, int, string, int, int, int, int, int)
   PSC_FUNCTION3(UltraDMD, void, ModifyScene00, string, string, string)
   PSC_FUNCTION4(UltraDMD, void, ModifyScene00Ex, string, string, string, int)
   PSC_FUNCTION8(UltraDMD, void, DisplayScene01, string, string, string, int, int, int, int, int)
   PSC_FUNCTION3(UltraDMD, void, DisplayText, string, int, int)
   PSC_FUNCTION6(UltraDMD, void, ScrollingCredits, string, string, int, int, int, int)
PSC_CLASS_END(UltraDMD)

#define PSC_VAR_RenderMode(variant) PSC_VAR_enum(RenderMode, variant)
#define PSC_VAR_SET_RenderMode(variant, value) PSC_VAR_SET_enum(RenderMode, variant, value)
PSC_CLASS_ALIAS(RenderMode, int32)

PSC_ARRAY1(ByteArray, uint8, 0)
#define PSC_VAR_SET_ByteArray(variant, value) PSC_VAR_SET_array1(ByteArray, variant, value)

PSC_ARRAY1(ShortArray, int16, 0)
#define PSC_VAR_SET_ShortArray(variant, value) PSC_VAR_SET_array1(ShortArray, variant, value)
#define PSC_VAR_ShortArray(variant) PSC_VAR_array1(uint16_t, variant)

PSC_ARRAY1(IntArray, int32, 0)
#define PSC_VAR_SET_IntArray(variant, value) PSC_VAR_SET_array1(IntArray, variant, value)

PSC_CLASS_START(FlexDMD)
   PSC_PROP_R(FlexDMD, int, Version)
   PSC_PROP_RW(FlexDMD, int, RuntimeVersion)
   PSC_PROP_RW(FlexDMD, bool, Run)
   PSC_PROP_RW(FlexDMD, bool, Show)
   PSC_PROP_RW(FlexDMD, string, GameName)
   PSC_PROP_RW(FlexDMD, int, Width)
   PSC_PROP_RW(FlexDMD, int, Height)
   PSC_PROP_RW(FlexDMD, uint, Color)
   PSC_PROP_RW(FlexDMD, RenderMode, RenderMode)
   PSC_PROP_RW(FlexDMD, string, ProjectFolder)
   PSC_PROP_RW(FlexDMD, string, TableFile)
   PSC_PROP_RW(FlexDMD, bool, Clear)
   PSC_PROP_R(FlexDMD, IntArray, DmdColoredPixels)
   PSC_PROP_R(FlexDMD, ByteArray, DmdPixels)
   PSC_PROP_W(FlexDMD, ShortArray, Segments)
   PSC_PROP_W(FlexDMD, ShortArray, Segments)
   PSC_PROP_R(FlexDMD, Group, Stage)
   PSC_FUNCTION0(FlexDMD, void, LockRenderThread)
   PSC_FUNCTION0(FlexDMD, void, UnlockRenderThread)
   PSC_FUNCTION1(FlexDMD, Group, NewGroup, string)
   PSC_FUNCTION1(FlexDMD, Frame, NewFrame, string)
   PSC_FUNCTION3(FlexDMD, Label, NewLabel, string, Font, string)
   PSC_FUNCTION2(FlexDMD, AnimatedActor, NewVideo, string, string)
   PSC_FUNCTION2(FlexDMD, Image, NewImage, string, string)
   PSC_FUNCTION4(FlexDMD, Font, NewFont, string, uint, uint, int)
   PSC_FUNCTION0(FlexDMD, UltraDMD, NewUltraDMD)
PSC_CLASS_END(FlexDMD)


///////////////////////////////////////////////////////////////////////////////////////////////////
// Plugin interface

static MsgPluginAPI* msgApi = nullptr;
static VPXPluginAPI* vpxApi = nullptr;
static ScriptablePluginAPI* scriptApi = nullptr;

static uint32_t endpointId, nextDmdId;

static std::vector<FlexDMD*> flexDmds;


///////////////////////////////////////////////////////////////////////////////////////////////////
// Alphanumeric segment displays

static bool hasAlpha = false;
static unsigned int onSegSrcChangedId, getSegSrcId;
static float segLuminances[16 * 128] = { 0 };

static SegDisplayFrame GetState(const CtlResId id)
{
   // FIXME implement new output ids
   if (id.endpointId != endpointId)
      return { 0, nullptr };
   int flexId = id.resId >> 8;
   int subId = id.resId & 0x0FF;
   static int sizes[17][14] = {
      { 0 }, // RenderMode_DMD_GRAY_2
      { 0 }, // RenderMode_DMD_GRAY_4
      { 0 }, // RenderMode_DMD_RGB
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
         for (int i = 0; i < subId; i++)
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
      msg.entries[msg.count] = { 0 };
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
            return { pFlex->GetFrameId(), pFlex->UpdateLum8Frame() };
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
            msg.entries[msg.count] = { 0 };
            msg.entries[msg.count].id = { endpointId, pFlex->GetId() };
            msg.entries[msg.count].groupId = { endpointId, pFlex->GetId() };
            msg.entries[msg.count].width = pFlex->GetWidth();
            msg.entries[msg.count].height = pFlex->GetHeight();
            msg.entries[msg.count].frameFormat = (pFlex->GetRenderMode() == RenderMode_DMD_RGB) ? CTLPI_DISPLAY_FORMAT_SRGB888 : CTLPI_DISPLAY_FORMAT_LUM8;
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

LPI_IMPLEMENT // Implement shared login support

static void OnShowChanged(FlexDMD* pFlex)
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
         msgApi->UnsubscribeMsg(getDmdSrcId, onGetRenderDMDSrc);
   }
   msgApi->BroadcastMsg(endpointId, onDmdSrcChangeId, nullptr);
   if (hasAlpha != hadAlpha)
   {
      if (hasAlpha)
         msgApi->SubscribeMsg(endpointId, getSegSrcId, onGetSegSrc, nullptr);
      else
         msgApi->UnsubscribeMsg(getSegSrcId, onGetSegSrc);
   }
   msgApi->BroadcastMsg(endpointId, onSegSrcChangedId, nullptr);
}

static void OnFlexDestroyed(FlexDMD* pFlex)
{
   bool showChanged = pFlex->GetShow();
   flexDmds.erase(std::remove(flexDmds.begin(), flexDmds.end(), pFlex), flexDmds.end());
   if (showChanged)
      OnShowChanged(pFlex);
}

}

using namespace Flex;

MSGPI_EXPORT void MSGPIAPI FlexDMDPluginLoad(const uint32_t sessionId, MsgPluginAPI* api)
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

   RegisterAlignmentSCD(aliasLambda);
   RegisterInterpolationSCD(aliasLambda);

   RegisterActorSCD(regLambda);
   RegisterAnimatedActorSCD(regLambda);
   RegisterLabelSCD(regLambda);
   RegisterFrameSCD(regLambda);
   RegisterImageSCD(regLambda);
   RegisterGroupSCD(regLambda);

   RegisterBitmapSCD(regLambda);
   RegisterFontDefSCD(regLambda);
   RegisterFontSCD(regLambda);

   RegisterActionSCD(regLambda);
   RegisterAddChildActionSCD(regLambda);
   RegisterBlinkActionSCD(regLambda);
   RegisterDelayedActionSCD(regLambda);
   RegisterMoveToActionSCD(regLambda);
   RegisterParallelActionSCD(regLambda);
   RegisterRemoveFromParentActionSCD(regLambda);
   RegisterRepeatActionSCD(regLambda);
   RegisterSeekActionSCD(regLambda);
   RegisterSequenceActionSCD(regLambda);
   RegisterShowActionSCD(regLambda);
   RegisterWaitActionSCD(regLambda);
   RegisterActionFactorySCD(regLambda);

   RegisterRenderModeSCD(aliasLambda);
   RegisterScalingSCD(aliasLambda);
   RegisterByteArraySCD(arrayLambda);
   RegisterShortArraySCD(arrayLambda);
   RegisterIntArraySCD(arrayLambda);
   RegisterUltraDMDSCD(regLambda);
   RegisterFlexDMDSCD(regLambda);

   scriptApi->SubmitTypeLibrary();

   nextDmdId = 0;
   FlexDMD_SCD->CreateObject = []() {
      FlexDMD* pFlex = new FlexDMD(vpxApi);
      pFlex->SetId(nextDmdId);
      pFlex->SetOnDMDChangedHandler(OnShowChanged);
      pFlex->SetOnDestroyHandler(OnFlexDestroyed);
      nextDmdId++;
      flexDmds.push_back(pFlex);
      return static_cast<void*>(pFlex);
   };

   scriptApi->SetCOMObjectOverride("FlexDMD.FlexDMD", FlexDMD_SCD);
   // FIXME scriptApi->SetCOMObjectOverride("UltraDMD.DMDObject", UltraDMD_SCD);
}

MSGPI_EXPORT void MSGPIAPI FlexDMDPluginUnload()
{
   // All FlexDMD must be destroyed before unloading the plugin
   assert(!hasDMD);
   assert(!hasAlpha);

   msgApi->ReleaseMsgID(onSegSrcChangedId);
   msgApi->ReleaseMsgID(getSegSrcId);
   msgApi->ReleaseMsgID(onDmdSrcChangeId);
   msgApi->ReleaseMsgID(getDmdSrcId);

   // TODO we should unregister the script API contribution
   // scriptApi->SetCOMObjectOverride("UltraDMD.DMDObject", nullptr);
   scriptApi->SetCOMObjectOverride("FlexDMD.FlexDMD", nullptr);
   scriptApi = nullptr;
   msgApi = nullptr;
   vpxApi = nullptr;
}
