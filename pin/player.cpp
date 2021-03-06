#include "stdafx.h"
//#define USE_IMGUI
#ifdef USE_IMGUI
 #include "imgui/imgui.h"
 #include "imgui/imgui_impl_dx9.h"
 #include "imgui/imgui_impl_win32.h"
 #include "imgui/implot/implot.h"

// utility structure for realtime plot //!! cleanup
class ScrollingData {
private:
    int MaxSize;
public:
    int Offset;
    ImVector<ImVec2> Data;
    ScrollingData() {
        MaxSize = 500;
        Offset  = 0;
        Data.reserve(MaxSize);
    }
    void AddPoint(const float x, const float y) {
        if (Data.size() < MaxSize)
            Data.push_back(ImVec2(x,y));
        else {
            Data[Offset] = ImVec2(x,y);
            Offset++;
            if (Offset == MaxSize)
                Offset = 0;
        }
    }
    void Erase() {
        if (Data.size() > 0) {
            Data.shrink(0);
            Offset  = 0;
        }
    }
    ImVec2 GetLast() {
        if (Data.size() == 0)
            return ImVec2(0.f, 0.f);
        else if (Data.size() < MaxSize || Offset == 0)
            return Data.back();
        else
            return Data[Offset - 1];
    }
};

// utility structure for realtime plot
/*class RollingData {
    float Span;
    ImVector<ImVec2> Data;
    RollingData() {
        Span = 10.0f;
        Data.reserve(500);
    }
    void AddPoint(const float x, const float y) {
        const float xmod = fmodf(x, Span);
        if (!Data.empty() && xmod < Data.back().x)
            Data.shrink(0);
        Data.push_back(ImVec2(xmod, y));
    }
};*/
#endif

#include <algorithm>
#include <time.h>
#include "../meshes/ballMesh.h"
#include "BallShader.h"

// touch defines, delete as soon as we can get rid of old compilers and use new ones that have these natively
//#define TEST_TOUCH_WITH_MOUSE
#ifdef TEST_TOUCH_WITH_MOUSE
#define WM_POINTERDOWN WM_LBUTTONDOWN
#define WM_POINTERUP WM_LBUTTONUP
#else
#define WM_POINTERDOWN 0x0246
#define WM_POINTERUP 0x0247
#endif

typedef enum tagPOINTER_INPUT_TYPE {
   PT_POINTER = 0x00000001,
   PT_TOUCH = 0x00000002,
   PT_PEN = 0x00000003,
   PT_MOUSE = 0x00000004
} POINTER_INPUT_TYPE;

typedef enum tagPOINTER_FLAGS
{
   POINTER_FLAG_NONE = 0x00000000,
   POINTER_FLAG_NEW = 0x00000001,
   POINTER_FLAG_INRANGE = 0x00000002,
   POINTER_FLAG_INCONTACT = 0x00000004,
   POINTER_FLAG_FIRSTBUTTON = 0x00000010,
   POINTER_FLAG_SECONDBUTTON = 0x00000020,
   POINTER_FLAG_THIRDBUTTON = 0x00000040,
   POINTER_FLAG_OTHERBUTTON = 0x00000080,
   POINTER_FLAG_PRIMARY = 0x00000100,
   POINTER_FLAG_CONFIDENCE = 0x00000200,
   POINTER_FLAG_CANCELLED = 0x00000400,
   POINTER_FLAG_DOWN = 0x00010000,
   POINTER_FLAG_UPDATE = 0x00020000,
   POINTER_FLAG_UP = 0x00040000,
   POINTER_FLAG_WHEEL = 0x00080000,
   POINTER_FLAG_HWHEEL = 0x00100000,
   POINTER_FLAG_CAPTURECHANGED = 0x00200000
} POINTER_FLAGS;

typedef enum _POINTER_BUTTON_CHANGE_TYPE {
   POINTER_CHANGE_NONE,
   POINTER_CHANGE_FIRSTBUTTON_DOWN,
   POINTER_CHANGE_FIRSTBUTTON_UP,
   POINTER_CHANGE_SECONDBUTTON_DOWN,
   POINTER_CHANGE_SECONDBUTTON_UP,
   POINTER_CHANGE_THIRDBUTTON_DOWN,
   POINTER_CHANGE_THIRDBUTTON_UP,
   POINTER_CHANGE_FOURTHBUTTON_DOWN,
   POINTER_CHANGE_FOURTHBUTTON_UP,
   POINTER_CHANGE_FIFTHBUTTON_DOWN,
   POINTER_CHANGE_FIFTHBUTTON_UP
} POINTER_BUTTON_CHANGE_TYPE;

typedef enum tagFEEDBACK_TYPE {
	FEEDBACK_TOUCH_CONTACTVISUALIZATION = 1,
	FEEDBACK_PEN_BARRELVISUALIZATION = 2,
	FEEDBACK_PEN_TAP = 3,
	FEEDBACK_PEN_DOUBLETAP = 4,
	FEEDBACK_PEN_PRESSANDHOLD = 5,
	FEEDBACK_PEN_RIGHTTAP = 6,
	FEEDBACK_TOUCH_TAP = 7,
	FEEDBACK_TOUCH_DOUBLETAP = 8,
	FEEDBACK_TOUCH_PRESSANDHOLD = 9,
	FEEDBACK_TOUCH_RIGHTTAP = 10,
	FEEDBACK_GESTURE_PRESSANDTAP = 11,
	FEEDBACK_MAX = 0xFFFFFFFF
} FEEDBACK_TYPE;

typedef BOOL(WINAPI *pSWFS)(
	HWND          hwnd,
	FEEDBACK_TYPE feedback,
	DWORD         dwFlags,
	UINT32        size,
	const VOID    *configuration
	);

static pSWFS SetWindowFeedbackSetting = NULL;

typedef struct tagPOINTER_INFO {
   POINTER_INPUT_TYPE         pointerType;
   UINT32                     pointerId;
   UINT32                     frameId;
   POINTER_FLAGS              pointerFlags;
   HANDLE                     sourceDevice;
   HWND                       hwndTarget;
   POINT                      ptPixelLocation;
   POINT                      ptHimetricLocation;
   POINT                      ptPixelLocationRaw;
   POINT                      ptHimetricLocationRaw;
   DWORD                      dwTime;
   UINT32                     historyCount;
   INT32                      inputData;
   DWORD                      dwKeyStates;
   UINT64                     PerformanceCount;
   POINTER_BUTTON_CHANGE_TYPE ButtonChangeType;
} POINTER_INFO;

typedef BOOL(WINAPI *pGPI)(UINT32 pointerId, POINTER_INFO *pointerInfo);

static pGPI GetPointerInfo = NULL;

#define GET_POINTERID_WPARAM(wParam) (LOWORD (wParam))

#define NID_READY 0x00000080
#define NID_MULTI_INPUT 0x00000040

#define SM_DIGITIZER 94
#define SM_MAXIMUMTOUCHES 95

typedef BOOL(WINAPI *pUnregisterTouchWindow)(HWND hWnd);
static pUnregisterTouchWindow UnregisterTouchWindow = NULL;

#if 0 // useful if supporting 'real' WM_TOUCH messages
typedef BOOL(WINAPI *pIsTouchWindow)(HWND hwnd, PULONG pulFlags);
static pIsTouchWindow IsTouchWindow = NULL;

typedef BOOL(WINAPI *pRegisterTouchWindow)(HWND hWnd, ULONG ulFlags);
static pRegisterTouchWindow RegisterTouchWindow = NULL;

#define MICROSOFT_TABLETPENSERVICE_PROPERTY _T("MicrosoftTabletPenServiceProperty")
#define TABLET_DISABLE_PRESSANDHOLD        0x00000001
#define TABLET_DISABLE_PENTAPFEEDBACK      0x00000008
#define TABLET_DISABLE_PENBARRELFEEDBACK   0x00000010
#define TABLET_DISABLE_TOUCHUIFORCEON      0x00000100
#define TABLET_DISABLE_TOUCHUIFORCEOFF     0x00000200
#define TABLET_DISABLE_TOUCHSWITCH         0x00008000
#define TABLET_DISABLE_FLICKS              0x00010000
#define TABLET_ENABLE_FLICKSONCONTEXT      0x00020000
#define TABLET_ENABLE_FLICKLEARNINGMODE    0x00040000
#define TABLET_DISABLE_SMOOTHSCROLLING     0x00080000
#define TABLET_DISABLE_FLICKFALLBACKKEYS   0x00100000
#define TABLET_ENABLE_MULTITOUCHDATA       0x01000000

#define GC_ALLGESTURES                              0x00000001

typedef struct tagGESTURECONFIG {
	DWORD dwID;                     // gesture ID
	DWORD dwWant;                   // settings related to gesture ID that are to be turned on
	DWORD dwBlock;                  // settings related to gesture ID that are to be turned off
} GESTURECONFIG, *PGESTURECONFIG;

typedef BOOL(WINAPI *pSetGestureConfig)(HWND hwnd, DWORD dwReserved, UINT cIDs, PGESTURECONFIG pGestureConfig, UINT cbSize);
static pSetGestureConfig SetGestureConfig = NULL;
#endif

//
//
//

// precomputed blue-noiseish points for oversampling the static pre-rendered objects

#define STATIC_PRERENDER_ITERATIONS /*32*/64/*128*/
/*   static const float xyLDBNbnot[STATIC_PRERENDER_ITERATIONS*2] = {
     0.00000f,0.00000f,
     0.23151f,0.02134f,
     0.36499f,0.09920f,
     0.72093f,0.06583f,
     0.10915f,0.14369f,
     0.28712f,0.25492f,
     0.56520f,0.15482f,
     0.79879f,0.23268f,
     0.97676f,0.14925f,
     0.14252f,0.35503f,
     0.34274f,0.44402f,
     0.46509f,0.31054f,
     0.65419f,0.33279f,
     0.83216f,0.41621f,
     0.98788f,0.34947f,
     0.00905f,0.46626f,
     0.20926f,0.52188f,
     0.38723f,0.62199f,
     0.54296f,0.47739f,
     0.66531f,0.57749f,
     0.82103f,0.60530f,
     0.07578f,0.64423f,
     0.13140f,0.79996f,
     0.27600f,0.72209f,
     0.53183f,0.68873f,
     0.68756f,0.77771f,
     0.96563f,0.69429f,
     0.17589f,0.97793f,
     0.32049f,0.92231f,
     0.48734f,0.85557f,
     0.67643f,0.95568f,
     0.85440f,0.82776f};*/

   static const float xyLDBNbnot[STATIC_PRERENDER_ITERATIONS*2] = {
     0.00000f,0.00000f,
     0.13926f,0.00971f,
     0.99043f,0.01365f,
     0.02105f,0.06487f,
     0.11562f,0.13580f,
     0.24960f,0.14369f,
     0.31265f,0.05699f,
     0.43087f,0.12004f,
     0.56484f,0.11216f,
     0.71459f,0.07670f,
     0.85645f,0.10822f,
     0.96678f,0.14763f,
     0.06045f,0.26978f,
     0.19443f,0.26190f,
     0.36782f,0.22250f,
     0.50968f,0.23826f,
     0.66730f,0.19097f,
     0.78552f,0.19491f,
     0.86433f,0.28161f,
     0.02893f,0.37224f,
     0.15503f,0.39588f,
     0.28900f,0.34071f,
     0.41510f,0.34860f,
     0.52544f,0.36436f,
     0.64366f,0.30131f,
     0.75399f,0.34465f,
     0.88797f,0.41558f,
     0.97466f,0.30525f,
     0.07621f,0.50622f,
     0.21019f,0.49834f,
     0.33629f,0.45105f,
     0.46239f,0.49046f,
     0.62789f,0.44317f,
     0.76975f,0.47075f,
     0.91161f,0.54168f,
     0.12350f,0.62443f,
     0.28112f,0.58503f,
     0.39146f,0.60867f,
     0.54908f,0.56139f,
     0.60425f,0.65596f,
     0.69094f,0.55745f,
     0.82492f,0.61261f,
     0.09001f,0.75841f,
     0.20822f,0.71113f,
     0.32644f,0.72689f,
     0.49194f,0.68748f,
     0.62592f,0.78994f,
     0.72838f,0.70719f,
     0.83871f,0.76235f,
     0.94905f,0.69142f,
     0.00331f,0.87663f,
     0.09789f,0.90816f,
     0.21610f,0.83723f,
     0.38949f,0.82146f,
     0.50771f,0.81358f,
     0.57864f,0.90027f,
     0.74414f,0.85693f,
     0.85448f,0.89633f,
     0.97269f,0.81752f,
     0.27915f,0.92392f,
     0.35008f,0.99485f,
     0.45254f,0.96332f,
     0.69685f,0.96726f,
     0.91753f,0.97515f};

/*   static const float xyLDBNbnot[STATIC_PRERENDER_ITERATIONS*2] = {
     0.00000f,0.00000f,
     0.01267f,0.07171f,
     0.10232f,0.04930f,
     0.18638f,0.01007f,
     0.28725f,0.02688f,
     0.43295f,0.08292f,
     0.50579f,0.00447f,
     0.53942f,0.07732f,
     0.66830f,0.03249f,
     0.75796f,0.06611f,
     0.88685f,0.01568f,
     0.98211f,0.05490f,
     0.07431f,0.14456f,
     0.15836f,0.13335f,
     0.22000f,0.09413f,
     0.30406f,0.13896f,
     0.36010f,0.08852f,
     0.52260f,0.16137f,
     0.61226f,0.12775f,
     0.69632f,0.11654f,
     0.78598f,0.17258f,
     0.84202f,0.09973f,
     0.91487f,0.11094f,
     0.99332f,0.15577f,
     0.01827f,0.18379f,
     0.13034f,0.23422f,
     0.23681f,0.18939f,
     0.33208f,0.22301f,
     0.41613f,0.17818f,
     0.48898f,0.22862f,
     0.59545f,0.24543f,
     0.67951f,0.20060f,
     0.77477f,0.25103f,
     0.88124f,0.19499f,
     0.97090f,0.23982f,
     0.04629f,0.27345f,
     0.08551f,0.34630f,
     0.19759f,0.26784f,
     0.25923f,0.31828f,
     0.35449f,0.30707f,
     0.43855f,0.29586f,
     0.52821f,0.31267f,
     0.62347f,0.33509f,
     0.70753f,0.28465f,
     0.80840f,0.32388f,
     0.89245f,0.27905f,
     0.99892f,0.32948f,
     0.06310f,0.42475f,
     0.16397f,0.35750f,
     0.24242f,0.43035f,
     0.32087f,0.39113f,
     0.41053f,0.38552f,
     0.47778f,0.41354f,
     0.56743f,0.39673f,
     0.64589f,0.43596f,
     0.71313f,0.37431f,
     0.81400f,0.40794f,
     0.90366f,0.36871f,
     0.97651f,0.41914f,
     0.00006f,0.45837f,
     0.10653f,0.52001f,
     0.16817f,0.45277f,
     0.29145f,0.51441f,
     0.35870f,0.48079f,
     0.44836f,0.50320f,
     0.54362f,0.49199f,
     0.62768f,0.53122f,
     0.73975f,0.46397f,
     0.87424f,0.47518f,
     0.95269f,0.49760f,
     0.01687f,0.54803f,
     0.16257f,0.61527f,
     0.20179f,0.54243f,
     0.26904f,0.60967f,
     0.36430f,0.58165f,
     0.46517f,0.59286f,
     0.54922f,0.58726f,
     0.70052f,0.55363f,
     0.79018f,0.53682f,
     0.80139f,0.62088f,
     0.88545f,0.57044f,
     0.07291f,0.70493f,
     0.07851f,0.62648f,
     0.20739f,0.68252f,
     0.30826f,0.68812f,
     0.39792f,0.66571f,
     0.49319f,0.67131f,
     0.60526f,0.64890f,
     0.69492f,0.63209f,
     0.77337f,0.71054f,
     0.86303f,0.69933f,
     0.93028f,0.63769f,
     0.01126f,0.73856f,
     0.14575f,0.76097f,
     0.24102f,0.77218f,
     0.31387f,0.79459f,
     0.40353f,0.74976f,
     0.48758f,0.76658f,
     0.57724f,0.72735f,
     0.68371f,0.72175f,
     0.72294f,0.80020f,
     0.82941f,0.77778f,
     0.93588f,0.75537f,
     0.06730f,0.81701f,
     0.11774f,0.88986f,
     0.19058f,0.85624f,
     0.28024f,0.88425f,
     0.38671f,0.83942f,
     0.47637f,0.86184f,
     0.56603f,0.83382f,
     0.63888f,0.80580f,
     0.76777f,0.87305f,
     0.84622f,0.86744f,
     0.91347f,0.84503f,
     0.02808f,0.90667f,
     0.10092f,0.97952f,
     0.18498f,0.96831f,
     0.26343f,0.97391f,
     0.36990f,0.92348f,
     0.45956f,0.94589f,
     0.55483f,0.91788f,
     0.65009f,0.89546f,
     0.70613f,0.95710f,
     0.80699f,0.96271f,
     0.90786f,0.95150f,
     0.99192f,0.91227f,
     0.00566f,0.99072f,
     0.61086f,0.98512f};*/

//
//
//

const RECT touchregion[8] = { //left,top,right,bottom (in % of screen)
   { 0, 0, 50, 10 },      // ExtraBall
   { 0, 10, 50, 50 },     // 2nd Left Button
   { 0, 50, 50, 90 },     // 1st Left Button (Flipper)
   { 0, 90, 50, 100 },    // Start
   { 50, 0, 100, 10 },    // Exit
   { 50, 10, 100, 50 },   // 2nd Right Button
   { 50, 50, 100, 90 },   // 1st Right Button (Flipper)
   { 50, 90, 100, 100 } }; // Plunger

EnumAssignKeys touchkeymap[8] = {
   eAddCreditKey, //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   eLeftMagnaSave,
   eLeftFlipperKey,
   eStartGameKey,
   eExitGame,
   eRightMagnaSave,
   eRightFlipperKey,
   ePlungerKey };

#if !(_WIN32_WINNT >= 0x0500)
 #define KEYEVENTF_SCANCODE    0x0008
#endif /* _WIN32_WINNT >= 0x0500 */

//

static unsigned int material_flips = 0;
static unsigned int stats_drawn_static_triangles = 0;

extern int disEnableTrueFullscreen; // set via command line

//

#define RECOMPUTEBUTTONCHECK WM_USER+100
#define RESIZE_FROM_EXPAND WM_USER+101

INT_PTR CALLBACK PauseProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);


Player::Player(const bool cameraMode, PinTable * const ptable) : m_cameraMode(cameraMode)
{
   {
      int regs[4];
      __cpuid(regs, 1);
      // check for SSE and exit if not available, as some code relies on it by now
      if ((regs[3] & 0x002000000) == 0) { // No SSE?
         ShowError("SSE is not supported on this processor");
         exit(0);
      }
      // disable denormalized floating point numbers, can be faster on some CPUs (and VP doesn't need to rely on denormals)
      if ((regs[3] & 0x004000000) != 0) // SSE2?
         _mm_setcsr(_mm_getcsr() | 0x8040); // flush denorms to zero and also treat incoming denorms as zeros
      else
         _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON); // only flush denorms to zero
   }

#ifdef STEPPING
   m_pause = false;
   m_step = false;
#endif

   m_pseudoPause = false;
   m_pauseRefCount = 0;
   m_noTimeCorrect = false;

   m_toogle_DTFS = false;

   m_isRenderingStatic = false;

   m_throwBalls = false;
   m_ballControl = false;
   m_pactiveballBC = NULL;
   m_pBCTarget = NULL;

#ifdef PLAYBACK
   m_playback = false;
   m_fplaylog = NULL;
#endif

#ifdef LOG
   m_flog = NULL;
#endif

   for (int i = 0; i < PININ_JOYMXCNT; ++i) {
      m_curAccel_x[i] = 0;
      m_curAccel_y[i] = 0;
   }

   m_sleeptime = 0;

   m_audio = NULL;
   m_pactiveball = NULL;

   m_curPlunger = JOYRANGEMN - 1;

   m_current_renderstage = 0;
   m_dmdstate = 0;

   m_VSync = LoadValueIntWithDefault("Player", "AdaptiveVSync", 0);
   m_maxPrerenderedFrames = LoadValueIntWithDefault("Player", "MaxPrerenderedFrames", 0);
   m_NudgeShake = LoadValueFloatWithDefault("Player", "NudgeStrength", 2e-2f);
   m_FXAA = LoadValueIntWithDefault("Player", "FXAA", Disabled);
   m_trailForBalls = LoadValueBoolWithDefault("Player", "BallTrail", true);
   m_reflectionForBalls = LoadValueBoolWithDefault("Player", "BallReflection", true);
   m_AA = LoadValueBoolWithDefault("Player", "USEAA", false);
   m_dynamicAO = LoadValueBoolWithDefault("Player", "DynamicAO", false);
   m_disableAO = LoadValueBoolWithDefault("Player", "DisableAO", false);
   m_ss_refl = LoadValueBoolWithDefault("Player", "SSRefl", false);
   m_pf_refl = LoadValueBoolWithDefault("Player", "PFRefl", true);
   m_stereo3D = LoadValueIntWithDefault("Player", "Stereo3D", 0);
   m_stereo3Denabled = LoadValueBoolWithDefault("Player", "Stereo3DEnabled", (m_stereo3D != 0));
   m_stereo3DY = LoadValueBoolWithDefault("Player", "Stereo3DYAxis", false);
   m_scaleFX_DMD = LoadValueBoolWithDefault("Player", "ScaleFXDMD", false);
   m_disableDWM = LoadValueBoolWithDefault("Player", "DisableDWM", false);
   m_useNvidiaApi = LoadValueBoolWithDefault("Player", "UseNVidiaAPI", false);
   m_bloomOff = LoadValueBoolWithDefault("Player", "ForceBloomOff", false);
   m_ditherOff = LoadValueBoolWithDefault("Player", "Render10Bit", false); // if rendering at 10bit output resolution, disable dithering
   m_BWrendering = LoadValueIntWithDefault("Player", "BWRendering", 0);
   m_detectScriptHang = LoadValueBoolWithDefault("Player", "DetectHang", false);

   m_ballImage = NULL;
   m_decalImage = NULL;

   m_overwriteBallImages = LoadValueBoolWithDefault("Player", "OverwriteBallImage", false);
   m_minphyslooptime = min(LoadValueIntWithDefault("Player", "MinPhysLoopTime", 0), 1000);

   if (m_overwriteBallImages)
   {
       char imageName[MAX_PATH];
       //memset(imageName, 0, sizeof(imageName));
       HRESULT hr = LoadValueString("Player", "BallImage", imageName, MAX_PATH);
       if (hr == S_OK)
       {
           BaseTexture * const tex = BaseTexture::CreateFromFile(imageName);

           if (tex != NULL)
               m_ballImage = new Texture(tex);
       }
       // clear the string buffer, otherwise it will hold the ball image filename if decal image is a null string in the registry
       //memset(imageName, 0, sizeof(ImageName));
       hr = LoadValueString("Player", "DecalImage", imageName, MAX_PATH);
       if (hr == S_OK)
       {
           BaseTexture * const tex = BaseTexture::CreateFromFile(imageName);

           if (tex != NULL)
               m_decalImage = new Texture(tex);
       }
   }
   
   m_throwBalls = LoadValueBoolWithDefault("Editor", "ThrowBallsAlwaysOn", false);
   m_ballControl = LoadValueBoolWithDefault("Editor", "BallControlAlwaysOn", false);
   m_debugBallSize = LoadValueIntWithDefault("Editor", "ThrowBallSize", 50);
   m_debugBallMass = LoadValueFloatWithDefault("Editor", "ThrowBallMass", 1.0f);

   //m_low_quality_bloom = LoadValueBoolWithDefault("Player", "LowQualityBloom", false);

   const int numberOfTimesToShowTouchMessage = LoadValueIntWithDefault("Player", "NumberOfTimesToShowTouchMessage", 10);
   SaveValueInt("Player", "NumberOfTimesToShowTouchMessage", max(numberOfTimesToShowTouchMessage-1,0));
   m_showTouchMessage = (numberOfTimesToShowTouchMessage != 0);

   m_showFPS = 0;

   m_closeDown = false;
   m_closeDownDelay = true;
   m_showWindowedCaption = false;
   m_closeType = 0;
   m_showDebugger = false;

   m_debugBalls = false;
   m_toggleDebugBalls = false;

   m_swap_ball_collision_handling = false;

   m_debugMode = false;

#ifdef STEPPING
   m_pauseTimeTarget = 0;
#endif
   m_pactiveballDebug = NULL;

   m_gameWindowActive = false;
   m_debugWindowActive = false;
   m_userDebugPaused = false;
   m_hwndDebugOutput = NULL;

   m_LastKnownGoodCounter = 0;
   m_ModalRefCount = 0;

   m_drawCursor = false;
   m_lastcursorx = 0xfffffff;
   m_lastcursory = 0xfffffff;

#ifdef DEBUGPHYSICS
   c_hitcnts = 0;
   c_collisioncnt = 0;
   c_contactcnt = 0;
#ifdef C_DYNAMIC
   c_staticcnt = 0;
#endif
   c_embedcnts = 0;
   c_timesearch = 0;

   c_kDNextlevels = 0;
   c_quadNextlevels = 0;

   c_traversed = 0;
   c_tested = 0;
   c_deepTested = 0;
#endif

   m_movedPlunger = 0;
   m_LastPlungerHit = 0;
   m_lastFlipTime = 0;

   for (unsigned int i = 0; i < 8; ++i)
      m_touchregion_pressed[i] = false;

   m_recordContacts = false;
   m_contacts.reserve(8);

   m_overall_frames = 0;

   m_dmdx = 0;
   m_dmdy = 0;
   m_texdmd = NULL;
   m_backdropSettingActive = 0;

   m_ScreenOffset = Vertex2D(0, 0);

   m_ballIndexBuffer = NULL;
   m_ballVertexBuffer = NULL;
#ifdef DEBUG_BALL_SPIN
   m_ballDebugPoints = NULL;
#endif
   m_ballTrailVertexBuffer = NULL;
   m_pFont = NULL;
   m_meshAsPlayfield = false;
   m_ptable = ptable;
}

Player::~Player()
{

    if (m_fontSprite)
    {
        m_fontSprite->Release();
        m_fontSprite = NULL;
    }
    if (m_pFont)
    {
        m_pFont->Release();
        m_pFont = NULL;
    }
    if (m_ballImage)
    {
       delete m_ballImage;
       m_ballImage = NULL;
    }
    if (m_decalImage)
    {
       delete m_decalImage;
       m_decalImage = NULL;
    }
    if (m_pBCTarget)
    {
       delete m_pBCTarget;
       m_pBCTarget = nullptr;
    }
}

void Player::PreRegisterClass(WNDCLASS& wc)
{
    wc.style = 0;
    wc.hInstance = g_pvp->theInstance;
    wc.lpszClassName = "VPPlayer"; // leave as-is as e.g. VPM relies on this
    wc.hIcon = LoadIcon(g_pvp->theInstance, MAKEINTRESOURCE(IDI_TABLE));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszMenuName = NULL;
}

void Player::PreCreate(CREATESTRUCT& cs)
{
    m_fullScreen = LoadValueBoolWithDefault("Player", "FullScreen", IsWindows10_1803orAbove());

    // command line override
    if (disEnableTrueFullscreen == 0)
        m_fullScreen = false;
    else if (disEnableTrueFullscreen == 1)
        m_fullScreen = true;

    int x, y;
    int display = LoadValueIntWithDefault("Player", "Display", -1);
    display = (display < getNumberOfDisplays()) ? display : -1;
    getDisplaySetupByID(display, x, y, m_screenwidth, m_screenheight);

    m_width = LoadValueIntWithDefault("Player", "Width", m_fullScreen ? -1 : DEFAULT_PLAYER_WIDTH);
    m_height = LoadValueIntWithDefault("Player", "Height", m_width * 9 / 16);
    if (m_width <= 0)
    {
        m_width = m_screenwidth;
        m_height = m_screenheight;
    }

    if (m_fullScreen)
    {
        x = 0;
        y = 0;
        m_screenwidth = m_width;
        m_screenheight = m_height;
        m_refreshrate = LoadValueIntWithDefault("Player", "RefreshRate", 0);
    }
    else
    {
        m_refreshrate = 0; // The default

        // constrain window to screen
        if (m_width > m_screenwidth)
        {
            m_width = m_screenwidth;
            m_height = m_width * 9 / 16;
        }

        if (m_height > m_screenheight)
        {
            m_height = m_screenheight;
            m_width = m_height * 16 / 9;
        }

        x += (m_screenwidth - m_width) / 2;
        y += (m_screenheight - m_height) / 2;

        // is this a non-fullscreen window? -> get previously saved window position
        if ((m_height != m_screenheight) || (m_width != m_screenwidth))
        {
            const int xn = LoadValueIntWithDefault("Player", "WindowPosX", x); //!! does this handle multi-display correctly like this?
            const int yn = LoadValueIntWithDefault("Player", "WindowPosY", y);

            RECT r;
            r.left = xn;
            r.top = yn;
            r.right = xn + m_width;
            r.bottom = yn + m_height;
            if (MonitorFromRect(&r, MONITOR_DEFAULTTONULL) != NULL) // window is visible somewhere, so use the coords from the registry
            {
                x = xn;
                y = yn;
            }
        }
    }

    int windowflags;
    int windowflagsex;

    const int captionheight = GetSystemMetrics(SM_CYCAPTION);

    if (false) // only do this nowadays if ESC menu is brought up //(!m_fullScreen && ((m_screenheight - m_height) >= (captionheight * 2))) // We have enough room for a frame?
    {
        // Add a pretty window border and standard control boxes.

        windowflags = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN;
        windowflagsex = WS_EX_OVERLAPPEDWINDOW;

        //!! does not respect borders so far!!! -> change width/height accordingly ??
        //!! like this the render window is scaled and thus implicitly blurred!
        y -= captionheight;
        m_height += captionheight;
    }
    else // No window border, title, or control boxes.
    {
        windowflags = WS_POPUP;
        windowflagsex = 0;
    }

    CalcBallAspectRatio();

    ZeroMemory(&cs, sizeof(cs));
    cs.x = x; 
    cs.y = y;
    cs.cx = m_width;
    cs.cy = m_height;
    cs.style = windowflags;
    cs.dwExStyle = windowflagsex;
    cs.hInstance = g_pvp->theInstance;
    cs.lpszName = "Visual Pinball Player"; // leave as-is as e.g. VPM relies on this
    cs.lpszClass = "VPPlayer"; // leave as-is as e.g. VPM relies on this
}

void Player::OnInitialUpdate()
{
    // Check for Touch support
    m_supportsTouch = ((GetSystemMetrics(SM_DIGITIZER) & NID_READY) != 0) && ((GetSystemMetrics(SM_DIGITIZER) & NID_MULTI_INPUT) != 0)
        && (GetSystemMetrics(SM_MAXIMUMTOUCHES) != 0);

#if 1 // we do not want to handle WM_TOUCH
    if (!UnregisterTouchWindow)
        UnregisterTouchWindow = (pUnregisterTouchWindow)GetProcAddress(GetModuleHandle(TEXT("user32.dll")), "UnregisterTouchWindow");
    if (UnregisterTouchWindow)
        UnregisterTouchWindow(GetHwnd());
#else // would be useful if handling WM_TOUCH instead of WM_POINTERDOWN
    // Disable palm detection
    if (!RegisterTouchWindow)
        RegisterTouchWindow = (pRegisterTouchWindow)GetProcAddress(GetModuleHandle(TEXT("user32.dll")), "RegisterTouchWindow");
    if (RegisterTouchWindow)
        RegisterTouchWindow(GetHwnd(), 0);

    if (!IsTouchWindow)
        IsTouchWindow = (pIsTouchWindow)GetProcAddress(GetModuleHandle(TEXT("user32.dll")), "IsTouchWindow");

    // Disable Gesture Detection
    if (!SetGestureConfig)
        SetGestureConfig = (pSetGestureConfig)GetProcAddress(GetModuleHandle(TEXT("user32.dll")), "SetGestureConfig");
    if (SetGestureConfig)
    {
        // http://msdn.microsoft.com/en-us/library/ms812373.aspx
        const DWORD dwHwndTabletProperty =
            TABLET_DISABLE_PRESSANDHOLD |      // disables press and hold (right-click) gesture
            TABLET_DISABLE_PENTAPFEEDBACK |    // disables UI feedback on pen up (waves)
            TABLET_DISABLE_PENBARRELFEEDBACK | // disables UI feedback on pen button down
            TABLET_DISABLE_FLICKS;             // disables pen flicks (back, forward, drag down, drag up)
        LPCTSTR tabletAtom = MICROSOFT_TABLETPENSERVICE_PROPERTY;

        // Get the Tablet PC atom ID
        const ATOM atomID = GlobalAddAtom(tabletAtom);
        if (atomID)
        {
            // Try to disable press and hold gesture 
            SetProp(m_playfieldHwnd, tabletAtom, (HANDLE)dwHwndTabletProperty);
        }
        // Gesture configuration
        GESTURECONFIG gc[] = { 0, 0, GC_ALLGESTURES };
        UINT uiGcs = 1;
        const BOOL bResult = SetGestureConfig(m_playfieldHwnd, 0, uiGcs, gc, sizeof(GESTURECONFIG));
    }
#endif

    // Disable visual feedback for touch, this saves one frame of latency on touchdisplays
    if (!SetWindowFeedbackSetting)
        SetWindowFeedbackSetting = (pSWFS)GetProcAddress(GetModuleHandle(TEXT("user32.dll")), "SetWindowFeedbackSetting");
    if (SetWindowFeedbackSetting)
    {
        const BOOL enabled = FALSE;

        SetWindowFeedbackSetting(GetHwnd(), FEEDBACK_TOUCH_CONTACTVISUALIZATION, 0, sizeof(enabled), &enabled);
        SetWindowFeedbackSetting(GetHwnd(), FEEDBACK_TOUCH_TAP, 0, sizeof(enabled), &enabled);
        SetWindowFeedbackSetting(GetHwnd(), FEEDBACK_TOUCH_DOUBLETAP, 0, sizeof(enabled), &enabled);
        SetWindowFeedbackSetting(GetHwnd(), FEEDBACK_TOUCH_PRESSANDHOLD, 0, sizeof(enabled), &enabled);
        SetWindowFeedbackSetting(GetHwnd(), FEEDBACK_TOUCH_RIGHTTAP, 0, sizeof(enabled), &enabled);
                                 
        SetWindowFeedbackSetting(GetHwnd(), FEEDBACK_PEN_BARRELVISUALIZATION, 0, sizeof(enabled), &enabled);
        SetWindowFeedbackSetting(GetHwnd(), FEEDBACK_PEN_TAP, 0, sizeof(enabled), &enabled);
        SetWindowFeedbackSetting(GetHwnd(), FEEDBACK_PEN_DOUBLETAP, 0, sizeof(enabled), &enabled);
        SetWindowFeedbackSetting(GetHwnd(), FEEDBACK_PEN_PRESSANDHOLD, 0, sizeof(enabled), &enabled);
        SetWindowFeedbackSetting(GetHwnd(), FEEDBACK_PEN_RIGHTTAP, 0, sizeof(enabled), &enabled);
                                 
        SetWindowFeedbackSetting(GetHwnd(), FEEDBACK_GESTURE_PRESSANDTAP, 0, sizeof(enabled), &enabled);
    }

    mixer_init(GetHwnd());
    hid_init();

    if (!m_fullScreen) // see above
        SetCursorPos(400, 999999);

    Init();
}

