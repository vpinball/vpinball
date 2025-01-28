#pragma once

// This file contains definitions used inside PinMame and for external integration through libpinmame


///////////////////////////////////////////////////////////////////////////////
//
// Complete machine output state, provided in a global state block
//
// 0 length array is a non standard extension used intentionally here. 
// Corresponding warning #4200 is therefore disabled when needed.
//

// Digital output instantaneous states

#pragma warning(disable : 4200) 
typedef struct
{
   double       updateTimestamp;
   unsigned int nOutputs;
   UINT32       outputBitset[];  // Bitset array of nOutputs bits with their current binary state
} pinmame_tBinaryStates;


// Controlled device states

typedef enum {
   PINMAME_DEVICE_STATE_TYPE_CUSTOM,   // Custom state defined by each driver (value maybe either binary of 0/1 or 0/255, or modulated between 0..255)
   PINMAME_DEVICE_STATE_TYPE_BULB,     // Bulb state defined by its relative luminance and average filament temperature
   PINMAME_DEVICE_STATE_TYPE_LED,      // LED state defined by its relative luminance
   PINMAME_DEVICE_STATE_TYPE_SOLENOID, // Solenoid (not yet defined)
   PINMAME_DEVICE_STATE_TYPE_STEPPER,  // Stepper motor (not yet defined)
} pinmame_tDeviceCategory;

typedef enum {
   PINMAME_DEVICE_TYPE_UNDEFINED,      // Hardware type identifier
   PINMAME_DEVICE_TYPE_VFD_BLUE,       // VFD used for segment display
   PINMAME_DEVICE_TYPE_VFD_GREEN,      // VFD used for segment display
   PINMAME_DEVICE_TYPE_NEON_PLASMA,    // Neon Plasma display (Panaplex, Burroughs,...) used for segment and dot matrix display
   PINMAME_DEVICE_TYPE_LED_RED,        // 
} pinmame_tDeviceType;

typedef struct
{
   pinmame_tDeviceCategory category;
   pinmame_tDeviceType     type;
   unsigned int            mapping;     // Device mapping to a user understandable id (for example lamp matrix are usually numbered by row/column and not index)
   union
   {
      // PINMAME_DEVICE_STATE_TYPE_CUSTOM
      UINT8 customState;            // Custom value, depending on each driver definition
      
      // PINMAME_DEVICE_STATE_TYPE_BULB
      struct
      {
         float luminance;           // relative luminance to bulb rating (equals 1.f when bulb is under its rating voltage after heating stabilization)
         float filamentTemperature; // perceived filament temperature (equals to bulb filament rating when bulb is at its rating voltage after heating stabilization)
      } bulb;
      
      // PINMAME_DEVICE_STATE_TYPE_LED
      float ledLuminance;           // relative luminance to bulb design (equals 1.f when LED is pulsed at its designed PWM)

      // PINMAME_DEVICE_STATE_TYPE_SOLENOID
      // To be added in a later revision

      // PINMAME_DEVICE_STATE_TYPE_STEPPER
      // To be added in a later revision
   };
} pinmame_tDeviceState;

#pragma warning(disable : 4200)
typedef struct
{
   double               updateTimestamp;
   unsigned int         nDevices;
   unsigned int         stateByteSize;   // sizeof(pinmame_tDeviceState), to be used to access states since the size of a single state my avolve in future revisions
   pinmame_tDeviceState states[];        // array of nDevices * stateByteSize with the current device states
} pinmame_tDeviceStates;


// Alphanumeric segment displays

// Individual segment layouts inside a display (usually made of multiple elements)
typedef enum {
   PINMAME_SEG_LAYOUT_7,          //  7 segments
   PINMAME_SEG_LAYOUT_7C,         //  7 segments with comma
   PINMAME_SEG_LAYOUT_7D,         //  7 segments with dot
   PINMAME_SEG_LAYOUT_9,          //  9 segments
   PINMAME_SEG_LAYOUT_9C,         //  9 segments with comma
   PINMAME_SEG_LAYOUT_14,         // 14 segments
   PINMAME_SEG_LAYOUT_14D,        // 14 segments with dot
   PINMAME_SEG_LAYOUT_14DC,       // 14 segments with dot and comma
   PINMAME_SEG_LAYOUT_16,         // 16 segments
} pinmame_tSegElementType;

