// touch defines, delete as soon as we can get rid of old compilers/SDKs and use new ones that have these natively

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