void Player::Shutdown()
{
#ifdef USE_IMGUI
   ImGui_ImplDX9_Shutdown();
   ImGui_ImplWin32_Shutdown();
   ImPlot::DestroyContext();
   ImGui::DestroyContext();
#endif

   if(m_toogle_DTFS && m_ptable->m_BG_current_set != 2)
       m_ptable->m_BG_current_set ^= 1;

   m_pininput.UnInit();

   SAFE_RELEASE(m_ballVertexBuffer);
   SAFE_RELEASE(m_ballIndexBuffer);
   if (m_ballShader)
   {
      CHECKD3D(m_ballShader->Core()->SetTexture("Texture0", NULL));
      CHECKD3D(m_ballShader->Core()->SetTexture("Texture1", NULL));
      CHECKD3D(m_ballShader->Core()->SetTexture("Texture2", NULL));
      CHECKD3D(m_ballShader->Core()->SetTexture("Texture3", NULL));
      delete m_ballShader;
      m_ballShader = 0;
   }
#ifdef DEBUG_BALL_SPIN
   SAFE_RELEASE(m_ballDebugPoints);
#endif
   SAFE_RELEASE(m_ballTrailVertexBuffer);
   if (m_ballImage)
   {
       delete m_ballImage;
       m_ballImage = NULL;
   }
   if (m_decalImage)
   {
       delete m_decalImage;
       m_decalImage = NULL;
   }

   m_limiter.Shutdown();

   for (size_t i = 0; i < m_vhitables.size(); ++i)
      m_vhitables[i]->EndPlay();

   for (size_t i = 0; i < m_vho.size(); i++)
      delete m_vho[i];
   m_vho.clear();

   for (size_t i = 0; i < m_vdebugho.size(); i++)
      delete m_vdebugho[i];
   m_vdebugho.clear();

   //!! cleanup the whole mem management for balls, this is a mess!

   // balls are added to the octree, but not the hit object vector
   for (size_t i = 0; i < m_vball.size(); i++)
   {
      Ball * const pball = m_vball[i];
      if (pball->m_pballex)
      {
         pball->m_pballex->m_pball = NULL;
         pball->m_pballex->Release();
      }

      delete pball->m_d.m_vpVolObjs;
      delete pball;
   }

   //!! see above
   //for (size_t i=0;i<m_vho_dynamic.size();i++)
   //      delete m_vho_dynamic[i];
   //m_vho_dynamic.clear();

   m_vball.clear();

   m_dmdx = 0;
   m_dmdy = 0;
   if (m_texdmd)
   {
      m_pin3d.m_pd3dPrimaryDevice->DMDShader->SetTexture("Texture0", (D3DTexture*)NULL);
      m_pin3d.m_pd3dPrimaryDevice->m_texMan.UnloadTexture(m_texdmd);
      delete m_texdmd;
      m_texdmd = NULL;
   }

#ifdef LOG
   if (m_flog)
      fclose(m_flog);
#endif
#ifdef PLAYBACK
   if (m_fplaylog)
      fclose(m_fplaylog);
#endif

   //CloseHandle(m_hSongCompletionEvent);

   if (m_audio)
   {
      delete m_audio;
      m_audio = NULL;
   }

   for (size_t i = 0; i < m_controlclsidsafe.size(); i++)
      delete m_controlclsidsafe[i];
   m_controlclsidsafe.clear();

   m_changed_vht.clear();

   restore_win_timer_resolution();
}

void Player::InitFPS()
{
    m_lastfpstime = m_time_msec;
    m_cframes = 0;
    m_fps = 0.0f;
    m_fpsAvg = 0.0f;
    m_fpsCount = 0;
    m_total = 0;
    m_count = 0;
    m_max = 0;
    m_max_total = 0;
    m_lastMaxChangeTime = 0;
    m_lastTime_usec = 0;

    m_phys_total = 0;
    m_phys_max = 0;
    m_phys_max_total = 0;
    m_phys_max_iterations = 0;
    m_phys_total_iterations = 0;

    m_script_total = 0;
    m_script_max = 0;
    m_script_max_total = 0;
}

void Player::ToggleFPS()
{
   ++m_showFPS;

   m_pin3d.m_gpu_profiler.Shutdown(); // Kill it so that it cannot influence standard rendering performance (and otherwise if just switching profile modes to not falsify counters and query info)
}

unsigned int Player::ProfilingMode()
{
   const unsigned int modes = (m_showFPS & 7);
   if (modes == 2)
      return 1;
   else if (modes == 3)
      return 2;
   else return 0;
}

bool Player::ShowFPS()
{
   const unsigned int modes = (m_showFPS & 7);
   return (modes == 1 || modes == 2 || modes == 3 || modes == 5 || modes == 7);
}

bool Player::RenderStaticOnly()
{
   const unsigned int modes = (m_showFPS & 7);
   return (modes == 5);
}

bool Player::RenderAOOnly()
{
   const unsigned int modes = (m_showFPS & 7);
   return (modes == 7);
}

void Player::RecomputePauseState()
{
   const bool oldPause = m_pause;
   const bool newPause = !(m_gameWindowActive || m_debugWindowActive);// || m_userDebugPaused;

   if (oldPause && newPause)
   {
      m_LastKnownGoodCounter++; // So our catcher doesn't catch on the last value
      m_noTimeCorrect = true;
   }

   m_pause = newPause;
}

void Player::RecomputePseudoPauseState()
{
   const bool oldPseudoPause = m_pseudoPause;
   m_pseudoPause = m_userDebugPaused || m_debugWindowActive;
   if (oldPseudoPause != m_pseudoPause)
   {
      if (m_pseudoPause)
         PauseMusic();
      else
         UnpauseMusic();
   }
}

//
// license:GPLv3+
// Ported at: VisualPinball.Engine/VPT/Table/TableHitGenerator.cs
//

void Player::AddCabinetBoundingHitShapes()
{
   // simple outer borders:
   LineSeg *plineseg;

   plineseg = new LineSeg(Vertex2D(m_ptable->m_right, m_ptable->m_top), Vertex2D(m_ptable->m_right, m_ptable->m_bottom), m_ptable->m_tableheight, m_ptable->m_glassheight);
   m_vho.push_back(plineseg);

   plineseg = new LineSeg(Vertex2D(m_ptable->m_left, m_ptable->m_bottom), Vertex2D(m_ptable->m_left, m_ptable->m_top), m_ptable->m_tableheight, m_ptable->m_glassheight);
   m_vho.push_back(plineseg);

   plineseg = new LineSeg(Vertex2D(m_ptable->m_right, m_ptable->m_bottom), Vertex2D(m_ptable->m_left, m_ptable->m_bottom), m_ptable->m_tableheight, m_ptable->m_glassheight);
   m_vho.push_back(plineseg);

   plineseg = new LineSeg(Vertex2D(m_ptable->m_left, m_ptable->m_top), Vertex2D(m_ptable->m_right, m_ptable->m_top), m_ptable->m_tableheight, m_ptable->m_glassheight);
   m_vho.push_back(plineseg);

   // glass:
   Vertex3Ds * const rgv3D = new Vertex3Ds[4];
   rgv3D[0] = Vertex3Ds(m_ptable->m_left, m_ptable->m_top, m_ptable->m_glassheight);
   rgv3D[1] = Vertex3Ds(m_ptable->m_right, m_ptable->m_top, m_ptable->m_glassheight);
   rgv3D[2] = Vertex3Ds(m_ptable->m_right, m_ptable->m_bottom, m_ptable->m_glassheight);
   rgv3D[3] = Vertex3Ds(m_ptable->m_left, m_ptable->m_bottom, m_ptable->m_glassheight);
   Hit3DPoly * const ph3dpoly = new Hit3DPoly(rgv3D, 4); //!!
   m_vho.push_back(ph3dpoly);

   /*
   // playfield:
   Vertex3Ds * const rgv3D = new Vertex3Ds[4];
   rgv3D[3] = Vertex3Ds(m_ptable->m_left, m_ptable->m_top, m_ptable->m_tableheight);
   rgv3D[2] = Vertex3Ds(m_ptable->m_right, m_ptable->m_top, m_ptable->m_tableheight);
   rgv3D[1] = Vertex3Ds(m_ptable->m_right, m_ptable->m_bottom, m_ptable->m_tableheight);
   rgv3D[0] = Vertex3Ds(m_ptable->m_left, m_ptable->m_bottom, m_ptable->m_tableheight);
   Hit3DPoly * const ph3dpoly = new Hit3DPoly(rgv3D, 4); //!!
   ph3dpoly->SetFriction(m_ptable->m_overridePhysics ? m_ptable->m_fOverrideContactFriction : m_ptable->m_friction);
   ph3dpoly->m_elasticity = m_ptable->m_overridePhysics ? m_ptable->m_fOverrideElasticity : m_ptable->m_elasticity;
   ph3dpoly->m_elasticityFalloff = m_ptable->m_overridePhysics ? m_ptable->m_fOverrideElasticityFalloff : m_ptable->m_elasticityFalloff;
   ph3dpoly->m_scatter = ANGTORAD(m_ptable->m_overridePhysics ? m_ptable->m_fOverrideScatterAngle : m_ptable->m_scatter);
   m_vho.push_back(ph3dpoly);
   */

   // playfield:
   m_hitPlayfield = HitPlane(Vertex3Ds(0, 0, 1), m_ptable->m_tableheight);
   m_hitPlayfield.SetFriction(m_ptable->m_overridePhysics ? m_ptable->m_fOverrideContactFriction : m_ptable->m_friction);
   m_hitPlayfield.m_elasticity = m_ptable->m_overridePhysics ? m_ptable->m_fOverrideElasticity : m_ptable->m_elasticity;
   m_hitPlayfield.m_elasticityFalloff = m_ptable->m_overridePhysics ? m_ptable->m_fOverrideElasticityFalloff : m_ptable->m_elasticityFalloff;
   m_hitPlayfield.m_scatter = ANGTORAD(m_ptable->m_overridePhysics ? m_ptable->m_fOverrideScatterAngle : m_ptable->m_scatter);

   // glass:
   m_hitTopGlass = HitPlane(Vertex3Ds(0, 0, -1), m_ptable->m_glassheight);
   m_hitTopGlass.m_elasticity = 0.2f;
}

//
// end of license:GPLv3+, back to 'old MAME'-like
//

void Player::InitKeys()
{
   for(unsigned int i = 0; i < eCKeys; ++i)
   {
      int key;
      const HRESULT hr = LoadValueInt("Player", regkey_string[i], &key);
      if (hr != S_OK || key > 0xdd)
          key = regkey_defdik[i];
      m_rgKeys[i] = (EnumAssignKeys)key;
   }
}

void Player::InitDebugHitStructure()
{
   for (size_t i = 0; i < m_vhitables.size(); ++i)
   {
      Hitable * const ph = m_vhitables[i];
      const size_t currentsize = m_vdebugho.size();
      ph->GetHitShapesDebug(m_vdebugho);
      const size_t newsize = m_vdebugho.size();
      // Save the objects the trouble of having the set the idispatch pointer themselves
      for (size_t hitloop = currentsize; hitloop < newsize; hitloop++)
         m_vdebugho[hitloop]->m_pfedebug = m_ptable->m_vedit[i]->GetIFireEvents();
   }

   for (size_t i = 0; i < m_vdebugho.size(); ++i)
   {
      m_vdebugho[i]->CalcHitBBox(); // maybe needed to update here, as only done lazily for some objects (i.e. balls!)
      m_debugoctree.AddElement(m_vdebugho[i]);
   }

   m_debugoctree.Initialize(m_ptable->GetBoundingBox());
}

Vertex3Ds g_viewDir;

static bool CompareHitableDepth(Hitable* h1, Hitable* h2)
{
   // GetDepth approximates direction in view distance to camera; sort ascending
   return h1->GetDepth(g_viewDir) >= h2->GetDepth(g_viewDir);
}

static bool CompareHitableDepthInverse(Hitable* h1, Hitable* h2)
{
	// GetDepth approximates direction in view distance to camera; sort descending
	return h1->GetDepth(g_viewDir) <= h2->GetDepth(g_viewDir);
}

static bool CompareHitableDepthReverse(Hitable* h1, Hitable* h2)
{
   // GetDepth approximates direction in view distance to camera; sort descending
   return h1->GetDepth(g_viewDir) < h2->GetDepth(g_viewDir);
}

static bool CompareHitableMaterial(Hitable* h1, Hitable* h2)
{
   return h1->GetMaterialID() < h2->GetMaterialID();
}

static bool CompareHitableImage(Hitable* h1, Hitable* h2)
{
	return h1->GetImageID() < h2->GetImageID();
}

void Player::UpdateBasicShaderMatrix(const Matrix3D& objectTrafo)
{
   D3DMATRIX worldMat;
   D3DMATRIX viewMat;
   D3DMATRIX projMat;
   m_pin3d.m_pd3dPrimaryDevice->GetTransform(TRANSFORMSTATE_WORLD, &worldMat);
   m_pin3d.m_pd3dPrimaryDevice->GetTransform(TRANSFORMSTATE_VIEW, &viewMat);
   m_pin3d.m_pd3dPrimaryDevice->GetTransform(TRANSFORMSTATE_PROJECTION, &projMat);

   D3DXMATRIX matProj(projMat);
   D3DXMATRIX matView(viewMat);
   D3DXMATRIX matWorld(worldMat);
   D3DXMATRIX matObject(objectTrafo);

   if (m_ptable->m_reflectionEnabled)
   {
      // *2.0f because every element is calculated that the lowest edge is around z=0 + table height so to get a correct
      // reflection the translation must be 1x table height + 1x table height to center around table height or 0
      matObject._43 -= m_ptable->m_tableheight*2.0f; 
   }

   D3DXMATRIX matWorldView = matObject * matWorld * matView;
   D3DXMATRIX matWorldViewProj = matWorldView * matProj;

   if (m_ptable->m_tblMirrorEnabled)
   {
      const D3DXMATRIX flipx(-1, 0, 0, 0,
         0, 1, 0, 0,
         0, 0, 1, 0,
         0, 0, 0, 1);
      const D3DXMATRIX flipy(1, 0, 0, 0,
         0, -1, 0, 0,
         0, 0, 1, 0,
         0, 0, 0, 1);
      const float rotation = fmodf(m_ptable->m_BG_rotation[m_ptable->m_BG_current_set], 360.f);
      matWorldViewProj = matWorldViewProj * (rotation != 0.0f ? flipy : flipx);
   }
   Matrix3D temp;
   memcpy(temp.m, matWorldView.m, 4 * 4 * sizeof(float));
   temp.Invert();
   temp.Transpose();
   D3DXMATRIX matWorldViewInvTrans;
   memcpy(matWorldViewInvTrans.m, temp.m, 4 * 4 * sizeof(float));

   m_pin3d.m_pd3dPrimaryDevice->basicShader->SetMatrix("matWorldViewProj", &matWorldViewProj);
   m_pin3d.m_pd3dPrimaryDevice->flasherShader->SetMatrix("matWorldViewProj", &matWorldViewProj);
   m_pin3d.m_pd3dPrimaryDevice->lightShader->SetMatrix("matWorldViewProj", &matWorldViewProj);
#ifdef SEPARATE_CLASSICLIGHTSHADER
   m_pin3d.m_pd3dPrimaryDevice->classicLightShader->SetMatrix("matWorldViewProj", &matWorldViewProj);
#endif

   m_pin3d.m_pd3dPrimaryDevice->DMDShader->SetMatrix("matWorldViewProj", &matWorldViewProj);

   m_pin3d.m_pd3dPrimaryDevice->basicShader->SetMatrix("matWorldView", &matWorldView);
   m_pin3d.m_pd3dPrimaryDevice->basicShader->SetMatrix("matWorldViewInverseTranspose", &matWorldViewInvTrans);
   //m_pin3d.m_pd3dDevice->basicShader->SetMatrix("matWorld", &matWorld);
   m_pin3d.m_pd3dPrimaryDevice->basicShader->SetMatrix("matView", &matView);
#ifdef SEPARATE_CLASSICLIGHTSHADER
   m_pin3d.m_pd3dPrimaryDevice->classicLightShader->SetMatrix("matWorldView", &matWorldView);
   m_pin3d.m_pd3dPrimaryDevice->classicLightShader->SetMatrix("matWorldViewInverseTranspose", &matWorldViewInvTrans);
   //m_pin3d.m_pd3dDevice->classicLightShader->SetMatrix("matWorld", &matWorld);
   m_pin3d.m_pd3dPrimaryDevice->classicLightShader->SetMatrix("matView", &matView);
#endif

   //memcpy(temp.m, matView.m, 4 * 4 * sizeof(float));
   //temp.Transpose();
   //D3DXMATRIX matViewInvInvTrans;
   //memcpy(matViewInvInvTrans.m, temp.m, 4 * 4 * sizeof(float));

   //m_pin3d.m_pd3dDevice->basicShader->SetMatrix("matViewInverseInverseTranspose", &matViewInvInvTrans);
#ifdef SEPARATE_CLASSICLIGHTSHADER
   //m_pin3d.m_pd3dDevice->classicLightShader->SetMatrix("matViewInverseInverseTranspose", &matViewInvInvTrans);
#endif
}

void Player::InitShader()
{
   /*D3DMATRIX worldMat;
   D3DMATRIX viewMat;
   D3DMATRIX projMat;
   m_pin3d.m_pd3dDevice->GetTransform(TRANSFORMSTATE_WORLD, &worldMat );
   m_pin3d.m_pd3dDevice->GetTransform(TRANSFORMSTATE_VIEW, &viewMat);
   m_pin3d.m_pd3dDevice->GetTransform(TRANSFORMSTATE_PROJECTION, &projMat);

   D3DXMATRIX matProj(projMat);
   D3DXMATRIX matView(viewMat);
   D3DXMATRIX matWorld(worldMat);
   D3DXMATRIX worldViewProj = matWorld * matView * matProj;*/

   UpdateBasicShaderMatrix();
   //vec4 cam( worldViewProj._41, worldViewProj._42, worldViewProj._43, 1 );
   //m_pin3d.m_pd3dDevice->basicShader->SetVector("camera", &cam);
#ifdef SEPARATE_CLASSICLIGHTSHADER
   //m_pin3d.m_pd3dDevice->classicLightShader->SetVector("camera", &cam);
#endif

   m_pin3d.m_pd3dPrimaryDevice->basicShader->SetBool("hdrEnvTextures", (m_pin3d.m_envTexture ? m_pin3d.m_envTexture : &m_pin3d.m_builtinEnvTexture)->IsHDR());
   m_pin3d.m_pd3dPrimaryDevice->basicShader->SetTexture("Texture1", m_pin3d.m_envTexture ? m_pin3d.m_envTexture : &m_pin3d.m_builtinEnvTexture, false);
   m_pin3d.m_pd3dPrimaryDevice->basicShader->SetTexture("Texture2", m_pin3d.m_pd3dPrimaryDevice->m_texMan.LoadTexture(m_pin3d.m_envRadianceTexture, false));
#ifdef SEPARATE_CLASSICLIGHTSHADER
   m_pin3d.m_pd3dPrimaryDevice->classicLightShader->SetBool("hdrEnvTextures", (m_pin3d.m_envTexture ? m_pin3d.m_envTexture : &m_pin3d.m_builtinEnvTexture)->IsHDR());
   m_pin3d.m_pd3dPrimaryDevice->classicLightShader->SetTexture("Texture1", m_pin3d.m_envTexture ? m_pin3d.m_envTexture : &m_pin3d.m_builtinEnvTexture);
   m_pin3d.m_pd3dPrimaryDevice->classicLightShader->SetTexture("Texture2", m_pd3dDevice->m_texMan.LoadTexture(m_envRadianceTexture));
#endif
   const vec4 st(m_ptable->m_envEmissionScale*m_globalEmissionScale, m_pin3d.m_envTexture ? (float)m_pin3d.m_envTexture->m_height/*+m_pin3d.m_envTexture->m_width)*0.5f*/ : (float)m_pin3d.m_builtinEnvTexture.m_height/*+m_pin3d.m_builtinEnvTexture.m_width)*0.5f*/, 0.f, 0.f);
   m_pin3d.m_pd3dPrimaryDevice->basicShader->SetVector("fenvEmissionScale_TexWidth", &st);
#ifdef SEPARATE_CLASSICLIGHTSHADER
   m_pin3d.m_pd3dPrimaryDevice->classicLightShader->SetVector("fenvEmissionScale_TexWidth", &st);
#endif

   InitBallShader();
}

void Player::UpdateBallShaderMatrix()
{
   D3DMATRIX worldMat;
   D3DMATRIX viewMat;
   D3DMATRIX projMat;
   m_pin3d.m_pd3dPrimaryDevice->GetTransform(TRANSFORMSTATE_WORLD, &worldMat);
   m_pin3d.m_pd3dPrimaryDevice->GetTransform(TRANSFORMSTATE_VIEW, &viewMat);
   m_pin3d.m_pd3dPrimaryDevice->GetTransform(TRANSFORMSTATE_PROJECTION, &projMat);

   const D3DXMATRIX matProj(projMat);
   const D3DXMATRIX matView(viewMat);
   const D3DXMATRIX matWorld(worldMat);

   const D3DXMATRIX matWorldView = matWorld * matView;
   D3DXMATRIX matWorldViewProj = matWorldView * matProj;

   if (m_ptable->m_tblMirrorEnabled)
   {
      const D3DXMATRIX flipx(-1, 0, 0, 0,
         0, 1, 0, 0,
         0, 0, 1, 0,
         0, 0, 0, 1);
      const D3DXMATRIX flipy(1, 0, 0, 0,
         0, -1, 0, 0,
         0, 0, 1, 0,
         0, 0, 0, 1);
      const float rotation = fmodf(m_ptable->m_BG_rotation[m_ptable->m_BG_current_set], 360.f);
      matWorldViewProj = matWorldViewProj * (rotation != 0.f ? flipy : flipx);
   }

   Matrix3D temp;
   memcpy(temp.m, matWorldView.m, 4 * 4 * sizeof(float));
   temp.Invert();
   D3DXMATRIX matWorldViewInv;
   memcpy(matWorldViewInv.m, temp.m, 4 * 4 * sizeof(float));
   //temp.Transpose();
   //D3DXMATRIX matWorldViewInvTrans;
   //memcpy(matWorldViewInvTrans.m, temp.m, 4 * 4 * sizeof(float));

   m_ballShader->SetMatrix("matWorldViewProj", &matWorldViewProj);
   m_ballShader->SetMatrix("matWorldView", &matWorldView);
   m_ballShader->SetMatrix("matWorldViewInverse", &matWorldViewInv);
   //m_ballShader->SetMatrix("matWorldViewInverseTranspose", &matWorldViewInvTrans);
   m_ballShader->SetMatrix("matView", &matView);

   //memcpy(temp.m, matView.m, 4 * 4 * sizeof(float));
   //temp.Transpose();
   //D3DXMATRIX matViewInvInvTrans;
   //memcpy(matViewInvInvTrans.m, temp.m, 4 * 4 * sizeof(float));

   //m_ballShader->SetMatrix("matViewInverseInverseTranspose", &matViewInvInvTrans);
}

void Player::InitBallShader()
{
   m_ballShader = new Shader(m_pin3d.m_pd3dPrimaryDevice);
   m_ballShader->Load(g_ballShaderCode, sizeof(g_ballShaderCode));

   UpdateBallShaderMatrix();

   //m_ballShader->SetBool("decalMode", m_ptable->m_BallDecalMode);
   const float rotation = fmodf(m_ptable->m_BG_rotation[m_ptable->m_BG_current_set], 360.f);
   m_cabinetMode = (rotation != 0.f);

   //m_ballShader->SetBool("cabMode", rotation != 0.f);

   //vec4 cam( matView._41, matView._42, matView._43, 1 );
   //m_ballShader->SetVector("camera", &cam);
   const vec4 st(m_ptable->m_envEmissionScale*m_globalEmissionScale, m_pin3d.m_envTexture ? (float)m_pin3d.m_envTexture->m_height/*+m_pin3d.m_envTexture->m_width)*0.5f*/ : (float)m_pin3d.m_builtinEnvTexture.m_height/*+m_pin3d.m_builtinEnvTexture.m_width)*0.5f*/, 0.f, 0.f);
   m_ballShader->SetVector("fenvEmissionScale_TexWidth", &st);
   //m_ballShader->SetInt("iLightPointNum",MAX_LIGHT_SOURCES);

   const float Roughness = 0.8f;
   const vec4 rwem(exp2f(10.0f * Roughness + 1.0f), 0.f, 1.f, 0.05f);
   m_ballShader->SetVector("Roughness_WrapL_Edge_Thickness", &rwem);

   Texture * const playfield = m_ptable->GetImage(m_ptable->m_szImage);
   if (playfield)
      m_ballShader->SetTexture("Texture1", playfield, false);

   m_ballShader->SetTexture("Texture2", m_pin3d.m_pd3dPrimaryDevice->m_texMan.LoadTexture(m_pin3d.m_envRadianceTexture, false));

   assert(m_ballIndexBuffer == NULL);
   const bool lowDetailBall = (m_ptable->GetDetailLevel() < 10);
   m_ballIndexBuffer = m_pin3d.m_pd3dPrimaryDevice->CreateAndFillIndexBuffer(lowDetailBall ? basicBallLoNumFaces : basicBallMidNumFaces, lowDetailBall ? basicBallLoIndices : basicBallMidIndices);

   // VB for normal ball
   assert(m_ballVertexBuffer == NULL);
   m_pin3d.m_pd3dPrimaryDevice->CreateVertexBuffer(lowDetailBall ? basicBallLoNumVertices : basicBallMidNumVertices, 0, MY_D3DFVF_NOTEX2_VERTEX, &m_ballVertexBuffer);

   // load precomputed ball vertices into vertex buffer
   Vertex3D_NoTex2 *buf;
   m_ballVertexBuffer->lock(0, 0, (void**)&buf, VertexBuffer::WRITEONLY);
   memcpy(buf, lowDetailBall ? basicBallLo : basicBallMid, sizeof(Vertex3D_NoTex2)*(lowDetailBall ? basicBallLoNumVertices : basicBallMidNumVertices));
   m_ballVertexBuffer->unlock();

   vec4 amb_lr = convertColor(m_ptable->m_lightAmbient, m_ptable->m_lightRange);
   amb_lr.x *= m_globalEmissionScale;
   amb_lr.y *= m_globalEmissionScale;
   amb_lr.z *= m_globalEmissionScale;
   m_ballShader->SetVector("cAmbient_LightRange", &amb_lr);
}

void Player::CreateDebugFont()
{
    int fontSize = 20;
    if (m_width > 1024 && m_width <= 1920)
        fontSize = 24;
    else if (m_width > 1920)
        fontSize = 30;

    const HRESULT hr = D3DXCreateFont(m_pin3d.m_pd3dPrimaryDevice->GetCoreDevice(), //device
                                fontSize,                              //font height
                                0,                                     //font width
                                FW_BOLD,                               //font weight
                                1,                                     //mip levels
                                fFalse,                                //italic
                                DEFAULT_CHARSET,                       //charset
                                OUT_DEFAULT_PRECIS,                    //output precision
                                DEFAULT_QUALITY,                       //quality
                                DEFAULT_PITCH | FF_DONTCARE,           //pitch and family
                                "Arial",                               //font name
                                &m_pFont);                             //font pointer
    if (FAILED(hr))
    {
        ShowError("Unable to create debug font via D3DXCreateFont!");
        m_pFont = NULL;
    }
    if (FAILED(D3DXCreateSprite(m_pin3d.m_pd3dPrimaryDevice->GetCoreDevice(), &m_fontSprite)))
        ShowError("D3DXCreateSprite failed!");

    SetRect(&m_fontRect, 0, 0, DBG_SPRITE_SIZE, DBG_SPRITE_SIZE);
}


void Player::SetDebugOutputPosition(const float x, const float y)
{
    const D3DXVECTOR2 spritePos(x,y);
    const D3DXVECTOR2 spriteCenter(DBG_SPRITE_SIZE/2, DBG_SPRITE_SIZE/2);

    const float angle = ANGTORAD(m_ptable->m_BG_rotation[m_ptable->m_BG_current_set]);
    D3DXMATRIX mat;
    D3DXMatrixTransformation2D(&mat, NULL, 0.0, NULL, &spriteCenter, angle, &spritePos);
    m_fontSprite->SetTransform(&mat);
}

void Player::DebugPrint(int x, int y, LPCSTR text, bool center /*= false*/)
{
   if(m_pFont)
   {
       int xx = x;
       m_fontSprite->Begin(D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_TEXTURE);
       RECT fontRect;
       SetRect(&fontRect, x, y, 0, 0);
       m_pFont->DrawText(m_fontSprite, text, -1, &fontRect, DT_CALCRECT, 0xFFFFFFFF);
       if (center)
           xx = x - (fontRect.right - fontRect.left) / 2;
       SetRect(&fontRect, xx, y, 0, 0);

       //if(shadow)
            for(unsigned int i = 0; i < 4; ++i)
            {
               const int offset = 1;
               RECT shadowRect;
               SetRect( &shadowRect, xx + ((i == 0) ? -offset : (i == 1) ? offset : 0), y + ((i == 2) ? -offset : (i == 3) ? offset : 0), 0, 0 );
               m_pFont->DrawText(m_fontSprite, text, -1, &shadowRect, DT_NOCLIP, 0xFF000000);
            }

      m_pFont->DrawText(m_fontSprite, text, -1, &fontRect, DT_NOCLIP, 0xFFFFFFFF);

      m_fontSprite->End();
   }
}

HRESULT Player::Init()
{
   TRACE_FUNCTION();

   set_lowest_possible_win_timer_resolution();

   //m_hSongCompletionEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

   m_ptable->m_progressDialog.SetProgress(10);
   m_ptable->m_progressDialog.SetName(std::string("Initializing Visuals..."));

   InitKeys();

   m_PlayMusic = LoadValueBoolWithDefault("Player", "PlayMusic", true);
   m_PlaySound = LoadValueBoolWithDefault("Player", "PlaySound", true);
   m_MusicVolume = LoadValueIntWithDefault("Player", "MusicVolume", 100);
   m_SoundVolume = LoadValueIntWithDefault("Player", "SoundVolume", 100);

   //
   const bool dynamicDayNight = LoadValueBoolWithDefault("Player", "DynamicDayNight", false);

   if(dynamicDayNight && !m_ptable->m_overwriteGlobalDayNight)
   {
       time_t hour_machine;
       time(&hour_machine);
       tm local_hour;
       localtime_s(&local_hour, &hour_machine);

       const float lat = LoadValueFloatWithDefault("Player", "Latitude", 52.52f);
       const float lon = LoadValueFloatWithDefault("Player", "Longitude", 13.37f);

       const double rlat = lat * (M_PI / 180.);
       const double rlong = lon * (M_PI / 180.);

       const double tr = TheoreticRadiation(local_hour.tm_mday, local_hour.tm_mon + 1, local_hour.tm_year + 1900, rlat);
       const double max_tr = MaxTheoreticRadiation(local_hour.tm_year + 1900, rlat);
       const double sset = SunsetSunriseLocalTime(local_hour.tm_mday, local_hour.tm_mon + 1, local_hour.tm_year + 1900, rlong, rlat, false);
       const double srise = SunsetSunriseLocalTime(local_hour.tm_mday, local_hour.tm_mon + 1, local_hour.tm_year + 1900, rlong, rlat, true);

       const double cur = local_hour.tm_hour + local_hour.tm_min / 60.0;

       const float factor = (float)(sin(M_PI* clamp((cur - srise) / (sset - srise), 0., 1.)) //!! leave space before sunrise and after sunset?
           * sqrt(tr / max_tr)); //!! magic, "emulates" that shorter days are usually also "darker",cloudier,whatever in most regions

       m_globalEmissionScale = clamp(factor, 0.15f, 1.f); //!! configurable clamp?
   }
   else
       m_globalEmissionScale = m_ptable->m_globalEmissionScale;

   //

   int vsync = (m_ptable->m_TableAdaptiveVSync == -1) ? m_VSync : m_ptable->m_TableAdaptiveVSync;

   const bool useAA = (m_AA && (m_ptable->m_useAA == -1)) || (m_ptable->m_useAA == 1);
   const unsigned int FXAA = (m_ptable->m_useFXAA == -1) ? m_FXAA : m_ptable->m_useFXAA;
   const bool ss_refl = (m_ss_refl && (m_ptable->m_useSSR == -1)) || (m_ptable->m_useSSR == 1);

   const int colordepth = LoadValueIntWithDefault("Player", "ColorDepth", 32);

   // colordepth & refreshrate are only defined if fullscreen is true.
   const HRESULT hr = m_pin3d.InitPin3D(m_fullScreen, m_width, m_height, colordepth,
                                        m_refreshrate, vsync, useAA, !!m_stereo3D, FXAA, !m_disableAO, ss_refl);

   if (hr != S_OK)
   {
      char szfoo[64];
      sprintf_s(szfoo, "InitPin3D Error code: %x", hr);
      ShowError(szfoo);
      return hr;
   }

   if (m_fullScreen)
      SetWindowPos(NULL, 0, 0, m_width, m_height, SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);

   m_pininput.Init(GetHwnd());

   //
   const unsigned int lflip = get_vk(m_rgKeys[eLeftFlipperKey]);
   const unsigned int rflip = get_vk(m_rgKeys[eRightFlipperKey]);

   if (((GetAsyncKeyState(VK_LSHIFT) & 0x8000) && (GetAsyncKeyState(VK_RSHIFT) & 0x8000))
      || ((lflip != ~0u) && (rflip != ~0u) && (GetAsyncKeyState(lflip) & 0x8000) && (GetAsyncKeyState(rflip) & 0x8000)))
   {
      m_ptable->m_tblMirrorEnabled = true;
   }
   else
   {
      m_ptable->m_tblMirrorEnabled = LoadValueBoolWithDefault("Player", "mirror", false);
   }
   m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_NONE); // re-init/thrash cache entry due to the hacky nature of the table mirroring
   m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_CCW);

   // if left flipper or shift hold during load, then swap DT/FS view (for quick testing)
   if (m_ptable->m_BG_current_set != 2 &&
       !m_ptable->m_tblMirrorEnabled &&
       ((GetAsyncKeyState(VK_LSHIFT) & 0x8000)
       || ((lflip != ~0u) && (GetAsyncKeyState(lflip) & 0x8000))))
   {
       m_toogle_DTFS = true;
       m_ptable->m_BG_current_set ^= 1;
   }
   else
       m_toogle_DTFS = false;

   m_pin3d.InitLayout(m_ptable->m_BG_enable_FSS);
#ifdef USE_IMGUI
   IMGUI_CHECKVERSION();
   ImGui::CreateContext();
   ImPlot::CreateContext();
   ImGuiIO& io = ImGui::GetIO();
   io.IniFilename = nullptr;  //don't use an ini file for configuration
   ImGui_ImplWin32_Init(GetHwnd());
   ImGui_ImplDX9_Init(m_pin3d.m_pd3dPrimaryDevice->GetCoreDevice());
#endif

   const float minSlope = (m_ptable->m_overridePhysics ? m_ptable->m_fOverrideMinSlope : m_ptable->m_angletiltMin);
   const float maxSlope = (m_ptable->m_overridePhysics ? m_ptable->m_fOverrideMaxSlope : m_ptable->m_angletiltMax);
   const float slope = minSlope + (maxSlope - minSlope) * m_ptable->m_globalDifficulty;

   m_gravity.x = 0.f;
   m_gravity.y =  sinf(ANGTORAD(slope))*(m_ptable->m_overridePhysics ? m_ptable->m_fOverrideGravityConstant : m_ptable->m_Gravity);
   m_gravity.z = -cosf(ANGTORAD(slope))*(m_ptable->m_overridePhysics ? m_ptable->m_fOverrideGravityConstant : m_ptable->m_Gravity);

   m_NudgeX = 0.f;
   m_NudgeY = 0.f;

   m_legacyNudgeTime = 0;

   m_legacyNudge = LoadValueBoolWithDefault("Player", "EnableLegacyNudge", false);
   m_legacyNudgeStrength = LoadValueFloatWithDefault("Player", "LegacyNudgeStrength", 1.f);

   m_legacyNudgeBackX = 0.f;
   m_legacyNudgeBackY = 0.f;

   m_movedPlunger = 0;

   Ball::ballID = 0;

   CreateDebugFont();
   m_ptable->m_progressDialog.SetProgress(30);
   m_ptable->m_progressDialog.SetName(std::string("Initializing Physics..."));

   // Initialize new nudging.
   m_tableVel.SetZero();
   m_tableDisplacement.SetZero();
   m_tableVelOld.SetZero();
   m_tableVelDelta.SetZero();

   // Table movement (displacement u) is modeled as a mass-spring-damper system
   //   u'' = -k u - c u'
   // with a spring constant k and a damping coefficient c.
   // See http://en.wikipedia.org/wiki/Damping#Linear_damping

   const float nudgeTime = m_ptable->m_nudgeTime;      // T
   const float dampingRatio = 0.5f;                    // zeta

   // time for one half period (one swing and swing back):
   //   T = pi / omega_d,
   // where
   //   omega_d = omega_0 * sqrt(1 - zeta^2)       (damped frequency)
   //   omega_0 = sqrt(k)                          (undamped frequency)
   // Solving for the spring constant k, we get
   m_nudgeSpring = (float)(M_PI*M_PI) / (nudgeTime*nudgeTime * (1.0f - dampingRatio*dampingRatio));

   // The formula for the damping ratio is
   //   zeta = c / (2 sqrt(k)).
   // Solving for the damping coefficient c, we get
   m_nudgeDamping = dampingRatio * 2.0f * sqrtf(m_nudgeSpring);


   // Need to set timecur here, for init functions that set timers
   m_time_msec = 0;

   InitFPS();
   m_showFPS = 0;

   g_pvp->ProfileLog("Hitables");

   for (size_t i = 0; i < m_ptable->m_vedit.size(); i++)
   {
      IEditable * const pe = m_ptable->m_vedit[i];
      Hitable * const ph = pe->GetIHitable();
      if (ph)
      {
#ifdef DEBUGPHYSICS
         if(pe->GetScriptable())
         {
            CComBSTR bstr;
            pe->GetScriptable()->get_Name(&bstr);
            char * bstr2 = MakeChar(bstr);
            CHAR wzDst[256];
            sprintf_s(wzDst, "Initializing Object-Physics %s...", bstr2);
            delete [] bstr2;
            m_ptable->m_progressDialog.SetName(std::string(bstr2));
         }
#endif
         const size_t currentsize = m_vho.size();
         ph->GetHitShapes(m_vho);
         const size_t newsize = m_vho.size();
         // Save the objects the trouble of having to set the idispatch pointer themselves
         for (size_t hitloop = currentsize; hitloop < newsize; hitloop++)
            m_vho[hitloop]->m_pfedebug = pe->GetIFireEvents();

         ph->GetTimers(m_vht);

         // build list of hitables
         m_vhitables.push_back(ph);

         // Adding objects to animation update list (slingshot is done below :/)
         if (pe->GetItemType() == eItemDispReel)
         {
             DispReel * const dispReel = (DispReel*)pe;
             m_vanimate.push_back(&dispReel->m_dispreelanim);
         } else
         if (pe->GetItemType() == eItemLightSeq)
         {
             LightSeq * const lightseq = (LightSeq*)pe;
             m_vanimate.push_back(&lightseq->m_lightseqanim);
         }
      }
   }

   m_ptable->m_progressDialog.SetProgress(45);
   m_ptable->m_progressDialog.SetName(std::string("Initializing Octree..."));

   g_pvp->ProfileLog("Octree");

   AddCabinetBoundingHitShapes();

   for (size_t i = 0; i < m_vho.size(); ++i)
   {
      HitObject * const pho = m_vho[i];

      pho->CalcHitBBox(); // maybe needed to update here, as only done lazily for some objects (i.e. balls!)
      m_hitoctree.AddElement(pho);

      if (pho->GetType() == eFlipper)
         m_vFlippers.push_back((HitFlipper*)pho);
      else if (pho->GetType() == eLineSegSlingshot) // Adding objects to animation update list, only slingshot! (dispreels and lightseqs are added above :/)
         m_vanimate.push_back(&((LineSegSlingshot*)pho)->m_slingshotanim);

      MoverObject * const pmo = pho->GetMoverObject();
      if (pmo && pmo->AddToList()) // Spinner, Gate, Flipper, Plunger (ball is added separately on each create ball)
         m_vmover.push_back(pmo);
   }

   FRect3D tableBounds = m_ptable->GetBoundingBox();
   m_hitoctree.Initialize(tableBounds);