typedef struct
{
   pinmame_tSegElementType type;               // see PINMAME_SEG_LAYOUT_xxx
   float                   luminance[16];      // relative luminance of each segment (from 7 to 16)
} pinmame_tAlphaSegmentState;

typedef struct
{
   unsigned int            nElements;         // Number of elements (each element is composed of 7 to 16 segments)
   pinmame_tDeviceType     type;              // Hardware reference (Panaplex, VFD, ...)
} pinmame_tAlphaDisplayDef;

#pragma warning(disable : 4200)
typedef struct
{
   double                   updateTimestamp;
   unsigned int             nDisplays;        // Number of displays (each display is composed of multiple elements)
   pinmame_tAlphaDisplayDef displayDefs[];    // Definition of each display (number of elements, hardware type,...)
   // pinmame_tAlphaSegmentState states[];    // State of each display element as a linear array
} pinmame_tAlphaStates;
#define PINMAME_STATE_BLOCK_FIRST_ALPHA_FRAME(pAlphaState) ((pinmame_tAlphaSegmentState*)((UINT8*)(pAlphaState) + sizeof(pinmame_tAlphaStates) + (pAlphaState->nDisplays) * sizeof(pinmame_tAlphaDisplayDef)))


// DMD and video displays

typedef enum {
   PINMAME_FRAME_FORMAT_LUM,     // Linear luminance (for monochrome DMD)
   PINMAME_FRAME_FORMAT_RGB,     // sRGB (for video frame)
   PINMAME_FRAME_FORMAT_BP2,     // 2 bitplanes, only used to identify frames
   PINMAME_FRAME_FORMAT_BP4      // 4 bitplanes, only used to identify frames
} pinmame_tFrameDataFormat;

#pragma warning(disable : 4200)
typedef struct
{
   unsigned int             structSize;      // Struct size including header and frame data in bytes (for safe DMD/Display array iteration)
   unsigned int             displayId;       // Unique Id, shared between render frame and raw frame used for frame identification
   double                   updateTimestamp;
   unsigned int             width;
   unsigned int             height;
   pinmame_tFrameDataFormat dataFormat;
   unsigned int             frameId;
   UINT8                    frameData[];     // The display frame data which size depends on width, height and data format
} pinmame_tFrameState;

typedef struct
{
   unsigned int nDisplays;
   // pinmame_tFrameState displays[]; // Array of nDisplays * pinmame_tFrameState (can't be directly declared since frame size is undefined)
} pinmame_tDisplayStates;
#define PINMAME_STATE_BLOCK_FIRST_DISPLAY_FRAME(pDisplayState) ((pinmame_tFrameState*)((UINT8*)(pDisplayState) + sizeof(pinmame_tDisplayStates)))
#define PINMAME_STATE_BLOCK_NEXT_DISPLAY_FRAME(pFrameState) ((pinmame_tFrameState*)((UINT8*)(pFrameState) + (pFrameState)->structSize))


// Global output state block

typedef struct
{
   unsigned int versionID;                               // Data block format version (current version
   pinmame_tBinaryStates*  controlledOutputBinaryStates; // Binary state (instantaneous state of controlled output)
   pinmame_tDeviceStates*  controlledDeviceStates;       // Device state (state of the emulated device)
   pinmame_tDeviceStates*  lampMatrixStates;             // State of emulated lamps powered from a strobed lamp matrix
   pinmame_tAlphaStates*   alphaDisplayStates;           // State of alphanumeric segment displays
   pinmame_tDisplayStates* displayStates;                // State of displays and DMDs
   pinmame_tDisplayStates* rawDMDStates;                 // Raw logic state of DMDs (stable view of the stae that can be used for frame identification)
} pinmame_tMachineOutputState;

#define PINMAME_STATE_REQMASK_GPOUTPUT_BINARY_STATE 0x01
#define PINMAME_STATE_REQMASK_GPOUTPUT_DEVICE_STATE 0x02
#define PINMAME_STATE_REQMASK_LAMP_DEVICE_STATE     0x04
#define PINMAME_STATE_REQMASK_ALPHA_DEVICE_STATE    0x08
#define PINMAME_STATE_REQMASK_DISPLAY_STATE         0x10
#define PINMAME_STATE_REQMASK_RAW_DMD_STATE         0x20
#define PINMAME_STATE_REQMASK_ALL                   0x3F

