// This file contains the resource ID definitions for Win32++.


// The resource ID for MENU, ICON, ToolBar Bitmap, Accelerator,
//  and Window Caption
#define IDW_MAIN                         41

// Resource ID for the About dialog
#define IDW_ABOUT                        42

// Resource IDs for menu items
#define IDW_VIEW_TOOLBAR                 43
#define IDW_VIEW_STATUSBAR               44

// Resource ID for frame status
#define IDW_READY                        45
#define IDW_INDICATOR_CAPS               46
#define IDW_INDICATOR_NUM                47
#define IDW_INDICATOR_SCRL               48

// Resource IDs for the ReBar Bands
#define IDW_MENUBAR                      49
#define IDW_TOOLBAR                      50

// Resource ID for the Accelerator key
#define IDW_QUIT                         51

// Resource IDs for MDI menu items
#define IDW_MDI_CASCADE                  52
#define IDW_MDI_TILE                     53
#define IDW_MDI_ARRANGE                  54
#define IDW_MDI_CLOSEALL                 55
#define IDW_FIRSTCHILD                   56
#define IDW_CHILD2                       57
#define IDW_CHILD3                       58
#define IDW_CHILD4                       59
#define IDW_CHILD5                       60
#define IDW_CHILD6                       61
#define IDW_CHILD7                       62
#define IDW_CHILD8                       63
#define IDW_CHILD9                       64
#define IDW_CHILD10                      65

#define IDW_FILE_MRU_FILE1               66
#define IDW_FILE_MRU_FILE2               67
#define IDW_FILE_MRU_FILE3               68
#define IDW_FILE_MRU_FILE4               69
#define IDW_FILE_MRU_FILE5               70
#define IDW_FILE_MRU_FILE6               71
#define IDW_FILE_MRU_FILE7               72
#define IDW_FILE_MRU_FILE8               73
#define IDW_FILE_MRU_FILE9               74
#define IDW_FILE_MRU_FILE10              75
#define IDW_FILE_MRU_FILE11              76
#define IDW_FILE_MRU_FILE12              77
#define IDW_FILE_MRU_FILE13              78
#define IDW_FILE_MRU_FILE14              79
#define IDW_FILE_MRU_FILE15              80
#define IDW_FILE_MRU_FILE16              81

// Cursor Resources
#define IDW_SPLITH                       82
#define IDW_SPLITV                       83
#define IDW_TRACK4WAY                    84

// Docking Bitmap Resources
#define IDW_SDBOTTOM                     85
#define IDW_SDCENTER                     86
#define IDW_SDLEFT                       87
#define IDW_SDMIDDLE                     88
#define IDW_SDRIGHT                      89
#define IDW_SDTOP                        90

// Print Preview Resources
#define IDW_PRINTPREVIEW                 91
#define IDW_PREVIEWPRINT                 92
#define IDW_PREVIEWSETUP                 93
#define IDW_PREVIEWPREV                  94
#define IDW_PREVIEWNEXT                  95
#define IDW_PREVIEWCLOSE                 96
#define IDW_PREVIEWPANE                  97

// A generic ID for any static control
#ifndef IDC_STATIC
  #define IDC_STATIC                     -1
#endif



// Notes about Resource IDs
// * In general, resource IDs can have values from 1 to 65535. Programs with
//   resource IDs higher than 65535 aren't supported on Windows 95
//
// * Windows uses the icon with the lowest resource ID as the application's
//   icon. The application's icon is IDW_MAIN, which is the first resource ID
//   defined by Win32++.
//
// * When more than one static control is used in a dialog, the controls should
//   have a unique ID, unless a resource ID of -1 is used.
//
// * Users of Win32++ are advised to begin their resource IDs from 120 to
//   allow for possible expansion of Win32++.