#if !defined(NDEBUG) && defined(PRINT_DEBUG_COLLISION_TREE)
   m_hitoctree.DumpTree(0);
#endif

   // initialize hit structure for dynamic objects
   m_hitoctree_dynamic.FillFromVector(m_vho_dynamic);

   //----------------------------------------------------------------------------------

   m_ptable->m_progressDialog.SetProgress(60);
   m_ptable->m_progressDialog.SetName(std::string("Rendering Table..."));

   g_pvp->ProfileLog("Render Table");

   //g_viewDir = m_pin3d.m_viewVec;
   g_viewDir = Vertex3Ds(0, 0, -1.0f);

   InitShader();

   // search through all collection for elements which support group rendering
   for (int i = 0; i < m_ptable->m_vcollection.Size(); i++)
   {
      Collection * const pcol = m_ptable->m_vcollection.ElementAt(i);
      for (int t = 0; t < pcol->m_visel.size(); t++)
      {
         // search for a primitive in the group, if found try to create a grouped render element
         ISelect * const pisel = pcol->m_visel.ElementAt(t);
         if (pisel !=NULL && pisel->GetItemType() == eItemPrimitive)
         {
            Primitive * const prim = (Primitive*)pisel;
            prim->CreateRenderGroup(pcol);
            break;
         }
      }
   }

   // Pre-render all non-changing elements such as 
   // static walls, rails, backdrops, etc. and also static playfield reflections
   InitStatic();

   for (size_t i = 0; i < m_ptable->m_vedit.size(); ++i)
   {
      IEditable * const pe = m_ptable->m_vedit[i];
      Hitable * const ph = pe->GetIHitable();
      if (ph)
      {
         // sort into proper categories
         if (ph->IsTransparent())
            m_vHitTrans.push_back(ph);
         else
            m_vHitNonTrans.push_back(ph);
      }
   }

   material_flips = 0;
   unsigned long long m;
   if (m_vHitNonTrans.size() > 0)
   {
      std::stable_sort(m_vHitNonTrans.begin(), m_vHitNonTrans.end(), CompareHitableDepthReverse); // stable, so that em reels (=same depth) will keep user defined order
      std::stable_sort(m_vHitNonTrans.begin(), m_vHitNonTrans.end(), CompareHitableImage); // stable, so that objects with same images will keep depth order
      // sort by vertexbuffer not useful currently
      std::stable_sort(m_vHitNonTrans.begin(), m_vHitNonTrans.end(), CompareHitableMaterial); // stable, so that objects with same materials will keep image order

      m = m_vHitNonTrans[0]->GetMaterialID();
      for (size_t i = 1; i < m_vHitNonTrans.size(); ++i)
         if (m_vHitNonTrans[i]->GetMaterialID() != m)
         {
            material_flips++;
            m = m_vHitNonTrans[i]->GetMaterialID();
         }
   }

   if (m_vHitTrans.size() > 0)
   {
      std::stable_sort(m_vHitTrans.begin(), m_vHitTrans.end(), CompareHitableImage); // see above
      // sort by vertexbuffer not useful currently
      std::stable_sort(m_vHitTrans.begin(), m_vHitTrans.end(), CompareHitableMaterial);
      std::stable_sort(m_vHitTrans.begin(), m_vHitTrans.end(), CompareHitableDepth);

      m = m_vHitTrans[0]->GetMaterialID();
      for (size_t i = 1; i < m_vHitTrans.size(); ++i)
         if (m_vHitTrans[i]->GetMaterialID() != m)
         {
            material_flips++;
            m = m_vHitTrans[i]->GetMaterialID();
         }
   }
   // Direct all renders to the back buffer.
   m_pin3d.SetPrimaryRenderTarget(m_pin3d.m_pddsBackBuffer, m_pin3d.m_pddsZBuffer);

   m_ptable->m_progressDialog.SetProgress(90);


#ifdef DEBUG_BALL_SPIN
   {
      std::vector< Vertex3D_TexelOnly > ballDbgVtx;
      for (int j = -1; j <= 1; ++j)
      {
         const int numPts = (j == 0) ? 6 : 3;
         const float theta = (float)(j * (M_PI / 4.0));
         for (int i = 0; i < numPts; ++i)
         {
            const float phi = (float)(i * (2.0 * M_PI) / numPts);
            Vertex3D_TexelOnly vtx;
            vtx.x = 25.0f * cosf(theta) * cosf(phi);
            vtx.y = 25.0f * cosf(theta) * sinf(phi);
            vtx.z = 25.0f * sinf(theta);
            vtx.tu = 0.f;
            vtx.tv = 0.f;
            ballDbgVtx.push_back(vtx);
         }
      }

      assert(m_ballDebugPoints == NULL);
      m_pin3d.m_pd3dPrimaryDevice->CreateVertexBuffer((unsigned int)ballDbgVtx.size(), 0, MY_D3DFVF_TEX, &m_ballDebugPoints);
      void *buf;
      m_ballDebugPoints->lock(0, 0, &buf, VertexBuffer::WRITEONLY);
      memcpy(buf, ballDbgVtx.data(), ballDbgVtx.size() * sizeof(ballDbgVtx[0]));
      m_ballDebugPoints->unlock();
   }
#endif

   assert(m_ballTrailVertexBuffer == NULL);
   m_pin3d.m_pd3dPrimaryDevice->CreateVertexBuffer((MAX_BALL_TRAIL_POS-2)*2+4, USAGE_DYNAMIC, MY_D3DFVF_NOTEX2_VERTEX, &m_ballTrailVertexBuffer);

   m_ptable->m_pcv->Start(); // Hook up to events and start cranking script

   m_ptable->m_progressDialog.SetName(std::string("Starting Game Scripts..."));

   g_pvp->ProfileLog("Start Scripts");

   m_ptable->FireVoidEvent(DISPID_GameEvents_Init);

#ifdef LOG
   m_flog = fopen("c:\\log.txt","w");
#endif

#ifdef PLAYBACK
   if (m_playback)
      m_fplaylog = fopen("c:\\badlog.txt", "r");
#endif

   wintimer_init();

   m_StartTime_usec = usec();

   m_curPhysicsFrameTime = m_StartTime_usec;
   m_nextPhysicsFrameTime = m_curPhysicsFrameTime + PHYSICS_STEPTIME;

#ifdef PLAYBACK
   if (m_playback)
   {
      float physicsStepTime;
      ParseLog((LARGE_INTEGER*)&physicsStepTime, (LARGE_INTEGER*)&m_StartTime_usec);
   }
#endif

#ifdef LOG
   fprintf(m_flog, "Step Time %llu\n", m_StartTime_usec);
   fprintf(m_flog, "End Frame\n");
#endif

   m_ptable->m_progressDialog.SetProgress(100);
   m_ptable->m_progressDialog.SetName(std::string("Starting..."));

   g_pvp->GetPropertiesDocker()->EnableWindow(FALSE);
   g_pvp->GetLayersDocker()->EnableWindow(FALSE);
   g_pvp->GetToolbarDocker()->EnableWindow(FALSE);

   if(g_pvp->GetNotesDocker()!=nullptr)
      g_pvp->GetNotesDocker()->EnableWindow(FALSE);

   m_ptable->EnableWindow(FALSE);

   m_ptable->m_progressDialog.Destroy();

   // Show the window.
   ShowWindow(SW_SHOW);
   SetForegroundWindow();
   SetFocus();

   LockForegroundWindow(true);

   // Call Init -- TODO: what's the relation to ptable->FireVoidEvent() above?
   for (size_t i = 0; i < m_vhitables.size(); ++i)
   {
      Hitable * const ph = m_vhitables[i];
      if (ph->GetEventProxyBase())
         ph->GetEventProxyBase()->FireVoidEvent(DISPID_GameEvents_Init);
   }

   if (m_detectScriptHang)
      g_pvp->PostWorkToWorkerThread(HANG_SNOOP_START, NULL);

   // 0 means disable limiting of draw-ahead queue
   m_limiter.Init(m_pin3d.m_pd3dPrimaryDevice, m_maxPrerenderedFrames);

   Render(); //!! why here already? potentially not all initialized yet??

   // Broadcast a message to notify front-ends that it is 
   // time to reveal the playfield. 
   UINT nMsgID = RegisterWindowMessage(_T("VPTableStart"));
	::PostMessage(HWND_BROADCAST, nMsgID, NULL, NULL);
 
   return S_OK;
}

// reflection is split into two parts static and dynamic
// for the static objects:
//  1. switch to a temporary mirror texture/back buffer and a mirror z-buffer (e.g. the static z-buffer)
//  2. render the mirrored elements into these buffers
//
// for the dynamic objects:
//  1. use the previous mirror depthbuffer
//  2. switch to a temporary mirror texture and render all dynamic elements into that buffer 
//  3. switch back to normal back buffer
//  4. render the dynamic mirror texture over the scene
//  5. render all dynamic objects as normal
void Player::RenderStaticMirror(const bool onlyBalls)
{
   // Direct all renders to the temporary mirror buffer (plus the static z-buffer)
   m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget(m_pin3d.m_pd3dPrimaryDevice->GetMirrorTmpBufferTexture());
   m_pin3d.m_pd3dPrimaryDevice->Clear(0, NULL, clearType::TARGET, 0, 1.0f, 0L);

   if (!onlyBalls)
   {
      m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::CLIPPLANEENABLE, RenderDevice::PLANE0);

      D3DMATRIX viewMat;
      m_pin3d.m_pd3dPrimaryDevice->GetTransform(TRANSFORMSTATE_VIEW, &viewMat);
      // flip camera
      viewMat._33 = -viewMat._33;
      const float rotation = fmodf(m_ptable->m_BG_rotation[m_ptable->m_BG_current_set], 360.f);
      if (rotation != 0.0f)
         viewMat._31 = -viewMat._31;
      else
         viewMat._32 = -viewMat._32;
      m_pin3d.m_pd3dPrimaryDevice->SetTransform(TRANSFORMSTATE_VIEW, &viewMat);

      m_ptable->m_reflectionEnabled = true;
      m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_NONE); // re-init/thrash cache entry due to the hacky nature of the table mirroring
      m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_CCW);

      UpdateBasicShaderMatrix();

      // render mirrored static elements
      for (size_t i = 0; i < m_ptable->m_vedit.size(); i++)
      {
         if (m_ptable->m_vedit[i]->GetItemType() != eItemDecal)
         {
            Hitable * const ph = m_ptable->m_vedit[i]->GetIHitable();
            if (ph)
            {
               ph->RenderStatic();
            }
         }
      }

      m_pin3d.DisableAlphaBlend();
      m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::DEPTHBIAS, 0); //!! paranoia set of old state, remove as soon as sure that no other code still relies on that legacy set
      m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::ZWRITEENABLE, RenderDevice::RS_TRUE);
      m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::BLENDOP, RenderDevice::BLENDOP_ADD);
      //m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_CCW);

      m_ptable->m_reflectionEnabled = false;
      m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_NONE); // re-init/thrash cache entry due to the hacky nature of the table mirroring
      m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_CCW);

      // and flip back camera
      viewMat._33 = -viewMat._33;
      if (rotation != 0.0f)
         viewMat._31 = -viewMat._31;
      else
         viewMat._32 = -viewMat._32;
      m_pin3d.m_pd3dPrimaryDevice->SetTransform(TRANSFORMSTATE_VIEW, &viewMat);
      UpdateBasicShaderMatrix();

      m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::CLIPPLANEENABLE, 0); // disable playfield clipplane again
   }

   m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget(m_pin3d.m_pddsStatic);
}

void Player::RenderDynamicMirror(const bool onlyBalls)
{
   // render into temp mirror back buffer 
   m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget(m_pin3d.m_pd3dPrimaryDevice->GetMirrorTmpBufferTexture());
   m_pin3d.m_pd3dPrimaryDevice->Clear(0, NULL, clearType::TARGET, 0, 1.0f, 0L);

   D3DMATRIX viewMat;
   m_pin3d.m_pd3dPrimaryDevice->GetTransform(TRANSFORMSTATE_VIEW, &viewMat);
   // flip camera
   viewMat._33 = -viewMat._33;
   const float rotation = fmodf(m_ptable->m_BG_rotation[m_ptable->m_BG_current_set], 360.f);
   if (rotation != 0.0f)
      viewMat._31 = -viewMat._31;
   else
      viewMat._32 = -viewMat._32;
   m_pin3d.m_pd3dPrimaryDevice->SetTransform(TRANSFORMSTATE_VIEW, &viewMat);

   m_ptable->m_reflectionEnabled = true; // set to let matrices and postrenderstatics know that we need to handle reflections now
   m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_NONE); // re-init/thrash cache entry due to the hacky nature of the table mirroring
   m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_CCW);

   if (!onlyBalls)
      UpdateBasicShaderMatrix();

   UpdateBallShaderMatrix();

   if (!onlyBalls)
   {
      std::stable_sort(m_vHitTrans.begin(), m_vHitTrans.end(), CompareHitableDepthInverse);

      // Draw transparent objects.
      for (size_t i = 0; i < m_vHitTrans.size(); ++i)
         m_vHitTrans[i]->RenderDynamic();

      std::stable_sort(m_vHitTrans.begin(), m_vHitTrans.end(), CompareHitableDepth);
   }

   DrawBalls();

   if (!onlyBalls)
   {
      // Draw non-transparent objects.
      for (size_t i = 0; i < m_vHitNonTrans.size(); ++i)
         m_vHitNonTrans[i]->RenderDynamic();
   }

   m_pin3d.DisableAlphaBlend();
   m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::DEPTHBIAS, 0); //!! paranoia set of old state, remove as soon as sure that no other code still relies on that legacy set
   m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::ZWRITEENABLE, RenderDevice::RS_TRUE);
   m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::BLENDOP, RenderDevice::BLENDOP_ADD);
   //m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_CCW);

   m_ptable->m_reflectionEnabled = false;
   m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_NONE); // re-init/thrash cache entry due to the hacky nature of the table mirroring
   m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_CCW);

   // and flip back camera
   viewMat._33 = -viewMat._33;
   if (rotation != 0.0f)
      viewMat._31 = -viewMat._31;
   else
      viewMat._32 = -viewMat._32;
   m_pin3d.m_pd3dPrimaryDevice->SetTransform(TRANSFORMSTATE_VIEW, &viewMat);

   if (!onlyBalls)
      UpdateBasicShaderMatrix();

   UpdateBallShaderMatrix();

   m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget(m_pin3d.m_pddsBackBuffer);
}

void Player::RenderMirrorOverlay()
{
   // render the mirrored texture over the playfield
   m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture("Texture0", m_pin3d.m_pd3dPrimaryDevice->GetMirrorTmpBufferTexture());
   m_pin3d.m_pd3dPrimaryDevice->FBShader->SetFloat("mirrorFactor", m_ptable->m_playfieldReflectionStrength);
   m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTechnique("fb_mirror");

   m_pin3d.EnableAlphaBlend(false, false);
   m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::DESTBLEND, RenderDevice::DST_ALPHA);
   // z-test must be enabled otherwise mirrored elements are drawn over blocking elements
   m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_NONE);
   m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::ZWRITEENABLE, RenderDevice::RS_FALSE);

   m_pin3d.m_pd3dPrimaryDevice->FBShader->Begin(0);
   m_pin3d.m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad();
   m_pin3d.m_pd3dPrimaryDevice->FBShader->End();

   m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::ZWRITEENABLE, RenderDevice::RS_TRUE);
   m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_CCW);
   m_pin3d.DisableAlphaBlend();
}

void Player::InitStatic()
{
   TRACE_FUNCTION();

   // Start the frame.
   for (size_t i = 0; i < m_vhitables.size(); ++i)
   {
      Hitable * const ph = m_vhitables[i];
      ph->RenderSetup();
   }

   m_pin3d.InitPlayfieldGraphics();

   // allocate system/CPU memory buffer to copy static rendering buffer to (and accumulation float32 buffer) to do brute force oversampling of the static rendering
   D3DSURFACE_DESC descStatic;
   m_pin3d.m_pddsStatic->GetDesc(&descStatic);
   RECT rectStatic;
   rectStatic.left = 0;
   rectStatic.right = descStatic.Width;
   rectStatic.top = 0;
   rectStatic.bottom = descStatic.Height;

   float * __restrict const pdestStatic = new float[descStatic.Width*descStatic.Height * 3]; // RGB float32
   memset(pdestStatic, 0, descStatic.Width*descStatic.Height * 3 * sizeof(float));

   RenderTarget* offscreenSurface;
   CHECKD3D(m_pin3d.m_pd3dPrimaryDevice->GetCoreDevice()->CreateOffscreenPlainSurface(descStatic.Width, descStatic.Height, descStatic.Format, (D3DPOOL)memoryPool::SYSTEM, &offscreenSurface, NULL));

   // if rendering static/with heavy oversampling, disable the aniso/trilinear filter to get a sharper/more precise result overall!
   if (!m_cameraMode)
   {
      m_isRenderingStatic = true;
      // set up the texture filter again, so that this is triggered correctly
      m_pin3d.m_pd3dPrimaryDevice->SetTextureFilter(0, TEXTURE_MODE_TRILINEAR);
      m_pin3d.m_pd3dPrimaryDevice->SetTextureFilter(4, TEXTURE_MODE_TRILINEAR);
   }

//#define STATIC_PRERENDER_ITERATIONS_KOROBOV 7.0 // for the (commented out) lattice-based QMC oversampling, 'magic factor', depending on the the number of iterations!
   // loop for X times and accumulate/average these renderings on CPU side
   // NOTE: iter == 0 MUST ALWAYS PRODUCE an offset of 0,0!
   for (int iter = m_cameraMode ? 0 : (STATIC_PRERENDER_ITERATIONS-1); iter >= 0; --iter) // just do one iteration if in dynamic camera/light/material tweaking mode
   {
   m_pin3d.m_pd3dPrimaryDevice->m_stats_drawn_triangles = 0;

   float u1 = xyLDBNbnot[iter*2  ];  //      (float)iter*(float)(1.0                                /STATIC_PRERENDER_ITERATIONS);
   float u2 = xyLDBNbnot[iter*2+1];  //fmodf((float)iter*(float)(STATIC_PRERENDER_ITERATIONS_KOROBOV/STATIC_PRERENDER_ITERATIONS), 1.f);
   // the following line implements filter importance sampling for a small gauss (i.e. less jaggies as it also samples neighboring pixels) -> but also potentially more artifacts in compositing!
   gaussianDistribution(u1, u2, 0.5f, 0.5f); //!! first 0.5 could be increased for more blur, but is pretty much what is recommended
   // sanity check to be sure to limit filter area to 3x3 in practice, as the gauss transformation is unbound (which is correct, but for our use-case/limited amount of samples very bad)
   assert(u1 > -1.f && u1 < 2.f);
   assert(u2 > -1.f && u2 < 2.f);
   // Last iteration MUST set a sample offset of 0,0 so that final depth buffer features 'correctly' centered pixel sample
   if (iter == 0)
      assert(u1 == 0.5f && u2 == 0.5f);

   // Setup Camera,etc matrices for each iteration. 
   m_pin3d.InitLayout(m_ptable->m_BG_enable_FSS, u1 - 0.5f, u2 - 0.5f);

   // Now begin rendering of static buffer
   m_pin3d.m_pd3dPrimaryDevice->BeginScene();

   // Direct all renders to the "static" buffer
   m_pin3d.SetPrimaryRenderTarget(m_pin3d.m_pddsStatic, m_pin3d.m_pddsStaticZ);

   m_pin3d.DrawBackground();

   // perform render setup and give elements a chance to render before the playfield
   for (size_t i = 0; i < m_vhitables.size(); ++i)
   {
      Hitable * const ph = m_vhitables[i];
      ph->PreRenderStatic(m_pin3d.m_pd3dPrimaryDevice);
   }

   // Initialize one User Clipplane to be the playfield (but not enabled yet)
   SetClipPlanePlayfield(true);

   if (!m_cameraMode)
   {
      const bool drawBallReflection = ((m_reflectionForBalls && (m_ptable->m_useReflectionForBalls == -1)) || (m_ptable->m_useReflectionForBalls == 1));
      if (!(m_ptable->m_reflectElementsOnPlayfield /*&& m_pf_refl*/) && drawBallReflection)
         RenderStaticMirror(true);
      else
         if (m_ptable->m_reflectElementsOnPlayfield /*&& m_pf_refl*/)
            RenderStaticMirror(false);

      // exclude playfield depth as dynamic mirror objects have to be added later-on
      m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::ZWRITEENABLE, RenderDevice::RS_FALSE);
      m_pin3d.RenderPlayfieldGraphics(false);
      m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::ZWRITEENABLE, RenderDevice::RS_TRUE);

      if (m_ptable->m_reflectElementsOnPlayfield /*&& m_pf_refl*/)
         RenderMirrorOverlay();

      // to compensate for this when rendering the static objects, enable clipplane
      SetClipPlanePlayfield(false);
      m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::CLIPPLANEENABLE, RenderDevice::PLANE0);

      // now render everything else
      for (size_t i = 0; i < m_ptable->m_vedit.size(); i++)
      {
         if (m_ptable->m_vedit[i]->GetItemType() != eItemDecal)
         {
            Hitable * const ph = m_ptable->m_vedit[i]->GetIHitable();
            if (ph)
            {
               ph->RenderStatic();
               if (((i % 16) == 0) && iter == 0)
                   m_ptable->m_progressDialog.SetProgress(60 + ((15 * (int)i) / (int)m_ptable->m_vedit.size()));
            }
         }
      }

      // Draw decals (they have transparency, so they have to be drawn after the wall they are on)
      for (size_t i = 0; i < m_ptable->m_vedit.size(); i++)
      {
         if (m_ptable->m_vedit[i]->GetItemType() == eItemDecal)
         {
            Hitable * const ph = m_ptable->m_vedit[i]->GetIHitable();
            if (ph)
            {
               ph->RenderStatic();
               if (((i % 16) == 0) && iter == 0)
                  m_ptable->m_progressDialog.SetProgress(75 + ((15 * (int)i) / (int)m_ptable->m_vedit.size()));
            }
         }
      }

      m_pin3d.DisableAlphaBlend();
      m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::DEPTHBIAS, 0); //!! paranoia set of old state, remove as soon as sure that no other code still relies on that legacy set
      m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::ZWRITEENABLE, RenderDevice::RS_TRUE);
      m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::BLENDOP, RenderDevice::BLENDOP_ADD);
      m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_CCW);

      m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::CLIPPLANEENABLE, 0);
      SetClipPlanePlayfield(true);
   }

   // Finish the frame.
   m_pin3d.m_pd3dPrimaryDevice->EndScene();

   // Readback static buffer, convert 16bit to 32bit float, and accumulate
   if (!m_cameraMode)
   {
   CHECKD3D(m_pin3d.m_pd3dPrimaryDevice->GetCoreDevice()->GetRenderTargetData(m_pin3d.m_pddsStatic, offscreenSurface));

   D3DLOCKED_RECT locked;
   CHECKD3D(offscreenSurface->LockRect(&locked, &rectStatic, D3DLOCK_READONLY));

   const unsigned short * __restrict const psrc = (unsigned short*)locked.pBits;
   for (unsigned int y = 0; y < descStatic.Height; ++y)
   {
      unsigned int ofs0 = y*descStatic.Width*3;
      unsigned int ofs1 = y*locked.Pitch/2;
      if (descStatic.Format == (D3DFORMAT)colorFormat::RGBA16F)
      {
      for (unsigned int x = 0; x < descStatic.Width; ++x,ofs0+=3,ofs1+=4)
      {
         pdestStatic[ofs0  ] += half2float(psrc[ofs1  ]);
         pdestStatic[ofs0+1] += half2float(psrc[ofs1+1]);
         pdestStatic[ofs0+2] += half2float(psrc[ofs1+2]);
      }
      }
      else if (descStatic.Format == (D3DFORMAT)colorFormat::RED16F)
      {
      for (unsigned int x = 0; x < descStatic.Width; ++x,++ofs0,++ofs1)
         pdestStatic[ofs0] += half2float(psrc[ofs1]);
      }
      else if (descStatic.Format == (D3DFORMAT)colorFormat::RG16F)
      {
      for (unsigned int x = 0; x < descStatic.Width; ++x,ofs0+=2,ofs1+=2)
      {
         pdestStatic[ofs0  ] += half2float(psrc[ofs1  ]);
         pdestStatic[ofs0+1] += half2float(psrc[ofs1+1]);
      }
      }
   }

   offscreenSurface->UnlockRect();
   }
   stats_drawn_static_triangles = m_pin3d.m_pd3dPrimaryDevice->m_stats_drawn_triangles;
   }

   // if rendering static/with heavy oversampling, re-enable the aniso/trilinear filter now for the normal rendering
   if (!m_cameraMode)
   {
      m_isRenderingStatic = false;
      m_pin3d.m_pd3dPrimaryDevice->SetTextureFilter(0, TEXTURE_MODE_TRILINEAR);
      m_pin3d.m_pd3dPrimaryDevice->SetTextureFilter(4, TEXTURE_MODE_TRILINEAR);
   }

   // now normalize oversampled result in pdestStatic, convert back to 16bit float, and copy to/overwrite the static GPU buffer
   if (!m_cameraMode)
   {
   D3DLOCKED_RECT locked;
   CHECKD3D(offscreenSurface->LockRect(&locked, &rectStatic, D3DLOCK_DISCARD));

   unsigned short * __restrict const psrc = (unsigned short*)locked.pBits;
   for (unsigned int y = 0; y < descStatic.Height; ++y)
   {
      unsigned int ofs0 = y*descStatic.Width*3;
      unsigned int ofs1 = y*locked.Pitch/2;
      if (descStatic.Format == (D3DFORMAT)colorFormat::RGBA16F)
      {
      for (unsigned int x = 0; x < descStatic.Width; ++x,ofs0+=3,ofs1+=4)
      {
         psrc[ofs1  ] = float2half(pdestStatic[ofs0  ]*(float)(1.0/STATIC_PRERENDER_ITERATIONS));
         psrc[ofs1+1] = float2half(pdestStatic[ofs0+1]*(float)(1.0/STATIC_PRERENDER_ITERATIONS));
         psrc[ofs1+2] = float2half(pdestStatic[ofs0+2]*(float)(1.0/STATIC_PRERENDER_ITERATIONS));
      }
      }
      else if (descStatic.Format == (D3DFORMAT)colorFormat::RED16F)
      {
      for (unsigned int x = 0; x < descStatic.Width; ++x,++ofs0,++ofs1)
         psrc[ofs1] = float2half(pdestStatic[ofs0]*(float)(1.0/STATIC_PRERENDER_ITERATIONS));
      }
      else if (descStatic.Format == (D3DFORMAT)colorFormat::RG16F)
      {
      for (unsigned int x = 0; x < descStatic.Width; ++x,ofs0+=2,ofs1+=2)
      {
         psrc[ofs1  ] = float2half(pdestStatic[ofs0  ]*(float)(1.0/STATIC_PRERENDER_ITERATIONS));
         psrc[ofs1+1] = float2half(pdestStatic[ofs0+1]*(float)(1.0/STATIC_PRERENDER_ITERATIONS));
      }
      }
   }

   offscreenSurface->UnlockRect();

   CHECKD3D(m_pin3d.m_pd3dPrimaryDevice->GetCoreDevice()->UpdateSurface(offscreenSurface, NULL, m_pin3d.m_pddsStatic, NULL));
   }

   delete [] pdestStatic;
   SAFE_RELEASE(offscreenSurface);

   // Now finalize static buffer with non-dynamic AO

   // Dynamic AO disabled? -> Pre-Render Static AO
   const bool useAO = ((m_dynamicAO && (m_ptable->m_useAO == -1)) || (m_ptable->m_useAO == 1));
   if (!m_disableAO && !useAO && m_pin3d.m_pd3dPrimaryDevice->DepthBufferReadBackAvailable() && (m_ptable->m_AOScale > 0.f))
   {
      const bool useAA = (m_AA && (m_ptable->m_useAA == -1)) || (m_ptable->m_useAA == 1);

      m_pin3d.m_pd3dPrimaryDevice->CopySurface(m_pin3d.m_pddsZBuffer, m_pin3d.m_pddsStaticZ); // cannot be called inside BeginScene -> EndScene cycle

      m_pin3d.m_pd3dPrimaryDevice->BeginScene();
      m_pin3d.RenderPlayfieldGraphics(true); // mirror depth buffer only contained static objects, but no playfield yet -> so render depth only to add this
      m_pin3d.m_pd3dPrimaryDevice->EndScene();

      m_pin3d.m_pd3dPrimaryDevice->CopyDepth(m_pin3d.m_pdds3DZBuffer, m_pin3d.m_pddsStaticZ); // do not put inside BeginScene/EndScene Block
      m_pin3d.m_pd3dPrimaryDevice->CopySurface(m_pin3d.m_pd3dPrimaryDevice->GetBackBufferTexture(), m_pin3d.m_pddsStatic);
      m_pin3d.m_pd3dPrimaryDevice->CopySurface(m_pin3d.m_pddsStaticZ, m_pin3d.m_pddsZBuffer); // cannot be called inside BeginScene -> EndScene cycle

      m_pin3d.m_pd3dPrimaryDevice->BeginScene();

      m_pin3d.DisableAlphaBlend();
      m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_NONE);
      m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::ZWRITEENABLE, RenderDevice::RS_FALSE);
      m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::ZENABLE, RenderDevice::RS_FALSE);

      m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget(m_pin3d.m_pddsAOBackTmpBuffer);
      m_pin3d.m_pd3dPrimaryDevice->Clear(0, NULL, clearType::TARGET, 0, 1.0f, 0L);

      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture("Texture3", m_pin3d.m_pdds3DZBuffer);
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture("Texture4", &m_pin3d.m_aoDitherTexture, true);
      const vec4 ao_s_tb(m_ptable->m_AOScale, 0.1f, 0.f,0.f);
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetVector("AO_scale_timeblur", &ao_s_tb);
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTechnique("AO");

      for (unsigned int i = 0; i < 50; ++i) // 50 iterations to get AO smooth
      {
         if (i != 0)
            m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget(m_pin3d.m_pddsAOBackTmpBuffer);

         m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture("Texture0", m_pin3d.m_pddsAOBackBuffer); //!! ?

         const vec4 w_h_height((float)(1.0 / (double)m_width), (float)(1.0 / (double)m_height),
            radical_inverse(i)*(float)(1. / 8.0),
             /*sobol*/radical_inverse<3>(i)*(float)(1. / 8.0)); // jitter within (64/8)x(64/8) neighborhood of 64x64 tex, good compromise between blotches and noise
         m_pin3d.m_pd3dPrimaryDevice->FBShader->SetVector("w_h_height", &w_h_height);

         m_pin3d.m_pd3dPrimaryDevice->FBShader->Begin(0);
         m_pin3d.m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad();
         m_pin3d.m_pd3dPrimaryDevice->FBShader->End();

         // flip AO buffers (avoids copy)
         D3DTexture *tmpAO = m_pin3d.m_pddsAOBackBuffer;
         m_pin3d.m_pddsAOBackBuffer = m_pin3d.m_pddsAOBackTmpBuffer;
         m_pin3d.m_pddsAOBackTmpBuffer = tmpAO;
      }

      m_pin3d.SetPrimaryRenderTarget(m_pin3d.m_pddsStatic, m_pin3d.m_pddsStaticZ);

      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture("Texture0", m_pin3d.m_pd3dPrimaryDevice->GetBackBufferTexture());
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture("Texture3", m_pin3d.m_pddsAOBackBuffer);

      const vec4 fb_inv_resolution_05((float)(0.5 / (double)m_width), (float)(0.5 / (double)m_height), 1.0f, 1.0f);
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetVector("w_h_height", &fb_inv_resolution_05);
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTechnique(useAA ? "fb_tonemap_AO_static" : "fb_tonemap_AO_no_filter_static");

      m_pin3d.m_pd3dPrimaryDevice->FBShader->Begin(0);
      m_pin3d.m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad();
      m_pin3d.m_pd3dPrimaryDevice->FBShader->End();

      m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::ZENABLE, RenderDevice::RS_TRUE);
      m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::ZWRITEENABLE, RenderDevice::RS_TRUE);
      m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_CCW);

      m_pin3d.m_pd3dPrimaryDevice->EndScene();
   }
}

Ball *Player::CreateBall(const float x, const float y, const float z, const float vx, const float vy, const float vz, const float radius, const float mass)
{
   Ball * const pball = new Ball();
   pball->m_d.m_radius = radius;
   pball->m_d.m_pos.x = x;
   pball->m_d.m_pos.y = y;
   pball->m_d.m_pos.z = z + pball->m_d.m_radius;
   pball->m_d.m_vel.x = vx;
   pball->m_d.m_vel.y = vy;
   pball->m_d.m_vel.z = vz;
   pball->m_bulb_intensity_scale = m_ptable->m_defaultBulbIntensityScaleOnBall;

   pball->Init(mass); // Call this after radius set to get proper inertial tensor set up

   pball->EnsureOMObject();

   pball->m_pfedebug = (IFireEvents *)pball->m_pballex;

   m_vball.push_back(pball);
   m_vmover.push_back(&pball->m_mover); // balls are always added separately to this list!

   pball->CalcHitBBox(); // need to update here, as only done lazily

   m_vho_dynamic.push_back(pball);
   m_hitoctree_dynamic.FillFromVector(m_vho_dynamic);

   if (!m_pactiveballDebug)
      m_pactiveballDebug = pball;

   return pball;
}

void Player::DestroyBall(Ball *pball)
{
   if (!pball) return;

   bool activeball;
   if (m_pactiveball == pball)
   {
      activeball = true;
      m_pactiveball = NULL;
   }
   else
      activeball = false;

   bool debugball;
   if (m_pactiveballDebug == pball)
   {
	   debugball = true;
	   m_pactiveballDebug = NULL;
   }
   else
	   debugball = false;

   if (m_pactiveballBC == pball)
	   m_pactiveballBC = NULL;

   if (pball->m_pballex)
   {
      pball->m_pballex->m_pball = NULL;
      pball->m_pballex->Release();
   }

   RemoveFromVectorSingle(m_vball, pball);
   RemoveFromVectorSingle<MoverObject*>(m_vmover, &pball->m_mover);
   RemoveFromVectorSingle<HitObject*>(m_vho_dynamic, pball);

   m_hitoctree_dynamic.FillFromVector(m_vho_dynamic);

   m_vballDelete.push_back(pball);

   if (debugball && !m_vball.empty())
      m_pactiveballDebug = m_vball.front();
   if (activeball && !m_vball.empty())
      m_pactiveball = m_vball.front();
}


