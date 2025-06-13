// license:GPLv3+

#pragma once

#include "MsgPlugin.h"
#include "ControllerPlugin.h"

///////////////////////////////////////////////////////////////////////////////
// VPX plugins
//
// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
// This interface is part of a work in progress and will evolve likely a lot
// before being considered stable. Do not use it, or if you do, use it knowing
// that you're plugin will be broken by the upcoming updates.
// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
//
// Plugins are a simple way to extend VPX core features. VPinballX application
// will scan for plugins in its 'plugin' folder, searching for subfolders
// containing a 'plugin.cfg' file. When found, the file will be read to find
// the needed metadata to present to the user as well as path for the native
// builds of the plugins for each supported platform. Then, the plugin will be
// available for the end user to enable it from the application settings.
// Another option is to statically build plugins into the application.
//
// This API takes for granted that at any time, only one game can be played. 
// All calls related to a running game may only be called between the 'OnGameStart' 
// and 'OnGameEnd' events.


#define VPXPI_NAMESPACE "VPX" // Namespace used for all VPX message definition

// Core VPX messages
#define VPXPI_MSG_GET_API               "GetAPI"              // Get the main VPX plugin API

// Core VPX events
#define VPXPI_EVT_ON_GAME_START         "OnGameStart"         // Broadcasted during player creation, before script initialization
#define VPXPI_EVT_ON_GAME_END           "OnGameEnd"           // Broadcasted during player shutdown
#define VPXPI_EVT_ON_PREPARE_FRAME      "OnPrepareFrame"      // Broadcasted when player starts preparing a new frame
#define VPXPI_EVT_ON_UPDATE_PHYSICS     "OnUpdatePhysics"     // Broadcasted when player update physics (happens often, so must be used with care)
#define VPXPI_EVT_ON_SETTINGS_CHANGED   "OnSettingsChanged"   // Broadcasted when settings have been changed
#define VPXPI_EVT_ON_ACTION_CHANGED     "OnActionChanged"     // Broadcasted when an action state change, event data is an VPXActionEvent whose isPressed field can be modified by plugins

// Anciliary window rendering
#define VPXPI_MSG_GET_AUX_RENDERER      "GetAuxRenderer"      // Broadcasted with a GetAnciliaryRendererMsg to discover anciliary window renderer implemented in plugins
#define VPXPI_EVT_AUX_RENDERER_CHG      "AuxRendererChanged"  // Broadcasted when an anciliray renderer is added or removed

typedef void* VPXTexture;

typedef enum
{
   VPXTEXFMT_BW,
   VPXTEXFMT_sRGB8,
   VPXTEXFMT_sRGBA8,
   VPXTEXFMT_sRGB565,
} VPXTextureFormat;

typedef enum
{
   VPXWINDOW_Backglass,
   VPXWINDOW_ScoreView,
   VPXWINDOW_Topper,
} VPXAnciliaryWindow;

typedef enum
{
   VPXDMDStyle_Legacy,
   VPXDMDStyle_Plasma,
   VPXDMDStyle_RedLED,
   VPXDMDStyle_GreenLED,
   VPXDMDStyle_YellowLED,
   VPXDMDStyle_GenPlasma,
   VPXDMDStyle_GenLED,
} VPXDisplayRenderStyle;

typedef enum
{
   VPXSegStyle_Plasma,
   VPXSegStyle_BlueVFD,
   VPXSegStyle_GreenVFD,
   VPXSegStyle_RedLED,
   VPXSegStyle_GreenLED,
   VPXSegStyle_YellowLED,
   VPXSegStyle_GenPlasma,
   VPXSegStyle_GenLED,
} VPXSegDisplayRenderStyle;

typedef enum
{
   Generic,
   Gottlieb,
   Williams,
   Bally,
   Atari,
} VPXSegDisplayHint;

