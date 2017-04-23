#pragma once
// Table

#define DISPID_Table_Inclination 104
#define DISPID_Table_FieldOfView 105
#define DISPID_Table_Layback 1040
#define DISPID_Table_MaxSeparation 1041
#define DISPID_Table_ZPD 1042
#define DISPID_Table_OverridePhysics 1043
#define DISPID_Table_Offset 1045
#define DISPID_Table_Nudge 21
#define DISPID_Table_Height 13
#define DISPID_Table_Width 14

#define DISPID_Surface_SideColor 6
#define DISPID_Surface_FaceColor 10

#define DISPID_Flipper_Speed 16
#define DISPID_Flipper_OverridePhysics 1044

#define DISPID_Light_State 2
#define DISPID_Light_BlinkInterval 10

#define DISPID_Textbox_BackColor 1
#define DISPID_Textbox_FontColor 2
#define DISPID_Textbox_Font 4

#define DISPID_DispReel_BackColor 1

#define DISPID_Gate_Length 3
#define DISPID_Gate_Rotation 4

#define DISPID_Spinner_Length 3

#define DISPID_Decal_SizingType 9
#define DISPID_Decal_Font 12

#define DISPID_Timer_Enabled 900
#define DISPID_Timer_Interval 901
#define DISPID_X 902
#define DISPID_Y 903
#define DISPID_Enabled 904

/////////////////////
/// Events
///////////////////

// Table
#define DISPID_GameEvents_KeyDown 1000
#define DISPID_GameEvents_KeyUp 1001
#define DISPID_GameEvents_Init 1002
#define DISPID_GameEvents_MusicDone 1003
//>>> added by chris
#define DISPID_GameEvents_Exit 1004
#define DISPID_GameEvents_Paused 1005
#define DISPID_GameEvents_UnPaused 1006
//<<<

// Surface
#define DISPID_SurfaceEvents_Slingshot 1101

// Flipper
#define DISPID_FlipperEvents_Collide 1200

// Timer
#define DISPID_TimerEvents_Timer 1300

// Spinner
#define DISPID_SpinnerEvents_Spin 1301

// HitTarget
#define DISPID_TargetEvents_Dropped 1302 
#define DISPID_TargetEvents_Raised  1303 

// Light Sequencer
#define DISPID_LightSeqEvents_PlayDone 1320

// Plunger
#define DISPID_PluFrames 464
#define DISPID_Width 465
#define DISPID_ZAdjust 466

#define DISPID_RodDiam                  467
#define DISPID_RingDiam                 468
#define DISPID_RingThickness            469
#define DISPID_SpringDiam               470
#define DISPID_TipShape                 471
#define DISPID_SpringGauge              472
#define DISPID_SpringLoops              473
#define DISPID_RingGap                  474
#define DISPID_SpringEndLoops           475

// Generic
#define DISPID_HitEvents_Hit 1400
#define DISPID_HitEvents_Unhit 1401
#define DISPID_LimitEvents_EOS 1402
#define DISPID_LimitEvents_BOS 1403

// PerPropertyDispids
#define DISPID_Image 1500
#define DISPID_Image2 1501
#define DISPID_Surface 1502
#define DISPID_Shape 1503
#define DISPID_UserValue 1504
#define DISPID_Image3 1505
#define DISPID_Sound 1506
#define DISPID_Collection 1507
#define DISPID_Image4 1508
#define DISPID_Image5 1509
#define DISPID_Image6 1510
#define DISPID_Image7 1511
#define DISPID_Image8 1512

// Primitive DispIDs
#define DISPID_SIDES                    378
#define DISPID_POSITION_X               7
#define DISPID_POSITION_Y               8
#define DISPID_POSITION_Z               4
#define DISPID_SIZE_X                   5
#define DISPID_SIZE_Y                   6
#define DISPID_SIZE_Z                   3
#define DISPID_LOADED_MESH_EDIT         1601
#define DISPID_LOAD_MESH_BUTTON         1602
#define DISPID_DELETE_MESH_BUTTON       1603

#define DISPID_ROTRA1                                   12
#define DISPID_ROTRA2                                   13
#define DISPID_ROTRA3                                   14
#define DISPID_ROTRA4                                   15
#define DISPID_ROTRA5                                   16
#define DISPID_ROTRA6                                   17
#define DISPID_ROTRA7                                   21
#define DISPID_ROTRA8                                   22
#define DISPID_ROTRA9                                   23
#define DISPID_ROT_X                                    24
#define DISPID_ROT_Y                                    25
#define DISPID_ROT_Z                                    26
#define DISPID_TRANS_X                                  27
#define DISPID_TRANS_Y                                  28
#define DISPID_TRANS_Z                                  29
#define DISPID_OBJROT_X                                 30
#define DISPID_OBJROT_Y                                 31
#define DISPID_OBJROT_Z                                 32

// Must have a carriage return at end of file for correct resource file parsing