void Player::CalcBallAspectRatio()
{
   const int ballStretchMode = LoadValueIntWithDefault("Player", "BallStretchMode", 0);

   // Monitors: 4:3, 16:9, 16:10, 21:10, 21:9
   //const int ballStretchMonitor = LoadValueIntWithDefault("Player", "BallStretchMonitor", 1); // assume 16:9
   const float ballAspecRatioOffsetX = LoadValueFloatWithDefault("Player", "BallCorrectionX", 0.f);
   const float ballAspecRatioOffsetY = LoadValueFloatWithDefault("Player", "BallCorrectionY", 0.f);

   const float scalebackX = (m_ptable->m_BG_scalex[m_ptable->m_BG_current_set] != 0.0f) ? ((m_ptable->m_BG_scalex[m_ptable->m_BG_current_set] + m_ptable->m_BG_scaley[m_ptable->m_BG_current_set])*0.5f) / m_ptable->m_BG_scalex[m_ptable->m_BG_current_set] : 1.0f;
   const float scalebackY = (m_ptable->m_BG_scaley[m_ptable->m_BG_current_set] != 0.0f) ? ((m_ptable->m_BG_scalex[m_ptable->m_BG_current_set] + m_ptable->m_BG_scaley[m_ptable->m_BG_current_set])*0.5f) / m_ptable->m_BG_scaley[m_ptable->m_BG_current_set] : 1.0f;
   double xMonitor = 16.0;
   double yMonitor = 9.0;

   const double aspect = (double)m_screenwidth / (double)m_screenheight;
   double factor = aspect*3.0;
   if (factor > 4.0)
   {
      factor = aspect*9.0;
      if ((int)(factor+0.5) == 16)
      {
         //16:9
         xMonitor = (16.0 + ballAspecRatioOffsetX) / 4.0;
         yMonitor = ( 9.0 + ballAspecRatioOffsetY) / 3.0;
      }
      else if ((int)(factor+0.5) == 21)
      {
         //21:9
         xMonitor = (21.0 + ballAspecRatioOffsetX) / 4.0;
         yMonitor = ( 9.0 + ballAspecRatioOffsetY) / 3.0;
      }
      else
      {
         factor = aspect*10.0;
         if ((int)(factor+0.5) == 16)
         {
            //16:10
            xMonitor = (16.0 + ballAspecRatioOffsetX) / 4.0;
            yMonitor = (10.0 + ballAspecRatioOffsetY) / 3.0;
         }
         else
         {
            //21:10
            xMonitor = (factor + ballAspecRatioOffsetX) / 4.0;
            yMonitor = (10.0   + ballAspecRatioOffsetY) / 3.0;
         }
      }
   }
   else
   {
      //4:3
      xMonitor = (factor + ballAspecRatioOffsetX) / 4.0;
      yMonitor = (3.0    + ballAspecRatioOffsetY) / 3.0;
   }

   /* legacy
   switch (ballStretchMonitor)
   {
       case 0:
       xMonitor = (4.0 / 4.0);
       yMonitor = (3.0 / 3.0);
       break;
       case 1:
       xMonitor = (16.0 / 4.0);
       yMonitor = (9.0 / 3.0);
       break;
       case 2:
       xMonitor = (16.0 / 4.0);
       yMonitor = (10.0 / 3.0);
       break;
       case 3:
       xMonitor = (21.0 / 4.0);
       yMonitor = (10.0 / 3.0);
       break;
       case 4:
       xMonitor = (3.0 / 4.0);
       yMonitor = (4.0 / 3.0);
       break;
       case 5:
       xMonitor = (9.0 / 4.0);
       yMonitor = (16.0 / 3.0);
       break;
       case 6:
       xMonitor = (10.0 / 4.0);
       yMonitor = (16.0 / 3.0);
       break;
       case 7:
       xMonitor = (10.0 / 4.0);
       yMonitor = (21.0 / 3.0);
       break;
       case 8:
       xMonitor = (9.0 / 4.0);
       yMonitor = (21.0 / 3.0);
       break;
       case 9:
       xMonitor = (21.0 / 4.0);
       yMonitor = (9.0 / 3.0);
       break;
       default:
       xMonitor = 16.0f;
       yMonitor = 9.0f;
       break;
   }
   xMonitor += -0.4f;
   yMonitor += 0.0f;
   */

   const double scalebackMonitorX = (xMonitor + yMonitor)*0.5 / xMonitor;
   const double scalebackMonitorY = (xMonitor + yMonitor)*0.5 / yMonitor;

   float temprotation = m_ptable->m_BG_rotation[m_ptable->m_BG_current_set];
   while (temprotation < 0.f)
      temprotation += 360.0f;

   const float c = sinf(ANGTORAD(fmodf(temprotation + 90.0f, 180.0f)));
   const float s = sinf(ANGTORAD(fmodf(temprotation, 180.0f)));
   m_antiStretchBall = false;

   switch (ballStretchMode)
   {
   case 0:
      m_BallStretchX = 1.0f;
      m_BallStretchY = 1.0f;
      break;
   case 1:
      m_BallStretchX = scalebackX*c + scalebackY*s;
      m_BallStretchY = scalebackY*c + scalebackX*s;
      break;
   case 2:
      m_BallStretchX = scalebackX*c + scalebackY*s;
      m_BallStretchY = scalebackY*c + scalebackX*s;
      if (m_fullScreen || (m_width == m_screenwidth && m_height == m_screenheight)) // detect windowed fullscreen
      {
         m_antiStretchBall = true;
         m_BallStretchX *= (float)(scalebackMonitorX*c + scalebackMonitorY*s);
         m_BallStretchY *= (float)(scalebackMonitorY*c + scalebackMonitorX*s);
      }
      break;
   }
}

void Player::NudgeX(const int x, const int j)
{
   int v = x;
   if (x > m_ptable->m_tblAccelMaxX) v = m_ptable->m_tblAccelMaxX;
   if (x < -m_ptable->m_tblAccelMaxX) v = -m_ptable->m_tblAccelMaxX;
   m_curAccel_x[j] = v;
}

void Player::NudgeY(const int y, const int j)
{
   int v = y;
   if (y > m_ptable->m_tblAccelMaxY) v = m_ptable->m_tblAccelMaxY;
   if (y < -m_ptable->m_tblAccelMaxY) v = -m_ptable->m_tblAccelMaxY;
   m_curAccel_y[j] = v;
}

#define GetNudgeX() (((F32)m_curAccel_x[0]) * (F32)(2.0 / JOYRANGE)) // Get the -2 .. 2 values from joystick input tilt sensor / ushock //!! why 2?
#define GetNudgeY() (((F32)m_curAccel_y[0]) * (F32)(2.0 / JOYRANGE))

#ifdef UNUSED_TILT
int Player::NudgeGetTilt()
{
   static U32 last_tilt_time;
   static U32 last_jolt_time;

   if(!m_ptable->m_tblAccelerometer || m_NudgeManual >= 0 ||                 //disabled or in joystick test mode
       m_ptable->m_tilt_amount == 0 || m_ptable->m_jolt_amount == 0) return 0; //disabled

   const U32 ms = msec();

   U32 tilt_2 = 0;
   for(int j = 0; j < m_pininput.e_JoyCnt; ++j)    //find largest value
   {
      tilt_2 = max(tilt_2, (U32)(m_curAccel_x[j] * m_curAccel_x[j] + m_curAccel_y[j] * m_curAccel_y[j])); //always postive numbers
   }

   if( ( ms - last_jolt_time > m_ptable->m_jolt_trigger_time ) &&
      ( ms - last_tilt_time > (U32)m_ptable->m_tilt_trigger_time ) &&
      tilt_2 > ( (U32)m_ptable->m_tilt_amount * (U32)m_ptable->m_tilt_amount ) )
   {
      last_tilt_time = ms;

      return 1;
   }

   if( ms - last_jolt_time > (U32)m_ptable->m_jolt_trigger_time && 
      tilt_2 > ( (U32)m_ptable->m_jolt_amount * (U32)m_ptable->m_jolt_amount ) )
   {
      last_jolt_time = ms;
   }

   return 0;
}
#endif

void Player::NudgeUpdate()      // called on every integral physics frame
{
   m_NudgeX = 0.f;   // accumulate over joysticks, these acceleration values are used in update ball velocity calculations
   m_NudgeY = 0.f;   // and are required to be acceleration values (not velocity or displacement)

   if (!m_ptable->m_tblAccelerometer) return;       // electronic accelerometer disabled 

   //rotate to match hardware mounting orentation, including left or right coordinates
   const float a = ANGTORAD(m_ptable->m_tblAccelAngle);
   const float cna = cosf(a);
   const float sna = sinf(a);

   for (int j = 0; j < m_pininput.e_JoyCnt; ++j)
   {
      float dx = ((float)m_curAccel_x[j])*(float)(1.0 / JOYRANGE);              // norm range -1 .. 1   
      const float dy = ((float)m_curAccel_y[j])*(float)(1.0 / JOYRANGE);
      if (m_ptable->m_tblMirrorEnabled)
         dx = -dx;
      m_NudgeX += m_ptable->m_tblAccelAmpX * (dx*cna + dy*sna) * (1.0f - nudge_get_sensitivity());        // calc Green's transform component for X
      const float nugY = m_ptable->m_tblAccelAmpY * (dy*cna - dx*sna) * (1.0f - nudge_get_sensitivity()); // calc Green's transform component for Y
      m_NudgeY = m_ptable->m_tblAccelNormalMount ? (m_NudgeY + nugY) : (m_NudgeY - nugY);                 // add as left or right hand coordinate system
   }
}

#define IIR_Order 4

// coefficients for IIR_Order Butterworth filter set to 10 Hz passband
const float IIR_a[IIR_Order + 1] = {
   0.0048243445f,
   0.019297378f,
   0.028946068f,
   0.019297378f,
   0.0048243445f };

const float IIR_b[IIR_Order + 1] = {
   1.00000000f, //if not 1 add division below
   -2.369513f,
   2.3139884f,
   -1.0546654f,
   0.1873795f };

void Player::MechPlungerUpdate()        // called on every integral physics frame, only really triggered if before MechPlungerIn() was called, which again relies on USHOCKTYPE_GENERIC,USHOCKTYPE_ULTRACADE,USHOCKTYPE_PBWIZARD,USHOCKTYPE_VIRTUAPIN,USHOCKTYPE_SIDEWINDER being used
{
   static int init = IIR_Order;    // first time call
   static float x[IIR_Order + 1] = { 0, 0, 0, 0, 0 };
   static float y[IIR_Order + 1] = { 0, 0, 0, 0, 0 };

   //http://www.dsptutor.freeuk.com/IIRFilterDesign/IIRFilterDesign.html  
   // (this applet is set to 8000Hz sample rate, therefore, multiply ...
   // our values by 80 to shift sample clock of 100hz to 8000hz)

   if (m_movedPlunger < 3)
   {
      init = IIR_Order;
      m_curMechPlungerPos = 0;
      return; // not until a real value is entered
   }

   if (!m_ptable->m_plungerFilter)
   {
      m_curMechPlungerPos = (float)m_curPlunger;
      return;
   }

   x[0] = (float)m_curPlunger; //initialize filter
   do
   {
      y[0] = IIR_a[0] * x[0];   // initial

      for (int i = IIR_Order; i > 0; --i) // all terms but the zero-th 
      {
         y[0] += (IIR_a[i] * x[i] - IIR_b[i] * y[i]);// /b[0]; always one     // sum terms from high to low
         x[i] = x[i - 1];          //shift 
         y[i] = y[i - 1];          //shift
      }
   } while (init-- > 0); //loop until all registers are initialized with the first input

   init = 0;

   m_curMechPlungerPos = y[0];
}

// MechPlunger NOTE: Normalized position is from 0.0 to +1.0f
// +1.0 is fully retracted, 0.0 is all the way forward.
//
// The traditional method requires calibration in control panel game controllers to work right.
// The calibrated zero value should match the rest position of the mechanical plunger.
// The method below uses a dual - piecewise linear function to map the mechanical pull and push 
// onto the virtual plunger position from 0..1, the pulunger properties has a ParkPosition setting 
// that matches the mechanical plunger zero position
//
// If the plunger device is a "linear plunger", we replace that calculation with a single linear
// scaling factor that applies on both sides of the park position.  This eliminates the need for
// separate calibration on each side of the park position, which seems to produce more consistent
// and linear behavior.  The Pinscape Controller plunger uses this method.
float PlungerMoverObject::MechPlunger() const
{
   if (g_pplayer->m_pininput.m_linearPlunger)
   {
      // Linear plunger device - the joystick must be calibrated such that the park
      // position reads as 0 and the fully retracted position reads as JOYRANGEMX.  The
      // scaling factor between physical units and joystick units must be the same on the
      // positive and negative sides.  (The maximum forward position is not calibrated.)
      const float m = (1.0f - m_restPos)*(float)(1.0 / JOYRANGEMX), b = m_restPos;
      return m*g_pplayer->m_curMechPlungerPos + b;
   }
   else
   {
      // Standard plunger device - the joystick must be calibrated such that the park
      // position reads as 0, the fully retracted position reads as JOYRANGEMN, and the
      // full forward position reads as JOYRANGMN.
      const float range = (float)JOYRANGEMX * (1.0f - m_restPos) - (float)JOYRANGEMN *m_restPos; // final range limit
      const float tmp = (g_pplayer->m_curMechPlungerPos < 0) ? g_pplayer->m_curMechPlungerPos*m_restPos : g_pplayer->m_curMechPlungerPos*(1.0f - m_restPos);
      return tmp / range + m_restPos;              //scale and offset
   }
}

void Player::MechPlungerIn(const int z)
{
   m_curPlunger = -z; //axis reversal

   if (++m_movedPlunger == 0xffffffff) m_movedPlunger = 3; //restart at 3
}

// Accelerometer data filter.
//
// This is designed to process the raw acceleration data from a
// physical accelerometer installed in a cabinet to yield more
// realistic effects on the virtual ball.  With a physical
// accelerometer, there are inherent inaccuracies due to small
// measurement errors from the instrument and the finite sampling
// rate.  In addition, the VP simulation only approximates real
// time, so VP can only approximate the duration of each
// instantaneous acceleration - this can exaggerate some inputs
// and under-apply others.  Some of these sources of error are
// random and tend to cancel out over time, but others compound
// over many samples.  In practice there can be noticeable bias
// that causes unrealistic results in the simulation.
//
// This filter is designed to compensate for these various sources
// of error by applying some assumptions about how a real cabinet
// should behave, and adjusting the real input accelerations to
// more closely match how our ideal model cabinet would behave.
//
// The main constraint we use in this filter is net zero motion.
// When you nudge a real cabinet, you make it sway on its legs a
// little, but you don't usually move the cabinet across the
// floor - real cabinets are quite heavy so they tend to stay
// firmly rooted in place during normal play.  So once the swaying
// from a nudge dies out, which happens fairly quickly (in about
// one second, say), the cabinet is back where it started.  This
// means that the cabinet experienced a series of accelerations,
// back and forth, that ultimately canceled out and left the
// box at rest at its original position.  This is the central
// assumption of this filter: we should be able to add up
// (integrate) the series of instantaneous velocities imparted
// by the instantaneous accelerations over the course of a nudge,
// and we know that at the end, the sum should be zero, because
// the cabinet is back at rest at its starting location.  In
// practice, real accelerometer data for a real nudge event will
// come *close* to netting to zero, but won't quite get there;
// the discrepancy is from the various sources of error described
// above.  This filter tries to reconcile the imperfect measured
// data with our mathematically ideal model by making small
// adjustments to the measured data to get it to match the
// ideal model results.
//
// The point of the filter is to make the results *feel* more
// realistic by reducing visible artifacts from the measurement
// inaccuracies.  We thus have to take care that the filter's
// meedling hand doesn't itself become apparent as another
// visible artifact.  We try to keep the filter's effects subtle
// by trying to limit its intervention to small adjustments.
// For the most part, it kicks in at the point in a nudge where
// the real accelerometer data says things are coming back to rest
// naturally, and simply cleans up the tail end of the nudge
// response to get it to mathematical exactness.  The filter also
// notices a special situation where it shouldn't intervene,
// which is when there's a sustained acceleration in one
// direction.  Assuming that we're not operating under
// extraordinary conditions (e.g., on board an airplane
// accelerating down the runway), a sustained acceleration can
// only mean that someone picked up one end of the cabinet and
// is holding it at an angle.  In this case the ball *should*
// be accelerated in the direction of the tilt, so we don't
// attempt to zero out the net accelerations when we notice this
// type of condition.
//
// It's important to understand that this filter is only useful
// when the nudge inputs are coming from a physical, analog
// accelerometer installed in a standard, free-standing pinball
// cabinet.  The model and the parameters are tailored for this
// particular physical configuration, and it won't give good results
// for other setups.  In particular, don't use this filter with
// "digital" on/off nudge inputs, such as keyboard-based nudging or
// with cabinet sensors based on mercury switches or plumb bobs.
// The nudge accelerations for these sorts of digital nudge inputs
// are simulated, so they're already free of the analog measurement
// errors that this filter is designed to compensate for.
//
// This filter *might* work with real accelerometers that aren't
// in standard cabinets, such as in mini-cabs or desktop controllers.
// It's designed to mimic the physics of a standard cabinet, so
// using it in another physical setup probably wouldn't emulate
// that setup's natural physical behavior.  But the filter might
// give pleasing results anyway simply because every VP setup is
// ultimately meant to simulate the cabinet experience, so you
// probably want the simulation to behave like a cabinet even when
// it's not actually running in a cabinet physically.
//

NudgeFilter::NudgeFilter()
{
   m_sum = m_prv = 0.0f;
   m_tMotion = m_tCorr = m_tzc = 0;
}

// Process a sample.  Adds the sample to the running total, and checks
// to see if a correction should be applied.  Replaces 'a' with the
// corrected value if a correction is needed.
void NudgeFilter::sample(float &a, const U64 now)
{
   IF_DEBUG_NUDGE(char notes[128] = ""; float aIn = a;)

      // if we're not roughly at rest, reset the last motion timer
      if (fabsf(a) >= .02f)
         m_tMotion = now;

   // check for a sign change
   if (fabsf(a) > .01f && fabsf(m_prv) > .01f
      && ((a < 0.f && m_prv > 0.f) || (a > 0.f && m_prv < 0.f)))
   {
      // sign change/zero crossing - note the time
      m_tzc = now;
      IF_DEBUG_NUDGE(strncat_s(notes, "zc ", sizeof(notes)-strnlen_s(notes, sizeof(notes))-1);)
   }
   else if (fabsf(a) <= .01f)
   {
      // small value -> not a sustained one-way acceleration
      m_tzc = now;
   }
   /*else if (fabsf(a) > .05f && now - m_tzc > 500000) // disabling this fixes an issue with Mot-Ion / Pinball Wizard controllers that suffer from calibration drift as they warm up
   {
      // More than 500 ms in motion with same sign - we must be
      // experiencing a gravitational acceleration due to a tilt
      // of the playfield rather than a transient acceleration
      // from a nudge.  Don't attempt to correct these - clear
      // the sum and do no further processing.
      m_sum = 0;
      IF_DEBUG_NUDGE(dbg("%f >>>\n", a));
      return;
   }*/

   // if this sample is non-zero, remember it as the previous sample
   if (a != 0.f)
      m_prv = a;

   // add this sample to the running total
   m_sum += a;

   // If the running total is near zero, correct it to exactly zero.
   // 
   // Otherwise, if it's been too long since the last correction, or
   // we've been roughly at rest a while, add a damping correction to
   // bring the running total toward rest.
   if (fabsf(m_sum) < .02f)
   {
      // bring the residual acceleration exactly to rest
      IF_DEBUG_NUDGE(strncat_s(notes, "zero ", sizeof(notes)-strnlen_s(notes, sizeof(notes))-1);)
         a -= m_sum;
      m_sum = 0.f;

      // this counts as a zero crossing reset
      m_prv = 0;
      m_tzc = m_tCorr = now;
   }
   else if (now - m_tCorr > 50000 || now - m_tMotion > 50000)
   {
      // bring the running total toward neutral
      const float corr = expf(0.33f*logf(fabsf(m_sum*(float)(1.0 / .02)))) * (m_sum < 0.0f ? -.02f : .02f);
      IF_DEBUG_NUDGE(strncat_s(notes, "damp ", sizeof(notes)-strnlen_s(notes, sizeof(notes))-1);)
         a -= corr;
      m_sum -= corr;

      // Advance the correction time slightly, but not all the
      // way to the present - we want to allow another forced
      // correction soon if necessary to get things back to
      // neutral quickly.
      m_tCorr = now - 40000;
   }

   IF_DEBUG_NUDGE(
      if (a != 0.f || aIn != 0.f)
         dbg(*axis() == 'x' ? "%f,%f, , ,%s\n" : " , ,%f,%f,%s\n",
         aIn, a, notes);)
}

// debug output
IF_DEBUG_NUDGE(void NudgeFilter::dbg(const char *fmt, ...) {
   va_list args;
   va_start(args, fmt);
   static FILE *fp = 0;
   if (fp == 0) fp = fopen("c:\\joystick.csv", "w");
   vfprintf(fp, fmt, args);
   va_end(args);
})

// apply nudge acceleration data filtering
void Player::FilterNudge()
{
   m_NudgeFilterX.sample(m_NudgeX, m_curPhysicsFrameTime);
   m_NudgeFilterY.sample(m_NudgeY, m_curPhysicsFrameTime);
}

//++++++++++++++++++++++++++++++++++++++++

void Player::SetGravity(float slopeDeg, float strength)
{
   m_gravity.x = 0;
   m_gravity.y = sinf(ANGTORAD(slopeDeg)) * strength;
   m_gravity.z = -cosf(ANGTORAD(slopeDeg)) * strength;
}

void Player::PhysicsSimulateCycle(float dtime) // move physics forward to this time
{
   int StaticCnts = STATICCNTS;    // maximum number of static counts

   // it's okay to have this code outside of the inner loop, as the ball hitrects already include the maximum distance they can travel in that timespan
   m_hitoctree_dynamic.Update();

   while (dtime > 0.f)
   {
      // first find hits, if any +++++++++++++++++++++ 
#ifdef DEBUGPHYSICS
      c_timesearch++;
#endif
      float hittime = dtime;       // begin time search from now ...  until delta ends

      // find earliest time where a flipper collides with its stop
      for (size_t i = 0; i < m_vFlippers.size(); ++i)
      {
         const float fliphit = m_vFlippers[i]->GetHitTime();
         //if ((fliphit >= 0.f) && !sign(fliphit) && (fliphit <= hittime))
         if ((fliphit > 0.f) && (fliphit <= hittime)) //!! >= 0.f causes infinite loop
            hittime = fliphit;
      }

      m_recordContacts = true;
      m_contacts.clear();

#ifdef USE_EMBREE
      for (size_t i = 0; i < m_vball.size(); i++)
         if (!m_vball[i]->m_d.m_frozen
#ifdef C_DYNAMIC
             && m_vball[i]->m_dynamic > 0
#endif
            ) // don't play with frozen balls
         {
            m_vball[i]->m_coll.m_hittime = hittime; // search upto current hittime
            m_vball[i]->m_coll.m_obj = NULL;
         }

      if (!m_vball.empty())
      {
         m_hitoctree.HitTestBall(m_vball);         // find the hit objects hit times
         m_hitoctree_dynamic.HitTestBall(m_vball); // dynamic objects !! should reuse the same embree scene created already in m_hitoctree.HitTestBall!
      }
#endif

      for (size_t i = 0; i < m_vball.size(); i++)
      {
         Ball * const pball = m_vball[i];

         if (!pball->m_d.m_frozen
#ifdef C_DYNAMIC
             && pball->m_dynamic > 0
#endif
            ) // don't play with frozen balls
         {
#ifndef USE_EMBREE
            pball->m_coll.m_hittime = hittime;          // search upto current hittime
            pball->m_coll.m_obj = NULL;
#endif
            // always check for playfield and top glass
            if (!m_meshAsPlayfield)
               DoHitTest(pball, &m_hitPlayfield, pball->m_coll);

            DoHitTest(pball, &m_hitTopGlass, pball->m_coll);

#ifndef USE_EMBREE
            if (rand_mt_01() < 0.5f) // swap order of dynamic and static obj checks randomly
            {
               m_hitoctree_dynamic.HitTestBall(pball, pball->m_coll); // dynamic objects
               m_hitoctree.HitTestBall(pball, pball->m_coll);         // find the static hit objects hit times
            }
            else
            {
               m_hitoctree.HitTestBall(pball, pball->m_coll);         // find the static hit objects hit times
               m_hitoctree_dynamic.HitTestBall(pball, pball->m_coll); // dynamic objects
            }
#endif
            const float htz = pball->m_coll.m_hittime; // this ball's hit time
            if (htz < 0.f) pball->m_coll.m_obj = nullptr; // no negative time allowed

            if (pball->m_coll.m_obj)                   // hit object
            {
#ifdef DEBUGPHYSICS
               ++c_hitcnts;                            // stats for display

               if (/*pball->m_coll.m_hitRigid &&*/ pball->m_coll.m_hitdistance < -0.0875f) //rigid and embedded
                  ++c_embedcnts;
#endif
               ///////////////////////////////////////////////////////////////////////////

               if (htz <= hittime)                     // smaller hit time??
               {
                  hittime = htz;                       // record actual event time

                  if (hittime < STATICTIME)            // less than static time interval
                  {
                     /*if (!pball->m_coll.m_hitRigid) hittime = STATICTIME; // non-rigid ... set Static time
                     else*/ if (--StaticCnts < 0)
                     {
                        StaticCnts = 0;                // keep from wrapping
                        hittime = STATICTIME;
                     }
                  }
               }
            }
         }
      } // end loop over all balls

      m_recordContacts = false;

      // hittime now set ... or full frame if no hit 
      // now update displacements to collide-contact or end of physics frame
      // !!!!! 2) move objects to hittime

      if (hittime > STATICTIME) StaticCnts = STATICCNTS; // allow more zeros next round

      for (size_t i = 0; i < m_vmover.size(); i++)
         m_vmover[i]->UpdateDisplacements(hittime); // step 2: move the objects about according to velocities (spinner, gate, flipper, plunger, ball)

      // find balls that need to be collided and script'ed (generally there will be one, but more are possible)

      for (size_t i = 0; i < m_vball.size(); i++) // use m_vball.size(), in case script deletes a ball
      {
         Ball * const pball = m_vball[i];

         if (
#ifdef C_DYNAMIC
             pball->m_dynamic > 0 &&
#endif
             pball->m_coll.m_obj && pball->m_coll.m_hittime <= hittime) // find balls with hit objects and minimum time
         {
            // now collision, contact and script reactions on active ball (object)+++++++++
            HitObject * const pho = pball->m_coll.m_obj; // object that ball hit in trials
            m_pactiveball = pball;                       // For script that wants the ball doing the collision
#ifdef DEBUGPHYSICS
            c_collisioncnt++;
#endif
            pho->Collide(pball->m_coll);                 //!!!!! 3) collision on active ball
            pball->m_coll.m_obj = NULL;                  // remove trial hit object pointer

            // Collide may have changed the velocity of the ball, 
            // and therefore the bounding box for the next hit cycle
            if (m_vball[i] != pball) // Ball still exists? may have been deleted from list
            {
               // collision script deleted the ball, back up one count
               --i;
               continue;
            }
            else
            {
#ifdef C_DYNAMIC
               // is this ball static? .. set static and quench        
               if (/*pball->m_coll.m_hitRigid &&*/ (pball->m_coll.m_hitdistance < (float)PHYS_TOUCH)) //rigid and close distance contacts //!! rather test isContact??
               {
                  const float mag = pball->m_vel.x*pball->m_vel.x + pball->m_vel.y*pball->m_vel.y; // values below are taken from simulation
                  if (pball->m_drsq < 8.0e-5f && mag < 1.0e-3f*m_ptable->m_Gravity*m_ptable->m_Gravity / GRAVITYCONST / GRAVITYCONST && fabsf(pball->m_vel.z) < 0.2f*m_ptable->m_Gravity / GRAVITYCONST)
                  {
                     if (--pball->m_dynamic <= 0)             //... ball static, cancels next gravity increment
                     {                                       // m_dynamic is cleared in ball gravity section
                        pball->m_dynamic = 0;
#ifdef DEBUGPHYSICS
                        c_staticcnt++;
#endif
                        pball->m_vel.x = pball->m_vel.y = pball->m_vel.z = 0.f; //quench the remaining velocity and set ...
                     }
                  }
               }
#endif
            }
         }
      }

#ifdef DEBUGPHYSICS
      c_contactcnt = (U32)m_contacts.size();
#endif
      /*
       * Now handle contacts.
       *
       * At this point UpdateDisplacements() was already called, so the state is different
       * from that at HitTest(). However, contacts have zero relative velocity, so
       * hopefully nothing catastrophic has happened in the meanwhile.
       *
       * Maybe a two-phase setup where we first process only contacts, then only collisions
       * could also work.
       */
      if (rand_mt_01() < 0.5f) // swap order of contact handling randomly
         for (size_t i = 0; i < m_contacts.size(); ++i)
            //if (m_contacts[i].m_hittime <= hittime) // does not happen often, and values then look sane, so do this check //!! why does this break some collisions (MM NZ&TT Reloaded Skitso, also CCC (Saloon))? maybe due to ball colliding with multiple things and then some sideeffect?
               m_contacts[i].m_obj->Contact(m_contacts[i], hittime);
      else
         for (size_t i = m_contacts.size() - 1; i != -1; --i)
            //if (m_contacts[i].m_hittime <= hittime) // does not happen often, and values then look sane, so do this check //!! why does this break some collisions (MM NZ&TT Reloaded Skitso, also CCC (Saloon))? maybe due to ball colliding with multiple things and then some sideeffect?
               m_contacts[i].m_obj->Contact(m_contacts[i], hittime);

      m_contacts.clear();

#ifdef C_BALL_SPIN_HACK
      // hacky killing of ball spin on resting balls (very low and very high spinning)
      for (size_t i = 0; i < m_vball.size(); i++)
      {
         Ball * const pball = m_vball[i];

         const unsigned int p0 = (pball->m_ringcounter_oldpos / (10000 / PHYSICS_STEPTIME) + 1) % MAX_BALL_TRAIL_POS;
         const unsigned int p1 = (pball->m_ringcounter_oldpos / (10000 / PHYSICS_STEPTIME) + 2) % MAX_BALL_TRAIL_POS;

         if (/*pball->m_coll.m_hitRigid &&*/ (pball->m_coll.m_hitdistance < (float)PHYS_TOUCH) && (pball->m_oldpos[p0].x != FLT_MAX) && (pball->m_oldpos[p1].x != FLT_MAX)) // only if already initialized
         {
            /*const float mag = pball->m_vel.x*pball->m_vel.x + pball->m_vel.y*pball->m_vel.y; // values below are copy pasted from above
            if (pball->m_drsq < 8.0e-5f && mag < 1.0e-3f*m_ptable->m_Gravity*m_ptable->m_Gravity / GRAVITYCONST / GRAVITYCONST && fabsf(pball->m_vel.z) < 0.2f*m_ptable->m_Gravity / GRAVITYCONST
            && pball->m_angularmomentum.Length() < 0.9f*m_ptable->m_Gravity / GRAVITYCONST
            ) //&& rand_mt_01() < 0.95f)
            {
            pball->m_angularmomentum *= 0.05f; // do not kill spin completely, otherwise stuck balls will happen during regular gameplay
            }*/

            const Vertex3Ds diff_pos = pball->m_oldpos[p0] - pball->m_d.m_pos;
            const float mag = diff_pos.x*diff_pos.x + diff_pos.y*diff_pos.y;
            const Vertex3Ds diff_pos2 = pball->m_oldpos[p1] - pball->m_d.m_pos;
            const float mag2 = diff_pos2.x*diff_pos2.x + diff_pos2.y*diff_pos2.y;

            const float threshold = (pball->m_angularmomentum.x*pball->m_angularmomentum.x + pball->m_angularmomentum.y*pball->m_angularmomentum.y) / max(mag, mag2);

            if (!infNaN(threshold) && threshold > 666.f)
            {
               const float damp = clamp(1.0f - (threshold - 666.f) / 10000.f, 0.23f, 1.f); // do not kill spin completely, otherwise stuck balls will happen during regular gameplay
               pball->m_angularmomentum *= damp;
            }
         }
      }
#endif

      dtime -= hittime;       //new delta .. i.e. time remaining

      m_swap_ball_collision_handling = !m_swap_ball_collision_handling; // swap order of ball-ball collisions

   } // end physics loop
}