typedef struct VPXRenderContext2D
{
   VPXAnciliaryWindow window; // Target window
   float srcWidth;            // Source surface width, used in DrawImage call, default to target surface width
   float srcHeight;           // Source surface height, used in DrawImage call, default to target surface height
   float outWidth;            // Target surface width, mostly to be used for apsect ratio computation, LOD and layout
   float outHeight;           // Target surface height, mostly to be used for apsect ratio computation, LOD and layout
   void(MSGPIAPI* DrawImage)(VPXRenderContext2D* ctx, VPXTexture texture,
      const float tintR, const float tintG, const float tintB, const float alpha, // tint color and alpha (0..1)
      const float texX, const float texY, const float texW, const float texH,  // coordinates in texture surface (0..tex.width, 0..tex.height)
      const float srcX, const float srcY, const float srcW, const float srcH); // coordinates in source surface (0..srcWidth, 0..srcHeight)
   void(MSGPIAPI* DrawDisplay)(VPXRenderContext2D* ctx, VPXDisplayRenderStyle style,
      // First layer is an optional tinted glass
      VPXTexture glassTex, const float glassTintR, const float glassTintG, const float glassTintB, const float glassRoughness,
      const float glassAreaX, const float glassAreaY, const float glassAreaW, const float glassAreaH,
      const float glassAmbientR, const float glassAmbientG, const float glassAmbientB,
      // Second layer is the emitter
      VPXTexture dispTex, const float dispTintR, const float dispTintG, const float dispTintB, const float brightness, const float alpha, // Emitter texture, tint, brightness and alpha
      const float dispPadL, const float dispPadT, const float dispPadR, const float dispPadB, // Display padding from glass bounds
      //
      const float srcX, const float srcY, const float srcW, const float srcH); // coordinates in source surface (0..srcWidth, 0..srcHeight)
   void(MSGPIAPI* DrawSegDisplay)(VPXRenderContext2D* ctx, VPXSegDisplayRenderStyle style, VPXSegDisplayHint shapeHint,
      // First layer is an optional tinted glass
      VPXTexture glassTex, const float glassTintR, const float glassTintG, const float glassTintB, const float glassRoughness, const float glassAreaX, const float glassAreaY,
      const float glassAreaW, const float glassAreaH, const float glassAmbientR, const float glassAmbientG, const float glassAmbientB,
      // Second layer is the emitter
      SegElementType type, const float* state, const float dispTintR, const float dispTintG, const float dispTintB, const float brightness, const float alpha, // Emitter, tint, brightness and alpha
      const float dispPadL, const float dispPadT, const float dispPadR, const float dispPadB, // Display padding from glass bounds
      //
      const float srcX, const float srcY, const float srcW, const float srcH); // coordinates in source surface (0..srcWidth, 0..srcHeight)
} VPXRenderContext2D;

typedef struct AnciliaryRendererDef
{
   const char* id;            // Unique id of the renderer (only alphanumeric characters, dots and underscore. Can be used for storing settings)
   const char* name;          // Human readable name of the renderer
   const char* description;   // Human readable description of the renderer
   void* context;             // Custom context to be passed when requesting rendering
   int(MSGPIAPI* Render)(VPXRenderContext2D* renderCtx, void* context);
} AnciliaryRendererDef;

typedef struct GetAnciliaryRendererMsg
{
   // Request
   VPXAnciliaryWindow window; // Target window
   unsigned int maxEntryCount; // see below
   // Response
   unsigned int count; // Number of entries, also position to put next entry
   AnciliaryRendererDef* entries; // Pointer to an array of maxEntryCount entries to be filled
} GetAnciliaryRendererMsg;


// Core VPX settings pages
// GetOption 'pageId' parameter is either the id of a loaded plugin or the id of one of the core VPX pages defined below
#define VPX_TWEAK_VIEW           "VPX.tweak.view"
#define VPX_TWEAK_TABLE          "VPX.tweak.table"
// GetOption bitmask flags defining where the setting should be displayed
#define VPX_OPT_SHOW_UI          1
#define VPX_OPT_SHOW_TWEAK       2

// Helper defines

// Conversions to/from VP units (50 VPU = 1.0625 inches which is 1"1/16, the default size of a ball, 1 inch is 2.54cm)
// These value are very slightly off from original values which used a VPU to MM of 0.540425 instead of 0.53975 (result of the following formula)
// So it used to be 0.125% larger which is not noticeable but makes it difficult to have perfect matches when playing between apps
#ifndef MMTOVPU
#define MMTOVPU(x) ((x) * (float)(50. / (25.4 * 1.0625)))
#define CMTOVPU(x) ((x) * (float)(50. / (2.54 * 1.0625)))
#define VPUTOMM(x) ((x) * (float)(25.4 * 1.0625 / 50.))
#define VPUTOCM(x) ((x) * (float)(2.54 * 1.0625 / 50.))
#define INCHESTOVPU(x) ((x) * (float)(50. / 1.0625))
#define VPUTOINCHES(x) ((x) * (float)(1.0625 / 50.))
#endif

typedef struct VPXInfo
{
   const char* path;              // [R_]
   const char* prefPath;          // [R_]
} VPXInfo;

typedef struct VPXTableInfo
{
   const char* path;              // [R_]
   float tableWidth, tableHeight; // [R_]
} VPXTableInfo;

