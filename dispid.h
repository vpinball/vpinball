#pragma once
// Table

#define DISPID_Table_GridSize 109
#define DISPID_Table_Inclination 104
#define DISPID_Table_FieldOfView 105
#define DISPID_Table_Layback 1040
#define DISPID_Table_DisplayGrid 106
#define DISPID_Table_DisplayBackdrop 107
#define DISPID_Table_Nudge 21
#define DISPID_Table_Height 13
#define DISPID_Table_Width 14

#define DISPID_Surface_SideColor 6
#define DISPID_Surface_FaceColor 10

#define DISPID_Flipper_Speed 16

#define DISPID_Light_State 2
#define DISPID_Light_BlinkInterval 10

#define DISPID_Textbox_BackColor 1
#define DISPID_Textbox_FontColor 2
#define DISPID_Textbox_Font 4

#define DISPID_DispReel_BackColor 1
#define DISPID_DispReel_FontColor 2
#define DISPID_DispReel_ReelColor 3
#define DISPID_DispReel_Font 4

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
#define DISPID_SurfaceEvents_Collide 1100
#define DISPID_SurfaceEvents_Slingshot 1101

// Flipper
#define DISPID_FlipperEvents_Collide 1200

// Timer
#define DISPID_TimerEvents_Timer 1300

// Spinner
#define DISPID_SpinnerEvents_Spin 1301

// Light Sequencer
#define DISPID_LightSeqEvents_PlayDone 1320

// Generic
#define DISPID_HitEvents_Hit 1400
#define DISPID_HitEvents_Unhit 1401
#define DISPID_LimitEvents_EOS 1402
#define DISPID_LimitEvents_BOS 1403

// PerPropetyDispids
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

// Primitive DispIDs
#define DISPID_SIDES                    378
#define DISPID_POSITION_X               7
#define DISPID_POSITION_Y               8
#define DISPID_POSITION_Z               4
#define DISPID_SIZE_X                   5
#define DISPID_SIZE_Y                   6
#define DISPID_SIZE_Z                   3
#define DISPID_AXISSCALE_XY             9
#define DISPID_AXISSCALE_XZ             10
#define DISPID_AXISSCALE_YX             11
#define DISPID_AXISSCALE_YZ             18
#define DISPID_AXISSCALE_ZX             19
#define DISPID_AXISSCALE_ZY             20
/*
#define DISPID_ROTATION_X               12
#define DISPID_ROTATION_Y               13
#define DISPID_ROTATION_Z               14
#define DISPID_TRANSPOSITION_X          15
#define DISPID_TRANSPOSITION_Y          16
#define DISPID_TRANSPOSITION_Z          17
*/
#define DISPID_ROTRA1					12
#define DISPID_ROTRA2					13
#define DISPID_ROTRA3					14
#define DISPID_ROTRA4					15
#define DISPID_ROTRA5					16
#define DISPID_ROTRA6					17
#define IDC_COMBO1                      340
#define IDC_COMBO2                      341
#define IDC_COMBO3                      342
#define IDC_COMBO4                      343
#define IDC_COMBO5                      344
#define IDC_COMBO6                      345

// Must have a carriage return at end of file for correct resource file parsing