void Player::UpdatePhysics()
{
   U64 initial_time_usec = usec();

   // DJRobX's crazy latency-reduction code
   U64 delta_frame = 0;
   if (m_minphyslooptime > 0 && m_lastFlipTime > 0)
   {
      // We want the physics loops to sync up to the the frames, not
      // the post-render period, as that can cause some judder.
      delta_frame = initial_time_usec - m_lastFlipTime;
      initial_time_usec -= delta_frame;
   }

   if (m_noTimeCorrect) // After debugging script
   {
      // Shift whole game foward in time
      m_StartTime_usec       += initial_time_usec - m_curPhysicsFrameTime;
      m_nextPhysicsFrameTime += initial_time_usec - m_curPhysicsFrameTime;
      m_curPhysicsFrameTime   = initial_time_usec; // 0 time frame
      m_noTimeCorrect = false;
   }

#ifdef STEPPING
#ifndef EVENPHYSICSTIME
   if (m_debugWindowActive || m_userDebugPaused)
   {
      // Shift whole game foward in time
      m_StartTime_usec       += initial_time_usec - m_curPhysicsFrameTime;
      m_nextPhysicsFrameTime += initial_time_usec - m_curPhysicsFrameTime;
      if (m_step)
      {
         // Walk one physics step foward
         m_curPhysicsFrameTime = initial_time_usec - PHYSICS_STEPTIME;
         m_step = false;
      }
      else
         m_curPhysicsFrameTime = initial_time_usec; // 0 time frame
   }
#endif
#endif

#ifdef EVENPHYSICSTIME
   if (!m_pause || m_step)
   {
      initial_time_usec = m_curPhysicsFrameTime - 3547811060 + 3547825450;
      m_step = false;
   }
   else
      initial_time_usec = m_curPhysicsFrameTime;
#endif

   //if (ShowFPS())
   {
      m_lastFrameDuration = (U32)(initial_time_usec - m_lastTime_usec);
      if (m_lastFrameDuration > 1000000)
         m_lastFrameDuration = 0;
      m_lastTime_usec = initial_time_usec;

      m_cframes++;
      if ((m_time_msec - m_lastfpstime) > 1000)
      {
         m_fps = (float)((double)m_cframes * 1000.0 / (m_time_msec - m_lastfpstime));
         m_lastfpstime = m_time_msec;
         m_fpsAvg += m_fps;
         m_fpsCount++;
         m_cframes = 0;
      }
   }

   m_script_period = 0;

#ifdef LOG
   const double timepassed = (double)(initial_time_usec - m_curPhysicsFrameTime) / 1000000.0;

   const float frametime =
#ifdef PLAYBACK
      (!m_playback) ? (float)(timepassed * 100.0) : ParseLog((LARGE_INTEGER*)&initial_time_usec, (LARGE_INTEGER*)&m_nextPhysicsFrameTime);
#else
#define TIMECORRECT 1
#ifdef TIMECORRECT
      (float)(timepassed * 100.0);
   // 1.456927f;
#else
      0.45f;
#endif
#endif //PLAYBACK

   fprintf(m_flog, "Frame Time %.20f %u %u %u %u\n", frametime, initial_time_usec>>32, initial_time_usec, m_nextPhysicsFrameTime>>32, m_nextPhysicsFrameTime);
   fprintf(m_flog, "End Frame\n");
#endif

   m_phys_iterations = 0;

   bool first_cycle = true;

   while (m_curPhysicsFrameTime < initial_time_usec) // loop here until current (real) time matches the physics (simulated) time
   {
      // Get time in milliseconds for timers
      m_time_msec = (U32)((m_curPhysicsFrameTime - m_StartTime_usec) / 1000);

      m_phys_iterations++;

      // Get the time until the next physics tick is done, and get the time
      // until the next frame is done
      // If the frame is the next thing to happen, update physics to that
      // point next update acceleration, and continue loop

      const float physics_diff_time = (float)((double)(m_nextPhysicsFrameTime - m_curPhysicsFrameTime)*(1.0 / DEFAULT_STEPTIME));
      //const float physics_to_graphic_diff_time = (float)((double)(initial_time_usec - m_curPhysicsFrameTime)*(1.0 / DEFAULT_STEPTIME));

      //if (physics_to_graphic_diff_time < physics_diff_time)          // is graphic frame time next???
      //{
      //      PhysicsSimulateCycle(physics_to_graphic_diff_time);      // advance physics to this time
      //      m_curPhysicsFrameTime = initial_time_usec;               // now current to the wall clock
      //      break;  //this is the common exit from the loop          // exit skipping accelerate
      //}                     // some rare cases will exit from while()


      // DJRobX's crazy latency-reduction code: Artificially lengthen the execution of the physics loop by X usecs, to give more opportunities to read changes from input(s) (try values in the multiple 100s up to maximum 1000 range, in general: the more, the faster the CPU is)
      //                                        Intended mainly to be used if vsync is enabled (e.g. most idle time is shifted from vsync-waiting to here)
      if (m_minphyslooptime > 0)
      {
          const U64 basetime = usec(); 
          const U64 targettime = ((U64)m_minphyslooptime * m_phys_iterations) + m_lastFlipTime;
          // If we're 3/4 of the way through the loop, fire a "frame sync" timer event so VPM can react to input.
          // This will effectively double the "-1" timer rate, but the goal, when this option is enabled, is to reduce latency
          // and those "-1" timer calls should be roughly halfway through the cycle
          if (m_phys_iterations == 750 / ((int)m_fps + 1))
          {
              first_cycle = true; //!! side effects!?!
              m_script_period = 0; // !!!! SIDE EFFECTS?!?!?!
          }
          if (basetime < targettime)
              uSleep(targettime - basetime);
      }
      // end DJRobX's crazy code
      const U64 cur_time_usec = usec()-delta_frame; //!! one could also do this directly in the while loop condition instead (so that the while loop will really match with the current time), but that leads to some stuttering on some heavy frames

      // hung in the physics loop over 200 milliseconds or the number of physics iterations to catch up on is high (i.e. very low/unplayable FPS)
      if ((cur_time_usec - initial_time_usec > 200000) || (m_phys_iterations > ((m_ptable->m_PhysicsMaxLoops == 0) || (m_ptable->m_PhysicsMaxLoops == 0xFFFFFFFFu) ? 0xFFFFFFFFu : (m_ptable->m_PhysicsMaxLoops*(10000 / PHYSICS_STEPTIME))/*2*/)))
      {                                                             // can not keep up to real time
         m_curPhysicsFrameTime  = initial_time_usec;                // skip physics forward ... slip-cycles -> 'slowed' down physics
         m_nextPhysicsFrameTime = initial_time_usec + PHYSICS_STEPTIME;
         break;                                                     // go draw frame
      }

      //update keys, hid, plumb, nudge, timers, etc
      //const U32 sim_msec = (U32)(m_curPhysicsFrameTime / 1000);
      const U32 cur_time_msec = (U32)(cur_time_usec / 1000);

      m_pininput.ProcessKeys(/*sim_msec,*/ cur_time_msec);

      mixer_update();
      hid_update(/*sim_msec*/cur_time_msec);
      plumb_update(/*sim_msec*/cur_time_msec, GetNudgeX(), GetNudgeY());

#ifdef ACCURATETIMERS
      // do the en/disable changes for the timers that piled up
      for(size_t i = 0; i < m_changed_vht.size(); ++i)
          if (m_changed_vht[i].m_enabled) // add the timer?
          {
              if (FindIndexOf(m_vht, m_changed_vht[i].m_timer) < 0)
                  m_vht.push_back(m_changed_vht[i].m_timer);
          }
          else // delete the timer?
          {
              const int idx = FindIndexOf(m_vht, m_changed_vht[i].m_timer);
              if (idx >= 0)
                  m_vht.erase(m_vht.begin() + idx);
          }
      m_changed_vht.clear();

      Ball * const old_pactiveball = m_pactiveball;
      m_pactiveball = NULL; // No ball is the active ball for timers/key events

      if(m_script_period <= 1000*MAX_TIMERS_MSEC_OVERALL) // if overall script time per frame exceeded, skip
      {
         const unsigned int p_timeCur = (unsigned int)((m_curPhysicsFrameTime - m_StartTime_usec) / 1000); // milliseconds

         for (size_t i = 0; i < m_vht.size(); i++)
         {
            HitTimer * const pht = m_vht[i];
            if ((pht->m_interval >= 0 && pht->m_nextfire <= p_timeCur) || (pht->m_interval < 0 && first_cycle))
            {
               const unsigned int curnextfire = pht->m_nextfire;
               pht->m_pfe->FireGroupEvent(DISPID_TimerEvents_Timer);
               // Only add interval if the next fire time hasn't changed since the event was run. 
               // Handles corner case:
               //Timer1.Enabled = False
               //Timer1.Interval = 1000
               //Timer1.Enabled = True
               if (curnextfire == pht->m_nextfire)
                  pht->m_nextfire += pht->m_interval;
            }
         }

         m_script_period += (unsigned int)(usec() - (cur_time_usec+delta_frame));
      }

      m_pactiveball = old_pactiveball;
#endif

      NudgeUpdate();       // physics_diff_time is the balance of time to move from the graphic frame position to the next
      MechPlungerUpdate(); // integral physics frame. So the previous graphics frame was (1.0 - physics_diff_time) before 
      // this integral physics frame. Accelerations and inputs are always physics frame aligned

      // table movement is modeled as a mass-spring-damper system
      //   u'' = -k u - c u'
      // with a spring constant k and a damping coefficient c
      const Vertex3Ds force = -m_nudgeSpring * m_tableDisplacement - m_nudgeDamping * m_tableVel;
      m_tableVel          += (float)PHYS_FACTOR * force;
      m_tableDisplacement += (float)PHYS_FACTOR * m_tableVel;

      m_tableVelDelta = m_tableVel - m_tableVelOld;
      m_tableVelOld = m_tableVel;

      // legacy/VP9 style keyboard nudging
      if (m_legacyNudge && m_legacyNudgeTime != 0)
      {
          --m_legacyNudgeTime;

          if (m_legacyNudgeTime == 95)
          {
              m_NudgeX = -m_legacyNudgeBackX * 2.0f;
              m_NudgeY =  m_legacyNudgeBackY * 2.0f;
          }
          else if (m_legacyNudgeTime == 90)
          {
              m_NudgeX =  m_legacyNudgeBackX;
              m_NudgeY = -m_legacyNudgeBackY;
          }

          if (m_NudgeShake > 0.0f)
              SetScreenOffset(m_NudgeShake * m_legacyNudgeBackX * sqrf((float)m_legacyNudgeTime*0.01f), -m_NudgeShake * m_legacyNudgeBackY * sqrf((float)m_legacyNudgeTime*0.01f));
      }
      else
          if (m_NudgeShake > 0.0f)
          {
              // NB: in table coordinates, +Y points down, but in screen coordinates, it points up,
              // so we have to flip the y component
              SetScreenOffset(m_NudgeShake * m_tableDisplacement.x, -m_NudgeShake * m_tableDisplacement.y);
          }

      // Apply our filter to the nudge data
      if (m_pininput.m_enable_nudge_filter)
         FilterNudge();

      for (size_t i = 0; i < m_vmover.size(); i++)
         m_vmover[i]->UpdateVelocities();      // always on integral physics frame boundary (spinner, gate, flipper, plunger, ball)

      //primary physics loop
      PhysicsSimulateCycle(physics_diff_time); // main simulator call

      //ball trail, keep old pos of balls
      for (size_t i = 0; i < m_vball.size(); i++)
      {
         Ball * const pball = m_vball[i];
         pball->m_oldpos[pball->m_ringcounter_oldpos / (10000 / PHYSICS_STEPTIME)] = pball->m_d.m_pos;

         pball->m_ringcounter_oldpos++;
         if (pball->m_ringcounter_oldpos == MAX_BALL_TRAIL_POS*(10000 / PHYSICS_STEPTIME))
            pball->m_ringcounter_oldpos = 0;
      }

      //slintf( "PT: %f %f %u %u %u\n", physics_diff_time, physics_to_graphic_diff_time, (U32)(m_curPhysicsFrameTime/1000), (U32)(initial_time_usec/1000), cur_time_msec );

      m_curPhysicsFrameTime = m_nextPhysicsFrameTime; // new cycle, on physics frame boundary
      m_nextPhysicsFrameTime += PHYSICS_STEPTIME;     // advance physics position

      first_cycle = false;
   } // end while (m_curPhysicsFrameTime < initial_time_usec)

   m_phys_period = (U32)((usec() - delta_frame) - initial_time_usec);
}

void Player::DMDdraw(const float DMDposx, const float DMDposy, const float DMDwidth, const float DMDheight, const COLORREF DMDcolor, const float intensity)
{
   if (m_texdmd)
   {
      float DMDVerts[4 * 5] =
      {
         1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
         0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
         1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
         0.0f, 0.0f, 0.0f, 0.0f, 0.0f
      };

      for (unsigned int i = 0; i < 4; ++i)
      {
         DMDVerts[i * 5] = (DMDVerts[i * 5] * DMDwidth + DMDposx)*2.0f - 1.0f;
         DMDVerts[i * 5 + 1] = 1.0f - (DMDVerts[i * 5 + 1] * DMDheight + DMDposy)*2.0f;
      }

      //const float width = m_pin3d.m_useAA ? 2.0f*(float)m_width : (float)m_width; //!! AA ?? -> should just work
      m_pin3d.m_pd3dPrimaryDevice->DMDShader->SetTechnique("basic_DMD"); //!! DMD_UPSCALE ?? -> should just work

      const vec4 c = convertColor(DMDcolor, intensity);
      m_pin3d.m_pd3dPrimaryDevice->DMDShader->SetVector("vColor_Intensity", &c);
#ifdef DMD_UPSCALE
      const vec4 r((float)(m_dmdx*3), (float)(m_dmdy*3), 1.f, (float)(g_pplayer->m_overall_frames%2048));
#else
      const vec4 r((float)m_dmdx, (float)m_dmdy, 1.f, (float)(g_pplayer->m_overall_frames%2048));
#endif
      m_pin3d.m_pd3dPrimaryDevice->DMDShader->SetVector("vRes_Alpha_time", &r);

      m_pin3d.m_pd3dPrimaryDevice->DMDShader->SetTexture("Texture0", m_pin3d.m_pd3dPrimaryDevice->m_texMan.LoadTexture(m_texdmd, false));

      m_pin3d.m_pd3dPrimaryDevice->DMDShader->Begin(0);
      m_pin3d.m_pd3dPrimaryDevice->DrawTexturedQuad((Vertex3D_TexelOnly*)DMDVerts);
      m_pin3d.m_pd3dPrimaryDevice->DMDShader->End();
   }
}

void Player::Spritedraw(const float posx, const float posy, const float width, const float height, const COLORREF color, Texture * const tex, const float intensity, const bool backdrop)
{
   RenderDevice * const pd3dDevice = backdrop ? m_pin3d.m_pd3dSecondaryDevice : m_pin3d.m_pd3dPrimaryDevice;

   float Verts[4 * 5] =
   {
       1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
       0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
       1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
       0.0f, 0.0f, 0.0f, 0.0f, 0.0f
   };

   for (unsigned int i = 0; i < 4; ++i)
   {
      Verts[i * 5] = (Verts[i * 5] * width + posx)*2.0f - 1.0f;
      Verts[i * 5 + 1] = 1.0f - (Verts[i * 5 + 1] * height + posy)*2.0f;
   }

   pd3dDevice->DMDShader->SetTechnique(tex ? "basic_noDMD" : "basic_noDMD_notex");

   const vec4 c = convertColor(color, intensity);
   pd3dDevice->DMDShader->SetVector("vColor_Intensity", &c);

   if (tex)
      pd3dDevice->DMDShader->SetTexture("Texture0", tex, false);

   pd3dDevice->DMDShader->Begin(0);
   pd3dDevice->DrawTexturedQuad((Vertex3D_TexelOnly*)Verts);
   pd3dDevice->DMDShader->End();
}

void Player::Spritedraw(const float posx, const float posy, const float width, const float height, const COLORREF color, D3DTexture * const tex, const float intensity, const bool backdrop)
{
   RenderDevice * const pd3dDevice = backdrop ? m_pin3d.m_pd3dSecondaryDevice : m_pin3d.m_pd3dPrimaryDevice;

   float Verts[4 * 5] =
   {
      1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
      0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
      1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 0.0f, 0.0f
   };

   for (unsigned int i = 0; i < 4; ++i)
   {
      Verts[i * 5] = (Verts[i * 5] * width + posx)*2.0f - 1.0f;
      Verts[i * 5 + 1] = 1.0f - (Verts[i * 5 + 1] * height + posy)*2.0f;
   }

   pd3dDevice->DMDShader->SetTechnique(tex ? "basic_noDMD" : "basic_noDMD_notex");

   const vec4 c = convertColor(color, intensity);
   pd3dDevice->DMDShader->SetVector("vColor_Intensity", &c);

   if (tex)
       pd3dDevice->DMDShader->SetTexture("Texture0", tex);

   pd3dDevice->DMDShader->Begin(0);
   pd3dDevice->DrawTexturedQuad((Vertex3D_TexelOnly*)Verts);
   pd3dDevice->DMDShader->End();
}

void Player::DrawBulbLightBuffer()
{
   // switch to 'bloom' output buffer to collect all bulb lights
   m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget(m_pin3d.m_pd3dPrimaryDevice->GetBloomBufferTexture());
   m_pin3d.m_pd3dPrimaryDevice->Clear(0, NULL, clearType::TARGET, 0, 1.0f, 0L);

   // check if any bulb specified at all
   bool do_renderstage = false;
   for (size_t i = 0; i < m_vHitTrans.size(); ++i)
      if (m_vHitTrans[i]->RenderToLightBuffer())
      {
         do_renderstage = true;
         break;
      }

   if (do_renderstage)
   {
      m_current_renderstage = 1; // for bulb lights so they know what they have to do

      m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::ZENABLE, RenderDevice::RS_FALSE); // disable all z-tests as zbuffer is in different resolution

      // Draw bulb lights with transmission scale only
      for (size_t i = 0; i < m_vHitTrans.size(); ++i)
         if (m_vHitTrans[i]->RenderToLightBuffer())
            m_vHitTrans[i]->RenderDynamic();

      m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::DEPTHBIAS, 0); //!! paranoia set of old state, remove as soon as sure that no other code still relies on that legacy set
      //m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::ZWRITEENABLE, RenderDevice::RS_TRUE);
      m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::BLENDOP, RenderDevice::BLENDOP_ADD);
      //m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_CCW);

      m_pin3d.DisableAlphaBlend();
      m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_NONE);
      m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::ZWRITEENABLE, RenderDevice::RS_FALSE);
      m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::ZENABLE, RenderDevice::RS_FALSE);

      //for (unsigned int blur = 0; blur < 2; ++blur) // uses larger blur kernel instead now (see below)
      {
         {
            m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture("Texture0", (D3DTexture*)NULL);

            // switch to 'bloom' temporary output buffer for horizontal phase of gaussian blur
            m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget(m_pin3d.m_pd3dPrimaryDevice->GetBloomTmpBufferTexture());

            m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture("Texture0", m_pin3d.m_pd3dPrimaryDevice->GetBloomBufferTexture());
            const vec4 fb_inv_resolution_05((float)(4.0 / (double)m_width), (float)(4.0 / (double)m_height), 1.0f, 1.0f);
            m_pin3d.m_pd3dPrimaryDevice->FBShader->SetVector("w_h_height", &fb_inv_resolution_05);
            m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTechnique("fb_bloom_horiz19x19");

            m_pin3d.m_pd3dPrimaryDevice->FBShader->Begin(0);
            m_pin3d.m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad();
            m_pin3d.m_pd3dPrimaryDevice->FBShader->End();
         }
         {
            m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture("Texture0", (D3DTexture*)NULL);

            // switch to 'bloom' output buffer for vertical phase of gaussian blur
            m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget(m_pin3d.m_pd3dPrimaryDevice->GetBloomBufferTexture());

            m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture("Texture0", m_pin3d.m_pd3dPrimaryDevice->GetBloomTmpBufferTexture());
            const vec4 fb_inv_resolution_05((float)(4.0 / (double)m_width), (float)(4.0 / (double)m_height), 1.0f, 1.0f);
            m_pin3d.m_pd3dPrimaryDevice->FBShader->SetVector("w_h_height", &fb_inv_resolution_05);
            m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTechnique("fb_bloom_vert19x19");

            m_pin3d.m_pd3dPrimaryDevice->FBShader->Begin(0);
            m_pin3d.m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad();
            m_pin3d.m_pd3dPrimaryDevice->FBShader->End();
         }
      }

      m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::ZENABLE, RenderDevice::RS_TRUE);
      m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::ZWRITEENABLE, RenderDevice::RS_TRUE);
      m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_CCW);

      m_current_renderstage = 0;
   }

   // switch back to render buffer
   m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget(m_pin3d.m_pddsBackBuffer);

   m_pin3d.m_pd3dPrimaryDevice->basicShader->SetTexture("Texture3", m_pin3d.m_pd3dPrimaryDevice->GetBloomBufferTexture());
}

void Player::RenderDynamics()
{
   TRACE_FUNCTION();

   unsigned int reflection_path = 0;
   if (!m_cameraMode)
   {
	   const bool drawBallReflection = ((m_reflectionForBalls && (m_ptable->m_useReflectionForBalls == -1)) || (m_ptable->m_useReflectionForBalls == 1));

	   if (!(m_ptable->m_reflectElementsOnPlayfield && m_pf_refl) && drawBallReflection)
		   reflection_path = 1;
	   else if (m_ptable->m_reflectElementsOnPlayfield && m_pf_refl)
		   reflection_path = 2;
   }

   if (reflection_path != 0)
   {
	   m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::CLIPPLANEENABLE, RenderDevice::PLANE0);
	   RenderDynamicMirror(reflection_path == 1);
	   m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::CLIPPLANEENABLE, 0); // disable playfield clipplane again

	   // depth-'remove' mirror objects from holes again for objects that vanish into the table //!! disabled as it will also look stupid and costs too much for this special case
	   //m_pin3d.m_pd3dDevice->EndScene();
	   //m_pin3d.m_pd3dDevice->CopySurface(m_pin3d.m_pddsZBuffer, m_pin3d.m_pddsStaticZ); // cannot be called inside BeginScene -> EndScene cycle
	   //m_pin3d.m_pd3dDevice->BeginScene();

	   RenderMirrorOverlay();
   }

   m_pin3d.RenderPlayfieldGraphics(true); // static depth buffer only contained static (&mirror) objects, but no playfield yet -> so render depth only to add this

   if (m_cameraMode)
   {
      m_pin3d.InitLights();

      const vec4 st(m_ptable->m_envEmissionScale*m_globalEmissionScale, m_pin3d.m_envTexture ? (float)m_pin3d.m_envTexture->m_height/*+m_pin3d.m_envTexture->m_width)*0.5f*/ : (float)m_pin3d.m_builtinEnvTexture.m_height/*+m_pin3d.m_builtinEnvTexture.m_width)*0.5f*/, 0.f, 0.f); //!! dto.
      m_pin3d.m_pd3dPrimaryDevice->basicShader->SetVector("fenvEmissionScale_TexWidth", &st);
#ifdef SEPARATE_CLASSICLIGHTSHADER
      m_pin3d.m_pd3dPrimaryDevice->classicLightShader->SetVector("fenvEmissionScale_TexWidth", &st);
#endif

      UpdateBallShaderMatrix();

      m_pin3d.RenderPlayfieldGraphics(false);

      for (size_t i = 0; i < m_ptable->m_vedit.size(); i++)
      {
         if (m_ptable->m_vedit[i]->GetItemType() != eItemDecal)
         {
            Hitable * const ph = m_ptable->m_vedit[i]->GetIHitable();
            if (ph)
            {
               ph->RenderStatic();
            }
         }
      }
      // Draw decals (they have transparency, so they have to be drawn after the wall they are on)
      for (size_t i = 0; i < m_ptable->m_vedit.size(); i++)
      {
         if (m_ptable->m_vedit[i]->GetItemType() == eItemDecal)
         {
            Hitable * const ph = m_ptable->m_vedit[i]->GetIHitable();
            if (ph)
            {
               ph->RenderStatic();
            }
         }
      }
   }

   if (ProfilingMode() == 1)
      m_pin3d.m_gpu_profiler.Timestamp(GTS_PlayfieldGraphics);

   if (ProfilingMode() != 2) // normal rendering path for standard gameplay
   {
      m_dmdstate = 0;
      // Draw non-transparent objects. No DMD's
      for (size_t i = 0; i < m_vHitNonTrans.size(); ++i)
        if(!m_vHitNonTrans[i]->IsDMD())
          m_vHitNonTrans[i]->RenderDynamic();

      m_dmdstate = 2;
      // Draw non-transparent DMD's
      for (size_t i = 0; i < m_vHitNonTrans.size(); ++i)
        if (m_vHitNonTrans[i]->IsDMD())
          m_vHitNonTrans[i]->RenderDynamic();

      DrawBalls();

      if (ProfilingMode() == 1)
         m_pin3d.m_gpu_profiler.Timestamp(GTS_NonTransparent);
      m_limiter.Execute(m_pin3d.m_pd3dPrimaryDevice); //!! move below other draw calls??

      DrawBulbLightBuffer();

      if (ProfilingMode() == 1)
         m_pin3d.m_gpu_profiler.Timestamp(GTS_LightBuffer);

      m_dmdstate = 0;
      // Draw transparent objects. No DMD's
      for (size_t i = 0; i < m_vHitTrans.size(); ++i)
        if(!m_vHitTrans[i]->IsDMD())
          m_vHitTrans[i]->RenderDynamic();

      m_dmdstate = 1;
      // Draw only transparent DMD's
      for (size_t i = 0; i < m_vHitNonTrans.size(); ++i) // NonTrans is correct as DMDs are always sorted in there
        if(m_vHitNonTrans[i]->IsDMD())
          m_vHitNonTrans[i]->RenderDynamic();

      if (ProfilingMode() == 1)
         m_pin3d.m_gpu_profiler.Timestamp(GTS_Transparent);
   }
   else // special profiling path by doing separate items, will not be accurate, both perf and rendering wise, but better than nothing
   {
      m_limiter.Execute(m_pin3d.m_pd3dPrimaryDevice); //!! move below other draw calls??

      DrawBulbLightBuffer();

      m_pin3d.m_gpu_profiler.BeginFrame(m_pin3d.m_pd3dPrimaryDevice->GetCoreDevice());

      m_dmdstate = 0;

      // Draw non-transparent Primitives.
      for (size_t i = 0; i < m_vHitNonTrans.size(); ++i)
         if (m_vHitNonTrans[i]->HitableGetItemType() == eItemPrimitive)
            m_vHitNonTrans[i]->RenderDynamic();
      m_pin3d.m_gpu_profiler.Timestamp(GTS_Primitives_NT);

      // Draw non-transparent Walls, Ramps, Rubbers.
      for (size_t i = 0; i < m_vHitNonTrans.size(); ++i)
         if (m_vHitNonTrans[i]->HitableGetItemType() == eItemSurface || m_vHitNonTrans[i]->HitableGetItemType() == eItemRamp || m_vHitNonTrans[i]->HitableGetItemType() == eItemRubber)
            m_vHitNonTrans[i]->RenderDynamic();
      m_pin3d.m_gpu_profiler.Timestamp(GTS_Walls_Ramps_Rubbers_NT);

      // Else.
      m_dmdstate = 2;
      for (size_t i = 0; i < m_vHitNonTrans.size(); ++i)
         if (m_vHitNonTrans[i]->IsDMD() && m_vHitNonTrans[i]->HitableGetItemType() == eItemFlasher)
            m_vHitNonTrans[i]->RenderDynamic();

      DrawBalls();

      for (size_t i = 0; i < m_vHitNonTrans.size(); ++i)
         if (m_vHitNonTrans[i]->HitableGetItemType() != eItemPrimitive && m_vHitNonTrans[i]->HitableGetItemType() != eItemSurface && m_vHitNonTrans[i]->HitableGetItemType() != eItemRamp && m_vHitNonTrans[i]->HitableGetItemType() != eItemRubber)
            m_vHitNonTrans[i]->RenderDynamic();

      for (size_t i = 0; i < m_vHitTrans.size(); ++i)
         if (m_vHitTrans[i]->HitableGetItemType() != eItemPrimitive && m_vHitTrans[i]->HitableGetItemType() != eItemSurface && m_vHitTrans[i]->HitableGetItemType() != eItemRamp && m_vHitTrans[i]->HitableGetItemType() != eItemRubber && m_vHitTrans[i]->HitableGetItemType() != eItemLight && m_vHitTrans[i]->HitableGetItemType() != eItemFlasher)
            m_vHitTrans[i]->RenderDynamic();
      m_pin3d.m_gpu_profiler.Timestamp(GTS_Else);

      // Draw transparent Walls, Ramps, Rubbers.
      for (size_t i = 0; i < m_vHitTrans.size(); ++i)
         if (m_vHitTrans[i]->HitableGetItemType() == eItemSurface || m_vHitTrans[i]->HitableGetItemType() == eItemRamp || m_vHitTrans[i]->HitableGetItemType() == eItemRubber)
            m_vHitTrans[i]->RenderDynamic();
      m_pin3d.m_gpu_profiler.Timestamp(GTS_Walls_Ramps_Rubbers_T);

      // Draw transparent Primitives.
      for (size_t i = 0; i < m_vHitTrans.size(); ++i)
         if (m_vHitTrans[i]->HitableGetItemType() == eItemPrimitive)
            m_vHitTrans[i]->RenderDynamic();
      m_pin3d.m_gpu_profiler.Timestamp(GTS_Primitives_T);

      // Draw Lights.
      for (size_t i = 0; i < m_vHitNonTrans.size(); ++i) //!! not necessary??!
         if (m_vHitNonTrans[i]->HitableGetItemType() == eItemLight)
            m_vHitNonTrans[i]->RenderDynamic();
      for (size_t i = 0; i < m_vHitTrans.size(); ++i)
         if (m_vHitTrans[i]->HitableGetItemType() == eItemLight)
            m_vHitTrans[i]->RenderDynamic();
      m_pin3d.m_gpu_profiler.Timestamp(GTS_Lights);

      // Draw Flashers.
      m_dmdstate = 0;
      for (size_t i = 0; i < m_vHitTrans.size(); ++i)
         if (!m_vHitTrans[i]->IsDMD() && m_vHitTrans[i]->HitableGetItemType() == eItemFlasher)
            m_vHitTrans[i]->RenderDynamic();
      m_dmdstate = 1;
      for (size_t i = 0; i < m_vHitNonTrans.size(); ++i)
         if (m_vHitNonTrans[i]->IsDMD() && m_vHitNonTrans[i]->HitableGetItemType() == eItemFlasher)
            m_vHitNonTrans[i]->RenderDynamic();
      m_pin3d.m_gpu_profiler.Timestamp(GTS_Flashers);

      // Unused so far.
      m_pin3d.m_gpu_profiler.Timestamp(GTS_UNUSED); //!!
   }

   m_dmdstate = 0;

   //

   m_pin3d.m_pd3dPrimaryDevice->basicShader->SetTexture("Texture3", (D3DTexture*)NULL); // need to reset the bulb light texture, as its used as render target for bloom again

   m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::DEPTHBIAS, 0); //!! paranoia set of old state, remove as soon as sure that no other code still relies on that legacy set
   m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::ZWRITEENABLE, RenderDevice::RS_TRUE);
   m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::BLENDOP, RenderDevice::BLENDOP_ADD);
   m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_CCW);

   if (!m_cameraMode)
   {
      // Draw the mixer volume.
      mixer_draw();
      // Debug draw of plumb.
      plumb_draw();
   }
}

void Player::SetClipPlanePlayfield(const bool clip_orientation)
{
	Matrix3D mT = m_pin3d.m_proj.m_matrixTotal; // = world * view * proj
	mT.Invert();
	mT.Transpose();
	D3DXMATRIX m(mT);
	D3DXPLANE clipSpacePlane;
	const D3DXPLANE plane(0.0f, 0.0f, clip_orientation ? -1.0f : 1.0f, clip_orientation ? m_ptable->m_tableheight : -m_ptable->m_tableheight);
	D3DXPlaneTransform(&clipSpacePlane, &plane, &m);
	m_pin3d.m_pd3dPrimaryDevice->GetCoreDevice()->SetClipPlane(0, clipSpacePlane);
}

void Player::SSRefl()
{
   m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget(m_pin3d.m_pd3dPrimaryDevice->GetReflectionBufferTexture());

   m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture("Texture0", m_pin3d.m_pd3dPrimaryDevice->GetBackBufferTexture());
   m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture("Texture3", m_pin3d.m_pdds3DZBuffer);
   m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture("Texture4", &m_pin3d.m_aoDitherTexture, true); //!!!

   const vec4 w_h_height((float)(1.0 / (double)m_width), (float)(1.0 / (double)m_height), 1.0f/*radical_inverse(m_overall_frames%2048)*/, 1.0f);
   m_pin3d.m_pd3dPrimaryDevice->FBShader->SetVector("w_h_height", &w_h_height);

   const float rotation = fmodf(m_ptable->m_BG_rotation[m_ptable->m_BG_current_set], 360.f);
   const vec4 SSR_bumpHeight_fresnelRefl_scale_FS(0.3f, 0.3f, m_ptable->m_SSRScale, rotation);
   m_pin3d.m_pd3dPrimaryDevice->FBShader->SetVector("SSR_bumpHeight_fresnelRefl_scale_FS", &SSR_bumpHeight_fresnelRefl_scale_FS);

   m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTechnique("SSReflection");

   m_pin3d.m_pd3dPrimaryDevice->FBShader->Begin(0);
   m_pin3d.m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad();
   m_pin3d.m_pd3dPrimaryDevice->FBShader->End();
}

void Player::Bloom()
{
   if (m_ptable->m_bloom_strength <= 0.0f || m_bloomOff)
   {
      // need to reset content from (optional) bulb light abuse of the buffer
      /*m_pin3d.m_pd3dDevice->SetRenderTarget(m_pin3d.m_pd3dDevice->GetBloomBufferTexture());
      m_pin3d.m_pd3dDevice->Clear(0, NULL, clearType::TARGET, 0, 1.0f, 0L);*/

      return;
   }

   const float shiftedVerts[4 * 5] =
   {
       1.0f,  1.0f, 0.0f, 1.0f + (float)(1.0 / (double)m_width), 0.0f + (float)(1.0 / (double)m_height),
      -1.0f,  1.0f, 0.0f, 0.0f + (float)(1.0 / (double)m_width), 0.0f + (float)(1.0 / (double)m_height),
       1.0f, -1.0f, 0.0f, 1.0f + (float)(1.0 / (double)m_width), 1.0f + (float)(1.0 / (double)m_height),
      -1.0f, -1.0f, 0.0f, 0.0f + (float)(1.0 / (double)m_width), 1.0f + (float)(1.0 / (double)m_height)
   };

   {
      // switch to 'bloom' output buffer to collect clipped framebuffer values
      m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget(m_pin3d.m_pd3dPrimaryDevice->GetBloomBufferTexture());

      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture("Texture0", m_pin3d.m_pd3dPrimaryDevice->GetBackBufferTexture());

      const vec4 fb_inv_resolution((float)(1.0 / (double)m_width), (float)(1.0 / (double)m_height), 1.0f, 1.0f);
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetVector("w_h_height", &fb_inv_resolution);
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTechnique("fb_bloom");

      m_pin3d.m_pd3dPrimaryDevice->FBShader->Begin(0);
      m_pin3d.m_pd3dPrimaryDevice->DrawTexturedQuad((Vertex3D_TexelOnly*)shiftedVerts);
      m_pin3d.m_pd3dPrimaryDevice->FBShader->End();
   }
   {
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture("Texture0", (D3DTexture*)NULL);

      // switch to 'bloom' temporary output buffer for horizontal phase of gaussian blur
      m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget(m_pin3d.m_pd3dPrimaryDevice->GetBloomTmpBufferTexture());

      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture("Texture0", m_pin3d.m_pd3dPrimaryDevice->GetBloomBufferTexture());
      const vec4 fb_inv_resolution_05((float)(4.0 / (double)m_width), (float)(4.0 / (double)m_height), 1.0f, 1.0f);
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetVector("w_h_height", &fb_inv_resolution_05);
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTechnique(/*m_low_quality_bloom ? "fb_bloom_horiz9x9" :*/ "fb_bloom_horiz39x39");

      m_pin3d.m_pd3dPrimaryDevice->FBShader->Begin(0);
      m_pin3d.m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad();
      m_pin3d.m_pd3dPrimaryDevice->FBShader->End();
   }
   {
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture("Texture0", (D3DTexture*)NULL);

      // switch to 'bloom' output buffer for vertical phase of gaussian blur
      m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget(m_pin3d.m_pd3dPrimaryDevice->GetBloomBufferTexture());

      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture("Texture0", m_pin3d.m_pd3dPrimaryDevice->GetBloomTmpBufferTexture());
      const vec4 fb_inv_resolution_05((float)(4.0 / (double)m_width), (float)(4.0 / (double)m_height), m_ptable->m_bloom_strength, 1.0f);
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetVector("w_h_height", &fb_inv_resolution_05);
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTechnique(/*m_low_quality_bloom ? "fb_bloom_vert9x9" :*/ "fb_bloom_vert39x39");

      m_pin3d.m_pd3dPrimaryDevice->FBShader->Begin(0);
      m_pin3d.m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad();
      m_pin3d.m_pd3dPrimaryDevice->FBShader->End();
   }
}

void Player::StereoFXAA(const bool stereo, const bool SMAA, const bool DLAA, const bool NFAA, const bool FXAA1, const bool FXAA2, const bool FXAA3, const bool depth_available) //!! SMAA, luma sharpen, dither?
{
   if (stereo) // stereo implicitly disables FXAA
   {
      m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget(m_pin3d.m_pd3dPrimaryDevice->GetOutputBackBuffer());

      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture("Texture0", m_pin3d.m_pd3dPrimaryDevice->GetBackBufferTmpTexture());
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture("Texture3", m_pin3d.m_pdds3DZBuffer);

      const vec4 ms_zpd_ya_td(m_ptable->GetMaxSeparation(), m_ptable->GetZPD(), m_stereo3DY ? 1.0f : 0.0f, (m_stereo3D == 3) ? 2.0f : (m_stereo3D == 1) ? 1.0f : 0.0f);
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetVector("ms_zpd_ya_td", &ms_zpd_ya_td);

      const vec4 w_h_height((float)(1.0 / (double)m_width), (float)(1.0 / (double)m_height), (float)m_height, m_ptable->Get3DOffset());
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetVector("w_h_height", &w_h_height);

      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTechnique("stereo");

      m_pin3d.m_pd3dPrimaryDevice->FBShader->Begin(0);
      m_pin3d.m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad();
      m_pin3d.m_pd3dPrimaryDevice->FBShader->End();
   }
   else if (SMAA || DLAA || NFAA || FXAA1 || FXAA2 || FXAA3)
   {
      if(SMAA || DLAA)
         m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget(m_pin3d.m_pd3dPrimaryDevice->GetBackBufferTexture());
      else
         m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget(m_pin3d.m_pd3dPrimaryDevice->GetOutputBackBuffer());

      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture("Texture0", m_pin3d.m_pd3dPrimaryDevice->GetBackBufferTmpTexture());
      if (depth_available)
         m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture("Texture3", m_pin3d.m_pdds3DZBuffer);

      const vec4 w_h_height((float)(1.0 / (double)m_width), (float)(1.0 / (double)m_height), (float)m_width, depth_available ? 1.f : 0.f);
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetVector("w_h_height", &w_h_height);

      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTechnique(SMAA ? "SMAA_ColorEdgeDetection" : (DLAA ? "DLAA_edge" : (NFAA ? "NFAA" : (FXAA3 ? "FXAA3" : (FXAA2 ? "FXAA2" : "FXAA1")))));

      m_pin3d.m_pd3dPrimaryDevice->FBShader->Begin(0);
      m_pin3d.m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad();
      m_pin3d.m_pd3dPrimaryDevice->FBShader->End();

      if (SMAA || DLAA) // actual SMAA/DLAA filtering pass, above only edge detection
      {
         if(SMAA)
            m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget(m_pin3d.m_pd3dPrimaryDevice->GetBackBufferTmpTexture2());
         else
            m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget(m_pin3d.m_pd3dPrimaryDevice->GetOutputBackBuffer());

         if (SMAA)
         {
             CHECKD3D(m_pin3d.m_pd3dPrimaryDevice->FBShader->Core()->SetTexture("edgesTex2D", m_pin3d.m_pd3dPrimaryDevice->GetBackBufferTexture())); //!! opt.?
         }
         else
             m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture("Texture0", m_pin3d.m_pd3dPrimaryDevice->GetBackBufferTexture());

         m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTechnique(SMAA ? "SMAA_BlendWeightCalculation" : "DLAA");

         m_pin3d.m_pd3dPrimaryDevice->FBShader->Begin(0);
         m_pin3d.m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad();
         m_pin3d.m_pd3dPrimaryDevice->FBShader->End();

         if (SMAA)
         {
            CHECKD3D(m_pin3d.m_pd3dPrimaryDevice->FBShader->Core()->SetTexture("edgesTex2D", NULL)); //!! opt.??

            m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget(m_pin3d.m_pd3dPrimaryDevice->GetOutputBackBuffer());

            CHECKD3D(m_pin3d.m_pd3dPrimaryDevice->FBShader->Core()->SetTexture("blendTex2D", m_pin3d.m_pd3dPrimaryDevice->GetBackBufferTmpTexture2())); //!! opt.?

            m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTechnique("SMAA_NeighborhoodBlending");

            m_pin3d.m_pd3dPrimaryDevice->FBShader->Begin(0);
            m_pin3d.m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad();
            m_pin3d.m_pd3dPrimaryDevice->FBShader->End();

            CHECKD3D(m_pin3d.m_pd3dPrimaryDevice->FBShader->Core()->SetTexture("blendTex2D", NULL)); //!! opt.?
         }
      }
   }
}

