// license:GPLv3+

#pragma once

// The resource ids which the core and table parts need: COM registration ids, localized strings, and a few
// control ids used as dispids by the property browser. The Win32 editor's own ~900 dialog, menu and control ids
// stay in ui/win/resource.h, which includes this one so that the .rc files and the editor still see everything.
// A build without that editor has no use for those, and this also keeps a dialog edit from rebuilding all of core.
// Both files draw from a single id space, and the '_APS_NEXT_*' counters which the Visual Studio resource editor
// maintains at the end of ui/win/resource.h do not account for this file: ids added here must not collide with it

#define IDC_EFFECT_COMBO                32996
#define IDC_MATERIAL_COMBO              340
#define IDC_MATERIAL_COMBO2             341
#define IDC_MATERIAL_COMBO3             426
#define IDC_MATERIAL_COMBO4             734
#define IDI_TABLE                       266
#define IDR_BALL                        167
#define IDR_BUMPER                      122
#define IDR_DISP_REEL                   277
#define IDR_FLIPPER                     108
#define IDR_Flasher                     393
#define IDR_GATE                        139
#define IDR_HITTARGET                   280
#define IDR_KICKER                      133
#define IDR_LIGHT                       130
#define IDR_LIGHT_SEQ                   181
#define IDR_PARTGROUP                   960
#define IDR_PLUNGER                     116
#define IDR_PRIMITIVE                   278
#define IDR_RAMP                        145
#define IDR_RUBBER                      279
#define IDR_SPINNER                     142
#define IDR_SURFACE                     101
#define IDR_TEXTBOX                     119
#define IDR_TIMER                       113
#define IDR_TRIGGER                     127
#define IDR_VPINBALL                    100
#define IDS_CONTROLPOINT                169
#define IDS_DEBUGNOCONVERT              341
#define IDS_HANG                        346
#define IDS_NONE                        338
#define IDS_SAVEERROR                   161
#define IDS_SCRIPT                      156
#define IDS_TABLE                       155
#define IDS_TB_BACKGLASS                162
#define IDS_TB_BALL                     676
#define IDS_TB_BUMPER                   122
#define IDS_TB_DECAL                    135
#define IDS_TB_DISPREEL                 123
#define IDS_TB_FLASHER                  140
#define IDS_TB_FLIPPER                  105
#define IDS_TB_GATE                     139
#define IDS_TB_KICKER                   133
#define IDS_TB_LIGHT                    130
#define IDS_TB_LIGHTSEQ                 314
#define IDS_TB_MAGNIFY                  137
#define IDS_TB_PARTGROUP                957
#define IDS_TB_PLAY                     106
#define IDS_TB_PLUNGER                  116
#define IDS_TB_PRIMITIVE                186
#define IDS_TB_PROPERTIES               136
#define IDS_TB_RAMP                     145
#define IDS_TB_RUBBER                   147
#define IDS_TB_SELECT                   103
#define IDS_TB_SPINNER                  142
#define IDS_TB_TARGET                   134
#define IDS_TB_TEXTBOX                  119
#define IDS_TB_TIMER                    101
#define IDS_TB_TRIGGER                  127
#define IDS_TB_WALL                     104
#define IDS_UNSECURECONTROL1            172
#define IDS_UNSECURECONTROL2            173
#define ID_DEBUGWINDOW                  295
