

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.01.0626 */
/* at Tue Jan 19 03:14:07 2038
 */
/* Compiler settings for D:\a\vpvr\vpvr\vpinball.idl:
    Oicf, W1, Zp8, env=Win64 (32b run), target_arch=AMD64 8.01.0626 
    protocol : all , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */



/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 500
#endif

#ifndef __STANDALONE__
#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif /* __RPCNDR_H_VERSION__ */
#endif

#ifndef __vpinball_h__
#define __vpinball_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef DECLSPEC_XFGVIRT
#if _CONTROL_FLOW_GUARD_XFG
#define DECLSPEC_XFGVIRT(base, func) __declspec(xfg_virtual(base, func))
#else
#define DECLSPEC_XFGVIRT(base, func)
#endif
#endif

/* Forward Declarations */ 

#ifndef __ICollection_FWD_DEFINED__
#define __ICollection_FWD_DEFINED__
typedef interface ICollection ICollection;

#endif 	/* __ICollection_FWD_DEFINED__ */


#ifndef __ICollectionEvents_FWD_DEFINED__
#define __ICollectionEvents_FWD_DEFINED__
typedef interface ICollectionEvents ICollectionEvents;

#endif 	/* __ICollectionEvents_FWD_DEFINED__ */


#ifndef __ITable_FWD_DEFINED__
#define __ITable_FWD_DEFINED__
typedef interface ITable ITable;

#endif 	/* __ITable_FWD_DEFINED__ */


#ifndef __ITableGlobal_FWD_DEFINED__
#define __ITableGlobal_FWD_DEFINED__
typedef interface ITableGlobal ITableGlobal;

#endif 	/* __ITableGlobal_FWD_DEFINED__ */


#ifndef __ITableEvents_FWD_DEFINED__
#define __ITableEvents_FWD_DEFINED__
typedef interface ITableEvents ITableEvents;

#endif 	/* __ITableEvents_FWD_DEFINED__ */


#ifndef __IVPDebug_FWD_DEFINED__
#define __IVPDebug_FWD_DEFINED__
typedef interface IVPDebug IVPDebug;

#endif 	/* __IVPDebug_FWD_DEFINED__ */


#ifndef __IWall_FWD_DEFINED__
#define __IWall_FWD_DEFINED__
typedef interface IWall IWall;

#endif 	/* __IWall_FWD_DEFINED__ */


#ifndef __IWallEvents_FWD_DEFINED__
#define __IWallEvents_FWD_DEFINED__
typedef interface IWallEvents IWallEvents;

#endif 	/* __IWallEvents_FWD_DEFINED__ */


#ifndef __IControlPoint_FWD_DEFINED__
#define __IControlPoint_FWD_DEFINED__
typedef interface IControlPoint IControlPoint;

#endif 	/* __IControlPoint_FWD_DEFINED__ */


#ifndef __IFlipper_FWD_DEFINED__
#define __IFlipper_FWD_DEFINED__
typedef interface IFlipper IFlipper;

#endif 	/* __IFlipper_FWD_DEFINED__ */


#ifndef __IFlipperEvents_FWD_DEFINED__
#define __IFlipperEvents_FWD_DEFINED__
typedef interface IFlipperEvents IFlipperEvents;

#endif 	/* __IFlipperEvents_FWD_DEFINED__ */


#ifndef __ITimer_FWD_DEFINED__
#define __ITimer_FWD_DEFINED__
typedef interface ITimer ITimer;

#endif 	/* __ITimer_FWD_DEFINED__ */


#ifndef __ITimerEvents_FWD_DEFINED__
#define __ITimerEvents_FWD_DEFINED__
typedef interface ITimerEvents ITimerEvents;

#endif 	/* __ITimerEvents_FWD_DEFINED__ */


#ifndef __IPlunger_FWD_DEFINED__
#define __IPlunger_FWD_DEFINED__
typedef interface IPlunger IPlunger;

#endif 	/* __IPlunger_FWD_DEFINED__ */


#ifndef __IPlungerEvents_FWD_DEFINED__
#define __IPlungerEvents_FWD_DEFINED__
typedef interface IPlungerEvents IPlungerEvents;

#endif 	/* __IPlungerEvents_FWD_DEFINED__ */


#ifndef __ITextbox_FWD_DEFINED__
#define __ITextbox_FWD_DEFINED__
typedef interface ITextbox ITextbox;

#endif 	/* __ITextbox_FWD_DEFINED__ */


#ifndef __ITextboxEvents_FWD_DEFINED__
#define __ITextboxEvents_FWD_DEFINED__
typedef interface ITextboxEvents ITextboxEvents;

#endif 	/* __ITextboxEvents_FWD_DEFINED__ */


#ifndef __IBumper_FWD_DEFINED__
#define __IBumper_FWD_DEFINED__
typedef interface IBumper IBumper;

#endif 	/* __IBumper_FWD_DEFINED__ */


#ifndef __IBumperEvents_FWD_DEFINED__
#define __IBumperEvents_FWD_DEFINED__
typedef interface IBumperEvents IBumperEvents;

#endif 	/* __IBumperEvents_FWD_DEFINED__ */


#ifndef __ITrigger_FWD_DEFINED__
#define __ITrigger_FWD_DEFINED__
typedef interface ITrigger ITrigger;

#endif 	/* __ITrigger_FWD_DEFINED__ */


#ifndef __ITriggerEvents_FWD_DEFINED__
#define __ITriggerEvents_FWD_DEFINED__
typedef interface ITriggerEvents ITriggerEvents;

#endif 	/* __ITriggerEvents_FWD_DEFINED__ */


#ifndef __ILight_FWD_DEFINED__
#define __ILight_FWD_DEFINED__
typedef interface ILight ILight;

#endif 	/* __ILight_FWD_DEFINED__ */


#ifndef __ILightEvents_FWD_DEFINED__
#define __ILightEvents_FWD_DEFINED__
typedef interface ILightEvents ILightEvents;

#endif 	/* __ILightEvents_FWD_DEFINED__ */


#ifndef __IKicker_FWD_DEFINED__
#define __IKicker_FWD_DEFINED__
typedef interface IKicker IKicker;

#endif 	/* __IKicker_FWD_DEFINED__ */


#ifndef __IKickerEvents_FWD_DEFINED__
#define __IKickerEvents_FWD_DEFINED__
typedef interface IKickerEvents IKickerEvents;

#endif 	/* __IKickerEvents_FWD_DEFINED__ */


#ifndef __IDecal_FWD_DEFINED__
#define __IDecal_FWD_DEFINED__
typedef interface IDecal IDecal;

#endif 	/* __IDecal_FWD_DEFINED__ */


#ifndef __IPrimitive_FWD_DEFINED__
#define __IPrimitive_FWD_DEFINED__
typedef interface IPrimitive IPrimitive;

#endif 	/* __IPrimitive_FWD_DEFINED__ */


#ifndef __IPrimitiveEvents_FWD_DEFINED__
#define __IPrimitiveEvents_FWD_DEFINED__
typedef interface IPrimitiveEvents IPrimitiveEvents;

#endif 	/* __IPrimitiveEvents_FWD_DEFINED__ */


#ifndef __IHitTarget_FWD_DEFINED__
#define __IHitTarget_FWD_DEFINED__
typedef interface IHitTarget IHitTarget;

#endif 	/* __IHitTarget_FWD_DEFINED__ */


#ifndef __IHitTargetEvents_FWD_DEFINED__
#define __IHitTargetEvents_FWD_DEFINED__
typedef interface IHitTargetEvents IHitTargetEvents;

#endif 	/* __IHitTargetEvents_FWD_DEFINED__ */


#ifndef __IGate_FWD_DEFINED__
#define __IGate_FWD_DEFINED__
typedef interface IGate IGate;

#endif 	/* __IGate_FWD_DEFINED__ */


#ifndef __IGateEvents_FWD_DEFINED__
#define __IGateEvents_FWD_DEFINED__
typedef interface IGateEvents IGateEvents;

#endif 	/* __IGateEvents_FWD_DEFINED__ */


#ifndef __ISpinner_FWD_DEFINED__
#define __ISpinner_FWD_DEFINED__
typedef interface ISpinner ISpinner;

#endif 	/* __ISpinner_FWD_DEFINED__ */


#ifndef __ISpinnerEvents_FWD_DEFINED__
#define __ISpinnerEvents_FWD_DEFINED__
typedef interface ISpinnerEvents ISpinnerEvents;

#endif 	/* __ISpinnerEvents_FWD_DEFINED__ */


#ifndef __IRamp_FWD_DEFINED__
#define __IRamp_FWD_DEFINED__
typedef interface IRamp IRamp;

#endif 	/* __IRamp_FWD_DEFINED__ */


#ifndef __IFlasher_FWD_DEFINED__
#define __IFlasher_FWD_DEFINED__
typedef interface IFlasher IFlasher;

#endif 	/* __IFlasher_FWD_DEFINED__ */


#ifndef __IRubber_FWD_DEFINED__
#define __IRubber_FWD_DEFINED__
typedef interface IRubber IRubber;

#endif 	/* __IRubber_FWD_DEFINED__ */


#ifndef __IBall_FWD_DEFINED__
#define __IBall_FWD_DEFINED__
typedef interface IBall IBall;

#endif 	/* __IBall_FWD_DEFINED__ */


#ifndef __IRampEvents_FWD_DEFINED__
#define __IRampEvents_FWD_DEFINED__
typedef interface IRampEvents IRampEvents;

#endif 	/* __IRampEvents_FWD_DEFINED__ */


#ifndef __IFlasherEvents_FWD_DEFINED__
#define __IFlasherEvents_FWD_DEFINED__
typedef interface IFlasherEvents IFlasherEvents;

#endif 	/* __IFlasherEvents_FWD_DEFINED__ */


#ifndef __IRubberEvents_FWD_DEFINED__
#define __IRubberEvents_FWD_DEFINED__
typedef interface IRubberEvents IRubberEvents;

#endif 	/* __IRubberEvents_FWD_DEFINED__ */


#ifndef __IDispReel_FWD_DEFINED__
#define __IDispReel_FWD_DEFINED__
typedef interface IDispReel IDispReel;

#endif 	/* __IDispReel_FWD_DEFINED__ */


#ifndef __IDispReelEvents_FWD_DEFINED__
#define __IDispReelEvents_FWD_DEFINED__
typedef interface IDispReelEvents IDispReelEvents;

#endif 	/* __IDispReelEvents_FWD_DEFINED__ */


#ifndef __ILightSeq_FWD_DEFINED__
#define __ILightSeq_FWD_DEFINED__
typedef interface ILightSeq ILightSeq;

#endif 	/* __ILightSeq_FWD_DEFINED__ */


#ifndef __ILightSeqEvents_FWD_DEFINED__
#define __ILightSeqEvents_FWD_DEFINED__
typedef interface ILightSeqEvents ILightSeqEvents;

#endif 	/* __ILightSeqEvents_FWD_DEFINED__ */


#ifndef __Table_FWD_DEFINED__
#define __Table_FWD_DEFINED__

#ifdef __cplusplus
typedef class Table Table;
#else
typedef struct Table Table;
#endif /* __cplusplus */

#endif 	/* __Table_FWD_DEFINED__ */


#ifndef __Wall_FWD_DEFINED__
#define __Wall_FWD_DEFINED__

#ifdef __cplusplus
typedef class Wall Wall;
#else
typedef struct Wall Wall;
#endif /* __cplusplus */

#endif 	/* __Wall_FWD_DEFINED__ */


#ifndef __DragPoint_FWD_DEFINED__
#define __DragPoint_FWD_DEFINED__

#ifdef __cplusplus
typedef class DragPoint DragPoint;
#else
typedef struct DragPoint DragPoint;
#endif /* __cplusplus */

#endif 	/* __DragPoint_FWD_DEFINED__ */


#ifndef __Flipper_FWD_DEFINED__
#define __Flipper_FWD_DEFINED__

#ifdef __cplusplus
typedef class Flipper Flipper;
#else
typedef struct Flipper Flipper;
#endif /* __cplusplus */

#endif 	/* __Flipper_FWD_DEFINED__ */


#ifndef __Timer_FWD_DEFINED__
#define __Timer_FWD_DEFINED__

#ifdef __cplusplus
typedef class Timer Timer;
#else
typedef struct Timer Timer;
#endif /* __cplusplus */

#endif 	/* __Timer_FWD_DEFINED__ */


#ifndef __Plunger_FWD_DEFINED__
#define __Plunger_FWD_DEFINED__

#ifdef __cplusplus
typedef class Plunger Plunger;
#else
typedef struct Plunger Plunger;
#endif /* __cplusplus */

#endif 	/* __Plunger_FWD_DEFINED__ */


#ifndef __Textbox_FWD_DEFINED__
#define __Textbox_FWD_DEFINED__

#ifdef __cplusplus
typedef class Textbox Textbox;
#else
typedef struct Textbox Textbox;
#endif /* __cplusplus */

#endif 	/* __Textbox_FWD_DEFINED__ */


#ifndef __Bumper_FWD_DEFINED__
#define __Bumper_FWD_DEFINED__

#ifdef __cplusplus
typedef class Bumper Bumper;
#else
typedef struct Bumper Bumper;
#endif /* __cplusplus */

#endif 	/* __Bumper_FWD_DEFINED__ */


#ifndef __Trigger_FWD_DEFINED__
#define __Trigger_FWD_DEFINED__

#ifdef __cplusplus
typedef class Trigger Trigger;
#else
typedef struct Trigger Trigger;
#endif /* __cplusplus */

#endif 	/* __Trigger_FWD_DEFINED__ */


#ifndef __Light_FWD_DEFINED__
#define __Light_FWD_DEFINED__

#ifdef __cplusplus
typedef class Light Light;
#else
typedef struct Light Light;
#endif /* __cplusplus */

#endif 	/* __Light_FWD_DEFINED__ */


#ifndef __Kicker_FWD_DEFINED__
#define __Kicker_FWD_DEFINED__

#ifdef __cplusplus
typedef class Kicker Kicker;
#else
typedef struct Kicker Kicker;
#endif /* __cplusplus */

#endif 	/* __Kicker_FWD_DEFINED__ */


#ifndef __Gate_FWD_DEFINED__
#define __Gate_FWD_DEFINED__

#ifdef __cplusplus
typedef class Gate Gate;
#else
typedef struct Gate Gate;
#endif /* __cplusplus */

#endif 	/* __Gate_FWD_DEFINED__ */


#ifndef __Spinner_FWD_DEFINED__
#define __Spinner_FWD_DEFINED__

#ifdef __cplusplus
typedef class Spinner Spinner;
#else
typedef struct Spinner Spinner;
#endif /* __cplusplus */

#endif 	/* __Spinner_FWD_DEFINED__ */


#ifndef __Ramp_FWD_DEFINED__
#define __Ramp_FWD_DEFINED__

#ifdef __cplusplus
typedef class Ramp Ramp;
#else
typedef struct Ramp Ramp;
#endif /* __cplusplus */

#endif 	/* __Ramp_FWD_DEFINED__ */


#ifndef __Flasher_FWD_DEFINED__
#define __Flasher_FWD_DEFINED__

#ifdef __cplusplus
typedef class Flasher Flasher;
#else
typedef struct Flasher Flasher;
#endif /* __cplusplus */

#endif 	/* __Flasher_FWD_DEFINED__ */


#ifndef __Rubber_FWD_DEFINED__
#define __Rubber_FWD_DEFINED__

#ifdef __cplusplus
typedef class Rubber Rubber;
#else
typedef struct Rubber Rubber;
#endif /* __cplusplus */

#endif 	/* __Rubber_FWD_DEFINED__ */


#ifndef __Ball_FWD_DEFINED__
#define __Ball_FWD_DEFINED__

#ifdef __cplusplus
typedef class Ball Ball;
#else
typedef struct Ball Ball;
#endif /* __cplusplus */

#endif 	/* __Ball_FWD_DEFINED__ */


#ifndef __Collection_FWD_DEFINED__
#define __Collection_FWD_DEFINED__

#ifdef __cplusplus
typedef class Collection Collection;
#else
typedef struct Collection Collection;
#endif /* __cplusplus */

#endif 	/* __Collection_FWD_DEFINED__ */


#ifndef __DispReel_FWD_DEFINED__
#define __DispReel_FWD_DEFINED__

#ifdef __cplusplus
typedef class DispReel DispReel;
#else
typedef struct DispReel DispReel;
#endif /* __cplusplus */

#endif 	/* __DispReel_FWD_DEFINED__ */


#ifndef __LightSeq_FWD_DEFINED__
#define __LightSeq_FWD_DEFINED__

#ifdef __cplusplus
typedef class LightSeq LightSeq;
#else
typedef struct LightSeq LightSeq;
#endif /* __cplusplus */

#endif 	/* __LightSeq_FWD_DEFINED__ */


#ifndef __Primitive_FWD_DEFINED__
#define __Primitive_FWD_DEFINED__

#ifdef __cplusplus
typedef class Primitive Primitive;
#else
typedef struct Primitive Primitive;
#endif /* __cplusplus */

#endif 	/* __Primitive_FWD_DEFINED__ */


#ifndef __HitTarget_FWD_DEFINED__
#define __HitTarget_FWD_DEFINED__

#ifdef __cplusplus
typedef class HitTarget HitTarget;
#else
typedef struct HitTarget HitTarget;
#endif /* __cplusplus */

#endif 	/* __HitTarget_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


/* interface __MIDL_itf_vpinball_0000_0000 */
/* [local] */ 


#pragma once



extern RPC_IF_HANDLE __MIDL_itf_vpinball_0000_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_vpinball_0000_0000_v0_0_s_ifspec;


#ifndef __VPinballLib_LIBRARY_DEFINED__
#define __VPinballLib_LIBRARY_DEFINED__

/* library VPinballLib */
/* [helpstring][version][uuid] */ 

typedef /* [uuid] */  DECLSPEC_UUID("0C49D03B-76F1-47be-A139-B1A4AFB17A37") 
enum LightState
    {
        LightStateOff	= 0,
        LightStateOn	= 1,
        LightStateBlinking	= 2
    } 	LightState;

typedef /* [uuid] */  DECLSPEC_UUID("AC2AB9B5-FD04-47D9-ABAE-3BAE8EEBADA1") 
enum BackglassIndex
    {
        DESKTOP	= 110,
        FULLSCREEN	= 111,
        FULL_SINGLE_SCREEN	= 112
    } 	BackglassIndex;

typedef /* [uuid] */  DECLSPEC_UUID("7AB6EE50-539C-11E4-916C-0800200C9A66") 
enum Filters
    {
        Filter_None	= 0,
        Filter_Additive	= 1,
        Filter_Overlay	= 2,
        Filter_Multiply	= 3,
        Filter_Screen	= 4
    } 	Filters;

typedef /* [uuid] */  DECLSPEC_UUID("CF09B5B7-2DD1-4ba3-ADE7-2F08AB27FCA0") 
enum ImageAlignment
    {
        ImageAlignWorld	= 0,
        ImageAlignTopLeft	= 1,
        ImageAlignCenter	= 2
    } 	ImageAlignment;

typedef /* [uuid] */  DECLSPEC_UUID("5323810C-F833-48ef-B049-D07506A179B5") 
enum Shape
    {
        ShapeCircle	= 0,
        ShapeCustom	= 1
    } 	Shape;

typedef /* [uuid] */  DECLSPEC_UUID("28F7715B-E800-49B6-B86E-A0C55898297E") 
enum TriggerShape
    {
        TriggerNone	= 0,
        TriggerWireA	= 1,
        TriggerStar	= 2,
        TriggerWireB	= 3,
        TriggerButton	= 4,
        TriggerWireC	= 5,
        TriggerWireD	= 6,
        TriggerInder	= 7
    } 	TriggerShape;

typedef /* [uuid] */  DECLSPEC_UUID("E15D60BC-7E32-4cc5-BA16-E5C368D5891B") 
enum RampType
    {
        RampTypeFlat	= 0,
        RampType4Wire	= 1,
        RampType2Wire	= 2,
        RampType3WireLeft	= 3,
        RampType3WireRight	= 4,
        RampType1Wire	= 5
    } 	RampType;

typedef /* [uuid] */  DECLSPEC_UUID("4DD01BC6-6A50-4061-A01E-5798021A04AE") 
enum PlungerType
    {
        PlungerTypeModern	= 1,
        PlungerTypeFlat	= 2,
        PlungerTypeCustom	= 3
    } 	PlungerType;

typedef /* [uuid] */  DECLSPEC_UUID("C7891DB6-F553-4E12-9423-E5F7B90CDA14") 
enum UserDefaultOnOff
    {
        Default	= -1,
        Off	= 0,
        On	= 1
    } 	UserDefaultOnOff;

typedef /* [uuid] */  DECLSPEC_UUID("C8D1C791-3B7E-49F7-8F9B-D3BF4ECFB975") 
enum FXAASettings
    {
        Defaults	= -1,
        Disabled	= 0,
        Fast_FXAA	= 1,
        Standard_FXAA	= 2,
        Quality_FXAA	= 3,
        Fast_NFAA	= 4,
        Standard_DLAA	= 5,
        Quality_SMAA	= 6
    } 	FXAASettings;

typedef /* [uuid] */  DECLSPEC_UUID("222A3EDE-7508-4D89-BE82-30BF9AA2F77A") 
enum PhysicsSet
    {
        Disable	= 0,
        Set1	= 1,
        Set2	= 2,
        Set3	= 3,
        Set4	= 4,
        Set5	= 5,
        Set6	= 6,
        Set7	= 7,
        Set8	= 8
    } 	PhysicsSet;

typedef /* [uuid] */  DECLSPEC_UUID("704231FA-A3F9-47EE-9278-BA21A1E12566") 
enum TargetType
    {
        DropTargetBeveled	= 1,
        DropTargetSimple	= 2,
        HitTargetRound	= 3,
        HitTargetRectangle	= 4,
        HitFatTargetRectangle	= 5,
        HitFatTargetSquare	= 6,
        DropTargetFlatSimple	= 7,
        HitFatTargetSlim	= 8,
        HitTargetSlim	= 9
    } 	TargetType;

typedef /* [uuid] */  DECLSPEC_UUID("5E83616D-C764-4071-8C17-AB2A73D1B7A9") 
enum GateType
    {
        GateWireW	= 1,
        GateWireRectangle	= 2,
        GatePlate	= 3,
        GateLongPlate	= 4
    } 	GateType;

typedef /* [uuid] */  DECLSPEC_UUID("08844451-F92F-4d5c-8FFF-5973F7DBFFB0") 
enum TextAlignment
    {
        TextAlignLeft	= 0,
        TextAlignCenter	= 1,
        TextAlignRight	= 2
    } 	TextAlignment;

typedef /* [uuid] */  DECLSPEC_UUID("35AA214F-CC1B-4488-9FE8-9B15E0902E4F") 
enum DecalType
    {
        DecalText	= 0,
        DecalImage	= 1
    } 	DecalType;

typedef /* [uuid] */  DECLSPEC_UUID("0FEABC0C-56A4-414e-B76D-4C2CECF17A96") 
enum SequencerState
    {
        SeqUpOn	= 1,
        SeqUpOff	= 2,
        SeqDownOn	= 3,
        SeqDownOff	= 4,
        SeqRightOn	= 5,
        SeqRightOff	= 6,
        SeqLeftOn	= 7,
        SeqLeftOff	= 8,
        SeqDiagUpRightOn	= 9,
        SeqDiagUpRightOff	= 10,
        SeqDiagUpLeftOn	= 11,
        SeqDiagUpLeftOff	= 12,
        SeqDiagDownRightOn	= 13,
        SeqDiagDownRightOff	= 14,
        SeqDiagDownLeftOn	= 15,
        SeqDiagDownLeftOff	= 16,
        SeqMiddleOutHorizOn	= 17,
        SeqMiddleOutHorizOff	= 18,
        SeqMiddleInHorizOn	= 19,
        SeqMiddleInHorizOff	= 20,
        SeqMiddleOutVertOn	= 21,
        SeqMiddleOutVertOff	= 22,
        SeqMiddleInVertOn	= 23,
        SeqMiddleInVertOff	= 24,
        SeqStripe1HorizOn	= 25,
        SeqStripe1HorizOff	= 26,
        SeqStripe2HorizOn	= 27,
        SeqStripe2HorizOff	= 28,
        SeqStripe1VertOn	= 29,
        SeqStripe1VertOff	= 30,
        SeqStripe2VertOn	= 31,
        SeqStripe2VertOff	= 32,
        SeqHatch1HorizOn	= 33,
        SeqHatch1HorizOff	= 34,
        SeqHatch2HorizOn	= 35,
        SeqHatch2HorizOff	= 36,
        SeqHatch1VertOn	= 37,
        SeqHatch1VertOff	= 38,
        SeqHatch2VertOn	= 39,
        SeqHatch2VertOff	= 40,
        SeqCircleOutOn	= 41,
        SeqCircleOutOff	= 42,
        SeqCircleInOn	= 43,
        SeqCircleInOff	= 44,
        SeqClockRightOn	= 45,
        SeqClockRightOff	= 46,
        SeqClockLeftOn	= 47,
        SeqClockLeftOff	= 48,
        SeqRadarRightOn	= 49,
        SeqRadarRightOff	= 50,
        SeqRadarLeftOn	= 51,
        SeqRadarLeftOff	= 52,
        SeqWiperRightOn	= 53,
        SeqWiperRightOff	= 54,
        SeqWiperLeftOn	= 55,
        SeqWiperLeftOff	= 56,
        SeqFanLeftUpOn	= 57,
        SeqFanLeftUpOff	= 58,
        SeqFanLeftDownOn	= 59,
        SeqFanLeftDownOff	= 60,
        SeqFanRightUpOn	= 61,
        SeqFanRightUpOff	= 62,
        SeqFanRightDownOn	= 63,
        SeqFanRightDownOff	= 64,
        SeqArcBottomLeftUpOn	= 65,
        SeqArcBottomLeftUpOff	= 66,
        SeqArcBottomLeftDownOn	= 67,
        SeqArcBottomLeftDownOff	= 68,
        SeqArcBottomRightUpOn	= 69,
        SeqArcBottomRightUpOff	= 70,
        SeqArcBottomRightDownOn	= 71,
        SeqArcBottomRightDownOff	= 72,
        SeqArcTopLeftUpOn	= 73,
        SeqArcTopLeftUpOff	= 74,
        SeqArcTopLeftDownOn	= 75,
        SeqArcTopLeftDownOff	= 76,
        SeqArcTopRightUpOn	= 77,
        SeqArcTopRightUpOff	= 78,
        SeqArcTopRightDownOn	= 79,
        SeqArcTopRightDownOff	= 80,
        SeqScrewRightOn	= 81,
        SeqScrewRightOff	= 82,
        SeqScrewLeftOn	= 83,
        SeqScrewLeftOff	= 84,
        SeqLastDynamic	= 85,
        SeqAllOff	= 1000,
        SeqAllOn	= 1001,
        SeqBlinking	= 1002,
        SeqRandom	= 1003
    } 	SequencerState;

typedef /* [uuid] */  DECLSPEC_UUID("A5C333FD-0AD4-490a-BA6B-994CAB23501E") 
enum SizingType
    {
        AutoSize	= 0,
        AutoWidth	= 1,
        ManualSize	= 2
    } 	SizingType;

typedef /* [uuid] */  DECLSPEC_UUID("C0E543A2-50B3-4546-85F8-4D35BFBE09A2") 
enum KickerType
    {
        KickerInvisible	= 0,
        KickerHole	= 1,
        KickerCup	= 2,
        KickerHoleSimple	= 3,
        KickerWilliams	= 4,
        KickerGottlieb	= 5,
        KickerCup2	= 6
    } 	KickerType;

typedef /* [uuid] */  DECLSPEC_UUID("29BE2E7F-9F33-4d46-BE2B-617D1E0865E2") 
enum RampImageAlignment
    {
        ImageModeWorld	= 0,
        ImageModeWrap	= 1
    } 	RampImageAlignment;

typedef /* [uuid] */  DECLSPEC_UUID("4A5DD663-5D96-4fd9-90FE-ED7A37ECDB46") 
enum VBColors
    {
        vbBlack	= 0,
        vbRed	= 0xff,
        vbGreen	= 0xff00,
        vbYellow	= 0xffff,
        vbBlue	= 0xff0000,
        vbMagenta	= 0xff00ff,
        vbCyan	= 0xffff00,
        vbWhite	= 0xffffff
    } 	VBColors;

typedef /* [uuid] */  DECLSPEC_UUID("E8493732-3AA5-4531-A9BD-B0D8A9D73D5F") 
enum VBComparison
    {
        vbBinaryCompare	= 0,
        vbTextCompare	= 1
    } 	VBComparison;

typedef /* [uuid] */  DECLSPEC_UUID("7A7D27E0-5B0E-48c7-B24F-37A7424B286E") 
enum VBDateTime
    {
        vbSunday	= 1,
        vbMonday	= 2,
        vbTuesday	= 3,
        vbWednesday	= 4,
        vbThursday	= 5,
        vbFriday	= 6,
        vbSaturday	= 7,
        vbUseSystem	= 0,
        vbUseSystemDayOfWeek	= 0,
        vbFirstJan1	= 1,
        vbFirstFourDays	= 2,
        vbFirstFullWeek	= 3
    } 	VBDateTime;

typedef /* [uuid] */  DECLSPEC_UUID("337A49A9-CBA4-4834-9530-00AB6CDFB16A") 
enum VBDateFormat
    {
        vbGeneralDate	= 0,
        vbLongDate	= 1,
        vbShortDate	= 2,
        vbLongTime	= 3,
        vbShortTime	= 4
    } 	VBDateFormat;

typedef /* [uuid] */  DECLSPEC_UUID("B1883E02-15C8-4005-B0F8-EDC1180DB7AF") 
enum VBMsgBox
    {
        vbOKOnly	= 0,
        vbOKCancel	= 1,
        vbAbortRetryIgnore	= 2,
        vbYesNoCancel	= 3,
        vbYesNo	= 4,
        vbRetryCancel	= 5,
        vbCritical	= 16,
        vbQuestion	= 32,
        vbExclamation	= 48,
        vbInformation	= 64,
        vbDefaultButton1	= 0,
        vbDefaultButton2	= 256,
        vbDefaultButton3	= 512,
        vbDefaultButton4	= 768,
        vbApplicationModal	= 0,
        vbSystemModal	= 4096
    } 	VBMsgBox;

typedef /* [uuid] */  DECLSPEC_UUID("A971CCA3-FC4B-451c-A470-BB727C481EF6") 
enum VBMsgBoxReturn
    {
        vbOK	= 1,
        vbCancel	= 2,
        vbAbort	= 3,
        vbRetry	= 4,
        vbIgnore	= 5,
        vbYes	= 6,
        vbNo	= 7
    } 	VBMsgBoxReturn;

typedef /* [uuid] */  DECLSPEC_UUID("3CE2E3BC-463C-41a6-A30B-B08C4845E9AB") 
enum VBTriState
    {
        vbUseDefault	= -2,
        vbTrue	= -1,
        vbFalse	= 0
    } 	VTriState;

typedef /* [uuid] */  DECLSPEC_UUID("D09B9F0E-C1AE-40dd-84D6-B1F74053AA8E") 
enum VBVarType
    {
        vbEmpty	= 0,
        vbNull	= 1,
        vbInteger	= 2,
        vbLong	= 3,
        vbSingle	= 4,
        vbDouble	= 5,
        vbCurrency	= 6,
        vbDate	= 7,
        vbString	= 8,
        vbObject	= 9,
        vbError	= 10,
        vbBoolean	= 11,
        vbVariant	= 12,
        vbDataObject	= 13,
        vbDecimal	= 14,
        vbByte	= 17,
        vbArray	= 8192
    } 	VBVarType;


EXTERN_C const IID LIBID_VPinballLib;

#ifndef __ICollection_INTERFACE_DEFINED__
#define __ICollection_INTERFACE_DEFINED__

/* interface ICollection */
/* [dual][uuid][object] */ 


EXTERN_C const IID IID_ICollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BE073465-7189-4489-93BC-5B6ABBE1F880")
    ICollection : public IDispatch
    {
    public:
        virtual /* [helpcontext][id][propget] */ HRESULT STDMETHODCALLTYPE get__NewEnum( 
            /* [retval][out] */ IUnknown **ppunk) = 0;
        
        virtual /* [helpcontext][id][propget] */ HRESULT STDMETHODCALLTYPE get_Item( 
            /* [in] */ long Index,
            /* [retval][out] */ IDispatch **ppobject) = 0;
        
        virtual /* [helpcontext][propget] */ HRESULT STDMETHODCALLTYPE get_Count( 
            /* [retval][out] */ long *Count) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct ICollectionVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICollection * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICollection * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICollection * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ICollection * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ICollection * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ICollection * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ICollection * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        DECLSPEC_XFGVIRT(ICollection, get__NewEnum)
        /* [helpcontext][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ICollection * This,
            /* [retval][out] */ IUnknown **ppunk);
        
        DECLSPEC_XFGVIRT(ICollection, get_Item)
        /* [helpcontext][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ICollection * This,
            /* [in] */ long Index,
            /* [retval][out] */ IDispatch **ppobject);
        
        DECLSPEC_XFGVIRT(ICollection, get_Count)
        /* [helpcontext][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ICollection * This,
            /* [retval][out] */ long *Count);
        
        END_INTERFACE
    } ICollectionVtbl;

    interface ICollection
    {
        CONST_VTBL struct ICollectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICollection_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ICollection_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ICollection_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ICollection_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define ICollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define ICollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define ICollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define ICollection_get__NewEnum(This,ppunk)	\
    ( (This)->lpVtbl -> get__NewEnum(This,ppunk) ) 

#define ICollection_get_Item(This,Index,ppobject)	\
    ( (This)->lpVtbl -> get_Item(This,Index,ppobject) ) 

#define ICollection_get_Count(This,Count)	\
    ( (This)->lpVtbl -> get_Count(This,Count) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ICollection_INTERFACE_DEFINED__ */


#ifndef __ICollectionEvents_DISPINTERFACE_DEFINED__
#define __ICollectionEvents_DISPINTERFACE_DEFINED__

/* dispinterface ICollectionEvents */
/* [uuid] */ 


EXTERN_C const IID DIID_ICollectionEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("5B214770-EE20-47f4-9B18-D6491DC59D45")
    ICollectionEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct ICollectionEventsVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICollectionEvents * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICollectionEvents * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICollectionEvents * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ICollectionEvents * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ICollectionEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ICollectionEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ICollectionEvents * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        END_INTERFACE
    } ICollectionEventsVtbl;

    interface ICollectionEvents
    {
        CONST_VTBL struct ICollectionEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICollectionEvents_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ICollectionEvents_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ICollectionEvents_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ICollectionEvents_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define ICollectionEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define ICollectionEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define ICollectionEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* __ICollectionEvents_DISPINTERFACE_DEFINED__ */


#ifndef __ITable_INTERFACE_DEFINED__
#define __ITable_INTERFACE_DEFINED__

/* interface ITable */
/* [dual][uuid][object] */ 


EXTERN_C const IID IID_ITable;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AE85BCAC-1734-4e5c-81DA-075AD47A136E")
    ITable : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_GlassHeight( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_GlassHeight( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_PlayfieldMaterial( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_PlayfieldMaterial( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BackdropColor( 
            /* [retval][out] */ OLE_COLOR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_BackdropColor( 
            /* [in] */ OLE_COLOR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SlopeMax( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SlopeMax( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SlopeMin( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SlopeMin( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Inclination( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Inclination( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_FieldOfView( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_FieldOfView( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Layback( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Layback( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Rotation( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Rotation( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Scalex( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Scalex( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Scaley( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Scaley( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Scalez( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Scalez( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Xlatex( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Xlatex( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Xlatey( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Xlatey( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Xlatez( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Xlatez( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Gravity( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Gravity( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Friction( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Friction( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Elasticity( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Elasticity( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ElasticityFalloff( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ElasticityFalloff( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Scatter( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Scatter( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DefaultScatter( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_DefaultScatter( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_NudgeTime( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_NudgeTime( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_PlungerNormalize( 
            /* [retval][out] */ int *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_PlungerNormalize( 
            /* [in] */ int newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_PhysicsLoopTime( 
            /* [retval][out] */ int *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_PhysicsLoopTime( 
            /* [in] */ int newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_PlungerFilter( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_PlungerFilter( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_YieldTime( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_YieldTime( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BallImage( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_BallImage( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BackdropImage_DT( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_BackdropImage_DT( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BackdropImage_FS( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_BackdropImage_FS( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BackdropImage_FSS( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_BackdropImage_FSS( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BackdropImageApplyNightDay( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_BackdropImageApplyNightDay( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ColorGradeImage( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ColorGradeImage( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Width( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Width( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Height( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Height( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_MaxSeparation( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_MaxSeparation( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ZPD( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ZPD( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Offset( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Offset( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_GlobalStereo3D( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_GlobalStereo3D( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BallDecalMode( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_BallDecalMode( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Image( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Image( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_FileName( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Name( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_EnableAntialiasing( 
            /* [retval][out] */ UserDefaultOnOff *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_EnableAntialiasing( 
            /* [in] */ UserDefaultOnOff newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_EnableAO( 
            /* [retval][out] */ UserDefaultOnOff *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_EnableAO( 
            /* [in] */ UserDefaultOnOff newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_EnableFXAA( 
            /* [retval][out] */ FXAASettings *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_EnableFXAA( 
            /* [in] */ FXAASettings newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_EnableSSR( 
            /* [retval][out] */ UserDefaultOnOff *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_EnableSSR( 
            /* [in] */ UserDefaultOnOff newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BloomStrength( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_BloomStrength( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BallFrontDecal( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_BallFrontDecal( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_OverridePhysics( 
            /* [retval][out] */ PhysicsSet *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_OverridePhysics( 
            /* [in] */ PhysicsSet newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_OverridePhysicsFlippers( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_OverridePhysicsFlippers( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_EnableEMReels( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_EnableEMReels( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_EnableDecals( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_EnableDecals( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ShowDT( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ShowDT( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ShowFSS( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ShowFSS( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ReflectElementsOnPlayfield( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ReflectElementsOnPlayfield( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_EnvironmentImage( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_EnvironmentImage( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BackglassMode( 
            /* [retval][out] */ BackglassIndex *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_BackglassMode( 
            /* [in] */ BackglassIndex newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Accelerometer( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Accelerometer( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AccelNormalMount( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_AccelNormalMount( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AccelerometerAngle( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_AccelerometerAngle( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_GlobalDifficulty( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_GlobalDifficulty( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TableHeight( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TableHeight( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DeadZone( 
            /* [retval][out] */ int *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_DeadZone( 
            /* [in] */ int newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_LightAmbient( 
            /* [retval][out] */ OLE_COLOR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_LightAmbient( 
            /* [in] */ OLE_COLOR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Light0Emission( 
            /* [retval][out] */ OLE_COLOR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Light0Emission( 
            /* [in] */ OLE_COLOR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_LightHeight( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_LightHeight( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_LightRange( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_LightRange( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_EnvironmentEmissionScale( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_EnvironmentEmissionScale( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_LightEmissionScale( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_LightEmissionScale( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AOScale( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_AOScale( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SSRScale( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SSRScale( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TableSoundVolume( 
            /* [retval][out] */ int *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TableSoundVolume( 
            /* [in] */ int newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TableMusicVolume( 
            /* [retval][out] */ int *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TableMusicVolume( 
            /* [in] */ int newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TableAdaptiveVSync( 
            /* [retval][out] */ int *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TableAdaptiveVSync( 
            /* [in] */ int newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BallReflection( 
            /* [retval][out] */ UserDefaultOnOff *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_BallReflection( 
            /* [in] */ UserDefaultOnOff newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_PlayfieldReflectionStrength( 
            /* [retval][out] */ int *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_PlayfieldReflectionStrength( 
            /* [in] */ int newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BallTrail( 
            /* [retval][out] */ UserDefaultOnOff *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_BallTrail( 
            /* [in] */ UserDefaultOnOff newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TrailStrength( 
            /* [retval][out] */ int *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TrailStrength( 
            /* [in] */ int newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BallPlayfieldReflectionScale( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_BallPlayfieldReflectionScale( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DefaultBulbIntensityScale( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_DefaultBulbIntensityScale( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DetailLevel( 
            /* [retval][out] */ int *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_DetailLevel( 
            /* [in] */ int newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_NightDay( 
            /* [retval][out] */ int *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_NightDay( 
            /* [in] */ int newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_GlobalAlphaAcc( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_GlobalAlphaAcc( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_GlobalDayNight( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_GlobalDayNight( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Version( 
            /* [retval][out] */ int *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VersionMajor( 
            /* [retval][out] */ int *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VersionMinor( 
            /* [retval][out] */ int *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VersionRevision( 
            /* [retval][out] */ int *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VPBuildVersion( 
            /* [retval][out] */ int *pVal) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct ITableVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITable * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITable * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITable * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITable * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITable * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITable * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITable * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        DECLSPEC_XFGVIRT(ITable, get_GlassHeight)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_GlassHeight )( 
            ITable * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_GlassHeight)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_GlassHeight )( 
            ITable * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_PlayfieldMaterial)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_PlayfieldMaterial )( 
            ITable * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_PlayfieldMaterial)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_PlayfieldMaterial )( 
            ITable * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_BackdropColor)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BackdropColor )( 
            ITable * This,
            /* [retval][out] */ OLE_COLOR *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_BackdropColor)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_BackdropColor )( 
            ITable * This,
            /* [in] */ OLE_COLOR newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_SlopeMax)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SlopeMax )( 
            ITable * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_SlopeMax)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_SlopeMax )( 
            ITable * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_SlopeMin)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SlopeMin )( 
            ITable * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_SlopeMin)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_SlopeMin )( 
            ITable * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_Inclination)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Inclination )( 
            ITable * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_Inclination)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Inclination )( 
            ITable * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_FieldOfView)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_FieldOfView )( 
            ITable * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_FieldOfView)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_FieldOfView )( 
            ITable * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_Layback)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Layback )( 
            ITable * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_Layback)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Layback )( 
            ITable * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_Rotation)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Rotation )( 
            ITable * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_Rotation)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Rotation )( 
            ITable * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_Scalex)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Scalex )( 
            ITable * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_Scalex)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Scalex )( 
            ITable * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_Scaley)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Scaley )( 
            ITable * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_Scaley)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Scaley )( 
            ITable * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_Scalez)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Scalez )( 
            ITable * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_Scalez)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Scalez )( 
            ITable * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_Xlatex)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Xlatex )( 
            ITable * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_Xlatex)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Xlatex )( 
            ITable * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_Xlatey)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Xlatey )( 
            ITable * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_Xlatey)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Xlatey )( 
            ITable * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_Xlatez)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Xlatez )( 
            ITable * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_Xlatez)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Xlatez )( 
            ITable * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_Gravity)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Gravity )( 
            ITable * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_Gravity)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Gravity )( 
            ITable * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_Friction)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Friction )( 
            ITable * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_Friction)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Friction )( 
            ITable * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_Elasticity)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Elasticity )( 
            ITable * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_Elasticity)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Elasticity )( 
            ITable * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_ElasticityFalloff)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ElasticityFalloff )( 
            ITable * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_ElasticityFalloff)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ElasticityFalloff )( 
            ITable * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_Scatter)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Scatter )( 
            ITable * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_Scatter)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Scatter )( 
            ITable * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_DefaultScatter)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_DefaultScatter )( 
            ITable * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_DefaultScatter)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_DefaultScatter )( 
            ITable * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_NudgeTime)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_NudgeTime )( 
            ITable * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_NudgeTime)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_NudgeTime )( 
            ITable * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_PlungerNormalize)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_PlungerNormalize )( 
            ITable * This,
            /* [retval][out] */ int *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_PlungerNormalize)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_PlungerNormalize )( 
            ITable * This,
            /* [in] */ int newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_PhysicsLoopTime)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_PhysicsLoopTime )( 
            ITable * This,
            /* [retval][out] */ int *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_PhysicsLoopTime)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_PhysicsLoopTime )( 
            ITable * This,
            /* [in] */ int newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_PlungerFilter)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_PlungerFilter )( 
            ITable * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_PlungerFilter)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_PlungerFilter )( 
            ITable * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_YieldTime)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_YieldTime )( 
            ITable * This,
            /* [retval][out] */ long *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_YieldTime)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_YieldTime )( 
            ITable * This,
            /* [in] */ long newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_BallImage)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BallImage )( 
            ITable * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_BallImage)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_BallImage )( 
            ITable * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_BackdropImage_DT)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BackdropImage_DT )( 
            ITable * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_BackdropImage_DT)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_BackdropImage_DT )( 
            ITable * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_BackdropImage_FS)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BackdropImage_FS )( 
            ITable * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_BackdropImage_FS)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_BackdropImage_FS )( 
            ITable * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_BackdropImage_FSS)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BackdropImage_FSS )( 
            ITable * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_BackdropImage_FSS)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_BackdropImage_FSS )( 
            ITable * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_BackdropImageApplyNightDay)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BackdropImageApplyNightDay )( 
            ITable * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_BackdropImageApplyNightDay)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_BackdropImageApplyNightDay )( 
            ITable * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_ColorGradeImage)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ColorGradeImage )( 
            ITable * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_ColorGradeImage)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ColorGradeImage )( 
            ITable * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_Width)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Width )( 
            ITable * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_Width)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Width )( 
            ITable * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_Height)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Height )( 
            ITable * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_Height)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Height )( 
            ITable * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_MaxSeparation)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_MaxSeparation )( 
            ITable * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_MaxSeparation)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_MaxSeparation )( 
            ITable * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_ZPD)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ZPD )( 
            ITable * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_ZPD)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ZPD )( 
            ITable * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_Offset)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Offset )( 
            ITable * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_Offset)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Offset )( 
            ITable * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_GlobalStereo3D)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_GlobalStereo3D )( 
            ITable * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_GlobalStereo3D)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_GlobalStereo3D )( 
            ITable * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_BallDecalMode)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BallDecalMode )( 
            ITable * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_BallDecalMode)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_BallDecalMode )( 
            ITable * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_Image)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Image )( 
            ITable * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_Image)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Image )( 
            ITable * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_FileName)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_FileName )( 
            ITable * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(ITable, get_Name)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ITable * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_Name)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            ITable * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_EnableAntialiasing)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_EnableAntialiasing )( 
            ITable * This,
            /* [retval][out] */ UserDefaultOnOff *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_EnableAntialiasing)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_EnableAntialiasing )( 
            ITable * This,
            /* [in] */ UserDefaultOnOff newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_EnableAO)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_EnableAO )( 
            ITable * This,
            /* [retval][out] */ UserDefaultOnOff *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_EnableAO)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_EnableAO )( 
            ITable * This,
            /* [in] */ UserDefaultOnOff newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_EnableFXAA)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_EnableFXAA )( 
            ITable * This,
            /* [retval][out] */ FXAASettings *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_EnableFXAA)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_EnableFXAA )( 
            ITable * This,
            /* [in] */ FXAASettings newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_EnableSSR)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_EnableSSR )( 
            ITable * This,
            /* [retval][out] */ UserDefaultOnOff *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_EnableSSR)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_EnableSSR )( 
            ITable * This,
            /* [in] */ UserDefaultOnOff newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_BloomStrength)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BloomStrength )( 
            ITable * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_BloomStrength)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_BloomStrength )( 
            ITable * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_BallFrontDecal)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BallFrontDecal )( 
            ITable * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_BallFrontDecal)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_BallFrontDecal )( 
            ITable * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_OverridePhysics)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_OverridePhysics )( 
            ITable * This,
            /* [retval][out] */ PhysicsSet *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_OverridePhysics)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_OverridePhysics )( 
            ITable * This,
            /* [in] */ PhysicsSet newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_OverridePhysicsFlippers)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_OverridePhysicsFlippers )( 
            ITable * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_OverridePhysicsFlippers)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_OverridePhysicsFlippers )( 
            ITable * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_EnableEMReels)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_EnableEMReels )( 
            ITable * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_EnableEMReels)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_EnableEMReels )( 
            ITable * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_EnableDecals)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_EnableDecals )( 
            ITable * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_EnableDecals)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_EnableDecals )( 
            ITable * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_ShowDT)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ShowDT )( 
            ITable * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_ShowDT)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ShowDT )( 
            ITable * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_ShowFSS)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ShowFSS )( 
            ITable * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_ShowFSS)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ShowFSS )( 
            ITable * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_ReflectElementsOnPlayfield)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ReflectElementsOnPlayfield )( 
            ITable * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_ReflectElementsOnPlayfield)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ReflectElementsOnPlayfield )( 
            ITable * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_EnvironmentImage)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_EnvironmentImage )( 
            ITable * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_EnvironmentImage)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_EnvironmentImage )( 
            ITable * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_BackglassMode)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BackglassMode )( 
            ITable * This,
            /* [retval][out] */ BackglassIndex *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_BackglassMode)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_BackglassMode )( 
            ITable * This,
            /* [in] */ BackglassIndex newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_Accelerometer)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Accelerometer )( 
            ITable * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_Accelerometer)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Accelerometer )( 
            ITable * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_AccelNormalMount)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AccelNormalMount )( 
            ITable * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_AccelNormalMount)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_AccelNormalMount )( 
            ITable * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_AccelerometerAngle)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AccelerometerAngle )( 
            ITable * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_AccelerometerAngle)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_AccelerometerAngle )( 
            ITable * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_GlobalDifficulty)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_GlobalDifficulty )( 
            ITable * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_GlobalDifficulty)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_GlobalDifficulty )( 
            ITable * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_TableHeight)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TableHeight )( 
            ITable * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_TableHeight)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TableHeight )( 
            ITable * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_DeadZone)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_DeadZone )( 
            ITable * This,
            /* [retval][out] */ int *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_DeadZone)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_DeadZone )( 
            ITable * This,
            /* [in] */ int newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_LightAmbient)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_LightAmbient )( 
            ITable * This,
            /* [retval][out] */ OLE_COLOR *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_LightAmbient)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_LightAmbient )( 
            ITable * This,
            /* [in] */ OLE_COLOR newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_Light0Emission)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Light0Emission )( 
            ITable * This,
            /* [retval][out] */ OLE_COLOR *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_Light0Emission)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Light0Emission )( 
            ITable * This,
            /* [in] */ OLE_COLOR newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_LightHeight)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_LightHeight )( 
            ITable * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_LightHeight)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_LightHeight )( 
            ITable * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_LightRange)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_LightRange )( 
            ITable * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_LightRange)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_LightRange )( 
            ITable * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_EnvironmentEmissionScale)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_EnvironmentEmissionScale )( 
            ITable * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_EnvironmentEmissionScale)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_EnvironmentEmissionScale )( 
            ITable * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_LightEmissionScale)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_LightEmissionScale )( 
            ITable * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_LightEmissionScale)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_LightEmissionScale )( 
            ITable * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_AOScale)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AOScale )( 
            ITable * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_AOScale)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_AOScale )( 
            ITable * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_SSRScale)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SSRScale )( 
            ITable * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_SSRScale)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_SSRScale )( 
            ITable * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_TableSoundVolume)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TableSoundVolume )( 
            ITable * This,
            /* [retval][out] */ int *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_TableSoundVolume)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TableSoundVolume )( 
            ITable * This,
            /* [in] */ int newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_TableMusicVolume)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TableMusicVolume )( 
            ITable * This,
            /* [retval][out] */ int *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_TableMusicVolume)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TableMusicVolume )( 
            ITable * This,
            /* [in] */ int newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_TableAdaptiveVSync)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TableAdaptiveVSync )( 
            ITable * This,
            /* [retval][out] */ int *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_TableAdaptiveVSync)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TableAdaptiveVSync )( 
            ITable * This,
            /* [in] */ int newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_BallReflection)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BallReflection )( 
            ITable * This,
            /* [retval][out] */ UserDefaultOnOff *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_BallReflection)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_BallReflection )( 
            ITable * This,
            /* [in] */ UserDefaultOnOff newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_PlayfieldReflectionStrength)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_PlayfieldReflectionStrength )( 
            ITable * This,
            /* [retval][out] */ int *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_PlayfieldReflectionStrength)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_PlayfieldReflectionStrength )( 
            ITable * This,
            /* [in] */ int newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_BallTrail)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BallTrail )( 
            ITable * This,
            /* [retval][out] */ UserDefaultOnOff *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_BallTrail)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_BallTrail )( 
            ITable * This,
            /* [in] */ UserDefaultOnOff newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_TrailStrength)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TrailStrength )( 
            ITable * This,
            /* [retval][out] */ int *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_TrailStrength)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TrailStrength )( 
            ITable * This,
            /* [in] */ int newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_BallPlayfieldReflectionScale)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BallPlayfieldReflectionScale )( 
            ITable * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_BallPlayfieldReflectionScale)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_BallPlayfieldReflectionScale )( 
            ITable * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_DefaultBulbIntensityScale)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_DefaultBulbIntensityScale )( 
            ITable * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_DefaultBulbIntensityScale)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_DefaultBulbIntensityScale )( 
            ITable * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_DetailLevel)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_DetailLevel )( 
            ITable * This,
            /* [retval][out] */ int *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_DetailLevel)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_DetailLevel )( 
            ITable * This,
            /* [in] */ int newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_NightDay)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_NightDay )( 
            ITable * This,
            /* [retval][out] */ int *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_NightDay)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_NightDay )( 
            ITable * This,
            /* [in] */ int newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_GlobalAlphaAcc)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_GlobalAlphaAcc )( 
            ITable * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_GlobalAlphaAcc)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_GlobalAlphaAcc )( 
            ITable * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_GlobalDayNight)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_GlobalDayNight )( 
            ITable * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(ITable, put_GlobalDayNight)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_GlobalDayNight )( 
            ITable * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(ITable, get_Version)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Version )( 
            ITable * This,
            /* [retval][out] */ int *pVal);
        
        DECLSPEC_XFGVIRT(ITable, get_VersionMajor)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VersionMajor )( 
            ITable * This,
            /* [retval][out] */ int *pVal);
        
        DECLSPEC_XFGVIRT(ITable, get_VersionMinor)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VersionMinor )( 
            ITable * This,
            /* [retval][out] */ int *pVal);
        
        DECLSPEC_XFGVIRT(ITable, get_VersionRevision)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VersionRevision )( 
            ITable * This,
            /* [retval][out] */ int *pVal);
        
        DECLSPEC_XFGVIRT(ITable, get_VPBuildVersion)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VPBuildVersion )( 
            ITable * This,
            /* [retval][out] */ int *pVal);
        
        END_INTERFACE
    } ITableVtbl;

    interface ITable
    {
        CONST_VTBL struct ITableVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITable_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ITable_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ITable_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ITable_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define ITable_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define ITable_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define ITable_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define ITable_get_GlassHeight(This,pVal)	\
    ( (This)->lpVtbl -> get_GlassHeight(This,pVal) ) 

#define ITable_put_GlassHeight(This,newVal)	\
    ( (This)->lpVtbl -> put_GlassHeight(This,newVal) ) 

#define ITable_get_PlayfieldMaterial(This,pVal)	\
    ( (This)->lpVtbl -> get_PlayfieldMaterial(This,pVal) ) 

#define ITable_put_PlayfieldMaterial(This,newVal)	\
    ( (This)->lpVtbl -> put_PlayfieldMaterial(This,newVal) ) 

#define ITable_get_BackdropColor(This,pVal)	\
    ( (This)->lpVtbl -> get_BackdropColor(This,pVal) ) 

#define ITable_put_BackdropColor(This,newVal)	\
    ( (This)->lpVtbl -> put_BackdropColor(This,newVal) ) 

#define ITable_get_SlopeMax(This,pVal)	\
    ( (This)->lpVtbl -> get_SlopeMax(This,pVal) ) 

#define ITable_put_SlopeMax(This,newVal)	\
    ( (This)->lpVtbl -> put_SlopeMax(This,newVal) ) 

#define ITable_get_SlopeMin(This,pVal)	\
    ( (This)->lpVtbl -> get_SlopeMin(This,pVal) ) 

#define ITable_put_SlopeMin(This,newVal)	\
    ( (This)->lpVtbl -> put_SlopeMin(This,newVal) ) 

#define ITable_get_Inclination(This,pVal)	\
    ( (This)->lpVtbl -> get_Inclination(This,pVal) ) 

#define ITable_put_Inclination(This,newVal)	\
    ( (This)->lpVtbl -> put_Inclination(This,newVal) ) 

#define ITable_get_FieldOfView(This,pVal)	\
    ( (This)->lpVtbl -> get_FieldOfView(This,pVal) ) 

#define ITable_put_FieldOfView(This,newVal)	\
    ( (This)->lpVtbl -> put_FieldOfView(This,newVal) ) 

#define ITable_get_Layback(This,pVal)	\
    ( (This)->lpVtbl -> get_Layback(This,pVal) ) 

#define ITable_put_Layback(This,newVal)	\
    ( (This)->lpVtbl -> put_Layback(This,newVal) ) 

#define ITable_get_Rotation(This,pVal)	\
    ( (This)->lpVtbl -> get_Rotation(This,pVal) ) 

#define ITable_put_Rotation(This,newVal)	\
    ( (This)->lpVtbl -> put_Rotation(This,newVal) ) 

#define ITable_get_Scalex(This,pVal)	\
    ( (This)->lpVtbl -> get_Scalex(This,pVal) ) 

#define ITable_put_Scalex(This,newVal)	\
    ( (This)->lpVtbl -> put_Scalex(This,newVal) ) 

#define ITable_get_Scaley(This,pVal)	\
    ( (This)->lpVtbl -> get_Scaley(This,pVal) ) 

#define ITable_put_Scaley(This,newVal)	\
    ( (This)->lpVtbl -> put_Scaley(This,newVal) ) 

#define ITable_get_Scalez(This,pVal)	\
    ( (This)->lpVtbl -> get_Scalez(This,pVal) ) 

#define ITable_put_Scalez(This,newVal)	\
    ( (This)->lpVtbl -> put_Scalez(This,newVal) ) 

#define ITable_get_Xlatex(This,pVal)	\
    ( (This)->lpVtbl -> get_Xlatex(This,pVal) ) 

#define ITable_put_Xlatex(This,newVal)	\
    ( (This)->lpVtbl -> put_Xlatex(This,newVal) ) 

#define ITable_get_Xlatey(This,pVal)	\
    ( (This)->lpVtbl -> get_Xlatey(This,pVal) ) 

#define ITable_put_Xlatey(This,newVal)	\
    ( (This)->lpVtbl -> put_Xlatey(This,newVal) ) 

#define ITable_get_Xlatez(This,pVal)	\
    ( (This)->lpVtbl -> get_Xlatez(This,pVal) ) 

#define ITable_put_Xlatez(This,newVal)	\
    ( (This)->lpVtbl -> put_Xlatez(This,newVal) ) 

#define ITable_get_Gravity(This,pVal)	\
    ( (This)->lpVtbl -> get_Gravity(This,pVal) ) 

#define ITable_put_Gravity(This,newVal)	\
    ( (This)->lpVtbl -> put_Gravity(This,newVal) ) 

#define ITable_get_Friction(This,pVal)	\
    ( (This)->lpVtbl -> get_Friction(This,pVal) ) 

#define ITable_put_Friction(This,newVal)	\
    ( (This)->lpVtbl -> put_Friction(This,newVal) ) 

#define ITable_get_Elasticity(This,pVal)	\
    ( (This)->lpVtbl -> get_Elasticity(This,pVal) ) 

#define ITable_put_Elasticity(This,newVal)	\
    ( (This)->lpVtbl -> put_Elasticity(This,newVal) ) 

#define ITable_get_ElasticityFalloff(This,pVal)	\
    ( (This)->lpVtbl -> get_ElasticityFalloff(This,pVal) ) 

#define ITable_put_ElasticityFalloff(This,newVal)	\
    ( (This)->lpVtbl -> put_ElasticityFalloff(This,newVal) ) 

#define ITable_get_Scatter(This,pVal)	\
    ( (This)->lpVtbl -> get_Scatter(This,pVal) ) 

#define ITable_put_Scatter(This,newVal)	\
    ( (This)->lpVtbl -> put_Scatter(This,newVal) ) 

#define ITable_get_DefaultScatter(This,pVal)	\
    ( (This)->lpVtbl -> get_DefaultScatter(This,pVal) ) 

#define ITable_put_DefaultScatter(This,newVal)	\
    ( (This)->lpVtbl -> put_DefaultScatter(This,newVal) ) 

#define ITable_get_NudgeTime(This,pVal)	\
    ( (This)->lpVtbl -> get_NudgeTime(This,pVal) ) 

#define ITable_put_NudgeTime(This,newVal)	\
    ( (This)->lpVtbl -> put_NudgeTime(This,newVal) ) 

#define ITable_get_PlungerNormalize(This,pVal)	\
    ( (This)->lpVtbl -> get_PlungerNormalize(This,pVal) ) 

#define ITable_put_PlungerNormalize(This,newVal)	\
    ( (This)->lpVtbl -> put_PlungerNormalize(This,newVal) ) 

#define ITable_get_PhysicsLoopTime(This,pVal)	\
    ( (This)->lpVtbl -> get_PhysicsLoopTime(This,pVal) ) 

#define ITable_put_PhysicsLoopTime(This,newVal)	\
    ( (This)->lpVtbl -> put_PhysicsLoopTime(This,newVal) ) 

#define ITable_get_PlungerFilter(This,pVal)	\
    ( (This)->lpVtbl -> get_PlungerFilter(This,pVal) ) 

#define ITable_put_PlungerFilter(This,newVal)	\
    ( (This)->lpVtbl -> put_PlungerFilter(This,newVal) ) 

#define ITable_get_YieldTime(This,pVal)	\
    ( (This)->lpVtbl -> get_YieldTime(This,pVal) ) 

#define ITable_put_YieldTime(This,newVal)	\
    ( (This)->lpVtbl -> put_YieldTime(This,newVal) ) 

#define ITable_get_BallImage(This,pVal)	\
    ( (This)->lpVtbl -> get_BallImage(This,pVal) ) 

#define ITable_put_BallImage(This,newVal)	\
    ( (This)->lpVtbl -> put_BallImage(This,newVal) ) 

#define ITable_get_BackdropImage_DT(This,pVal)	\
    ( (This)->lpVtbl -> get_BackdropImage_DT(This,pVal) ) 

#define ITable_put_BackdropImage_DT(This,newVal)	\
    ( (This)->lpVtbl -> put_BackdropImage_DT(This,newVal) ) 

#define ITable_get_BackdropImage_FS(This,pVal)	\
    ( (This)->lpVtbl -> get_BackdropImage_FS(This,pVal) ) 

#define ITable_put_BackdropImage_FS(This,newVal)	\
    ( (This)->lpVtbl -> put_BackdropImage_FS(This,newVal) ) 

#define ITable_get_BackdropImage_FSS(This,pVal)	\
    ( (This)->lpVtbl -> get_BackdropImage_FSS(This,pVal) ) 

#define ITable_put_BackdropImage_FSS(This,newVal)	\
    ( (This)->lpVtbl -> put_BackdropImage_FSS(This,newVal) ) 

#define ITable_get_BackdropImageApplyNightDay(This,pVal)	\
    ( (This)->lpVtbl -> get_BackdropImageApplyNightDay(This,pVal) ) 

#define ITable_put_BackdropImageApplyNightDay(This,newVal)	\
    ( (This)->lpVtbl -> put_BackdropImageApplyNightDay(This,newVal) ) 

#define ITable_get_ColorGradeImage(This,pVal)	\
    ( (This)->lpVtbl -> get_ColorGradeImage(This,pVal) ) 

#define ITable_put_ColorGradeImage(This,newVal)	\
    ( (This)->lpVtbl -> put_ColorGradeImage(This,newVal) ) 

#define ITable_get_Width(This,pVal)	\
    ( (This)->lpVtbl -> get_Width(This,pVal) ) 

#define ITable_put_Width(This,newVal)	\
    ( (This)->lpVtbl -> put_Width(This,newVal) ) 

#define ITable_get_Height(This,pVal)	\
    ( (This)->lpVtbl -> get_Height(This,pVal) ) 

#define ITable_put_Height(This,newVal)	\
    ( (This)->lpVtbl -> put_Height(This,newVal) ) 

#define ITable_get_MaxSeparation(This,pVal)	\
    ( (This)->lpVtbl -> get_MaxSeparation(This,pVal) ) 

#define ITable_put_MaxSeparation(This,newVal)	\
    ( (This)->lpVtbl -> put_MaxSeparation(This,newVal) ) 

#define ITable_get_ZPD(This,pVal)	\
    ( (This)->lpVtbl -> get_ZPD(This,pVal) ) 

#define ITable_put_ZPD(This,newVal)	\
    ( (This)->lpVtbl -> put_ZPD(This,newVal) ) 

#define ITable_get_Offset(This,pVal)	\
    ( (This)->lpVtbl -> get_Offset(This,pVal) ) 

#define ITable_put_Offset(This,newVal)	\
    ( (This)->lpVtbl -> put_Offset(This,newVal) ) 

#define ITable_get_GlobalStereo3D(This,pVal)	\
    ( (This)->lpVtbl -> get_GlobalStereo3D(This,pVal) ) 

#define ITable_put_GlobalStereo3D(This,newVal)	\
    ( (This)->lpVtbl -> put_GlobalStereo3D(This,newVal) ) 

#define ITable_get_BallDecalMode(This,pVal)	\
    ( (This)->lpVtbl -> get_BallDecalMode(This,pVal) ) 

#define ITable_put_BallDecalMode(This,newVal)	\
    ( (This)->lpVtbl -> put_BallDecalMode(This,newVal) ) 

#define ITable_get_Image(This,pVal)	\
    ( (This)->lpVtbl -> get_Image(This,pVal) ) 

#define ITable_put_Image(This,newVal)	\
    ( (This)->lpVtbl -> put_Image(This,newVal) ) 

#define ITable_get_FileName(This,pVal)	\
    ( (This)->lpVtbl -> get_FileName(This,pVal) ) 

#define ITable_get_Name(This,pVal)	\
    ( (This)->lpVtbl -> get_Name(This,pVal) ) 

#define ITable_put_Name(This,newVal)	\
    ( (This)->lpVtbl -> put_Name(This,newVal) ) 

#define ITable_get_EnableAntialiasing(This,pVal)	\
    ( (This)->lpVtbl -> get_EnableAntialiasing(This,pVal) ) 

#define ITable_put_EnableAntialiasing(This,newVal)	\
    ( (This)->lpVtbl -> put_EnableAntialiasing(This,newVal) ) 

#define ITable_get_EnableAO(This,pVal)	\
    ( (This)->lpVtbl -> get_EnableAO(This,pVal) ) 

#define ITable_put_EnableAO(This,newVal)	\
    ( (This)->lpVtbl -> put_EnableAO(This,newVal) ) 

#define ITable_get_EnableFXAA(This,pVal)	\
    ( (This)->lpVtbl -> get_EnableFXAA(This,pVal) ) 

#define ITable_put_EnableFXAA(This,newVal)	\
    ( (This)->lpVtbl -> put_EnableFXAA(This,newVal) ) 

#define ITable_get_EnableSSR(This,pVal)	\
    ( (This)->lpVtbl -> get_EnableSSR(This,pVal) ) 

#define ITable_put_EnableSSR(This,newVal)	\
    ( (This)->lpVtbl -> put_EnableSSR(This,newVal) ) 

#define ITable_get_BloomStrength(This,pVal)	\
    ( (This)->lpVtbl -> get_BloomStrength(This,pVal) ) 

#define ITable_put_BloomStrength(This,newVal)	\
    ( (This)->lpVtbl -> put_BloomStrength(This,newVal) ) 

#define ITable_get_BallFrontDecal(This,pVal)	\
    ( (This)->lpVtbl -> get_BallFrontDecal(This,pVal) ) 

#define ITable_put_BallFrontDecal(This,newVal)	\
    ( (This)->lpVtbl -> put_BallFrontDecal(This,newVal) ) 

#define ITable_get_OverridePhysics(This,pVal)	\
    ( (This)->lpVtbl -> get_OverridePhysics(This,pVal) ) 

#define ITable_put_OverridePhysics(This,newVal)	\
    ( (This)->lpVtbl -> put_OverridePhysics(This,newVal) ) 

#define ITable_get_OverridePhysicsFlippers(This,pVal)	\
    ( (This)->lpVtbl -> get_OverridePhysicsFlippers(This,pVal) ) 

#define ITable_put_OverridePhysicsFlippers(This,newVal)	\
    ( (This)->lpVtbl -> put_OverridePhysicsFlippers(This,newVal) ) 

#define ITable_get_EnableEMReels(This,pVal)	\
    ( (This)->lpVtbl -> get_EnableEMReels(This,pVal) ) 

#define ITable_put_EnableEMReels(This,newVal)	\
    ( (This)->lpVtbl -> put_EnableEMReels(This,newVal) ) 

#define ITable_get_EnableDecals(This,pVal)	\
    ( (This)->lpVtbl -> get_EnableDecals(This,pVal) ) 

#define ITable_put_EnableDecals(This,newVal)	\
    ( (This)->lpVtbl -> put_EnableDecals(This,newVal) ) 

#define ITable_get_ShowDT(This,pVal)	\
    ( (This)->lpVtbl -> get_ShowDT(This,pVal) ) 

#define ITable_put_ShowDT(This,newVal)	\
    ( (This)->lpVtbl -> put_ShowDT(This,newVal) ) 

#define ITable_get_ShowFSS(This,pVal)	\
    ( (This)->lpVtbl -> get_ShowFSS(This,pVal) ) 

#define ITable_put_ShowFSS(This,newVal)	\
    ( (This)->lpVtbl -> put_ShowFSS(This,newVal) ) 

#define ITable_get_ReflectElementsOnPlayfield(This,pVal)	\
    ( (This)->lpVtbl -> get_ReflectElementsOnPlayfield(This,pVal) ) 

#define ITable_put_ReflectElementsOnPlayfield(This,newVal)	\
    ( (This)->lpVtbl -> put_ReflectElementsOnPlayfield(This,newVal) ) 

#define ITable_get_EnvironmentImage(This,pVal)	\
    ( (This)->lpVtbl -> get_EnvironmentImage(This,pVal) ) 

#define ITable_put_EnvironmentImage(This,newVal)	\
    ( (This)->lpVtbl -> put_EnvironmentImage(This,newVal) ) 

#define ITable_get_BackglassMode(This,pVal)	\
    ( (This)->lpVtbl -> get_BackglassMode(This,pVal) ) 

#define ITable_put_BackglassMode(This,newVal)	\
    ( (This)->lpVtbl -> put_BackglassMode(This,newVal) ) 

#define ITable_get_Accelerometer(This,pVal)	\
    ( (This)->lpVtbl -> get_Accelerometer(This,pVal) ) 

#define ITable_put_Accelerometer(This,newVal)	\
    ( (This)->lpVtbl -> put_Accelerometer(This,newVal) ) 

#define ITable_get_AccelNormalMount(This,pVal)	\
    ( (This)->lpVtbl -> get_AccelNormalMount(This,pVal) ) 

#define ITable_put_AccelNormalMount(This,newVal)	\
    ( (This)->lpVtbl -> put_AccelNormalMount(This,newVal) ) 

#define ITable_get_AccelerometerAngle(This,pVal)	\
    ( (This)->lpVtbl -> get_AccelerometerAngle(This,pVal) ) 

#define ITable_put_AccelerometerAngle(This,newVal)	\
    ( (This)->lpVtbl -> put_AccelerometerAngle(This,newVal) ) 

#define ITable_get_GlobalDifficulty(This,pVal)	\
    ( (This)->lpVtbl -> get_GlobalDifficulty(This,pVal) ) 

#define ITable_put_GlobalDifficulty(This,newVal)	\
    ( (This)->lpVtbl -> put_GlobalDifficulty(This,newVal) ) 

#define ITable_get_TableHeight(This,pVal)	\
    ( (This)->lpVtbl -> get_TableHeight(This,pVal) ) 

#define ITable_put_TableHeight(This,newVal)	\
    ( (This)->lpVtbl -> put_TableHeight(This,newVal) ) 

#define ITable_get_DeadZone(This,pVal)	\
    ( (This)->lpVtbl -> get_DeadZone(This,pVal) ) 

#define ITable_put_DeadZone(This,newVal)	\
    ( (This)->lpVtbl -> put_DeadZone(This,newVal) ) 

#define ITable_get_LightAmbient(This,pVal)	\
    ( (This)->lpVtbl -> get_LightAmbient(This,pVal) ) 

#define ITable_put_LightAmbient(This,newVal)	\
    ( (This)->lpVtbl -> put_LightAmbient(This,newVal) ) 

#define ITable_get_Light0Emission(This,pVal)	\
    ( (This)->lpVtbl -> get_Light0Emission(This,pVal) ) 

#define ITable_put_Light0Emission(This,newVal)	\
    ( (This)->lpVtbl -> put_Light0Emission(This,newVal) ) 

#define ITable_get_LightHeight(This,pVal)	\
    ( (This)->lpVtbl -> get_LightHeight(This,pVal) ) 

#define ITable_put_LightHeight(This,newVal)	\
    ( (This)->lpVtbl -> put_LightHeight(This,newVal) ) 

#define ITable_get_LightRange(This,pVal)	\
    ( (This)->lpVtbl -> get_LightRange(This,pVal) ) 

#define ITable_put_LightRange(This,newVal)	\
    ( (This)->lpVtbl -> put_LightRange(This,newVal) ) 

#define ITable_get_EnvironmentEmissionScale(This,pVal)	\
    ( (This)->lpVtbl -> get_EnvironmentEmissionScale(This,pVal) ) 

#define ITable_put_EnvironmentEmissionScale(This,newVal)	\
    ( (This)->lpVtbl -> put_EnvironmentEmissionScale(This,newVal) ) 

#define ITable_get_LightEmissionScale(This,pVal)	\
    ( (This)->lpVtbl -> get_LightEmissionScale(This,pVal) ) 

#define ITable_put_LightEmissionScale(This,newVal)	\
    ( (This)->lpVtbl -> put_LightEmissionScale(This,newVal) ) 

#define ITable_get_AOScale(This,pVal)	\
    ( (This)->lpVtbl -> get_AOScale(This,pVal) ) 

#define ITable_put_AOScale(This,newVal)	\
    ( (This)->lpVtbl -> put_AOScale(This,newVal) ) 

#define ITable_get_SSRScale(This,pVal)	\
    ( (This)->lpVtbl -> get_SSRScale(This,pVal) ) 

#define ITable_put_SSRScale(This,newVal)	\
    ( (This)->lpVtbl -> put_SSRScale(This,newVal) ) 

#define ITable_get_TableSoundVolume(This,pVal)	\
    ( (This)->lpVtbl -> get_TableSoundVolume(This,pVal) ) 

#define ITable_put_TableSoundVolume(This,newVal)	\
    ( (This)->lpVtbl -> put_TableSoundVolume(This,newVal) ) 

#define ITable_get_TableMusicVolume(This,pVal)	\
    ( (This)->lpVtbl -> get_TableMusicVolume(This,pVal) ) 

#define ITable_put_TableMusicVolume(This,newVal)	\
    ( (This)->lpVtbl -> put_TableMusicVolume(This,newVal) ) 

#define ITable_get_TableAdaptiveVSync(This,pVal)	\
    ( (This)->lpVtbl -> get_TableAdaptiveVSync(This,pVal) ) 

#define ITable_put_TableAdaptiveVSync(This,newVal)	\
    ( (This)->lpVtbl -> put_TableAdaptiveVSync(This,newVal) ) 

#define ITable_get_BallReflection(This,pVal)	\
    ( (This)->lpVtbl -> get_BallReflection(This,pVal) ) 

#define ITable_put_BallReflection(This,newVal)	\
    ( (This)->lpVtbl -> put_BallReflection(This,newVal) ) 

#define ITable_get_PlayfieldReflectionStrength(This,pVal)	\
    ( (This)->lpVtbl -> get_PlayfieldReflectionStrength(This,pVal) ) 

#define ITable_put_PlayfieldReflectionStrength(This,newVal)	\
    ( (This)->lpVtbl -> put_PlayfieldReflectionStrength(This,newVal) ) 

#define ITable_get_BallTrail(This,pVal)	\
    ( (This)->lpVtbl -> get_BallTrail(This,pVal) ) 

#define ITable_put_BallTrail(This,newVal)	\
    ( (This)->lpVtbl -> put_BallTrail(This,newVal) ) 

#define ITable_get_TrailStrength(This,pVal)	\
    ( (This)->lpVtbl -> get_TrailStrength(This,pVal) ) 

#define ITable_put_TrailStrength(This,newVal)	\
    ( (This)->lpVtbl -> put_TrailStrength(This,newVal) ) 

#define ITable_get_BallPlayfieldReflectionScale(This,pVal)	\
    ( (This)->lpVtbl -> get_BallPlayfieldReflectionScale(This,pVal) ) 

#define ITable_put_BallPlayfieldReflectionScale(This,newVal)	\
    ( (This)->lpVtbl -> put_BallPlayfieldReflectionScale(This,newVal) ) 

#define ITable_get_DefaultBulbIntensityScale(This,pVal)	\
    ( (This)->lpVtbl -> get_DefaultBulbIntensityScale(This,pVal) ) 

#define ITable_put_DefaultBulbIntensityScale(This,newVal)	\
    ( (This)->lpVtbl -> put_DefaultBulbIntensityScale(This,newVal) ) 

#define ITable_get_DetailLevel(This,pVal)	\
    ( (This)->lpVtbl -> get_DetailLevel(This,pVal) ) 

#define ITable_put_DetailLevel(This,newVal)	\
    ( (This)->lpVtbl -> put_DetailLevel(This,newVal) ) 

#define ITable_get_NightDay(This,pVal)	\
    ( (This)->lpVtbl -> get_NightDay(This,pVal) ) 

#define ITable_put_NightDay(This,newVal)	\
    ( (This)->lpVtbl -> put_NightDay(This,newVal) ) 

#define ITable_get_GlobalAlphaAcc(This,pVal)	\
    ( (This)->lpVtbl -> get_GlobalAlphaAcc(This,pVal) ) 

#define ITable_put_GlobalAlphaAcc(This,newVal)	\
    ( (This)->lpVtbl -> put_GlobalAlphaAcc(This,newVal) ) 

#define ITable_get_GlobalDayNight(This,pVal)	\
    ( (This)->lpVtbl -> get_GlobalDayNight(This,pVal) ) 

#define ITable_put_GlobalDayNight(This,newVal)	\
    ( (This)->lpVtbl -> put_GlobalDayNight(This,newVal) ) 

#define ITable_get_Version(This,pVal)	\
    ( (This)->lpVtbl -> get_Version(This,pVal) ) 

#define ITable_get_VersionMajor(This,pVal)	\
    ( (This)->lpVtbl -> get_VersionMajor(This,pVal) ) 

#define ITable_get_VersionMinor(This,pVal)	\
    ( (This)->lpVtbl -> get_VersionMinor(This,pVal) ) 

#define ITable_get_VersionRevision(This,pVal)	\
    ( (This)->lpVtbl -> get_VersionRevision(This,pVal) ) 

#define ITable_get_VPBuildVersion(This,pVal)	\
    ( (This)->lpVtbl -> get_VPBuildVersion(This,pVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ITable_get_TableHeight_Proxy( 
    ITable * This,
    /* [retval][out] */ float *pVal);


void __RPC_STUB ITable_get_TableHeight_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE ITable_put_TableHeight_Proxy( 
    ITable * This,
    /* [in] */ float newVal);


void __RPC_STUB ITable_put_TableHeight_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ITable_get_DeadZone_Proxy( 
    ITable * This,
    /* [retval][out] */ int *pVal);


void __RPC_STUB ITable_get_DeadZone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE ITable_put_DeadZone_Proxy( 
    ITable * This,
    /* [in] */ int newVal);


void __RPC_STUB ITable_put_DeadZone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ITable_get_LightAmbient_Proxy( 
    ITable * This,
    /* [retval][out] */ OLE_COLOR *pVal);


void __RPC_STUB ITable_get_LightAmbient_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE ITable_put_LightAmbient_Proxy( 
    ITable * This,
    /* [in] */ OLE_COLOR newVal);


void __RPC_STUB ITable_put_LightAmbient_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ITable_get_Light0Emission_Proxy( 
    ITable * This,
    /* [retval][out] */ OLE_COLOR *pVal);


void __RPC_STUB ITable_get_Light0Emission_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE ITable_put_Light0Emission_Proxy( 
    ITable * This,
    /* [in] */ OLE_COLOR newVal);


void __RPC_STUB ITable_put_Light0Emission_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ITable_get_LightHeight_Proxy( 
    ITable * This,
    /* [retval][out] */ float *pVal);


void __RPC_STUB ITable_get_LightHeight_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE ITable_put_LightHeight_Proxy( 
    ITable * This,
    /* [in] */ float newVal);


void __RPC_STUB ITable_put_LightHeight_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ITable_get_LightRange_Proxy( 
    ITable * This,
    /* [retval][out] */ float *pVal);


void __RPC_STUB ITable_get_LightRange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE ITable_put_LightRange_Proxy( 
    ITable * This,
    /* [in] */ float newVal);


void __RPC_STUB ITable_put_LightRange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ITable_get_EnvironmentEmissionScale_Proxy( 
    ITable * This,
    /* [retval][out] */ float *pVal);


void __RPC_STUB ITable_get_EnvironmentEmissionScale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE ITable_put_EnvironmentEmissionScale_Proxy( 
    ITable * This,
    /* [in] */ float newVal);


void __RPC_STUB ITable_put_EnvironmentEmissionScale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ITable_get_LightEmissionScale_Proxy( 
    ITable * This,
    /* [retval][out] */ float *pVal);


void __RPC_STUB ITable_get_LightEmissionScale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE ITable_put_LightEmissionScale_Proxy( 
    ITable * This,
    /* [in] */ float newVal);


void __RPC_STUB ITable_put_LightEmissionScale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ITable_get_AOScale_Proxy( 
    ITable * This,
    /* [retval][out] */ float *pVal);


void __RPC_STUB ITable_get_AOScale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE ITable_put_AOScale_Proxy( 
    ITable * This,
    /* [in] */ float newVal);


void __RPC_STUB ITable_put_AOScale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ITable_get_SSRScale_Proxy( 
    ITable * This,
    /* [retval][out] */ float *pVal);


void __RPC_STUB ITable_get_SSRScale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE ITable_put_SSRScale_Proxy( 
    ITable * This,
    /* [in] */ float newVal);


void __RPC_STUB ITable_put_SSRScale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ITable_get_TableSoundVolume_Proxy( 
    ITable * This,
    /* [retval][out] */ int *pVal);


void __RPC_STUB ITable_get_TableSoundVolume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE ITable_put_TableSoundVolume_Proxy( 
    ITable * This,
    /* [in] */ int newVal);


void __RPC_STUB ITable_put_TableSoundVolume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ITable_get_TableMusicVolume_Proxy( 
    ITable * This,
    /* [retval][out] */ int *pVal);


void __RPC_STUB ITable_get_TableMusicVolume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE ITable_put_TableMusicVolume_Proxy( 
    ITable * This,
    /* [in] */ int newVal);


void __RPC_STUB ITable_put_TableMusicVolume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ITable_get_TableAdaptiveVSync_Proxy( 
    ITable * This,
    /* [retval][out] */ int *pVal);


void __RPC_STUB ITable_get_TableAdaptiveVSync_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE ITable_put_TableAdaptiveVSync_Proxy( 
    ITable * This,
    /* [in] */ int newVal);


void __RPC_STUB ITable_put_TableAdaptiveVSync_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ITable_get_BallReflection_Proxy( 
    ITable * This,
    /* [retval][out] */ UserDefaultOnOff *pVal);


void __RPC_STUB ITable_get_BallReflection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE ITable_put_BallReflection_Proxy( 
    ITable * This,
    /* [in] */ UserDefaultOnOff newVal);


void __RPC_STUB ITable_put_BallReflection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ITable_get_PlayfieldReflectionStrength_Proxy( 
    ITable * This,
    /* [retval][out] */ int *pVal);


void __RPC_STUB ITable_get_PlayfieldReflectionStrength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE ITable_put_PlayfieldReflectionStrength_Proxy( 
    ITable * This,
    /* [in] */ int newVal);


void __RPC_STUB ITable_put_PlayfieldReflectionStrength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ITable_get_BallTrail_Proxy( 
    ITable * This,
    /* [retval][out] */ UserDefaultOnOff *pVal);


void __RPC_STUB ITable_get_BallTrail_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE ITable_put_BallTrail_Proxy( 
    ITable * This,
    /* [in] */ UserDefaultOnOff newVal);


void __RPC_STUB ITable_put_BallTrail_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ITable_get_TrailStrength_Proxy( 
    ITable * This,
    /* [retval][out] */ int *pVal);


void __RPC_STUB ITable_get_TrailStrength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE ITable_put_TrailStrength_Proxy( 
    ITable * This,
    /* [in] */ int newVal);


void __RPC_STUB ITable_put_TrailStrength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ITable_get_BallPlayfieldReflectionScale_Proxy( 
    ITable * This,
    /* [retval][out] */ float *pVal);


void __RPC_STUB ITable_get_BallPlayfieldReflectionScale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE ITable_put_BallPlayfieldReflectionScale_Proxy( 
    ITable * This,
    /* [in] */ float newVal);


void __RPC_STUB ITable_put_BallPlayfieldReflectionScale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ITable_get_DefaultBulbIntensityScale_Proxy( 
    ITable * This,
    /* [retval][out] */ float *pVal);


void __RPC_STUB ITable_get_DefaultBulbIntensityScale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE ITable_put_DefaultBulbIntensityScale_Proxy( 
    ITable * This,
    /* [in] */ float newVal);


void __RPC_STUB ITable_put_DefaultBulbIntensityScale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ITable_get_DetailLevel_Proxy( 
    ITable * This,
    /* [retval][out] */ int *pVal);


void __RPC_STUB ITable_get_DetailLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE ITable_put_DetailLevel_Proxy( 
    ITable * This,
    /* [in] */ int newVal);


void __RPC_STUB ITable_put_DetailLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ITable_get_NightDay_Proxy( 
    ITable * This,
    /* [retval][out] */ int *pVal);


void __RPC_STUB ITable_get_NightDay_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE ITable_put_NightDay_Proxy( 
    ITable * This,
    /* [in] */ int newVal);


void __RPC_STUB ITable_put_NightDay_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ITable_get_GlobalAlphaAcc_Proxy( 
    ITable * This,
    /* [retval][out] */ VARIANT_BOOL *pVal);


void __RPC_STUB ITable_get_GlobalAlphaAcc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE ITable_put_GlobalAlphaAcc_Proxy( 
    ITable * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB ITable_put_GlobalAlphaAcc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ITable_get_GlobalDayNight_Proxy( 
    ITable * This,
    /* [retval][out] */ VARIANT_BOOL *pVal);


void __RPC_STUB ITable_get_GlobalDayNight_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE ITable_put_GlobalDayNight_Proxy( 
    ITable * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB ITable_put_GlobalDayNight_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ITable_get_Version_Proxy( 
    ITable * This,
    /* [retval][out] */ int *pVal);


void __RPC_STUB ITable_get_Version_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ITable_get_VersionMajor_Proxy( 
    ITable * This,
    /* [retval][out] */ int *pVal);


void __RPC_STUB ITable_get_VersionMajor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ITable_get_VersionMinor_Proxy( 
    ITable * This,
    /* [retval][out] */ int *pVal);


void __RPC_STUB ITable_get_VersionMinor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ITable_get_VersionRevision_Proxy( 
    ITable * This,
    /* [retval][out] */ int *pVal);


void __RPC_STUB ITable_get_VersionRevision_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ITable_get_VPBuildVersion_Proxy( 
    ITable * This,
    /* [retval][out] */ int *pVal);


void __RPC_STUB ITable_get_VPBuildVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ITable_INTERFACE_DEFINED__ */


#ifndef __ITableGlobal_INTERFACE_DEFINED__
#define __ITableGlobal_INTERFACE_DEFINED__

/* interface ITableGlobal */
/* [dual][uuid][object] */ 


EXTERN_C const IID IID_ITableGlobal;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2981E0E0-8E64-44fc-9A01-64CFFA1F7DBA")
    ITableGlobal : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE PlaySound( 
            BSTR Sound,
            /* [defaultvalue] */ long LoopCount = 1,
            /* [defaultvalue] */ float Volume = 1,
            /* [defaultvalue] */ float pan = 0,
            /* [defaultvalue] */ float randompitch = 0,
            /* [defaultvalue] */ long pitch = 0,
            /* [defaultvalue] */ VARIANT_BOOL usesame = 0,
            /* [defaultvalue] */ VARIANT_BOOL restart = 1,
            /* [defaultvalue] */ float front_rear_fade = 0) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_LeftFlipperKey( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RightFlipperKey( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_LeftTiltKey( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RightTiltKey( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CenterTiltKey( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_PlungerKey( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE PlayMusic( 
            BSTR str,
            /* [defaultvalue] */ float Volume = 1) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_MusicVolume( 
            float Volume) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE EndMusic( void) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_StartGameKey( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_UserDirectory( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_GetPlayerHWnd( 
            /* [retval][out] */ SIZE_T *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE StopSound( 
            BSTR Sound) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SaveValue( 
            BSTR TableName,
            BSTR ValueName,
            VARIANT Value) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE LoadValue( 
            BSTR TableName,
            BSTR ValueName,
            /* [retval][out] */ VARIANT *Value) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ActiveBall( 
            /* [retval][out] */ IBall **pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AddCreditKey( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AddCreditKey2( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_GameTime( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SystemTime( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetCustomParam( 
            long index,
            /* [retval][out] */ BSTR *param) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetTextFile( 
            BSTR FileName,
            /* [retval][out] */ BSTR *pContents) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE BeginModal( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE EndModal( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Nudge( 
            /* [in] */ float Angle,
            /* [in] */ float Force) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE NudgeGetCalibration( 
            /* [out] */ VARIANT *XMax,
            /* [out] */ VARIANT *YMax,
            /* [out] */ VARIANT *XGain,
            /* [out] */ VARIANT *YGain,
            /* [out] */ VARIANT *DeadZone,
            /* [out] */ VARIANT *TiltSensitivty) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE NudgeSetCalibration( 
            /* [in] */ int XMax,
            /* [in] */ int YMax,
            /* [in] */ int XGain,
            /* [in] */ int YGain,
            /* [in] */ int DeadZone,
            /* [in] */ int TiltSensitivty) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE NudgeSensorStatus( 
            /* [out] */ VARIANT *XNudge,
            /* [out] */ VARIANT *YNudge) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE NudgeTiltStatus( 
            /* [out] */ VARIANT *XPlumb,
            /* [out] */ VARIANT *YPlumb,
            /* [out] */ VARIANT *TiltPercent) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_MechanicalTilt( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_LeftMagnaSave( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RightMagnaSave( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ExitGame( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_LockbarKey( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE FireKnocker( 
            /* [defaultvalue] */ int Count = 1) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE QuitPlayer( 
            /* [defaultvalue] */ int CloseType = 0) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ShowDT( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ShowFSS( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_NightDay( 
            /* [retval][out] */ int *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetBalls( 
            /* [retval][out] */ SAFEARRAY * *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetElements( 
            /* [retval][out] */ SAFEARRAY * *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetElementByName( 
            /* [in] */ BSTR name,
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ActiveTable( 
            /* [retval][out] */ ITable **pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_DMDWidth( 
            /* [in] */ int pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_DMDHeight( 
            /* [in] */ int pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_DMDPixels( 
            /* [in] */ VARIANT pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_DMDColoredPixels( 
            /* [in] */ VARIANT pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Version( 
            /* [retval][out] */ int *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VersionMajor( 
            /* [retval][out] */ int *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VersionMinor( 
            /* [retval][out] */ int *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VersionRevision( 
            /* [retval][out] */ int *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VPBuildVersion( 
            /* [retval][out] */ int *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetSerialDevices( 
            /* [retval][out] */ VARIANT *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE OpenSerial( 
            /* [in] */ BSTR device) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE CloseSerial( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE FlushSerial( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetupSerial( 
            /* [in] */ int baud,
            /* [in] */ int bits,
            /* [in] */ int parity,
            /* [in] */ int stopbit,
            /* [in] */ VARIANT_BOOL rts,
            /* [in] */ VARIANT_BOOL dtr) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ReadSerial( 
            /* [in] */ int size,
            /* [out] */ VARIANT *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE WriteSerial( 
            /* [in] */ VARIANT pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RenderingMode( 
            /* [retval][out] */ int *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE UpdateMaterial( 
            /* [in] */ BSTR pVal,
            /* [in] */ float wrapLighting,
            /* [in] */ float roughness,
            /* [in] */ float glossyImageLerp,
            /* [in] */ float thickness,
            /* [in] */ float edge,
            /* [in] */ float edgeAlpha,
            /* [in] */ float opacity,
            /* [in] */ OLE_COLOR base,
            /* [in] */ OLE_COLOR glossy,
            /* [in] */ OLE_COLOR clearcoat,
            /* [in] */ VARIANT_BOOL isMetal,
            /* [in] */ VARIANT_BOOL opacityActive,
            /* [in] */ float elasticity,
            /* [in] */ float elasticityFalloff,
            /* [in] */ float friction,
            /* [in] */ float scatterAngle) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetMaterial( 
            /* [in] */ BSTR pVal,
            /* [out] */ VARIANT *wrapLighting,
            /* [out] */ VARIANT *roughness,
            /* [out] */ VARIANT *glossyImageLerp,
            /* [out] */ VARIANT *thickness,
            /* [out] */ VARIANT *edge,
            /* [out] */ VARIANT *edgeAlpha,
            /* [out] */ VARIANT *opacity,
            /* [out] */ VARIANT *base,
            /* [out] */ VARIANT *glossy,
            /* [out] */ VARIANT *clearcoat,
            /* [out] */ VARIANT *isMetal,
            /* [out] */ VARIANT *opacityActive,
            /* [out] */ VARIANT *elasticity,
            /* [out] */ VARIANT *elasticityFalloff,
            /* [out] */ VARIANT *friction,
            /* [out] */ VARIANT *scatterAngle) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE UpdateMaterialPhysics( 
            /* [in] */ BSTR pVal,
            /* [in] */ float elasticity,
            /* [in] */ float elasticityFalloff,
            /* [in] */ float friction,
            /* [in] */ float scatterAngle) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetMaterialPhysics( 
            /* [in] */ BSTR pVal,
            /* [out] */ VARIANT *elasticity,
            /* [out] */ VARIANT *elasticityFalloff,
            /* [out] */ VARIANT *friction,
            /* [out] */ VARIANT *scatterAngle) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE MaterialColor( 
            /* [in] */ BSTR name,
            /* [in] */ OLE_COLOR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_WindowWidth( 
            /* [retval][out] */ int *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_WindowHeight( 
            /* [retval][out] */ int *pVal) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct ITableGlobalVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITableGlobal * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITableGlobal * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITableGlobal * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITableGlobal * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITableGlobal * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITableGlobal * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITableGlobal * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        DECLSPEC_XFGVIRT(ITableGlobal, PlaySound)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *PlaySound )( 
            ITableGlobal * This,
            BSTR Sound,
            /* [defaultvalue] */ long LoopCount,
            /* [defaultvalue] */ float Volume,
            /* [defaultvalue] */ float pan,
            /* [defaultvalue] */ float randompitch,
            /* [defaultvalue] */ long pitch,
            /* [defaultvalue] */ VARIANT_BOOL usesame,
            /* [defaultvalue] */ VARIANT_BOOL restart,
            /* [defaultvalue] */ float front_rear_fade);
        
        DECLSPEC_XFGVIRT(ITableGlobal, get_LeftFlipperKey)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_LeftFlipperKey )( 
            ITableGlobal * This,
            /* [retval][out] */ long *pVal);
        
        DECLSPEC_XFGVIRT(ITableGlobal, get_RightFlipperKey)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RightFlipperKey )( 
            ITableGlobal * This,
            /* [retval][out] */ long *pVal);
        
        DECLSPEC_XFGVIRT(ITableGlobal, get_LeftTiltKey)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_LeftTiltKey )( 
            ITableGlobal * This,
            /* [retval][out] */ long *pVal);
        
        DECLSPEC_XFGVIRT(ITableGlobal, get_RightTiltKey)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RightTiltKey )( 
            ITableGlobal * This,
            /* [retval][out] */ long *pVal);
        
        DECLSPEC_XFGVIRT(ITableGlobal, get_CenterTiltKey)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CenterTiltKey )( 
            ITableGlobal * This,
            /* [retval][out] */ long *pVal);
        
        DECLSPEC_XFGVIRT(ITableGlobal, get_PlungerKey)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_PlungerKey )( 
            ITableGlobal * This,
            /* [retval][out] */ long *pVal);
        
        DECLSPEC_XFGVIRT(ITableGlobal, PlayMusic)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *PlayMusic )( 
            ITableGlobal * This,
            BSTR str,
            /* [defaultvalue] */ float Volume);
        
        DECLSPEC_XFGVIRT(ITableGlobal, put_MusicVolume)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_MusicVolume )( 
            ITableGlobal * This,
            float Volume);
        
        DECLSPEC_XFGVIRT(ITableGlobal, EndMusic)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *EndMusic )( 
            ITableGlobal * This);
        
        DECLSPEC_XFGVIRT(ITableGlobal, get_StartGameKey)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_StartGameKey )( 
            ITableGlobal * This,
            /* [retval][out] */ long *pVal);
        
        DECLSPEC_XFGVIRT(ITableGlobal, get_UserDirectory)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_UserDirectory )( 
            ITableGlobal * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(ITableGlobal, get_GetPlayerHWnd)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_GetPlayerHWnd )( 
            ITableGlobal * This,
            /* [retval][out] */ SIZE_T *pVal);
        
        DECLSPEC_XFGVIRT(ITableGlobal, StopSound)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *StopSound )( 
            ITableGlobal * This,
            BSTR Sound);
        
        DECLSPEC_XFGVIRT(ITableGlobal, SaveValue)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SaveValue )( 
            ITableGlobal * This,
            BSTR TableName,
            BSTR ValueName,
            VARIANT Value);
        
        DECLSPEC_XFGVIRT(ITableGlobal, LoadValue)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *LoadValue )( 
            ITableGlobal * This,
            BSTR TableName,
            BSTR ValueName,
            /* [retval][out] */ VARIANT *Value);
        
        DECLSPEC_XFGVIRT(ITableGlobal, get_ActiveBall)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ActiveBall )( 
            ITableGlobal * This,
            /* [retval][out] */ IBall **pVal);
        
        DECLSPEC_XFGVIRT(ITableGlobal, get_AddCreditKey)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AddCreditKey )( 
            ITableGlobal * This,
            /* [retval][out] */ long *pVal);
        
        DECLSPEC_XFGVIRT(ITableGlobal, get_AddCreditKey2)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AddCreditKey2 )( 
            ITableGlobal * This,
            /* [retval][out] */ long *pVal);
        
        DECLSPEC_XFGVIRT(ITableGlobal, get_GameTime)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_GameTime )( 
            ITableGlobal * This,
            /* [retval][out] */ long *pVal);
        
        DECLSPEC_XFGVIRT(ITableGlobal, get_SystemTime)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SystemTime )( 
            ITableGlobal * This,
            /* [retval][out] */ long *pVal);
        
        DECLSPEC_XFGVIRT(ITableGlobal, GetCustomParam)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetCustomParam )( 
            ITableGlobal * This,
            long index,
            /* [retval][out] */ BSTR *param);
        
        DECLSPEC_XFGVIRT(ITableGlobal, GetTextFile)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetTextFile )( 
            ITableGlobal * This,
            BSTR FileName,
            /* [retval][out] */ BSTR *pContents);
        
        DECLSPEC_XFGVIRT(ITableGlobal, BeginModal)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *BeginModal )( 
            ITableGlobal * This);
        
        DECLSPEC_XFGVIRT(ITableGlobal, EndModal)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *EndModal )( 
            ITableGlobal * This);
        
        DECLSPEC_XFGVIRT(ITableGlobal, Nudge)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Nudge )( 
            ITableGlobal * This,
            /* [in] */ float Angle,
            /* [in] */ float Force);
        
        DECLSPEC_XFGVIRT(ITableGlobal, NudgeGetCalibration)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *NudgeGetCalibration )( 
            ITableGlobal * This,
            /* [out] */ VARIANT *XMax,
            /* [out] */ VARIANT *YMax,
            /* [out] */ VARIANT *XGain,
            /* [out] */ VARIANT *YGain,
            /* [out] */ VARIANT *DeadZone,
            /* [out] */ VARIANT *TiltSensitivty);
        
        DECLSPEC_XFGVIRT(ITableGlobal, NudgeSetCalibration)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *NudgeSetCalibration )( 
            ITableGlobal * This,
            /* [in] */ int XMax,
            /* [in] */ int YMax,
            /* [in] */ int XGain,
            /* [in] */ int YGain,
            /* [in] */ int DeadZone,
            /* [in] */ int TiltSensitivty);
        
        DECLSPEC_XFGVIRT(ITableGlobal, NudgeSensorStatus)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *NudgeSensorStatus )( 
            ITableGlobal * This,
            /* [out] */ VARIANT *XNudge,
            /* [out] */ VARIANT *YNudge);
        
        DECLSPEC_XFGVIRT(ITableGlobal, NudgeTiltStatus)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *NudgeTiltStatus )( 
            ITableGlobal * This,
            /* [out] */ VARIANT *XPlumb,
            /* [out] */ VARIANT *YPlumb,
            /* [out] */ VARIANT *TiltPercent);
        
        DECLSPEC_XFGVIRT(ITableGlobal, get_MechanicalTilt)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_MechanicalTilt )( 
            ITableGlobal * This,
            /* [retval][out] */ long *pVal);
        
        DECLSPEC_XFGVIRT(ITableGlobal, get_LeftMagnaSave)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_LeftMagnaSave )( 
            ITableGlobal * This,
            /* [retval][out] */ long *pVal);
        
        DECLSPEC_XFGVIRT(ITableGlobal, get_RightMagnaSave)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RightMagnaSave )( 
            ITableGlobal * This,
            /* [retval][out] */ long *pVal);
        
        DECLSPEC_XFGVIRT(ITableGlobal, get_ExitGame)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ExitGame )( 
            ITableGlobal * This,
            /* [retval][out] */ long *pVal);
        
        DECLSPEC_XFGVIRT(ITableGlobal, get_LockbarKey)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_LockbarKey )( 
            ITableGlobal * This,
            /* [retval][out] */ long *pVal);
        
        DECLSPEC_XFGVIRT(ITableGlobal, FireKnocker)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *FireKnocker )( 
            ITableGlobal * This,
            /* [defaultvalue] */ int Count);
        
        DECLSPEC_XFGVIRT(ITableGlobal, QuitPlayer)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *QuitPlayer )( 
            ITableGlobal * This,
            /* [defaultvalue] */ int CloseType);
        
        DECLSPEC_XFGVIRT(ITableGlobal, get_ShowDT)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ShowDT )( 
            ITableGlobal * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(ITableGlobal, get_ShowFSS)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ShowFSS )( 
            ITableGlobal * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(ITableGlobal, get_NightDay)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_NightDay )( 
            ITableGlobal * This,
            /* [retval][out] */ int *pVal);
        
        DECLSPEC_XFGVIRT(ITableGlobal, GetBalls)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetBalls )( 
            ITableGlobal * This,
            /* [retval][out] */ SAFEARRAY * *pVal);
        
        DECLSPEC_XFGVIRT(ITableGlobal, GetElements)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetElements )( 
            ITableGlobal * This,
            /* [retval][out] */ SAFEARRAY * *pVal);
        
        DECLSPEC_XFGVIRT(ITableGlobal, GetElementByName)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetElementByName )( 
            ITableGlobal * This,
            /* [in] */ BSTR name,
            /* [retval][out] */ IDispatch **pVal);
        
        DECLSPEC_XFGVIRT(ITableGlobal, get_ActiveTable)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ActiveTable )( 
            ITableGlobal * This,
            /* [retval][out] */ ITable **pVal);
        
        DECLSPEC_XFGVIRT(ITableGlobal, put_DMDWidth)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_DMDWidth )( 
            ITableGlobal * This,
            /* [in] */ int pVal);
        
        DECLSPEC_XFGVIRT(ITableGlobal, put_DMDHeight)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_DMDHeight )( 
            ITableGlobal * This,
            /* [in] */ int pVal);
        
        DECLSPEC_XFGVIRT(ITableGlobal, put_DMDPixels)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_DMDPixels )( 
            ITableGlobal * This,
            /* [in] */ VARIANT pVal);
        
        DECLSPEC_XFGVIRT(ITableGlobal, put_DMDColoredPixels)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_DMDColoredPixels )( 
            ITableGlobal * This,
            /* [in] */ VARIANT pVal);
        
        DECLSPEC_XFGVIRT(ITableGlobal, get_Version)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Version )( 
            ITableGlobal * This,
            /* [retval][out] */ int *pVal);
        
        DECLSPEC_XFGVIRT(ITableGlobal, get_VersionMajor)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VersionMajor )( 
            ITableGlobal * This,
            /* [retval][out] */ int *pVal);
        
        DECLSPEC_XFGVIRT(ITableGlobal, get_VersionMinor)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VersionMinor )( 
            ITableGlobal * This,
            /* [retval][out] */ int *pVal);
        
        DECLSPEC_XFGVIRT(ITableGlobal, get_VersionRevision)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VersionRevision )( 
            ITableGlobal * This,
            /* [retval][out] */ int *pVal);
        
        DECLSPEC_XFGVIRT(ITableGlobal, get_VPBuildVersion)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VPBuildVersion )( 
            ITableGlobal * This,
            /* [retval][out] */ int *pVal);
        
        DECLSPEC_XFGVIRT(ITableGlobal, GetSerialDevices)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetSerialDevices )( 
            ITableGlobal * This,
            /* [retval][out] */ VARIANT *pVal);
        
        DECLSPEC_XFGVIRT(ITableGlobal, OpenSerial)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *OpenSerial )( 
            ITableGlobal * This,
            /* [in] */ BSTR device);
        
        DECLSPEC_XFGVIRT(ITableGlobal, CloseSerial)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *CloseSerial )( 
            ITableGlobal * This);
        
        DECLSPEC_XFGVIRT(ITableGlobal, FlushSerial)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *FlushSerial )( 
            ITableGlobal * This);
        
        DECLSPEC_XFGVIRT(ITableGlobal, SetupSerial)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetupSerial )( 
            ITableGlobal * This,
            /* [in] */ int baud,
            /* [in] */ int bits,
            /* [in] */ int parity,
            /* [in] */ int stopbit,
            /* [in] */ VARIANT_BOOL rts,
            /* [in] */ VARIANT_BOOL dtr);
        
        DECLSPEC_XFGVIRT(ITableGlobal, ReadSerial)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *ReadSerial )( 
            ITableGlobal * This,
            /* [in] */ int size,
            /* [out] */ VARIANT *pVal);
        
        DECLSPEC_XFGVIRT(ITableGlobal, WriteSerial)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *WriteSerial )( 
            ITableGlobal * This,
            /* [in] */ VARIANT pVal);
        
        DECLSPEC_XFGVIRT(ITableGlobal, get_RenderingMode)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RenderingMode )( 
            ITableGlobal * This,
            /* [retval][out] */ int *pVal);
        
        DECLSPEC_XFGVIRT(ITableGlobal, UpdateMaterial)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *UpdateMaterial )( 
            ITableGlobal * This,
            /* [in] */ BSTR pVal,
            /* [in] */ float wrapLighting,
            /* [in] */ float roughness,
            /* [in] */ float glossyImageLerp,
            /* [in] */ float thickness,
            /* [in] */ float edge,
            /* [in] */ float edgeAlpha,
            /* [in] */ float opacity,
            /* [in] */ OLE_COLOR base,
            /* [in] */ OLE_COLOR glossy,
            /* [in] */ OLE_COLOR clearcoat,
            /* [in] */ VARIANT_BOOL isMetal,
            /* [in] */ VARIANT_BOOL opacityActive,
            /* [in] */ float elasticity,
            /* [in] */ float elasticityFalloff,
            /* [in] */ float friction,
            /* [in] */ float scatterAngle);
        
        DECLSPEC_XFGVIRT(ITableGlobal, GetMaterial)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetMaterial )( 
            ITableGlobal * This,
            /* [in] */ BSTR pVal,
            /* [out] */ VARIANT *wrapLighting,
            /* [out] */ VARIANT *roughness,
            /* [out] */ VARIANT *glossyImageLerp,
            /* [out] */ VARIANT *thickness,
            /* [out] */ VARIANT *edge,
            /* [out] */ VARIANT *edgeAlpha,
            /* [out] */ VARIANT *opacity,
            /* [out] */ VARIANT *base,
            /* [out] */ VARIANT *glossy,
            /* [out] */ VARIANT *clearcoat,
            /* [out] */ VARIANT *isMetal,
            /* [out] */ VARIANT *opacityActive,
            /* [out] */ VARIANT *elasticity,
            /* [out] */ VARIANT *elasticityFalloff,
            /* [out] */ VARIANT *friction,
            /* [out] */ VARIANT *scatterAngle);
        
        DECLSPEC_XFGVIRT(ITableGlobal, UpdateMaterialPhysics)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *UpdateMaterialPhysics )( 
            ITableGlobal * This,
            /* [in] */ BSTR pVal,
            /* [in] */ float elasticity,
            /* [in] */ float elasticityFalloff,
            /* [in] */ float friction,
            /* [in] */ float scatterAngle);
        
        DECLSPEC_XFGVIRT(ITableGlobal, GetMaterialPhysics)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetMaterialPhysics )( 
            ITableGlobal * This,
            /* [in] */ BSTR pVal,
            /* [out] */ VARIANT *elasticity,
            /* [out] */ VARIANT *elasticityFalloff,
            /* [out] */ VARIANT *friction,
            /* [out] */ VARIANT *scatterAngle);
        
        DECLSPEC_XFGVIRT(ITableGlobal, MaterialColor)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *MaterialColor )( 
            ITableGlobal * This,
            /* [in] */ BSTR name,
            /* [in] */ OLE_COLOR newVal);
        
        DECLSPEC_XFGVIRT(ITableGlobal, get_WindowWidth)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_WindowWidth )( 
            ITableGlobal * This,
            /* [retval][out] */ int *pVal);
        
        DECLSPEC_XFGVIRT(ITableGlobal, get_WindowHeight)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_WindowHeight )( 
            ITableGlobal * This,
            /* [retval][out] */ int *pVal);
        
        END_INTERFACE
    } ITableGlobalVtbl;

    interface ITableGlobal
    {
        CONST_VTBL struct ITableGlobalVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITableGlobal_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ITableGlobal_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ITableGlobal_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ITableGlobal_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define ITableGlobal_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define ITableGlobal_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define ITableGlobal_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define ITableGlobal_PlaySound(This,Sound,LoopCount,Volume,pan,randompitch,pitch,usesame,restart,front_rear_fade)	\
    ( (This)->lpVtbl -> PlaySound(This,Sound,LoopCount,Volume,pan,randompitch,pitch,usesame,restart,front_rear_fade) ) 

#define ITableGlobal_get_LeftFlipperKey(This,pVal)	\
    ( (This)->lpVtbl -> get_LeftFlipperKey(This,pVal) ) 

#define ITableGlobal_get_RightFlipperKey(This,pVal)	\
    ( (This)->lpVtbl -> get_RightFlipperKey(This,pVal) ) 

#define ITableGlobal_get_LeftTiltKey(This,pVal)	\
    ( (This)->lpVtbl -> get_LeftTiltKey(This,pVal) ) 

#define ITableGlobal_get_RightTiltKey(This,pVal)	\
    ( (This)->lpVtbl -> get_RightTiltKey(This,pVal) ) 

#define ITableGlobal_get_CenterTiltKey(This,pVal)	\
    ( (This)->lpVtbl -> get_CenterTiltKey(This,pVal) ) 

#define ITableGlobal_get_PlungerKey(This,pVal)	\
    ( (This)->lpVtbl -> get_PlungerKey(This,pVal) ) 

#define ITableGlobal_PlayMusic(This,str,Volume)	\
    ( (This)->lpVtbl -> PlayMusic(This,str,Volume) ) 

#define ITableGlobal_put_MusicVolume(This,Volume)	\
    ( (This)->lpVtbl -> put_MusicVolume(This,Volume) ) 

#define ITableGlobal_EndMusic(This)	\
    ( (This)->lpVtbl -> EndMusic(This) ) 

#define ITableGlobal_get_StartGameKey(This,pVal)	\
    ( (This)->lpVtbl -> get_StartGameKey(This,pVal) ) 

#define ITableGlobal_get_UserDirectory(This,pVal)	\
    ( (This)->lpVtbl -> get_UserDirectory(This,pVal) ) 

#define ITableGlobal_get_GetPlayerHWnd(This,pVal)	\
    ( (This)->lpVtbl -> get_GetPlayerHWnd(This,pVal) ) 

#define ITableGlobal_StopSound(This,Sound)	\
    ( (This)->lpVtbl -> StopSound(This,Sound) ) 

#define ITableGlobal_SaveValue(This,TableName,ValueName,Value)	\
    ( (This)->lpVtbl -> SaveValue(This,TableName,ValueName,Value) ) 

#define ITableGlobal_LoadValue(This,TableName,ValueName,Value)	\
    ( (This)->lpVtbl -> LoadValue(This,TableName,ValueName,Value) ) 

#define ITableGlobal_get_ActiveBall(This,pVal)	\
    ( (This)->lpVtbl -> get_ActiveBall(This,pVal) ) 

#define ITableGlobal_get_AddCreditKey(This,pVal)	\
    ( (This)->lpVtbl -> get_AddCreditKey(This,pVal) ) 

#define ITableGlobal_get_AddCreditKey2(This,pVal)	\
    ( (This)->lpVtbl -> get_AddCreditKey2(This,pVal) ) 

#define ITableGlobal_get_GameTime(This,pVal)	\
    ( (This)->lpVtbl -> get_GameTime(This,pVal) ) 

#define ITableGlobal_get_SystemTime(This,pVal)	\
    ( (This)->lpVtbl -> get_SystemTime(This,pVal) ) 

#define ITableGlobal_GetCustomParam(This,index,param)	\
    ( (This)->lpVtbl -> GetCustomParam(This,index,param) ) 

#define ITableGlobal_GetTextFile(This,FileName,pContents)	\
    ( (This)->lpVtbl -> GetTextFile(This,FileName,pContents) ) 

#define ITableGlobal_BeginModal(This)	\
    ( (This)->lpVtbl -> BeginModal(This) ) 

#define ITableGlobal_EndModal(This)	\
    ( (This)->lpVtbl -> EndModal(This) ) 

#define ITableGlobal_Nudge(This,Angle,Force)	\
    ( (This)->lpVtbl -> Nudge(This,Angle,Force) ) 

#define ITableGlobal_NudgeGetCalibration(This,XMax,YMax,XGain,YGain,DeadZone,TiltSensitivty)	\
    ( (This)->lpVtbl -> NudgeGetCalibration(This,XMax,YMax,XGain,YGain,DeadZone,TiltSensitivty) ) 

#define ITableGlobal_NudgeSetCalibration(This,XMax,YMax,XGain,YGain,DeadZone,TiltSensitivty)	\
    ( (This)->lpVtbl -> NudgeSetCalibration(This,XMax,YMax,XGain,YGain,DeadZone,TiltSensitivty) ) 

#define ITableGlobal_NudgeSensorStatus(This,XNudge,YNudge)	\
    ( (This)->lpVtbl -> NudgeSensorStatus(This,XNudge,YNudge) ) 

#define ITableGlobal_NudgeTiltStatus(This,XPlumb,YPlumb,TiltPercent)	\
    ( (This)->lpVtbl -> NudgeTiltStatus(This,XPlumb,YPlumb,TiltPercent) ) 

#define ITableGlobal_get_MechanicalTilt(This,pVal)	\
    ( (This)->lpVtbl -> get_MechanicalTilt(This,pVal) ) 

#define ITableGlobal_get_LeftMagnaSave(This,pVal)	\
    ( (This)->lpVtbl -> get_LeftMagnaSave(This,pVal) ) 

#define ITableGlobal_get_RightMagnaSave(This,pVal)	\
    ( (This)->lpVtbl -> get_RightMagnaSave(This,pVal) ) 

#define ITableGlobal_get_ExitGame(This,pVal)	\
    ( (This)->lpVtbl -> get_ExitGame(This,pVal) ) 

#define ITableGlobal_get_LockbarKey(This,pVal)	\
    ( (This)->lpVtbl -> get_LockbarKey(This,pVal) ) 

#define ITableGlobal_FireKnocker(This,Count)	\
    ( (This)->lpVtbl -> FireKnocker(This,Count) ) 

#define ITableGlobal_QuitPlayer(This,CloseType)	\
    ( (This)->lpVtbl -> QuitPlayer(This,CloseType) ) 

#define ITableGlobal_get_ShowDT(This,pVal)	\
    ( (This)->lpVtbl -> get_ShowDT(This,pVal) ) 

#define ITableGlobal_get_ShowFSS(This,pVal)	\
    ( (This)->lpVtbl -> get_ShowFSS(This,pVal) ) 

#define ITableGlobal_get_NightDay(This,pVal)	\
    ( (This)->lpVtbl -> get_NightDay(This,pVal) ) 

#define ITableGlobal_GetBalls(This,pVal)	\
    ( (This)->lpVtbl -> GetBalls(This,pVal) ) 

#define ITableGlobal_GetElements(This,pVal)	\
    ( (This)->lpVtbl -> GetElements(This,pVal) ) 

#define ITableGlobal_GetElementByName(This,name,pVal)	\
    ( (This)->lpVtbl -> GetElementByName(This,name,pVal) ) 

#define ITableGlobal_get_ActiveTable(This,pVal)	\
    ( (This)->lpVtbl -> get_ActiveTable(This,pVal) ) 

#define ITableGlobal_put_DMDWidth(This,pVal)	\
    ( (This)->lpVtbl -> put_DMDWidth(This,pVal) ) 

#define ITableGlobal_put_DMDHeight(This,pVal)	\
    ( (This)->lpVtbl -> put_DMDHeight(This,pVal) ) 

#define ITableGlobal_put_DMDPixels(This,pVal)	\
    ( (This)->lpVtbl -> put_DMDPixels(This,pVal) ) 

#define ITableGlobal_put_DMDColoredPixels(This,pVal)	\
    ( (This)->lpVtbl -> put_DMDColoredPixels(This,pVal) ) 

#define ITableGlobal_get_Version(This,pVal)	\
    ( (This)->lpVtbl -> get_Version(This,pVal) ) 

#define ITableGlobal_get_VersionMajor(This,pVal)	\
    ( (This)->lpVtbl -> get_VersionMajor(This,pVal) ) 

#define ITableGlobal_get_VersionMinor(This,pVal)	\
    ( (This)->lpVtbl -> get_VersionMinor(This,pVal) ) 

#define ITableGlobal_get_VersionRevision(This,pVal)	\
    ( (This)->lpVtbl -> get_VersionRevision(This,pVal) ) 

#define ITableGlobal_get_VPBuildVersion(This,pVal)	\
    ( (This)->lpVtbl -> get_VPBuildVersion(This,pVal) ) 

#define ITableGlobal_GetSerialDevices(This,pVal)	\
    ( (This)->lpVtbl -> GetSerialDevices(This,pVal) ) 

#define ITableGlobal_OpenSerial(This,device)	\
    ( (This)->lpVtbl -> OpenSerial(This,device) ) 

#define ITableGlobal_CloseSerial(This)	\
    ( (This)->lpVtbl -> CloseSerial(This) ) 

#define ITableGlobal_FlushSerial(This)	\
    ( (This)->lpVtbl -> FlushSerial(This) ) 

#define ITableGlobal_SetupSerial(This,baud,bits,parity,stopbit,rts,dtr)	\
    ( (This)->lpVtbl -> SetupSerial(This,baud,bits,parity,stopbit,rts,dtr) ) 

#define ITableGlobal_ReadSerial(This,size,pVal)	\
    ( (This)->lpVtbl -> ReadSerial(This,size,pVal) ) 

#define ITableGlobal_WriteSerial(This,pVal)	\
    ( (This)->lpVtbl -> WriteSerial(This,pVal) ) 

#define ITableGlobal_get_RenderingMode(This,pVal)	\
    ( (This)->lpVtbl -> get_RenderingMode(This,pVal) ) 

#define ITableGlobal_UpdateMaterial(This,pVal,wrapLighting,roughness,glossyImageLerp,thickness,edge,edgeAlpha,opacity,base,glossy,clearcoat,isMetal,opacityActive,elasticity,elasticityFalloff,friction,scatterAngle)	\
    ( (This)->lpVtbl -> UpdateMaterial(This,pVal,wrapLighting,roughness,glossyImageLerp,thickness,edge,edgeAlpha,opacity,base,glossy,clearcoat,isMetal,opacityActive,elasticity,elasticityFalloff,friction,scatterAngle) ) 

#define ITableGlobal_GetMaterial(This,pVal,wrapLighting,roughness,glossyImageLerp,thickness,edge,edgeAlpha,opacity,base,glossy,clearcoat,isMetal,opacityActive,elasticity,elasticityFalloff,friction,scatterAngle)	\
    ( (This)->lpVtbl -> GetMaterial(This,pVal,wrapLighting,roughness,glossyImageLerp,thickness,edge,edgeAlpha,opacity,base,glossy,clearcoat,isMetal,opacityActive,elasticity,elasticityFalloff,friction,scatterAngle) ) 

#define ITableGlobal_UpdateMaterialPhysics(This,pVal,elasticity,elasticityFalloff,friction,scatterAngle)	\
    ( (This)->lpVtbl -> UpdateMaterialPhysics(This,pVal,elasticity,elasticityFalloff,friction,scatterAngle) ) 

#define ITableGlobal_GetMaterialPhysics(This,pVal,elasticity,elasticityFalloff,friction,scatterAngle)	\
    ( (This)->lpVtbl -> GetMaterialPhysics(This,pVal,elasticity,elasticityFalloff,friction,scatterAngle) ) 

#define ITableGlobal_MaterialColor(This,name,newVal)	\
    ( (This)->lpVtbl -> MaterialColor(This,name,newVal) ) 

#define ITableGlobal_get_WindowWidth(This,pVal)	\
    ( (This)->lpVtbl -> get_WindowWidth(This,pVal) ) 

#define ITableGlobal_get_WindowHeight(This,pVal)	\
    ( (This)->lpVtbl -> get_WindowHeight(This,pVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ITableGlobal_INTERFACE_DEFINED__ */


#ifndef __ITableEvents_DISPINTERFACE_DEFINED__
#define __ITableEvents_DISPINTERFACE_DEFINED__

/* dispinterface ITableEvents */
/* [uuid] */ 


EXTERN_C const IID DIID_ITableEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("D3AD16E8-633B-47de-A3B8-92507F37CF65")
    ITableEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct ITableEventsVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITableEvents * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITableEvents * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITableEvents * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITableEvents * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITableEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITableEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITableEvents * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        END_INTERFACE
    } ITableEventsVtbl;

    interface ITableEvents
    {
        CONST_VTBL struct ITableEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITableEvents_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ITableEvents_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ITableEvents_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ITableEvents_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define ITableEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define ITableEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define ITableEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* __ITableEvents_DISPINTERFACE_DEFINED__ */


#ifndef __IVPDebug_INTERFACE_DEFINED__
#define __IVPDebug_INTERFACE_DEFINED__

/* interface IVPDebug */
/* [dual][uuid][object] */ 


EXTERN_C const IID IID_IVPDebug;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8668327A-610E-4041-91C4-AF69D261E381")
    IVPDebug : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Print( 
            /* [optional][in] */ VARIANT *pvar) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IVPDebugVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVPDebug * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVPDebug * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVPDebug * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IVPDebug * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IVPDebug * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IVPDebug * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IVPDebug * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        DECLSPEC_XFGVIRT(IVPDebug, Print)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Print )( 
            IVPDebug * This,
            /* [optional][in] */ VARIANT *pvar);
        
        END_INTERFACE
    } IVPDebugVtbl;

    interface IVPDebug
    {
        CONST_VTBL struct IVPDebugVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVPDebug_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IVPDebug_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IVPDebug_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IVPDebug_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IVPDebug_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IVPDebug_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IVPDebug_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IVPDebug_Print(This,pvar)	\
    ( (This)->lpVtbl -> Print(This,pvar) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IVPDebug_INTERFACE_DEFINED__ */


#ifndef __IWall_INTERFACE_DEFINED__
#define __IWall_INTERFACE_DEFINED__

/* interface IWall */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IWall;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("882F3E68-8503-4300-8AC0-91E8DF3F8D6D")
    IWall : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Name( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TimerEnabled( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TimerEnabled( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TimerInterval( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TimerInterval( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_HasHitEvent( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_HasHitEvent( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Threshold( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Threshold( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Image( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Image( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SideMaterial( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SideMaterial( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ImageAlignment( 
            /* [retval][out] */ ImageAlignment *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ImageAlignment( 
            /* [in] */ ImageAlignment newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_HeightBottom( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_HeightBottom( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_HeightTop( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_HeightTop( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TopMaterial( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TopMaterial( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CanDrop( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_CanDrop( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Collidable( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Collidable( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_IsDropped( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_IsDropped( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DisplayTexture( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_DisplayTexture( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SlingshotStrength( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SlingshotStrength( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Elasticity( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Elasticity( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ElasticityFalloff( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ElasticityFalloff( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SideImage( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SideImage( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Visible( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Visible( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Disabled( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Disabled( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SideVisible( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SideVisible( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_UserValue( 
            /* [retval][out] */ VARIANT *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_UserValue( 
            /* [in] */ VARIANT *newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SlingshotMaterial( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SlingshotMaterial( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SlingshotThreshold( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SlingshotThreshold( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SlingshotAnimation( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SlingshotAnimation( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_FlipbookAnimation( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_FlipbookAnimation( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_IsBottomSolid( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_IsBottomSolid( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DisableLighting( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_DisableLighting( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BlendDisableLighting( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_BlendDisableLighting( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BlendDisableLightingFromBelow( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_BlendDisableLightingFromBelow( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Friction( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Friction( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Scatter( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Scatter( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ReflectionEnabled( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ReflectionEnabled( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE PlaySlingshotHit( void) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_PhysicsMaterial( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_PhysicsMaterial( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_OverwritePhysics( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_OverwritePhysics( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IWallVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWall * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWall * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWall * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWall * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWall * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWall * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWall * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        DECLSPEC_XFGVIRT(IWall, get_Name)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IWall * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IWall, put_Name)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IWall * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IWall, get_TimerEnabled)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TimerEnabled )( 
            IWall * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IWall, put_TimerEnabled)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TimerEnabled )( 
            IWall * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IWall, get_TimerInterval)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TimerInterval )( 
            IWall * This,
            /* [retval][out] */ long *pVal);
        
        DECLSPEC_XFGVIRT(IWall, put_TimerInterval)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TimerInterval )( 
            IWall * This,
            /* [in] */ long newVal);
        
        DECLSPEC_XFGVIRT(IWall, get_HasHitEvent)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HasHitEvent )( 
            IWall * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IWall, put_HasHitEvent)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_HasHitEvent )( 
            IWall * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IWall, get_Threshold)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Threshold )( 
            IWall * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IWall, put_Threshold)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Threshold )( 
            IWall * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IWall, get_Image)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Image )( 
            IWall * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IWall, put_Image)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Image )( 
            IWall * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IWall, get_SideMaterial)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SideMaterial )( 
            IWall * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IWall, put_SideMaterial)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_SideMaterial )( 
            IWall * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IWall, get_ImageAlignment)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ImageAlignment )( 
            IWall * This,
            /* [retval][out] */ ImageAlignment *pVal);
        
        DECLSPEC_XFGVIRT(IWall, put_ImageAlignment)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ImageAlignment )( 
            IWall * This,
            /* [in] */ ImageAlignment newVal);
        
        DECLSPEC_XFGVIRT(IWall, get_HeightBottom)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HeightBottom )( 
            IWall * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IWall, put_HeightBottom)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_HeightBottom )( 
            IWall * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IWall, get_HeightTop)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HeightTop )( 
            IWall * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IWall, put_HeightTop)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_HeightTop )( 
            IWall * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IWall, get_TopMaterial)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TopMaterial )( 
            IWall * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IWall, put_TopMaterial)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TopMaterial )( 
            IWall * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IWall, get_CanDrop)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CanDrop )( 
            IWall * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IWall, put_CanDrop)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_CanDrop )( 
            IWall * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IWall, get_Collidable)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Collidable )( 
            IWall * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IWall, put_Collidable)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Collidable )( 
            IWall * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IWall, get_IsDropped)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_IsDropped )( 
            IWall * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IWall, put_IsDropped)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_IsDropped )( 
            IWall * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IWall, get_DisplayTexture)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_DisplayTexture )( 
            IWall * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IWall, put_DisplayTexture)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_DisplayTexture )( 
            IWall * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IWall, get_SlingshotStrength)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SlingshotStrength )( 
            IWall * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IWall, put_SlingshotStrength)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_SlingshotStrength )( 
            IWall * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IWall, get_Elasticity)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Elasticity )( 
            IWall * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IWall, put_Elasticity)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Elasticity )( 
            IWall * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IWall, get_ElasticityFalloff)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ElasticityFalloff )( 
            IWall * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IWall, put_ElasticityFalloff)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ElasticityFalloff )( 
            IWall * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IWall, get_SideImage)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SideImage )( 
            IWall * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IWall, put_SideImage)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_SideImage )( 
            IWall * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IWall, get_Visible)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Visible )( 
            IWall * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IWall, put_Visible)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Visible )( 
            IWall * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IWall, get_Disabled)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Disabled )( 
            IWall * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IWall, put_Disabled)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Disabled )( 
            IWall * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IWall, get_SideVisible)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SideVisible )( 
            IWall * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IWall, put_SideVisible)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_SideVisible )( 
            IWall * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IWall, get_UserValue)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_UserValue )( 
            IWall * This,
            /* [retval][out] */ VARIANT *pVal);
        
        DECLSPEC_XFGVIRT(IWall, put_UserValue)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_UserValue )( 
            IWall * This,
            /* [in] */ VARIANT *newVal);
        
        DECLSPEC_XFGVIRT(IWall, get_SlingshotMaterial)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SlingshotMaterial )( 
            IWall * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IWall, put_SlingshotMaterial)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_SlingshotMaterial )( 
            IWall * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IWall, get_SlingshotThreshold)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SlingshotThreshold )( 
            IWall * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IWall, put_SlingshotThreshold)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_SlingshotThreshold )( 
            IWall * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IWall, get_SlingshotAnimation)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SlingshotAnimation )( 
            IWall * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IWall, put_SlingshotAnimation)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_SlingshotAnimation )( 
            IWall * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IWall, get_FlipbookAnimation)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_FlipbookAnimation )( 
            IWall * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IWall, put_FlipbookAnimation)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_FlipbookAnimation )( 
            IWall * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IWall, get_IsBottomSolid)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_IsBottomSolid )( 
            IWall * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IWall, put_IsBottomSolid)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_IsBottomSolid )( 
            IWall * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IWall, get_DisableLighting)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_DisableLighting )( 
            IWall * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IWall, put_DisableLighting)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_DisableLighting )( 
            IWall * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IWall, get_BlendDisableLighting)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BlendDisableLighting )( 
            IWall * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IWall, put_BlendDisableLighting)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_BlendDisableLighting )( 
            IWall * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IWall, get_BlendDisableLightingFromBelow)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BlendDisableLightingFromBelow )( 
            IWall * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IWall, put_BlendDisableLightingFromBelow)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_BlendDisableLightingFromBelow )( 
            IWall * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IWall, get_Friction)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Friction )( 
            IWall * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IWall, put_Friction)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Friction )( 
            IWall * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IWall, get_Scatter)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Scatter )( 
            IWall * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IWall, put_Scatter)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Scatter )( 
            IWall * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IWall, get_ReflectionEnabled)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ReflectionEnabled )( 
            IWall * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IWall, put_ReflectionEnabled)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ReflectionEnabled )( 
            IWall * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IWall, PlaySlingshotHit)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *PlaySlingshotHit )( 
            IWall * This);
        
        DECLSPEC_XFGVIRT(IWall, get_PhysicsMaterial)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_PhysicsMaterial )( 
            IWall * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IWall, put_PhysicsMaterial)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_PhysicsMaterial )( 
            IWall * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IWall, get_OverwritePhysics)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_OverwritePhysics )( 
            IWall * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IWall, put_OverwritePhysics)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_OverwritePhysics )( 
            IWall * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        END_INTERFACE
    } IWallVtbl;

    interface IWall
    {
        CONST_VTBL struct IWallVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWall_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IWall_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IWall_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IWall_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IWall_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IWall_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IWall_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IWall_get_Name(This,pVal)	\
    ( (This)->lpVtbl -> get_Name(This,pVal) ) 

#define IWall_put_Name(This,newVal)	\
    ( (This)->lpVtbl -> put_Name(This,newVal) ) 

#define IWall_get_TimerEnabled(This,pVal)	\
    ( (This)->lpVtbl -> get_TimerEnabled(This,pVal) ) 

#define IWall_put_TimerEnabled(This,newVal)	\
    ( (This)->lpVtbl -> put_TimerEnabled(This,newVal) ) 

#define IWall_get_TimerInterval(This,pVal)	\
    ( (This)->lpVtbl -> get_TimerInterval(This,pVal) ) 

#define IWall_put_TimerInterval(This,newVal)	\
    ( (This)->lpVtbl -> put_TimerInterval(This,newVal) ) 

#define IWall_get_HasHitEvent(This,pVal)	\
    ( (This)->lpVtbl -> get_HasHitEvent(This,pVal) ) 

#define IWall_put_HasHitEvent(This,newVal)	\
    ( (This)->lpVtbl -> put_HasHitEvent(This,newVal) ) 

#define IWall_get_Threshold(This,pVal)	\
    ( (This)->lpVtbl -> get_Threshold(This,pVal) ) 

#define IWall_put_Threshold(This,newVal)	\
    ( (This)->lpVtbl -> put_Threshold(This,newVal) ) 

#define IWall_get_Image(This,pVal)	\
    ( (This)->lpVtbl -> get_Image(This,pVal) ) 

#define IWall_put_Image(This,newVal)	\
    ( (This)->lpVtbl -> put_Image(This,newVal) ) 

#define IWall_get_SideMaterial(This,pVal)	\
    ( (This)->lpVtbl -> get_SideMaterial(This,pVal) ) 

#define IWall_put_SideMaterial(This,newVal)	\
    ( (This)->lpVtbl -> put_SideMaterial(This,newVal) ) 

#define IWall_get_ImageAlignment(This,pVal)	\
    ( (This)->lpVtbl -> get_ImageAlignment(This,pVal) ) 

#define IWall_put_ImageAlignment(This,newVal)	\
    ( (This)->lpVtbl -> put_ImageAlignment(This,newVal) ) 

#define IWall_get_HeightBottom(This,pVal)	\
    ( (This)->lpVtbl -> get_HeightBottom(This,pVal) ) 

#define IWall_put_HeightBottom(This,newVal)	\
    ( (This)->lpVtbl -> put_HeightBottom(This,newVal) ) 

#define IWall_get_HeightTop(This,pVal)	\
    ( (This)->lpVtbl -> get_HeightTop(This,pVal) ) 

#define IWall_put_HeightTop(This,newVal)	\
    ( (This)->lpVtbl -> put_HeightTop(This,newVal) ) 

#define IWall_get_TopMaterial(This,pVal)	\
    ( (This)->lpVtbl -> get_TopMaterial(This,pVal) ) 

#define IWall_put_TopMaterial(This,newVal)	\
    ( (This)->lpVtbl -> put_TopMaterial(This,newVal) ) 

#define IWall_get_CanDrop(This,pVal)	\
    ( (This)->lpVtbl -> get_CanDrop(This,pVal) ) 

#define IWall_put_CanDrop(This,newVal)	\
    ( (This)->lpVtbl -> put_CanDrop(This,newVal) ) 

#define IWall_get_Collidable(This,pVal)	\
    ( (This)->lpVtbl -> get_Collidable(This,pVal) ) 

#define IWall_put_Collidable(This,newVal)	\
    ( (This)->lpVtbl -> put_Collidable(This,newVal) ) 

#define IWall_get_IsDropped(This,pVal)	\
    ( (This)->lpVtbl -> get_IsDropped(This,pVal) ) 

#define IWall_put_IsDropped(This,newVal)	\
    ( (This)->lpVtbl -> put_IsDropped(This,newVal) ) 

#define IWall_get_DisplayTexture(This,pVal)	\
    ( (This)->lpVtbl -> get_DisplayTexture(This,pVal) ) 

#define IWall_put_DisplayTexture(This,newVal)	\
    ( (This)->lpVtbl -> put_DisplayTexture(This,newVal) ) 

#define IWall_get_SlingshotStrength(This,pVal)	\
    ( (This)->lpVtbl -> get_SlingshotStrength(This,pVal) ) 

#define IWall_put_SlingshotStrength(This,newVal)	\
    ( (This)->lpVtbl -> put_SlingshotStrength(This,newVal) ) 

#define IWall_get_Elasticity(This,pVal)	\
    ( (This)->lpVtbl -> get_Elasticity(This,pVal) ) 

#define IWall_put_Elasticity(This,newVal)	\
    ( (This)->lpVtbl -> put_Elasticity(This,newVal) ) 

#define IWall_get_ElasticityFalloff(This,pVal)	\
    ( (This)->lpVtbl -> get_ElasticityFalloff(This,pVal) ) 

#define IWall_put_ElasticityFalloff(This,newVal)	\
    ( (This)->lpVtbl -> put_ElasticityFalloff(This,newVal) ) 

#define IWall_get_SideImage(This,pVal)	\
    ( (This)->lpVtbl -> get_SideImage(This,pVal) ) 

#define IWall_put_SideImage(This,newVal)	\
    ( (This)->lpVtbl -> put_SideImage(This,newVal) ) 

#define IWall_get_Visible(This,pVal)	\
    ( (This)->lpVtbl -> get_Visible(This,pVal) ) 

#define IWall_put_Visible(This,newVal)	\
    ( (This)->lpVtbl -> put_Visible(This,newVal) ) 

#define IWall_get_Disabled(This,pVal)	\
    ( (This)->lpVtbl -> get_Disabled(This,pVal) ) 

#define IWall_put_Disabled(This,newVal)	\
    ( (This)->lpVtbl -> put_Disabled(This,newVal) ) 

#define IWall_get_SideVisible(This,pVal)	\
    ( (This)->lpVtbl -> get_SideVisible(This,pVal) ) 

#define IWall_put_SideVisible(This,newVal)	\
    ( (This)->lpVtbl -> put_SideVisible(This,newVal) ) 

#define IWall_get_UserValue(This,pVal)	\
    ( (This)->lpVtbl -> get_UserValue(This,pVal) ) 

#define IWall_put_UserValue(This,newVal)	\
    ( (This)->lpVtbl -> put_UserValue(This,newVal) ) 

#define IWall_get_SlingshotMaterial(This,pVal)	\
    ( (This)->lpVtbl -> get_SlingshotMaterial(This,pVal) ) 

#define IWall_put_SlingshotMaterial(This,newVal)	\
    ( (This)->lpVtbl -> put_SlingshotMaterial(This,newVal) ) 

#define IWall_get_SlingshotThreshold(This,pVal)	\
    ( (This)->lpVtbl -> get_SlingshotThreshold(This,pVal) ) 

#define IWall_put_SlingshotThreshold(This,newVal)	\
    ( (This)->lpVtbl -> put_SlingshotThreshold(This,newVal) ) 

#define IWall_get_SlingshotAnimation(This,pVal)	\
    ( (This)->lpVtbl -> get_SlingshotAnimation(This,pVal) ) 

#define IWall_put_SlingshotAnimation(This,newVal)	\
    ( (This)->lpVtbl -> put_SlingshotAnimation(This,newVal) ) 

#define IWall_get_FlipbookAnimation(This,pVal)	\
    ( (This)->lpVtbl -> get_FlipbookAnimation(This,pVal) ) 

#define IWall_put_FlipbookAnimation(This,newVal)	\
    ( (This)->lpVtbl -> put_FlipbookAnimation(This,newVal) ) 

#define IWall_get_IsBottomSolid(This,pVal)	\
    ( (This)->lpVtbl -> get_IsBottomSolid(This,pVal) ) 

#define IWall_put_IsBottomSolid(This,newVal)	\
    ( (This)->lpVtbl -> put_IsBottomSolid(This,newVal) ) 

#define IWall_get_DisableLighting(This,pVal)	\
    ( (This)->lpVtbl -> get_DisableLighting(This,pVal) ) 

#define IWall_put_DisableLighting(This,newVal)	\
    ( (This)->lpVtbl -> put_DisableLighting(This,newVal) ) 

#define IWall_get_BlendDisableLighting(This,pVal)	\
    ( (This)->lpVtbl -> get_BlendDisableLighting(This,pVal) ) 

#define IWall_put_BlendDisableLighting(This,newVal)	\
    ( (This)->lpVtbl -> put_BlendDisableLighting(This,newVal) ) 

#define IWall_get_BlendDisableLightingFromBelow(This,pVal)	\
    ( (This)->lpVtbl -> get_BlendDisableLightingFromBelow(This,pVal) ) 

#define IWall_put_BlendDisableLightingFromBelow(This,newVal)	\
    ( (This)->lpVtbl -> put_BlendDisableLightingFromBelow(This,newVal) ) 

#define IWall_get_Friction(This,pVal)	\
    ( (This)->lpVtbl -> get_Friction(This,pVal) ) 

#define IWall_put_Friction(This,newVal)	\
    ( (This)->lpVtbl -> put_Friction(This,newVal) ) 

#define IWall_get_Scatter(This,pVal)	\
    ( (This)->lpVtbl -> get_Scatter(This,pVal) ) 

#define IWall_put_Scatter(This,newVal)	\
    ( (This)->lpVtbl -> put_Scatter(This,newVal) ) 

#define IWall_get_ReflectionEnabled(This,pVal)	\
    ( (This)->lpVtbl -> get_ReflectionEnabled(This,pVal) ) 

#define IWall_put_ReflectionEnabled(This,newVal)	\
    ( (This)->lpVtbl -> put_ReflectionEnabled(This,newVal) ) 

#define IWall_PlaySlingshotHit(This)	\
    ( (This)->lpVtbl -> PlaySlingshotHit(This) ) 

#define IWall_get_PhysicsMaterial(This,pVal)	\
    ( (This)->lpVtbl -> get_PhysicsMaterial(This,pVal) ) 

#define IWall_put_PhysicsMaterial(This,newVal)	\
    ( (This)->lpVtbl -> put_PhysicsMaterial(This,newVal) ) 

#define IWall_get_OverwritePhysics(This,pVal)	\
    ( (This)->lpVtbl -> get_OverwritePhysics(This,pVal) ) 

#define IWall_put_OverwritePhysics(This,newVal)	\
    ( (This)->lpVtbl -> put_OverwritePhysics(This,newVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IWall_INTERFACE_DEFINED__ */


#ifndef __IWallEvents_DISPINTERFACE_DEFINED__
#define __IWallEvents_DISPINTERFACE_DEFINED__

/* dispinterface IWallEvents */
/* [uuid] */ 


EXTERN_C const IID DIID_IWallEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("1128A15E-1388-450f-A517-2819808CF7CF")
    IWallEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct IWallEventsVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWallEvents * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWallEvents * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWallEvents * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWallEvents * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWallEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWallEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWallEvents * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        END_INTERFACE
    } IWallEventsVtbl;

    interface IWallEvents
    {
        CONST_VTBL struct IWallEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWallEvents_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IWallEvents_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IWallEvents_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IWallEvents_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IWallEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IWallEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IWallEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* __IWallEvents_DISPINTERFACE_DEFINED__ */


#ifndef __IControlPoint_INTERFACE_DEFINED__
#define __IControlPoint_INTERFACE_DEFINED__

/* interface IControlPoint */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IControlPoint;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("353B4F85-BABC-4FEB-B664-91CE3DB3C0FA")
    IControlPoint : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_X( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_X( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Y( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Y( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Z( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Z( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CalcHeight( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Smooth( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Smooth( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_IsAutoTextureCoordinate( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_IsAutoTextureCoordinate( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TextureCoordinateU( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TextureCoordinateU( 
            /* [in] */ float newVal) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IControlPointVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IControlPoint * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IControlPoint * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IControlPoint * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IControlPoint * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IControlPoint * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IControlPoint * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IControlPoint * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        DECLSPEC_XFGVIRT(IControlPoint, get_X)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_X )( 
            IControlPoint * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IControlPoint, put_X)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_X )( 
            IControlPoint * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IControlPoint, get_Y)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Y )( 
            IControlPoint * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IControlPoint, put_Y)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Y )( 
            IControlPoint * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IControlPoint, get_Z)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Z )( 
            IControlPoint * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IControlPoint, put_Z)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Z )( 
            IControlPoint * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IControlPoint, get_CalcHeight)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CalcHeight )( 
            IControlPoint * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IControlPoint, get_Smooth)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Smooth )( 
            IControlPoint * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IControlPoint, put_Smooth)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Smooth )( 
            IControlPoint * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IControlPoint, get_IsAutoTextureCoordinate)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_IsAutoTextureCoordinate )( 
            IControlPoint * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IControlPoint, put_IsAutoTextureCoordinate)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_IsAutoTextureCoordinate )( 
            IControlPoint * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IControlPoint, get_TextureCoordinateU)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TextureCoordinateU )( 
            IControlPoint * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IControlPoint, put_TextureCoordinateU)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TextureCoordinateU )( 
            IControlPoint * This,
            /* [in] */ float newVal);
        
        END_INTERFACE
    } IControlPointVtbl;

    interface IControlPoint
    {
        CONST_VTBL struct IControlPointVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IControlPoint_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IControlPoint_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IControlPoint_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IControlPoint_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IControlPoint_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IControlPoint_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IControlPoint_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IControlPoint_get_X(This,pVal)	\
    ( (This)->lpVtbl -> get_X(This,pVal) ) 

#define IControlPoint_put_X(This,newVal)	\
    ( (This)->lpVtbl -> put_X(This,newVal) ) 

#define IControlPoint_get_Y(This,pVal)	\
    ( (This)->lpVtbl -> get_Y(This,pVal) ) 

#define IControlPoint_put_Y(This,newVal)	\
    ( (This)->lpVtbl -> put_Y(This,newVal) ) 

#define IControlPoint_get_Z(This,pVal)	\
    ( (This)->lpVtbl -> get_Z(This,pVal) ) 

#define IControlPoint_put_Z(This,newVal)	\
    ( (This)->lpVtbl -> put_Z(This,newVal) ) 

#define IControlPoint_get_CalcHeight(This,pVal)	\
    ( (This)->lpVtbl -> get_CalcHeight(This,pVal) ) 

#define IControlPoint_get_Smooth(This,pVal)	\
    ( (This)->lpVtbl -> get_Smooth(This,pVal) ) 

#define IControlPoint_put_Smooth(This,newVal)	\
    ( (This)->lpVtbl -> put_Smooth(This,newVal) ) 

#define IControlPoint_get_IsAutoTextureCoordinate(This,pVal)	\
    ( (This)->lpVtbl -> get_IsAutoTextureCoordinate(This,pVal) ) 

#define IControlPoint_put_IsAutoTextureCoordinate(This,newVal)	\
    ( (This)->lpVtbl -> put_IsAutoTextureCoordinate(This,newVal) ) 

#define IControlPoint_get_TextureCoordinateU(This,pVal)	\
    ( (This)->lpVtbl -> get_TextureCoordinateU(This,pVal) ) 

#define IControlPoint_put_TextureCoordinateU(This,newVal)	\
    ( (This)->lpVtbl -> put_TextureCoordinateU(This,newVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IControlPoint_INTERFACE_DEFINED__ */


#ifndef __IFlipper_INTERFACE_DEFINED__
#define __IFlipper_INTERFACE_DEFINED__

/* interface IFlipper */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IFlipper;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B4957EA2-33BE-45E6-93C2-CA16B8755BF3")
    IFlipper : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BaseRadius( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_BaseRadius( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_EndRadius( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_EndRadius( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Length( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Length( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_StartAngle( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_StartAngle( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE RotateToEnd( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE RotateToStart( void) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_EndAngle( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_EndAngle( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TimerEnabled( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TimerEnabled( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TimerInterval( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TimerInterval( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][nonbrowsable][id][propget] */ HRESULT STDMETHODCALLTYPE get_CurrentAngle( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_X( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_X( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Y( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Y( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Material( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Material( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Mass( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Mass( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_OverridePhysics( 
            /* [retval][out] */ PhysicsSet *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_OverridePhysics( 
            /* [in] */ PhysicsSet newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RubberMaterial( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_RubberMaterial( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RubberThickness( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_RubberThickness( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Strength( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Strength( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Visible( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Visible( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Enabled( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Enabled( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Elasticity( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Elasticity( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Scatter( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Scatter( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Return( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Return( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RubberHeight( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_RubberHeight( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RubberWidth( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_RubberWidth( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Friction( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Friction( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RampUp( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_RampUp( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ElasticityFalloff( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ElasticityFalloff( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Surface( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Surface( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Name( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_UserValue( 
            /* [retval][out] */ VARIANT *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_UserValue( 
            /* [in] */ VARIANT *newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Height( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Height( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_EOSTorque( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_EOSTorque( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_EOSTorqueAngle( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_EOSTorqueAngle( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_FlipperRadiusMin( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_FlipperRadiusMin( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Image( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Image( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ReflectionEnabled( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ReflectionEnabled( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IFlipperVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFlipper * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFlipper * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFlipper * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFlipper * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFlipper * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFlipper * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFlipper * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        DECLSPEC_XFGVIRT(IFlipper, get_BaseRadius)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BaseRadius )( 
            IFlipper * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IFlipper, put_BaseRadius)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_BaseRadius )( 
            IFlipper * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IFlipper, get_EndRadius)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_EndRadius )( 
            IFlipper * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IFlipper, put_EndRadius)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_EndRadius )( 
            IFlipper * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IFlipper, get_Length)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Length )( 
            IFlipper * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IFlipper, put_Length)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Length )( 
            IFlipper * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IFlipper, get_StartAngle)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_StartAngle )( 
            IFlipper * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IFlipper, put_StartAngle)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_StartAngle )( 
            IFlipper * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IFlipper, RotateToEnd)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *RotateToEnd )( 
            IFlipper * This);
        
        DECLSPEC_XFGVIRT(IFlipper, RotateToStart)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *RotateToStart )( 
            IFlipper * This);
        
        DECLSPEC_XFGVIRT(IFlipper, get_EndAngle)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_EndAngle )( 
            IFlipper * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IFlipper, put_EndAngle)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_EndAngle )( 
            IFlipper * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IFlipper, get_TimerEnabled)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TimerEnabled )( 
            IFlipper * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IFlipper, put_TimerEnabled)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TimerEnabled )( 
            IFlipper * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IFlipper, get_TimerInterval)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TimerInterval )( 
            IFlipper * This,
            /* [retval][out] */ long *pVal);
        
        DECLSPEC_XFGVIRT(IFlipper, put_TimerInterval)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TimerInterval )( 
            IFlipper * This,
            /* [in] */ long newVal);
        
        DECLSPEC_XFGVIRT(IFlipper, get_CurrentAngle)
        /* [helpstring][nonbrowsable][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CurrentAngle )( 
            IFlipper * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IFlipper, get_X)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_X )( 
            IFlipper * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IFlipper, put_X)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_X )( 
            IFlipper * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IFlipper, get_Y)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Y )( 
            IFlipper * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IFlipper, put_Y)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Y )( 
            IFlipper * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IFlipper, get_Material)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Material )( 
            IFlipper * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IFlipper, put_Material)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Material )( 
            IFlipper * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IFlipper, get_Mass)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Mass )( 
            IFlipper * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IFlipper, put_Mass)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Mass )( 
            IFlipper * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IFlipper, get_OverridePhysics)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_OverridePhysics )( 
            IFlipper * This,
            /* [retval][out] */ PhysicsSet *pVal);
        
        DECLSPEC_XFGVIRT(IFlipper, put_OverridePhysics)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_OverridePhysics )( 
            IFlipper * This,
            /* [in] */ PhysicsSet newVal);
        
        DECLSPEC_XFGVIRT(IFlipper, get_RubberMaterial)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RubberMaterial )( 
            IFlipper * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IFlipper, put_RubberMaterial)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_RubberMaterial )( 
            IFlipper * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IFlipper, get_RubberThickness)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RubberThickness )( 
            IFlipper * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IFlipper, put_RubberThickness)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_RubberThickness )( 
            IFlipper * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IFlipper, get_Strength)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Strength )( 
            IFlipper * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IFlipper, put_Strength)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Strength )( 
            IFlipper * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IFlipper, get_Visible)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Visible )( 
            IFlipper * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IFlipper, put_Visible)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Visible )( 
            IFlipper * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IFlipper, get_Enabled)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Enabled )( 
            IFlipper * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IFlipper, put_Enabled)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Enabled )( 
            IFlipper * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IFlipper, get_Elasticity)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Elasticity )( 
            IFlipper * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IFlipper, put_Elasticity)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Elasticity )( 
            IFlipper * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IFlipper, get_Scatter)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Scatter )( 
            IFlipper * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IFlipper, put_Scatter)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Scatter )( 
            IFlipper * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IFlipper, get_Return)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Return )( 
            IFlipper * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IFlipper, put_Return)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Return )( 
            IFlipper * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IFlipper, get_RubberHeight)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RubberHeight )( 
            IFlipper * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IFlipper, put_RubberHeight)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_RubberHeight )( 
            IFlipper * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IFlipper, get_RubberWidth)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RubberWidth )( 
            IFlipper * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IFlipper, put_RubberWidth)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_RubberWidth )( 
            IFlipper * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IFlipper, get_Friction)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Friction )( 
            IFlipper * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IFlipper, put_Friction)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Friction )( 
            IFlipper * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IFlipper, get_RampUp)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RampUp )( 
            IFlipper * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IFlipper, put_RampUp)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_RampUp )( 
            IFlipper * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IFlipper, get_ElasticityFalloff)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ElasticityFalloff )( 
            IFlipper * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IFlipper, put_ElasticityFalloff)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ElasticityFalloff )( 
            IFlipper * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IFlipper, get_Surface)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Surface )( 
            IFlipper * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IFlipper, put_Surface)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Surface )( 
            IFlipper * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IFlipper, get_Name)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IFlipper * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IFlipper, put_Name)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IFlipper * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IFlipper, get_UserValue)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_UserValue )( 
            IFlipper * This,
            /* [retval][out] */ VARIANT *pVal);
        
        DECLSPEC_XFGVIRT(IFlipper, put_UserValue)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_UserValue )( 
            IFlipper * This,
            /* [in] */ VARIANT *newVal);
        
        DECLSPEC_XFGVIRT(IFlipper, get_Height)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Height )( 
            IFlipper * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IFlipper, put_Height)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Height )( 
            IFlipper * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IFlipper, get_EOSTorque)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_EOSTorque )( 
            IFlipper * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IFlipper, put_EOSTorque)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_EOSTorque )( 
            IFlipper * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IFlipper, get_EOSTorqueAngle)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_EOSTorqueAngle )( 
            IFlipper * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IFlipper, put_EOSTorqueAngle)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_EOSTorqueAngle )( 
            IFlipper * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IFlipper, get_FlipperRadiusMin)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_FlipperRadiusMin )( 
            IFlipper * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IFlipper, put_FlipperRadiusMin)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_FlipperRadiusMin )( 
            IFlipper * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IFlipper, get_Image)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Image )( 
            IFlipper * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IFlipper, put_Image)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Image )( 
            IFlipper * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IFlipper, get_ReflectionEnabled)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ReflectionEnabled )( 
            IFlipper * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IFlipper, put_ReflectionEnabled)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ReflectionEnabled )( 
            IFlipper * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        END_INTERFACE
    } IFlipperVtbl;

    interface IFlipper
    {
        CONST_VTBL struct IFlipperVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFlipper_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IFlipper_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IFlipper_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IFlipper_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IFlipper_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IFlipper_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IFlipper_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IFlipper_get_BaseRadius(This,pVal)	\
    ( (This)->lpVtbl -> get_BaseRadius(This,pVal) ) 

#define IFlipper_put_BaseRadius(This,newVal)	\
    ( (This)->lpVtbl -> put_BaseRadius(This,newVal) ) 

#define IFlipper_get_EndRadius(This,pVal)	\
    ( (This)->lpVtbl -> get_EndRadius(This,pVal) ) 

#define IFlipper_put_EndRadius(This,newVal)	\
    ( (This)->lpVtbl -> put_EndRadius(This,newVal) ) 

#define IFlipper_get_Length(This,pVal)	\
    ( (This)->lpVtbl -> get_Length(This,pVal) ) 

#define IFlipper_put_Length(This,newVal)	\
    ( (This)->lpVtbl -> put_Length(This,newVal) ) 

#define IFlipper_get_StartAngle(This,pVal)	\
    ( (This)->lpVtbl -> get_StartAngle(This,pVal) ) 

#define IFlipper_put_StartAngle(This,newVal)	\
    ( (This)->lpVtbl -> put_StartAngle(This,newVal) ) 

#define IFlipper_RotateToEnd(This)	\
    ( (This)->lpVtbl -> RotateToEnd(This) ) 

#define IFlipper_RotateToStart(This)	\
    ( (This)->lpVtbl -> RotateToStart(This) ) 

#define IFlipper_get_EndAngle(This,pVal)	\
    ( (This)->lpVtbl -> get_EndAngle(This,pVal) ) 

#define IFlipper_put_EndAngle(This,newVal)	\
    ( (This)->lpVtbl -> put_EndAngle(This,newVal) ) 

#define IFlipper_get_TimerEnabled(This,pVal)	\
    ( (This)->lpVtbl -> get_TimerEnabled(This,pVal) ) 

#define IFlipper_put_TimerEnabled(This,newVal)	\
    ( (This)->lpVtbl -> put_TimerEnabled(This,newVal) ) 

#define IFlipper_get_TimerInterval(This,pVal)	\
    ( (This)->lpVtbl -> get_TimerInterval(This,pVal) ) 

#define IFlipper_put_TimerInterval(This,newVal)	\
    ( (This)->lpVtbl -> put_TimerInterval(This,newVal) ) 

#define IFlipper_get_CurrentAngle(This,pVal)	\
    ( (This)->lpVtbl -> get_CurrentAngle(This,pVal) ) 

#define IFlipper_get_X(This,pVal)	\
    ( (This)->lpVtbl -> get_X(This,pVal) ) 

#define IFlipper_put_X(This,newVal)	\
    ( (This)->lpVtbl -> put_X(This,newVal) ) 

#define IFlipper_get_Y(This,pVal)	\
    ( (This)->lpVtbl -> get_Y(This,pVal) ) 

#define IFlipper_put_Y(This,newVal)	\
    ( (This)->lpVtbl -> put_Y(This,newVal) ) 

#define IFlipper_get_Material(This,pVal)	\
    ( (This)->lpVtbl -> get_Material(This,pVal) ) 

#define IFlipper_put_Material(This,newVal)	\
    ( (This)->lpVtbl -> put_Material(This,newVal) ) 

#define IFlipper_get_Mass(This,pVal)	\
    ( (This)->lpVtbl -> get_Mass(This,pVal) ) 

#define IFlipper_put_Mass(This,newVal)	\
    ( (This)->lpVtbl -> put_Mass(This,newVal) ) 

#define IFlipper_get_OverridePhysics(This,pVal)	\
    ( (This)->lpVtbl -> get_OverridePhysics(This,pVal) ) 

#define IFlipper_put_OverridePhysics(This,newVal)	\
    ( (This)->lpVtbl -> put_OverridePhysics(This,newVal) ) 

#define IFlipper_get_RubberMaterial(This,pVal)	\
    ( (This)->lpVtbl -> get_RubberMaterial(This,pVal) ) 

#define IFlipper_put_RubberMaterial(This,newVal)	\
    ( (This)->lpVtbl -> put_RubberMaterial(This,newVal) ) 

#define IFlipper_get_RubberThickness(This,pVal)	\
    ( (This)->lpVtbl -> get_RubberThickness(This,pVal) ) 

#define IFlipper_put_RubberThickness(This,newVal)	\
    ( (This)->lpVtbl -> put_RubberThickness(This,newVal) ) 

#define IFlipper_get_Strength(This,pVal)	\
    ( (This)->lpVtbl -> get_Strength(This,pVal) ) 

#define IFlipper_put_Strength(This,newVal)	\
    ( (This)->lpVtbl -> put_Strength(This,newVal) ) 

#define IFlipper_get_Visible(This,pVal)	\
    ( (This)->lpVtbl -> get_Visible(This,pVal) ) 

#define IFlipper_put_Visible(This,newVal)	\
    ( (This)->lpVtbl -> put_Visible(This,newVal) ) 

#define IFlipper_get_Enabled(This,pVal)	\
    ( (This)->lpVtbl -> get_Enabled(This,pVal) ) 

#define IFlipper_put_Enabled(This,newVal)	\
    ( (This)->lpVtbl -> put_Enabled(This,newVal) ) 

#define IFlipper_get_Elasticity(This,pVal)	\
    ( (This)->lpVtbl -> get_Elasticity(This,pVal) ) 

#define IFlipper_put_Elasticity(This,newVal)	\
    ( (This)->lpVtbl -> put_Elasticity(This,newVal) ) 

#define IFlipper_get_Scatter(This,pVal)	\
    ( (This)->lpVtbl -> get_Scatter(This,pVal) ) 

#define IFlipper_put_Scatter(This,newVal)	\
    ( (This)->lpVtbl -> put_Scatter(This,newVal) ) 

#define IFlipper_get_Return(This,pVal)	\
    ( (This)->lpVtbl -> get_Return(This,pVal) ) 

#define IFlipper_put_Return(This,newVal)	\
    ( (This)->lpVtbl -> put_Return(This,newVal) ) 

#define IFlipper_get_RubberHeight(This,pVal)	\
    ( (This)->lpVtbl -> get_RubberHeight(This,pVal) ) 

#define IFlipper_put_RubberHeight(This,newVal)	\
    ( (This)->lpVtbl -> put_RubberHeight(This,newVal) ) 

#define IFlipper_get_RubberWidth(This,pVal)	\
    ( (This)->lpVtbl -> get_RubberWidth(This,pVal) ) 

#define IFlipper_put_RubberWidth(This,newVal)	\
    ( (This)->lpVtbl -> put_RubberWidth(This,newVal) ) 

#define IFlipper_get_Friction(This,pVal)	\
    ( (This)->lpVtbl -> get_Friction(This,pVal) ) 

#define IFlipper_put_Friction(This,newVal)	\
    ( (This)->lpVtbl -> put_Friction(This,newVal) ) 

#define IFlipper_get_RampUp(This,pVal)	\
    ( (This)->lpVtbl -> get_RampUp(This,pVal) ) 

#define IFlipper_put_RampUp(This,newVal)	\
    ( (This)->lpVtbl -> put_RampUp(This,newVal) ) 

#define IFlipper_get_ElasticityFalloff(This,pVal)	\
    ( (This)->lpVtbl -> get_ElasticityFalloff(This,pVal) ) 

#define IFlipper_put_ElasticityFalloff(This,newVal)	\
    ( (This)->lpVtbl -> put_ElasticityFalloff(This,newVal) ) 

#define IFlipper_get_Surface(This,pVal)	\
    ( (This)->lpVtbl -> get_Surface(This,pVal) ) 

#define IFlipper_put_Surface(This,newVal)	\
    ( (This)->lpVtbl -> put_Surface(This,newVal) ) 

#define IFlipper_get_Name(This,pVal)	\
    ( (This)->lpVtbl -> get_Name(This,pVal) ) 

#define IFlipper_put_Name(This,newVal)	\
    ( (This)->lpVtbl -> put_Name(This,newVal) ) 

#define IFlipper_get_UserValue(This,pVal)	\
    ( (This)->lpVtbl -> get_UserValue(This,pVal) ) 

#define IFlipper_put_UserValue(This,newVal)	\
    ( (This)->lpVtbl -> put_UserValue(This,newVal) ) 

#define IFlipper_get_Height(This,pVal)	\
    ( (This)->lpVtbl -> get_Height(This,pVal) ) 

#define IFlipper_put_Height(This,newVal)	\
    ( (This)->lpVtbl -> put_Height(This,newVal) ) 

#define IFlipper_get_EOSTorque(This,pVal)	\
    ( (This)->lpVtbl -> get_EOSTorque(This,pVal) ) 

#define IFlipper_put_EOSTorque(This,newVal)	\
    ( (This)->lpVtbl -> put_EOSTorque(This,newVal) ) 

#define IFlipper_get_EOSTorqueAngle(This,pVal)	\
    ( (This)->lpVtbl -> get_EOSTorqueAngle(This,pVal) ) 

#define IFlipper_put_EOSTorqueAngle(This,newVal)	\
    ( (This)->lpVtbl -> put_EOSTorqueAngle(This,newVal) ) 

#define IFlipper_get_FlipperRadiusMin(This,pVal)	\
    ( (This)->lpVtbl -> get_FlipperRadiusMin(This,pVal) ) 

#define IFlipper_put_FlipperRadiusMin(This,newVal)	\
    ( (This)->lpVtbl -> put_FlipperRadiusMin(This,newVal) ) 

#define IFlipper_get_Image(This,pVal)	\
    ( (This)->lpVtbl -> get_Image(This,pVal) ) 

#define IFlipper_put_Image(This,newVal)	\
    ( (This)->lpVtbl -> put_Image(This,newVal) ) 

#define IFlipper_get_ReflectionEnabled(This,pVal)	\
    ( (This)->lpVtbl -> get_ReflectionEnabled(This,pVal) ) 

#define IFlipper_put_ReflectionEnabled(This,newVal)	\
    ( (This)->lpVtbl -> put_ReflectionEnabled(This,newVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IFlipper_INTERFACE_DEFINED__ */


#ifndef __IFlipperEvents_DISPINTERFACE_DEFINED__
#define __IFlipperEvents_DISPINTERFACE_DEFINED__

/* dispinterface IFlipperEvents */
/* [uuid] */ 


EXTERN_C const IID DIID_IFlipperEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("C1001167-49EE-4d66-B9F4-A1623847510A")
    IFlipperEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct IFlipperEventsVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFlipperEvents * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFlipperEvents * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFlipperEvents * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFlipperEvents * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFlipperEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFlipperEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFlipperEvents * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        END_INTERFACE
    } IFlipperEventsVtbl;

    interface IFlipperEvents
    {
        CONST_VTBL struct IFlipperEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFlipperEvents_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IFlipperEvents_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IFlipperEvents_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IFlipperEvents_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IFlipperEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IFlipperEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IFlipperEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* __IFlipperEvents_DISPINTERFACE_DEFINED__ */


#ifndef __ITimer_INTERFACE_DEFINED__
#define __ITimer_INTERFACE_DEFINED__

/* interface ITimer */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_ITimer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C74673FA-0F67-49F0-8469-18ADDAF52355")
    ITimer : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Enabled( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Enabled( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Interval( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Interval( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Name( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_UserValue( 
            /* [retval][out] */ VARIANT *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_UserValue( 
            /* [in] */ VARIANT *newVal) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct ITimerVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITimer * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITimer * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITimer * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITimer * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITimer * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITimer * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITimer * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        DECLSPEC_XFGVIRT(ITimer, get_Enabled)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Enabled )( 
            ITimer * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(ITimer, put_Enabled)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Enabled )( 
            ITimer * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(ITimer, get_Interval)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Interval )( 
            ITimer * This,
            /* [retval][out] */ long *pVal);
        
        DECLSPEC_XFGVIRT(ITimer, put_Interval)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Interval )( 
            ITimer * This,
            /* [in] */ long newVal);
        
        DECLSPEC_XFGVIRT(ITimer, get_Name)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ITimer * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(ITimer, put_Name)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            ITimer * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(ITimer, get_UserValue)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_UserValue )( 
            ITimer * This,
            /* [retval][out] */ VARIANT *pVal);
        
        DECLSPEC_XFGVIRT(ITimer, put_UserValue)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_UserValue )( 
            ITimer * This,
            /* [in] */ VARIANT *newVal);
        
        END_INTERFACE
    } ITimerVtbl;

    interface ITimer
    {
        CONST_VTBL struct ITimerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITimer_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ITimer_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ITimer_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ITimer_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define ITimer_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define ITimer_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define ITimer_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define ITimer_get_Enabled(This,pVal)	\
    ( (This)->lpVtbl -> get_Enabled(This,pVal) ) 

#define ITimer_put_Enabled(This,newVal)	\
    ( (This)->lpVtbl -> put_Enabled(This,newVal) ) 

#define ITimer_get_Interval(This,pVal)	\
    ( (This)->lpVtbl -> get_Interval(This,pVal) ) 

#define ITimer_put_Interval(This,newVal)	\
    ( (This)->lpVtbl -> put_Interval(This,newVal) ) 

#define ITimer_get_Name(This,pVal)	\
    ( (This)->lpVtbl -> get_Name(This,pVal) ) 

#define ITimer_put_Name(This,newVal)	\
    ( (This)->lpVtbl -> put_Name(This,newVal) ) 

#define ITimer_get_UserValue(This,pVal)	\
    ( (This)->lpVtbl -> get_UserValue(This,pVal) ) 

#define ITimer_put_UserValue(This,newVal)	\
    ( (This)->lpVtbl -> put_UserValue(This,newVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ITimer_INTERFACE_DEFINED__ */


#ifndef __ITimerEvents_DISPINTERFACE_DEFINED__
#define __ITimerEvents_DISPINTERFACE_DEFINED__

/* dispinterface ITimerEvents */
/* [uuid] */ 


EXTERN_C const IID DIID_ITimerEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("DC13DC5B-FF29-4ef4-93E1-06C4B8603AEF")
    ITimerEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct ITimerEventsVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITimerEvents * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITimerEvents * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITimerEvents * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITimerEvents * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITimerEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITimerEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITimerEvents * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        END_INTERFACE
    } ITimerEventsVtbl;

    interface ITimerEvents
    {
        CONST_VTBL struct ITimerEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITimerEvents_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ITimerEvents_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ITimerEvents_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ITimerEvents_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define ITimerEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define ITimerEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define ITimerEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* __ITimerEvents_DISPINTERFACE_DEFINED__ */


#ifndef __IPlunger_INTERFACE_DEFINED__
#define __IPlunger_INTERFACE_DEFINED__

/* interface IPlunger */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IPlunger;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3B0BF779-B5D2-4445-B804-EF1E9890FE98")
    IPlunger : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE PullBack( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Fire( void) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_PullSpeed( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_PullSpeed( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_FireSpeed( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_FireSpeed( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE CreateBall( 
            /* [retval][out] */ IBall **Ball) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Position( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE PullBackandRetract( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE MotionDevice( 
            /* [retval][out] */ int *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TimerEnabled( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TimerEnabled( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TimerInterval( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TimerInterval( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_X( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_X( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Y( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Y( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Width( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Width( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ZAdjust( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ZAdjust( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Surface( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Surface( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Name( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_UserValue( 
            /* [retval][out] */ VARIANT *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_UserValue( 
            /* [in] */ VARIANT *newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Type( 
            /* [retval][out] */ PlungerType *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Type( 
            /* [in] */ PlungerType newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Material( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Material( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Image( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Image( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AnimFrames( 
            /* [retval][out] */ int *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_AnimFrames( 
            /* [in] */ int newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TipShape( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TipShape( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RodDiam( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_RodDiam( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RingGap( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_RingGap( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RingDiam( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_RingDiam( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RingWidth( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_RingWidth( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SpringDiam( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SpringDiam( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SpringGauge( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SpringGauge( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SpringLoops( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SpringLoops( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SpringEndLoops( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SpringEndLoops( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_MechPlunger( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_MechPlunger( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AutoPlunger( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_AutoPlunger( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Visible( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Visible( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_MechStrength( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_MechStrength( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ParkPosition( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ParkPosition( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Stroke( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Stroke( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ScatterVelocity( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ScatterVelocity( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_MomentumXfer( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_MomentumXfer( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ReflectionEnabled( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ReflectionEnabled( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IPlungerVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPlunger * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPlunger * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPlunger * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IPlunger * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IPlunger * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IPlunger * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IPlunger * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        DECLSPEC_XFGVIRT(IPlunger, PullBack)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *PullBack )( 
            IPlunger * This);
        
        DECLSPEC_XFGVIRT(IPlunger, Fire)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Fire )( 
            IPlunger * This);
        
        DECLSPEC_XFGVIRT(IPlunger, get_PullSpeed)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_PullSpeed )( 
            IPlunger * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPlunger, put_PullSpeed)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_PullSpeed )( 
            IPlunger * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPlunger, get_FireSpeed)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_FireSpeed )( 
            IPlunger * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPlunger, put_FireSpeed)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_FireSpeed )( 
            IPlunger * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPlunger, CreateBall)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *CreateBall )( 
            IPlunger * This,
            /* [retval][out] */ IBall **Ball);
        
        DECLSPEC_XFGVIRT(IPlunger, Position)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Position )( 
            IPlunger * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPlunger, PullBackandRetract)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *PullBackandRetract )( 
            IPlunger * This);
        
        DECLSPEC_XFGVIRT(IPlunger, MotionDevice)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *MotionDevice )( 
            IPlunger * This,
            /* [retval][out] */ int *pVal);
        
        DECLSPEC_XFGVIRT(IPlunger, get_TimerEnabled)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TimerEnabled )( 
            IPlunger * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IPlunger, put_TimerEnabled)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TimerEnabled )( 
            IPlunger * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IPlunger, get_TimerInterval)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TimerInterval )( 
            IPlunger * This,
            /* [retval][out] */ long *pVal);
        
        DECLSPEC_XFGVIRT(IPlunger, put_TimerInterval)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TimerInterval )( 
            IPlunger * This,
            /* [in] */ long newVal);
        
        DECLSPEC_XFGVIRT(IPlunger, get_X)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_X )( 
            IPlunger * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPlunger, put_X)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_X )( 
            IPlunger * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPlunger, get_Y)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Y )( 
            IPlunger * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPlunger, put_Y)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Y )( 
            IPlunger * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPlunger, get_Width)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Width )( 
            IPlunger * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPlunger, put_Width)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Width )( 
            IPlunger * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPlunger, get_ZAdjust)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ZAdjust )( 
            IPlunger * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPlunger, put_ZAdjust)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ZAdjust )( 
            IPlunger * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPlunger, get_Surface)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Surface )( 
            IPlunger * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IPlunger, put_Surface)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Surface )( 
            IPlunger * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IPlunger, get_Name)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IPlunger * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IPlunger, put_Name)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IPlunger * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IPlunger, get_UserValue)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_UserValue )( 
            IPlunger * This,
            /* [retval][out] */ VARIANT *pVal);
        
        DECLSPEC_XFGVIRT(IPlunger, put_UserValue)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_UserValue )( 
            IPlunger * This,
            /* [in] */ VARIANT *newVal);
        
        DECLSPEC_XFGVIRT(IPlunger, get_Type)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            IPlunger * This,
            /* [retval][out] */ PlungerType *pVal);
        
        DECLSPEC_XFGVIRT(IPlunger, put_Type)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Type )( 
            IPlunger * This,
            /* [in] */ PlungerType newVal);
        
        DECLSPEC_XFGVIRT(IPlunger, get_Material)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Material )( 
            IPlunger * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IPlunger, put_Material)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Material )( 
            IPlunger * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IPlunger, get_Image)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Image )( 
            IPlunger * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IPlunger, put_Image)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Image )( 
            IPlunger * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IPlunger, get_AnimFrames)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AnimFrames )( 
            IPlunger * This,
            /* [retval][out] */ int *pVal);
        
        DECLSPEC_XFGVIRT(IPlunger, put_AnimFrames)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_AnimFrames )( 
            IPlunger * This,
            /* [in] */ int newVal);
        
        DECLSPEC_XFGVIRT(IPlunger, get_TipShape)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TipShape )( 
            IPlunger * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IPlunger, put_TipShape)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TipShape )( 
            IPlunger * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IPlunger, get_RodDiam)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RodDiam )( 
            IPlunger * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPlunger, put_RodDiam)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_RodDiam )( 
            IPlunger * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPlunger, get_RingGap)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RingGap )( 
            IPlunger * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPlunger, put_RingGap)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_RingGap )( 
            IPlunger * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPlunger, get_RingDiam)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RingDiam )( 
            IPlunger * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPlunger, put_RingDiam)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_RingDiam )( 
            IPlunger * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPlunger, get_RingWidth)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RingWidth )( 
            IPlunger * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPlunger, put_RingWidth)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_RingWidth )( 
            IPlunger * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPlunger, get_SpringDiam)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SpringDiam )( 
            IPlunger * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPlunger, put_SpringDiam)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_SpringDiam )( 
            IPlunger * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPlunger, get_SpringGauge)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SpringGauge )( 
            IPlunger * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPlunger, put_SpringGauge)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_SpringGauge )( 
            IPlunger * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPlunger, get_SpringLoops)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SpringLoops )( 
            IPlunger * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPlunger, put_SpringLoops)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_SpringLoops )( 
            IPlunger * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPlunger, get_SpringEndLoops)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SpringEndLoops )( 
            IPlunger * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPlunger, put_SpringEndLoops)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_SpringEndLoops )( 
            IPlunger * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPlunger, get_MechPlunger)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_MechPlunger )( 
            IPlunger * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IPlunger, put_MechPlunger)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_MechPlunger )( 
            IPlunger * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IPlunger, get_AutoPlunger)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AutoPlunger )( 
            IPlunger * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IPlunger, put_AutoPlunger)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_AutoPlunger )( 
            IPlunger * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IPlunger, get_Visible)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Visible )( 
            IPlunger * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IPlunger, put_Visible)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Visible )( 
            IPlunger * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IPlunger, get_MechStrength)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_MechStrength )( 
            IPlunger * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPlunger, put_MechStrength)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_MechStrength )( 
            IPlunger * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPlunger, get_ParkPosition)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ParkPosition )( 
            IPlunger * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPlunger, put_ParkPosition)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ParkPosition )( 
            IPlunger * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPlunger, get_Stroke)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Stroke )( 
            IPlunger * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPlunger, put_Stroke)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Stroke )( 
            IPlunger * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPlunger, get_ScatterVelocity)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ScatterVelocity )( 
            IPlunger * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPlunger, put_ScatterVelocity)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ScatterVelocity )( 
            IPlunger * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPlunger, get_MomentumXfer)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_MomentumXfer )( 
            IPlunger * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPlunger, put_MomentumXfer)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_MomentumXfer )( 
            IPlunger * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPlunger, get_ReflectionEnabled)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ReflectionEnabled )( 
            IPlunger * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IPlunger, put_ReflectionEnabled)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ReflectionEnabled )( 
            IPlunger * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        END_INTERFACE
    } IPlungerVtbl;

    interface IPlunger
    {
        CONST_VTBL struct IPlungerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPlunger_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IPlunger_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IPlunger_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IPlunger_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IPlunger_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IPlunger_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IPlunger_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IPlunger_PullBack(This)	\
    ( (This)->lpVtbl -> PullBack(This) ) 

#define IPlunger_Fire(This)	\
    ( (This)->lpVtbl -> Fire(This) ) 

#define IPlunger_get_PullSpeed(This,pVal)	\
    ( (This)->lpVtbl -> get_PullSpeed(This,pVal) ) 

#define IPlunger_put_PullSpeed(This,newVal)	\
    ( (This)->lpVtbl -> put_PullSpeed(This,newVal) ) 

#define IPlunger_get_FireSpeed(This,pVal)	\
    ( (This)->lpVtbl -> get_FireSpeed(This,pVal) ) 

#define IPlunger_put_FireSpeed(This,newVal)	\
    ( (This)->lpVtbl -> put_FireSpeed(This,newVal) ) 

#define IPlunger_CreateBall(This,Ball)	\
    ( (This)->lpVtbl -> CreateBall(This,Ball) ) 

#define IPlunger_Position(This,pVal)	\
    ( (This)->lpVtbl -> Position(This,pVal) ) 

#define IPlunger_PullBackandRetract(This)	\
    ( (This)->lpVtbl -> PullBackandRetract(This) ) 

#define IPlunger_MotionDevice(This,pVal)	\
    ( (This)->lpVtbl -> MotionDevice(This,pVal) ) 

#define IPlunger_get_TimerEnabled(This,pVal)	\
    ( (This)->lpVtbl -> get_TimerEnabled(This,pVal) ) 

#define IPlunger_put_TimerEnabled(This,newVal)	\
    ( (This)->lpVtbl -> put_TimerEnabled(This,newVal) ) 

#define IPlunger_get_TimerInterval(This,pVal)	\
    ( (This)->lpVtbl -> get_TimerInterval(This,pVal) ) 

#define IPlunger_put_TimerInterval(This,newVal)	\
    ( (This)->lpVtbl -> put_TimerInterval(This,newVal) ) 

#define IPlunger_get_X(This,pVal)	\
    ( (This)->lpVtbl -> get_X(This,pVal) ) 

#define IPlunger_put_X(This,newVal)	\
    ( (This)->lpVtbl -> put_X(This,newVal) ) 

#define IPlunger_get_Y(This,pVal)	\
    ( (This)->lpVtbl -> get_Y(This,pVal) ) 

#define IPlunger_put_Y(This,newVal)	\
    ( (This)->lpVtbl -> put_Y(This,newVal) ) 

#define IPlunger_get_Width(This,pVal)	\
    ( (This)->lpVtbl -> get_Width(This,pVal) ) 

#define IPlunger_put_Width(This,newVal)	\
    ( (This)->lpVtbl -> put_Width(This,newVal) ) 

#define IPlunger_get_ZAdjust(This,pVal)	\
    ( (This)->lpVtbl -> get_ZAdjust(This,pVal) ) 

#define IPlunger_put_ZAdjust(This,newVal)	\
    ( (This)->lpVtbl -> put_ZAdjust(This,newVal) ) 

#define IPlunger_get_Surface(This,pVal)	\
    ( (This)->lpVtbl -> get_Surface(This,pVal) ) 

#define IPlunger_put_Surface(This,newVal)	\
    ( (This)->lpVtbl -> put_Surface(This,newVal) ) 

#define IPlunger_get_Name(This,pVal)	\
    ( (This)->lpVtbl -> get_Name(This,pVal) ) 

#define IPlunger_put_Name(This,newVal)	\
    ( (This)->lpVtbl -> put_Name(This,newVal) ) 

#define IPlunger_get_UserValue(This,pVal)	\
    ( (This)->lpVtbl -> get_UserValue(This,pVal) ) 

#define IPlunger_put_UserValue(This,newVal)	\
    ( (This)->lpVtbl -> put_UserValue(This,newVal) ) 

#define IPlunger_get_Type(This,pVal)	\
    ( (This)->lpVtbl -> get_Type(This,pVal) ) 

#define IPlunger_put_Type(This,newVal)	\
    ( (This)->lpVtbl -> put_Type(This,newVal) ) 

#define IPlunger_get_Material(This,pVal)	\
    ( (This)->lpVtbl -> get_Material(This,pVal) ) 

#define IPlunger_put_Material(This,newVal)	\
    ( (This)->lpVtbl -> put_Material(This,newVal) ) 

#define IPlunger_get_Image(This,pVal)	\
    ( (This)->lpVtbl -> get_Image(This,pVal) ) 

#define IPlunger_put_Image(This,newVal)	\
    ( (This)->lpVtbl -> put_Image(This,newVal) ) 

#define IPlunger_get_AnimFrames(This,pVal)	\
    ( (This)->lpVtbl -> get_AnimFrames(This,pVal) ) 

#define IPlunger_put_AnimFrames(This,newVal)	\
    ( (This)->lpVtbl -> put_AnimFrames(This,newVal) ) 

#define IPlunger_get_TipShape(This,pVal)	\
    ( (This)->lpVtbl -> get_TipShape(This,pVal) ) 

#define IPlunger_put_TipShape(This,newVal)	\
    ( (This)->lpVtbl -> put_TipShape(This,newVal) ) 

#define IPlunger_get_RodDiam(This,pVal)	\
    ( (This)->lpVtbl -> get_RodDiam(This,pVal) ) 

#define IPlunger_put_RodDiam(This,newVal)	\
    ( (This)->lpVtbl -> put_RodDiam(This,newVal) ) 

#define IPlunger_get_RingGap(This,pVal)	\
    ( (This)->lpVtbl -> get_RingGap(This,pVal) ) 

#define IPlunger_put_RingGap(This,newVal)	\
    ( (This)->lpVtbl -> put_RingGap(This,newVal) ) 

#define IPlunger_get_RingDiam(This,pVal)	\
    ( (This)->lpVtbl -> get_RingDiam(This,pVal) ) 

#define IPlunger_put_RingDiam(This,newVal)	\
    ( (This)->lpVtbl -> put_RingDiam(This,newVal) ) 

#define IPlunger_get_RingWidth(This,pVal)	\
    ( (This)->lpVtbl -> get_RingWidth(This,pVal) ) 

#define IPlunger_put_RingWidth(This,newVal)	\
    ( (This)->lpVtbl -> put_RingWidth(This,newVal) ) 

#define IPlunger_get_SpringDiam(This,pVal)	\
    ( (This)->lpVtbl -> get_SpringDiam(This,pVal) ) 

#define IPlunger_put_SpringDiam(This,newVal)	\
    ( (This)->lpVtbl -> put_SpringDiam(This,newVal) ) 

#define IPlunger_get_SpringGauge(This,pVal)	\
    ( (This)->lpVtbl -> get_SpringGauge(This,pVal) ) 

#define IPlunger_put_SpringGauge(This,newVal)	\
    ( (This)->lpVtbl -> put_SpringGauge(This,newVal) ) 

#define IPlunger_get_SpringLoops(This,pVal)	\
    ( (This)->lpVtbl -> get_SpringLoops(This,pVal) ) 

#define IPlunger_put_SpringLoops(This,newVal)	\
    ( (This)->lpVtbl -> put_SpringLoops(This,newVal) ) 

#define IPlunger_get_SpringEndLoops(This,pVal)	\
    ( (This)->lpVtbl -> get_SpringEndLoops(This,pVal) ) 

#define IPlunger_put_SpringEndLoops(This,newVal)	\
    ( (This)->lpVtbl -> put_SpringEndLoops(This,newVal) ) 

#define IPlunger_get_MechPlunger(This,pVal)	\
    ( (This)->lpVtbl -> get_MechPlunger(This,pVal) ) 

#define IPlunger_put_MechPlunger(This,newVal)	\
    ( (This)->lpVtbl -> put_MechPlunger(This,newVal) ) 

#define IPlunger_get_AutoPlunger(This,pVal)	\
    ( (This)->lpVtbl -> get_AutoPlunger(This,pVal) ) 

#define IPlunger_put_AutoPlunger(This,newVal)	\
    ( (This)->lpVtbl -> put_AutoPlunger(This,newVal) ) 

#define IPlunger_get_Visible(This,pVal)	\
    ( (This)->lpVtbl -> get_Visible(This,pVal) ) 

#define IPlunger_put_Visible(This,newVal)	\
    ( (This)->lpVtbl -> put_Visible(This,newVal) ) 

#define IPlunger_get_MechStrength(This,pVal)	\
    ( (This)->lpVtbl -> get_MechStrength(This,pVal) ) 

#define IPlunger_put_MechStrength(This,newVal)	\
    ( (This)->lpVtbl -> put_MechStrength(This,newVal) ) 

#define IPlunger_get_ParkPosition(This,pVal)	\
    ( (This)->lpVtbl -> get_ParkPosition(This,pVal) ) 

#define IPlunger_put_ParkPosition(This,newVal)	\
    ( (This)->lpVtbl -> put_ParkPosition(This,newVal) ) 

#define IPlunger_get_Stroke(This,pVal)	\
    ( (This)->lpVtbl -> get_Stroke(This,pVal) ) 

#define IPlunger_put_Stroke(This,newVal)	\
    ( (This)->lpVtbl -> put_Stroke(This,newVal) ) 

#define IPlunger_get_ScatterVelocity(This,pVal)	\
    ( (This)->lpVtbl -> get_ScatterVelocity(This,pVal) ) 

#define IPlunger_put_ScatterVelocity(This,newVal)	\
    ( (This)->lpVtbl -> put_ScatterVelocity(This,newVal) ) 

#define IPlunger_get_MomentumXfer(This,pVal)	\
    ( (This)->lpVtbl -> get_MomentumXfer(This,pVal) ) 

#define IPlunger_put_MomentumXfer(This,newVal)	\
    ( (This)->lpVtbl -> put_MomentumXfer(This,newVal) ) 

#define IPlunger_get_ReflectionEnabled(This,pVal)	\
    ( (This)->lpVtbl -> get_ReflectionEnabled(This,pVal) ) 

#define IPlunger_put_ReflectionEnabled(This,newVal)	\
    ( (This)->lpVtbl -> put_ReflectionEnabled(This,newVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IPlunger_INTERFACE_DEFINED__ */


#ifndef __IPlungerEvents_DISPINTERFACE_DEFINED__
#define __IPlungerEvents_DISPINTERFACE_DEFINED__

/* dispinterface IPlungerEvents */
/* [uuid] */ 


EXTERN_C const IID DIID_IPlungerEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("9BCA5DD9-B893-4595-9D3A-8BD77FB5DE71")
    IPlungerEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct IPlungerEventsVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPlungerEvents * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPlungerEvents * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPlungerEvents * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IPlungerEvents * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IPlungerEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IPlungerEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IPlungerEvents * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        END_INTERFACE
    } IPlungerEventsVtbl;

    interface IPlungerEvents
    {
        CONST_VTBL struct IPlungerEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPlungerEvents_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IPlungerEvents_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IPlungerEvents_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IPlungerEvents_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IPlungerEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IPlungerEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IPlungerEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* __IPlungerEvents_DISPINTERFACE_DEFINED__ */


#ifndef __ITextbox_INTERFACE_DEFINED__
#define __ITextbox_INTERFACE_DEFINED__

/* interface ITextbox */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_ITextbox;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("596FCD76-C5E8-4B6A-A333-FB0D645E0518")
    ITextbox : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BackColor( 
            /* [retval][out] */ OLE_COLOR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_BackColor( 
            /* [in] */ OLE_COLOR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_FontColor( 
            /* [retval][out] */ OLE_COLOR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_FontColor( 
            /* [in] */ OLE_COLOR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Text( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Text( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Font( 
            /* [retval][out] */ IFontDisp **pVal) = 0;
        
        virtual /* [helpstring][id][propputref] */ HRESULT STDMETHODCALLTYPE putref_Font( 
            /* [in] */ IFontDisp *newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Width( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Width( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Height( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Height( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Alignment( 
            /* [retval][out] */ TextAlignment *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Alignment( 
            /* [in] */ TextAlignment newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_IsTransparent( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_IsTransparent( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DMD( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_DMD( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TimerEnabled( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TimerEnabled( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TimerInterval( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TimerInterval( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_X( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_X( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Y( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Y( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_IntensityScale( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_IntensityScale( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Name( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_UserValue( 
            /* [retval][out] */ VARIANT *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_UserValue( 
            /* [in] */ VARIANT *newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Visible( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Visible( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct ITextboxVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITextbox * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITextbox * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITextbox * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITextbox * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITextbox * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITextbox * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITextbox * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        DECLSPEC_XFGVIRT(ITextbox, get_BackColor)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BackColor )( 
            ITextbox * This,
            /* [retval][out] */ OLE_COLOR *pVal);
        
        DECLSPEC_XFGVIRT(ITextbox, put_BackColor)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_BackColor )( 
            ITextbox * This,
            /* [in] */ OLE_COLOR newVal);
        
        DECLSPEC_XFGVIRT(ITextbox, get_FontColor)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_FontColor )( 
            ITextbox * This,
            /* [retval][out] */ OLE_COLOR *pVal);
        
        DECLSPEC_XFGVIRT(ITextbox, put_FontColor)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_FontColor )( 
            ITextbox * This,
            /* [in] */ OLE_COLOR newVal);
        
        DECLSPEC_XFGVIRT(ITextbox, get_Text)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Text )( 
            ITextbox * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(ITextbox, put_Text)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Text )( 
            ITextbox * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(ITextbox, get_Font)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Font )( 
            ITextbox * This,
            /* [retval][out] */ IFontDisp **pVal);
        
        DECLSPEC_XFGVIRT(ITextbox, putref_Font)
        /* [helpstring][id][propputref] */ HRESULT ( STDMETHODCALLTYPE *putref_Font )( 
            ITextbox * This,
            /* [in] */ IFontDisp *newVal);
        
        DECLSPEC_XFGVIRT(ITextbox, get_Width)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Width )( 
            ITextbox * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITextbox, put_Width)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Width )( 
            ITextbox * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITextbox, get_Height)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Height )( 
            ITextbox * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITextbox, put_Height)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Height )( 
            ITextbox * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITextbox, get_Alignment)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Alignment )( 
            ITextbox * This,
            /* [retval][out] */ TextAlignment *pVal);
        
        DECLSPEC_XFGVIRT(ITextbox, put_Alignment)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Alignment )( 
            ITextbox * This,
            /* [in] */ TextAlignment newVal);
        
        DECLSPEC_XFGVIRT(ITextbox, get_IsTransparent)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_IsTransparent )( 
            ITextbox * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(ITextbox, put_IsTransparent)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_IsTransparent )( 
            ITextbox * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(ITextbox, get_DMD)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_DMD )( 
            ITextbox * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(ITextbox, put_DMD)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_DMD )( 
            ITextbox * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(ITextbox, get_TimerEnabled)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TimerEnabled )( 
            ITextbox * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(ITextbox, put_TimerEnabled)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TimerEnabled )( 
            ITextbox * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(ITextbox, get_TimerInterval)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TimerInterval )( 
            ITextbox * This,
            /* [retval][out] */ long *pVal);
        
        DECLSPEC_XFGVIRT(ITextbox, put_TimerInterval)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TimerInterval )( 
            ITextbox * This,
            /* [in] */ long newVal);
        
        DECLSPEC_XFGVIRT(ITextbox, get_X)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_X )( 
            ITextbox * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITextbox, put_X)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_X )( 
            ITextbox * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITextbox, get_Y)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Y )( 
            ITextbox * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITextbox, put_Y)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Y )( 
            ITextbox * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITextbox, get_IntensityScale)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_IntensityScale )( 
            ITextbox * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITextbox, put_IntensityScale)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_IntensityScale )( 
            ITextbox * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITextbox, get_Name)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ITextbox * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(ITextbox, put_Name)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            ITextbox * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(ITextbox, get_UserValue)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_UserValue )( 
            ITextbox * This,
            /* [retval][out] */ VARIANT *pVal);
        
        DECLSPEC_XFGVIRT(ITextbox, put_UserValue)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_UserValue )( 
            ITextbox * This,
            /* [in] */ VARIANT *newVal);
        
        DECLSPEC_XFGVIRT(ITextbox, get_Visible)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Visible )( 
            ITextbox * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(ITextbox, put_Visible)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Visible )( 
            ITextbox * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        END_INTERFACE
    } ITextboxVtbl;

    interface ITextbox
    {
        CONST_VTBL struct ITextboxVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITextbox_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ITextbox_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ITextbox_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ITextbox_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define ITextbox_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define ITextbox_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define ITextbox_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define ITextbox_get_BackColor(This,pVal)	\
    ( (This)->lpVtbl -> get_BackColor(This,pVal) ) 

#define ITextbox_put_BackColor(This,newVal)	\
    ( (This)->lpVtbl -> put_BackColor(This,newVal) ) 

#define ITextbox_get_FontColor(This,pVal)	\
    ( (This)->lpVtbl -> get_FontColor(This,pVal) ) 

#define ITextbox_put_FontColor(This,newVal)	\
    ( (This)->lpVtbl -> put_FontColor(This,newVal) ) 

#define ITextbox_get_Text(This,pVal)	\
    ( (This)->lpVtbl -> get_Text(This,pVal) ) 

#define ITextbox_put_Text(This,newVal)	\
    ( (This)->lpVtbl -> put_Text(This,newVal) ) 

#define ITextbox_get_Font(This,pVal)	\
    ( (This)->lpVtbl -> get_Font(This,pVal) ) 

#define ITextbox_putref_Font(This,newVal)	\
    ( (This)->lpVtbl -> putref_Font(This,newVal) ) 

#define ITextbox_get_Width(This,pVal)	\
    ( (This)->lpVtbl -> get_Width(This,pVal) ) 

#define ITextbox_put_Width(This,newVal)	\
    ( (This)->lpVtbl -> put_Width(This,newVal) ) 

#define ITextbox_get_Height(This,pVal)	\
    ( (This)->lpVtbl -> get_Height(This,pVal) ) 

#define ITextbox_put_Height(This,newVal)	\
    ( (This)->lpVtbl -> put_Height(This,newVal) ) 

#define ITextbox_get_Alignment(This,pVal)	\
    ( (This)->lpVtbl -> get_Alignment(This,pVal) ) 

#define ITextbox_put_Alignment(This,newVal)	\
    ( (This)->lpVtbl -> put_Alignment(This,newVal) ) 

#define ITextbox_get_IsTransparent(This,pVal)	\
    ( (This)->lpVtbl -> get_IsTransparent(This,pVal) ) 

#define ITextbox_put_IsTransparent(This,newVal)	\
    ( (This)->lpVtbl -> put_IsTransparent(This,newVal) ) 

#define ITextbox_get_DMD(This,pVal)	\
    ( (This)->lpVtbl -> get_DMD(This,pVal) ) 

#define ITextbox_put_DMD(This,newVal)	\
    ( (This)->lpVtbl -> put_DMD(This,newVal) ) 

#define ITextbox_get_TimerEnabled(This,pVal)	\
    ( (This)->lpVtbl -> get_TimerEnabled(This,pVal) ) 

#define ITextbox_put_TimerEnabled(This,newVal)	\
    ( (This)->lpVtbl -> put_TimerEnabled(This,newVal) ) 

#define ITextbox_get_TimerInterval(This,pVal)	\
    ( (This)->lpVtbl -> get_TimerInterval(This,pVal) ) 

#define ITextbox_put_TimerInterval(This,newVal)	\
    ( (This)->lpVtbl -> put_TimerInterval(This,newVal) ) 

#define ITextbox_get_X(This,pVal)	\
    ( (This)->lpVtbl -> get_X(This,pVal) ) 

#define ITextbox_put_X(This,newVal)	\
    ( (This)->lpVtbl -> put_X(This,newVal) ) 

#define ITextbox_get_Y(This,pVal)	\
    ( (This)->lpVtbl -> get_Y(This,pVal) ) 

#define ITextbox_put_Y(This,newVal)	\
    ( (This)->lpVtbl -> put_Y(This,newVal) ) 

#define ITextbox_get_IntensityScale(This,pVal)	\
    ( (This)->lpVtbl -> get_IntensityScale(This,pVal) ) 

#define ITextbox_put_IntensityScale(This,newVal)	\
    ( (This)->lpVtbl -> put_IntensityScale(This,newVal) ) 

#define ITextbox_get_Name(This,pVal)	\
    ( (This)->lpVtbl -> get_Name(This,pVal) ) 

#define ITextbox_put_Name(This,newVal)	\
    ( (This)->lpVtbl -> put_Name(This,newVal) ) 

#define ITextbox_get_UserValue(This,pVal)	\
    ( (This)->lpVtbl -> get_UserValue(This,pVal) ) 

#define ITextbox_put_UserValue(This,newVal)	\
    ( (This)->lpVtbl -> put_UserValue(This,newVal) ) 

#define ITextbox_get_Visible(This,pVal)	\
    ( (This)->lpVtbl -> get_Visible(This,pVal) ) 

#define ITextbox_put_Visible(This,newVal)	\
    ( (This)->lpVtbl -> put_Visible(This,newVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ITextbox_INTERFACE_DEFINED__ */


#ifndef __ITextboxEvents_DISPINTERFACE_DEFINED__
#define __ITextboxEvents_DISPINTERFACE_DEFINED__

/* dispinterface ITextboxEvents */
/* [uuid] */ 


EXTERN_C const IID DIID_ITextboxEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("F1C767B8-7351-4ebc-8022-E73143BE6F5D")
    ITextboxEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct ITextboxEventsVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITextboxEvents * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITextboxEvents * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITextboxEvents * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITextboxEvents * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITextboxEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITextboxEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITextboxEvents * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        END_INTERFACE
    } ITextboxEventsVtbl;

    interface ITextboxEvents
    {
        CONST_VTBL struct ITextboxEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITextboxEvents_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ITextboxEvents_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ITextboxEvents_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ITextboxEvents_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define ITextboxEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define ITextboxEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define ITextboxEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* __ITextboxEvents_DISPINTERFACE_DEFINED__ */


#ifndef __IBumper_INTERFACE_DEFINED__
#define __IBumper_INTERFACE_DEFINED__

/* interface IBumper */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IBumper;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4CB2FCCA-9336-48FF-90BD-F8D01C16BE8C")
    IBumper : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Radius( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Radius( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_X( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_X( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Y( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Y( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BaseMaterial( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_BaseMaterial( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SkirtMaterial( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SkirtMaterial( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_UserValue( 
            /* [retval][out] */ VARIANT *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_UserValue( 
            /* [in] */ VARIANT *newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Surface( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Surface( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Force( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Force( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Threshold( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Threshold( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TimerEnabled( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TimerEnabled( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TimerInterval( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TimerInterval( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CapMaterial( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_CapMaterial( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RingMaterial( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_RingMaterial( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_HeightScale( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_HeightScale( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Orientation( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Orientation( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RingSpeed( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_RingSpeed( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RingDropOffset( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_RingDropOffset( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Name( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_HasHitEvent( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_HasHitEvent( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CapVisible( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_CapVisible( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BaseVisible( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_BaseVisible( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RingVisible( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_RingVisible( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SkirtVisible( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SkirtVisible( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Collidable( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Collidable( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ReflectionEnabled( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ReflectionEnabled( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Scatter( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Scatter( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_EnableSkirtAnimation( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_EnableSkirtAnimation( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE PlayHit( void) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IBumperVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IBumper * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IBumper * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IBumper * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IBumper * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IBumper * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IBumper * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IBumper * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        DECLSPEC_XFGVIRT(IBumper, get_Radius)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Radius )( 
            IBumper * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IBumper, put_Radius)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Radius )( 
            IBumper * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IBumper, get_X)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_X )( 
            IBumper * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IBumper, put_X)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_X )( 
            IBumper * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IBumper, get_Y)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Y )( 
            IBumper * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IBumper, put_Y)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Y )( 
            IBumper * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IBumper, get_BaseMaterial)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BaseMaterial )( 
            IBumper * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IBumper, put_BaseMaterial)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_BaseMaterial )( 
            IBumper * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IBumper, get_SkirtMaterial)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SkirtMaterial )( 
            IBumper * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IBumper, put_SkirtMaterial)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_SkirtMaterial )( 
            IBumper * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IBumper, get_UserValue)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_UserValue )( 
            IBumper * This,
            /* [retval][out] */ VARIANT *pVal);
        
        DECLSPEC_XFGVIRT(IBumper, put_UserValue)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_UserValue )( 
            IBumper * This,
            /* [in] */ VARIANT *newVal);
        
        DECLSPEC_XFGVIRT(IBumper, get_Surface)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Surface )( 
            IBumper * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IBumper, put_Surface)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Surface )( 
            IBumper * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IBumper, get_Force)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Force )( 
            IBumper * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IBumper, put_Force)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Force )( 
            IBumper * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IBumper, get_Threshold)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Threshold )( 
            IBumper * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IBumper, put_Threshold)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Threshold )( 
            IBumper * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IBumper, get_TimerEnabled)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TimerEnabled )( 
            IBumper * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IBumper, put_TimerEnabled)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TimerEnabled )( 
            IBumper * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IBumper, get_TimerInterval)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TimerInterval )( 
            IBumper * This,
            /* [retval][out] */ long *pVal);
        
        DECLSPEC_XFGVIRT(IBumper, put_TimerInterval)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TimerInterval )( 
            IBumper * This,
            /* [in] */ long newVal);
        
        DECLSPEC_XFGVIRT(IBumper, get_CapMaterial)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CapMaterial )( 
            IBumper * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IBumper, put_CapMaterial)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_CapMaterial )( 
            IBumper * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IBumper, get_RingMaterial)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RingMaterial )( 
            IBumper * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IBumper, put_RingMaterial)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_RingMaterial )( 
            IBumper * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IBumper, get_HeightScale)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HeightScale )( 
            IBumper * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IBumper, put_HeightScale)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_HeightScale )( 
            IBumper * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IBumper, get_Orientation)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Orientation )( 
            IBumper * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IBumper, put_Orientation)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Orientation )( 
            IBumper * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IBumper, get_RingSpeed)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RingSpeed )( 
            IBumper * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IBumper, put_RingSpeed)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_RingSpeed )( 
            IBumper * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IBumper, get_RingDropOffset)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RingDropOffset )( 
            IBumper * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IBumper, put_RingDropOffset)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_RingDropOffset )( 
            IBumper * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IBumper, get_Name)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IBumper * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IBumper, put_Name)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IBumper * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IBumper, get_HasHitEvent)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HasHitEvent )( 
            IBumper * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IBumper, put_HasHitEvent)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_HasHitEvent )( 
            IBumper * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IBumper, get_CapVisible)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CapVisible )( 
            IBumper * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IBumper, put_CapVisible)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_CapVisible )( 
            IBumper * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IBumper, get_BaseVisible)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BaseVisible )( 
            IBumper * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IBumper, put_BaseVisible)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_BaseVisible )( 
            IBumper * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IBumper, get_RingVisible)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RingVisible )( 
            IBumper * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IBumper, put_RingVisible)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_RingVisible )( 
            IBumper * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IBumper, get_SkirtVisible)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SkirtVisible )( 
            IBumper * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IBumper, put_SkirtVisible)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_SkirtVisible )( 
            IBumper * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IBumper, get_Collidable)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Collidable )( 
            IBumper * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IBumper, put_Collidable)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Collidable )( 
            IBumper * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IBumper, get_ReflectionEnabled)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ReflectionEnabled )( 
            IBumper * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IBumper, put_ReflectionEnabled)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ReflectionEnabled )( 
            IBumper * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IBumper, get_Scatter)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Scatter )( 
            IBumper * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IBumper, put_Scatter)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Scatter )( 
            IBumper * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IBumper, get_EnableSkirtAnimation)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_EnableSkirtAnimation )( 
            IBumper * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IBumper, put_EnableSkirtAnimation)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_EnableSkirtAnimation )( 
            IBumper * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IBumper, PlayHit)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *PlayHit )( 
            IBumper * This);
        
        END_INTERFACE
    } IBumperVtbl;

    interface IBumper
    {
        CONST_VTBL struct IBumperVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBumper_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IBumper_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IBumper_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IBumper_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IBumper_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IBumper_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IBumper_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IBumper_get_Radius(This,pVal)	\
    ( (This)->lpVtbl -> get_Radius(This,pVal) ) 

#define IBumper_put_Radius(This,newVal)	\
    ( (This)->lpVtbl -> put_Radius(This,newVal) ) 

#define IBumper_get_X(This,pVal)	\
    ( (This)->lpVtbl -> get_X(This,pVal) ) 

#define IBumper_put_X(This,newVal)	\
    ( (This)->lpVtbl -> put_X(This,newVal) ) 

#define IBumper_get_Y(This,pVal)	\
    ( (This)->lpVtbl -> get_Y(This,pVal) ) 

#define IBumper_put_Y(This,newVal)	\
    ( (This)->lpVtbl -> put_Y(This,newVal) ) 

#define IBumper_get_BaseMaterial(This,pVal)	\
    ( (This)->lpVtbl -> get_BaseMaterial(This,pVal) ) 

#define IBumper_put_BaseMaterial(This,newVal)	\
    ( (This)->lpVtbl -> put_BaseMaterial(This,newVal) ) 

#define IBumper_get_SkirtMaterial(This,pVal)	\
    ( (This)->lpVtbl -> get_SkirtMaterial(This,pVal) ) 

#define IBumper_put_SkirtMaterial(This,newVal)	\
    ( (This)->lpVtbl -> put_SkirtMaterial(This,newVal) ) 

#define IBumper_get_UserValue(This,pVal)	\
    ( (This)->lpVtbl -> get_UserValue(This,pVal) ) 

#define IBumper_put_UserValue(This,newVal)	\
    ( (This)->lpVtbl -> put_UserValue(This,newVal) ) 

#define IBumper_get_Surface(This,pVal)	\
    ( (This)->lpVtbl -> get_Surface(This,pVal) ) 

#define IBumper_put_Surface(This,newVal)	\
    ( (This)->lpVtbl -> put_Surface(This,newVal) ) 

#define IBumper_get_Force(This,pVal)	\
    ( (This)->lpVtbl -> get_Force(This,pVal) ) 

#define IBumper_put_Force(This,newVal)	\
    ( (This)->lpVtbl -> put_Force(This,newVal) ) 

#define IBumper_get_Threshold(This,pVal)	\
    ( (This)->lpVtbl -> get_Threshold(This,pVal) ) 

#define IBumper_put_Threshold(This,newVal)	\
    ( (This)->lpVtbl -> put_Threshold(This,newVal) ) 

#define IBumper_get_TimerEnabled(This,pVal)	\
    ( (This)->lpVtbl -> get_TimerEnabled(This,pVal) ) 

#define IBumper_put_TimerEnabled(This,newVal)	\
    ( (This)->lpVtbl -> put_TimerEnabled(This,newVal) ) 

#define IBumper_get_TimerInterval(This,pVal)	\
    ( (This)->lpVtbl -> get_TimerInterval(This,pVal) ) 

#define IBumper_put_TimerInterval(This,newVal)	\
    ( (This)->lpVtbl -> put_TimerInterval(This,newVal) ) 

#define IBumper_get_CapMaterial(This,pVal)	\
    ( (This)->lpVtbl -> get_CapMaterial(This,pVal) ) 

#define IBumper_put_CapMaterial(This,newVal)	\
    ( (This)->lpVtbl -> put_CapMaterial(This,newVal) ) 

#define IBumper_get_RingMaterial(This,pVal)	\
    ( (This)->lpVtbl -> get_RingMaterial(This,pVal) ) 

#define IBumper_put_RingMaterial(This,newVal)	\
    ( (This)->lpVtbl -> put_RingMaterial(This,newVal) ) 

#define IBumper_get_HeightScale(This,pVal)	\
    ( (This)->lpVtbl -> get_HeightScale(This,pVal) ) 

#define IBumper_put_HeightScale(This,newVal)	\
    ( (This)->lpVtbl -> put_HeightScale(This,newVal) ) 

#define IBumper_get_Orientation(This,pVal)	\
    ( (This)->lpVtbl -> get_Orientation(This,pVal) ) 

#define IBumper_put_Orientation(This,newVal)	\
    ( (This)->lpVtbl -> put_Orientation(This,newVal) ) 

#define IBumper_get_RingSpeed(This,pVal)	\
    ( (This)->lpVtbl -> get_RingSpeed(This,pVal) ) 

#define IBumper_put_RingSpeed(This,newVal)	\
    ( (This)->lpVtbl -> put_RingSpeed(This,newVal) ) 

#define IBumper_get_RingDropOffset(This,pVal)	\
    ( (This)->lpVtbl -> get_RingDropOffset(This,pVal) ) 

#define IBumper_put_RingDropOffset(This,newVal)	\
    ( (This)->lpVtbl -> put_RingDropOffset(This,newVal) ) 

#define IBumper_get_Name(This,pVal)	\
    ( (This)->lpVtbl -> get_Name(This,pVal) ) 

#define IBumper_put_Name(This,newVal)	\
    ( (This)->lpVtbl -> put_Name(This,newVal) ) 

#define IBumper_get_HasHitEvent(This,pVal)	\
    ( (This)->lpVtbl -> get_HasHitEvent(This,pVal) ) 

#define IBumper_put_HasHitEvent(This,newVal)	\
    ( (This)->lpVtbl -> put_HasHitEvent(This,newVal) ) 

#define IBumper_get_CapVisible(This,pVal)	\
    ( (This)->lpVtbl -> get_CapVisible(This,pVal) ) 

#define IBumper_put_CapVisible(This,newVal)	\
    ( (This)->lpVtbl -> put_CapVisible(This,newVal) ) 

#define IBumper_get_BaseVisible(This,pVal)	\
    ( (This)->lpVtbl -> get_BaseVisible(This,pVal) ) 

#define IBumper_put_BaseVisible(This,newVal)	\
    ( (This)->lpVtbl -> put_BaseVisible(This,newVal) ) 

#define IBumper_get_RingVisible(This,pVal)	\
    ( (This)->lpVtbl -> get_RingVisible(This,pVal) ) 

#define IBumper_put_RingVisible(This,newVal)	\
    ( (This)->lpVtbl -> put_RingVisible(This,newVal) ) 

#define IBumper_get_SkirtVisible(This,pVal)	\
    ( (This)->lpVtbl -> get_SkirtVisible(This,pVal) ) 

#define IBumper_put_SkirtVisible(This,newVal)	\
    ( (This)->lpVtbl -> put_SkirtVisible(This,newVal) ) 

#define IBumper_get_Collidable(This,pVal)	\
    ( (This)->lpVtbl -> get_Collidable(This,pVal) ) 

#define IBumper_put_Collidable(This,newVal)	\
    ( (This)->lpVtbl -> put_Collidable(This,newVal) ) 

#define IBumper_get_ReflectionEnabled(This,pVal)	\
    ( (This)->lpVtbl -> get_ReflectionEnabled(This,pVal) ) 

#define IBumper_put_ReflectionEnabled(This,newVal)	\
    ( (This)->lpVtbl -> put_ReflectionEnabled(This,newVal) ) 

#define IBumper_get_Scatter(This,pVal)	\
    ( (This)->lpVtbl -> get_Scatter(This,pVal) ) 

#define IBumper_put_Scatter(This,newVal)	\
    ( (This)->lpVtbl -> put_Scatter(This,newVal) ) 

#define IBumper_get_EnableSkirtAnimation(This,pVal)	\
    ( (This)->lpVtbl -> get_EnableSkirtAnimation(This,pVal) ) 

#define IBumper_put_EnableSkirtAnimation(This,newVal)	\
    ( (This)->lpVtbl -> put_EnableSkirtAnimation(This,newVal) ) 

#define IBumper_PlayHit(This)	\
    ( (This)->lpVtbl -> PlayHit(This) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IBumper_INTERFACE_DEFINED__ */


#ifndef __IBumperEvents_DISPINTERFACE_DEFINED__
#define __IBumperEvents_DISPINTERFACE_DEFINED__

/* dispinterface IBumperEvents */
/* [uuid] */ 


EXTERN_C const IID DIID_IBumperEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("D868EE8D-AF83-4c7a-A68E-1DBA956434A7")
    IBumperEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct IBumperEventsVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IBumperEvents * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IBumperEvents * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IBumperEvents * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IBumperEvents * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IBumperEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IBumperEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IBumperEvents * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        END_INTERFACE
    } IBumperEventsVtbl;

    interface IBumperEvents
    {
        CONST_VTBL struct IBumperEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBumperEvents_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IBumperEvents_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IBumperEvents_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IBumperEvents_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IBumperEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IBumperEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IBumperEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* __IBumperEvents_DISPINTERFACE_DEFINED__ */


#ifndef __ITrigger_INTERFACE_DEFINED__
#define __ITrigger_INTERFACE_DEFINED__

/* interface ITrigger */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_ITrigger;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CD5054C6-016F-4BB5-B131-12FE29720DF8")
    ITrigger : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Radius( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Radius( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TimerEnabled( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TimerEnabled( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TimerInterval( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TimerInterval( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_X( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_X( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Y( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Y( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Enabled( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Enabled( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Visible( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Visible( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TriggerShape( 
            /* [retval][out] */ TriggerShape *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TriggerShape( 
            /* [in] */ TriggerShape newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Surface( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Surface( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Name( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_UserValue( 
            /* [retval][out] */ VARIANT *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_UserValue( 
            /* [in] */ VARIANT *newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE BallCntOver( 
            /* [retval][out] */ int *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE DestroyBall( 
            /* [retval][out] */ int *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_HitHeight( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_HitHeight( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Material( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Material( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Rotation( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Rotation( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_WireThickness( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_WireThickness( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AnimSpeed( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_AnimSpeed( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ReflectionEnabled( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ReflectionEnabled( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct ITriggerVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITrigger * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITrigger * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITrigger * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITrigger * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITrigger * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITrigger * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITrigger * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        DECLSPEC_XFGVIRT(ITrigger, get_Radius)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Radius )( 
            ITrigger * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITrigger, put_Radius)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Radius )( 
            ITrigger * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITrigger, get_TimerEnabled)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TimerEnabled )( 
            ITrigger * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(ITrigger, put_TimerEnabled)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TimerEnabled )( 
            ITrigger * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(ITrigger, get_TimerInterval)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TimerInterval )( 
            ITrigger * This,
            /* [retval][out] */ long *pVal);
        
        DECLSPEC_XFGVIRT(ITrigger, put_TimerInterval)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TimerInterval )( 
            ITrigger * This,
            /* [in] */ long newVal);
        
        DECLSPEC_XFGVIRT(ITrigger, get_X)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_X )( 
            ITrigger * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITrigger, put_X)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_X )( 
            ITrigger * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITrigger, get_Y)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Y )( 
            ITrigger * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITrigger, put_Y)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Y )( 
            ITrigger * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITrigger, get_Enabled)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Enabled )( 
            ITrigger * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(ITrigger, put_Enabled)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Enabled )( 
            ITrigger * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(ITrigger, get_Visible)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Visible )( 
            ITrigger * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(ITrigger, put_Visible)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Visible )( 
            ITrigger * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(ITrigger, get_TriggerShape)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TriggerShape )( 
            ITrigger * This,
            /* [retval][out] */ TriggerShape *pVal);
        
        DECLSPEC_XFGVIRT(ITrigger, put_TriggerShape)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TriggerShape )( 
            ITrigger * This,
            /* [in] */ TriggerShape newVal);
        
        DECLSPEC_XFGVIRT(ITrigger, get_Surface)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Surface )( 
            ITrigger * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(ITrigger, put_Surface)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Surface )( 
            ITrigger * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(ITrigger, get_Name)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ITrigger * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(ITrigger, put_Name)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            ITrigger * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(ITrigger, get_UserValue)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_UserValue )( 
            ITrigger * This,
            /* [retval][out] */ VARIANT *pVal);
        
        DECLSPEC_XFGVIRT(ITrigger, put_UserValue)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_UserValue )( 
            ITrigger * This,
            /* [in] */ VARIANT *newVal);
        
        DECLSPEC_XFGVIRT(ITrigger, BallCntOver)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *BallCntOver )( 
            ITrigger * This,
            /* [retval][out] */ int *pVal);
        
        DECLSPEC_XFGVIRT(ITrigger, DestroyBall)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *DestroyBall )( 
            ITrigger * This,
            /* [retval][out] */ int *pVal);
        
        DECLSPEC_XFGVIRT(ITrigger, get_HitHeight)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HitHeight )( 
            ITrigger * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITrigger, put_HitHeight)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_HitHeight )( 
            ITrigger * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITrigger, get_Material)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Material )( 
            ITrigger * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(ITrigger, put_Material)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Material )( 
            ITrigger * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(ITrigger, get_Rotation)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Rotation )( 
            ITrigger * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITrigger, put_Rotation)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Rotation )( 
            ITrigger * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITrigger, get_WireThickness)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_WireThickness )( 
            ITrigger * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITrigger, put_WireThickness)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_WireThickness )( 
            ITrigger * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITrigger, get_AnimSpeed)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AnimSpeed )( 
            ITrigger * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ITrigger, put_AnimSpeed)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_AnimSpeed )( 
            ITrigger * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ITrigger, get_ReflectionEnabled)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ReflectionEnabled )( 
            ITrigger * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(ITrigger, put_ReflectionEnabled)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ReflectionEnabled )( 
            ITrigger * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        END_INTERFACE
    } ITriggerVtbl;

    interface ITrigger
    {
        CONST_VTBL struct ITriggerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITrigger_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ITrigger_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ITrigger_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ITrigger_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define ITrigger_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define ITrigger_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define ITrigger_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define ITrigger_get_Radius(This,pVal)	\
    ( (This)->lpVtbl -> get_Radius(This,pVal) ) 

#define ITrigger_put_Radius(This,newVal)	\
    ( (This)->lpVtbl -> put_Radius(This,newVal) ) 

#define ITrigger_get_TimerEnabled(This,pVal)	\
    ( (This)->lpVtbl -> get_TimerEnabled(This,pVal) ) 

#define ITrigger_put_TimerEnabled(This,newVal)	\
    ( (This)->lpVtbl -> put_TimerEnabled(This,newVal) ) 

#define ITrigger_get_TimerInterval(This,pVal)	\
    ( (This)->lpVtbl -> get_TimerInterval(This,pVal) ) 

#define ITrigger_put_TimerInterval(This,newVal)	\
    ( (This)->lpVtbl -> put_TimerInterval(This,newVal) ) 

#define ITrigger_get_X(This,pVal)	\
    ( (This)->lpVtbl -> get_X(This,pVal) ) 

#define ITrigger_put_X(This,newVal)	\
    ( (This)->lpVtbl -> put_X(This,newVal) ) 

#define ITrigger_get_Y(This,pVal)	\
    ( (This)->lpVtbl -> get_Y(This,pVal) ) 

#define ITrigger_put_Y(This,newVal)	\
    ( (This)->lpVtbl -> put_Y(This,newVal) ) 

#define ITrigger_get_Enabled(This,pVal)	\
    ( (This)->lpVtbl -> get_Enabled(This,pVal) ) 

#define ITrigger_put_Enabled(This,newVal)	\
    ( (This)->lpVtbl -> put_Enabled(This,newVal) ) 

#define ITrigger_get_Visible(This,pVal)	\
    ( (This)->lpVtbl -> get_Visible(This,pVal) ) 

#define ITrigger_put_Visible(This,newVal)	\
    ( (This)->lpVtbl -> put_Visible(This,newVal) ) 

#define ITrigger_get_TriggerShape(This,pVal)	\
    ( (This)->lpVtbl -> get_TriggerShape(This,pVal) ) 

#define ITrigger_put_TriggerShape(This,newVal)	\
    ( (This)->lpVtbl -> put_TriggerShape(This,newVal) ) 

#define ITrigger_get_Surface(This,pVal)	\
    ( (This)->lpVtbl -> get_Surface(This,pVal) ) 

#define ITrigger_put_Surface(This,newVal)	\
    ( (This)->lpVtbl -> put_Surface(This,newVal) ) 

#define ITrigger_get_Name(This,pVal)	\
    ( (This)->lpVtbl -> get_Name(This,pVal) ) 

#define ITrigger_put_Name(This,newVal)	\
    ( (This)->lpVtbl -> put_Name(This,newVal) ) 

#define ITrigger_get_UserValue(This,pVal)	\
    ( (This)->lpVtbl -> get_UserValue(This,pVal) ) 

#define ITrigger_put_UserValue(This,newVal)	\
    ( (This)->lpVtbl -> put_UserValue(This,newVal) ) 

#define ITrigger_BallCntOver(This,pVal)	\
    ( (This)->lpVtbl -> BallCntOver(This,pVal) ) 

#define ITrigger_DestroyBall(This,pVal)	\
    ( (This)->lpVtbl -> DestroyBall(This,pVal) ) 

#define ITrigger_get_HitHeight(This,pVal)	\
    ( (This)->lpVtbl -> get_HitHeight(This,pVal) ) 

#define ITrigger_put_HitHeight(This,newVal)	\
    ( (This)->lpVtbl -> put_HitHeight(This,newVal) ) 

#define ITrigger_get_Material(This,pVal)	\
    ( (This)->lpVtbl -> get_Material(This,pVal) ) 

#define ITrigger_put_Material(This,newVal)	\
    ( (This)->lpVtbl -> put_Material(This,newVal) ) 

#define ITrigger_get_Rotation(This,pVal)	\
    ( (This)->lpVtbl -> get_Rotation(This,pVal) ) 

#define ITrigger_put_Rotation(This,newVal)	\
    ( (This)->lpVtbl -> put_Rotation(This,newVal) ) 

#define ITrigger_get_WireThickness(This,pVal)	\
    ( (This)->lpVtbl -> get_WireThickness(This,pVal) ) 

#define ITrigger_put_WireThickness(This,newVal)	\
    ( (This)->lpVtbl -> put_WireThickness(This,newVal) ) 

#define ITrigger_get_AnimSpeed(This,pVal)	\
    ( (This)->lpVtbl -> get_AnimSpeed(This,pVal) ) 

#define ITrigger_put_AnimSpeed(This,newVal)	\
    ( (This)->lpVtbl -> put_AnimSpeed(This,newVal) ) 

#define ITrigger_get_ReflectionEnabled(This,pVal)	\
    ( (This)->lpVtbl -> get_ReflectionEnabled(This,pVal) ) 

#define ITrigger_put_ReflectionEnabled(This,newVal)	\
    ( (This)->lpVtbl -> put_ReflectionEnabled(This,newVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ITrigger_INTERFACE_DEFINED__ */


#ifndef __ITriggerEvents_DISPINTERFACE_DEFINED__
#define __ITriggerEvents_DISPINTERFACE_DEFINED__

/* dispinterface ITriggerEvents */
/* [uuid] */ 


EXTERN_C const IID DIID_ITriggerEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("8B71CB94-698C-4839-8832-956501D3DECA")
    ITriggerEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct ITriggerEventsVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITriggerEvents * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITriggerEvents * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITriggerEvents * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITriggerEvents * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITriggerEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITriggerEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITriggerEvents * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        END_INTERFACE
    } ITriggerEventsVtbl;

    interface ITriggerEvents
    {
        CONST_VTBL struct ITriggerEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITriggerEvents_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ITriggerEvents_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ITriggerEvents_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ITriggerEvents_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define ITriggerEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define ITriggerEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define ITriggerEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* __ITriggerEvents_DISPINTERFACE_DEFINED__ */


#ifndef __ILight_INTERFACE_DEFINED__
#define __ILight_INTERFACE_DEFINED__

/* interface ILight */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_ILight;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D3013008-52AF-410B-AD03-66EB37D1C8AC")
    ILight : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Falloff( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Falloff( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_FalloffPower( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_FalloffPower( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_State( 
            /* [retval][out] */ LightState *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_State( 
            /* [in] */ LightState newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetInPlayState( 
            /* [retval][out] */ LightState *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetInPlayStateBool( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetInPlayIntensity( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Color( 
            /* [retval][out] */ OLE_COLOR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Color( 
            /* [in] */ OLE_COLOR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ColorFull( 
            /* [retval][out] */ OLE_COLOR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ColorFull( 
            /* [in] */ OLE_COLOR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TimerEnabled( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TimerEnabled( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TimerInterval( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TimerInterval( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_X( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_X( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Y( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Y( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BlinkPattern( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_BlinkPattern( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BlinkInterval( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_BlinkInterval( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Duration( 
            /* [in] */ long startState,
            /* [in] */ long newVal,
            /* [in] */ long endState) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Intensity( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Intensity( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TransmissionScale( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TransmissionScale( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_IntensityScale( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_IntensityScale( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Surface( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Surface( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Name( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_UserValue( 
            /* [retval][out] */ VARIANT *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_UserValue( 
            /* [in] */ VARIANT *newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Image( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Image( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ImageMode( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ImageMode( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DepthBias( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_DepthBias( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_FadeSpeedUp( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_FadeSpeedUp( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_FadeSpeedDown( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_FadeSpeedDown( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Bulb( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Bulb( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ShowBulbMesh( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ShowBulbMesh( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_StaticBulbMesh( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_StaticBulbMesh( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ShowReflectionOnBall( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ShowReflectionOnBall( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ScaleBulbMesh( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ScaleBulbMesh( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BulbModulateVsAdd( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_BulbModulateVsAdd( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BulbHaloHeight( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_BulbHaloHeight( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Visible( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Visible( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct ILightVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ILight * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ILight * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ILight * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ILight * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ILight * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ILight * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ILight * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        DECLSPEC_XFGVIRT(ILight, get_Falloff)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Falloff )( 
            ILight * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ILight, put_Falloff)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Falloff )( 
            ILight * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ILight, get_FalloffPower)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_FalloffPower )( 
            ILight * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ILight, put_FalloffPower)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_FalloffPower )( 
            ILight * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ILight, get_State)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_State )( 
            ILight * This,
            /* [retval][out] */ LightState *pVal);
        
        DECLSPEC_XFGVIRT(ILight, put_State)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_State )( 
            ILight * This,
            /* [in] */ LightState newVal);
        
        DECLSPEC_XFGVIRT(ILight, GetInPlayState)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetInPlayState )( 
            ILight * This,
            /* [retval][out] */ LightState *pVal);
        
        DECLSPEC_XFGVIRT(ILight, GetInPlayStateBool)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetInPlayStateBool )( 
            ILight * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(ILight, GetInPlayIntensity)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetInPlayIntensity )( 
            ILight * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ILight, get_Color)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Color )( 
            ILight * This,
            /* [retval][out] */ OLE_COLOR *pVal);
        
        DECLSPEC_XFGVIRT(ILight, put_Color)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Color )( 
            ILight * This,
            /* [in] */ OLE_COLOR newVal);
        
        DECLSPEC_XFGVIRT(ILight, get_ColorFull)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ColorFull )( 
            ILight * This,
            /* [retval][out] */ OLE_COLOR *pVal);
        
        DECLSPEC_XFGVIRT(ILight, put_ColorFull)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ColorFull )( 
            ILight * This,
            /* [in] */ OLE_COLOR newVal);
        
        DECLSPEC_XFGVIRT(ILight, get_TimerEnabled)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TimerEnabled )( 
            ILight * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(ILight, put_TimerEnabled)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TimerEnabled )( 
            ILight * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(ILight, get_TimerInterval)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TimerInterval )( 
            ILight * This,
            /* [retval][out] */ long *pVal);
        
        DECLSPEC_XFGVIRT(ILight, put_TimerInterval)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TimerInterval )( 
            ILight * This,
            /* [in] */ long newVal);
        
        DECLSPEC_XFGVIRT(ILight, get_X)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_X )( 
            ILight * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ILight, put_X)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_X )( 
            ILight * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ILight, get_Y)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Y )( 
            ILight * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ILight, put_Y)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Y )( 
            ILight * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ILight, get_BlinkPattern)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BlinkPattern )( 
            ILight * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(ILight, put_BlinkPattern)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_BlinkPattern )( 
            ILight * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(ILight, get_BlinkInterval)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BlinkInterval )( 
            ILight * This,
            /* [retval][out] */ long *pVal);
        
        DECLSPEC_XFGVIRT(ILight, put_BlinkInterval)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_BlinkInterval )( 
            ILight * This,
            /* [in] */ long newVal);
        
        DECLSPEC_XFGVIRT(ILight, Duration)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Duration )( 
            ILight * This,
            /* [in] */ long startState,
            /* [in] */ long newVal,
            /* [in] */ long endState);
        
        DECLSPEC_XFGVIRT(ILight, get_Intensity)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Intensity )( 
            ILight * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ILight, put_Intensity)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Intensity )( 
            ILight * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ILight, get_TransmissionScale)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TransmissionScale )( 
            ILight * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ILight, put_TransmissionScale)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TransmissionScale )( 
            ILight * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ILight, get_IntensityScale)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_IntensityScale )( 
            ILight * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ILight, put_IntensityScale)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_IntensityScale )( 
            ILight * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ILight, get_Surface)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Surface )( 
            ILight * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(ILight, put_Surface)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Surface )( 
            ILight * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(ILight, get_Name)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ILight * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(ILight, put_Name)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            ILight * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(ILight, get_UserValue)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_UserValue )( 
            ILight * This,
            /* [retval][out] */ VARIANT *pVal);
        
        DECLSPEC_XFGVIRT(ILight, put_UserValue)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_UserValue )( 
            ILight * This,
            /* [in] */ VARIANT *newVal);
        
        DECLSPEC_XFGVIRT(ILight, get_Image)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Image )( 
            ILight * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(ILight, put_Image)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Image )( 
            ILight * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(ILight, get_ImageMode)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ImageMode )( 
            ILight * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(ILight, put_ImageMode)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ImageMode )( 
            ILight * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(ILight, get_DepthBias)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_DepthBias )( 
            ILight * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ILight, put_DepthBias)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_DepthBias )( 
            ILight * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ILight, get_FadeSpeedUp)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_FadeSpeedUp )( 
            ILight * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ILight, put_FadeSpeedUp)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_FadeSpeedUp )( 
            ILight * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ILight, get_FadeSpeedDown)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_FadeSpeedDown )( 
            ILight * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ILight, put_FadeSpeedDown)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_FadeSpeedDown )( 
            ILight * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ILight, get_Bulb)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Bulb )( 
            ILight * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(ILight, put_Bulb)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Bulb )( 
            ILight * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(ILight, get_ShowBulbMesh)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ShowBulbMesh )( 
            ILight * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(ILight, put_ShowBulbMesh)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ShowBulbMesh )( 
            ILight * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(ILight, get_StaticBulbMesh)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_StaticBulbMesh )( 
            ILight * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(ILight, put_StaticBulbMesh)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_StaticBulbMesh )( 
            ILight * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(ILight, get_ShowReflectionOnBall)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ShowReflectionOnBall )( 
            ILight * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(ILight, put_ShowReflectionOnBall)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ShowReflectionOnBall )( 
            ILight * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(ILight, get_ScaleBulbMesh)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ScaleBulbMesh )( 
            ILight * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ILight, put_ScaleBulbMesh)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ScaleBulbMesh )( 
            ILight * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ILight, get_BulbModulateVsAdd)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BulbModulateVsAdd )( 
            ILight * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ILight, put_BulbModulateVsAdd)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_BulbModulateVsAdd )( 
            ILight * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ILight, get_BulbHaloHeight)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BulbHaloHeight )( 
            ILight * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ILight, put_BulbHaloHeight)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_BulbHaloHeight )( 
            ILight * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ILight, get_Visible)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Visible )( 
            ILight * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(ILight, put_Visible)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Visible )( 
            ILight * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        END_INTERFACE
    } ILightVtbl;

    interface ILight
    {
        CONST_VTBL struct ILightVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ILight_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ILight_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ILight_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ILight_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define ILight_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define ILight_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define ILight_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define ILight_get_Falloff(This,pVal)	\
    ( (This)->lpVtbl -> get_Falloff(This,pVal) ) 

#define ILight_put_Falloff(This,newVal)	\
    ( (This)->lpVtbl -> put_Falloff(This,newVal) ) 

#define ILight_get_FalloffPower(This,pVal)	\
    ( (This)->lpVtbl -> get_FalloffPower(This,pVal) ) 

#define ILight_put_FalloffPower(This,newVal)	\
    ( (This)->lpVtbl -> put_FalloffPower(This,newVal) ) 

#define ILight_get_State(This,pVal)	\
    ( (This)->lpVtbl -> get_State(This,pVal) ) 

#define ILight_put_State(This,newVal)	\
    ( (This)->lpVtbl -> put_State(This,newVal) ) 

#define ILight_GetInPlayState(This,pVal)	\
    ( (This)->lpVtbl -> GetInPlayState(This,pVal) ) 

#define ILight_GetInPlayStateBool(This,pVal)	\
    ( (This)->lpVtbl -> GetInPlayStateBool(This,pVal) ) 

#define ILight_GetInPlayIntensity(This,pVal)	\
    ( (This)->lpVtbl -> GetInPlayIntensity(This,pVal) ) 

#define ILight_get_Color(This,pVal)	\
    ( (This)->lpVtbl -> get_Color(This,pVal) ) 

#define ILight_put_Color(This,newVal)	\
    ( (This)->lpVtbl -> put_Color(This,newVal) ) 

#define ILight_get_ColorFull(This,pVal)	\
    ( (This)->lpVtbl -> get_ColorFull(This,pVal) ) 

#define ILight_put_ColorFull(This,newVal)	\
    ( (This)->lpVtbl -> put_ColorFull(This,newVal) ) 

#define ILight_get_TimerEnabled(This,pVal)	\
    ( (This)->lpVtbl -> get_TimerEnabled(This,pVal) ) 

#define ILight_put_TimerEnabled(This,newVal)	\
    ( (This)->lpVtbl -> put_TimerEnabled(This,newVal) ) 

#define ILight_get_TimerInterval(This,pVal)	\
    ( (This)->lpVtbl -> get_TimerInterval(This,pVal) ) 

#define ILight_put_TimerInterval(This,newVal)	\
    ( (This)->lpVtbl -> put_TimerInterval(This,newVal) ) 

#define ILight_get_X(This,pVal)	\
    ( (This)->lpVtbl -> get_X(This,pVal) ) 

#define ILight_put_X(This,newVal)	\
    ( (This)->lpVtbl -> put_X(This,newVal) ) 

#define ILight_get_Y(This,pVal)	\
    ( (This)->lpVtbl -> get_Y(This,pVal) ) 

#define ILight_put_Y(This,newVal)	\
    ( (This)->lpVtbl -> put_Y(This,newVal) ) 

#define ILight_get_BlinkPattern(This,pVal)	\
    ( (This)->lpVtbl -> get_BlinkPattern(This,pVal) ) 

#define ILight_put_BlinkPattern(This,newVal)	\
    ( (This)->lpVtbl -> put_BlinkPattern(This,newVal) ) 

#define ILight_get_BlinkInterval(This,pVal)	\
    ( (This)->lpVtbl -> get_BlinkInterval(This,pVal) ) 

#define ILight_put_BlinkInterval(This,newVal)	\
    ( (This)->lpVtbl -> put_BlinkInterval(This,newVal) ) 

#define ILight_Duration(This,startState,newVal,endState)	\
    ( (This)->lpVtbl -> Duration(This,startState,newVal,endState) ) 

#define ILight_get_Intensity(This,pVal)	\
    ( (This)->lpVtbl -> get_Intensity(This,pVal) ) 

#define ILight_put_Intensity(This,newVal)	\
    ( (This)->lpVtbl -> put_Intensity(This,newVal) ) 

#define ILight_get_TransmissionScale(This,pVal)	\
    ( (This)->lpVtbl -> get_TransmissionScale(This,pVal) ) 

#define ILight_put_TransmissionScale(This,newVal)	\
    ( (This)->lpVtbl -> put_TransmissionScale(This,newVal) ) 

#define ILight_get_IntensityScale(This,pVal)	\
    ( (This)->lpVtbl -> get_IntensityScale(This,pVal) ) 

#define ILight_put_IntensityScale(This,newVal)	\
    ( (This)->lpVtbl -> put_IntensityScale(This,newVal) ) 

#define ILight_get_Surface(This,pVal)	\
    ( (This)->lpVtbl -> get_Surface(This,pVal) ) 

#define ILight_put_Surface(This,newVal)	\
    ( (This)->lpVtbl -> put_Surface(This,newVal) ) 

#define ILight_get_Name(This,pVal)	\
    ( (This)->lpVtbl -> get_Name(This,pVal) ) 

#define ILight_put_Name(This,newVal)	\
    ( (This)->lpVtbl -> put_Name(This,newVal) ) 

#define ILight_get_UserValue(This,pVal)	\
    ( (This)->lpVtbl -> get_UserValue(This,pVal) ) 

#define ILight_put_UserValue(This,newVal)	\
    ( (This)->lpVtbl -> put_UserValue(This,newVal) ) 

#define ILight_get_Image(This,pVal)	\
    ( (This)->lpVtbl -> get_Image(This,pVal) ) 

#define ILight_put_Image(This,newVal)	\
    ( (This)->lpVtbl -> put_Image(This,newVal) ) 

#define ILight_get_ImageMode(This,pVal)	\
    ( (This)->lpVtbl -> get_ImageMode(This,pVal) ) 

#define ILight_put_ImageMode(This,newVal)	\
    ( (This)->lpVtbl -> put_ImageMode(This,newVal) ) 

#define ILight_get_DepthBias(This,pVal)	\
    ( (This)->lpVtbl -> get_DepthBias(This,pVal) ) 

#define ILight_put_DepthBias(This,newVal)	\
    ( (This)->lpVtbl -> put_DepthBias(This,newVal) ) 

#define ILight_get_FadeSpeedUp(This,pVal)	\
    ( (This)->lpVtbl -> get_FadeSpeedUp(This,pVal) ) 

#define ILight_put_FadeSpeedUp(This,newVal)	\
    ( (This)->lpVtbl -> put_FadeSpeedUp(This,newVal) ) 

#define ILight_get_FadeSpeedDown(This,pVal)	\
    ( (This)->lpVtbl -> get_FadeSpeedDown(This,pVal) ) 

#define ILight_put_FadeSpeedDown(This,newVal)	\
    ( (This)->lpVtbl -> put_FadeSpeedDown(This,newVal) ) 

#define ILight_get_Bulb(This,pVal)	\
    ( (This)->lpVtbl -> get_Bulb(This,pVal) ) 

#define ILight_put_Bulb(This,newVal)	\
    ( (This)->lpVtbl -> put_Bulb(This,newVal) ) 

#define ILight_get_ShowBulbMesh(This,pVal)	\
    ( (This)->lpVtbl -> get_ShowBulbMesh(This,pVal) ) 

#define ILight_put_ShowBulbMesh(This,newVal)	\
    ( (This)->lpVtbl -> put_ShowBulbMesh(This,newVal) ) 

#define ILight_get_StaticBulbMesh(This,pVal)	\
    ( (This)->lpVtbl -> get_StaticBulbMesh(This,pVal) ) 

#define ILight_put_StaticBulbMesh(This,newVal)	\
    ( (This)->lpVtbl -> put_StaticBulbMesh(This,newVal) ) 

#define ILight_get_ShowReflectionOnBall(This,pVal)	\
    ( (This)->lpVtbl -> get_ShowReflectionOnBall(This,pVal) ) 

#define ILight_put_ShowReflectionOnBall(This,newVal)	\
    ( (This)->lpVtbl -> put_ShowReflectionOnBall(This,newVal) ) 

#define ILight_get_ScaleBulbMesh(This,pVal)	\
    ( (This)->lpVtbl -> get_ScaleBulbMesh(This,pVal) ) 

#define ILight_put_ScaleBulbMesh(This,newVal)	\
    ( (This)->lpVtbl -> put_ScaleBulbMesh(This,newVal) ) 

#define ILight_get_BulbModulateVsAdd(This,pVal)	\
    ( (This)->lpVtbl -> get_BulbModulateVsAdd(This,pVal) ) 

#define ILight_put_BulbModulateVsAdd(This,newVal)	\
    ( (This)->lpVtbl -> put_BulbModulateVsAdd(This,newVal) ) 

#define ILight_get_BulbHaloHeight(This,pVal)	\
    ( (This)->lpVtbl -> get_BulbHaloHeight(This,pVal) ) 

#define ILight_put_BulbHaloHeight(This,newVal)	\
    ( (This)->lpVtbl -> put_BulbHaloHeight(This,newVal) ) 

#define ILight_get_Visible(This,pVal)	\
    ( (This)->lpVtbl -> get_Visible(This,pVal) ) 

#define ILight_put_Visible(This,newVal)	\
    ( (This)->lpVtbl -> put_Visible(This,newVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ILight_INTERFACE_DEFINED__ */


#ifndef __ILightEvents_DISPINTERFACE_DEFINED__
#define __ILightEvents_DISPINTERFACE_DEFINED__

/* dispinterface ILightEvents */
/* [uuid] */ 


EXTERN_C const IID DIID_ILightEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("9C07F82F-325E-482f-BC09-10F014834755")
    ILightEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct ILightEventsVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ILightEvents * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ILightEvents * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ILightEvents * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ILightEvents * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ILightEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ILightEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ILightEvents * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        END_INTERFACE
    } ILightEventsVtbl;

    interface ILightEvents
    {
        CONST_VTBL struct ILightEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ILightEvents_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ILightEvents_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ILightEvents_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ILightEvents_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define ILightEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define ILightEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define ILightEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* __ILightEvents_DISPINTERFACE_DEFINED__ */


#ifndef __IKicker_INTERFACE_DEFINED__
#define __IKicker_INTERFACE_DEFINED__

/* interface IKicker */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IKicker;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DBC4625D-6226-4AFA-8649-75B43C332041")
    IKicker : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Name( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE CreateBall( 
            /* [retval][out] */ IBall **Ball) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE CreateSizedBall( 
            /* [in] */ float radius,
            /* [retval][out] */ IBall **Ball) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE CreateSizedBallWithMass( 
            /* [in] */ float radius,
            /* [in] */ float mass,
            /* [retval][out] */ IBall **Ball) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE DestroyBall( 
            /* [retval][out] */ int *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TimerEnabled( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TimerEnabled( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TimerInterval( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TimerInterval( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Kick( 
            float angle,
            float speed,
            /* [defaultvalue] */ float inclination = 0) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE KickXYZ( 
            float angle,
            float speed,
            float inclination,
            float x,
            float y,
            float z) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE KickZ( 
            float angle,
            float speed,
            float inclination,
            float heightz) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_X( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_X( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Y( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Y( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Enabled( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Enabled( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DrawStyle( 
            /* [retval][out] */ KickerType *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_DrawStyle( 
            /* [in] */ KickerType newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Material( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Material( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Surface( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Surface( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_UserValue( 
            /* [retval][out] */ VARIANT *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_UserValue( 
            /* [in] */ VARIANT *newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE BallCntOver( 
            /* [retval][out] */ int *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Scatter( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Scatter( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_HitAccuracy( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_HitAccuracy( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_HitHeight( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_HitHeight( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Orientation( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Orientation( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Radius( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Radius( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_FallThrough( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_FallThrough( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Legacy( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Legacy( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_LastCapturedBall( 
            /* [retval][out] */ IBall **pVal) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IKickerVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IKicker * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IKicker * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IKicker * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IKicker * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IKicker * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IKicker * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IKicker * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        DECLSPEC_XFGVIRT(IKicker, get_Name)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IKicker * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IKicker, put_Name)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IKicker * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IKicker, CreateBall)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *CreateBall )( 
            IKicker * This,
            /* [retval][out] */ IBall **Ball);
        
        DECLSPEC_XFGVIRT(IKicker, CreateSizedBall)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *CreateSizedBall )( 
            IKicker * This,
            /* [in] */ float radius,
            /* [retval][out] */ IBall **Ball);
        
        DECLSPEC_XFGVIRT(IKicker, CreateSizedBallWithMass)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *CreateSizedBallWithMass )( 
            IKicker * This,
            /* [in] */ float radius,
            /* [in] */ float mass,
            /* [retval][out] */ IBall **Ball);
        
        DECLSPEC_XFGVIRT(IKicker, DestroyBall)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *DestroyBall )( 
            IKicker * This,
            /* [retval][out] */ int *pVal);
        
        DECLSPEC_XFGVIRT(IKicker, get_TimerEnabled)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TimerEnabled )( 
            IKicker * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IKicker, put_TimerEnabled)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TimerEnabled )( 
            IKicker * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IKicker, get_TimerInterval)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TimerInterval )( 
            IKicker * This,
            /* [retval][out] */ long *pVal);
        
        DECLSPEC_XFGVIRT(IKicker, put_TimerInterval)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TimerInterval )( 
            IKicker * This,
            /* [in] */ long newVal);
        
        DECLSPEC_XFGVIRT(IKicker, Kick)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Kick )( 
            IKicker * This,
            float angle,
            float speed,
            /* [defaultvalue] */ float inclination);
        
        DECLSPEC_XFGVIRT(IKicker, KickXYZ)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *KickXYZ )( 
            IKicker * This,
            float angle,
            float speed,
            float inclination,
            float x,
            float y,
            float z);
        
        DECLSPEC_XFGVIRT(IKicker, KickZ)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *KickZ )( 
            IKicker * This,
            float angle,
            float speed,
            float inclination,
            float heightz);
        
        DECLSPEC_XFGVIRT(IKicker, get_X)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_X )( 
            IKicker * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IKicker, put_X)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_X )( 
            IKicker * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IKicker, get_Y)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Y )( 
            IKicker * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IKicker, put_Y)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Y )( 
            IKicker * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IKicker, get_Enabled)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Enabled )( 
            IKicker * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IKicker, put_Enabled)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Enabled )( 
            IKicker * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IKicker, get_DrawStyle)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_DrawStyle )( 
            IKicker * This,
            /* [retval][out] */ KickerType *pVal);
        
        DECLSPEC_XFGVIRT(IKicker, put_DrawStyle)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_DrawStyle )( 
            IKicker * This,
            /* [in] */ KickerType newVal);
        
        DECLSPEC_XFGVIRT(IKicker, get_Material)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Material )( 
            IKicker * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IKicker, put_Material)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Material )( 
            IKicker * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IKicker, get_Surface)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Surface )( 
            IKicker * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IKicker, put_Surface)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Surface )( 
            IKicker * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IKicker, get_UserValue)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_UserValue )( 
            IKicker * This,
            /* [retval][out] */ VARIANT *pVal);
        
        DECLSPEC_XFGVIRT(IKicker, put_UserValue)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_UserValue )( 
            IKicker * This,
            /* [in] */ VARIANT *newVal);
        
        DECLSPEC_XFGVIRT(IKicker, BallCntOver)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *BallCntOver )( 
            IKicker * This,
            /* [retval][out] */ int *pVal);
        
        DECLSPEC_XFGVIRT(IKicker, get_Scatter)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Scatter )( 
            IKicker * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IKicker, put_Scatter)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Scatter )( 
            IKicker * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IKicker, get_HitAccuracy)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HitAccuracy )( 
            IKicker * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IKicker, put_HitAccuracy)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_HitAccuracy )( 
            IKicker * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IKicker, get_HitHeight)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HitHeight )( 
            IKicker * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IKicker, put_HitHeight)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_HitHeight )( 
            IKicker * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IKicker, get_Orientation)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Orientation )( 
            IKicker * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IKicker, put_Orientation)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Orientation )( 
            IKicker * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IKicker, get_Radius)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Radius )( 
            IKicker * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IKicker, put_Radius)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Radius )( 
            IKicker * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IKicker, get_FallThrough)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_FallThrough )( 
            IKicker * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IKicker, put_FallThrough)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_FallThrough )( 
            IKicker * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IKicker, get_Legacy)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Legacy )( 
            IKicker * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IKicker, put_Legacy)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Legacy )( 
            IKicker * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IKicker, get_LastCapturedBall)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_LastCapturedBall )( 
            IKicker * This,
            /* [retval][out] */ IBall **pVal);
        
        END_INTERFACE
    } IKickerVtbl;

    interface IKicker
    {
        CONST_VTBL struct IKickerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IKicker_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IKicker_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IKicker_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IKicker_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IKicker_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IKicker_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IKicker_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IKicker_get_Name(This,pVal)	\
    ( (This)->lpVtbl -> get_Name(This,pVal) ) 

#define IKicker_put_Name(This,newVal)	\
    ( (This)->lpVtbl -> put_Name(This,newVal) ) 

#define IKicker_CreateBall(This,Ball)	\
    ( (This)->lpVtbl -> CreateBall(This,Ball) ) 

#define IKicker_CreateSizedBall(This,radius,Ball)	\
    ( (This)->lpVtbl -> CreateSizedBall(This,radius,Ball) ) 

#define IKicker_CreateSizedBallWithMass(This,radius,mass,Ball)	\
    ( (This)->lpVtbl -> CreateSizedBallWithMass(This,radius,mass,Ball) ) 

#define IKicker_DestroyBall(This,pVal)	\
    ( (This)->lpVtbl -> DestroyBall(This,pVal) ) 

#define IKicker_get_TimerEnabled(This,pVal)	\
    ( (This)->lpVtbl -> get_TimerEnabled(This,pVal) ) 

#define IKicker_put_TimerEnabled(This,newVal)	\
    ( (This)->lpVtbl -> put_TimerEnabled(This,newVal) ) 

#define IKicker_get_TimerInterval(This,pVal)	\
    ( (This)->lpVtbl -> get_TimerInterval(This,pVal) ) 

#define IKicker_put_TimerInterval(This,newVal)	\
    ( (This)->lpVtbl -> put_TimerInterval(This,newVal) ) 

#define IKicker_Kick(This,angle,speed,inclination)	\
    ( (This)->lpVtbl -> Kick(This,angle,speed,inclination) ) 

#define IKicker_KickXYZ(This,angle,speed,inclination,x,y,z)	\
    ( (This)->lpVtbl -> KickXYZ(This,angle,speed,inclination,x,y,z) ) 

#define IKicker_KickZ(This,angle,speed,inclination,heightz)	\
    ( (This)->lpVtbl -> KickZ(This,angle,speed,inclination,heightz) ) 

#define IKicker_get_X(This,pVal)	\
    ( (This)->lpVtbl -> get_X(This,pVal) ) 

#define IKicker_put_X(This,newVal)	\
    ( (This)->lpVtbl -> put_X(This,newVal) ) 

#define IKicker_get_Y(This,pVal)	\
    ( (This)->lpVtbl -> get_Y(This,pVal) ) 

#define IKicker_put_Y(This,newVal)	\
    ( (This)->lpVtbl -> put_Y(This,newVal) ) 

#define IKicker_get_Enabled(This,pVal)	\
    ( (This)->lpVtbl -> get_Enabled(This,pVal) ) 

#define IKicker_put_Enabled(This,newVal)	\
    ( (This)->lpVtbl -> put_Enabled(This,newVal) ) 

#define IKicker_get_DrawStyle(This,pVal)	\
    ( (This)->lpVtbl -> get_DrawStyle(This,pVal) ) 

#define IKicker_put_DrawStyle(This,newVal)	\
    ( (This)->lpVtbl -> put_DrawStyle(This,newVal) ) 

#define IKicker_get_Material(This,pVal)	\
    ( (This)->lpVtbl -> get_Material(This,pVal) ) 

#define IKicker_put_Material(This,newVal)	\
    ( (This)->lpVtbl -> put_Material(This,newVal) ) 

#define IKicker_get_Surface(This,pVal)	\
    ( (This)->lpVtbl -> get_Surface(This,pVal) ) 

#define IKicker_put_Surface(This,newVal)	\
    ( (This)->lpVtbl -> put_Surface(This,newVal) ) 

#define IKicker_get_UserValue(This,pVal)	\
    ( (This)->lpVtbl -> get_UserValue(This,pVal) ) 

#define IKicker_put_UserValue(This,newVal)	\
    ( (This)->lpVtbl -> put_UserValue(This,newVal) ) 

#define IKicker_BallCntOver(This,pVal)	\
    ( (This)->lpVtbl -> BallCntOver(This,pVal) ) 

#define IKicker_get_Scatter(This,pVal)	\
    ( (This)->lpVtbl -> get_Scatter(This,pVal) ) 

#define IKicker_put_Scatter(This,newVal)	\
    ( (This)->lpVtbl -> put_Scatter(This,newVal) ) 

#define IKicker_get_HitAccuracy(This,pVal)	\
    ( (This)->lpVtbl -> get_HitAccuracy(This,pVal) ) 

#define IKicker_put_HitAccuracy(This,newVal)	\
    ( (This)->lpVtbl -> put_HitAccuracy(This,newVal) ) 

#define IKicker_get_HitHeight(This,pVal)	\
    ( (This)->lpVtbl -> get_HitHeight(This,pVal) ) 

#define IKicker_put_HitHeight(This,newVal)	\
    ( (This)->lpVtbl -> put_HitHeight(This,newVal) ) 

#define IKicker_get_Orientation(This,pVal)	\
    ( (This)->lpVtbl -> get_Orientation(This,pVal) ) 

#define IKicker_put_Orientation(This,newVal)	\
    ( (This)->lpVtbl -> put_Orientation(This,newVal) ) 

#define IKicker_get_Radius(This,pVal)	\
    ( (This)->lpVtbl -> get_Radius(This,pVal) ) 

#define IKicker_put_Radius(This,newVal)	\
    ( (This)->lpVtbl -> put_Radius(This,newVal) ) 

#define IKicker_get_FallThrough(This,pVal)	\
    ( (This)->lpVtbl -> get_FallThrough(This,pVal) ) 

#define IKicker_put_FallThrough(This,newVal)	\
    ( (This)->lpVtbl -> put_FallThrough(This,newVal) ) 

#define IKicker_get_Legacy(This,pVal)	\
    ( (This)->lpVtbl -> get_Legacy(This,pVal) ) 

#define IKicker_put_Legacy(This,newVal)	\
    ( (This)->lpVtbl -> put_Legacy(This,newVal) ) 

#define IKicker_get_LastCapturedBall(This,pVal)	\
    ( (This)->lpVtbl -> get_LastCapturedBall(This,pVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IKicker_INTERFACE_DEFINED__ */


#ifndef __IKickerEvents_DISPINTERFACE_DEFINED__
#define __IKickerEvents_DISPINTERFACE_DEFINED__

/* dispinterface IKickerEvents */
/* [uuid] */ 


EXTERN_C const IID DIID_IKickerEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("BA80F24A-78B2-4c5d-A28D-0F1A9E7C555C")
    IKickerEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct IKickerEventsVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IKickerEvents * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IKickerEvents * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IKickerEvents * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IKickerEvents * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IKickerEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IKickerEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IKickerEvents * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        END_INTERFACE
    } IKickerEventsVtbl;

    interface IKickerEvents
    {
        CONST_VTBL struct IKickerEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IKickerEvents_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IKickerEvents_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IKickerEvents_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IKickerEvents_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IKickerEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IKickerEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IKickerEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* __IKickerEvents_DISPINTERFACE_DEFINED__ */


#ifndef __IDecal_INTERFACE_DEFINED__
#define __IDecal_INTERFACE_DEFINED__

/* interface IDecal */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IDecal;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("56DDB212-2F4A-41ef-875C-C1A9CA2038C9")
    IDecal : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Rotation( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Rotation( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Image( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Image( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Width( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Width( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Height( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Height( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_X( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_X( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Y( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Y( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Type( 
            /* [retval][out] */ DecalType *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Type( 
            /* [in] */ DecalType newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Text( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Text( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SizingType( 
            /* [retval][out] */ SizingType *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SizingType( 
            /* [in] */ SizingType newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_FontColor( 
            /* [retval][out] */ OLE_COLOR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_FontColor( 
            /* [in] */ OLE_COLOR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Material( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Material( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Font( 
            /* [retval][out] */ IFontDisp **pVal) = 0;
        
        virtual /* [helpstring][id][propputref] */ HRESULT STDMETHODCALLTYPE putref_Font( 
            /* [in] */ IFontDisp *newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_HasVerticalText( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_HasVerticalText( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Surface( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Surface( 
            /* [in] */ BSTR newVal) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IDecalVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDecal * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDecal * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDecal * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IDecal * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IDecal * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IDecal * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IDecal * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        DECLSPEC_XFGVIRT(IDecal, get_Rotation)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Rotation )( 
            IDecal * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IDecal, put_Rotation)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Rotation )( 
            IDecal * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IDecal, get_Image)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Image )( 
            IDecal * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IDecal, put_Image)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Image )( 
            IDecal * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IDecal, get_Width)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Width )( 
            IDecal * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IDecal, put_Width)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Width )( 
            IDecal * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IDecal, get_Height)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Height )( 
            IDecal * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IDecal, put_Height)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Height )( 
            IDecal * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IDecal, get_X)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_X )( 
            IDecal * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IDecal, put_X)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_X )( 
            IDecal * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IDecal, get_Y)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Y )( 
            IDecal * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IDecal, put_Y)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Y )( 
            IDecal * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IDecal, get_Type)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            IDecal * This,
            /* [retval][out] */ DecalType *pVal);
        
        DECLSPEC_XFGVIRT(IDecal, put_Type)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Type )( 
            IDecal * This,
            /* [in] */ DecalType newVal);
        
        DECLSPEC_XFGVIRT(IDecal, get_Text)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Text )( 
            IDecal * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IDecal, put_Text)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Text )( 
            IDecal * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IDecal, get_SizingType)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SizingType )( 
            IDecal * This,
            /* [retval][out] */ SizingType *pVal);
        
        DECLSPEC_XFGVIRT(IDecal, put_SizingType)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_SizingType )( 
            IDecal * This,
            /* [in] */ SizingType newVal);
        
        DECLSPEC_XFGVIRT(IDecal, get_FontColor)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_FontColor )( 
            IDecal * This,
            /* [retval][out] */ OLE_COLOR *pVal);
        
        DECLSPEC_XFGVIRT(IDecal, put_FontColor)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_FontColor )( 
            IDecal * This,
            /* [in] */ OLE_COLOR newVal);
        
        DECLSPEC_XFGVIRT(IDecal, get_Material)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Material )( 
            IDecal * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IDecal, put_Material)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Material )( 
            IDecal * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IDecal, get_Font)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Font )( 
            IDecal * This,
            /* [retval][out] */ IFontDisp **pVal);
        
        DECLSPEC_XFGVIRT(IDecal, putref_Font)
        /* [helpstring][id][propputref] */ HRESULT ( STDMETHODCALLTYPE *putref_Font )( 
            IDecal * This,
            /* [in] */ IFontDisp *newVal);
        
        DECLSPEC_XFGVIRT(IDecal, get_HasVerticalText)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HasVerticalText )( 
            IDecal * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IDecal, put_HasVerticalText)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_HasVerticalText )( 
            IDecal * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IDecal, get_Surface)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Surface )( 
            IDecal * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IDecal, put_Surface)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Surface )( 
            IDecal * This,
            /* [in] */ BSTR newVal);
        
        END_INTERFACE
    } IDecalVtbl;

    interface IDecal
    {
        CONST_VTBL struct IDecalVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDecal_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IDecal_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IDecal_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IDecal_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IDecal_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IDecal_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IDecal_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IDecal_get_Rotation(This,pVal)	\
    ( (This)->lpVtbl -> get_Rotation(This,pVal) ) 

#define IDecal_put_Rotation(This,newVal)	\
    ( (This)->lpVtbl -> put_Rotation(This,newVal) ) 

#define IDecal_get_Image(This,pVal)	\
    ( (This)->lpVtbl -> get_Image(This,pVal) ) 

#define IDecal_put_Image(This,newVal)	\
    ( (This)->lpVtbl -> put_Image(This,newVal) ) 

#define IDecal_get_Width(This,pVal)	\
    ( (This)->lpVtbl -> get_Width(This,pVal) ) 

#define IDecal_put_Width(This,newVal)	\
    ( (This)->lpVtbl -> put_Width(This,newVal) ) 

#define IDecal_get_Height(This,pVal)	\
    ( (This)->lpVtbl -> get_Height(This,pVal) ) 

#define IDecal_put_Height(This,newVal)	\
    ( (This)->lpVtbl -> put_Height(This,newVal) ) 

#define IDecal_get_X(This,pVal)	\
    ( (This)->lpVtbl -> get_X(This,pVal) ) 

#define IDecal_put_X(This,newVal)	\
    ( (This)->lpVtbl -> put_X(This,newVal) ) 

#define IDecal_get_Y(This,pVal)	\
    ( (This)->lpVtbl -> get_Y(This,pVal) ) 

#define IDecal_put_Y(This,newVal)	\
    ( (This)->lpVtbl -> put_Y(This,newVal) ) 

#define IDecal_get_Type(This,pVal)	\
    ( (This)->lpVtbl -> get_Type(This,pVal) ) 

#define IDecal_put_Type(This,newVal)	\
    ( (This)->lpVtbl -> put_Type(This,newVal) ) 

#define IDecal_get_Text(This,pVal)	\
    ( (This)->lpVtbl -> get_Text(This,pVal) ) 

#define IDecal_put_Text(This,newVal)	\
    ( (This)->lpVtbl -> put_Text(This,newVal) ) 

#define IDecal_get_SizingType(This,pVal)	\
    ( (This)->lpVtbl -> get_SizingType(This,pVal) ) 

#define IDecal_put_SizingType(This,newVal)	\
    ( (This)->lpVtbl -> put_SizingType(This,newVal) ) 

#define IDecal_get_FontColor(This,pVal)	\
    ( (This)->lpVtbl -> get_FontColor(This,pVal) ) 

#define IDecal_put_FontColor(This,newVal)	\
    ( (This)->lpVtbl -> put_FontColor(This,newVal) ) 

#define IDecal_get_Material(This,pVal)	\
    ( (This)->lpVtbl -> get_Material(This,pVal) ) 

#define IDecal_put_Material(This,newVal)	\
    ( (This)->lpVtbl -> put_Material(This,newVal) ) 

#define IDecal_get_Font(This,pVal)	\
    ( (This)->lpVtbl -> get_Font(This,pVal) ) 

#define IDecal_putref_Font(This,newVal)	\
    ( (This)->lpVtbl -> putref_Font(This,newVal) ) 

#define IDecal_get_HasVerticalText(This,pVal)	\
    ( (This)->lpVtbl -> get_HasVerticalText(This,pVal) ) 

#define IDecal_put_HasVerticalText(This,newVal)	\
    ( (This)->lpVtbl -> put_HasVerticalText(This,newVal) ) 

#define IDecal_get_Surface(This,pVal)	\
    ( (This)->lpVtbl -> get_Surface(This,pVal) ) 

#define IDecal_put_Surface(This,newVal)	\
    ( (This)->lpVtbl -> put_Surface(This,newVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IDecal_INTERFACE_DEFINED__ */


#ifndef __IPrimitive_INTERFACE_DEFINED__
#define __IPrimitive_INTERFACE_DEFINED__

/* interface IPrimitive */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IPrimitive;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5E972141-B842-43C8-BA84-B99DE34B2E73")
    IPrimitive : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE PlayAnim( 
            /* [in] */ float startFrame,
            /* [in] */ float speed) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE PlayAnimEndless( 
            /* [in] */ float speed) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE StopAnim( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ShowFrame( 
            /* [in] */ float frame) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ContinueAnim( 
            /* [in] */ float speed) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DisplayTexture( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_DisplayTexture( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Sides( 
            /* [retval][out] */ int *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Sides( 
            /* [in] */ int newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Visible( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Visible( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Material( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Material( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SideColor( 
            /* [retval][out] */ OLE_COLOR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SideColor( 
            /* [in] */ OLE_COLOR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DrawTexturesInside( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_DrawTexturesInside( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_UserValue( 
            /* [retval][out] */ VARIANT *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_UserValue( 
            /* [in] */ VARIANT *newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Image( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Image( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_NormalMap( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_NormalMap( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_X( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_X( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Y( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Y( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Z( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Z( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Size_X( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Size_X( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Size_Y( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Size_Y( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Size_Z( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Size_Z( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RotAndTra0( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_RotAndTra0( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RotX( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_RotX( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RotAndTra1( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_RotAndTra1( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RotY( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_RotY( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RotAndTra2( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_RotAndTra2( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RotZ( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_RotZ( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RotAndTra3( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_RotAndTra3( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TransX( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TransX( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RotAndTra4( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_RotAndTra4( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TransY( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TransY( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RotAndTra5( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_RotAndTra5( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TransZ( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TransZ( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RotAndTra6( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_RotAndTra6( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ObjRotX( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ObjRotX( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RotAndTra7( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_RotAndTra7( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ObjRotY( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ObjRotY( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RotAndTra8( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_RotAndTra8( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ObjRotZ( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ObjRotZ( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_EdgeFactorUI( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_EdgeFactorUI( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CollisionReductionFactor( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_CollisionReductionFactor( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Name( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_EnableStaticRendering( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_EnableStaticRendering( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Elasticity( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Elasticity( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ElasticityFalloff( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ElasticityFalloff( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Collidable( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Collidable( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_IsToy( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_IsToy( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BackfacesEnabled( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_BackfacesEnabled( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_HasHitEvent( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_HasHitEvent( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Threshold( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Threshold( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Friction( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Friction( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Scatter( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Scatter( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DisableLighting( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_DisableLighting( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BlendDisableLighting( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_BlendDisableLighting( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BlendDisableLightingFromBelow( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_BlendDisableLightingFromBelow( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ReflectionEnabled( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ReflectionEnabled( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Opacity( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Opacity( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AddBlend( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_AddBlend( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Color( 
            /* [retval][out] */ OLE_COLOR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Color( 
            /* [in] */ OLE_COLOR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DepthBias( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_DepthBias( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_PhysicsMaterial( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_PhysicsMaterial( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_OverwritePhysics( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_OverwritePhysics( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_HitThreshold( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ObjectSpaceNormalMap( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ObjectSpaceNormalMap( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IPrimitiveVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPrimitive * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPrimitive * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPrimitive * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IPrimitive * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IPrimitive * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IPrimitive * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IPrimitive * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        DECLSPEC_XFGVIRT(IPrimitive, PlayAnim)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *PlayAnim )( 
            IPrimitive * This,
            /* [in] */ float startFrame,
            /* [in] */ float speed);
        
        DECLSPEC_XFGVIRT(IPrimitive, PlayAnimEndless)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *PlayAnimEndless )( 
            IPrimitive * This,
            /* [in] */ float speed);
        
        DECLSPEC_XFGVIRT(IPrimitive, StopAnim)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *StopAnim )( 
            IPrimitive * This);
        
        DECLSPEC_XFGVIRT(IPrimitive, ShowFrame)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *ShowFrame )( 
            IPrimitive * This,
            /* [in] */ float frame);
        
        DECLSPEC_XFGVIRT(IPrimitive, ContinueAnim)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *ContinueAnim )( 
            IPrimitive * This,
            /* [in] */ float speed);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_DisplayTexture)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_DisplayTexture )( 
            IPrimitive * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_DisplayTexture)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_DisplayTexture )( 
            IPrimitive * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_Sides)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Sides )( 
            IPrimitive * This,
            /* [retval][out] */ int *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_Sides)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Sides )( 
            IPrimitive * This,
            /* [in] */ int newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_Visible)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Visible )( 
            IPrimitive * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_Visible)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Visible )( 
            IPrimitive * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_Material)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Material )( 
            IPrimitive * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_Material)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Material )( 
            IPrimitive * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_SideColor)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SideColor )( 
            IPrimitive * This,
            /* [retval][out] */ OLE_COLOR *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_SideColor)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_SideColor )( 
            IPrimitive * This,
            /* [in] */ OLE_COLOR newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_DrawTexturesInside)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_DrawTexturesInside )( 
            IPrimitive * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_DrawTexturesInside)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_DrawTexturesInside )( 
            IPrimitive * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_UserValue)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_UserValue )( 
            IPrimitive * This,
            /* [retval][out] */ VARIANT *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_UserValue)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_UserValue )( 
            IPrimitive * This,
            /* [in] */ VARIANT *newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_Image)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Image )( 
            IPrimitive * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_Image)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Image )( 
            IPrimitive * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_NormalMap)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_NormalMap )( 
            IPrimitive * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_NormalMap)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_NormalMap )( 
            IPrimitive * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_X)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_X )( 
            IPrimitive * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_X)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_X )( 
            IPrimitive * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_Y)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Y )( 
            IPrimitive * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_Y)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Y )( 
            IPrimitive * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_Z)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Z )( 
            IPrimitive * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_Z)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Z )( 
            IPrimitive * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_Size_X)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Size_X )( 
            IPrimitive * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_Size_X)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Size_X )( 
            IPrimitive * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_Size_Y)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Size_Y )( 
            IPrimitive * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_Size_Y)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Size_Y )( 
            IPrimitive * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_Size_Z)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Size_Z )( 
            IPrimitive * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_Size_Z)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Size_Z )( 
            IPrimitive * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_RotAndTra0)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RotAndTra0 )( 
            IPrimitive * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_RotAndTra0)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_RotAndTra0 )( 
            IPrimitive * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_RotX)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RotX )( 
            IPrimitive * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_RotX)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_RotX )( 
            IPrimitive * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_RotAndTra1)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RotAndTra1 )( 
            IPrimitive * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_RotAndTra1)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_RotAndTra1 )( 
            IPrimitive * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_RotY)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RotY )( 
            IPrimitive * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_RotY)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_RotY )( 
            IPrimitive * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_RotAndTra2)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RotAndTra2 )( 
            IPrimitive * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_RotAndTra2)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_RotAndTra2 )( 
            IPrimitive * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_RotZ)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RotZ )( 
            IPrimitive * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_RotZ)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_RotZ )( 
            IPrimitive * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_RotAndTra3)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RotAndTra3 )( 
            IPrimitive * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_RotAndTra3)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_RotAndTra3 )( 
            IPrimitive * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_TransX)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TransX )( 
            IPrimitive * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_TransX)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TransX )( 
            IPrimitive * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_RotAndTra4)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RotAndTra4 )( 
            IPrimitive * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_RotAndTra4)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_RotAndTra4 )( 
            IPrimitive * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_TransY)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TransY )( 
            IPrimitive * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_TransY)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TransY )( 
            IPrimitive * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_RotAndTra5)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RotAndTra5 )( 
            IPrimitive * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_RotAndTra5)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_RotAndTra5 )( 
            IPrimitive * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_TransZ)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TransZ )( 
            IPrimitive * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_TransZ)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TransZ )( 
            IPrimitive * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_RotAndTra6)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RotAndTra6 )( 
            IPrimitive * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_RotAndTra6)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_RotAndTra6 )( 
            IPrimitive * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_ObjRotX)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ObjRotX )( 
            IPrimitive * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_ObjRotX)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ObjRotX )( 
            IPrimitive * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_RotAndTra7)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RotAndTra7 )( 
            IPrimitive * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_RotAndTra7)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_RotAndTra7 )( 
            IPrimitive * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_ObjRotY)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ObjRotY )( 
            IPrimitive * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_ObjRotY)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ObjRotY )( 
            IPrimitive * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_RotAndTra8)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RotAndTra8 )( 
            IPrimitive * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_RotAndTra8)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_RotAndTra8 )( 
            IPrimitive * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_ObjRotZ)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ObjRotZ )( 
            IPrimitive * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_ObjRotZ)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ObjRotZ )( 
            IPrimitive * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_EdgeFactorUI)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_EdgeFactorUI )( 
            IPrimitive * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_EdgeFactorUI)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_EdgeFactorUI )( 
            IPrimitive * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_CollisionReductionFactor)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CollisionReductionFactor )( 
            IPrimitive * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_CollisionReductionFactor)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_CollisionReductionFactor )( 
            IPrimitive * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_Name)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IPrimitive * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_Name)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IPrimitive * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_EnableStaticRendering)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_EnableStaticRendering )( 
            IPrimitive * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_EnableStaticRendering)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_EnableStaticRendering )( 
            IPrimitive * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_Elasticity)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Elasticity )( 
            IPrimitive * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_Elasticity)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Elasticity )( 
            IPrimitive * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_ElasticityFalloff)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ElasticityFalloff )( 
            IPrimitive * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_ElasticityFalloff)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ElasticityFalloff )( 
            IPrimitive * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_Collidable)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Collidable )( 
            IPrimitive * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_Collidable)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Collidable )( 
            IPrimitive * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_IsToy)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_IsToy )( 
            IPrimitive * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_IsToy)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_IsToy )( 
            IPrimitive * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_BackfacesEnabled)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BackfacesEnabled )( 
            IPrimitive * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_BackfacesEnabled)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_BackfacesEnabled )( 
            IPrimitive * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_HasHitEvent)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HasHitEvent )( 
            IPrimitive * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_HasHitEvent)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_HasHitEvent )( 
            IPrimitive * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_Threshold)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Threshold )( 
            IPrimitive * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_Threshold)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Threshold )( 
            IPrimitive * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_Friction)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Friction )( 
            IPrimitive * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_Friction)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Friction )( 
            IPrimitive * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_Scatter)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Scatter )( 
            IPrimitive * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_Scatter)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Scatter )( 
            IPrimitive * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_DisableLighting)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_DisableLighting )( 
            IPrimitive * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_DisableLighting)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_DisableLighting )( 
            IPrimitive * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_BlendDisableLighting)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BlendDisableLighting )( 
            IPrimitive * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_BlendDisableLighting)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_BlendDisableLighting )( 
            IPrimitive * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_BlendDisableLightingFromBelow)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BlendDisableLightingFromBelow )( 
            IPrimitive * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_BlendDisableLightingFromBelow)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_BlendDisableLightingFromBelow )( 
            IPrimitive * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_ReflectionEnabled)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ReflectionEnabled )( 
            IPrimitive * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_ReflectionEnabled)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ReflectionEnabled )( 
            IPrimitive * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_Opacity)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Opacity )( 
            IPrimitive * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_Opacity)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Opacity )( 
            IPrimitive * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_AddBlend)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AddBlend )( 
            IPrimitive * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_AddBlend)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_AddBlend )( 
            IPrimitive * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_Color)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Color )( 
            IPrimitive * This,
            /* [retval][out] */ OLE_COLOR *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_Color)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Color )( 
            IPrimitive * This,
            /* [in] */ OLE_COLOR newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_DepthBias)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_DepthBias )( 
            IPrimitive * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_DepthBias)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_DepthBias )( 
            IPrimitive * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_PhysicsMaterial)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_PhysicsMaterial )( 
            IPrimitive * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_PhysicsMaterial)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_PhysicsMaterial )( 
            IPrimitive * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_OverwritePhysics)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_OverwritePhysics )( 
            IPrimitive * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_OverwritePhysics)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_OverwritePhysics )( 
            IPrimitive * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_HitThreshold)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HitThreshold )( 
            IPrimitive * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, get_ObjectSpaceNormalMap)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ObjectSpaceNormalMap )( 
            IPrimitive * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IPrimitive, put_ObjectSpaceNormalMap)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ObjectSpaceNormalMap )( 
            IPrimitive * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        END_INTERFACE
    } IPrimitiveVtbl;

    interface IPrimitive
    {
        CONST_VTBL struct IPrimitiveVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPrimitive_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IPrimitive_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IPrimitive_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IPrimitive_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IPrimitive_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IPrimitive_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IPrimitive_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IPrimitive_PlayAnim(This,startFrame,speed)	\
    ( (This)->lpVtbl -> PlayAnim(This,startFrame,speed) ) 

#define IPrimitive_PlayAnimEndless(This,speed)	\
    ( (This)->lpVtbl -> PlayAnimEndless(This,speed) ) 

#define IPrimitive_StopAnim(This)	\
    ( (This)->lpVtbl -> StopAnim(This) ) 

#define IPrimitive_ShowFrame(This,frame)	\
    ( (This)->lpVtbl -> ShowFrame(This,frame) ) 

#define IPrimitive_ContinueAnim(This,speed)	\
    ( (This)->lpVtbl -> ContinueAnim(This,speed) ) 

#define IPrimitive_get_DisplayTexture(This,pVal)	\
    ( (This)->lpVtbl -> get_DisplayTexture(This,pVal) ) 

#define IPrimitive_put_DisplayTexture(This,newVal)	\
    ( (This)->lpVtbl -> put_DisplayTexture(This,newVal) ) 

#define IPrimitive_get_Sides(This,pVal)	\
    ( (This)->lpVtbl -> get_Sides(This,pVal) ) 

#define IPrimitive_put_Sides(This,newVal)	\
    ( (This)->lpVtbl -> put_Sides(This,newVal) ) 

#define IPrimitive_get_Visible(This,pVal)	\
    ( (This)->lpVtbl -> get_Visible(This,pVal) ) 

#define IPrimitive_put_Visible(This,newVal)	\
    ( (This)->lpVtbl -> put_Visible(This,newVal) ) 

#define IPrimitive_get_Material(This,pVal)	\
    ( (This)->lpVtbl -> get_Material(This,pVal) ) 

#define IPrimitive_put_Material(This,newVal)	\
    ( (This)->lpVtbl -> put_Material(This,newVal) ) 

#define IPrimitive_get_SideColor(This,pVal)	\
    ( (This)->lpVtbl -> get_SideColor(This,pVal) ) 

#define IPrimitive_put_SideColor(This,newVal)	\
    ( (This)->lpVtbl -> put_SideColor(This,newVal) ) 

#define IPrimitive_get_DrawTexturesInside(This,pVal)	\
    ( (This)->lpVtbl -> get_DrawTexturesInside(This,pVal) ) 

#define IPrimitive_put_DrawTexturesInside(This,newVal)	\
    ( (This)->lpVtbl -> put_DrawTexturesInside(This,newVal) ) 

#define IPrimitive_get_UserValue(This,pVal)	\
    ( (This)->lpVtbl -> get_UserValue(This,pVal) ) 

#define IPrimitive_put_UserValue(This,newVal)	\
    ( (This)->lpVtbl -> put_UserValue(This,newVal) ) 

#define IPrimitive_get_Image(This,pVal)	\
    ( (This)->lpVtbl -> get_Image(This,pVal) ) 

#define IPrimitive_put_Image(This,newVal)	\
    ( (This)->lpVtbl -> put_Image(This,newVal) ) 

#define IPrimitive_get_NormalMap(This,pVal)	\
    ( (This)->lpVtbl -> get_NormalMap(This,pVal) ) 

#define IPrimitive_put_NormalMap(This,newVal)	\
    ( (This)->lpVtbl -> put_NormalMap(This,newVal) ) 

#define IPrimitive_get_X(This,pVal)	\
    ( (This)->lpVtbl -> get_X(This,pVal) ) 

#define IPrimitive_put_X(This,newVal)	\
    ( (This)->lpVtbl -> put_X(This,newVal) ) 

#define IPrimitive_get_Y(This,pVal)	\
    ( (This)->lpVtbl -> get_Y(This,pVal) ) 

#define IPrimitive_put_Y(This,newVal)	\
    ( (This)->lpVtbl -> put_Y(This,newVal) ) 

#define IPrimitive_get_Z(This,pVal)	\
    ( (This)->lpVtbl -> get_Z(This,pVal) ) 

#define IPrimitive_put_Z(This,newVal)	\
    ( (This)->lpVtbl -> put_Z(This,newVal) ) 

#define IPrimitive_get_Size_X(This,pVal)	\
    ( (This)->lpVtbl -> get_Size_X(This,pVal) ) 

#define IPrimitive_put_Size_X(This,newVal)	\
    ( (This)->lpVtbl -> put_Size_X(This,newVal) ) 

#define IPrimitive_get_Size_Y(This,pVal)	\
    ( (This)->lpVtbl -> get_Size_Y(This,pVal) ) 

#define IPrimitive_put_Size_Y(This,newVal)	\
    ( (This)->lpVtbl -> put_Size_Y(This,newVal) ) 

#define IPrimitive_get_Size_Z(This,pVal)	\
    ( (This)->lpVtbl -> get_Size_Z(This,pVal) ) 

#define IPrimitive_put_Size_Z(This,newVal)	\
    ( (This)->lpVtbl -> put_Size_Z(This,newVal) ) 

#define IPrimitive_get_RotAndTra0(This,pVal)	\
    ( (This)->lpVtbl -> get_RotAndTra0(This,pVal) ) 

#define IPrimitive_put_RotAndTra0(This,newVal)	\
    ( (This)->lpVtbl -> put_RotAndTra0(This,newVal) ) 

#define IPrimitive_get_RotX(This,pVal)	\
    ( (This)->lpVtbl -> get_RotX(This,pVal) ) 

#define IPrimitive_put_RotX(This,newVal)	\
    ( (This)->lpVtbl -> put_RotX(This,newVal) ) 

#define IPrimitive_get_RotAndTra1(This,pVal)	\
    ( (This)->lpVtbl -> get_RotAndTra1(This,pVal) ) 

#define IPrimitive_put_RotAndTra1(This,newVal)	\
    ( (This)->lpVtbl -> put_RotAndTra1(This,newVal) ) 

#define IPrimitive_get_RotY(This,pVal)	\
    ( (This)->lpVtbl -> get_RotY(This,pVal) ) 

#define IPrimitive_put_RotY(This,newVal)	\
    ( (This)->lpVtbl -> put_RotY(This,newVal) ) 

#define IPrimitive_get_RotAndTra2(This,pVal)	\
    ( (This)->lpVtbl -> get_RotAndTra2(This,pVal) ) 

#define IPrimitive_put_RotAndTra2(This,newVal)	\
    ( (This)->lpVtbl -> put_RotAndTra2(This,newVal) ) 

#define IPrimitive_get_RotZ(This,pVal)	\
    ( (This)->lpVtbl -> get_RotZ(This,pVal) ) 

#define IPrimitive_put_RotZ(This,newVal)	\
    ( (This)->lpVtbl -> put_RotZ(This,newVal) ) 

#define IPrimitive_get_RotAndTra3(This,pVal)	\
    ( (This)->lpVtbl -> get_RotAndTra3(This,pVal) ) 

#define IPrimitive_put_RotAndTra3(This,newVal)	\
    ( (This)->lpVtbl -> put_RotAndTra3(This,newVal) ) 

#define IPrimitive_get_TransX(This,pVal)	\
    ( (This)->lpVtbl -> get_TransX(This,pVal) ) 

#define IPrimitive_put_TransX(This,newVal)	\
    ( (This)->lpVtbl -> put_TransX(This,newVal) ) 

#define IPrimitive_get_RotAndTra4(This,pVal)	\
    ( (This)->lpVtbl -> get_RotAndTra4(This,pVal) ) 

#define IPrimitive_put_RotAndTra4(This,newVal)	\
    ( (This)->lpVtbl -> put_RotAndTra4(This,newVal) ) 

#define IPrimitive_get_TransY(This,pVal)	\
    ( (This)->lpVtbl -> get_TransY(This,pVal) ) 

#define IPrimitive_put_TransY(This,newVal)	\
    ( (This)->lpVtbl -> put_TransY(This,newVal) ) 

#define IPrimitive_get_RotAndTra5(This,pVal)	\
    ( (This)->lpVtbl -> get_RotAndTra5(This,pVal) ) 

#define IPrimitive_put_RotAndTra5(This,newVal)	\
    ( (This)->lpVtbl -> put_RotAndTra5(This,newVal) ) 

#define IPrimitive_get_TransZ(This,pVal)	\
    ( (This)->lpVtbl -> get_TransZ(This,pVal) ) 

#define IPrimitive_put_TransZ(This,newVal)	\
    ( (This)->lpVtbl -> put_TransZ(This,newVal) ) 

#define IPrimitive_get_RotAndTra6(This,pVal)	\
    ( (This)->lpVtbl -> get_RotAndTra6(This,pVal) ) 

#define IPrimitive_put_RotAndTra6(This,newVal)	\
    ( (This)->lpVtbl -> put_RotAndTra6(This,newVal) ) 

#define IPrimitive_get_ObjRotX(This,pVal)	\
    ( (This)->lpVtbl -> get_ObjRotX(This,pVal) ) 

#define IPrimitive_put_ObjRotX(This,newVal)	\
    ( (This)->lpVtbl -> put_ObjRotX(This,newVal) ) 

#define IPrimitive_get_RotAndTra7(This,pVal)	\
    ( (This)->lpVtbl -> get_RotAndTra7(This,pVal) ) 

#define IPrimitive_put_RotAndTra7(This,newVal)	\
    ( (This)->lpVtbl -> put_RotAndTra7(This,newVal) ) 

#define IPrimitive_get_ObjRotY(This,pVal)	\
    ( (This)->lpVtbl -> get_ObjRotY(This,pVal) ) 

#define IPrimitive_put_ObjRotY(This,newVal)	\
    ( (This)->lpVtbl -> put_ObjRotY(This,newVal) ) 

#define IPrimitive_get_RotAndTra8(This,pVal)	\
    ( (This)->lpVtbl -> get_RotAndTra8(This,pVal) ) 

#define IPrimitive_put_RotAndTra8(This,newVal)	\
    ( (This)->lpVtbl -> put_RotAndTra8(This,newVal) ) 

#define IPrimitive_get_ObjRotZ(This,pVal)	\
    ( (This)->lpVtbl -> get_ObjRotZ(This,pVal) ) 

#define IPrimitive_put_ObjRotZ(This,newVal)	\
    ( (This)->lpVtbl -> put_ObjRotZ(This,newVal) ) 

#define IPrimitive_get_EdgeFactorUI(This,pVal)	\
    ( (This)->lpVtbl -> get_EdgeFactorUI(This,pVal) ) 

#define IPrimitive_put_EdgeFactorUI(This,newVal)	\
    ( (This)->lpVtbl -> put_EdgeFactorUI(This,newVal) ) 

#define IPrimitive_get_CollisionReductionFactor(This,pVal)	\
    ( (This)->lpVtbl -> get_CollisionReductionFactor(This,pVal) ) 

#define IPrimitive_put_CollisionReductionFactor(This,newVal)	\
    ( (This)->lpVtbl -> put_CollisionReductionFactor(This,newVal) ) 

#define IPrimitive_get_Name(This,pVal)	\
    ( (This)->lpVtbl -> get_Name(This,pVal) ) 

#define IPrimitive_put_Name(This,newVal)	\
    ( (This)->lpVtbl -> put_Name(This,newVal) ) 

#define IPrimitive_get_EnableStaticRendering(This,pVal)	\
    ( (This)->lpVtbl -> get_EnableStaticRendering(This,pVal) ) 

#define IPrimitive_put_EnableStaticRendering(This,newVal)	\
    ( (This)->lpVtbl -> put_EnableStaticRendering(This,newVal) ) 

#define IPrimitive_get_Elasticity(This,pVal)	\
    ( (This)->lpVtbl -> get_Elasticity(This,pVal) ) 

#define IPrimitive_put_Elasticity(This,newVal)	\
    ( (This)->lpVtbl -> put_Elasticity(This,newVal) ) 

#define IPrimitive_get_ElasticityFalloff(This,pVal)	\
    ( (This)->lpVtbl -> get_ElasticityFalloff(This,pVal) ) 

#define IPrimitive_put_ElasticityFalloff(This,newVal)	\
    ( (This)->lpVtbl -> put_ElasticityFalloff(This,newVal) ) 

#define IPrimitive_get_Collidable(This,pVal)	\
    ( (This)->lpVtbl -> get_Collidable(This,pVal) ) 

#define IPrimitive_put_Collidable(This,newVal)	\
    ( (This)->lpVtbl -> put_Collidable(This,newVal) ) 

#define IPrimitive_get_IsToy(This,pVal)	\
    ( (This)->lpVtbl -> get_IsToy(This,pVal) ) 

#define IPrimitive_put_IsToy(This,newVal)	\
    ( (This)->lpVtbl -> put_IsToy(This,newVal) ) 

#define IPrimitive_get_BackfacesEnabled(This,pVal)	\
    ( (This)->lpVtbl -> get_BackfacesEnabled(This,pVal) ) 

#define IPrimitive_put_BackfacesEnabled(This,newVal)	\
    ( (This)->lpVtbl -> put_BackfacesEnabled(This,newVal) ) 

#define IPrimitive_get_HasHitEvent(This,pVal)	\
    ( (This)->lpVtbl -> get_HasHitEvent(This,pVal) ) 

#define IPrimitive_put_HasHitEvent(This,newVal)	\
    ( (This)->lpVtbl -> put_HasHitEvent(This,newVal) ) 

#define IPrimitive_get_Threshold(This,pVal)	\
    ( (This)->lpVtbl -> get_Threshold(This,pVal) ) 

#define IPrimitive_put_Threshold(This,newVal)	\
    ( (This)->lpVtbl -> put_Threshold(This,newVal) ) 

#define IPrimitive_get_Friction(This,pVal)	\
    ( (This)->lpVtbl -> get_Friction(This,pVal) ) 

#define IPrimitive_put_Friction(This,newVal)	\
    ( (This)->lpVtbl -> put_Friction(This,newVal) ) 

#define IPrimitive_get_Scatter(This,pVal)	\
    ( (This)->lpVtbl -> get_Scatter(This,pVal) ) 

#define IPrimitive_put_Scatter(This,newVal)	\
    ( (This)->lpVtbl -> put_Scatter(This,newVal) ) 

#define IPrimitive_get_DisableLighting(This,pVal)	\
    ( (This)->lpVtbl -> get_DisableLighting(This,pVal) ) 

#define IPrimitive_put_DisableLighting(This,newVal)	\
    ( (This)->lpVtbl -> put_DisableLighting(This,newVal) ) 

#define IPrimitive_get_BlendDisableLighting(This,pVal)	\
    ( (This)->lpVtbl -> get_BlendDisableLighting(This,pVal) ) 

#define IPrimitive_put_BlendDisableLighting(This,newVal)	\
    ( (This)->lpVtbl -> put_BlendDisableLighting(This,newVal) ) 

#define IPrimitive_get_BlendDisableLightingFromBelow(This,pVal)	\
    ( (This)->lpVtbl -> get_BlendDisableLightingFromBelow(This,pVal) ) 

#define IPrimitive_put_BlendDisableLightingFromBelow(This,newVal)	\
    ( (This)->lpVtbl -> put_BlendDisableLightingFromBelow(This,newVal) ) 

#define IPrimitive_get_ReflectionEnabled(This,pVal)	\
    ( (This)->lpVtbl -> get_ReflectionEnabled(This,pVal) ) 

#define IPrimitive_put_ReflectionEnabled(This,newVal)	\
    ( (This)->lpVtbl -> put_ReflectionEnabled(This,newVal) ) 

#define IPrimitive_get_Opacity(This,pVal)	\
    ( (This)->lpVtbl -> get_Opacity(This,pVal) ) 

#define IPrimitive_put_Opacity(This,newVal)	\
    ( (This)->lpVtbl -> put_Opacity(This,newVal) ) 

#define IPrimitive_get_AddBlend(This,pVal)	\
    ( (This)->lpVtbl -> get_AddBlend(This,pVal) ) 

#define IPrimitive_put_AddBlend(This,newVal)	\
    ( (This)->lpVtbl -> put_AddBlend(This,newVal) ) 

#define IPrimitive_get_Color(This,pVal)	\
    ( (This)->lpVtbl -> get_Color(This,pVal) ) 

#define IPrimitive_put_Color(This,newVal)	\
    ( (This)->lpVtbl -> put_Color(This,newVal) ) 

#define IPrimitive_get_DepthBias(This,pVal)	\
    ( (This)->lpVtbl -> get_DepthBias(This,pVal) ) 

#define IPrimitive_put_DepthBias(This,newVal)	\
    ( (This)->lpVtbl -> put_DepthBias(This,newVal) ) 

#define IPrimitive_get_PhysicsMaterial(This,pVal)	\
    ( (This)->lpVtbl -> get_PhysicsMaterial(This,pVal) ) 

#define IPrimitive_put_PhysicsMaterial(This,newVal)	\
    ( (This)->lpVtbl -> put_PhysicsMaterial(This,newVal) ) 

#define IPrimitive_get_OverwritePhysics(This,pVal)	\
    ( (This)->lpVtbl -> get_OverwritePhysics(This,pVal) ) 

#define IPrimitive_put_OverwritePhysics(This,newVal)	\
    ( (This)->lpVtbl -> put_OverwritePhysics(This,newVal) ) 

#define IPrimitive_get_HitThreshold(This,pVal)	\
    ( (This)->lpVtbl -> get_HitThreshold(This,pVal) ) 

#define IPrimitive_get_ObjectSpaceNormalMap(This,pVal)	\
    ( (This)->lpVtbl -> get_ObjectSpaceNormalMap(This,pVal) ) 

#define IPrimitive_put_ObjectSpaceNormalMap(This,newVal)	\
    ( (This)->lpVtbl -> put_ObjectSpaceNormalMap(This,newVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IPrimitive_INTERFACE_DEFINED__ */


#ifndef __IPrimitiveEvents_DISPINTERFACE_DEFINED__
#define __IPrimitiveEvents_DISPINTERFACE_DEFINED__

/* dispinterface IPrimitiveEvents */
/* [uuid] */ 


EXTERN_C const IID DIID_IPrimitiveEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("99BDCE95-9BD2-42D6-B1F9-1DF8317932C0")
    IPrimitiveEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct IPrimitiveEventsVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPrimitiveEvents * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPrimitiveEvents * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPrimitiveEvents * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IPrimitiveEvents * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IPrimitiveEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IPrimitiveEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IPrimitiveEvents * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        END_INTERFACE
    } IPrimitiveEventsVtbl;

    interface IPrimitiveEvents
    {
        CONST_VTBL struct IPrimitiveEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPrimitiveEvents_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IPrimitiveEvents_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IPrimitiveEvents_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IPrimitiveEvents_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IPrimitiveEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IPrimitiveEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IPrimitiveEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* __IPrimitiveEvents_DISPINTERFACE_DEFINED__ */


#ifndef __IHitTarget_INTERFACE_DEFINED__
#define __IHitTarget_INTERFACE_DEFINED__

/* interface IHitTarget */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IHitTarget;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A67DE998-7D97-4E03-BE91-55BFD3A48DB6")
    IHitTarget : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TimerEnabled( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TimerEnabled( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TimerInterval( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TimerInterval( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Visible( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Visible( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Material( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Material( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_UserValue( 
            /* [retval][out] */ VARIANT *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_UserValue( 
            /* [in] */ VARIANT *newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Image( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Image( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_X( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_X( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Y( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Y( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Z( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Z( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ScaleX( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ScaleX( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ScaleY( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ScaleY( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ScaleZ( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ScaleZ( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Orientation( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Orientation( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Name( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Elasticity( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Elasticity( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ElasticityFalloff( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ElasticityFalloff( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Collidable( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Collidable( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_HasHitEvent( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_HasHitEvent( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Threshold( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Threshold( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Friction( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Friction( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Scatter( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Scatter( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DisableLighting( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_DisableLighting( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BlendDisableLighting( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_BlendDisableLighting( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BlendDisableLightingFromBelow( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_BlendDisableLightingFromBelow( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ReflectionEnabled( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ReflectionEnabled( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DepthBias( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_DepthBias( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DropSpeed( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_DropSpeed( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_IsDropped( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_IsDropped( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DrawStyle( 
            /* [retval][out] */ TargetType *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_DrawStyle( 
            /* [in] */ TargetType newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_LegacyMode( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_LegacyMode( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RaiseDelay( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_RaiseDelay( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_PhysicsMaterial( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_PhysicsMaterial( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_OverwritePhysics( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_OverwritePhysics( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_HitThreshold( 
            /* [retval][out] */ float *pVal) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IHitTargetVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IHitTarget * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IHitTarget * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IHitTarget * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IHitTarget * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IHitTarget * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IHitTarget * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IHitTarget * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        DECLSPEC_XFGVIRT(IHitTarget, get_TimerEnabled)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TimerEnabled )( 
            IHitTarget * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, put_TimerEnabled)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TimerEnabled )( 
            IHitTarget * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, get_TimerInterval)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TimerInterval )( 
            IHitTarget * This,
            /* [retval][out] */ long *pVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, put_TimerInterval)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TimerInterval )( 
            IHitTarget * This,
            /* [in] */ long newVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, get_Visible)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Visible )( 
            IHitTarget * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, put_Visible)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Visible )( 
            IHitTarget * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, get_Material)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Material )( 
            IHitTarget * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, put_Material)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Material )( 
            IHitTarget * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, get_UserValue)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_UserValue )( 
            IHitTarget * This,
            /* [retval][out] */ VARIANT *pVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, put_UserValue)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_UserValue )( 
            IHitTarget * This,
            /* [in] */ VARIANT *newVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, get_Image)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Image )( 
            IHitTarget * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, put_Image)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Image )( 
            IHitTarget * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, get_X)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_X )( 
            IHitTarget * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, put_X)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_X )( 
            IHitTarget * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, get_Y)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Y )( 
            IHitTarget * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, put_Y)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Y )( 
            IHitTarget * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, get_Z)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Z )( 
            IHitTarget * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, put_Z)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Z )( 
            IHitTarget * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, get_ScaleX)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ScaleX )( 
            IHitTarget * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, put_ScaleX)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ScaleX )( 
            IHitTarget * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, get_ScaleY)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ScaleY )( 
            IHitTarget * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, put_ScaleY)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ScaleY )( 
            IHitTarget * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, get_ScaleZ)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ScaleZ )( 
            IHitTarget * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, put_ScaleZ)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ScaleZ )( 
            IHitTarget * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, get_Orientation)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Orientation )( 
            IHitTarget * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, put_Orientation)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Orientation )( 
            IHitTarget * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, get_Name)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IHitTarget * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, put_Name)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IHitTarget * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, get_Elasticity)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Elasticity )( 
            IHitTarget * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, put_Elasticity)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Elasticity )( 
            IHitTarget * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, get_ElasticityFalloff)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ElasticityFalloff )( 
            IHitTarget * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, put_ElasticityFalloff)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ElasticityFalloff )( 
            IHitTarget * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, get_Collidable)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Collidable )( 
            IHitTarget * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, put_Collidable)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Collidable )( 
            IHitTarget * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, get_HasHitEvent)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HasHitEvent )( 
            IHitTarget * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, put_HasHitEvent)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_HasHitEvent )( 
            IHitTarget * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, get_Threshold)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Threshold )( 
            IHitTarget * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, put_Threshold)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Threshold )( 
            IHitTarget * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, get_Friction)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Friction )( 
            IHitTarget * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, put_Friction)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Friction )( 
            IHitTarget * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, get_Scatter)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Scatter )( 
            IHitTarget * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, put_Scatter)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Scatter )( 
            IHitTarget * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, get_DisableLighting)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_DisableLighting )( 
            IHitTarget * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, put_DisableLighting)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_DisableLighting )( 
            IHitTarget * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, get_BlendDisableLighting)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BlendDisableLighting )( 
            IHitTarget * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, put_BlendDisableLighting)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_BlendDisableLighting )( 
            IHitTarget * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, get_BlendDisableLightingFromBelow)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BlendDisableLightingFromBelow )( 
            IHitTarget * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, put_BlendDisableLightingFromBelow)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_BlendDisableLightingFromBelow )( 
            IHitTarget * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, get_ReflectionEnabled)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ReflectionEnabled )( 
            IHitTarget * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, put_ReflectionEnabled)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ReflectionEnabled )( 
            IHitTarget * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, get_DepthBias)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_DepthBias )( 
            IHitTarget * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, put_DepthBias)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_DepthBias )( 
            IHitTarget * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, get_DropSpeed)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_DropSpeed )( 
            IHitTarget * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, put_DropSpeed)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_DropSpeed )( 
            IHitTarget * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, get_IsDropped)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_IsDropped )( 
            IHitTarget * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, put_IsDropped)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_IsDropped )( 
            IHitTarget * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, get_DrawStyle)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_DrawStyle )( 
            IHitTarget * This,
            /* [retval][out] */ TargetType *pVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, put_DrawStyle)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_DrawStyle )( 
            IHitTarget * This,
            /* [in] */ TargetType newVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, get_LegacyMode)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_LegacyMode )( 
            IHitTarget * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, put_LegacyMode)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_LegacyMode )( 
            IHitTarget * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, get_RaiseDelay)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RaiseDelay )( 
            IHitTarget * This,
            /* [retval][out] */ long *pVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, put_RaiseDelay)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_RaiseDelay )( 
            IHitTarget * This,
            /* [in] */ long newVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, get_PhysicsMaterial)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_PhysicsMaterial )( 
            IHitTarget * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, put_PhysicsMaterial)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_PhysicsMaterial )( 
            IHitTarget * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, get_OverwritePhysics)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_OverwritePhysics )( 
            IHitTarget * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, put_OverwritePhysics)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_OverwritePhysics )( 
            IHitTarget * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IHitTarget, get_HitThreshold)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HitThreshold )( 
            IHitTarget * This,
            /* [retval][out] */ float *pVal);
        
        END_INTERFACE
    } IHitTargetVtbl;

    interface IHitTarget
    {
        CONST_VTBL struct IHitTargetVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IHitTarget_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IHitTarget_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IHitTarget_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IHitTarget_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IHitTarget_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IHitTarget_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IHitTarget_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IHitTarget_get_TimerEnabled(This,pVal)	\
    ( (This)->lpVtbl -> get_TimerEnabled(This,pVal) ) 

#define IHitTarget_put_TimerEnabled(This,newVal)	\
    ( (This)->lpVtbl -> put_TimerEnabled(This,newVal) ) 

#define IHitTarget_get_TimerInterval(This,pVal)	\
    ( (This)->lpVtbl -> get_TimerInterval(This,pVal) ) 

#define IHitTarget_put_TimerInterval(This,newVal)	\
    ( (This)->lpVtbl -> put_TimerInterval(This,newVal) ) 

#define IHitTarget_get_Visible(This,pVal)	\
    ( (This)->lpVtbl -> get_Visible(This,pVal) ) 

#define IHitTarget_put_Visible(This,newVal)	\
    ( (This)->lpVtbl -> put_Visible(This,newVal) ) 

#define IHitTarget_get_Material(This,pVal)	\
    ( (This)->lpVtbl -> get_Material(This,pVal) ) 

#define IHitTarget_put_Material(This,newVal)	\
    ( (This)->lpVtbl -> put_Material(This,newVal) ) 

#define IHitTarget_get_UserValue(This,pVal)	\
    ( (This)->lpVtbl -> get_UserValue(This,pVal) ) 

#define IHitTarget_put_UserValue(This,newVal)	\
    ( (This)->lpVtbl -> put_UserValue(This,newVal) ) 

#define IHitTarget_get_Image(This,pVal)	\
    ( (This)->lpVtbl -> get_Image(This,pVal) ) 

#define IHitTarget_put_Image(This,newVal)	\
    ( (This)->lpVtbl -> put_Image(This,newVal) ) 

#define IHitTarget_get_X(This,pVal)	\
    ( (This)->lpVtbl -> get_X(This,pVal) ) 

#define IHitTarget_put_X(This,newVal)	\
    ( (This)->lpVtbl -> put_X(This,newVal) ) 

#define IHitTarget_get_Y(This,pVal)	\
    ( (This)->lpVtbl -> get_Y(This,pVal) ) 

#define IHitTarget_put_Y(This,newVal)	\
    ( (This)->lpVtbl -> put_Y(This,newVal) ) 

#define IHitTarget_get_Z(This,pVal)	\
    ( (This)->lpVtbl -> get_Z(This,pVal) ) 

#define IHitTarget_put_Z(This,newVal)	\
    ( (This)->lpVtbl -> put_Z(This,newVal) ) 

#define IHitTarget_get_ScaleX(This,pVal)	\
    ( (This)->lpVtbl -> get_ScaleX(This,pVal) ) 

#define IHitTarget_put_ScaleX(This,newVal)	\
    ( (This)->lpVtbl -> put_ScaleX(This,newVal) ) 

#define IHitTarget_get_ScaleY(This,pVal)	\
    ( (This)->lpVtbl -> get_ScaleY(This,pVal) ) 

#define IHitTarget_put_ScaleY(This,newVal)	\
    ( (This)->lpVtbl -> put_ScaleY(This,newVal) ) 

#define IHitTarget_get_ScaleZ(This,pVal)	\
    ( (This)->lpVtbl -> get_ScaleZ(This,pVal) ) 

#define IHitTarget_put_ScaleZ(This,newVal)	\
    ( (This)->lpVtbl -> put_ScaleZ(This,newVal) ) 

#define IHitTarget_get_Orientation(This,pVal)	\
    ( (This)->lpVtbl -> get_Orientation(This,pVal) ) 

#define IHitTarget_put_Orientation(This,newVal)	\
    ( (This)->lpVtbl -> put_Orientation(This,newVal) ) 

#define IHitTarget_get_Name(This,pVal)	\
    ( (This)->lpVtbl -> get_Name(This,pVal) ) 

#define IHitTarget_put_Name(This,newVal)	\
    ( (This)->lpVtbl -> put_Name(This,newVal) ) 

#define IHitTarget_get_Elasticity(This,pVal)	\
    ( (This)->lpVtbl -> get_Elasticity(This,pVal) ) 

#define IHitTarget_put_Elasticity(This,newVal)	\
    ( (This)->lpVtbl -> put_Elasticity(This,newVal) ) 

#define IHitTarget_get_ElasticityFalloff(This,pVal)	\
    ( (This)->lpVtbl -> get_ElasticityFalloff(This,pVal) ) 

#define IHitTarget_put_ElasticityFalloff(This,newVal)	\
    ( (This)->lpVtbl -> put_ElasticityFalloff(This,newVal) ) 

#define IHitTarget_get_Collidable(This,pVal)	\
    ( (This)->lpVtbl -> get_Collidable(This,pVal) ) 

#define IHitTarget_put_Collidable(This,newVal)	\
    ( (This)->lpVtbl -> put_Collidable(This,newVal) ) 

#define IHitTarget_get_HasHitEvent(This,pVal)	\
    ( (This)->lpVtbl -> get_HasHitEvent(This,pVal) ) 

#define IHitTarget_put_HasHitEvent(This,newVal)	\
    ( (This)->lpVtbl -> put_HasHitEvent(This,newVal) ) 

#define IHitTarget_get_Threshold(This,pVal)	\
    ( (This)->lpVtbl -> get_Threshold(This,pVal) ) 

#define IHitTarget_put_Threshold(This,newVal)	\
    ( (This)->lpVtbl -> put_Threshold(This,newVal) ) 

#define IHitTarget_get_Friction(This,pVal)	\
    ( (This)->lpVtbl -> get_Friction(This,pVal) ) 

#define IHitTarget_put_Friction(This,newVal)	\
    ( (This)->lpVtbl -> put_Friction(This,newVal) ) 

#define IHitTarget_get_Scatter(This,pVal)	\
    ( (This)->lpVtbl -> get_Scatter(This,pVal) ) 

#define IHitTarget_put_Scatter(This,newVal)	\
    ( (This)->lpVtbl -> put_Scatter(This,newVal) ) 

#define IHitTarget_get_DisableLighting(This,pVal)	\
    ( (This)->lpVtbl -> get_DisableLighting(This,pVal) ) 

#define IHitTarget_put_DisableLighting(This,newVal)	\
    ( (This)->lpVtbl -> put_DisableLighting(This,newVal) ) 

#define IHitTarget_get_BlendDisableLighting(This,pVal)	\
    ( (This)->lpVtbl -> get_BlendDisableLighting(This,pVal) ) 

#define IHitTarget_put_BlendDisableLighting(This,newVal)	\
    ( (This)->lpVtbl -> put_BlendDisableLighting(This,newVal) ) 

#define IHitTarget_get_BlendDisableLightingFromBelow(This,pVal)	\
    ( (This)->lpVtbl -> get_BlendDisableLightingFromBelow(This,pVal) ) 

#define IHitTarget_put_BlendDisableLightingFromBelow(This,newVal)	\
    ( (This)->lpVtbl -> put_BlendDisableLightingFromBelow(This,newVal) ) 

#define IHitTarget_get_ReflectionEnabled(This,pVal)	\
    ( (This)->lpVtbl -> get_ReflectionEnabled(This,pVal) ) 

#define IHitTarget_put_ReflectionEnabled(This,newVal)	\
    ( (This)->lpVtbl -> put_ReflectionEnabled(This,newVal) ) 

#define IHitTarget_get_DepthBias(This,pVal)	\
    ( (This)->lpVtbl -> get_DepthBias(This,pVal) ) 

#define IHitTarget_put_DepthBias(This,newVal)	\
    ( (This)->lpVtbl -> put_DepthBias(This,newVal) ) 

#define IHitTarget_get_DropSpeed(This,pVal)	\
    ( (This)->lpVtbl -> get_DropSpeed(This,pVal) ) 

#define IHitTarget_put_DropSpeed(This,newVal)	\
    ( (This)->lpVtbl -> put_DropSpeed(This,newVal) ) 

#define IHitTarget_get_IsDropped(This,pVal)	\
    ( (This)->lpVtbl -> get_IsDropped(This,pVal) ) 

#define IHitTarget_put_IsDropped(This,newVal)	\
    ( (This)->lpVtbl -> put_IsDropped(This,newVal) ) 

#define IHitTarget_get_DrawStyle(This,pVal)	\
    ( (This)->lpVtbl -> get_DrawStyle(This,pVal) ) 

#define IHitTarget_put_DrawStyle(This,newVal)	\
    ( (This)->lpVtbl -> put_DrawStyle(This,newVal) ) 

#define IHitTarget_get_LegacyMode(This,pVal)	\
    ( (This)->lpVtbl -> get_LegacyMode(This,pVal) ) 

#define IHitTarget_put_LegacyMode(This,newVal)	\
    ( (This)->lpVtbl -> put_LegacyMode(This,newVal) ) 

#define IHitTarget_get_RaiseDelay(This,pVal)	\
    ( (This)->lpVtbl -> get_RaiseDelay(This,pVal) ) 

#define IHitTarget_put_RaiseDelay(This,newVal)	\
    ( (This)->lpVtbl -> put_RaiseDelay(This,newVal) ) 

#define IHitTarget_get_PhysicsMaterial(This,pVal)	\
    ( (This)->lpVtbl -> get_PhysicsMaterial(This,pVal) ) 

#define IHitTarget_put_PhysicsMaterial(This,newVal)	\
    ( (This)->lpVtbl -> put_PhysicsMaterial(This,newVal) ) 

#define IHitTarget_get_OverwritePhysics(This,pVal)	\
    ( (This)->lpVtbl -> get_OverwritePhysics(This,pVal) ) 

#define IHitTarget_put_OverwritePhysics(This,newVal)	\
    ( (This)->lpVtbl -> put_OverwritePhysics(This,newVal) ) 

#define IHitTarget_get_HitThreshold(This,pVal)	\
    ( (This)->lpVtbl -> get_HitThreshold(This,pVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IHitTarget_INTERFACE_DEFINED__ */


#ifndef __IHitTargetEvents_DISPINTERFACE_DEFINED__
#define __IHitTargetEvents_DISPINTERFACE_DEFINED__

/* dispinterface IHitTargetEvents */
/* [uuid] */ 


EXTERN_C const IID DIID_IHitTargetEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("4BC3733D-7EA2-45F5-86B3-D863547C8A44")
    IHitTargetEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct IHitTargetEventsVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IHitTargetEvents * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IHitTargetEvents * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IHitTargetEvents * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IHitTargetEvents * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IHitTargetEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IHitTargetEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IHitTargetEvents * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        END_INTERFACE
    } IHitTargetEventsVtbl;

    interface IHitTargetEvents
    {
        CONST_VTBL struct IHitTargetEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IHitTargetEvents_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IHitTargetEvents_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IHitTargetEvents_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IHitTargetEvents_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IHitTargetEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IHitTargetEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IHitTargetEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* __IHitTargetEvents_DISPINTERFACE_DEFINED__ */


#ifndef __IGate_INTERFACE_DEFINED__
#define __IGate_INTERFACE_DEFINED__

/* interface IGate */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IGate;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9459EA61-42FE-4315-9289-A2CB298324CC")
    IGate : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Name( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TimerEnabled( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TimerEnabled( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TimerInterval( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TimerInterval( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Length( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Length( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Height( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Height( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Rotation( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Rotation( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_X( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_X( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Y( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Y( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Open( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Open( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Damping( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Damping( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_GravityFactor( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_GravityFactor( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Move( 
            int dir,
            /* [defaultvalue] */ float speed = 0,
            /* [defaultvalue] */ float angle = 0) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Material( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Material( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Elasticity( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Elasticity( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Surface( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Surface( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_UserValue( 
            /* [retval][out] */ VARIANT *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_UserValue( 
            /* [in] */ VARIANT *newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CloseAngle( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_CloseAngle( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_OpenAngle( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_OpenAngle( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Collidable( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Collidable( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Friction( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Friction( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Visible( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Visible( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TwoWay( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TwoWay( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ShowBracket( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ShowBracket( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][nonbrowsable][id][propget] */ HRESULT STDMETHODCALLTYPE get_CurrentAngle( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ReflectionEnabled( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ReflectionEnabled( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DrawStyle( 
            /* [retval][out] */ GateType *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_DrawStyle( 
            /* [in] */ GateType newVal) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IGateVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGate * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGate * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGate * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IGate * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IGate * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IGate * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IGate * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        DECLSPEC_XFGVIRT(IGate, get_Name)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IGate * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IGate, put_Name)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IGate * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IGate, get_TimerEnabled)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TimerEnabled )( 
            IGate * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IGate, put_TimerEnabled)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TimerEnabled )( 
            IGate * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IGate, get_TimerInterval)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TimerInterval )( 
            IGate * This,
            /* [retval][out] */ long *pVal);
        
        DECLSPEC_XFGVIRT(IGate, put_TimerInterval)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TimerInterval )( 
            IGate * This,
            /* [in] */ long newVal);
        
        DECLSPEC_XFGVIRT(IGate, get_Length)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Length )( 
            IGate * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IGate, put_Length)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Length )( 
            IGate * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IGate, get_Height)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Height )( 
            IGate * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IGate, put_Height)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Height )( 
            IGate * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IGate, get_Rotation)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Rotation )( 
            IGate * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IGate, put_Rotation)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Rotation )( 
            IGate * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IGate, get_X)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_X )( 
            IGate * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IGate, put_X)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_X )( 
            IGate * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IGate, get_Y)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Y )( 
            IGate * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IGate, put_Y)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Y )( 
            IGate * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IGate, get_Open)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Open )( 
            IGate * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IGate, put_Open)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Open )( 
            IGate * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IGate, get_Damping)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Damping )( 
            IGate * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IGate, put_Damping)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Damping )( 
            IGate * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IGate, get_GravityFactor)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_GravityFactor )( 
            IGate * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IGate, put_GravityFactor)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_GravityFactor )( 
            IGate * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IGate, Move)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Move )( 
            IGate * This,
            int dir,
            /* [defaultvalue] */ float speed,
            /* [defaultvalue] */ float angle);
        
        DECLSPEC_XFGVIRT(IGate, get_Material)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Material )( 
            IGate * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IGate, put_Material)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Material )( 
            IGate * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IGate, get_Elasticity)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Elasticity )( 
            IGate * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IGate, put_Elasticity)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Elasticity )( 
            IGate * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IGate, get_Surface)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Surface )( 
            IGate * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IGate, put_Surface)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Surface )( 
            IGate * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IGate, get_UserValue)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_UserValue )( 
            IGate * This,
            /* [retval][out] */ VARIANT *pVal);
        
        DECLSPEC_XFGVIRT(IGate, put_UserValue)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_UserValue )( 
            IGate * This,
            /* [in] */ VARIANT *newVal);
        
        DECLSPEC_XFGVIRT(IGate, get_CloseAngle)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CloseAngle )( 
            IGate * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IGate, put_CloseAngle)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_CloseAngle )( 
            IGate * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IGate, get_OpenAngle)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_OpenAngle )( 
            IGate * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IGate, put_OpenAngle)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_OpenAngle )( 
            IGate * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IGate, get_Collidable)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Collidable )( 
            IGate * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IGate, put_Collidable)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Collidable )( 
            IGate * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IGate, get_Friction)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Friction )( 
            IGate * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IGate, put_Friction)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Friction )( 
            IGate * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IGate, get_Visible)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Visible )( 
            IGate * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IGate, put_Visible)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Visible )( 
            IGate * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IGate, get_TwoWay)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TwoWay )( 
            IGate * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IGate, put_TwoWay)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TwoWay )( 
            IGate * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IGate, get_ShowBracket)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ShowBracket )( 
            IGate * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IGate, put_ShowBracket)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ShowBracket )( 
            IGate * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IGate, get_CurrentAngle)
        /* [helpstring][nonbrowsable][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CurrentAngle )( 
            IGate * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IGate, get_ReflectionEnabled)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ReflectionEnabled )( 
            IGate * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IGate, put_ReflectionEnabled)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ReflectionEnabled )( 
            IGate * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IGate, get_DrawStyle)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_DrawStyle )( 
            IGate * This,
            /* [retval][out] */ GateType *pVal);
        
        DECLSPEC_XFGVIRT(IGate, put_DrawStyle)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_DrawStyle )( 
            IGate * This,
            /* [in] */ GateType newVal);
        
        END_INTERFACE
    } IGateVtbl;

    interface IGate
    {
        CONST_VTBL struct IGateVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGate_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IGate_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IGate_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IGate_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IGate_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IGate_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IGate_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IGate_get_Name(This,pVal)	\
    ( (This)->lpVtbl -> get_Name(This,pVal) ) 

#define IGate_put_Name(This,newVal)	\
    ( (This)->lpVtbl -> put_Name(This,newVal) ) 

#define IGate_get_TimerEnabled(This,pVal)	\
    ( (This)->lpVtbl -> get_TimerEnabled(This,pVal) ) 

#define IGate_put_TimerEnabled(This,newVal)	\
    ( (This)->lpVtbl -> put_TimerEnabled(This,newVal) ) 

#define IGate_get_TimerInterval(This,pVal)	\
    ( (This)->lpVtbl -> get_TimerInterval(This,pVal) ) 

#define IGate_put_TimerInterval(This,newVal)	\
    ( (This)->lpVtbl -> put_TimerInterval(This,newVal) ) 

#define IGate_get_Length(This,pVal)	\
    ( (This)->lpVtbl -> get_Length(This,pVal) ) 

#define IGate_put_Length(This,newVal)	\
    ( (This)->lpVtbl -> put_Length(This,newVal) ) 

#define IGate_get_Height(This,pVal)	\
    ( (This)->lpVtbl -> get_Height(This,pVal) ) 

#define IGate_put_Height(This,newVal)	\
    ( (This)->lpVtbl -> put_Height(This,newVal) ) 

#define IGate_get_Rotation(This,pVal)	\
    ( (This)->lpVtbl -> get_Rotation(This,pVal) ) 

#define IGate_put_Rotation(This,newVal)	\
    ( (This)->lpVtbl -> put_Rotation(This,newVal) ) 

#define IGate_get_X(This,pVal)	\
    ( (This)->lpVtbl -> get_X(This,pVal) ) 

#define IGate_put_X(This,newVal)	\
    ( (This)->lpVtbl -> put_X(This,newVal) ) 

#define IGate_get_Y(This,pVal)	\
    ( (This)->lpVtbl -> get_Y(This,pVal) ) 

#define IGate_put_Y(This,newVal)	\
    ( (This)->lpVtbl -> put_Y(This,newVal) ) 

#define IGate_get_Open(This,pVal)	\
    ( (This)->lpVtbl -> get_Open(This,pVal) ) 

#define IGate_put_Open(This,newVal)	\
    ( (This)->lpVtbl -> put_Open(This,newVal) ) 

#define IGate_get_Damping(This,pVal)	\
    ( (This)->lpVtbl -> get_Damping(This,pVal) ) 

#define IGate_put_Damping(This,newVal)	\
    ( (This)->lpVtbl -> put_Damping(This,newVal) ) 

#define IGate_get_GravityFactor(This,pVal)	\
    ( (This)->lpVtbl -> get_GravityFactor(This,pVal) ) 

#define IGate_put_GravityFactor(This,newVal)	\
    ( (This)->lpVtbl -> put_GravityFactor(This,newVal) ) 

#define IGate_Move(This,dir,speed,angle)	\
    ( (This)->lpVtbl -> Move(This,dir,speed,angle) ) 

#define IGate_get_Material(This,pVal)	\
    ( (This)->lpVtbl -> get_Material(This,pVal) ) 

#define IGate_put_Material(This,newVal)	\
    ( (This)->lpVtbl -> put_Material(This,newVal) ) 

#define IGate_get_Elasticity(This,pVal)	\
    ( (This)->lpVtbl -> get_Elasticity(This,pVal) ) 

#define IGate_put_Elasticity(This,newVal)	\
    ( (This)->lpVtbl -> put_Elasticity(This,newVal) ) 

#define IGate_get_Surface(This,pVal)	\
    ( (This)->lpVtbl -> get_Surface(This,pVal) ) 

#define IGate_put_Surface(This,newVal)	\
    ( (This)->lpVtbl -> put_Surface(This,newVal) ) 

#define IGate_get_UserValue(This,pVal)	\
    ( (This)->lpVtbl -> get_UserValue(This,pVal) ) 

#define IGate_put_UserValue(This,newVal)	\
    ( (This)->lpVtbl -> put_UserValue(This,newVal) ) 

#define IGate_get_CloseAngle(This,pVal)	\
    ( (This)->lpVtbl -> get_CloseAngle(This,pVal) ) 

#define IGate_put_CloseAngle(This,newVal)	\
    ( (This)->lpVtbl -> put_CloseAngle(This,newVal) ) 

#define IGate_get_OpenAngle(This,pVal)	\
    ( (This)->lpVtbl -> get_OpenAngle(This,pVal) ) 

#define IGate_put_OpenAngle(This,newVal)	\
    ( (This)->lpVtbl -> put_OpenAngle(This,newVal) ) 

#define IGate_get_Collidable(This,pVal)	\
    ( (This)->lpVtbl -> get_Collidable(This,pVal) ) 

#define IGate_put_Collidable(This,newVal)	\
    ( (This)->lpVtbl -> put_Collidable(This,newVal) ) 

#define IGate_get_Friction(This,pVal)	\
    ( (This)->lpVtbl -> get_Friction(This,pVal) ) 

#define IGate_put_Friction(This,newVal)	\
    ( (This)->lpVtbl -> put_Friction(This,newVal) ) 

#define IGate_get_Visible(This,pVal)	\
    ( (This)->lpVtbl -> get_Visible(This,pVal) ) 

#define IGate_put_Visible(This,newVal)	\
    ( (This)->lpVtbl -> put_Visible(This,newVal) ) 

#define IGate_get_TwoWay(This,pVal)	\
    ( (This)->lpVtbl -> get_TwoWay(This,pVal) ) 

#define IGate_put_TwoWay(This,newVal)	\
    ( (This)->lpVtbl -> put_TwoWay(This,newVal) ) 

#define IGate_get_ShowBracket(This,pVal)	\
    ( (This)->lpVtbl -> get_ShowBracket(This,pVal) ) 

#define IGate_put_ShowBracket(This,newVal)	\
    ( (This)->lpVtbl -> put_ShowBracket(This,newVal) ) 

#define IGate_get_CurrentAngle(This,pVal)	\
    ( (This)->lpVtbl -> get_CurrentAngle(This,pVal) ) 

#define IGate_get_ReflectionEnabled(This,pVal)	\
    ( (This)->lpVtbl -> get_ReflectionEnabled(This,pVal) ) 

#define IGate_put_ReflectionEnabled(This,newVal)	\
    ( (This)->lpVtbl -> put_ReflectionEnabled(This,newVal) ) 

#define IGate_get_DrawStyle(This,pVal)	\
    ( (This)->lpVtbl -> get_DrawStyle(This,pVal) ) 

#define IGate_put_DrawStyle(This,newVal)	\
    ( (This)->lpVtbl -> put_DrawStyle(This,newVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IGate_INTERFACE_DEFINED__ */


#ifndef __IGateEvents_DISPINTERFACE_DEFINED__
#define __IGateEvents_DISPINTERFACE_DEFINED__

/* dispinterface IGateEvents */
/* [uuid] */ 


EXTERN_C const IID DIID_IGateEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("D7753568-BFD4-4843-B2B1-766CCD9A20BA")
    IGateEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct IGateEventsVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGateEvents * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGateEvents * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGateEvents * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IGateEvents * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IGateEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IGateEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IGateEvents * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        END_INTERFACE
    } IGateEventsVtbl;

    interface IGateEvents
    {
        CONST_VTBL struct IGateEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGateEvents_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IGateEvents_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IGateEvents_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IGateEvents_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IGateEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IGateEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IGateEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* __IGateEvents_DISPINTERFACE_DEFINED__ */


#ifndef __ISpinner_INTERFACE_DEFINED__
#define __ISpinner_INTERFACE_DEFINED__

/* interface ISpinner */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_ISpinner;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("ED5CE186-E654-4AEB-9959-E4DC93128958")
    ISpinner : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Name( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TimerEnabled( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TimerEnabled( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TimerInterval( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TimerInterval( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Length( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Length( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Rotation( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Rotation( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Height( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Height( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Damping( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Damping( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Image( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Image( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Material( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Material( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_X( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_X( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Y( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Y( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Surface( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Surface( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_UserValue( 
            /* [retval][out] */ VARIANT *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_UserValue( 
            /* [in] */ VARIANT *newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ShowBracket( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ShowBracket( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AngleMax( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_AngleMax( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AngleMin( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_AngleMin( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Elasticity( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Elasticity( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Visible( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Visible( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ReflectionEnabled( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ReflectionEnabled( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][nonbrowsable][id][propget] */ HRESULT STDMETHODCALLTYPE get_CurrentAngle( 
            /* [retval][out] */ float *pVal) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct ISpinnerVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpinner * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpinner * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpinner * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpinner * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpinner * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpinner * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpinner * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        DECLSPEC_XFGVIRT(ISpinner, get_Name)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ISpinner * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(ISpinner, put_Name)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            ISpinner * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(ISpinner, get_TimerEnabled)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TimerEnabled )( 
            ISpinner * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(ISpinner, put_TimerEnabled)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TimerEnabled )( 
            ISpinner * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(ISpinner, get_TimerInterval)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TimerInterval )( 
            ISpinner * This,
            /* [retval][out] */ long *pVal);
        
        DECLSPEC_XFGVIRT(ISpinner, put_TimerInterval)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TimerInterval )( 
            ISpinner * This,
            /* [in] */ long newVal);
        
        DECLSPEC_XFGVIRT(ISpinner, get_Length)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Length )( 
            ISpinner * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ISpinner, put_Length)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Length )( 
            ISpinner * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ISpinner, get_Rotation)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Rotation )( 
            ISpinner * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ISpinner, put_Rotation)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Rotation )( 
            ISpinner * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ISpinner, get_Height)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Height )( 
            ISpinner * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ISpinner, put_Height)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Height )( 
            ISpinner * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ISpinner, get_Damping)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Damping )( 
            ISpinner * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ISpinner, put_Damping)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Damping )( 
            ISpinner * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ISpinner, get_Image)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Image )( 
            ISpinner * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(ISpinner, put_Image)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Image )( 
            ISpinner * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(ISpinner, get_Material)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Material )( 
            ISpinner * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(ISpinner, put_Material)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Material )( 
            ISpinner * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(ISpinner, get_X)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_X )( 
            ISpinner * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ISpinner, put_X)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_X )( 
            ISpinner * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ISpinner, get_Y)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Y )( 
            ISpinner * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ISpinner, put_Y)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Y )( 
            ISpinner * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ISpinner, get_Surface)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Surface )( 
            ISpinner * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(ISpinner, put_Surface)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Surface )( 
            ISpinner * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(ISpinner, get_UserValue)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_UserValue )( 
            ISpinner * This,
            /* [retval][out] */ VARIANT *pVal);
        
        DECLSPEC_XFGVIRT(ISpinner, put_UserValue)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_UserValue )( 
            ISpinner * This,
            /* [in] */ VARIANT *newVal);
        
        DECLSPEC_XFGVIRT(ISpinner, get_ShowBracket)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ShowBracket )( 
            ISpinner * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(ISpinner, put_ShowBracket)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ShowBracket )( 
            ISpinner * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(ISpinner, get_AngleMax)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AngleMax )( 
            ISpinner * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ISpinner, put_AngleMax)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_AngleMax )( 
            ISpinner * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ISpinner, get_AngleMin)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AngleMin )( 
            ISpinner * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ISpinner, put_AngleMin)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_AngleMin )( 
            ISpinner * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ISpinner, get_Elasticity)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Elasticity )( 
            ISpinner * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ISpinner, put_Elasticity)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Elasticity )( 
            ISpinner * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ISpinner, get_Visible)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Visible )( 
            ISpinner * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(ISpinner, put_Visible)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Visible )( 
            ISpinner * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(ISpinner, get_ReflectionEnabled)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ReflectionEnabled )( 
            ISpinner * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(ISpinner, put_ReflectionEnabled)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ReflectionEnabled )( 
            ISpinner * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(ISpinner, get_CurrentAngle)
        /* [helpstring][nonbrowsable][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CurrentAngle )( 
            ISpinner * This,
            /* [retval][out] */ float *pVal);
        
        END_INTERFACE
    } ISpinnerVtbl;

    interface ISpinner
    {
        CONST_VTBL struct ISpinnerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpinner_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ISpinner_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ISpinner_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ISpinner_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define ISpinner_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define ISpinner_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define ISpinner_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define ISpinner_get_Name(This,pVal)	\
    ( (This)->lpVtbl -> get_Name(This,pVal) ) 

#define ISpinner_put_Name(This,newVal)	\
    ( (This)->lpVtbl -> put_Name(This,newVal) ) 

#define ISpinner_get_TimerEnabled(This,pVal)	\
    ( (This)->lpVtbl -> get_TimerEnabled(This,pVal) ) 

#define ISpinner_put_TimerEnabled(This,newVal)	\
    ( (This)->lpVtbl -> put_TimerEnabled(This,newVal) ) 

#define ISpinner_get_TimerInterval(This,pVal)	\
    ( (This)->lpVtbl -> get_TimerInterval(This,pVal) ) 

#define ISpinner_put_TimerInterval(This,newVal)	\
    ( (This)->lpVtbl -> put_TimerInterval(This,newVal) ) 

#define ISpinner_get_Length(This,pVal)	\
    ( (This)->lpVtbl -> get_Length(This,pVal) ) 

#define ISpinner_put_Length(This,newVal)	\
    ( (This)->lpVtbl -> put_Length(This,newVal) ) 

#define ISpinner_get_Rotation(This,pVal)	\
    ( (This)->lpVtbl -> get_Rotation(This,pVal) ) 

#define ISpinner_put_Rotation(This,newVal)	\
    ( (This)->lpVtbl -> put_Rotation(This,newVal) ) 

#define ISpinner_get_Height(This,pVal)	\
    ( (This)->lpVtbl -> get_Height(This,pVal) ) 

#define ISpinner_put_Height(This,newVal)	\
    ( (This)->lpVtbl -> put_Height(This,newVal) ) 

#define ISpinner_get_Damping(This,pVal)	\
    ( (This)->lpVtbl -> get_Damping(This,pVal) ) 

#define ISpinner_put_Damping(This,newVal)	\
    ( (This)->lpVtbl -> put_Damping(This,newVal) ) 

#define ISpinner_get_Image(This,pVal)	\
    ( (This)->lpVtbl -> get_Image(This,pVal) ) 

#define ISpinner_put_Image(This,newVal)	\
    ( (This)->lpVtbl -> put_Image(This,newVal) ) 

#define ISpinner_get_Material(This,pVal)	\
    ( (This)->lpVtbl -> get_Material(This,pVal) ) 

#define ISpinner_put_Material(This,newVal)	\
    ( (This)->lpVtbl -> put_Material(This,newVal) ) 

#define ISpinner_get_X(This,pVal)	\
    ( (This)->lpVtbl -> get_X(This,pVal) ) 

#define ISpinner_put_X(This,newVal)	\
    ( (This)->lpVtbl -> put_X(This,newVal) ) 

#define ISpinner_get_Y(This,pVal)	\
    ( (This)->lpVtbl -> get_Y(This,pVal) ) 

#define ISpinner_put_Y(This,newVal)	\
    ( (This)->lpVtbl -> put_Y(This,newVal) ) 

#define ISpinner_get_Surface(This,pVal)	\
    ( (This)->lpVtbl -> get_Surface(This,pVal) ) 

#define ISpinner_put_Surface(This,newVal)	\
    ( (This)->lpVtbl -> put_Surface(This,newVal) ) 

#define ISpinner_get_UserValue(This,pVal)	\
    ( (This)->lpVtbl -> get_UserValue(This,pVal) ) 

#define ISpinner_put_UserValue(This,newVal)	\
    ( (This)->lpVtbl -> put_UserValue(This,newVal) ) 

#define ISpinner_get_ShowBracket(This,pVal)	\
    ( (This)->lpVtbl -> get_ShowBracket(This,pVal) ) 

#define ISpinner_put_ShowBracket(This,newVal)	\
    ( (This)->lpVtbl -> put_ShowBracket(This,newVal) ) 

#define ISpinner_get_AngleMax(This,pVal)	\
    ( (This)->lpVtbl -> get_AngleMax(This,pVal) ) 

#define ISpinner_put_AngleMax(This,newVal)	\
    ( (This)->lpVtbl -> put_AngleMax(This,newVal) ) 

#define ISpinner_get_AngleMin(This,pVal)	\
    ( (This)->lpVtbl -> get_AngleMin(This,pVal) ) 

#define ISpinner_put_AngleMin(This,newVal)	\
    ( (This)->lpVtbl -> put_AngleMin(This,newVal) ) 

#define ISpinner_get_Elasticity(This,pVal)	\
    ( (This)->lpVtbl -> get_Elasticity(This,pVal) ) 

#define ISpinner_put_Elasticity(This,newVal)	\
    ( (This)->lpVtbl -> put_Elasticity(This,newVal) ) 

#define ISpinner_get_Visible(This,pVal)	\
    ( (This)->lpVtbl -> get_Visible(This,pVal) ) 

#define ISpinner_put_Visible(This,newVal)	\
    ( (This)->lpVtbl -> put_Visible(This,newVal) ) 

#define ISpinner_get_ReflectionEnabled(This,pVal)	\
    ( (This)->lpVtbl -> get_ReflectionEnabled(This,pVal) ) 

#define ISpinner_put_ReflectionEnabled(This,newVal)	\
    ( (This)->lpVtbl -> put_ReflectionEnabled(This,newVal) ) 

#define ISpinner_get_CurrentAngle(This,pVal)	\
    ( (This)->lpVtbl -> get_CurrentAngle(This,pVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ISpinner_INTERFACE_DEFINED__ */


#ifndef __ISpinnerEvents_DISPINTERFACE_DEFINED__
#define __ISpinnerEvents_DISPINTERFACE_DEFINED__

/* dispinterface ISpinnerEvents */
/* [uuid] */ 


EXTERN_C const IID DIID_ISpinnerEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("1B57054E-DB6D-4a12-AD63-BDAB51148562")
    ISpinnerEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct ISpinnerEventsVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpinnerEvents * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpinnerEvents * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpinnerEvents * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpinnerEvents * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpinnerEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpinnerEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpinnerEvents * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        END_INTERFACE
    } ISpinnerEventsVtbl;

    interface ISpinnerEvents
    {
        CONST_VTBL struct ISpinnerEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpinnerEvents_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ISpinnerEvents_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ISpinnerEvents_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ISpinnerEvents_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define ISpinnerEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define ISpinnerEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define ISpinnerEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* __ISpinnerEvents_DISPINTERFACE_DEFINED__ */


#ifndef __IRamp_INTERFACE_DEFINED__
#define __IRamp_INTERFACE_DEFINED__

/* interface IRamp */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IRamp;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A90308CF-09D8-4c38-99B6-9E371EA41B97")
    IRamp : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Name( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TimerEnabled( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TimerEnabled( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TimerInterval( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TimerInterval( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_HeightBottom( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_HeightBottom( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_HeightTop( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_HeightTop( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_WidthBottom( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_WidthBottom( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_WidthTop( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_WidthTop( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Material( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Material( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Type( 
            /* [retval][out] */ RampType *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Type( 
            /* [in] */ RampType newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Image( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Image( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ImageAlignment( 
            /* [retval][out] */ RampImageAlignment *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ImageAlignment( 
            /* [in] */ RampImageAlignment newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_HasWallImage( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_HasWallImage( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_LeftWallHeight( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_LeftWallHeight( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RightWallHeight( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_RightWallHeight( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_UserValue( 
            /* [retval][out] */ VARIANT *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_UserValue( 
            /* [in] */ VARIANT *newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VisibleLeftWallHeight( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_VisibleLeftWallHeight( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VisibleRightWallHeight( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_VisibleRightWallHeight( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Elasticity( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Elasticity( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Collidable( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Collidable( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_HasHitEvent( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_HasHitEvent( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Threshold( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Threshold( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Visible( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Visible( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Friction( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Friction( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Scatter( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Scatter( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DepthBias( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_DepthBias( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_WireDiameter( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_WireDiameter( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_WireDistanceX( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_WireDistanceX( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_WireDistanceY( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_WireDistanceY( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ReflectionEnabled( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ReflectionEnabled( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_PhysicsMaterial( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_PhysicsMaterial( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_OverwritePhysics( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_OverwritePhysics( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IRampVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRamp * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRamp * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRamp * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IRamp * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IRamp * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IRamp * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IRamp * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        DECLSPEC_XFGVIRT(IRamp, get_Name)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IRamp * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IRamp, put_Name)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IRamp * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IRamp, get_TimerEnabled)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TimerEnabled )( 
            IRamp * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IRamp, put_TimerEnabled)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TimerEnabled )( 
            IRamp * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IRamp, get_TimerInterval)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TimerInterval )( 
            IRamp * This,
            /* [retval][out] */ long *pVal);
        
        DECLSPEC_XFGVIRT(IRamp, put_TimerInterval)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TimerInterval )( 
            IRamp * This,
            /* [in] */ long newVal);
        
        DECLSPEC_XFGVIRT(IRamp, get_HeightBottom)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HeightBottom )( 
            IRamp * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IRamp, put_HeightBottom)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_HeightBottom )( 
            IRamp * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IRamp, get_HeightTop)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HeightTop )( 
            IRamp * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IRamp, put_HeightTop)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_HeightTop )( 
            IRamp * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IRamp, get_WidthBottom)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_WidthBottom )( 
            IRamp * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IRamp, put_WidthBottom)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_WidthBottom )( 
            IRamp * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IRamp, get_WidthTop)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_WidthTop )( 
            IRamp * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IRamp, put_WidthTop)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_WidthTop )( 
            IRamp * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IRamp, get_Material)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Material )( 
            IRamp * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IRamp, put_Material)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Material )( 
            IRamp * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IRamp, get_Type)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            IRamp * This,
            /* [retval][out] */ RampType *pVal);
        
        DECLSPEC_XFGVIRT(IRamp, put_Type)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Type )( 
            IRamp * This,
            /* [in] */ RampType newVal);
        
        DECLSPEC_XFGVIRT(IRamp, get_Image)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Image )( 
            IRamp * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IRamp, put_Image)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Image )( 
            IRamp * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IRamp, get_ImageAlignment)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ImageAlignment )( 
            IRamp * This,
            /* [retval][out] */ RampImageAlignment *pVal);
        
        DECLSPEC_XFGVIRT(IRamp, put_ImageAlignment)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ImageAlignment )( 
            IRamp * This,
            /* [in] */ RampImageAlignment newVal);
        
        DECLSPEC_XFGVIRT(IRamp, get_HasWallImage)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HasWallImage )( 
            IRamp * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IRamp, put_HasWallImage)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_HasWallImage )( 
            IRamp * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IRamp, get_LeftWallHeight)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_LeftWallHeight )( 
            IRamp * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IRamp, put_LeftWallHeight)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_LeftWallHeight )( 
            IRamp * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IRamp, get_RightWallHeight)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RightWallHeight )( 
            IRamp * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IRamp, put_RightWallHeight)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_RightWallHeight )( 
            IRamp * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IRamp, get_UserValue)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_UserValue )( 
            IRamp * This,
            /* [retval][out] */ VARIANT *pVal);
        
        DECLSPEC_XFGVIRT(IRamp, put_UserValue)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_UserValue )( 
            IRamp * This,
            /* [in] */ VARIANT *newVal);
        
        DECLSPEC_XFGVIRT(IRamp, get_VisibleLeftWallHeight)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VisibleLeftWallHeight )( 
            IRamp * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IRamp, put_VisibleLeftWallHeight)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_VisibleLeftWallHeight )( 
            IRamp * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IRamp, get_VisibleRightWallHeight)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VisibleRightWallHeight )( 
            IRamp * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IRamp, put_VisibleRightWallHeight)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_VisibleRightWallHeight )( 
            IRamp * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IRamp, get_Elasticity)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Elasticity )( 
            IRamp * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IRamp, put_Elasticity)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Elasticity )( 
            IRamp * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IRamp, get_Collidable)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Collidable )( 
            IRamp * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IRamp, put_Collidable)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Collidable )( 
            IRamp * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IRamp, get_HasHitEvent)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HasHitEvent )( 
            IRamp * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IRamp, put_HasHitEvent)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_HasHitEvent )( 
            IRamp * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IRamp, get_Threshold)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Threshold )( 
            IRamp * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IRamp, put_Threshold)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Threshold )( 
            IRamp * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IRamp, get_Visible)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Visible )( 
            IRamp * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IRamp, put_Visible)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Visible )( 
            IRamp * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IRamp, get_Friction)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Friction )( 
            IRamp * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IRamp, put_Friction)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Friction )( 
            IRamp * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IRamp, get_Scatter)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Scatter )( 
            IRamp * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IRamp, put_Scatter)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Scatter )( 
            IRamp * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IRamp, get_DepthBias)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_DepthBias )( 
            IRamp * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IRamp, put_DepthBias)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_DepthBias )( 
            IRamp * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IRamp, get_WireDiameter)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_WireDiameter )( 
            IRamp * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IRamp, put_WireDiameter)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_WireDiameter )( 
            IRamp * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IRamp, get_WireDistanceX)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_WireDistanceX )( 
            IRamp * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IRamp, put_WireDistanceX)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_WireDistanceX )( 
            IRamp * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IRamp, get_WireDistanceY)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_WireDistanceY )( 
            IRamp * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IRamp, put_WireDistanceY)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_WireDistanceY )( 
            IRamp * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IRamp, get_ReflectionEnabled)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ReflectionEnabled )( 
            IRamp * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IRamp, put_ReflectionEnabled)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ReflectionEnabled )( 
            IRamp * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IRamp, get_PhysicsMaterial)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_PhysicsMaterial )( 
            IRamp * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IRamp, put_PhysicsMaterial)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_PhysicsMaterial )( 
            IRamp * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IRamp, get_OverwritePhysics)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_OverwritePhysics )( 
            IRamp * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IRamp, put_OverwritePhysics)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_OverwritePhysics )( 
            IRamp * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        END_INTERFACE
    } IRampVtbl;

    interface IRamp
    {
        CONST_VTBL struct IRampVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRamp_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IRamp_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IRamp_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IRamp_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IRamp_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IRamp_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IRamp_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IRamp_get_Name(This,pVal)	\
    ( (This)->lpVtbl -> get_Name(This,pVal) ) 

#define IRamp_put_Name(This,newVal)	\
    ( (This)->lpVtbl -> put_Name(This,newVal) ) 

#define IRamp_get_TimerEnabled(This,pVal)	\
    ( (This)->lpVtbl -> get_TimerEnabled(This,pVal) ) 

#define IRamp_put_TimerEnabled(This,newVal)	\
    ( (This)->lpVtbl -> put_TimerEnabled(This,newVal) ) 

#define IRamp_get_TimerInterval(This,pVal)	\
    ( (This)->lpVtbl -> get_TimerInterval(This,pVal) ) 

#define IRamp_put_TimerInterval(This,newVal)	\
    ( (This)->lpVtbl -> put_TimerInterval(This,newVal) ) 

#define IRamp_get_HeightBottom(This,pVal)	\
    ( (This)->lpVtbl -> get_HeightBottom(This,pVal) ) 

#define IRamp_put_HeightBottom(This,newVal)	\
    ( (This)->lpVtbl -> put_HeightBottom(This,newVal) ) 

#define IRamp_get_HeightTop(This,pVal)	\
    ( (This)->lpVtbl -> get_HeightTop(This,pVal) ) 

#define IRamp_put_HeightTop(This,newVal)	\
    ( (This)->lpVtbl -> put_HeightTop(This,newVal) ) 

#define IRamp_get_WidthBottom(This,pVal)	\
    ( (This)->lpVtbl -> get_WidthBottom(This,pVal) ) 

#define IRamp_put_WidthBottom(This,newVal)	\
    ( (This)->lpVtbl -> put_WidthBottom(This,newVal) ) 

#define IRamp_get_WidthTop(This,pVal)	\
    ( (This)->lpVtbl -> get_WidthTop(This,pVal) ) 

#define IRamp_put_WidthTop(This,newVal)	\
    ( (This)->lpVtbl -> put_WidthTop(This,newVal) ) 

#define IRamp_get_Material(This,pVal)	\
    ( (This)->lpVtbl -> get_Material(This,pVal) ) 

#define IRamp_put_Material(This,newVal)	\
    ( (This)->lpVtbl -> put_Material(This,newVal) ) 

#define IRamp_get_Type(This,pVal)	\
    ( (This)->lpVtbl -> get_Type(This,pVal) ) 

#define IRamp_put_Type(This,newVal)	\
    ( (This)->lpVtbl -> put_Type(This,newVal) ) 

#define IRamp_get_Image(This,pVal)	\
    ( (This)->lpVtbl -> get_Image(This,pVal) ) 

#define IRamp_put_Image(This,newVal)	\
    ( (This)->lpVtbl -> put_Image(This,newVal) ) 

#define IRamp_get_ImageAlignment(This,pVal)	\
    ( (This)->lpVtbl -> get_ImageAlignment(This,pVal) ) 

#define IRamp_put_ImageAlignment(This,newVal)	\
    ( (This)->lpVtbl -> put_ImageAlignment(This,newVal) ) 

#define IRamp_get_HasWallImage(This,pVal)	\
    ( (This)->lpVtbl -> get_HasWallImage(This,pVal) ) 

#define IRamp_put_HasWallImage(This,newVal)	\
    ( (This)->lpVtbl -> put_HasWallImage(This,newVal) ) 

#define IRamp_get_LeftWallHeight(This,pVal)	\
    ( (This)->lpVtbl -> get_LeftWallHeight(This,pVal) ) 

#define IRamp_put_LeftWallHeight(This,newVal)	\
    ( (This)->lpVtbl -> put_LeftWallHeight(This,newVal) ) 

#define IRamp_get_RightWallHeight(This,pVal)	\
    ( (This)->lpVtbl -> get_RightWallHeight(This,pVal) ) 

#define IRamp_put_RightWallHeight(This,newVal)	\
    ( (This)->lpVtbl -> put_RightWallHeight(This,newVal) ) 

#define IRamp_get_UserValue(This,pVal)	\
    ( (This)->lpVtbl -> get_UserValue(This,pVal) ) 

#define IRamp_put_UserValue(This,newVal)	\
    ( (This)->lpVtbl -> put_UserValue(This,newVal) ) 

#define IRamp_get_VisibleLeftWallHeight(This,pVal)	\
    ( (This)->lpVtbl -> get_VisibleLeftWallHeight(This,pVal) ) 

#define IRamp_put_VisibleLeftWallHeight(This,newVal)	\
    ( (This)->lpVtbl -> put_VisibleLeftWallHeight(This,newVal) ) 

#define IRamp_get_VisibleRightWallHeight(This,pVal)	\
    ( (This)->lpVtbl -> get_VisibleRightWallHeight(This,pVal) ) 

#define IRamp_put_VisibleRightWallHeight(This,newVal)	\
    ( (This)->lpVtbl -> put_VisibleRightWallHeight(This,newVal) ) 

#define IRamp_get_Elasticity(This,pVal)	\
    ( (This)->lpVtbl -> get_Elasticity(This,pVal) ) 

#define IRamp_put_Elasticity(This,newVal)	\
    ( (This)->lpVtbl -> put_Elasticity(This,newVal) ) 

#define IRamp_get_Collidable(This,pVal)	\
    ( (This)->lpVtbl -> get_Collidable(This,pVal) ) 

#define IRamp_put_Collidable(This,newVal)	\
    ( (This)->lpVtbl -> put_Collidable(This,newVal) ) 

#define IRamp_get_HasHitEvent(This,pVal)	\
    ( (This)->lpVtbl -> get_HasHitEvent(This,pVal) ) 

#define IRamp_put_HasHitEvent(This,newVal)	\
    ( (This)->lpVtbl -> put_HasHitEvent(This,newVal) ) 

#define IRamp_get_Threshold(This,pVal)	\
    ( (This)->lpVtbl -> get_Threshold(This,pVal) ) 

#define IRamp_put_Threshold(This,newVal)	\
    ( (This)->lpVtbl -> put_Threshold(This,newVal) ) 

#define IRamp_get_Visible(This,pVal)	\
    ( (This)->lpVtbl -> get_Visible(This,pVal) ) 

#define IRamp_put_Visible(This,newVal)	\
    ( (This)->lpVtbl -> put_Visible(This,newVal) ) 

#define IRamp_get_Friction(This,pVal)	\
    ( (This)->lpVtbl -> get_Friction(This,pVal) ) 

#define IRamp_put_Friction(This,newVal)	\
    ( (This)->lpVtbl -> put_Friction(This,newVal) ) 

#define IRamp_get_Scatter(This,pVal)	\
    ( (This)->lpVtbl -> get_Scatter(This,pVal) ) 

#define IRamp_put_Scatter(This,newVal)	\
    ( (This)->lpVtbl -> put_Scatter(This,newVal) ) 

#define IRamp_get_DepthBias(This,pVal)	\
    ( (This)->lpVtbl -> get_DepthBias(This,pVal) ) 

#define IRamp_put_DepthBias(This,newVal)	\
    ( (This)->lpVtbl -> put_DepthBias(This,newVal) ) 

#define IRamp_get_WireDiameter(This,pVal)	\
    ( (This)->lpVtbl -> get_WireDiameter(This,pVal) ) 

#define IRamp_put_WireDiameter(This,newVal)	\
    ( (This)->lpVtbl -> put_WireDiameter(This,newVal) ) 

#define IRamp_get_WireDistanceX(This,pVal)	\
    ( (This)->lpVtbl -> get_WireDistanceX(This,pVal) ) 

#define IRamp_put_WireDistanceX(This,newVal)	\
    ( (This)->lpVtbl -> put_WireDistanceX(This,newVal) ) 

#define IRamp_get_WireDistanceY(This,pVal)	\
    ( (This)->lpVtbl -> get_WireDistanceY(This,pVal) ) 

#define IRamp_put_WireDistanceY(This,newVal)	\
    ( (This)->lpVtbl -> put_WireDistanceY(This,newVal) ) 

#define IRamp_get_ReflectionEnabled(This,pVal)	\
    ( (This)->lpVtbl -> get_ReflectionEnabled(This,pVal) ) 

#define IRamp_put_ReflectionEnabled(This,newVal)	\
    ( (This)->lpVtbl -> put_ReflectionEnabled(This,newVal) ) 

#define IRamp_get_PhysicsMaterial(This,pVal)	\
    ( (This)->lpVtbl -> get_PhysicsMaterial(This,pVal) ) 

#define IRamp_put_PhysicsMaterial(This,newVal)	\
    ( (This)->lpVtbl -> put_PhysicsMaterial(This,newVal) ) 

#define IRamp_get_OverwritePhysics(This,pVal)	\
    ( (This)->lpVtbl -> get_OverwritePhysics(This,pVal) ) 

#define IRamp_put_OverwritePhysics(This,newVal)	\
    ( (This)->lpVtbl -> put_OverwritePhysics(This,newVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IRamp_INTERFACE_DEFINED__ */


#ifndef __IFlasher_INTERFACE_DEFINED__
#define __IFlasher_INTERFACE_DEFINED__

/* interface IFlasher */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IFlasher;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("87DAB93E-7D6F-4fe4-A5F9-632FD82BDB4A")
    IFlasher : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Name( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_X( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_X( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Y( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Y( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ImageAlignment( 
            /* [retval][out] */ RampImageAlignment *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ImageAlignment( 
            /* [in] */ RampImageAlignment newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Height( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Height( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RotZ( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_RotZ( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RotY( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_RotY( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RotX( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_RotX( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Color( 
            /* [retval][out] */ OLE_COLOR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Color( 
            /* [in] */ OLE_COLOR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ImageA( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ImageA( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ImageB( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ImageB( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DisplayTexture( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_DisplayTexture( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Opacity( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Opacity( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_IntensityScale( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_IntensityScale( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ModulateVsAdd( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ModulateVsAdd( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_UserValue( 
            /* [retval][out] */ VARIANT *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_UserValue( 
            /* [in] */ VARIANT *newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Visible( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Visible( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AddBlend( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_AddBlend( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DMD( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_DMD( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_VideoCapWidth( 
            /* [in] */ long cWidth) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_VideoCapHeight( 
            /* [in] */ long cHeight) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_VideoCapUpdate( 
            /* [in] */ BSTR cWinTitle) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DepthBias( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_DepthBias( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Filter( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Filter( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Amount( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Amount( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TimerEnabled( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TimerEnabled( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TimerInterval( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TimerInterval( 
            /* [in] */ long newVal) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IFlasherVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFlasher * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFlasher * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFlasher * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFlasher * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFlasher * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFlasher * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFlasher * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        DECLSPEC_XFGVIRT(IFlasher, get_Name)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IFlasher * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IFlasher, put_Name)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IFlasher * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IFlasher, get_X)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_X )( 
            IFlasher * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IFlasher, put_X)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_X )( 
            IFlasher * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IFlasher, get_Y)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Y )( 
            IFlasher * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IFlasher, put_Y)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Y )( 
            IFlasher * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IFlasher, get_ImageAlignment)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ImageAlignment )( 
            IFlasher * This,
            /* [retval][out] */ RampImageAlignment *pVal);
        
        DECLSPEC_XFGVIRT(IFlasher, put_ImageAlignment)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ImageAlignment )( 
            IFlasher * This,
            /* [in] */ RampImageAlignment newVal);
        
        DECLSPEC_XFGVIRT(IFlasher, get_Height)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Height )( 
            IFlasher * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IFlasher, put_Height)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Height )( 
            IFlasher * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IFlasher, get_RotZ)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RotZ )( 
            IFlasher * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IFlasher, put_RotZ)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_RotZ )( 
            IFlasher * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IFlasher, get_RotY)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RotY )( 
            IFlasher * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IFlasher, put_RotY)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_RotY )( 
            IFlasher * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IFlasher, get_RotX)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RotX )( 
            IFlasher * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IFlasher, put_RotX)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_RotX )( 
            IFlasher * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IFlasher, get_Color)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Color )( 
            IFlasher * This,
            /* [retval][out] */ OLE_COLOR *pVal);
        
        DECLSPEC_XFGVIRT(IFlasher, put_Color)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Color )( 
            IFlasher * This,
            /* [in] */ OLE_COLOR newVal);
        
        DECLSPEC_XFGVIRT(IFlasher, get_ImageA)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ImageA )( 
            IFlasher * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IFlasher, put_ImageA)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ImageA )( 
            IFlasher * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IFlasher, get_ImageB)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ImageB )( 
            IFlasher * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IFlasher, put_ImageB)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ImageB )( 
            IFlasher * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IFlasher, get_DisplayTexture)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_DisplayTexture )( 
            IFlasher * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IFlasher, put_DisplayTexture)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_DisplayTexture )( 
            IFlasher * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IFlasher, get_Opacity)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Opacity )( 
            IFlasher * This,
            /* [retval][out] */ long *pVal);
        
        DECLSPEC_XFGVIRT(IFlasher, put_Opacity)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Opacity )( 
            IFlasher * This,
            /* [in] */ long newVal);
        
        DECLSPEC_XFGVIRT(IFlasher, get_IntensityScale)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_IntensityScale )( 
            IFlasher * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IFlasher, put_IntensityScale)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_IntensityScale )( 
            IFlasher * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IFlasher, get_ModulateVsAdd)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ModulateVsAdd )( 
            IFlasher * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IFlasher, put_ModulateVsAdd)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ModulateVsAdd )( 
            IFlasher * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IFlasher, get_UserValue)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_UserValue )( 
            IFlasher * This,
            /* [retval][out] */ VARIANT *pVal);
        
        DECLSPEC_XFGVIRT(IFlasher, put_UserValue)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_UserValue )( 
            IFlasher * This,
            /* [in] */ VARIANT *newVal);
        
        DECLSPEC_XFGVIRT(IFlasher, get_Visible)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Visible )( 
            IFlasher * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IFlasher, put_Visible)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Visible )( 
            IFlasher * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IFlasher, get_AddBlend)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AddBlend )( 
            IFlasher * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IFlasher, put_AddBlend)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_AddBlend )( 
            IFlasher * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IFlasher, get_DMD)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_DMD )( 
            IFlasher * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IFlasher, put_DMD)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_DMD )( 
            IFlasher * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IFlasher, put_VideoCapWidth)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_VideoCapWidth )( 
            IFlasher * This,
            /* [in] */ long cWidth);
        
        DECLSPEC_XFGVIRT(IFlasher, put_VideoCapHeight)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_VideoCapHeight )( 
            IFlasher * This,
            /* [in] */ long cHeight);
        
        DECLSPEC_XFGVIRT(IFlasher, put_VideoCapUpdate)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_VideoCapUpdate )( 
            IFlasher * This,
            /* [in] */ BSTR cWinTitle);
        
        DECLSPEC_XFGVIRT(IFlasher, get_DepthBias)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_DepthBias )( 
            IFlasher * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IFlasher, put_DepthBias)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_DepthBias )( 
            IFlasher * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IFlasher, get_Filter)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Filter )( 
            IFlasher * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IFlasher, put_Filter)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Filter )( 
            IFlasher * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IFlasher, get_Amount)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Amount )( 
            IFlasher * This,
            /* [retval][out] */ long *pVal);
        
        DECLSPEC_XFGVIRT(IFlasher, put_Amount)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Amount )( 
            IFlasher * This,
            /* [in] */ long newVal);
        
        DECLSPEC_XFGVIRT(IFlasher, get_TimerEnabled)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TimerEnabled )( 
            IFlasher * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IFlasher, put_TimerEnabled)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TimerEnabled )( 
            IFlasher * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IFlasher, get_TimerInterval)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TimerInterval )( 
            IFlasher * This,
            /* [retval][out] */ long *pVal);
        
        DECLSPEC_XFGVIRT(IFlasher, put_TimerInterval)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TimerInterval )( 
            IFlasher * This,
            /* [in] */ long newVal);
        
        END_INTERFACE
    } IFlasherVtbl;

    interface IFlasher
    {
        CONST_VTBL struct IFlasherVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFlasher_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IFlasher_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IFlasher_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IFlasher_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IFlasher_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IFlasher_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IFlasher_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IFlasher_get_Name(This,pVal)	\
    ( (This)->lpVtbl -> get_Name(This,pVal) ) 

#define IFlasher_put_Name(This,newVal)	\
    ( (This)->lpVtbl -> put_Name(This,newVal) ) 

#define IFlasher_get_X(This,pVal)	\
    ( (This)->lpVtbl -> get_X(This,pVal) ) 

#define IFlasher_put_X(This,newVal)	\
    ( (This)->lpVtbl -> put_X(This,newVal) ) 

#define IFlasher_get_Y(This,pVal)	\
    ( (This)->lpVtbl -> get_Y(This,pVal) ) 

#define IFlasher_put_Y(This,newVal)	\
    ( (This)->lpVtbl -> put_Y(This,newVal) ) 

#define IFlasher_get_ImageAlignment(This,pVal)	\
    ( (This)->lpVtbl -> get_ImageAlignment(This,pVal) ) 

#define IFlasher_put_ImageAlignment(This,newVal)	\
    ( (This)->lpVtbl -> put_ImageAlignment(This,newVal) ) 

#define IFlasher_get_Height(This,pVal)	\
    ( (This)->lpVtbl -> get_Height(This,pVal) ) 

#define IFlasher_put_Height(This,newVal)	\
    ( (This)->lpVtbl -> put_Height(This,newVal) ) 

#define IFlasher_get_RotZ(This,pVal)	\
    ( (This)->lpVtbl -> get_RotZ(This,pVal) ) 

#define IFlasher_put_RotZ(This,newVal)	\
    ( (This)->lpVtbl -> put_RotZ(This,newVal) ) 

#define IFlasher_get_RotY(This,pVal)	\
    ( (This)->lpVtbl -> get_RotY(This,pVal) ) 

#define IFlasher_put_RotY(This,newVal)	\
    ( (This)->lpVtbl -> put_RotY(This,newVal) ) 

#define IFlasher_get_RotX(This,pVal)	\
    ( (This)->lpVtbl -> get_RotX(This,pVal) ) 

#define IFlasher_put_RotX(This,newVal)	\
    ( (This)->lpVtbl -> put_RotX(This,newVal) ) 

#define IFlasher_get_Color(This,pVal)	\
    ( (This)->lpVtbl -> get_Color(This,pVal) ) 

#define IFlasher_put_Color(This,newVal)	\
    ( (This)->lpVtbl -> put_Color(This,newVal) ) 

#define IFlasher_get_ImageA(This,pVal)	\
    ( (This)->lpVtbl -> get_ImageA(This,pVal) ) 

#define IFlasher_put_ImageA(This,newVal)	\
    ( (This)->lpVtbl -> put_ImageA(This,newVal) ) 

#define IFlasher_get_ImageB(This,pVal)	\
    ( (This)->lpVtbl -> get_ImageB(This,pVal) ) 

#define IFlasher_put_ImageB(This,newVal)	\
    ( (This)->lpVtbl -> put_ImageB(This,newVal) ) 

#define IFlasher_get_DisplayTexture(This,pVal)	\
    ( (This)->lpVtbl -> get_DisplayTexture(This,pVal) ) 

#define IFlasher_put_DisplayTexture(This,newVal)	\
    ( (This)->lpVtbl -> put_DisplayTexture(This,newVal) ) 

#define IFlasher_get_Opacity(This,pVal)	\
    ( (This)->lpVtbl -> get_Opacity(This,pVal) ) 

#define IFlasher_put_Opacity(This,newVal)	\
    ( (This)->lpVtbl -> put_Opacity(This,newVal) ) 

#define IFlasher_get_IntensityScale(This,pVal)	\
    ( (This)->lpVtbl -> get_IntensityScale(This,pVal) ) 

#define IFlasher_put_IntensityScale(This,newVal)	\
    ( (This)->lpVtbl -> put_IntensityScale(This,newVal) ) 

#define IFlasher_get_ModulateVsAdd(This,pVal)	\
    ( (This)->lpVtbl -> get_ModulateVsAdd(This,pVal) ) 

#define IFlasher_put_ModulateVsAdd(This,newVal)	\
    ( (This)->lpVtbl -> put_ModulateVsAdd(This,newVal) ) 

#define IFlasher_get_UserValue(This,pVal)	\
    ( (This)->lpVtbl -> get_UserValue(This,pVal) ) 

#define IFlasher_put_UserValue(This,newVal)	\
    ( (This)->lpVtbl -> put_UserValue(This,newVal) ) 

#define IFlasher_get_Visible(This,pVal)	\
    ( (This)->lpVtbl -> get_Visible(This,pVal) ) 

#define IFlasher_put_Visible(This,newVal)	\
    ( (This)->lpVtbl -> put_Visible(This,newVal) ) 

#define IFlasher_get_AddBlend(This,pVal)	\
    ( (This)->lpVtbl -> get_AddBlend(This,pVal) ) 

#define IFlasher_put_AddBlend(This,newVal)	\
    ( (This)->lpVtbl -> put_AddBlend(This,newVal) ) 

#define IFlasher_get_DMD(This,pVal)	\
    ( (This)->lpVtbl -> get_DMD(This,pVal) ) 

#define IFlasher_put_DMD(This,newVal)	\
    ( (This)->lpVtbl -> put_DMD(This,newVal) ) 

#define IFlasher_put_VideoCapWidth(This,cWidth)	\
    ( (This)->lpVtbl -> put_VideoCapWidth(This,cWidth) ) 

#define IFlasher_put_VideoCapHeight(This,cHeight)	\
    ( (This)->lpVtbl -> put_VideoCapHeight(This,cHeight) ) 

#define IFlasher_put_VideoCapUpdate(This,cWinTitle)	\
    ( (This)->lpVtbl -> put_VideoCapUpdate(This,cWinTitle) ) 

#define IFlasher_get_DepthBias(This,pVal)	\
    ( (This)->lpVtbl -> get_DepthBias(This,pVal) ) 

#define IFlasher_put_DepthBias(This,newVal)	\
    ( (This)->lpVtbl -> put_DepthBias(This,newVal) ) 

#define IFlasher_get_Filter(This,pVal)	\
    ( (This)->lpVtbl -> get_Filter(This,pVal) ) 

#define IFlasher_put_Filter(This,newVal)	\
    ( (This)->lpVtbl -> put_Filter(This,newVal) ) 

#define IFlasher_get_Amount(This,pVal)	\
    ( (This)->lpVtbl -> get_Amount(This,pVal) ) 

#define IFlasher_put_Amount(This,newVal)	\
    ( (This)->lpVtbl -> put_Amount(This,newVal) ) 

#define IFlasher_get_TimerEnabled(This,pVal)	\
    ( (This)->lpVtbl -> get_TimerEnabled(This,pVal) ) 

#define IFlasher_put_TimerEnabled(This,newVal)	\
    ( (This)->lpVtbl -> put_TimerEnabled(This,newVal) ) 

#define IFlasher_get_TimerInterval(This,pVal)	\
    ( (This)->lpVtbl -> get_TimerInterval(This,pVal) ) 

#define IFlasher_put_TimerInterval(This,newVal)	\
    ( (This)->lpVtbl -> put_TimerInterval(This,newVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IFlasher_INTERFACE_DEFINED__ */


#ifndef __IRubber_INTERFACE_DEFINED__
#define __IRubber_INTERFACE_DEFINED__

/* interface IRubber */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IRubber;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("496CC241-134D-4543-9974-5B0DF6BC5D1D")
    IRubber : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Name( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Height( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Height( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_HitHeight( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_HitHeight( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_HasHitEvent( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_HasHitEvent( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Thickness( 
            /* [retval][out] */ int *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Thickness( 
            /* [in] */ int newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RotX( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_RotX( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RotZ( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_RotZ( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RotY( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_RotY( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Material( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Material( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Image( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Image( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Elasticity( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Elasticity( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ElasticityFalloff( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ElasticityFalloff( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Collidable( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Collidable( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Visible( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Visible( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Friction( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Friction( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Scatter( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Scatter( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_EnableStaticRendering( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_EnableStaticRendering( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_EnableShowInEditor( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_EnableShowInEditor( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ReflectionEnabled( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ReflectionEnabled( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TimerEnabled( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TimerEnabled( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TimerInterval( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TimerInterval( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_UserValue( 
            /* [retval][out] */ VARIANT *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_UserValue( 
            /* [in] */ VARIANT *newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_PhysicsMaterial( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_PhysicsMaterial( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_OverwritePhysics( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_OverwritePhysics( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IRubberVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRubber * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRubber * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRubber * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IRubber * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IRubber * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IRubber * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IRubber * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        DECLSPEC_XFGVIRT(IRubber, get_Name)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IRubber * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IRubber, put_Name)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IRubber * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IRubber, get_Height)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Height )( 
            IRubber * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IRubber, put_Height)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Height )( 
            IRubber * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IRubber, get_HitHeight)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HitHeight )( 
            IRubber * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IRubber, put_HitHeight)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_HitHeight )( 
            IRubber * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IRubber, get_HasHitEvent)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HasHitEvent )( 
            IRubber * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IRubber, put_HasHitEvent)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_HasHitEvent )( 
            IRubber * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IRubber, get_Thickness)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Thickness )( 
            IRubber * This,
            /* [retval][out] */ int *pVal);
        
        DECLSPEC_XFGVIRT(IRubber, put_Thickness)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Thickness )( 
            IRubber * This,
            /* [in] */ int newVal);
        
        DECLSPEC_XFGVIRT(IRubber, get_RotX)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RotX )( 
            IRubber * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IRubber, put_RotX)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_RotX )( 
            IRubber * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IRubber, get_RotZ)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RotZ )( 
            IRubber * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IRubber, put_RotZ)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_RotZ )( 
            IRubber * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IRubber, get_RotY)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RotY )( 
            IRubber * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IRubber, put_RotY)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_RotY )( 
            IRubber * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IRubber, get_Material)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Material )( 
            IRubber * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IRubber, put_Material)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Material )( 
            IRubber * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IRubber, get_Image)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Image )( 
            IRubber * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IRubber, put_Image)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Image )( 
            IRubber * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IRubber, get_Elasticity)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Elasticity )( 
            IRubber * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IRubber, put_Elasticity)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Elasticity )( 
            IRubber * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IRubber, get_ElasticityFalloff)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ElasticityFalloff )( 
            IRubber * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IRubber, put_ElasticityFalloff)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ElasticityFalloff )( 
            IRubber * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IRubber, get_Collidable)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Collidable )( 
            IRubber * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IRubber, put_Collidable)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Collidable )( 
            IRubber * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IRubber, get_Visible)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Visible )( 
            IRubber * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IRubber, put_Visible)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Visible )( 
            IRubber * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IRubber, get_Friction)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Friction )( 
            IRubber * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IRubber, put_Friction)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Friction )( 
            IRubber * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IRubber, get_Scatter)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Scatter )( 
            IRubber * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IRubber, put_Scatter)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Scatter )( 
            IRubber * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IRubber, get_EnableStaticRendering)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_EnableStaticRendering )( 
            IRubber * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IRubber, put_EnableStaticRendering)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_EnableStaticRendering )( 
            IRubber * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IRubber, get_EnableShowInEditor)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_EnableShowInEditor )( 
            IRubber * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IRubber, put_EnableShowInEditor)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_EnableShowInEditor )( 
            IRubber * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IRubber, get_ReflectionEnabled)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ReflectionEnabled )( 
            IRubber * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IRubber, put_ReflectionEnabled)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ReflectionEnabled )( 
            IRubber * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IRubber, get_TimerEnabled)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TimerEnabled )( 
            IRubber * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IRubber, put_TimerEnabled)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TimerEnabled )( 
            IRubber * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IRubber, get_TimerInterval)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TimerInterval )( 
            IRubber * This,
            /* [retval][out] */ long *pVal);
        
        DECLSPEC_XFGVIRT(IRubber, put_TimerInterval)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TimerInterval )( 
            IRubber * This,
            /* [in] */ long newVal);
        
        DECLSPEC_XFGVIRT(IRubber, get_UserValue)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_UserValue )( 
            IRubber * This,
            /* [retval][out] */ VARIANT *pVal);
        
        DECLSPEC_XFGVIRT(IRubber, put_UserValue)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_UserValue )( 
            IRubber * This,
            /* [in] */ VARIANT *newVal);
        
        DECLSPEC_XFGVIRT(IRubber, get_PhysicsMaterial)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_PhysicsMaterial )( 
            IRubber * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IRubber, put_PhysicsMaterial)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_PhysicsMaterial )( 
            IRubber * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IRubber, get_OverwritePhysics)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_OverwritePhysics )( 
            IRubber * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IRubber, put_OverwritePhysics)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_OverwritePhysics )( 
            IRubber * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        END_INTERFACE
    } IRubberVtbl;

    interface IRubber
    {
        CONST_VTBL struct IRubberVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRubber_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IRubber_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IRubber_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IRubber_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IRubber_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IRubber_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IRubber_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IRubber_get_Name(This,pVal)	\
    ( (This)->lpVtbl -> get_Name(This,pVal) ) 

#define IRubber_put_Name(This,newVal)	\
    ( (This)->lpVtbl -> put_Name(This,newVal) ) 

#define IRubber_get_Height(This,pVal)	\
    ( (This)->lpVtbl -> get_Height(This,pVal) ) 

#define IRubber_put_Height(This,newVal)	\
    ( (This)->lpVtbl -> put_Height(This,newVal) ) 

#define IRubber_get_HitHeight(This,pVal)	\
    ( (This)->lpVtbl -> get_HitHeight(This,pVal) ) 

#define IRubber_put_HitHeight(This,newVal)	\
    ( (This)->lpVtbl -> put_HitHeight(This,newVal) ) 

#define IRubber_get_HasHitEvent(This,pVal)	\
    ( (This)->lpVtbl -> get_HasHitEvent(This,pVal) ) 

#define IRubber_put_HasHitEvent(This,newVal)	\
    ( (This)->lpVtbl -> put_HasHitEvent(This,newVal) ) 

#define IRubber_get_Thickness(This,pVal)	\
    ( (This)->lpVtbl -> get_Thickness(This,pVal) ) 

#define IRubber_put_Thickness(This,newVal)	\
    ( (This)->lpVtbl -> put_Thickness(This,newVal) ) 

#define IRubber_get_RotX(This,pVal)	\
    ( (This)->lpVtbl -> get_RotX(This,pVal) ) 

#define IRubber_put_RotX(This,newVal)	\
    ( (This)->lpVtbl -> put_RotX(This,newVal) ) 

#define IRubber_get_RotZ(This,pVal)	\
    ( (This)->lpVtbl -> get_RotZ(This,pVal) ) 

#define IRubber_put_RotZ(This,newVal)	\
    ( (This)->lpVtbl -> put_RotZ(This,newVal) ) 

#define IRubber_get_RotY(This,pVal)	\
    ( (This)->lpVtbl -> get_RotY(This,pVal) ) 

#define IRubber_put_RotY(This,newVal)	\
    ( (This)->lpVtbl -> put_RotY(This,newVal) ) 

#define IRubber_get_Material(This,pVal)	\
    ( (This)->lpVtbl -> get_Material(This,pVal) ) 

#define IRubber_put_Material(This,newVal)	\
    ( (This)->lpVtbl -> put_Material(This,newVal) ) 

#define IRubber_get_Image(This,pVal)	\
    ( (This)->lpVtbl -> get_Image(This,pVal) ) 

#define IRubber_put_Image(This,newVal)	\
    ( (This)->lpVtbl -> put_Image(This,newVal) ) 

#define IRubber_get_Elasticity(This,pVal)	\
    ( (This)->lpVtbl -> get_Elasticity(This,pVal) ) 

#define IRubber_put_Elasticity(This,newVal)	\
    ( (This)->lpVtbl -> put_Elasticity(This,newVal) ) 

#define IRubber_get_ElasticityFalloff(This,pVal)	\
    ( (This)->lpVtbl -> get_ElasticityFalloff(This,pVal) ) 

#define IRubber_put_ElasticityFalloff(This,newVal)	\
    ( (This)->lpVtbl -> put_ElasticityFalloff(This,newVal) ) 

#define IRubber_get_Collidable(This,pVal)	\
    ( (This)->lpVtbl -> get_Collidable(This,pVal) ) 

#define IRubber_put_Collidable(This,newVal)	\
    ( (This)->lpVtbl -> put_Collidable(This,newVal) ) 

#define IRubber_get_Visible(This,pVal)	\
    ( (This)->lpVtbl -> get_Visible(This,pVal) ) 

#define IRubber_put_Visible(This,newVal)	\
    ( (This)->lpVtbl -> put_Visible(This,newVal) ) 

#define IRubber_get_Friction(This,pVal)	\
    ( (This)->lpVtbl -> get_Friction(This,pVal) ) 

#define IRubber_put_Friction(This,newVal)	\
    ( (This)->lpVtbl -> put_Friction(This,newVal) ) 

#define IRubber_get_Scatter(This,pVal)	\
    ( (This)->lpVtbl -> get_Scatter(This,pVal) ) 

#define IRubber_put_Scatter(This,newVal)	\
    ( (This)->lpVtbl -> put_Scatter(This,newVal) ) 

#define IRubber_get_EnableStaticRendering(This,pVal)	\
    ( (This)->lpVtbl -> get_EnableStaticRendering(This,pVal) ) 

#define IRubber_put_EnableStaticRendering(This,newVal)	\
    ( (This)->lpVtbl -> put_EnableStaticRendering(This,newVal) ) 

#define IRubber_get_EnableShowInEditor(This,pVal)	\
    ( (This)->lpVtbl -> get_EnableShowInEditor(This,pVal) ) 

#define IRubber_put_EnableShowInEditor(This,newVal)	\
    ( (This)->lpVtbl -> put_EnableShowInEditor(This,newVal) ) 

#define IRubber_get_ReflectionEnabled(This,pVal)	\
    ( (This)->lpVtbl -> get_ReflectionEnabled(This,pVal) ) 

#define IRubber_put_ReflectionEnabled(This,newVal)	\
    ( (This)->lpVtbl -> put_ReflectionEnabled(This,newVal) ) 

#define IRubber_get_TimerEnabled(This,pVal)	\
    ( (This)->lpVtbl -> get_TimerEnabled(This,pVal) ) 

#define IRubber_put_TimerEnabled(This,newVal)	\
    ( (This)->lpVtbl -> put_TimerEnabled(This,newVal) ) 

#define IRubber_get_TimerInterval(This,pVal)	\
    ( (This)->lpVtbl -> get_TimerInterval(This,pVal) ) 

#define IRubber_put_TimerInterval(This,newVal)	\
    ( (This)->lpVtbl -> put_TimerInterval(This,newVal) ) 

#define IRubber_get_UserValue(This,pVal)	\
    ( (This)->lpVtbl -> get_UserValue(This,pVal) ) 

#define IRubber_put_UserValue(This,newVal)	\
    ( (This)->lpVtbl -> put_UserValue(This,newVal) ) 

#define IRubber_get_PhysicsMaterial(This,pVal)	\
    ( (This)->lpVtbl -> get_PhysicsMaterial(This,pVal) ) 

#define IRubber_put_PhysicsMaterial(This,newVal)	\
    ( (This)->lpVtbl -> put_PhysicsMaterial(This,newVal) ) 

#define IRubber_get_OverwritePhysics(This,pVal)	\
    ( (This)->lpVtbl -> get_OverwritePhysics(This,pVal) ) 

#define IRubber_put_OverwritePhysics(This,newVal)	\
    ( (This)->lpVtbl -> put_OverwritePhysics(This,newVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IRubber_INTERFACE_DEFINED__ */


#ifndef __IBall_INTERFACE_DEFINED__
#define __IBall_INTERFACE_DEFINED__

/* interface IBall */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IBall;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CE5E583F-1264-4927-8509-B1359CB23034")
    IBall : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_X( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_X( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Y( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Y( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VelX( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_VelX( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VelY( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_VelY( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Z( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Z( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VelZ( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_VelZ( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AngVelX( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_AngVelX( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AngVelY( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_AngVelY( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AngVelZ( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_AngVelZ( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AngMomX( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_AngMomX( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AngMomY( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_AngMomY( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AngMomZ( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_AngMomZ( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Color( 
            /* [retval][out] */ OLE_COLOR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Color( 
            /* [in] */ OLE_COLOR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Image( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Image( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_FrontDecal( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_FrontDecal( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DecalMode( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_DecalMode( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_UserValue( 
            /* [retval][out] */ VARIANT *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_UserValue( 
            /* [in] */ VARIANT *newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Mass( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Mass( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Radius( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Radius( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ID( 
            /* [retval][out] */ int *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ID( 
            /* [in] */ int newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Name( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BulbIntensityScale( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_BulbIntensityScale( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ReflectionEnabled( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ReflectionEnabled( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_PlayfieldReflectionScale( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_PlayfieldReflectionScale( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ForceReflection( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ForceReflection( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Visible( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Visible( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE DestroyBall( 
            /* [retval][out] */ int *pVal) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IBallVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IBall * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IBall * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IBall * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IBall * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IBall * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IBall * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IBall * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        DECLSPEC_XFGVIRT(IBall, get_X)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_X )( 
            IBall * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IBall, put_X)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_X )( 
            IBall * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IBall, get_Y)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Y )( 
            IBall * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IBall, put_Y)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Y )( 
            IBall * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IBall, get_VelX)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VelX )( 
            IBall * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IBall, put_VelX)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_VelX )( 
            IBall * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IBall, get_VelY)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VelY )( 
            IBall * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IBall, put_VelY)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_VelY )( 
            IBall * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IBall, get_Z)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Z )( 
            IBall * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IBall, put_Z)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Z )( 
            IBall * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IBall, get_VelZ)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VelZ )( 
            IBall * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IBall, put_VelZ)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_VelZ )( 
            IBall * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IBall, get_AngVelX)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AngVelX )( 
            IBall * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IBall, put_AngVelX)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_AngVelX )( 
            IBall * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IBall, get_AngVelY)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AngVelY )( 
            IBall * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IBall, put_AngVelY)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_AngVelY )( 
            IBall * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IBall, get_AngVelZ)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AngVelZ )( 
            IBall * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IBall, put_AngVelZ)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_AngVelZ )( 
            IBall * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IBall, get_AngMomX)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AngMomX )( 
            IBall * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IBall, put_AngMomX)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_AngMomX )( 
            IBall * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IBall, get_AngMomY)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AngMomY )( 
            IBall * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IBall, put_AngMomY)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_AngMomY )( 
            IBall * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IBall, get_AngMomZ)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AngMomZ )( 
            IBall * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IBall, put_AngMomZ)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_AngMomZ )( 
            IBall * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IBall, get_Color)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Color )( 
            IBall * This,
            /* [retval][out] */ OLE_COLOR *pVal);
        
        DECLSPEC_XFGVIRT(IBall, put_Color)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Color )( 
            IBall * This,
            /* [in] */ OLE_COLOR newVal);
        
        DECLSPEC_XFGVIRT(IBall, get_Image)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Image )( 
            IBall * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IBall, put_Image)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Image )( 
            IBall * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IBall, get_FrontDecal)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_FrontDecal )( 
            IBall * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IBall, put_FrontDecal)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_FrontDecal )( 
            IBall * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IBall, get_DecalMode)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_DecalMode )( 
            IBall * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IBall, put_DecalMode)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_DecalMode )( 
            IBall * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IBall, get_UserValue)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_UserValue )( 
            IBall * This,
            /* [retval][out] */ VARIANT *pVal);
        
        DECLSPEC_XFGVIRT(IBall, put_UserValue)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_UserValue )( 
            IBall * This,
            /* [in] */ VARIANT *newVal);
        
        DECLSPEC_XFGVIRT(IBall, get_Mass)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Mass )( 
            IBall * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IBall, put_Mass)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Mass )( 
            IBall * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IBall, get_Radius)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Radius )( 
            IBall * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IBall, put_Radius)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Radius )( 
            IBall * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IBall, get_ID)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ID )( 
            IBall * This,
            /* [retval][out] */ int *pVal);
        
        DECLSPEC_XFGVIRT(IBall, put_ID)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ID )( 
            IBall * This,
            /* [in] */ int newVal);
        
        DECLSPEC_XFGVIRT(IBall, get_Name)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IBall * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IBall, put_Name)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IBall * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IBall, get_BulbIntensityScale)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BulbIntensityScale )( 
            IBall * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IBall, put_BulbIntensityScale)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_BulbIntensityScale )( 
            IBall * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IBall, get_ReflectionEnabled)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ReflectionEnabled )( 
            IBall * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IBall, put_ReflectionEnabled)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ReflectionEnabled )( 
            IBall * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IBall, get_PlayfieldReflectionScale)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_PlayfieldReflectionScale )( 
            IBall * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IBall, put_PlayfieldReflectionScale)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_PlayfieldReflectionScale )( 
            IBall * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IBall, get_ForceReflection)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ForceReflection )( 
            IBall * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IBall, put_ForceReflection)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ForceReflection )( 
            IBall * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IBall, get_Visible)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Visible )( 
            IBall * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IBall, put_Visible)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Visible )( 
            IBall * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IBall, DestroyBall)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *DestroyBall )( 
            IBall * This,
            /* [retval][out] */ int *pVal);
        
        END_INTERFACE
    } IBallVtbl;

    interface IBall
    {
        CONST_VTBL struct IBallVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBall_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IBall_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IBall_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IBall_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IBall_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IBall_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IBall_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IBall_get_X(This,pVal)	\
    ( (This)->lpVtbl -> get_X(This,pVal) ) 

#define IBall_put_X(This,newVal)	\
    ( (This)->lpVtbl -> put_X(This,newVal) ) 

#define IBall_get_Y(This,pVal)	\
    ( (This)->lpVtbl -> get_Y(This,pVal) ) 

#define IBall_put_Y(This,newVal)	\
    ( (This)->lpVtbl -> put_Y(This,newVal) ) 

#define IBall_get_VelX(This,pVal)	\
    ( (This)->lpVtbl -> get_VelX(This,pVal) ) 

#define IBall_put_VelX(This,newVal)	\
    ( (This)->lpVtbl -> put_VelX(This,newVal) ) 

#define IBall_get_VelY(This,pVal)	\
    ( (This)->lpVtbl -> get_VelY(This,pVal) ) 

#define IBall_put_VelY(This,newVal)	\
    ( (This)->lpVtbl -> put_VelY(This,newVal) ) 

#define IBall_get_Z(This,pVal)	\
    ( (This)->lpVtbl -> get_Z(This,pVal) ) 

#define IBall_put_Z(This,newVal)	\
    ( (This)->lpVtbl -> put_Z(This,newVal) ) 

#define IBall_get_VelZ(This,pVal)	\
    ( (This)->lpVtbl -> get_VelZ(This,pVal) ) 

#define IBall_put_VelZ(This,newVal)	\
    ( (This)->lpVtbl -> put_VelZ(This,newVal) ) 

#define IBall_get_AngVelX(This,pVal)	\
    ( (This)->lpVtbl -> get_AngVelX(This,pVal) ) 

#define IBall_put_AngVelX(This,newVal)	\
    ( (This)->lpVtbl -> put_AngVelX(This,newVal) ) 

#define IBall_get_AngVelY(This,pVal)	\
    ( (This)->lpVtbl -> get_AngVelY(This,pVal) ) 

#define IBall_put_AngVelY(This,newVal)	\
    ( (This)->lpVtbl -> put_AngVelY(This,newVal) ) 

#define IBall_get_AngVelZ(This,pVal)	\
    ( (This)->lpVtbl -> get_AngVelZ(This,pVal) ) 

#define IBall_put_AngVelZ(This,newVal)	\
    ( (This)->lpVtbl -> put_AngVelZ(This,newVal) ) 

#define IBall_get_AngMomX(This,pVal)	\
    ( (This)->lpVtbl -> get_AngMomX(This,pVal) ) 

#define IBall_put_AngMomX(This,newVal)	\
    ( (This)->lpVtbl -> put_AngMomX(This,newVal) ) 

#define IBall_get_AngMomY(This,pVal)	\
    ( (This)->lpVtbl -> get_AngMomY(This,pVal) ) 

#define IBall_put_AngMomY(This,newVal)	\
    ( (This)->lpVtbl -> put_AngMomY(This,newVal) ) 

#define IBall_get_AngMomZ(This,pVal)	\
    ( (This)->lpVtbl -> get_AngMomZ(This,pVal) ) 

#define IBall_put_AngMomZ(This,newVal)	\
    ( (This)->lpVtbl -> put_AngMomZ(This,newVal) ) 

#define IBall_get_Color(This,pVal)	\
    ( (This)->lpVtbl -> get_Color(This,pVal) ) 

#define IBall_put_Color(This,newVal)	\
    ( (This)->lpVtbl -> put_Color(This,newVal) ) 

#define IBall_get_Image(This,pVal)	\
    ( (This)->lpVtbl -> get_Image(This,pVal) ) 

#define IBall_put_Image(This,newVal)	\
    ( (This)->lpVtbl -> put_Image(This,newVal) ) 

#define IBall_get_FrontDecal(This,pVal)	\
    ( (This)->lpVtbl -> get_FrontDecal(This,pVal) ) 

#define IBall_put_FrontDecal(This,newVal)	\
    ( (This)->lpVtbl -> put_FrontDecal(This,newVal) ) 

#define IBall_get_DecalMode(This,pVal)	\
    ( (This)->lpVtbl -> get_DecalMode(This,pVal) ) 

#define IBall_put_DecalMode(This,newVal)	\
    ( (This)->lpVtbl -> put_DecalMode(This,newVal) ) 

#define IBall_get_UserValue(This,pVal)	\
    ( (This)->lpVtbl -> get_UserValue(This,pVal) ) 

#define IBall_put_UserValue(This,newVal)	\
    ( (This)->lpVtbl -> put_UserValue(This,newVal) ) 

#define IBall_get_Mass(This,pVal)	\
    ( (This)->lpVtbl -> get_Mass(This,pVal) ) 

#define IBall_put_Mass(This,newVal)	\
    ( (This)->lpVtbl -> put_Mass(This,newVal) ) 

#define IBall_get_Radius(This,pVal)	\
    ( (This)->lpVtbl -> get_Radius(This,pVal) ) 

#define IBall_put_Radius(This,newVal)	\
    ( (This)->lpVtbl -> put_Radius(This,newVal) ) 

#define IBall_get_ID(This,pVal)	\
    ( (This)->lpVtbl -> get_ID(This,pVal) ) 

#define IBall_put_ID(This,newVal)	\
    ( (This)->lpVtbl -> put_ID(This,newVal) ) 

#define IBall_get_Name(This,pVal)	\
    ( (This)->lpVtbl -> get_Name(This,pVal) ) 

#define IBall_put_Name(This,newVal)	\
    ( (This)->lpVtbl -> put_Name(This,newVal) ) 

#define IBall_get_BulbIntensityScale(This,pVal)	\
    ( (This)->lpVtbl -> get_BulbIntensityScale(This,pVal) ) 

#define IBall_put_BulbIntensityScale(This,newVal)	\
    ( (This)->lpVtbl -> put_BulbIntensityScale(This,newVal) ) 

#define IBall_get_ReflectionEnabled(This,pVal)	\
    ( (This)->lpVtbl -> get_ReflectionEnabled(This,pVal) ) 

#define IBall_put_ReflectionEnabled(This,newVal)	\
    ( (This)->lpVtbl -> put_ReflectionEnabled(This,newVal) ) 

#define IBall_get_PlayfieldReflectionScale(This,pVal)	\
    ( (This)->lpVtbl -> get_PlayfieldReflectionScale(This,pVal) ) 

#define IBall_put_PlayfieldReflectionScale(This,newVal)	\
    ( (This)->lpVtbl -> put_PlayfieldReflectionScale(This,newVal) ) 

#define IBall_get_ForceReflection(This,pVal)	\
    ( (This)->lpVtbl -> get_ForceReflection(This,pVal) ) 

#define IBall_put_ForceReflection(This,newVal)	\
    ( (This)->lpVtbl -> put_ForceReflection(This,newVal) ) 

#define IBall_get_Visible(This,pVal)	\
    ( (This)->lpVtbl -> get_Visible(This,pVal) ) 

#define IBall_put_Visible(This,newVal)	\
    ( (This)->lpVtbl -> put_Visible(This,newVal) ) 

#define IBall_DestroyBall(This,pVal)	\
    ( (This)->lpVtbl -> DestroyBall(This,pVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IBall_INTERFACE_DEFINED__ */


#ifndef __IRampEvents_DISPINTERFACE_DEFINED__
#define __IRampEvents_DISPINTERFACE_DEFINED__

/* dispinterface IRampEvents */
/* [uuid] */ 


EXTERN_C const IID DIID_IRampEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("4392878D-6B34-404e-8CE5-BD368C60133A")
    IRampEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct IRampEventsVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRampEvents * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRampEvents * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRampEvents * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IRampEvents * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IRampEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IRampEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IRampEvents * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        END_INTERFACE
    } IRampEventsVtbl;

    interface IRampEvents
    {
        CONST_VTBL struct IRampEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRampEvents_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IRampEvents_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IRampEvents_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IRampEvents_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IRampEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IRampEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IRampEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* __IRampEvents_DISPINTERFACE_DEFINED__ */


#ifndef __IFlasherEvents_DISPINTERFACE_DEFINED__
#define __IFlasherEvents_DISPINTERFACE_DEFINED__

/* dispinterface IFlasherEvents */
/* [uuid] */ 


EXTERN_C const IID DIID_IFlasherEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("AC861BD8-A2BB-4d29-9EC5-A8E9C49D80FB")
    IFlasherEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct IFlasherEventsVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFlasherEvents * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFlasherEvents * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFlasherEvents * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFlasherEvents * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFlasherEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFlasherEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFlasherEvents * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        END_INTERFACE
    } IFlasherEventsVtbl;

    interface IFlasherEvents
    {
        CONST_VTBL struct IFlasherEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFlasherEvents_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IFlasherEvents_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IFlasherEvents_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IFlasherEvents_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IFlasherEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IFlasherEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IFlasherEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* __IFlasherEvents_DISPINTERFACE_DEFINED__ */


#ifndef __IRubberEvents_DISPINTERFACE_DEFINED__
#define __IRubberEvents_DISPINTERFACE_DEFINED__

/* dispinterface IRubberEvents */
/* [uuid] */ 


EXTERN_C const IID DIID_IRubberEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("710DDCD8-919C-4B04-815B-4FA8A950B869")
    IRubberEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct IRubberEventsVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRubberEvents * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRubberEvents * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRubberEvents * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IRubberEvents * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IRubberEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IRubberEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IRubberEvents * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        END_INTERFACE
    } IRubberEventsVtbl;

    interface IRubberEvents
    {
        CONST_VTBL struct IRubberEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRubberEvents_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IRubberEvents_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IRubberEvents_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IRubberEvents_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IRubberEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IRubberEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IRubberEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* __IRubberEvents_DISPINTERFACE_DEFINED__ */


#ifndef __IDispReel_INTERFACE_DEFINED__
#define __IDispReel_INTERFACE_DEFINED__

/* interface IDispReel */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IDispReel;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("35265276-0C22-4195-8F2E-7122703814B8")
    IDispReel : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BackColor( 
            /* [retval][out] */ OLE_COLOR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_BackColor( 
            /* [in] */ OLE_COLOR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Image( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Image( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Reels( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Reels( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Width( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Width( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Height( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Height( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Spacing( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Spacing( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_IsTransparent( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_IsTransparent( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Sound( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Sound( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Steps( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Steps( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TimerEnabled( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TimerEnabled( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TimerInterval( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TimerInterval( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_X( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_X( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Y( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Y( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Range( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Range( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Name( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_UpdateInterval( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_UpdateInterval( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_UserValue( 
            /* [retval][out] */ VARIANT *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_UserValue( 
            /* [in] */ VARIANT *newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_UseImageGrid( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_UseImageGrid( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Visible( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Visible( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ImagesPerGridRow( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ImagesPerGridRow( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE AddValue( 
            /* [in] */ long Value) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ResetToZero( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SpinReel( 
            /* [in] */ long ReelNumber,
            /* [in] */ long PulseCount) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetValue( 
            /* [in] */ long Value) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IDispReelVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDispReel * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDispReel * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDispReel * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IDispReel * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IDispReel * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IDispReel * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IDispReel * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        DECLSPEC_XFGVIRT(IDispReel, get_BackColor)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BackColor )( 
            IDispReel * This,
            /* [retval][out] */ OLE_COLOR *pVal);
        
        DECLSPEC_XFGVIRT(IDispReel, put_BackColor)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_BackColor )( 
            IDispReel * This,
            /* [in] */ OLE_COLOR newVal);
        
        DECLSPEC_XFGVIRT(IDispReel, get_Image)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Image )( 
            IDispReel * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IDispReel, put_Image)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Image )( 
            IDispReel * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IDispReel, get_Reels)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Reels )( 
            IDispReel * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IDispReel, put_Reels)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Reels )( 
            IDispReel * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IDispReel, get_Width)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Width )( 
            IDispReel * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IDispReel, put_Width)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Width )( 
            IDispReel * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IDispReel, get_Height)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Height )( 
            IDispReel * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IDispReel, put_Height)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Height )( 
            IDispReel * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IDispReel, get_Spacing)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Spacing )( 
            IDispReel * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IDispReel, put_Spacing)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Spacing )( 
            IDispReel * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IDispReel, get_IsTransparent)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_IsTransparent )( 
            IDispReel * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IDispReel, put_IsTransparent)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_IsTransparent )( 
            IDispReel * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IDispReel, get_Sound)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Sound )( 
            IDispReel * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IDispReel, put_Sound)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Sound )( 
            IDispReel * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IDispReel, get_Steps)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Steps )( 
            IDispReel * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IDispReel, put_Steps)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Steps )( 
            IDispReel * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IDispReel, get_TimerEnabled)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TimerEnabled )( 
            IDispReel * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IDispReel, put_TimerEnabled)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TimerEnabled )( 
            IDispReel * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IDispReel, get_TimerInterval)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TimerInterval )( 
            IDispReel * This,
            /* [retval][out] */ long *pVal);
        
        DECLSPEC_XFGVIRT(IDispReel, put_TimerInterval)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TimerInterval )( 
            IDispReel * This,
            /* [in] */ long newVal);
        
        DECLSPEC_XFGVIRT(IDispReel, get_X)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_X )( 
            IDispReel * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IDispReel, put_X)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_X )( 
            IDispReel * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IDispReel, get_Y)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Y )( 
            IDispReel * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IDispReel, put_Y)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Y )( 
            IDispReel * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IDispReel, get_Range)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Range )( 
            IDispReel * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(IDispReel, put_Range)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Range )( 
            IDispReel * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(IDispReel, get_Name)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IDispReel * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IDispReel, put_Name)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IDispReel * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(IDispReel, get_UpdateInterval)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_UpdateInterval )( 
            IDispReel * This,
            /* [retval][out] */ long *pVal);
        
        DECLSPEC_XFGVIRT(IDispReel, put_UpdateInterval)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_UpdateInterval )( 
            IDispReel * This,
            /* [in] */ long newVal);
        
        DECLSPEC_XFGVIRT(IDispReel, get_UserValue)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_UserValue )( 
            IDispReel * This,
            /* [retval][out] */ VARIANT *pVal);
        
        DECLSPEC_XFGVIRT(IDispReel, put_UserValue)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_UserValue )( 
            IDispReel * This,
            /* [in] */ VARIANT *newVal);
        
        DECLSPEC_XFGVIRT(IDispReel, get_UseImageGrid)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_UseImageGrid )( 
            IDispReel * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IDispReel, put_UseImageGrid)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_UseImageGrid )( 
            IDispReel * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IDispReel, get_Visible)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Visible )( 
            IDispReel * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IDispReel, put_Visible)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Visible )( 
            IDispReel * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(IDispReel, get_ImagesPerGridRow)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ImagesPerGridRow )( 
            IDispReel * This,
            /* [retval][out] */ long *pVal);
        
        DECLSPEC_XFGVIRT(IDispReel, put_ImagesPerGridRow)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ImagesPerGridRow )( 
            IDispReel * This,
            /* [in] */ long newVal);
        
        DECLSPEC_XFGVIRT(IDispReel, AddValue)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *AddValue )( 
            IDispReel * This,
            /* [in] */ long Value);
        
        DECLSPEC_XFGVIRT(IDispReel, ResetToZero)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *ResetToZero )( 
            IDispReel * This);
        
        DECLSPEC_XFGVIRT(IDispReel, SpinReel)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SpinReel )( 
            IDispReel * This,
            /* [in] */ long ReelNumber,
            /* [in] */ long PulseCount);
        
        DECLSPEC_XFGVIRT(IDispReel, SetValue)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetValue )( 
            IDispReel * This,
            /* [in] */ long Value);
        
        END_INTERFACE
    } IDispReelVtbl;

    interface IDispReel
    {
        CONST_VTBL struct IDispReelVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDispReel_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IDispReel_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IDispReel_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IDispReel_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IDispReel_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IDispReel_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IDispReel_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IDispReel_get_BackColor(This,pVal)	\
    ( (This)->lpVtbl -> get_BackColor(This,pVal) ) 

#define IDispReel_put_BackColor(This,newVal)	\
    ( (This)->lpVtbl -> put_BackColor(This,newVal) ) 

#define IDispReel_get_Image(This,pVal)	\
    ( (This)->lpVtbl -> get_Image(This,pVal) ) 

#define IDispReel_put_Image(This,newVal)	\
    ( (This)->lpVtbl -> put_Image(This,newVal) ) 

#define IDispReel_get_Reels(This,pVal)	\
    ( (This)->lpVtbl -> get_Reels(This,pVal) ) 

#define IDispReel_put_Reels(This,newVal)	\
    ( (This)->lpVtbl -> put_Reels(This,newVal) ) 

#define IDispReel_get_Width(This,pVal)	\
    ( (This)->lpVtbl -> get_Width(This,pVal) ) 

#define IDispReel_put_Width(This,newVal)	\
    ( (This)->lpVtbl -> put_Width(This,newVal) ) 

#define IDispReel_get_Height(This,pVal)	\
    ( (This)->lpVtbl -> get_Height(This,pVal) ) 

#define IDispReel_put_Height(This,newVal)	\
    ( (This)->lpVtbl -> put_Height(This,newVal) ) 

#define IDispReel_get_Spacing(This,pVal)	\
    ( (This)->lpVtbl -> get_Spacing(This,pVal) ) 

#define IDispReel_put_Spacing(This,newVal)	\
    ( (This)->lpVtbl -> put_Spacing(This,newVal) ) 

#define IDispReel_get_IsTransparent(This,pVal)	\
    ( (This)->lpVtbl -> get_IsTransparent(This,pVal) ) 

#define IDispReel_put_IsTransparent(This,newVal)	\
    ( (This)->lpVtbl -> put_IsTransparent(This,newVal) ) 

#define IDispReel_get_Sound(This,pVal)	\
    ( (This)->lpVtbl -> get_Sound(This,pVal) ) 

#define IDispReel_put_Sound(This,newVal)	\
    ( (This)->lpVtbl -> put_Sound(This,newVal) ) 

#define IDispReel_get_Steps(This,pVal)	\
    ( (This)->lpVtbl -> get_Steps(This,pVal) ) 

#define IDispReel_put_Steps(This,newVal)	\
    ( (This)->lpVtbl -> put_Steps(This,newVal) ) 

#define IDispReel_get_TimerEnabled(This,pVal)	\
    ( (This)->lpVtbl -> get_TimerEnabled(This,pVal) ) 

#define IDispReel_put_TimerEnabled(This,newVal)	\
    ( (This)->lpVtbl -> put_TimerEnabled(This,newVal) ) 

#define IDispReel_get_TimerInterval(This,pVal)	\
    ( (This)->lpVtbl -> get_TimerInterval(This,pVal) ) 

#define IDispReel_put_TimerInterval(This,newVal)	\
    ( (This)->lpVtbl -> put_TimerInterval(This,newVal) ) 

#define IDispReel_get_X(This,pVal)	\
    ( (This)->lpVtbl -> get_X(This,pVal) ) 

#define IDispReel_put_X(This,newVal)	\
    ( (This)->lpVtbl -> put_X(This,newVal) ) 

#define IDispReel_get_Y(This,pVal)	\
    ( (This)->lpVtbl -> get_Y(This,pVal) ) 

#define IDispReel_put_Y(This,newVal)	\
    ( (This)->lpVtbl -> put_Y(This,newVal) ) 

#define IDispReel_get_Range(This,pVal)	\
    ( (This)->lpVtbl -> get_Range(This,pVal) ) 

#define IDispReel_put_Range(This,newVal)	\
    ( (This)->lpVtbl -> put_Range(This,newVal) ) 

#define IDispReel_get_Name(This,pVal)	\
    ( (This)->lpVtbl -> get_Name(This,pVal) ) 

#define IDispReel_put_Name(This,newVal)	\
    ( (This)->lpVtbl -> put_Name(This,newVal) ) 

#define IDispReel_get_UpdateInterval(This,pVal)	\
    ( (This)->lpVtbl -> get_UpdateInterval(This,pVal) ) 

#define IDispReel_put_UpdateInterval(This,newVal)	\
    ( (This)->lpVtbl -> put_UpdateInterval(This,newVal) ) 

#define IDispReel_get_UserValue(This,pVal)	\
    ( (This)->lpVtbl -> get_UserValue(This,pVal) ) 

#define IDispReel_put_UserValue(This,newVal)	\
    ( (This)->lpVtbl -> put_UserValue(This,newVal) ) 

#define IDispReel_get_UseImageGrid(This,pVal)	\
    ( (This)->lpVtbl -> get_UseImageGrid(This,pVal) ) 

#define IDispReel_put_UseImageGrid(This,newVal)	\
    ( (This)->lpVtbl -> put_UseImageGrid(This,newVal) ) 

#define IDispReel_get_Visible(This,pVal)	\
    ( (This)->lpVtbl -> get_Visible(This,pVal) ) 

#define IDispReel_put_Visible(This,newVal)	\
    ( (This)->lpVtbl -> put_Visible(This,newVal) ) 

#define IDispReel_get_ImagesPerGridRow(This,pVal)	\
    ( (This)->lpVtbl -> get_ImagesPerGridRow(This,pVal) ) 

#define IDispReel_put_ImagesPerGridRow(This,newVal)	\
    ( (This)->lpVtbl -> put_ImagesPerGridRow(This,newVal) ) 

#define IDispReel_AddValue(This,Value)	\
    ( (This)->lpVtbl -> AddValue(This,Value) ) 

#define IDispReel_ResetToZero(This)	\
    ( (This)->lpVtbl -> ResetToZero(This) ) 

#define IDispReel_SpinReel(This,ReelNumber,PulseCount)	\
    ( (This)->lpVtbl -> SpinReel(This,ReelNumber,PulseCount) ) 

#define IDispReel_SetValue(This,Value)	\
    ( (This)->lpVtbl -> SetValue(This,Value) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IDispReel_INTERFACE_DEFINED__ */


#ifndef __IDispReelEvents_DISPINTERFACE_DEFINED__
#define __IDispReelEvents_DISPINTERFACE_DEFINED__

/* dispinterface IDispReelEvents */
/* [uuid] */ 


EXTERN_C const IID DIID_IDispReelEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("416D3717-1F94-4dae-86FF-67EA11128597")
    IDispReelEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct IDispReelEventsVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDispReelEvents * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDispReelEvents * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDispReelEvents * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IDispReelEvents * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IDispReelEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IDispReelEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IDispReelEvents * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        END_INTERFACE
    } IDispReelEventsVtbl;

    interface IDispReelEvents
    {
        CONST_VTBL struct IDispReelEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDispReelEvents_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IDispReelEvents_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IDispReelEvents_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IDispReelEvents_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IDispReelEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IDispReelEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IDispReelEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* __IDispReelEvents_DISPINTERFACE_DEFINED__ */


#ifndef __ILightSeq_INTERFACE_DEFINED__
#define __ILightSeq_INTERFACE_DEFINED__

/* interface ILightSeq */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_ILightSeq;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4F9B83A5-F165-4411-83FD-48EEEAAD43A4")
    ILightSeq : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Name( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Collection( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Collection( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CenterX( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_CenterX( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CenterY( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_CenterY( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_UpdateInterval( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_UpdateInterval( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TimerEnabled( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TimerEnabled( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TimerInterval( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TimerInterval( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_UserValue( 
            /* [retval][out] */ VARIANT *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_UserValue( 
            /* [in] */ VARIANT *newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Play( 
            /* [in] */ SequencerState Animation,
            /* [defaultvalue] */ long TailLength = 0,
            /* [defaultvalue] */ long Repeat = 1,
            /* [defaultvalue] */ long Pause = 0) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE StopPlay( void) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct ILightSeqVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ILightSeq * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ILightSeq * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ILightSeq * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ILightSeq * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ILightSeq * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ILightSeq * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ILightSeq * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        DECLSPEC_XFGVIRT(ILightSeq, get_Name)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ILightSeq * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(ILightSeq, put_Name)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            ILightSeq * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(ILightSeq, get_Collection)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Collection )( 
            ILightSeq * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(ILightSeq, put_Collection)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Collection )( 
            ILightSeq * This,
            /* [in] */ BSTR newVal);
        
        DECLSPEC_XFGVIRT(ILightSeq, get_CenterX)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CenterX )( 
            ILightSeq * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ILightSeq, put_CenterX)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_CenterX )( 
            ILightSeq * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ILightSeq, get_CenterY)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CenterY )( 
            ILightSeq * This,
            /* [retval][out] */ float *pVal);
        
        DECLSPEC_XFGVIRT(ILightSeq, put_CenterY)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_CenterY )( 
            ILightSeq * This,
            /* [in] */ float newVal);
        
        DECLSPEC_XFGVIRT(ILightSeq, get_UpdateInterval)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_UpdateInterval )( 
            ILightSeq * This,
            /* [retval][out] */ long *pVal);
        
        DECLSPEC_XFGVIRT(ILightSeq, put_UpdateInterval)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_UpdateInterval )( 
            ILightSeq * This,
            /* [in] */ long newVal);
        
        DECLSPEC_XFGVIRT(ILightSeq, get_TimerEnabled)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TimerEnabled )( 
            ILightSeq * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(ILightSeq, put_TimerEnabled)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TimerEnabled )( 
            ILightSeq * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        DECLSPEC_XFGVIRT(ILightSeq, get_TimerInterval)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TimerInterval )( 
            ILightSeq * This,
            /* [retval][out] */ long *pVal);
        
        DECLSPEC_XFGVIRT(ILightSeq, put_TimerInterval)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TimerInterval )( 
            ILightSeq * This,
            /* [in] */ long newVal);
        
        DECLSPEC_XFGVIRT(ILightSeq, get_UserValue)
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_UserValue )( 
            ILightSeq * This,
            /* [retval][out] */ VARIANT *pVal);
        
        DECLSPEC_XFGVIRT(ILightSeq, put_UserValue)
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_UserValue )( 
            ILightSeq * This,
            /* [in] */ VARIANT *newVal);
        
        DECLSPEC_XFGVIRT(ILightSeq, Play)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Play )( 
            ILightSeq * This,
            /* [in] */ SequencerState Animation,
            /* [defaultvalue] */ long TailLength,
            /* [defaultvalue] */ long Repeat,
            /* [defaultvalue] */ long Pause);
        
        DECLSPEC_XFGVIRT(ILightSeq, StopPlay)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *StopPlay )( 
            ILightSeq * This);
        
        END_INTERFACE
    } ILightSeqVtbl;

    interface ILightSeq
    {
        CONST_VTBL struct ILightSeqVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ILightSeq_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ILightSeq_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ILightSeq_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ILightSeq_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define ILightSeq_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define ILightSeq_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define ILightSeq_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define ILightSeq_get_Name(This,pVal)	\
    ( (This)->lpVtbl -> get_Name(This,pVal) ) 

#define ILightSeq_put_Name(This,newVal)	\
    ( (This)->lpVtbl -> put_Name(This,newVal) ) 

#define ILightSeq_get_Collection(This,pVal)	\
    ( (This)->lpVtbl -> get_Collection(This,pVal) ) 

#define ILightSeq_put_Collection(This,newVal)	\
    ( (This)->lpVtbl -> put_Collection(This,newVal) ) 

#define ILightSeq_get_CenterX(This,pVal)	\
    ( (This)->lpVtbl -> get_CenterX(This,pVal) ) 

#define ILightSeq_put_CenterX(This,newVal)	\
    ( (This)->lpVtbl -> put_CenterX(This,newVal) ) 

#define ILightSeq_get_CenterY(This,pVal)	\
    ( (This)->lpVtbl -> get_CenterY(This,pVal) ) 

#define ILightSeq_put_CenterY(This,newVal)	\
    ( (This)->lpVtbl -> put_CenterY(This,newVal) ) 

#define ILightSeq_get_UpdateInterval(This,pVal)	\
    ( (This)->lpVtbl -> get_UpdateInterval(This,pVal) ) 

#define ILightSeq_put_UpdateInterval(This,newVal)	\
    ( (This)->lpVtbl -> put_UpdateInterval(This,newVal) ) 

#define ILightSeq_get_TimerEnabled(This,pVal)	\
    ( (This)->lpVtbl -> get_TimerEnabled(This,pVal) ) 

#define ILightSeq_put_TimerEnabled(This,newVal)	\
    ( (This)->lpVtbl -> put_TimerEnabled(This,newVal) ) 

#define ILightSeq_get_TimerInterval(This,pVal)	\
    ( (This)->lpVtbl -> get_TimerInterval(This,pVal) ) 

#define ILightSeq_put_TimerInterval(This,newVal)	\
    ( (This)->lpVtbl -> put_TimerInterval(This,newVal) ) 

#define ILightSeq_get_UserValue(This,pVal)	\
    ( (This)->lpVtbl -> get_UserValue(This,pVal) ) 

#define ILightSeq_put_UserValue(This,newVal)	\
    ( (This)->lpVtbl -> put_UserValue(This,newVal) ) 

#define ILightSeq_Play(This,Animation,TailLength,Repeat,Pause)	\
    ( (This)->lpVtbl -> Play(This,Animation,TailLength,Repeat,Pause) ) 

#define ILightSeq_StopPlay(This)	\
    ( (This)->lpVtbl -> StopPlay(This) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ILightSeq_INTERFACE_DEFINED__ */


#ifndef __ILightSeqEvents_DISPINTERFACE_DEFINED__
#define __ILightSeqEvents_DISPINTERFACE_DEFINED__

/* dispinterface ILightSeqEvents */
/* [uuid] */ 


EXTERN_C const IID DIID_ILightSeqEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("FE6B6615-41B1-4e34-A37F-447AE1B70853")
    ILightSeqEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct ILightSeqEventsVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ILightSeqEvents * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ILightSeqEvents * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ILightSeqEvents * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ILightSeqEvents * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ILightSeqEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ILightSeqEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ILightSeqEvents * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        END_INTERFACE
    } ILightSeqEventsVtbl;

    interface ILightSeqEvents
    {
        CONST_VTBL struct ILightSeqEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ILightSeqEvents_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ILightSeqEvents_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ILightSeqEvents_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ILightSeqEvents_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define ILightSeqEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define ILightSeqEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define ILightSeqEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* __ILightSeqEvents_DISPINTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_Table;

#ifdef __cplusplus

class DECLSPEC_UUID("8A942B56-F305-468d-97B3-F100921487C5")
Table;
#endif

EXTERN_C const CLSID CLSID_Wall;

#ifdef __cplusplus

class DECLSPEC_UUID("CF3B5615-2271-4FE4-A1BF-E77D81B88C5D")
Wall;
#endif

EXTERN_C const CLSID CLSID_DragPoint;

#ifdef __cplusplus

class DECLSPEC_UUID("83296774-CAA2-4A0D-B330-98DA6495DFE0")
DragPoint;
#endif

EXTERN_C const CLSID CLSID_Flipper;

#ifdef __cplusplus

class DECLSPEC_UUID("68AB2BBC-8209-40F3-B6F4-54F8ADAA0DC7")
Flipper;
#endif

EXTERN_C const CLSID CLSID_Timer;

#ifdef __cplusplus

class DECLSPEC_UUID("A0B914E6-56A8-4CC1-A846-45FFF4D8CA17")
Timer;
#endif

EXTERN_C const CLSID CLSID_Plunger;

#ifdef __cplusplus

class DECLSPEC_UUID("4C43960F-E04C-4A51-9FBD-83AE50EECC7B")
Plunger;
#endif

EXTERN_C const CLSID CLSID_Textbox;

#ifdef __cplusplus

class DECLSPEC_UUID("CB6071D5-E0E5-4CF9-B114-BB84EA701E9C")
Textbox;
#endif

EXTERN_C const CLSID CLSID_Bumper;

#ifdef __cplusplus

class DECLSPEC_UUID("C242DDFF-0D79-4C56-967F-765A30DE8ED6")
Bumper;
#endif

EXTERN_C const CLSID CLSID_Trigger;

#ifdef __cplusplus

class DECLSPEC_UUID("44D58C78-14BA-47F3-B82C-425853ABB698")
Trigger;
#endif

EXTERN_C const CLSID CLSID_Light;

#ifdef __cplusplus

class DECLSPEC_UUID("31DD37E7-DB9B-4AB1-94C9-FAA06B252DFA")
Light;
#endif

EXTERN_C const CLSID CLSID_Kicker;

#ifdef __cplusplus

class DECLSPEC_UUID("498EB992-C01F-4CD4-B710-E4E1D268C994")
Kicker;
#endif

EXTERN_C const CLSID CLSID_Gate;

#ifdef __cplusplus

class DECLSPEC_UUID("5692B7A1-B753-4F59-A9F9-8EE85E1A1C3D")
Gate;
#endif

EXTERN_C const CLSID CLSID_Spinner;

#ifdef __cplusplus

class DECLSPEC_UUID("355FA5FF-DB59-4D94-99E3-16D3B993D6B5")
Spinner;
#endif

EXTERN_C const CLSID CLSID_Ramp;

#ifdef __cplusplus

class DECLSPEC_UUID("374F02B0-57A4-40BC-8730-D351C858B9D3")
Ramp;
#endif

EXTERN_C const CLSID CLSID_Flasher;

#ifdef __cplusplus

class DECLSPEC_UUID("847B5F6C-65DC-4b3a-A721-B8F931C23133")
Flasher;
#endif

EXTERN_C const CLSID CLSID_Rubber;

#ifdef __cplusplus

class DECLSPEC_UUID("B0715DC0-002F-11E4-9191-0800200C9A66")
Rubber;
#endif

EXTERN_C const CLSID CLSID_Ball;

#ifdef __cplusplus

class DECLSPEC_UUID("D4966068-DEDB-4EB9-9AB8-4574CCAA5F1D")
Ball;
#endif

EXTERN_C const CLSID CLSID_Collection;

#ifdef __cplusplus

class DECLSPEC_UUID("D4D4E043-594F-49f0-B561-4AA9942DCF18")
Collection;
#endif

EXTERN_C const CLSID CLSID_DispReel;

#ifdef __cplusplus

class DECLSPEC_UUID("47B68E91-098C-44EE-B2B1-04C87654CF5E")
DispReel;
#endif

EXTERN_C const CLSID CLSID_LightSeq;

#ifdef __cplusplus

class DECLSPEC_UUID("F4758E77-AD9A-49A6-8146-AB94540F0102")
LightSeq;
#endif

EXTERN_C const CLSID CLSID_Primitive;

#ifdef __cplusplus

class DECLSPEC_UUID("FFA76BF2-B558-4EF6-AA81-5FDC5F71CFE8")
Primitive;
#endif

EXTERN_C const CLSID CLSID_HitTarget;

#ifdef __cplusplus

class DECLSPEC_UUID("7770B875-0F54-4187-81E3-389742D97BAF")
HitTarget;
#endif
#endif /* __VPinballLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