#ifdef USE_IMGUI
// call UpddateHUD_IMGUI outside of m_pin3d.m_pd3dPrimaryDevice->BeginScene()/EndSecene()
void Player::UpdateHUD_IMGUI()
{
   static bool profiling = false;
   if (!ShowFPS() || m_cameraMode || m_closeDown)
      return;

   ImGui_ImplDX9_NewFrame();
   ImGui_ImplWin32_NewFrame();
   ImGui::NewFrame();
   ImGui::SetNextWindowSize(ImVec2(600, 350), ImGuiCond_FirstUseEver);
   ImGui::SetNextWindowPos(ImVec2(10, 10));
   ImGui::Begin("Statistics");
   if (ImGui::Button("Toggle Profiling"))
      profiling = !profiling;

   const float fpsAvg = (m_fpsCount == 0) ? 0.0f : m_fpsAvg / m_fpsCount;
   ImGui::Text("FPS: % .1f (% .1f avg)  Display % s Objects(% uk / % uk Triangles)", m_fps + 0.01f, fpsAvg + 0.01f, RenderStaticOnly() ? "only static" : "all", (m_pin3d.m_pd3dPrimaryDevice->m_stats_drawn_triangles + 999) / 1000, (stats_drawn_static_triangles + m_pin3d.m_pd3dPrimaryDevice->m_stats_drawn_triangles + 999) / 1000);
   ImGui::Text("DayNight %u%%", quantizeUnsignedPercent(m_globalEmissionScale));

   const U32 period = m_lastFrameDuration;
   if (period > m_max || m_time_msec - m_lastMaxChangeTime > 1000)
      m_max = period;
   if (period > m_max_total && period < 100000)
      m_max_total = period;

   if (m_phys_period - m_script_period > m_phys_max || m_time_msec - m_lastMaxChangeTime > 1000)
      m_phys_max = m_phys_period - m_script_period;
   if (m_phys_period - m_script_period > m_phys_max_total)
      m_phys_max_total = m_phys_period - m_script_period;

   if (m_phys_iterations > m_phys_max_iterations || m_time_msec - m_lastMaxChangeTime > 1000)
      m_phys_max_iterations = m_phys_iterations;

   if (m_script_period > m_script_max || m_time_msec - m_lastMaxChangeTime > 1000)
      m_script_max = m_script_period;
   if (m_script_period > m_script_max_total)
      m_script_max_total = m_script_period;

   if (m_time_msec - m_lastMaxChangeTime > 1000)
      m_lastMaxChangeTime = m_time_msec;

   if (m_count == 0)
   {
      m_total = period;
      m_phys_total = m_phys_period - m_script_period;
      m_phys_total_iterations = m_phys_iterations;
      m_script_total = m_script_period;
      m_count = 1;
   }
   else
   {
      m_total += period;
      m_phys_total += m_phys_period - m_script_period;
      m_phys_total_iterations += m_phys_iterations;
      m_script_total += m_script_period;
      m_count++;
   }
   ImGui::Text("Overall: %.1f ms (%.1f (%.1f) avg %.1f max)", float(1e-3 * period), float(1e-3 * (double)m_total / (double)m_count), float(1e-3 * m_max), float(1e-3 * m_max_total));
   ImGui::Text("%4.1f%% Physics: %.1f ms (%.1f (%.1f %4.1f%%) avg %.1f max)",
               float((m_phys_period - m_script_period) * 100.0 / period), float(1e-3 * (m_phys_period - m_script_period)),
               float(1e-3 * (double)m_phys_total / (double)m_count), float(1e-3 * m_phys_max), float((double)m_phys_total * 100.0 / (double)m_total), float(1e-3 * m_phys_max_total));

   ImGui::Text("%4.1f%% Scripts: %.1f ms (%.1f (%.1f %4.1f%%) avg %.1f max)",
               float(m_script_period * 100.0 / period), float(1e-3 * m_script_period),
               float(1e-3 * (double)m_script_total / (double)m_count), float(1e-3 * m_script_max), float((double)m_script_total * 100.0 / (double)m_total), float(1e-3 * m_script_max_total));

   // performance counters
   ImGui::Text("Draw calls: %u (%u Locks)", m_pin3d.m_pd3dPrimaryDevice->Perf_GetNumDrawCalls(), m_pin3d.m_pd3dPrimaryDevice->Perf_GetNumLockCalls());
   ImGui::Text("State changes: %u", m_pin3d.m_pd3dPrimaryDevice->Perf_GetNumStateChanges());
   ImGui::Text("Texture changes: %u (%u Uploads)", m_pin3d.m_pd3dPrimaryDevice->Perf_GetNumTextureChanges(), m_pin3d.m_pd3dPrimaryDevice->Perf_GetNumTextureUploads());
   ImGui::Text("Shader/Parameter changes: %u / %u (%u Material ID changes)", m_pin3d.m_pd3dPrimaryDevice->Perf_GetNumTechniqueChanges(), m_pin3d.m_pd3dPrimaryDevice->Perf_GetNumParameterChanges(), material_flips);
   ImGui::Text("Objects: %u Transparent, %u Solid", (unsigned int)m_vHitTrans.size(), (unsigned int)m_vHitNonTrans.size());

   ImGui::Text("Physics: %u iterations per frame (%u avg %u max)    Ball Velocity / Ang.Vel.: %.1f %.1f",
      m_phys_iterations,
      (U32)(m_phys_total_iterations / m_count),
      m_phys_max_iterations,
      m_pactiveball ? (m_pactiveball->m_d.m_vel + (float)PHYS_FACTOR * m_gravity).Length() : -1.f, m_pactiveball ? (m_pactiveball->m_angularmomentum / m_pactiveball->Inertia()).Length() : -1.f);

   ImGui::Text("Left Flipper keypress to rotate: %.1f ms (%d f) to eos: %.1f ms (%d f)",
      (INT64)(m_pininput.m_leftkey_down_usec_rotate_to_end - m_pininput.m_leftkey_down_usec) < 0 ? int_as_float(0x7FC00000) : (double)(m_pininput.m_leftkey_down_usec_rotate_to_end - m_pininput.m_leftkey_down_usec) / 1000.,
      (int)(m_pininput.m_leftkey_down_frame_rotate_to_end - m_pininput.m_leftkey_down_frame) < 0 ? -1 : (int)(m_pininput.m_leftkey_down_frame_rotate_to_end - m_pininput.m_leftkey_down_frame),
      (INT64)(m_pininput.m_leftkey_down_usec_EOS - m_pininput.m_leftkey_down_usec) < 0 ? int_as_float(0x7FC00000) : (double)(m_pininput.m_leftkey_down_usec_EOS - m_pininput.m_leftkey_down_usec) / 1000.,
      (int)(m_pininput.m_leftkey_down_frame_EOS - m_pininput.m_leftkey_down_frame) < 0 ? -1 : (int)(m_pininput.m_leftkey_down_frame_EOS - m_pininput.m_leftkey_down_frame));
   ImGui::End();

   if (profiling)
   {
      ImGui::Begin("Detailed (approximate) GPU profiling:");

      m_pin3d.m_gpu_profiler.WaitForDataAndUpdate();

      double dTDrawTotal = 0.0;
      for (GTS gts = GTS_BeginFrame; gts < GTS_EndFrame; gts = GTS(gts + 1))
         dTDrawTotal += m_pin3d.m_gpu_profiler.DtAvg(gts);

      ImGui::Text(" Draw time: %.2f ms", float(1000.0 * dTDrawTotal));
      for (GTS gts = GTS(GTS_BeginFrame + 1); gts < GTS_EndFrame; gts = GTS(gts + 1))
         ImGui::Text("   %s: %.2f ms (%4.1f%%)", GTS_name[gts], float(1000.0 * m_pin3d.m_gpu_profiler.DtAvg(gts)), float(100. * m_pin3d.m_gpu_profiler.DtAvg(gts) / dTDrawTotal));
      ImGui::Text(" Frame time: %.2f ms", float(1000.0 * (dTDrawTotal + m_pin3d.m_gpu_profiler.DtAvg(GTS_EndFrame))));

/*      if (ProfilingMode() == 1)
      {
         for (GTS gts = GTS(GTS_BeginFrame + 1); gts < GTS_EndFrame; gts = GTS(gts + 1))
         {
            sprintf_s(szFoo, "   %s: %.2f ms (%4.1f%%)", GTS_name[gts], float(1000.0 * m_pin3d.m_gpu_profiler.DtAvg(gts)), float(100. * m_pin3d.m_gpu_profiler.DtAvg(gts) / dTDrawTotal));
            DebugPrint(0, 320 + gts * 20, szFoo);
         }
         sprintf_s(szFoo, " Frame time: %.2f ms", float(1000.0 * (dTDrawTotal + m_pin3d.m_gpu_profiler.DtAvg(GTS_EndFrame))));
         DebugPrint(0, 320 + GTS_EndFrame * 20, szFoo);
      }
      else
      {
         for (GTS gts = GTS(GTS_BeginFrame + 1); gts < GTS_EndFrame; gts = GTS(gts + 1))
         {
            sprintf_s(szFoo, " %s: %.2f ms (%4.1f%%)", GTS_name_item[gts], float(1000.0 * m_pin3d.m_gpu_profiler.DtAvg(gts)), float(100. * m_pin3d.m_gpu_profiler.DtAvg(gts) / dTDrawTotal));
            DebugPrint(0, 300 + gts * 20, szFoo);
         }
      }
      */
      ImGui::End();
   }

   ImGui::SetNextWindowSize(ImVec2(530, 550), ImGuiCond_FirstUseEver);
   ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_FirstUseEver);
   ImGui::Begin("Plots");
       //!! This example assumes 60 FPS. Higher FPS requires larger buffer size.
       static ScrollingData sdata1, sdata2, sdata3, sdata4, sdata5, sdata6;
       //static RollingData   rdata1, rdata2;
       static double t = 0.f;
       t += ImGui::GetIO().DeltaTime;

       sdata6.AddPoint((float)t, float(1e-3 * m_script_period) * 1.f);
       sdata5.AddPoint((float)t, sdata5.GetLast().y*0.95f + sdata6.GetLast().y*0.05f);

       sdata4.AddPoint((float)t, float(1e-3 * (m_phys_period - m_script_period)) * 5.f);
       sdata3.AddPoint((float)t, sdata3.GetLast().y*0.95f + sdata4.GetLast().y*0.05f);

       sdata2.AddPoint((float)t, m_fps * 0.003f);
       //rdata2.AddPoint((float)t, m_fps * 0.003f);
       sdata1.AddPoint((float)t, sdata1.GetLast().y*0.95f + sdata2.GetLast().y*0.05f);
       //rdata1.AddPoint((float)t, sdata1.GetLast().y*0.95f + sdata2.GetLast().y*0.05f);

       static float history = 2.5f;
       ImGui::SliderFloat("History", &history, 1, 10, "%.1f s");
       //rdata1.Span = history;
       //rdata2.Span = history;
       ImPlot::SetNextPlotLimitsX(t - history, t, ImGuiCond_Always);
       const int rt_axis = ImPlotAxisFlags_NoTickLabels;
       if (ImPlot::BeginPlot("##ScrollingFPS", NULL, NULL, ImVec2(-1, 150), ImPlotFlags_None, rt_axis, rt_axis | ImPlotAxisFlags_LockMin)) {
           ImPlot::PlotLine("FPS", &sdata2.Data[0].x, &sdata2.Data[0].y, sdata2.Data.size(), sdata2.Offset, 2 * sizeof(float));
           ImPlot::PushStyleColor(ImPlotCol_Fill, ImVec4(1, 0, 0, 0.25f));
           ImPlot::PlotLine("Smoothed FPS", &sdata1.Data[0].x, &sdata1.Data[0].y, sdata1.Data.size(), sdata1.Offset, 2 * sizeof(float));
           ImPlot::PopStyleColor();
           ImPlot::EndPlot();
       }
       /*ImPlot::SetNextPlotLimitsX(0, history, ImGuiCond_Always);
       if (ImPlot::BeginPlot("##RollingFPS", NULL, NULL, ImVec2(-1, 150), ImPlotFlags_Default, rt_axis, rt_axis)) {
           ImPlot::PlotLine("Average FPS", &rdata1.Data[0].x, &rdata1.Data[0].y, rdata1.Data.size(), 0, 2 * sizeof(float));
           ImPlot::PlotLine("FPS", &rdata2.Data[0].x, &rdata2.Data[0].y, rdata2.Data.size(), 0, 2 * sizeof(float));
           ImPlot::EndPlot();
       }*/
       ImPlot::SetNextPlotLimitsX(t - history, t, ImGuiCond_Always);
       if (ImPlot::BeginPlot("##ScrollingPhysics", NULL, NULL, ImVec2(-1, 150), ImPlotFlags_None, rt_axis, rt_axis | ImPlotAxisFlags_LockMin)) {
           ImPlot::PlotLine("ms Physics", &sdata4.Data[0].x, &sdata4.Data[0].y, sdata4.Data.size(), sdata4.Offset, 2 * sizeof(float));
           ImPlot::PushStyleColor(ImPlotCol_Fill, ImVec4(1, 0, 0, 0.25f));
           ImPlot::PlotLine("Smoothed ms Physics", &sdata3.Data[0].x, &sdata3.Data[0].y, sdata3.Data.size(), sdata3.Offset, 2 * sizeof(float));
           ImPlot::PopStyleColor();
           ImPlot::EndPlot();
       }
       ImPlot::SetNextPlotLimitsX(t - history, t, ImGuiCond_Always);
       if (ImPlot::BeginPlot("##ScrollingScript", NULL, NULL, ImVec2(-1, 150), ImPlotFlags_None, rt_axis, rt_axis | ImPlotAxisFlags_LockMin)) {
           ImPlot::PlotLine("ms Script", &sdata6.Data[0].x, &sdata6.Data[0].y, sdata6.Data.size(), sdata6.Offset, 2 * sizeof(float));
           ImPlot::PushStyleColor(ImPlotCol_Fill, ImVec4(1, 0, 0, 0.25f));
           ImPlot::PlotLine("Smoothed ms Script", &sdata5.Data[0].x, &sdata5.Data[0].y, sdata5.Data.size(), sdata5.Offset, 2 * sizeof(float));
           ImPlot::PopStyleColor();
           ImPlot::EndPlot();
       }
   ImGui::End();

   ImGui::EndFrame();
}

void Player::RenderHUD_IMGUI()
{
   if (!ShowFPS() || m_cameraMode || m_closeDown)
      return;

   ImGui::Render();
   ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

#else

void Player::UpdateHUD()
{
    // set debug output pos for left aligned text
    float x = 0.f, y = 0.f;
    if (m_ptable->m_BG_rotation[m_ptable->m_BG_current_set] == 270.0f)
    {
        x = 0.0f;
        y = (float)(m_height - DBG_SPRITE_SIZE);
    }
    else if (m_ptable->m_BG_rotation[m_ptable->m_BG_current_set] == 90.0f)
    {
        x = (float)(m_width - DBG_SPRITE_SIZE);
        y = 0.0f;
    }
    SetDebugOutputPosition(x, y);

	// draw all kinds of stats, incl. FPS counter
	if (ShowFPS() && !m_cameraMode && !m_closeDown)
	{
		char szFoo[256];

		// Draw the amount of video memory used.
		//!! Disabled until we can compute this correctly.
		//int len = sprintf_s(szFoo, " Used Graphics Memory: %.2f MB ", (float)NumVideoBytes / (float)(1024 * 1024));
		// TextOut(hdcNull, 10, 30, szFoo, len);

		// Draw the framerate.
		const float fpsAvg = (m_fpsCount == 0) ? 0.0f : m_fpsAvg / m_fpsCount;
		sprintf_s(szFoo, "FPS: %.1f (%.1f avg)  Display %s Objects (%uk/%uk Triangles)  DayNight %u%%", m_fps+0.01f, fpsAvg+0.01f, RenderStaticOnly() ? "only static" : "all", (m_pin3d.m_pd3dPrimaryDevice->m_stats_drawn_triangles + 999) / 1000, (stats_drawn_static_triangles + m_pin3d.m_pd3dPrimaryDevice->m_stats_drawn_triangles + 999) / 1000, quantizeUnsignedPercent(m_globalEmissionScale));
		DebugPrint(0, 10, szFoo);

		const U32 period = m_lastFrameDuration;
		if (period > m_max || m_time_msec - m_lastMaxChangeTime > 1000)
			m_max = period;
		if (period > m_max_total && period < 100000)
			m_max_total = period;

		if (m_phys_period-m_script_period > m_phys_max || m_time_msec - m_lastMaxChangeTime > 1000)
			m_phys_max = m_phys_period-m_script_period;
		if (m_phys_period-m_script_period > m_phys_max_total)
			m_phys_max_total = m_phys_period-m_script_period;

		if (m_phys_iterations > m_phys_max_iterations || m_time_msec - m_lastMaxChangeTime > 1000)
			m_phys_max_iterations = m_phys_iterations;

		if (m_script_period > m_script_max || m_time_msec - m_lastMaxChangeTime > 1000)
			m_script_max = m_script_period;
		if (m_script_period > m_script_max_total)
			m_script_max_total = m_script_period;

		if (m_time_msec - m_lastMaxChangeTime > 1000)
			m_lastMaxChangeTime = m_time_msec;

		if (m_count == 0)
		{
			m_total = period;
			m_phys_total = m_phys_period-m_script_period;
			m_phys_total_iterations = m_phys_iterations;
			m_script_total = m_script_period;
			m_count = 1;
		}
		else
		{
			m_total += period;
			m_phys_total += m_phys_period-m_script_period;
			m_phys_total_iterations += m_phys_iterations;
			m_script_total += m_script_period;
			m_count++;
		}

		int len = sprintf_s(szFoo, "Overall: %.1f ms (%.1f (%.1f) avg %.1f max)",
			float(1e-3*period), float(1e-3 * (double)m_total / (double)m_count), float(1e-3*m_max), float(1e-3*m_max_total));
		DebugPrint(0, 30, szFoo);
		len = sprintf_s(szFoo, "%4.1f%% Physics: %.1f ms (%.1f (%.1f %4.1f%%) avg %.1f max)",
			float((m_phys_period-m_script_period)*100.0 / period), float(1e-3*(m_phys_period-m_script_period)),
			float(1e-3 * (double)m_phys_total / (double)m_count), float(1e-3*m_phys_max), float((double)m_phys_total*100.0 / (double)m_total), float(1e-3*m_phys_max_total));
		DebugPrint(0, 50, szFoo);
		len = sprintf_s(szFoo, "%4.1f%% Scripts: %.1f ms (%.1f (%.1f %4.1f%%) avg %.1f max)",
			float(m_script_period*100.0 / period), float(1e-3*m_script_period),
			float(1e-3 * (double)m_script_total / (double)m_count), float(1e-3*m_script_max), float((double)m_script_total*100.0 / (double)m_total), float(1e-3*m_script_max_total));
		DebugPrint(0, 70, szFoo);

		// performance counters
		len = sprintf_s(szFoo, "Draw calls: %u (%u Locks)", m_pin3d.m_pd3dPrimaryDevice->Perf_GetNumDrawCalls(), m_pin3d.m_pd3dPrimaryDevice->Perf_GetNumLockCalls());
		DebugPrint(0, 95, szFoo);
		len = sprintf_s(szFoo, "State changes: %u", m_pin3d.m_pd3dPrimaryDevice->Perf_GetNumStateChanges());
		DebugPrint(0, 115, szFoo);
		len = sprintf_s(szFoo, "Texture changes: %u (%u Uploads)", m_pin3d.m_pd3dPrimaryDevice->Perf_GetNumTextureChanges(), m_pin3d.m_pd3dPrimaryDevice->Perf_GetNumTextureUploads());
		DebugPrint(0, 135, szFoo);
		len = sprintf_s(szFoo, "Shader/Parameter changes: %u / %u (%u Material ID changes)", m_pin3d.m_pd3dPrimaryDevice->Perf_GetNumTechniqueChanges(), m_pin3d.m_pd3dPrimaryDevice->Perf_GetNumParameterChanges(), material_flips);
		DebugPrint(0, 155, szFoo);
		len = sprintf_s(szFoo, "Objects: %u Transparent, %u Solid", (unsigned int)m_vHitTrans.size(), (unsigned int)m_vHitNonTrans.size());
		DebugPrint(0, 175, szFoo);

		len = sprintf_s(szFoo, "Physics: %u iterations per frame (%u avg %u max)    Ball Velocity / Ang.Vel.: %.1f %.1f",
			m_phys_iterations,
			(U32)(m_phys_total_iterations / m_count),
			m_phys_max_iterations,
			m_pactiveball ? (m_pactiveball->m_d.m_vel + (float)PHYS_FACTOR*m_gravity).Length() : -1.f, m_pactiveball ? (m_pactiveball->m_angularmomentum / m_pactiveball->Inertia()).Length() : -1.f);
		DebugPrint(0, 200, szFoo);

#ifdef DEBUGPHYSICS
#ifdef C_DYNAMIC
		len = sprintf_s(szFoo, "Hits:%5u Collide:%5u Ctacs:%5u Static:%5u Embed:%5u TimeSearch:%5u",
			c_hitcnts, c_collisioncnt, c_contactcnt, c_staticcnt, c_embedcnts, c_timesearch);
#else
		len = sprintf_s(szFoo, "Hits:%5u Collide:%5u Ctacs:%5u Embed:%5u TimeSearch:%5u",
			c_hitcnts, c_collisioncnt, c_contactcnt, c_embedcnts, c_timesearch);
#endif
		DebugPrint(0, 220, szFoo);

		len = sprintf_s(szFoo, "kDObjects: %5u kD:%5u QuadObjects: %5u Quadtree:%5u Traversed:%5u Tested:%5u DeepTested:%5u",
			c_kDObjects, c_kDNextlevels, c_quadObjects, c_quadNextlevels, c_traversed, c_tested, c_deepTested);
		DebugPrint(0, 240, szFoo);
#endif

		len = sprintf_s(szFoo, "Left Flipper keypress to rotate: %.1f ms (%d f) to eos: %.1f ms (%d f)",
			(INT64)(m_pininput.m_leftkey_down_usec_rotate_to_end - m_pininput.m_leftkey_down_usec) < 0 ? int_as_float(0x7FC00000) : (double)(m_pininput.m_leftkey_down_usec_rotate_to_end - m_pininput.m_leftkey_down_usec) / 1000.,
			(int)(m_pininput.m_leftkey_down_frame_rotate_to_end - m_pininput.m_leftkey_down_frame) < 0 ? -1 : (int)(m_pininput.m_leftkey_down_frame_rotate_to_end - m_pininput.m_leftkey_down_frame),
			(INT64)(m_pininput.m_leftkey_down_usec_EOS - m_pininput.m_leftkey_down_usec) < 0 ? int_as_float(0x7FC00000) : (double)(m_pininput.m_leftkey_down_usec_EOS - m_pininput.m_leftkey_down_usec) / 1000.,
			(int)(m_pininput.m_leftkey_down_frame_EOS - m_pininput.m_leftkey_down_frame) < 0 ? -1 : (int)(m_pininput.m_leftkey_down_frame_EOS - m_pininput.m_leftkey_down_frame));
		DebugPrint(0, 260, szFoo);
	}

	// Draw performance readout - at end of CPU frame, so hopefully the previous frame
	//  (whose data we're getting) will have finished on the GPU by now.
	if (ProfilingMode() != 0 && !m_closeDown && !m_cameraMode)
	{
		char szFoo[256];
		sprintf_s(szFoo, "Detailed (approximate) GPU profiling:");
		DebugPrint(0, 300, szFoo);

		m_pin3d.m_gpu_profiler.WaitForDataAndUpdate();

		double dTDrawTotal = 0.0;
		for (GTS gts = GTS_BeginFrame; gts < GTS_EndFrame; gts = GTS(gts + 1))
			dTDrawTotal += m_pin3d.m_gpu_profiler.DtAvg(gts);

		if (ProfilingMode() == 1)
		{
			sprintf_s(szFoo, " Draw time: %.2f ms", float(1000.0 * dTDrawTotal));
			DebugPrint(0, 320, szFoo);
			for (GTS gts = GTS(GTS_BeginFrame + 1); gts < GTS_EndFrame; gts = GTS(gts + 1))
			{
				sprintf_s(szFoo, "   %s: %.2f ms (%4.1f%%)", GTS_name[gts], float(1000.0 * m_pin3d.m_gpu_profiler.DtAvg(gts)), float(100. * m_pin3d.m_gpu_profiler.DtAvg(gts)/dTDrawTotal));
				DebugPrint(0, 320 + gts * 20, szFoo);
			}
			sprintf_s(szFoo, " Frame time: %.2f ms", float(1000.0 * (dTDrawTotal + m_pin3d.m_gpu_profiler.DtAvg(GTS_EndFrame))));
			DebugPrint(0, 320 + GTS_EndFrame * 20, szFoo);
		}
		else
		{
			for (GTS gts = GTS(GTS_BeginFrame + 1); gts < GTS_EndFrame; gts = GTS(gts + 1))
			{
				sprintf_s(szFoo, " %s: %.2f ms (%4.1f%%)", GTS_name_item[gts], float(1000.0 * m_pin3d.m_gpu_profiler.DtAvg(gts)), float(100. * m_pin3d.m_gpu_profiler.DtAvg(gts)/dTDrawTotal));
				DebugPrint(0, 300 + gts * 20, szFoo);
			}
		}
	}

    // set debug output pos for centered text
    x = (float)(m_width - DBG_SPRITE_SIZE)*0.5f;
    if (m_ptable->m_BG_rotation[m_ptable->m_BG_current_set] == 270.0f)
    {
        x = 0.0f;
        y = (float)(m_height - DBG_SPRITE_SIZE)*0.5f;
    }
    else if (m_ptable->m_BG_rotation[m_ptable->m_BG_current_set] == 90.0f)
    {
        x = (float)(m_width - DBG_SPRITE_SIZE);
        y = (float)(m_height - DBG_SPRITE_SIZE)*0.5f;
    }
    SetDebugOutputPosition(x, y);

    if (!m_closeDown && (m_stereo3D != 0) && !m_stereo3Denabled && (usec() < m_StartTime_usec + 4e+6)) // show for max. 4 seconds
    {
        char szFoo[256];
        sprintf_s(szFoo, "3D Stereo is enabled but currently toggled off, press F10 to toggle 3D Stereo on");
        DebugPrint(DBG_SPRITE_SIZE/2, 10, szFoo, true);
    }

    if (!m_closeDown && m_supportsTouch && m_showTouchMessage && (usec() < m_StartTime_usec + 12e+6)) // show for max. 12 seconds
    {
        char szFoo[256];
        sprintf_s(szFoo, "You can use Touch controls on this display: bottom left area to Start Game, bottom right area to use the Plunger");
        DebugPrint(DBG_SPRITE_SIZE/2, 40, szFoo, true);
        sprintf_s(szFoo, "lower left/right for Flippers, upper left/right for Magna buttons, top left for Credits and (hold) top right to Exit"); //!!!!!!!!!!!! Extra Button?
        DebugPrint(DBG_SPRITE_SIZE/2, 70, szFoo, true);

        //!! visualize with real buttons or at least the areas??
    }

	if (m_fullScreen && m_closeDown && !IsWindows10_1803orAbove()) // cannot use dialog boxes in exclusive fullscreen on older windows versions, so necessary
	{
		char szFoo[256];
		sprintf_s(szFoo, "Press 'Enter' to continue or Press 'Q' to exit");
		DebugPrint(DBG_SPRITE_SIZE/2, m_height/2-5, szFoo, true);
	}

	if (m_closeDown) // print table name,author,version and blurb and description in pause mode
	{
		char szFoo[256];
		szFoo[0] = 0;

		int line = 0;

		if ( !m_ptable->m_szTableName.empty() )
			strncat_s(szFoo, m_ptable->m_szTableName.c_str(), sizeof(szFoo)-strnlen_s(szFoo, sizeof(szFoo))-1);
		else
			strncat_s(szFoo, "Table", sizeof(szFoo)-strnlen_s(szFoo, sizeof(szFoo))-1);
		if (!m_ptable->m_szAuthor.empty())
		{
			strncat_s(szFoo, " by ", sizeof(szFoo)-strnlen_s(szFoo, sizeof(szFoo))-1);
			strncat_s(szFoo, m_ptable->m_szAuthor.c_str(), sizeof(szFoo)-strnlen_s(szFoo, sizeof(szFoo))-1);
		}
		if (!m_ptable->m_szVersion.empty())
		{
			strncat_s(szFoo, " (", sizeof(szFoo)-strnlen_s(szFoo, sizeof(szFoo))-1);
			strncat_s(szFoo, m_ptable->m_szVersion.c_str(), sizeof(szFoo)-strnlen_s(szFoo, sizeof(szFoo))-1);
			strncat_s(szFoo, ")", sizeof(szFoo)-strnlen_s(szFoo, sizeof(szFoo))-1);
		}

		char buffer[256];
		sprintf_s(buffer, " (%s Revision %u)", !m_ptable->m_szDateSaved.empty() ? m_ptable->m_szDateSaved.c_str() : "N.A.", m_ptable->m_numTimesSaved);
		strncat_s(szFoo, buffer, sizeof(szFoo)-strnlen_s(szFoo, sizeof(szFoo))-1);

		if (strnlen_s(szFoo,sizeof(szFoo)) > 0)
		{
			DebugPrint(DBG_SPRITE_SIZE/2, line * 20 + 10, szFoo, true);
			line += 2;
			DebugPrint(DBG_SPRITE_SIZE/2, line * 20 + 10, "========================================", true);
			line += 2;
		}

		for (unsigned int i2 = 0; i2 < 2; ++i2)
		{
			const std::string& s = (i2 == 0) ? m_ptable->m_szBlurb : m_ptable->m_szDescription;
			int length = (int)s.length();
			const char *desc = s.c_str();
			while (length > 0)
			{
				unsigned int o = 0;
				for (unsigned int i = 0; i < 100; ++i, ++o)
					if (desc[i] != '\n' && desc[i] != 0)
						szFoo[o] = desc[i];
					else
						break;

				szFoo[o] = 0;

				DebugPrint(DBG_SPRITE_SIZE/2, line * 20 + 10, szFoo, true);

				if (o < 100)
					o++;
				length -= o;
				desc += o;

				line++;
			}

			if (i2 == 0 && !s.empty())
			{
				line++;
				DebugPrint(DBG_SPRITE_SIZE/2, line * 20 + 10, "========================================", true);
				line+=2;
			}
		}
	}
}
#endif

void Player::PrepareVideoBuffersNormal()
{
   const bool useAA = (m_AA && (m_ptable->m_useAA == -1)) || (m_ptable->m_useAA == 1);
   const bool stereo= ((m_stereo3D != 0) && m_stereo3Denabled && m_pin3d.m_pd3dPrimaryDevice->DepthBufferReadBackAvailable());
   const bool SMAA  = (((m_FXAA == Quality_SMAA)  && (m_ptable->m_useFXAA == -1)) || (m_ptable->m_useFXAA == Quality_SMAA));
   const bool DLAA  = (((m_FXAA == Standard_DLAA) && (m_ptable->m_useFXAA == -1)) || (m_ptable->m_useFXAA == Standard_DLAA));
   const bool NFAA  = (((m_FXAA == Fast_NFAA)     && (m_ptable->m_useFXAA == -1)) || (m_ptable->m_useFXAA == Fast_NFAA));
   const bool FXAA1 = (((m_FXAA == Fast_FXAA)     && (m_ptable->m_useFXAA == -1)) || (m_ptable->m_useFXAA == Fast_FXAA));
   const bool FXAA2 = (((m_FXAA == Standard_FXAA) && (m_ptable->m_useFXAA == -1)) || (m_ptable->m_useFXAA == Standard_FXAA));
   const bool FXAA3 = (((m_FXAA == Quality_FXAA)  && (m_ptable->m_useFXAA == -1)) || (m_ptable->m_useFXAA == Quality_FXAA));
   const bool ss_refl = (((m_ss_refl && (m_ptable->m_useSSR == -1)) || (m_ptable->m_useSSR == 1)) && m_pin3d.m_pd3dPrimaryDevice->DepthBufferReadBackAvailable() && m_ptable->m_SSRScale > 0.f);

   if (stereo || ss_refl)
      m_pin3d.m_pd3dPrimaryDevice->CopyDepth(m_pin3d.m_pdds3DZBuffer, m_pin3d.m_pddsZBuffer); // do not put inside BeginScene/EndScene Block

   const float shiftedVerts[4 * 5] =
   {
       1.0f + m_ScreenOffset.x,  1.0f + m_ScreenOffset.y, 0.0f, 1.0f + (float)(1.0 / (double)m_width), 0.0f + (float)(1.0 / (double)m_height),
      -1.0f + m_ScreenOffset.x,  1.0f + m_ScreenOffset.y, 0.0f, 0.0f + (float)(1.0 / (double)m_width), 0.0f + (float)(1.0 / (double)m_height),
       1.0f + m_ScreenOffset.x, -1.0f + m_ScreenOffset.y, 0.0f, 1.0f + (float)(1.0 / (double)m_width), 1.0f + (float)(1.0 / (double)m_height),
      -1.0f + m_ScreenOffset.x, -1.0f + m_ScreenOffset.y, 0.0f, 0.0f + (float)(1.0 / (double)m_width), 1.0f + (float)(1.0 / (double)m_height)
   };

   m_pin3d.m_pd3dPrimaryDevice->BeginScene();

   m_pin3d.DisableAlphaBlend();
   m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_NONE);
   m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::ZWRITEENABLE, RenderDevice::RS_FALSE);
   m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::ZENABLE, RenderDevice::RS_FALSE);

   Bloom();

   if (ProfilingMode() == 1)
      m_pin3d.m_gpu_profiler.Timestamp(GTS_Bloom);

   if (ss_refl)
      SSRefl();

   if (ProfilingMode() == 1)
   {
      m_pin3d.m_gpu_profiler.Timestamp(GTS_SSR);
      m_pin3d.m_gpu_profiler.Timestamp(GTS_AO);
   }

   // switch to output buffer
   if (!(stereo || SMAA || DLAA || NFAA || FXAA1 || FXAA2 || FXAA3))
      m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget(m_pin3d.m_pd3dPrimaryDevice->GetOutputBackBuffer());
   else
      m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget(m_pin3d.m_pd3dPrimaryDevice->GetBackBufferTmpTexture());

   // copy framebuffer over from texture and tonemap/gamma
   if (ss_refl)
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture("Texture0", m_pin3d.m_pd3dPrimaryDevice->GetReflectionBufferTexture());
   else
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture("Texture0", m_pin3d.m_pd3dPrimaryDevice->GetBackBufferTexture());
   if (m_ptable->m_bloom_strength > 0.0f && !m_bloomOff)
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture("Texture1", m_pin3d.m_pd3dPrimaryDevice->GetBloomBufferTexture());

   Texture * const pin = m_ptable->GetImage(m_ptable->m_szImageColorGrade);
   if (pin)
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture("Texture4", pin, false);
   m_pin3d.m_pd3dPrimaryDevice->FBShader->SetBool("color_grade", pin != NULL);
   m_pin3d.m_pd3dPrimaryDevice->FBShader->SetBool("do_dither", !m_ditherOff);
   m_pin3d.m_pd3dPrimaryDevice->FBShader->SetBool("do_bloom", (m_ptable->m_bloom_strength > 0.0f && !m_bloomOff));

   //const unsigned int jittertime = (unsigned int)((U64)msec()*90/1000);
   const float jitter = (float)((msec()&2047)/1000.0);
   const vec4 fb_inv_resolution_05((float)(0.5 / (double)m_width), (float)(0.5 / (double)m_height),
      //1.0f, 1.0f);
      jitter, //radical_inverse(jittertime)*11.0f,
      jitter);//sobol(jittertime)*13.0f); // jitter for dither pattern
   m_pin3d.m_pd3dPrimaryDevice->FBShader->SetVector("w_h_height", &fb_inv_resolution_05);
   m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTechnique(useAA ? "fb_tonemap" : (m_BWrendering == 1 ? "fb_tonemap_no_filterRG" : (m_BWrendering == 2 ? "fb_tonemap_no_filterR" : "fb_tonemap_no_filterRGB")));

   m_pin3d.m_pd3dPrimaryDevice->FBShader->Begin(0);
   m_pin3d.m_pd3dPrimaryDevice->DrawTexturedQuad((Vertex3D_TexelOnly*)shiftedVerts);
   m_pin3d.m_pd3dPrimaryDevice->FBShader->End();

   StereoFXAA(stereo, SMAA, DLAA, NFAA, FXAA1, FXAA2, FXAA3, false);

   if (ProfilingMode() == 1)
      m_pin3d.m_gpu_profiler.Timestamp(GTS_PostProcess);

   m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::ZENABLE, RenderDevice::RS_TRUE);
   m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::ZWRITEENABLE, RenderDevice::RS_TRUE);
   m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_CCW);

   if (m_cameraMode)
       UpdateCameraModeDisplay();