typedef struct VPXViewSetupDef
{
   // See ViewSetup class for member description
   int viewMode;                                       // [R_]
   float sceneScaleX, sceneScaleY, sceneScaleZ;        // [R_]
   float viewX, viewY, viewZ;                          // [RW]
   float lookAt;                                       // [R_]
   float viewportRotation;                             // [R_]
   float FOV;                                          // [R_]
   float layback;                                      // [R_]
   float viewHOfs, viewVOfs;                           // [R_]
   float windowTopZOfs, windowBottomZOfs;              // [R_]
   // Following fields are defined in user settings
   float screenWidth, screenHeight, screenInclination; // [R_]
   float realToVirtualScale;                           // [R_]
   float interpupillaryDistance;                       // [R_] TODO upgrade to RW to allow head tracking to measure and adjust accordingly
} VPXViewSetupDef;

typedef enum
{
   VPXACTION_LeftFlipperKey,
   VPXACTION_RightFlipperKey,
   VPXACTION_StagedLeftFlipperKey,
   VPXACTION_StagedRightFlipperKey,
   VPXACTION_LeftTiltKey,
   VPXACTION_RightTiltKey,
   VPXACTION_CenterTiltKey,
   VPXACTION_PlungerKey,
   VPXACTION_FrameCount,
   VPXACTION_DBGBalls,
   VPXACTION_Debugger,
   VPXACTION_AddCreditKey,
   VPXACTION_AddCreditKey2,
   VPXACTION_StartGameKey,
   VPXACTION_MechanicalTilt,
   VPXACTION_RightMagnaSave,
   VPXACTION_LeftMagnaSave,
   VPXACTION_ExitGame,
   VPXACTION_VolumeUp,
   VPXACTION_VolumeDown,
   VPXACTION_LockbarKey,
   VPXACTION_Enable3D,
   VPXACTION_TableRecenter,
   VPXACTION_TableUp,
   VPXACTION_TableDown,
   VPXACTION_Escape,
   VPXACTION_Pause,
   VPXACTION_Tweak,
} VPXAction;

typedef struct VPXActionEvent
{
   VPXAction action;
   int isPressed;
} VPXActionEvent;

typedef struct VPXPluginAPI
{
   // General information API
   void (MSGPIAPI *GetVpxInfo)(VPXInfo* info);
   void (MSGPIAPI *GetTableInfo)(VPXTableInfo* info);

   // User Interface
   enum OptionUnit { NONE, PERCENT };
   float (MSGPIAPI *GetOption)(const char* pageId, const char* optionId, const unsigned int showMask, const char* optionName, const float minValue, const float maxValue, const float step, const float defaultValue, const enum OptionUnit unit, const char** values);
   unsigned int (MSGPIAPI *PushNotification)(const char* msg, const int lengthMs);
   void (MSGPIAPI *UpdateNotification)(const unsigned int handle, const char* msg, const int lengthMs);

   // View management
   void (MSGPIAPI *DisableStaticPrerendering)(const int /* bool */ disable);
   void (MSGPIAPI *GetActiveViewSetup)(VPXViewSetupDef* view);
   void (MSGPIAPI *SetActiveViewSetup)(VPXViewSetupDef* view);

   // Input management
   void(MSGPIAPI* GetInputState)(uint64_t* keyState, float* nudgeX, float* nudgeY, float* plunger);
   void(MSGPIAPI* SetInputState)(const uint64_t keyState, const float nudgeX, const float nudgeY, const float plunger);

   // Rendering
   
   // Create a texture from encoded data (Webp, Exr, ...).
   // Texture must be destroyed by the caller using DeleteTexture.
   // Thread safe
   VPXTexture(MSGPIAPI* CreateTexture)(uint8_t* rawData, int size);
   // Create or update a texture from 'flat' data in the specified format
   // - if texture object already exists, it is either updated or destroyed/recreated. texture pointer is 
   //   updated with the new/updated texture object, this object should be provided for later updates
   //   to allow reuse of texture objects.
   // Texture must be destroyed by the caller using DeleteTexture.
   // NOT Thread safe
   void(MSGPIAPI* UpdateTexture)(VPXTexture* texture, int width, int height, VPXTextureFormat format, const uint8_t* image);
   // Give access to texture informations.
   // NOT Thread safe
   void(MSGPIAPI* GetTextureInfo)(VPXTexture texture, int* width, int* height);
   // Destroy a texture created through this API.
   // Thread safe
   void(MSGPIAPI* DeleteTexture)(VPXTexture texture);

} VPXPluginAPI;