#ifdef USE_IMGUI
   RenderHUD_IMGUI();
#else
   UpdateHUD();
#endif

   m_pin3d.m_pd3dPrimaryDevice->EndScene();
}

void Player::FlipVideoBuffers(const bool vsync)
{
   // display frame
   m_pin3d.Flip(vsync);

   // switch to texture output buffer again
   m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture("Texture0", (D3DTexture*)NULL);
   m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget(m_pin3d.m_pddsBackBuffer);

   m_lastFlipTime = usec();
}

void Player::PrepareVideoBuffersAO()
{
   const bool useAA = (m_AA && (m_ptable->m_useAA == -1)) || (m_ptable->m_useAA == 1);
   const bool stereo= ((m_stereo3D != 0) && m_stereo3Denabled && m_pin3d.m_pd3dPrimaryDevice->DepthBufferReadBackAvailable());
   const bool SMAA  = (((m_FXAA == Quality_SMAA)  && (m_ptable->m_useFXAA == -1)) || (m_ptable->m_useFXAA == Quality_SMAA));
   const bool DLAA  = (((m_FXAA == Standard_DLAA) && (m_ptable->m_useFXAA == -1)) || (m_ptable->m_useFXAA == Standard_DLAA));
   const bool NFAA  = (((m_FXAA == Fast_NFAA)     && (m_ptable->m_useFXAA == -1)) || (m_ptable->m_useFXAA == Fast_NFAA));
   const bool FXAA1 = (((m_FXAA == Fast_FXAA)     && (m_ptable->m_useFXAA == -1)) || (m_ptable->m_useFXAA == Fast_FXAA));
   const bool FXAA2 = (((m_FXAA == Standard_FXAA) && (m_ptable->m_useFXAA == -1)) || (m_ptable->m_useFXAA == Standard_FXAA));
   const bool FXAA3 = (((m_FXAA == Quality_FXAA)  && (m_ptable->m_useFXAA == -1)) || (m_ptable->m_useFXAA == Quality_FXAA));
   const bool ss_refl = (((m_ss_refl && (m_ptable->m_useSSR == -1)) || (m_ptable->m_useSSR == 1)) && m_pin3d.m_pd3dPrimaryDevice->DepthBufferReadBackAvailable() && m_ptable->m_SSRScale > 0.f);

   m_pin3d.m_pd3dPrimaryDevice->CopyDepth(m_pin3d.m_pdds3DZBuffer, m_pin3d.m_pddsZBuffer); // do not put inside BeginScene/EndScene Block

   m_pin3d.m_pd3dPrimaryDevice->BeginScene();

   m_pin3d.DisableAlphaBlend();
   m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_NONE);
   m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::ZWRITEENABLE, RenderDevice::RS_FALSE);
   m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::ZENABLE, RenderDevice::RS_FALSE);

   Bloom();

   if (ProfilingMode() == 1)
      m_pin3d.m_gpu_profiler.Timestamp(GTS_Bloom);

   if (ss_refl)
      SSRefl();

   if (ProfilingMode() == 1)
      m_pin3d.m_gpu_profiler.Timestamp(GTS_SSR);

   // separate normal generation pass, currently roughly same perf or even much worse
   /*m_pin3d.m_pd3dDevice->SetRenderTarget(m_pin3d.m_pd3dDevice->GetBackBufferTmpTexture()); //!! expects stereo or FXAA enabled

   m_pin3d.m_pd3dDevice->FBShader->SetTexture("Texture3", m_pin3d.m_pdds3DZBuffer);
   
   const vec4 w_h_height((float)(1.0 / (double)m_width), (float)(1.0 / (double)m_height),
      radical_inverse(m_overall_frames%2048)*(float)(1. / 8.0),
      sobol(m_overall_frames%2048)*(float)(5. / 8.0)); // jitter within lattice cell //!! ?
   m_pin3d.m_pd3dDevice->FBShader->SetVector("w_h_height", &w_h_height);

   m_pin3d.m_pd3dDevice->FBShader->SetTechnique("normals");

   m_pin3d.m_pd3dDevice->FBShader->Begin(0);
   m_pin3d.m_pd3dDevice->DrawFullscreenTexturedQuad();
   m_pin3d.m_pd3dDevice->FBShader->End();*/

   m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget(m_pin3d.m_pddsAOBackTmpBuffer);

   m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture("Texture0", m_pin3d.m_pddsAOBackBuffer);
   //m_pin3d.m_pd3dDevice->FBShader->SetTexture("Texture1", m_pin3d.m_pd3dDevice->GetBackBufferTmpTexture()); // temporary normals
   m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture("Texture4", &m_pin3d.m_aoDitherTexture, true);
   m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture("Texture3", m_pin3d.m_pdds3DZBuffer);

   const vec4 w_h_height((float)(1.0 / (double)m_width), (float)(1.0 / (double)m_height),
      radical_inverse(m_overall_frames%2048)*(float)(1. / 8.0),
      /*sobol*/radical_inverse<3>(m_overall_frames%2048)*(float)(1. / 8.0)); // jitter within (64/8)x(64/8) neighborhood of 64x64 tex, good compromise between blotches and noise
   m_pin3d.m_pd3dPrimaryDevice->FBShader->SetVector("w_h_height", &w_h_height);
   const vec4 ao_s_tb(m_ptable->m_AOScale, 0.4f, 0.f,0.f); //!! 0.4f: fake global option in video pref? or time dependent? //!! commonly used is 0.1, but would require to clear history for moving stuff
   m_pin3d.m_pd3dPrimaryDevice->FBShader->SetVector("AO_scale_timeblur", &ao_s_tb);

   m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTechnique("AO");

   m_pin3d.m_pd3dPrimaryDevice->FBShader->Begin(0);
   m_pin3d.m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad();
   m_pin3d.m_pd3dPrimaryDevice->FBShader->End();

   if (ProfilingMode() == 1)
      m_pin3d.m_gpu_profiler.Timestamp(GTS_AO);

   // flip AO buffers (avoids copy)
   D3DTexture *tmpAO = m_pin3d.m_pddsAOBackBuffer;
   m_pin3d.m_pddsAOBackBuffer = m_pin3d.m_pddsAOBackTmpBuffer;
   m_pin3d.m_pddsAOBackTmpBuffer = tmpAO;

   // switch to output buffer
   if (!(stereo || SMAA || DLAA || NFAA || FXAA1 || FXAA2 || FXAA3))
      m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget(m_pin3d.m_pd3dPrimaryDevice->GetOutputBackBuffer());
   else
      m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget(m_pin3d.m_pd3dPrimaryDevice->GetBackBufferTmpTexture());

   const float shiftedVerts[4 * 5] =
   {
       1.0f + m_ScreenOffset.x,  1.0f + m_ScreenOffset.y, 0.0f, 1.0f + (float)(1.0 / (double)m_width), 0.0f + (float)(1.0 / (double)m_height),
      -1.0f + m_ScreenOffset.x,  1.0f + m_ScreenOffset.y, 0.0f, 0.0f + (float)(1.0 / (double)m_width), 0.0f + (float)(1.0 / (double)m_height),
       1.0f + m_ScreenOffset.x, -1.0f + m_ScreenOffset.y, 0.0f, 1.0f + (float)(1.0 / (double)m_width), 1.0f + (float)(1.0 / (double)m_height),
      -1.0f + m_ScreenOffset.x, -1.0f + m_ScreenOffset.y, 0.0f, 0.0f + (float)(1.0 / (double)m_width), 1.0f + (float)(1.0 / (double)m_height)
   };

   if (ss_refl)
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture("Texture0", m_pin3d.m_pd3dPrimaryDevice->GetReflectionBufferTexture());
   else
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture("Texture0", m_pin3d.m_pd3dPrimaryDevice->GetBackBufferTexture());
   if (m_ptable->m_bloom_strength > 0.0f && !m_bloomOff)
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture("Texture1", m_pin3d.m_pd3dPrimaryDevice->GetBloomBufferTexture());
   m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture("Texture3", m_pin3d.m_pddsAOBackBuffer);

   Texture * const pin = m_ptable->GetImage(m_ptable->m_szImageColorGrade);
   if (pin)
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture("Texture4", pin, false);
   m_pin3d.m_pd3dPrimaryDevice->FBShader->SetBool("color_grade", pin != NULL);
   m_pin3d.m_pd3dPrimaryDevice->FBShader->SetBool("do_dither", !m_ditherOff);
   m_pin3d.m_pd3dPrimaryDevice->FBShader->SetBool("do_bloom", (m_ptable->m_bloom_strength > 0.0f && !m_bloomOff));

   //const unsigned int jittertime = (unsigned int)((U64)msec()*90/1000);
   const float jitter = (float)((msec()&2047)/1000.0);
   const vec4 fb_inv_resolution_05((float)(0.5 / (double)m_width), (float)(0.5 / (double)m_height),
      //1.0f, 1.0f);
      jitter, //radical_inverse(jittertime)*11.0f,
      jitter);//sobol(jittertime)*13.0f); // jitter for dither pattern
   m_pin3d.m_pd3dPrimaryDevice->FBShader->SetVector("w_h_height", &fb_inv_resolution_05);
   m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTechnique(RenderAOOnly() ? "fb_AO" :
                                                (useAA ? "fb_tonemap_AO" : "fb_tonemap_AO_no_filter"));

   m_pin3d.m_pd3dPrimaryDevice->FBShader->Begin(0);
   m_pin3d.m_pd3dPrimaryDevice->DrawTexturedQuad((Vertex3D_TexelOnly*)shiftedVerts);
   m_pin3d.m_pd3dPrimaryDevice->FBShader->End();

   StereoFXAA(stereo, SMAA, DLAA, NFAA, FXAA1, FXAA2, FXAA3, true);

   if (ProfilingMode() == 1)
      m_pin3d.m_gpu_profiler.Timestamp(GTS_PostProcess);

   //

   m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::ZENABLE, RenderDevice::RS_TRUE);
   m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::ZWRITEENABLE, RenderDevice::RS_TRUE);
   m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_CCW);

   if (m_cameraMode)
       UpdateCameraModeDisplay();

#ifdef USE_IMGUI
   RenderHUD_IMGUI();
#else
   UpdateHUD();
#endif

   m_pin3d.m_pd3dPrimaryDevice->EndScene();
}

void Player::SetScreenOffset(const float x, const float y)
{
   const float rotation = fmodf(m_ptable->m_BG_rotation[m_ptable->m_BG_current_set], 360.f);
   m_ScreenOffset.x = (rotation != 0.0f ? -y : x);
   m_ScreenOffset.y = (rotation != 0.0f ?  x : y);
}

void Player::UpdateBackdropSettings(const bool up)
{
   const float thesign = !up ? -0.2f : 0.2f;

   switch (m_backdropSettingActive)
   {
   case 0:
   {
      m_ptable->m_BG_inclination[m_ptable->m_BG_current_set] += thesign;
      m_ptable->SetNonUndoableDirty(eSaveDirty);
      break;
   }
   case 1:
   {
      m_ptable->m_BG_FOV[m_ptable->m_BG_current_set] += thesign;
      m_ptable->SetNonUndoableDirty(eSaveDirty);
      break;
   }
   case 2:
   {
      m_ptable->m_BG_layback[m_ptable->m_BG_current_set] += thesign;
      m_ptable->SetNonUndoableDirty(eSaveDirty);
      break;
   }
   case 3:
   {
      m_ptable->m_BG_scalex[m_ptable->m_BG_current_set] += 0.01f*thesign;
      m_ptable->m_BG_scaley[m_ptable->m_BG_current_set] += 0.01f*thesign;
      m_ptable->SetNonUndoableDirty(eSaveDirty);
      break;
   }
   case 4:
   {
       m_ptable->m_BG_scalex[m_ptable->m_BG_current_set] += 0.01f*thesign;
       m_ptable->SetNonUndoableDirty(eSaveDirty);
       break;
   }
   case 5:
   {
      m_ptable->m_BG_scaley[m_ptable->m_BG_current_set] += 0.01f*thesign;
      m_ptable->SetNonUndoableDirty(eSaveDirty);
      break;
   }
   case 6:
   {
      m_ptable->m_BG_scalez[m_ptable->m_BG_current_set] += 0.01f*thesign;
      m_ptable->SetNonUndoableDirty(eSaveDirty);
      break;
   }
   case 7:
   {
      m_ptable->m_BG_xlatex[m_ptable->m_BG_current_set] += thesign;
      m_ptable->SetNonUndoableDirty(eSaveDirty);
      break;
   }
   case 8:
   {
      m_ptable->m_BG_xlatey[m_ptable->m_BG_current_set] += thesign;
      m_ptable->SetNonUndoableDirty(eSaveDirty);
      break;
   }
   case 9:
   {
      m_ptable->m_BG_xlatez[m_ptable->m_BG_current_set] += thesign*50.0f;
      m_ptable->SetNonUndoableDirty(eSaveDirty);
      break;
   }
   case 10:
   {
      m_ptable->m_lightEmissionScale += thesign*100000.f;
      if (m_ptable->m_lightEmissionScale < 0.f)
         m_ptable->m_lightEmissionScale = 0.f;
      m_ptable->SetNonUndoableDirty(eSaveDirty);
      break;
   }
   case 11:
   {
      m_ptable->m_lightRange += thesign*1000.f;
      if (m_ptable->m_lightRange < 0.f)
         m_ptable->m_lightRange = 0.f;
      m_ptable->SetNonUndoableDirty(eSaveDirty);
      break;
   }
   case 12:
   {
      m_ptable->m_lightHeight += thesign*100.f;
      if (m_ptable->m_lightHeight < 100.f)
         m_ptable->m_lightHeight = 100.f;
      m_ptable->SetNonUndoableDirty(eSaveDirty);
      break;
   }
   case 13:
   {
      m_ptable->m_envEmissionScale += thesign*0.5f;
      if (m_ptable->m_envEmissionScale < 0.f)
         m_ptable->m_envEmissionScale = 0.f;
      m_ptable->SetNonUndoableDirty(eSaveDirty);
      break;
   }
   }
}

void Player::UpdateCameraModeDisplay()
{
   char szFoo[128];
   int len;

   float x = 0.f, y = 0.f;
   if (m_ptable->m_BG_rotation[m_ptable->m_BG_current_set] == 270.0f)
   {
       x = (float)(m_width - 256);
       y = (float)(m_height - DBG_SPRITE_SIZE-10);
   }
   else if (m_ptable->m_BG_rotation[m_ptable->m_BG_current_set] == 90.0f)
   {
       x = (float)(-DBG_SPRITE_SIZE/1.3);
       y = 0.0f;
   }
   SetDebugOutputPosition(x, y);

   len = sprintf_s(szFoo, "Camera / Light / Material Edit Mode");
   DebugPrint(0, 10, szFoo);
   len = sprintf_s(szFoo, "Left / Right flipper key = decrease / increase value");
   DebugPrint(0, 50, szFoo);
   len = sprintf_s(szFoo, "Left / Right magna save key = previous / next option");
   DebugPrint(0, 70, szFoo);

   switch (m_backdropSettingActive)
   {
   case 0:
   {
      len = sprintf_s(szFoo, "Inclination: %.3f", m_ptable->m_BG_inclination[m_ptable->m_BG_current_set]);
      break;
   }
   case 1:
   {
      len = sprintf_s(szFoo, "Field Of View: %.3f", m_ptable->m_BG_FOV[m_ptable->m_BG_current_set]);
      break;
   }
   case 2:
   {
      len = sprintf_s(szFoo, "Layback: %.3f", m_ptable->m_BG_layback[m_ptable->m_BG_current_set]);
      break;
   }
   case 3:
   {
       len = sprintf_s(szFoo, "X/Y Scale: %.3f / %.3f", m_ptable->m_BG_scalex[m_ptable->m_BG_current_set], m_ptable->m_BG_scaley[m_ptable->m_BG_current_set]);
       break;
   }
   case 4:
   {
      len = sprintf_s(szFoo, "X Scale: %.3f", m_ptable->m_BG_scalex[m_ptable->m_BG_current_set]);
      break;
   }
   case 5:
   {
      len = sprintf_s(szFoo, "Y Scale: %.3f", m_ptable->m_BG_scaley[m_ptable->m_BG_current_set]);
      break;
   }
   case 6:
   {
      len = sprintf_s(szFoo, "Z Scale: %.3f", m_ptable->m_BG_scalez[m_ptable->m_BG_current_set]);
      break;
   }
   case 7:
   {
      len = sprintf_s(szFoo, "X Offset: %.3f", m_ptable->m_BG_xlatex[m_ptable->m_BG_current_set]);
      break;
   }
   case 8:
   {
      len = sprintf_s(szFoo, "Y Offset: %.3f", m_ptable->m_BG_xlatey[m_ptable->m_BG_current_set]);
      break;
   }
   case 9:
   {
      len = sprintf_s(szFoo, "Z Offset: %.3f", m_ptable->m_BG_xlatez[m_ptable->m_BG_current_set]);
      break;
   }
   case 10:
   {
      len = sprintf_s(szFoo, "Light Emission Scale: %.3f", m_ptable->m_lightEmissionScale);
      break;
   }
   case 11:
   {
      len = sprintf_s(szFoo, "Light Range: %.3f", m_ptable->m_lightRange);
      break;
   }
   case 12:
   {
      len = sprintf_s(szFoo, "Light Height: %.3f", m_ptable->m_lightHeight);
      break;
   }
   case 13:
   {
      len = sprintf_s(szFoo, "Environment Emission: %.3f", m_ptable->m_envEmissionScale);
      break;
   }
   default:
   {
      len = sprintf_s(szFoo, "unknown");
   }
   }
   DebugPrint(0, 130, szFoo);
   m_pin3d.InitLayout(m_ptable->m_BG_enable_FSS);
   len = sprintf_s(szFoo, "Camera at X: %f Y: %f Z: %f", -m_pin3d.m_proj.m_matView._41, (m_ptable->m_BG_current_set == 0 || m_ptable->m_BG_current_set == 2) ? m_pin3d.m_proj.m_matView._42 : -m_pin3d.m_proj.m_matView._42, m_pin3d.m_proj.m_matView._43); // DT & FSS
   DebugPrint(0, 110, szFoo);
   len = sprintf_s(szFoo, "Navigate around with the Arrow Keys and Left Alt Key (if enabled in the Key settings)");
   DebugPrint(0, 170, szFoo);
   len = sprintf_s(szFoo, "Use the Debugger / Interactive Editor to change Lights / Materials");
   DebugPrint(0, 210, szFoo);
}

void Player::LockForegroundWindow(const bool enable)
{
#if(_WIN32_WINNT >= 0x0500)
    if (m_fullScreen) // revert special tweaks of exclusive fullscreen app
    {
        if (enable)
        {
            ::LockSetForegroundWindow(LSFW_LOCK);
            ::ShowCursor(FALSE);
        }
        else
        {
            ::LockSetForegroundWindow(LSFW_UNLOCK);
            ::ShowCursor(TRUE);
        }
    }
#else
#pragma message ( "Warning: Missing LockSetForegroundWindow()" )
#endif

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void Player::Render()
{
   U64 timeforframe = usec();

   m_pininput.ProcessKeys(/*sim_msec,*/ -(int)(timeforframe / 1000)); // trigger key events mainly for VPM<->VP rountrip

   if (m_overall_frames < 10)
   {
      const HWND hVPMWnd = FindWindow("MAME", NULL);
      if (hVPMWnd != NULL)
      {
         if (::IsWindowVisible(hVPMWnd))
            ::SetWindowPos(hVPMWnd, HWND_TOPMOST, 0, 0, 0, 0, (SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOACTIVATE)); // in some strange cases the vpinmame window is not on top, so enforce it
      }
   }

   if (m_sleeptime > 0)
      Sleep(m_sleeptime - 1);

   m_pininput.ProcessKeys(/*sim_msec,*/ -(int)(timeforframe / 1000)); // trigger key events mainly for VPM<->VP rountrip

#ifdef DEBUGPHYSICS
   c_hitcnts = 0;
   c_collisioncnt = 0;
   c_contactcnt = 0;
#ifdef C_DYNAMIC
   c_staticcnt = 0;
#endif
   c_embedcnts = 0;
   c_timesearch = 0;

   c_kDNextlevels = 0;
   //c_quadNextlevels = 0; // not updated per frame so keep!

   c_traversed = 0;
   c_tested = 0;
   c_deepTested = 0;
#endif

   m_LastKnownGoodCounter++;

   m_pin3d.m_pd3dPrimaryDevice->m_stats_drawn_triangles = 0;

   // copy static buffers to back buffer and z buffer
   m_pin3d.m_pd3dPrimaryDevice->CopySurface(m_pin3d.m_pddsBackBuffer, m_pin3d.m_pddsStatic);
   m_pin3d.m_pd3dPrimaryDevice->CopySurface(m_pin3d.m_pddsZBuffer, m_pin3d.m_pddsStaticZ); // cannot be called inside BeginScene -> EndScene cycle

   // Physics/Timer updates, done at the last moment, especially to handle key input (VP<->VPM rountrip) and animation triggers
   //if ( !cameraMode )
   if (m_minphyslooptime == 0) // (vsync) latency reduction code not active? -> Do Physics Updates here
      UpdatePhysics();

   m_overall_frames++;

   // Process all AnimObjects (currently only DispReel, LightSeq and Slingshot)
   for (size_t l = 0; l < m_vanimate.size(); ++l)
      m_vanimate[l]->Animate();

   if (ProfilingMode() == 1)
      m_pin3d.m_gpu_profiler.BeginFrame(m_pin3d.m_pd3dPrimaryDevice->GetCoreDevice());

   if (!RenderStaticOnly())
   {
      m_pin3d.m_pd3dPrimaryDevice->BeginScene();
      RenderDynamics();
      m_pin3d.m_pd3dPrimaryDevice->EndScene();
   }

   m_pininput.ProcessKeys(/*sim_msec,*/ -(int)(timeforframe / 1000)); // trigger key events mainly for VPM<->VP rountrip

   // Check if we should turn animate the plunger light.
   hid_set_output(HID_OUTPUT_PLUNGER, ((m_time_msec - m_LastPlungerHit) < 512) && ((m_time_msec & 512) > 0));

   int localvsync = (m_ptable->m_TableAdaptiveVSync == -1) ? m_VSync : m_ptable->m_TableAdaptiveVSync;
   if (localvsync > m_refreshrate) // cannot sync, just limit to selected framerate
      localvsync = 0;
   else if (localvsync > 1) // adaptive sync to refresh rate
      localvsync = m_refreshrate;

   bool vsync = false;
   if (localvsync > 0)
      if (localvsync != 1) // do nothing for 1, as already enforced during device set
         if (m_fps > localvsync*ADAPT_VSYNC_FACTOR)
            vsync = true;

   const bool useAO = ((m_dynamicAO && (m_ptable->m_useAO == -1)) || (m_ptable->m_useAO == 1)) && m_pin3d.m_pd3dPrimaryDevice->DepthBufferReadBackAvailable() && (m_ptable->m_AOScale > 0.f);

#ifdef USE_IMGUI
   UpdateHUD_IMGUI();
#endif

   if (useAO && !m_disableAO)
      PrepareVideoBuffersAO();
   else
      PrepareVideoBuffersNormal();

   // DJRobX's crazy latency-reduction code active? Insert some Physics updates before vsync'ing
   if (m_minphyslooptime > 0)
   {
      UpdatePhysics();
      m_pininput.ProcessKeys(/*sim_msec,*/ -(int)(timeforframe / 1000)); // trigger key events mainly for VPM<->VP rountrip
   }
   FlipVideoBuffers(vsync);

   if (ProfilingMode() != 0)
      m_pin3d.m_gpu_profiler.EndFrame();

#ifndef ACCURATETIMERS
   // do the en/disable changes for the timers that piled up
   for (size_t i = 0; i < m_changed_vht.size(); ++i)
       if (m_changed_vht[i].enabled) // add the timer?
       {
           if (FindIndexOf(m_vht, m_changed_vht[i].m_timer) < 0)
               m_vht.push_back(m_changed_vht[i].m_timer);
       }
       else // delete the timer?
       {
           const int idx = FindIndexOf(m_vht, m_changed_vht[i].m_timer);
           if (idx >= 0)
               m_vht.erase(m_vht.begin() + idx);
       }
   m_changed_vht.clear();

   Ball * const old_pactiveball = m_pactiveball;
   m_pactiveball = NULL;  // No ball is the active ball for timers/key events

   for (size_t i=0;i<m_vht.size();i++)
   {
      HitTimer * const pht = m_vht[i];
      if ((pht->m_interval >= 0 && pht->m_nextfire <= m_time_msec) || pht->m_interval < 0) 
      {
         const unsigned int curnextfire = pht->m_nextfire;
         pht->m_pfe->FireGroupEvent(DISPID_TimerEvents_Timer);
         // Only add interval if the next fire time hasn't changed since the event was run. 
         // Handles corner case:
         //Timer1.Enabled = False
         //Timer1.Interval = 1000
         //Timer1.Enabled = True
         if (curnextfire == pht->m_nextfire)
            pht->m_nextfire += pht->m_interval;
      }
   }

   m_pactiveball = old_pactiveball;
#else
   m_pininput.ProcessKeys(/*sim_msec,*/ -(int)(timeforframe / 1000)); // trigger key events mainly for VPM<->VP rountrip
#endif

   // Update music stream
   if (m_audio)
   {
      if (!m_audio->MusicActive())
      {
         delete m_audio;
         m_audio = NULL;
         m_ptable->FireVoidEvent(DISPID_GameEvents_MusicDone);
      }
   }

   for (size_t i = 0; i < m_vballDelete.size(); i++)
   {
      Ball * const pball = m_vballDelete[i];
      delete pball->m_d.m_vpVolObjs;
      delete pball;
   }

   m_vballDelete.clear();

#ifdef STEPPING
   if ((m_pauseTimeTarget > 0) && (m_pauseTimeTarget <= m_time_msec))
   {
      m_pauseTimeTarget = 0;
      m_userDebugPaused = true;
      RecomputePseudoPauseState();
      if(m_debuggerDialog.IsWindow())
        m_debuggerDialog.SendMessage(RECOMPUTEBUTTONCHECK, 0, 0);
   }
#endif

   // limit framerate if requested by user (vsync Hz higher than refreshrate of gfxcard/monitor)
   localvsync = (m_ptable->m_TableAdaptiveVSync == -1) ? m_VSync : m_ptable->m_TableAdaptiveVSync;
   if (localvsync > m_refreshrate)
   {
      timeforframe = usec() - timeforframe;
      if (timeforframe < 1000000ull / localvsync)
         uSleep(1000000ull / localvsync - timeforframe);
   }

   if (m_ptable->m_pcv->m_scriptError)
   {
      // Crash back to the editor
      //SendMessage(WM_CLOSE, 0, 0);
      m_ptable->SendMessage(WM_COMMAND, ID_TABLE_STOP_PLAY, 0);
   }
   else
   {
      if (m_closeDown && m_closeDownDelay) // wait for one frame to stop game, to be able to display the additional text (table info, etc)
      {
		   m_closeDownDelay = false;

		   // add or remove caption, border and buttons (only if in windowed mode)?
		   const int captionheight = GetSystemMetrics(SM_CYCAPTION);
		   if (!m_fullScreen && (m_showWindowedCaption || (!m_showWindowedCaption && ((m_screenheight - m_height) >= (captionheight * 2))))) // We have enough room for a frame? //!! *2 ??
		   {
			   RECT rect;
			   ::GetWindowRect(GetHwnd(), &rect);
			   const int x = rect.left;
			   const int y = rect.top;

			   // Add/Remove a pretty window border and standard control boxes.
			   const int windowflags = m_showWindowedCaption ? WS_POPUP : (WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN);
			   const int windowflagsex = m_showWindowedCaption ? 0 : WS_EX_OVERLAPPEDWINDOW;

			   //!! does not respect borders so far!!! -> remove them or change width/height accordingly ?? otherwise ignore as eventually it will be restored anyway??
			   //!! like this the render window is scaled and thus implicitly blurred though!
			   SetWindowLongPtr(GWL_STYLE, windowflags);
			   SetWindowLongPtr(GWL_EXSTYLE, windowflagsex);
			   SetWindowPos(NULL, x, m_showWindowedCaption ? (y + captionheight) : (y - captionheight), m_width, m_height + (m_showWindowedCaption ? 0 : captionheight), SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
			   ShowWindow(SW_SHOW);

			   // Save position of non-fullscreen player window to registry, and only if it was potentially moved around (i.e. when caption was already visible)
			   if (m_showWindowedCaption)
			   {
				   HRESULT hr = SaveValueInt("Player", "WindowPosX", x);
				   hr = SaveValueInt("Player", "WindowPosY", y + captionheight);
			   }

			   m_showWindowedCaption = !m_showWindowedCaption;
		   }
      }
      else if (m_closeDown)
      {
         PauseMusic();

         size_t option;

         if (m_closeType == 2)
         {
            exit(-9999); // blast into space
         }
         else if (!VPinball::m_open_minimized && m_closeType == 0)
         {
             ShowCursor(TRUE);
             option = DialogBox(g_pvp->theInstance, MAKEINTRESOURCE(IDD_GAMEPAUSE), GetHwnd(), PauseProc);
             if(option!=ID_DEBUGWINDOW)
                ShowCursor(FALSE);
         }
         else //m_closeType == all others
         {
            option = ID_QUIT;
            SendMessage(g_pvp->GetHwnd(), WM_COMMAND, ID_FILE_EXIT, NULL);
         }

         m_closeDown = false;
         m_closeDownDelay = true;
         m_noTimeCorrect = true; // Skip the time we were in the dialog
         UnpauseMusic();

         if (option == ID_QUIT)
             m_ptable->SendMessage(WM_COMMAND, ID_TABLE_STOP_PLAY, 0);
      }
      else if(m_showDebugger && !VPinball::m_open_minimized)
      {
          m_debugMode = true;
          m_showDebugger = false;
          if (!m_debuggerDialog.IsWindow())
          {
              m_debuggerDialog.Create(GetHwnd());
              m_debuggerDialog.ShowWindow();
          }
          else
              m_debuggerDialog.SetForegroundWindow();

          EndDialog( g_pvp->GetHwnd(), ID_DEBUGWINDOW );
      }
   }
   ///// Don't put anything here - the ID_QUIT check must be the last thing done
   ///// in this function
}

void Player::PauseMusic()
{
   if (m_pauseRefCount == 0)
   {
      if (m_audio)
         m_audio->MusicPause();

      // signal the script that the game is now paused
      m_ptable->FireVoidEvent(DISPID_GameEvents_Paused);
   }

   m_pauseRefCount++;
}

void Player::UnpauseMusic()
{
   m_pauseRefCount--;
   if (m_pauseRefCount == 0)
   {
      if (m_audio)
         m_audio->MusicUnpause();

      // signal the script that the game is now running again
      m_ptable->FireVoidEvent(DISPID_GameEvents_UnPaused);
   }
   else if (m_pauseRefCount < 0)
      m_pauseRefCount = 0;
}

inline float map_bulblight_to_emission(const Light* const l) // magic mapping of bulblight parameters to "real" lightsource emission
{
   return l->m_d.m_currentIntensity * clamp(powf(l->m_d.m_falloff*0.6f, l->m_d.m_falloff_power*0.6f), 0.f, 23000.f); //!! 0.6f,0.6f = magic, also clamp 23000
}

void search_for_nearest(const Ball * const pball, const std::vector<Light*> &lights, Light* light_nearest[MAX_BALL_LIGHT_SOURCES])
{
   for (unsigned int l = 0; l < MAX_BALL_LIGHT_SOURCES; ++l)
   {
      float min_dist = FLT_MAX;
      light_nearest[l] = NULL;
      for (size_t i = 0; i < lights.size(); ++i)
      {
         bool already_processed = false;
         for (unsigned int i2 = 0; i2 < MAX_BALL_LIGHT_SOURCES - 1; ++i2)
            if (l > i2 && light_nearest[i2] == lights[i]) {
               already_processed = true;
               break;
            }
         if (already_processed)
            continue;

         const float dist = Vertex3Ds(lights[i]->m_d.m_vCenter.x - pball->m_d.m_pos.x, lights[i]->m_d.m_vCenter.y - pball->m_d.m_pos.y, lights[i]->m_d.m_meshRadius + lights[i]->m_surfaceHeight - pball->m_d.m_pos.z).LengthSquared(); //!! z pos
         //const float contribution = map_bulblight_to_emission(lights[i]) / dist; // could also weight in light color if necessary //!! JF didn't like that, seems like only distance is a measure better suited for the human eye
         if (dist < min_dist)
         {
            min_dist = dist;
            light_nearest[l] = lights[i];
         }
      }
   }
}

void Player::GetBallAspectRatio(const Ball * const pball, float &stretchX, float &stretchY, const float zHeight)
{
   // always use lowest detail level for fastest update
   Vertex3Ds rgvIn[(basicBallLoNumVertices+1) / 2];
   Vertex2D rgvOut[(basicBallLoNumVertices+1) / 2];

   //     rgvIn[0].x = pball->m_pos.x;                    rgvIn[0].y = pball->m_pos.y+pball->m_radius;    rgvIn[0].z = zHeight;
   //     rgvIn[1].x = pball->m_pos.x + pball->m_radius;  rgvIn[1].y = pball->m_pos.y;                    rgvIn[1].z = zHeight;
   //     rgvIn[2].x = pball->m_pos.x;                    rgvIn[2].y = pball->m_pos.y - pball->m_radius;  rgvIn[2].z = zHeight;
   //     rgvIn[3].x = pball->m_pos.x - pball->m_radius;  rgvIn[3].y = pball->m_pos.y;                    rgvIn[3].z = zHeight;
   //     rgvIn[4].x = pball->m_pos.x;                    rgvIn[4].y = pball->m_pos.y;                    rgvIn[4].z = zHeight + pball->m_radius;
   //     rgvIn[5].x = pball->m_pos.x;                    rgvIn[5].y = pball->m_pos.y;                    rgvIn[5].z = zHeight - pball->m_radius;
   
   for (unsigned int i = 0, t = 0; i < basicBallLoNumVertices; i += 2, t++)
   {
      rgvIn[t].x = basicBallLo[i].x*pball->m_d.m_radius + pball->m_d.m_pos.x;
      rgvIn[t].y = basicBallLo[i].y*pball->m_d.m_radius + pball->m_d.m_pos.y;
      rgvIn[t].z = basicBallLo[i].z*pball->m_d.m_radius + zHeight;
   }
   
   m_pin3d.m_proj.TransformVertices(rgvIn, NULL, basicBallLoNumVertices / 2, rgvOut);
   
   float maxX = -FLT_MAX;
   float minX = FLT_MAX;
   float maxY = -FLT_MAX;
   float minY = FLT_MAX;
   for (unsigned int i = 0; i < basicBallLoNumVertices / 2; i++)
   {
      if (maxX < rgvOut[i].x) maxX = rgvOut[i].x;
      if (minX > rgvOut[i].x) minX = rgvOut[i].x;
      if (maxY < rgvOut[i].y) maxY = rgvOut[i].y;
      if (minY > rgvOut[i].y) minY = rgvOut[i].y;
   }

   const float midX = maxX - minX;
   const float midY = maxY - minY;
   stretchY = midY/midX;
   //stretchX = midX/midY;
   stretchX = 1.0f;
}

// not used anymore. Reflection of the ball is done in RenderDynamicMirror()!
/*void Player::DrawBallReflection(Ball *pball, const float zheight, const bool lowDetailBall)
{
   // this is the old ball reflection hack and can be removed if the new reflection works!
   const vec4 pos_radRef(pball->m_pos.x, pball->m_pos.y, zheight + m_ptable->m_tableheight, pball->m_radius);
   m_ballShader->SetVector("position_radius", &pos_radRef);
   const vec4 refl((float)m_ptable->m_ballReflectionStrength * (float)(1.0 / 255.0), m_ptable->m_playfieldReflectionStrength, 0.f, 0.f);
   m_ballShader->SetVector("reflection_ball_playfield", &refl);
   m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, RenderDevice::RS_FALSE);
   m_pin3d.EnableAlphaBlend(false, false);
   m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::DESTBLEND, RenderDevice::DST_ALPHA);
   m_ballShader->SetTechnique("RenderBallReflection");

   m_ballShader->Begin(0);
   m_pin3d.m_pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, ballVertexBuffer, 0, lowDetailBall ? basicBallLoNumVertices : basicBallMidNumVertices, ballIndexBuffer, 0, lowDetailBall ? basicBallLoNumFaces : basicBallMidNumFaces);
   m_ballShader->End();

   m_pin3d.DisableAlphaBlend();
}*/

void Player::DrawBalls()
{
   m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::DEPTHBIAS, 0);
   m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::BLENDOP, RenderDevice::BLENDOP_ADD);
   m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_CCW);

   if (m_toggleDebugBalls && m_debugBalls)
      // Set the render state to something that will always display.
      m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::ZENABLE, RenderDevice::RS_FALSE);

   // collect all lights that can reflect on balls (currently only bulbs and if flag set to do so)
   std::vector<Light*> lights;
   for (size_t i = 0; i < m_ptable->m_vedit.size(); i++)
   {
      IEditable * const item = m_ptable->m_vedit[i];
      if (item && item->GetItemType() == eItemLight && ((Light *)item)->m_d.m_BulbLight && ((Light *)item)->m_d.m_showReflectionOnBall)
         lights.push_back((Light *)item);
   }

   bool drawReflection = ((m_reflectionForBalls && (m_ptable->m_useReflectionForBalls == -1)) || (m_ptable->m_useReflectionForBalls == 1));
   const bool orgDrawReflection = drawReflection;
   //     if (reflectionOnly && !drawReflection)
   //        return;

   //m_pin3d.m_pd3dDevice->SetTextureAddressMode(0, RenderDevice::TEX_CLAMP);
   //m_pin3d.m_pd3dDevice->SetTextureFilter(0, TEXTURE_MODE_TRILINEAR);

   const Material * const playfield_mat = m_ptable->GetMaterial(m_ptable->m_szPlayfieldMaterial);
   const vec4 playfield_cBaseF = convertColor(playfield_mat->m_cBase);
   const float playfield_avg_diffuse = playfield_cBaseF.x*0.176204f + playfield_cBaseF.y*0.812985f + playfield_cBaseF.z*0.0108109f;

   for (size_t i = 0; i < m_vball.size(); i++)
   {
      Ball * const pball = m_vball[i];

      if(!pball->m_visible)
          continue;

      if (orgDrawReflection && !pball->m_reflectionEnabled)
         drawReflection = false;
      if (orgDrawReflection && pball->m_reflectionEnabled)
         drawReflection = true;

      // calculate/adapt height of ball
      float zheight = (!pball->m_d.m_frozen) ? pball->m_d.m_pos.z : (pball->m_d.m_pos.z - pball->m_d.m_radius);

      if (m_ptable->m_reflectionEnabled)
         zheight -= m_ptable->m_tableheight*2.0f;

      const float maxz = (pball->m_d.m_radius + m_ptable->m_tableheight) + 3.0f;
      const float minz = (pball->m_d.m_radius + m_ptable->m_tableheight) - 0.1f;
      if ((m_reflectionForBalls && pball->m_reflectionEnabled && !pball->m_forceReflection && (m_ptable->m_useReflectionForBalls == -1)) || (m_ptable->m_useReflectionForBalls == 1 && !pball->m_forceReflection))
         // don't draw reflection if the ball is not on the playfield (e.g. on a ramp/kicker)
         drawReflection = !((zheight > maxz) || pball->m_d.m_frozen || (pball->m_d.m_pos.z < minz));

      if (!drawReflection && m_ptable->m_reflectionEnabled)
         continue;

      const float inv_tablewidth = 1.0f / (m_ptable->m_right - m_ptable->m_left);
      const float inv_tableheight = 1.0f / (m_ptable->m_bottom - m_ptable->m_top);
      //const float inclination = ANGTORAD(m_ptable->m_inclination);
      const vec4 phr(inv_tablewidth, inv_tableheight, m_ptable->m_tableheight,
					 m_ptable->m_ballPlayfieldReflectionStrength*pball->m_playfieldReflectionStrength
					 *playfield_avg_diffuse //!! hack: multiply average diffuse from playfield onto strength, as only diffuse lighting is used for reflection
					 *0.5f                  //!! additional magic correction factor due to everything being wrong in the earlier reflection/lighting implementation
					 );
      m_ballShader->SetVector("invTableRes__playfield_height_reflection", &phr);

      if ((zheight > maxz) || (pball->m_d.m_pos.z < minz))
      {
         // scaling the ball height by the z scale value results in a flying ball over the playfield/ramp
         // by reducing it with 0.96f (a factor found by trial'n error) the ball is on the ramp again
         if (m_ptable->m_BG_scalez[m_ptable->m_BG_current_set] != 1.0f)
            zheight *= (m_ptable->m_BG_scalez[m_ptable->m_BG_current_set] * 0.96f);
      }

      // collect the x nearest lights that can reflect on balls
      Light* light_nearest[MAX_BALL_LIGHT_SOURCES];
      search_for_nearest(pball, lights, light_nearest);

	  struct CLight
	  {
		  float vPos[3];
		  float vEmission[3];
	  };
	  CLight l[MAX_LIGHT_SOURCES + MAX_BALL_LIGHT_SOURCES];

	  vec4 emission = convertColor(m_ptable->m_Light[0].emission);
	  emission.x *= m_ptable->m_lightEmissionScale*m_globalEmissionScale;
	  emission.y *= m_ptable->m_lightEmissionScale*m_globalEmissionScale;
	  emission.z *= m_ptable->m_lightEmissionScale*m_globalEmissionScale;

	  for (unsigned int i2 = 0; i2 < MAX_LIGHT_SOURCES; ++i2)
	  {
		  memcpy(&l[i2].vPos, &m_ptable->m_Light[i2].pos, sizeof(float) * 3);
		  memcpy(&l[i2].vEmission, &emission, sizeof(float) * 3);
	  }

	  for (unsigned int light_i = 0; light_i < MAX_BALL_LIGHT_SOURCES; ++light_i)
		  if (light_nearest[light_i] != NULL)
		  {
			  l[light_i + MAX_LIGHT_SOURCES].vPos[0] = light_nearest[light_i]->m_d.m_vCenter.x;
			  l[light_i + MAX_LIGHT_SOURCES].vPos[1] = light_nearest[light_i]->m_d.m_vCenter.y;
			  l[light_i + MAX_LIGHT_SOURCES].vPos[2] = light_nearest[light_i]->m_d.m_meshRadius + light_nearest[light_i]->m_surfaceHeight; //!! z pos
			  const float c = map_bulblight_to_emission(light_nearest[light_i]) * pball->m_bulb_intensity_scale;
			  const vec4 color = convertColor(light_nearest[light_i]->m_d.m_color);
			  l[light_i + MAX_LIGHT_SOURCES].vEmission[0] = color.x*c;
			  l[light_i + MAX_LIGHT_SOURCES].vEmission[1] = color.y*c;
			  l[light_i + MAX_LIGHT_SOURCES].vEmission[2] = color.z*c;
		  }
		  else //!! rather just set the max number of ball lights!?
		  {
			  l[light_i + MAX_LIGHT_SOURCES].vPos[0] = -100000.0f;
			  l[light_i + MAX_LIGHT_SOURCES].vPos[1] = -100000.0f;
			  l[light_i + MAX_LIGHT_SOURCES].vPos[2] = -100000.0f;
			  l[light_i + MAX_LIGHT_SOURCES].vEmission[0] = 0.0f;
			  l[light_i + MAX_LIGHT_SOURCES].vEmission[1] = 0.0f;
			  l[light_i + MAX_LIGHT_SOURCES].vEmission[2] = 0.0f;
		  }

      m_ballShader->SetValue("packedLights", l, sizeof(CLight)*(MAX_LIGHT_SOURCES + MAX_BALL_LIGHT_SOURCES));

      // now for a weird hack: make material more rough, depending on how near the nearest lightsource is, to 'emulate' the area of the bulbs (as VP only features point lights so far)
      float Roughness = 0.8f;
      if (light_nearest[0] != NULL)
      {
          const float dist = Vertex3Ds(light_nearest[0]->m_d.m_vCenter.x - pball->m_d.m_pos.x, light_nearest[0]->m_d.m_vCenter.y - pball->m_d.m_pos.y, light_nearest[0]->m_d.m_meshRadius + light_nearest[0]->m_surfaceHeight - pball->m_d.m_pos.z).Length(); //!! z pos
          Roughness = min(max(dist*0.006f, 0.4f), Roughness);
      }
      const vec4 rwem(exp2f(10.0f * Roughness + 1.0f), 0.f, 1.f, 0.05f);
      m_ballShader->SetVector("Roughness_WrapL_Edge_Thickness", &rwem);

      // ************************* draw the ball itself ****************************
      float sx, sy;
      if (m_antiStretchBall && m_ptable->m_BG_rotation[m_ptable->m_BG_current_set] != 0.0f)
         //const vec4 bs(m_BallStretchX/* +sx*/, m_BallStretchY - sy, inv_tablewidth, inv_tableheight);
         GetBallAspectRatio(pball, sx, sy, zheight);
      else
      {
         sx = m_BallStretchX;
         sy = m_BallStretchY;
      }

      const vec4 diffuse = convertColor(pball->m_color, 1.0f);
      m_ballShader->SetVector("cBase_Alpha", &diffuse);

      D3DXMATRIX m(pball->m_orientation.m_d[0][0], pball->m_orientation.m_d[1][0], pball->m_orientation.m_d[2][0], 0.0f,
         pball->m_orientation.m_d[0][1], pball->m_orientation.m_d[1][1], pball->m_orientation.m_d[2][1], 0.0f,
         pball->m_orientation.m_d[0][2], pball->m_orientation.m_d[1][2], pball->m_orientation.m_d[2][2], 0.0f,
         0.f, 0.f, 0.f, 1.f);
      Matrix3D temp;
      memcpy(temp.m, m.m, 4 * 4 * sizeof(float));
      Matrix3D m3D_full;
      m3D_full.SetScaling(pball->m_d.m_radius*sx, pball->m_d.m_radius*sy, pball->m_d.m_radius);
      m3D_full.Multiply(temp, m3D_full);
      temp.SetTranslation(pball->m_d.m_pos.x, pball->m_d.m_pos.y, zheight);
      temp.Multiply(m3D_full, m3D_full);
      memcpy(m.m, m3D_full.m, 4 * 4 * sizeof(float));
      m_ballShader->SetMatrix("orientation", &m);

      if (!pball->m_pinballEnv)
      {
         m_ballShader->SetBool("hdrTexture0", m_pin3d.m_pinballEnvTexture.IsHDR()); // should always be false, as read from (LDR-Bitmap-)Resources
         m_ballShader->SetTexture("Texture0", &m_pin3d.m_pinballEnvTexture, false);
      }
      else
      {
         m_ballShader->SetBool("hdrTexture0", pball->m_pinballEnv->IsHDR());
         m_ballShader->SetTexture("Texture0", pball->m_pinballEnv, false);
      }

      if (pball->m_pinballDecal)
         m_ballShader->SetTexture("Texture3", pball->m_pinballDecal, false);

      const bool lowDetailBall = m_ptable->GetDetailLevel() < 10;

      // old ball reflection code
      //if (drawReflection)
      //   DrawBallReflection(pball, zheight, lowDetailBall);

      //m_ballShader->SetFloat("reflection_ball_playfield", m_ptable->m_playfieldReflectionStrength);
      m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::ZWRITEENABLE, RenderDevice::RS_TRUE);

      if (m_cabinetMode && !pball->m_decalMode)
          strncpy_s(m_ballShaderTechnique, "RenderBall_CabMode", sizeof(m_ballShaderTechnique)-1);
      else if (m_cabinetMode && pball->m_decalMode)
          strncpy_s(m_ballShaderTechnique, "RenderBall_CabMode_DecalMode", sizeof(m_ballShaderTechnique)-1);
      else if (!m_cabinetMode && pball->m_decalMode)
          strncpy_s(m_ballShaderTechnique, "RenderBall_DecalMode", sizeof(m_ballShaderTechnique)-1);
      else //if (!m_fCabinetMode && !pball->m_decalMode)
          strncpy_s(m_ballShaderTechnique, "RenderBall", sizeof(m_ballShaderTechnique)-1);

      m_ballShader->SetTechnique(m_ballShaderTechnique);

      m_ballShader->Begin(0);
      m_pin3d.m_pd3dPrimaryDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, m_ballVertexBuffer, 0, lowDetailBall ? basicBallLoNumVertices : basicBallMidNumVertices, m_ballIndexBuffer, 0, lowDetailBall ? basicBallLoNumFaces : basicBallMidNumFaces);
      m_ballShader->End();

      // ball trails
      if((!m_ptable->m_reflectionEnabled) && // do not render trails in reflection pass
         ((m_trailForBalls && (m_ptable->m_useTrailForBalls == -1)) || (m_ptable->m_useTrailForBalls == 1)))
      {
         Vertex3D_NoTex2 rgv3D_all[MAX_BALL_TRAIL_POS * 2];
         unsigned int num_rgv3D = 0;

         for (int i2 = 0; i2 < MAX_BALL_TRAIL_POS - 1; ++i2)
         {
            int i3 = pball->m_ringcounter_oldpos / (10000 / PHYSICS_STEPTIME) - 1 - i2;
            if (i3 < 0)
               i3 += MAX_BALL_TRAIL_POS;
            int io = i3 - 1;
            if (io < 0)
               io += MAX_BALL_TRAIL_POS;

            if ((pball->m_oldpos[i3].x != FLT_MAX) && (pball->m_oldpos[io].x != FLT_MAX)) // only if already initialized
            {
               Vertex3Ds vec;
               vec.x = pball->m_oldpos[io].x - pball->m_oldpos[i3].x;
               vec.y = pball->m_oldpos[io].y - pball->m_oldpos[i3].y;
               vec.z = pball->m_oldpos[io].z - pball->m_oldpos[i3].z;
               const float bc = m_ptable->m_ballTrailStrength * powf(1.f - 1.f / max(vec.Length(), 1.0f), 64.0f); //!! 64=magic alpha falloff
               const float r = min(pball->m_d.m_radius*0.9f, 2.0f*pball->m_d.m_radius / powf((float)(i2 + 2), 0.6f)); //!! consts are for magic radius falloff

               if (bc > 0.f && r > FLT_MIN)
               {
                  Vertex3Ds v = vec;
                  v.Normalize();
                  const Vertex3Ds up(0.f, 0.f, 1.f);
                  Vertex3Ds n = CrossProduct(v, up);
                  n.x *= r;
                  n.y *= r;
                  n.z *= r;

                  Vertex3D_NoTex2 rgv3D[4];
                  rgv3D[0].x = pball->m_oldpos[i3].x - n.x;
                  rgv3D[0].y = pball->m_oldpos[i3].y - n.y;
                  rgv3D[0].z = pball->m_oldpos[i3].z - n.z;
                  rgv3D[1].x = pball->m_oldpos[i3].x + n.x;
                  rgv3D[1].y = pball->m_oldpos[i3].y + n.y;
                  rgv3D[1].z = pball->m_oldpos[i3].z + n.z;
                  rgv3D[2].x = pball->m_oldpos[io].x + n.x;
                  rgv3D[2].y = pball->m_oldpos[io].y + n.y;
                  rgv3D[2].z = pball->m_oldpos[io].z + n.z;
                  rgv3D[3].x = pball->m_oldpos[io].x - n.x;
                  rgv3D[3].y = pball->m_oldpos[io].y - n.y;
                  rgv3D[3].z = pball->m_oldpos[io].z - n.z;

                  rgv3D[0].nx = rgv3D[1].nx = rgv3D[2].nx = rgv3D[3].nx = bc; //!! abuses normal for now for the color/alpha

                  rgv3D[0].tu = 0.5f + (float)(i2)*(float)(1.0 / (2.0*(MAX_BALL_TRAIL_POS - 1)));
                  rgv3D[0].tv = 0.f;
                  rgv3D[1].tu = rgv3D[0].tu;
                  rgv3D[1].tv = 1.f;
                  rgv3D[2].tu = 0.5f + (float)(i2 + 1)*(float)(1.0 / (2.0*(MAX_BALL_TRAIL_POS - 1)));
                  rgv3D[2].tv = 1.f;
                  rgv3D[3].tu = rgv3D[2].tu;
                  rgv3D[3].tv = 0.f;

                  if (num_rgv3D == 0)
                  {
                     rgv3D_all[0] = rgv3D[0];
                     rgv3D_all[1] = rgv3D[1];
                     rgv3D_all[2] = rgv3D[3];
                     rgv3D_all[3] = rgv3D[2];
                  }
                  else
                  {
                     rgv3D_all[num_rgv3D - 2].x = (rgv3D[0].x + rgv3D_all[num_rgv3D - 2].x)*0.5f;
                     rgv3D_all[num_rgv3D - 2].y = (rgv3D[0].y + rgv3D_all[num_rgv3D - 2].y)*0.5f;
                     rgv3D_all[num_rgv3D - 2].z = (rgv3D[0].z + rgv3D_all[num_rgv3D - 2].z)*0.5f;
                     rgv3D_all[num_rgv3D - 1].x = (rgv3D[1].x + rgv3D_all[num_rgv3D - 1].x)*0.5f;
                     rgv3D_all[num_rgv3D - 1].y = (rgv3D[1].y + rgv3D_all[num_rgv3D - 1].y)*0.5f;
                     rgv3D_all[num_rgv3D - 1].z = (rgv3D[1].z + rgv3D_all[num_rgv3D - 1].z)*0.5f;
                     rgv3D_all[num_rgv3D] = rgv3D[3];
                     rgv3D_all[num_rgv3D + 1] = rgv3D[2];
                  }

                  if (num_rgv3D == 0)
                     num_rgv3D += 4;
                  else
                     num_rgv3D += 2;
               }
            }
         }

         if (num_rgv3D > 0)
         {
            Vertex3D_NoTex2 *bufvb;
            m_ballTrailVertexBuffer->lock(0, 0, (void**)&bufvb, VertexBuffer::DISCARDCONTENTS);
            memcpy(bufvb,rgv3D_all,num_rgv3D*sizeof(Vertex3D_NoTex2));
            m_ballTrailVertexBuffer->unlock();

            m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::ZWRITEENABLE, RenderDevice::RS_FALSE);
            m_pin3d.EnableAlphaBlend(false);

            m_ballShader->SetTechnique("RenderBallTrail");
            m_ballShader->Begin(0);
            m_pin3d.m_pd3dPrimaryDevice->DrawPrimitiveVB(RenderDevice::TRIANGLESTRIP, MY_D3DFVF_NOTEX2_VERTEX, m_ballTrailVertexBuffer, 0, num_rgv3D);
            m_ballShader->End();
         }
      }

#ifdef DEBUG_BALL_SPIN        // draw debug points for visualizing ball rotation
      if (ShowFPS())
      {
         // set transform
         Matrix3D matOrig, matNew, matRot;
         matOrig = m_pin3d.GetWorldTransform();
         matNew.SetTranslation(pball->m_d.m_pos);
         matOrig.Multiply(matNew, matNew);
         matRot.SetIdentity();
         for (int j = 0; j < 3; ++j)
            for (int k = 0; k < 3; ++k)
               matRot.m[j][k] = pball->m_orientation.m_d[k][j];
         matNew.Multiply(matRot, matNew);
         m_pin3d.m_pd3dPrimaryDevice->SetTransform(TRANSFORMSTATE_WORLD, &matNew);
         m_pin3d.DisableAlphaBlend();

         // draw points
         const float ptsize = 5.0f;
         m_pin3d.m_pd3dPrimaryDevice->SetRenderState((RenderDevice::RenderStates)D3DRS_POINTSIZE, *((DWORD*)&ptsize));
         m_pin3d.m_pd3dPrimaryDevice->DrawPrimitiveVB(RenderDevice::POINTLIST, MY_D3DFVF_TEX, m_ballDebugPoints, 0, 12);

         // reset transform
         m_pin3d.m_pd3dPrimaryDevice->SetTransform(TRANSFORMSTATE_WORLD, &matOrig);
      }
#endif

   }   // end loop over all balls

   //m_pin3d.DisableAlphaBlend(); //!! not necessary anymore

   // Set the render state to something that will always display.
   if (m_toggleDebugBalls && m_debugBalls)
      m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::ZENABLE, RenderDevice::RS_TRUE);
   if (m_toggleDebugBalls)
      m_toggleDebugBalls = false;
}

struct DebugMenuItem
{
   int objectindex;
   std::vector<int> *pvdispid;
   HMENU hmenu;
};

void AddEventToDebugMenu(char *sz, int index, int dispid, LPARAM lparam)
{
   DebugMenuItem * const pdmi = (DebugMenuItem *)lparam;
   const HMENU hmenu = pdmi->hmenu;
   const int menuid = ((pdmi->objectindex + 1) << 16) | (int)pdmi->pvdispid->size();
   pdmi->pvdispid->push_back(dispid);
   AppendMenu(hmenu, MF_STRING, menuid, sz);
}

void Player::DoDebugObjectMenu(const int x, const int y)
{
   if (m_vdebugho.size() == 0)
   {
      // First time the debug hit-testing has been used
      InitDebugHitStructure();
   }

   Matrix3D mat3D = m_pin3d.m_proj.m_matrixTotal;
   mat3D.Invert();

   ViewPort vp;
   m_pin3d.m_pd3dPrimaryDevice->GetViewport(&vp);
   const float rClipWidth = (float)vp.Width*0.5f;
   const float rClipHeight = (float)vp.Height*0.5f;

   const float xcoord = ((float)x - rClipWidth) / rClipWidth;
   const float ycoord = (rClipHeight - (float)y) / rClipHeight;

   // Use the inverse of our 3D transform to determine where in 3D space the
   // screen pixel the user clicked on is at.  Get the point at the near
   // clipping plane (z=0) and the far clipping plane (z=1) to get the whole
   // range we need to hit test
   Vertex3Ds v3d, v3d2;
   mat3D.MultiplyVector(Vertex3Ds(xcoord, ycoord, 0.f), v3d);
   mat3D.MultiplyVector(Vertex3Ds(xcoord, ycoord, 1.f), v3d2);

   // Create a ray (ball) that travels in 3D space from the screen pixel at
   // the near clipping plane to the far clipping plane, and find what
   // it intersects with.
   Ball ballT;
   ballT.m_d.m_pos = v3d;
   ballT.m_d.m_vel = v3d2 - v3d;
   ballT.m_d.m_radius = 0.f;
   ballT.m_coll.m_hittime = 1.0f;

   ballT.CalcHitBBox(); // need to update here, as only done lazily

   //const float slope = (v3d2.y - v3d.y)/(v3d2.z - v3d.z);
   //const float yhit = v3d.y - (v3d.z*slope);

   //const float slopex = (v3d2.x - v3d.x)/(v3d2.z - v3d.z);
   //const float xhit = v3d.x - (v3d.z*slopex);

   vector<HitObject*> vhoHit;
   m_hitoctree_dynamic.HitTestXRay(&ballT, vhoHit, ballT.m_coll);
   m_hitoctree.HitTestXRay(&ballT, vhoHit, ballT.m_coll);
   m_debugoctree.HitTestXRay(&ballT, vhoHit, ballT.m_coll);

   if (vhoHit.size() == 0)
   {
      // Nothing was hit-tested
      return;
   }

   PauseMusic();

   const HMENU hmenu = CreatePopupMenu();

   vector<IFireEvents*> vpfe;
   std::vector<HMENU> vsubmenu;
   std::vector< std::vector<int>* > vvdispid;
   for (size_t i = 0; i < vhoHit.size(); i++)
   {
      HitObject * const pho = vhoHit[i];
      // Make sure we don't do the same object twice through 2 different Hitobjs.
      if (pho->m_pfedebug && (FindIndexOf(vpfe, pho->m_pfedebug) == -1))
      {
         vpfe.push_back(pho->m_pfedebug);
         CComVariant var;
         DISPPARAMS dispparams = {
            NULL,
            NULL,
            0,
            0
         };
         const HRESULT hr = pho->m_pfedebug->GetDispatch()->Invoke(
            0x80010000, IID_NULL,
            LOCALE_USER_DEFAULT,
            DISPATCH_PROPERTYGET,
            &dispparams, &var, NULL, NULL);

         const HMENU submenu = CreatePopupMenu();
         vsubmenu.push_back(submenu);
         if (hr == S_OK)
         {
            WCHAR *wzT;
            wzT = V_BSTR(&var);
            AppendMenuW(hmenu, MF_STRING | MF_POPUP, (UINT_PTR)submenu, wzT);

            std::vector<int> *pvdispid = new std::vector<int>();
            vvdispid.push_back(pvdispid);

            DebugMenuItem dmi;
            dmi.objectindex = (int)i;
            dmi.pvdispid = pvdispid;
            dmi.hmenu = submenu;
            EnumEventsFromDispatch(pho->m_pfedebug->GetDispatch(), AddEventToDebugMenu, (LPARAM)&dmi);
         }

         IDebugCommands * const pdc = pho->m_pfedebug->GetDebugCommands();
         if (pdc)
         {
            std::vector<int> vids;
            std::vector<int> vcommandid;

            pdc->GetDebugCommands(vids, vcommandid);
            for (size_t l = 0; l < vids.size(); l++)
            {
               const LocalString ls(vids[l]);
               AppendMenu(submenu, MF_STRING, ((i + 1) << 16) | vcommandid[l] | 0x8000, ls.m_szbuffer);
            }
         }
      }
      else
      {
         vvdispid.push_back(NULL); // Put a spacer in so we can keep track of indexes
      }
   }

   POINT pt;
   pt.x = x;
   pt.y = y;
   ClientToScreen(pt);

   const int icmd = TrackPopupMenuEx(hmenu, TPM_RETURNCMD | TPM_RIGHTBUTTON, pt.x, pt.y, GetHwnd(), NULL);

   if (icmd != 0 && vsubmenu.size() > 0)
   {
      const int highword = HIWORD(icmd) - 1;
      const int lowword = icmd & 0xffff;
      IFireEvents * const pfe = vhoHit[highword]->m_pfedebug;
      if (lowword & 0x8000) // custom debug command
      {
         pfe->GetDebugCommands()->RunDebugCommand(lowword & 0x7fff);
      }
      else
      {
         const int dispid = (*vvdispid[highword])[lowword];
         m_pactiveball = m_pactiveballDebug;
         pfe->FireGroupEvent(dispid);
         m_pactiveball = NULL;
      }
   }

   DestroyMenu(hmenu);
   for (size_t i = 0; i < vsubmenu.size(); i++)
      DestroyMenu(vsubmenu[i]);

   for (size_t i = 0; i < vvdispid.size(); i++)
      delete vvdispid[i];

   UnpauseMusic();
}

LRESULT Player::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#ifdef USE_IMGUI
    if (ImGui_ImplWin32_WndProcHandler(GetHwnd(), uMsg, wParam, lParam))
      return true;
#endif

    switch (uMsg)
    {
    case MM_MIXM_CONTROL_CHANGE:
        mixer_get_volume();
        break;

    case WM_CLOSE:
    {
        // In Windows 10 1803, there may be a significant lag waiting for WM_DESTROY if script is not closed first.   
        // Shut down script first if in exclusive mode.  
        if (m_fullScreen)
            StopPlayer();

        break;
    }
    case WM_DESTROY:
    {
        if (!m_fullScreen)
            StopPlayer();

        Shutdown();
        m_ptable->SendMessage(WM_COMMAND, ID_TABLE_PLAYER_STOPPED, 0);
        return 0;
    }
    case WM_KEYDOWN:
        m_drawCursor = false;
        SetCursor(NULL);
        break;

    case WM_MOUSEMOVE:
        if (m_lastcursorx != LOWORD(lParam) || m_lastcursory != HIWORD(lParam))
        {
            m_drawCursor = true;
            m_lastcursorx = LOWORD(lParam);
            m_lastcursory = HIWORD(lParam);
        }
        break;

#ifdef STEPPING
#ifdef MOUSEPAUSE
    case WM_LBUTTONDOWN:
        if (m_pause)
        {
            m_step = true;
        }
        break;

    case WM_RBUTTONDOWN:
        if (!m_pause)
        {
            m_pause = true;

            m_gameWindowActive = false;
            RecomputePauseState();
            RecomputePseudoPauseState();
        }
        else
        {
            m_pause = false;

            m_gameWindowActive = true;
            SetCursor(NULL);
            m_noTimeCorrect = true;
        }
        break;
#endif
#endif
    case WM_RBUTTONUP:
    {
        if (m_debugMode)
        {
            const int x = lParam & 0xffff;
            const int y = (lParam >> 16) & 0xffff;
            DoDebugObjectMenu(x, y);
        }
        break;
    }
    

    case WM_POINTERDOWN:
    case WM_POINTERUP:
    {
#ifndef TEST_TOUCH_WITH_MOUSE
        if (!GetPointerInfo)
            GetPointerInfo = (pGPI)GetProcAddress(GetModuleHandle(TEXT("user32.dll")),
                "GetPointerInfo");
        if (GetPointerInfo)
#endif
        {
            POINTER_INFO pointerInfo;
#ifdef TEST_TOUCH_WITH_MOUSE
            GetCursorPos(&pointerInfo.ptPixelLocation);
#else
            if (GetPointerInfo(GET_POINTERID_WPARAM(wParam), &pointerInfo))
#endif
            {
                ScreenToClient(pointerInfo.ptPixelLocation);
                for (unsigned int i = 0; i < 8; ++i)
                    if ((m_touchregion_pressed[i] != (uMsg == WM_POINTERDOWN)) && Intersect(touchregion[i], m_width, m_height, pointerInfo.ptPixelLocation, fmodf(m_ptable->m_BG_rotation[m_ptable->m_BG_current_set], 360.0f) != 0.f))
                    {
                        m_touchregion_pressed[i] = (uMsg == WM_POINTERDOWN);

                        DIDEVICEOBJECTDATA didod;
                        didod.dwOfs = m_rgKeys[touchkeymap[i]];
                        didod.dwData = m_touchregion_pressed[i] ? 0x80 : 0;
                        m_pininput.PushQueue(&didod, APP_KEYBOARD/*, curr_time_msec*/);
                    }
            }
        }
        break;
    }
    

    case WM_ACTIVATE:
        if (wParam != WA_INACTIVE)
            SetCursor(NULL);
        {
            if (wParam != WA_INACTIVE)
            {
                m_gameWindowActive = true;
                m_noTimeCorrect = true;
#ifdef STEPPING
                m_pause = false;
#endif
            }
            else
            {
                m_gameWindowActive = false;
#ifdef STEPPING
                m_pause = true;
#endif
            }
            RecomputePauseState();
        }
        break;

    case WM_EXITMENULOOP:
        m_noTimeCorrect = true;
        break;

    case WM_SETCURSOR:
        if (LOWORD(lParam) == HTCLIENT && !m_drawCursor)
        {
            SetCursor(NULL);
        }
        else
        {
            SetCursor(LoadCursor(NULL, IDC_ARROW));
        }
        return TRUE;
    }

    return WndProcDefault(uMsg, wParam, lParam);
}

void Player::StopPlayer()
{
   if (m_audio)
      m_audio->MusicPause();

   // signal the script that the game is now exited to allow any cleanup
   m_ptable->FireVoidEvent(DISPID_GameEvents_Exit);
   if (m_detectScriptHang)
      g_pvp->PostWorkToWorkerThread(HANG_SNOOP_STOP, NULL);

   g_pvp->GetPropertiesDocker()->EnableWindow();
   g_pvp->GetLayersDocker()->EnableWindow();
   g_pvp->GetToolbarDocker()->EnableWindow();
   if(g_pvp->GetNotesDocker()!=nullptr)
      g_pvp->GetNotesDocker()->EnableWindow();
   m_ptable->EnableWindow();

   LockForegroundWindow(false);
}

INT_PTR CALLBACK PauseProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   switch (uMsg)
   {
      case WM_INITDIALOG:
      {
         RECT rcDialog;
         RECT rcMain;
         GetWindowRect(GetParent(hwndDlg), &rcMain);
         GetWindowRect(hwndDlg, &rcDialog);

         SetWindowPos(hwndDlg, NULL,
            (rcMain.right + rcMain.left) / 2 - (rcDialog.right - rcDialog.left) / 2,
            (rcMain.bottom + rcMain.top) / 2 - (rcDialog.bottom - rcDialog.top) / 2,
            0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE/* | SWP_NOMOVE*/);

         return TRUE;
      }
      case WM_COMMAND:
      {
         switch (HIWORD(wParam))
         {
            case BN_CLICKED:
            {
               switch (LOWORD(wParam))
               {
               case IDCANCEL:
               case ID_RESUME:
               {
                  EndDialog(hwndDlg, ID_RESUME);
                  break;
               }
               case ID_DEBUGWINDOW:
               {
                     g_pplayer->m_debugMode = true;
                     if (!g_pplayer->m_debuggerDialog.IsWindow())
                     {
                         g_pplayer->m_debuggerDialog.Create(g_pplayer->GetHwnd());
                         g_pplayer->m_debuggerDialog.ShowWindow();
                     }
                     else
                     {
                         g_pplayer->m_debuggerDialog.ShowWindow(SW_SHOW);
                         g_pplayer->m_debuggerDialog.SetActiveWindow();
                     }

                     EndDialog(hwndDlg, ID_DEBUGWINDOW);
                  break;
               }
               case ID_QUIT:
               {
                  EndDialog(hwndDlg, ID_QUIT);
                  break;
               }
               }
               break;
            }//case BN_CLICKED:
         }//switch (HIWORD(wParam))
      }
   }
   return FALSE;
}

#ifdef PLAYBACK
float Player::ParseLog(LARGE_INTEGER *pli1, LARGE_INTEGER *pli2)
{
   char szLine[MAXSTRING];
   float dtime = 0.45f;

   while (1)
   {
      int c=0;

      while ((szLine[c] = getc(m_fplaylog)) != '\n')
      {
         if (szLine[c] == EOF)
         {
            fclose(m_fplaylog);
            m_playback = false;
            m_fplaylog = NULL;
            return dtime;
         }
         c++;
      }

      char szWord[64];
      char szSubWord[64];
      int index;
      sscanf_s(szLine, "%s",szWord, (unsigned)_countof(szWord));

      if (!strcmp(szWord,"Key"))
      {
         sscanf_s(szLine, "%s %s %d",szWord, (unsigned)_countof(szWord), szSubWord, (unsigned)_countof(szSubWord), &index);
         if (!strcmp(szSubWord, "Down"))
         {
            m_ptable->FireKeyEvent(DISPID_GameEvents_KeyDown, index);
         }
         else // Release
         {
            m_ptable->FireKeyEvent(DISPID_GameEvents_KeyUp, index);
         }
      }
      else if (!strcmp(szWord, "Physics"))
      {
         sscanf_s(szLine, "%s %s %f",szWord, (unsigned)_countof(szWord), szSubWord, (unsigned)_countof(szSubWord), &dtime);
      }
      else if (!strcmp(szWord, "Frame"))
      {
         int a,b,c,d;
         sscanf_s(szLine, "%s %s %f %u %u %u %u",szWord, (unsigned)_countof(szWord), szSubWord, (unsigned)_countof(szSubWord), &dtime, &a, &b, &c, &d);
         pli1->HighPart = a;
         pli1->LowPart = b;
         pli2->HighPart = c;
         pli2->LowPart = d;
      }
      else if (!strcmp(szWord, "Step"))
      {
         int a,b,c,d;
         sscanf_s(szLine, "%s %s %u %u %u %u",szWord, (unsigned)_countof(szWord), szSubWord, (unsigned)_countof(szSubWord), &a, &b, &c, &d);
         pli1->HighPart = a;
         pli1->LowPart = b;
         pli2->HighPart = c;
         pli2->LowPart = d;
      }
      else if (!strcmp(szWord,"End"))
      {
         return dtime;
      }
   }
}

#endif
