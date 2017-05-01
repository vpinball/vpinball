#include "stdafx.h"

#include <initguid.h>
//#include <Windowsx.h>
// The GUID used to identify the coclass of the VB Script engine
// 	{B54F3741-5B07-11cf-A4B0-00AA004A55E8}
#define szCLSID_VBScript "{B54F3741-5B07-11cf-A4B0-00AA004A55E8}"
DEFINE_GUID(CLSID_VBScript, 0xb54f3741, 0x5b07, 0x11cf, 0xa4, 0xb0, 0x0, 0xaa, 0x0, 0x4a, 0x55, 0xe8);
//DEFINE_GUID(IID_IActiveScriptParse32, 0xbb1a2ae2, 0xa4f9, 0x11cf, 0x8f, 0x20, 0x0, 0x80, 0x5f, 0x2c, 0xd0, 0x64);
//DEFINE_GUID(IID_IActiveScriptParse64,0xc7ef7658,0xe1ee,0x480e,0x97,0xea,0xd5,0x2c,0xb4,0xd7,0x6d,0x17);
//DEFINE_GUID(IID_IActiveScriptDebug, 0x51973C10, 0xCB0C, 0x11d0, 0xB5, 0xC9, 0x00, 0xA0, 0x24, 0x4A, 0x0E, 0x7A);

#define RECOLOR_LINE	WM_USER+100
#define CONTEXTCOOKIE_NORMAL 1000
#define CONTEXTCOOKIE_DEBUG 1001

UINT g_FindMsgString; // Windows message for the FindText dialog

//Scintillia Lexer parses only lower case unless otherwise told
const char vbsReservedWords[] =
"and as byref byval case call const "
"continue dim do each else elseif end error exit false for function global "
"goto if in loop me new next not nothing on optional or private public "
"redim rem resume select set sub then to true type while with "
"boolean byte currency date double integer long object single string type "
"variant option explicit randomize";

char CaretTextBuff[MAX_FIND_LENGTH];
char ConstructTextBuff[MAX_FIND_LENGTH];

LRESULT CALLBACK CodeViewWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK CVPrefProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

WNDPROC g_RichEditProc;


IScriptable::IScriptable()
{
   m_wzName[0] = 0;
   m_wzMatchName[0] = 0;
}

CodeViewDispatch::CodeViewDispatch()
{
   m_wzName[0] = 0;
}

CodeViewDispatch::~CodeViewDispatch()
{
}

int CodeViewDispatch::SortAgainst(CodeViewDispatch *pcvd/*void *pvoid*/)
{
   return SortAgainstValue(pcvd->m_wzName);
}

int CodeViewDispatch::SortAgainstValue(void *pv)
{
   MAKE_ANSIPTR_FROMWIDE(szName1, (WCHAR *)pv);
   CharLowerBuff(szName1, lstrlen(szName1));
   MAKE_ANSIPTR_FROMWIDE(szName2, m_wzName);
   CharLowerBuff(szName2, lstrlen(szName2));
   return lstrcmp(szName1, szName2); //WideStrCmp((WCHAR *)pv, m_wzName);
}

CodeViewer::CodeViewer()
{
   m_haccel = NULL;
}

void CodeViewer::Init(IScriptableHost *psh)
{
   CComObject<DebuggerModule>::CreateInstance(&m_pdm);
   m_pdm->AddRef();
   m_pdm->Init(this);

   m_psh = psh;

   m_hwndMain = NULL;
   m_hwndFind = NULL;
   m_hwndStatus = NULL;

   szFindString[0] = '\0';
   szReplaceString[0] = '\0';

   g_FindMsgString = RegisterWindowMessage(FINDMSGSTRING);

   m_pScript = NULL;

   const HRESULT res = InitializeScriptEngine();
   if (res != S_OK)
   {
      char bla[128];
      sprintf_s(bla, "Cannot initialize Script Engine 0x%X", res);
      MessageBox(g_pvp->m_hwnd, bla, "Error", MB_ICONERROR);
   }

   m_sdsDirty = eSaveClean;
   m_fIgnoreDirty = fFalse;

   m_findreplaceold.lStructSize = 0; // So we know nothing has been searched for yet

   m_errorLineNumber = -1;
}

CodeViewer::~CodeViewer()
{
   Destroy();

   for (int i = 0; i < m_vcvd.Size(); ++i)
      delete m_vcvd.ElementAt(i);

   if (m_haccel)
      DestroyAcceleratorTable(m_haccel);

   if (g_pvp->m_pcv == this)
      g_pvp->m_pcv = NULL;

   m_pdm->Release();
}

void GetRange(const HWND m_hwndScintilla, const size_t start, const size_t end, char * const text)
{
   Sci_TextRange tr;
   tr.chrg.cpMin = start;
   tr.chrg.cpMax = end;
   tr.lpstrText = text;
   SendMessage(m_hwndScintilla, SCI_GETTEXTRANGE, 0, (LPARAM)&tr);
}

void CodeViewer::GetWordUnderCaret()
{

	const LRESULT CurPos = SendMessage(m_hwndScintilla, SCI_GETCURRENTPOS, 0, 0 );
	WordUnderCaret.chrg.cpMin = SendMessage(m_hwndScintilla, SCI_WORDSTARTPOSITION, CurPos, TRUE);
	WordUnderCaret.chrg.cpMax = SendMessage(m_hwndScintilla, SCI_WORDENDPOSITION, CurPos, TRUE);
	if (( WordUnderCaret.chrg.cpMax - WordUnderCaret.chrg.cpMin) > MAX_FIND_LENGTH) return;

   SendMessage(m_hwndScintilla, SCI_GETTEXTRANGE, 0, (LPARAM)&WordUnderCaret);
}

void CodeViewer::SetClean(const SaveDirtyState sds)
{
   if (sds == eSaveClean)
      SendMessage(m_hwndScintilla, SCI_SETSAVEPOINT, 0, 0);
   m_sdsDirty = sds;
   m_psh->SetDirtyScript(sds);
}

void CodeViewer::EndSession()
{
   CleanUpScriptEngine();

   InitializeScriptEngine();

   for (int i = 0; i < m_vcvdTemp.Size(); ++i)
      delete m_vcvdTemp.ElementAt(i);
   m_vcvdTemp.RemoveAllElements();
}

HRESULT CodeViewer::AddTemporaryItem(const BSTR bstr, IDispatch * const pdisp)
{
   CodeViewDispatch * const pcvd = new CodeViewDispatch();

   WideStrNCopy(bstr, pcvd->m_wzName, 32);
   pcvd->m_pdisp = pdisp;
   pcvd->m_pdisp->QueryInterface(IID_IUnknown, (void **)&pcvd->m_punk);
   pcvd->m_punk->Release();
   pcvd->m_piscript = NULL;
   pcvd->m_fGlobal = fFalse;

   if (m_vcvd.GetSortedIndex(pcvd) != -1 || m_vcvdTemp.GetSortedIndex(pcvd) != -1)
   {
      delete pcvd;
      return E_FAIL; //already exists
   }

   m_vcvdTemp.AddSortedString(pcvd);

   const int flags = SCRIPTITEM_ISSOURCE | SCRIPTITEM_ISVISIBLE;

   /*const HRESULT hr =*/ m_pScript->AddNamedItem(bstr, flags);

   m_pScript->SetScriptState(SCRIPTSTATE_CONNECTED);

   return S_OK;
}

HRESULT CodeViewer::AddItem(IScriptable * const piscript, const BOOL fGlobal)
{
   CodeViewDispatch * const pcvd = new CodeViewDispatch();

   CComBSTR bstr;
   piscript->get_Name(&bstr);

   WideStrNCopy(bstr, pcvd->m_wzName, 32);
   pcvd->m_pdisp = piscript->GetDispatch();
   pcvd->m_pdisp->QueryInterface(IID_IUnknown, (void **)&pcvd->m_punk);
   pcvd->m_punk->Release();
   pcvd->m_piscript = piscript;
   pcvd->m_fGlobal = fGlobal;

   if (m_vcvd.GetSortedIndex(pcvd) != -1)
   {
      delete pcvd;
      return E_FAIL;
   }

   m_vcvd.AddSortedString(pcvd);

   // Add item to dropdown
   char szT[64]; // Names can only be 32 characters (plus terminator)
   WideCharToMultiByte(CP_ACP, 0, pcvd->m_wzName, -1, szT, 64, NULL, NULL);
   const size_t index = SendMessage(m_hwndItemList, CB_ADDSTRING, 0, (size_t)szT);
   SendMessage(m_hwndItemList, CB_SETITEMDATA, index, (size_t)piscript);
	//AndyS - WIP insert new item into autocomplete list??
   return S_OK;
}

void CodeViewer::RemoveItem(IScriptable * const piscript)
{
   CComBSTR bstr;
   piscript->get_Name(&bstr);

   int idx = m_vcvd.GetSortedIndex(bstr);

   if (idx == -1)
      return;

   CodeViewDispatch *pcvd = m_vcvd.ElementAt(idx);

   _ASSERTE(pcvd);

   m_vcvd.RemoveElementAt(idx);

   // Remove item from dropdown
   char szT[64]; // Names can only be 32 characters (plus terminator)
   WideCharToMultiByte(CP_ACP, 0, bstr, -1, szT, 64, NULL, NULL);
   size_t index = SendMessage(m_hwndItemList, CB_FINDSTRINGEXACT, ~0u, (size_t)szT);
   SendMessage(m_hwndItemList, CB_DELETESTRING, index, 0);

   delete pcvd;
}

void CodeViewer::SelectItem(IScriptable * const piscript)
{
   CComBSTR bstr;
   piscript->get_Name(&bstr);

   char szT[64]; // Names can only be 32 characters (plus terminator)
   WideCharToMultiByte(CP_ACP, 0, bstr, -1, szT, 64, NULL, NULL);

   const size_t index = SendMessage(m_hwndItemList, CB_FINDSTRINGEXACT, ~0u, (size_t)szT);
   SendMessage(m_hwndItemList, CB_SETCURSEL, index, 0);

   ListEventsFromItem();
}

HRESULT CodeViewer::ReplaceName(IScriptable * const piscript, WCHAR * const wzNew)
{
   if (m_vcvd.GetSortedIndex(wzNew) != -1)
      return E_FAIL;

   CComBSTR bstr;
   piscript->get_Name(&bstr);

   const int idx = m_vcvd.GetSortedIndex(bstr);
   if (idx == -1)
      return E_FAIL;

   CodeViewDispatch * const pcvd = m_vcvd.ElementAt(idx);

   _ASSERTE(pcvd);

   m_vcvd.RemoveElementAt(idx);

   lstrcpynW(pcvd->m_wzName, wzNew, 32);

   m_vcvd.AddSortedString(pcvd);

   // Remove old name from dropdown and replace it with the new
   char szT[64]; // Names can only be 32 characters (plus terminator)
   WideCharToMultiByte(CP_ACP, 0, bstr, -1, szT, 64, NULL, NULL);
   size_t index = SendMessage(m_hwndItemList, CB_FINDSTRINGEXACT, ~0u, (size_t)szT);
   SendMessage(m_hwndItemList, CB_DELETESTRING, index, 0);

   WideCharToMultiByte(CP_ACP, 0, wzNew, -1, szT, 64, NULL, NULL);
   index = SendMessage(m_hwndItemList, CB_ADDSTRING, 0, (size_t)szT);
   SendMessage(m_hwndItemList, CB_SETITEMDATA, index, (size_t)piscript);

   SendMessage(m_hwndItemList, CB_SETCURSEL, index, 0);
   ListEventsFromItem(); // Just to get us into a good state

   return S_OK;
}

STDMETHODIMP CodeViewer::InitializeScriptEngine()
{
   const HRESULT result = CoCreateInstance(CLSID_VBScript, 0, CLSCTX_ALL/*CLSCTX_INPROC_SERVER*/, IID_IActiveScriptParse, (LPVOID*)&m_pScriptParse); //!! CLSCTX_INPROC_SERVER good enough?!
   if (result == S_OK)
   {
      m_pScriptParse->QueryInterface(IID_IActiveScript,
         (LPVOID*)&m_pScript);

      m_pScriptParse->QueryInterface(IID_IActiveScriptDebug,
         (LPVOID*)&m_pScriptDebug);

      m_pScriptParse->InitNew();
      m_pScript->SetScriptSite(this);

      IObjectSafety *pios;
      m_pScriptParse->QueryInterface(IID_IObjectSafety, (LPVOID*)&pios);

      if (pios)
      {
         DWORD supported, enabled;
         pios->GetInterfaceSafetyOptions(IID_IActiveScript, &supported, &enabled);

         /*const HRESULT hr =*/ pios->SetInterfaceSafetyOptions(IID_IActiveScript, supported, INTERFACE_USES_SECURITY_MANAGER);

         pios->Release();
      }

      return S_OK;
   }

   return result;
}

STDMETHODIMP CodeViewer::CleanUpScriptEngine()
{
   if (m_pScript)
   {
      m_pScript->SetScriptState(SCRIPTSTATE_DISCONNECTED);
      m_pScript->SetScriptState(SCRIPTSTATE_CLOSED);
      m_pScript->Release();
      m_pScriptParse->Release();
      m_pScriptDebug->Release();
   }
   return S_OK;
}

void CodeViewer::SetVisible(const BOOL fVisible)
{

   if(!fVisible)
   {
       RECT rc;
       GetWindowRect( m_hwndMain, &rc );
       SetRegValue( "Editor", "CodeViewPosX", REG_DWORD, &rc.left, 4 );
       SetRegValue( "Editor", "CodeViewPosY", REG_DWORD, &rc.top, 4 );
       const int w = rc.right - rc.left;
       SetRegValue( "Editor", "CodeViewPosWidth", REG_DWORD, &w, 4 );
       const int h = rc.bottom - rc.top;
       SetRegValue( "Editor", "CodeViewPosHeight", REG_DWORD, &h, 4 );
   }

   if (m_hwndFind && !fVisible)
   {
      DestroyWindow(m_hwndFind);
      m_hwndFind = NULL;
   }

   if (IsIconic(m_hwndMain))
   {
      // SW_RESTORE usually works in all cases, but if the window
      // is maximized, we don't want to restore to a smaller size,
      // so we check IsIconic to only restore in the minimized state.
      ShowWindow(m_hwndMain, fVisible ? SW_RESTORE : SW_HIDE);
   }
   else
      ShowWindow(m_hwndMain, fVisible ? SW_SHOW : SW_HIDE);

   if(fVisible)
   {
       const int x = GetRegIntWithDefault( "Editor", "CodeViewPosX", 0 );
       const int y = GetRegIntWithDefault( "Editor", "CodeViewPosY", 0 );
       const int w = GetRegIntWithDefault( "Editor", "CodeViewPosWidth", 640 );
       const int h = GetRegIntWithDefault( "Editor", "CodeViewPosHeight", 490 );
       SetWindowPos( m_hwndMain, HWND_TOP, x, y, w, h, SWP_NOMOVE | SWP_NOSIZE );
   }
}

void CodeViewer::SetEnabled(const BOOL fEnabled)
{
   SendMessage(m_hwndScintilla, SCI_SETREADONLY, !fEnabled, 0);

   EnableWindow(m_hwndItemList, fEnabled);
   EnableWindow(m_hwndEventList, fEnabled);
}

void CodeViewer::SetCaption(const char * const szCaption)
{
   char szT[_MAX_PATH];
   strcpy_s(szT, sizeof(szT), szCaption);
   LocalString ls(IDS_SCRIPT);
   strcat_s(szT, sizeof(szT), " ");
   strcat_s(szT, sizeof(szT), ls.m_szbuffer);
   SetWindowText(m_hwndMain, szT);
}

void CodeViewer::UpdatePrefsfromReg()
{
	crBackColor = GetRegIntWithDefault("CVEdit", "BackGroundColor", RGB(255,255,255));
	DisplayAutoComplete = GetRegBoolWithDefault("CVEdit", "DisplayAutoComplete", true );
	DisplayAutoCompleteLength = GetRegIntWithDefault("CVEdit", "DisplayAutoCompleteAfter", 1);
	DwellDisplay = GetRegBoolWithDefault("CVEdit", "DwellDisplay", true );
	DwellHelp = GetRegBoolWithDefault("CVEdit", "DwellHelp", true );
	DwellDisplayTime = GetRegIntWithDefault("CVEdit", "DwellDisplayTime", 700);
	for (size_t i = 0; i < lPrefsList->size(); ++i)
	{
		lPrefsList->at(i)->GetPrefsFromReg();
	}
}

void CodeViewer::UpdateRegWithPrefs()
{
	SetRegValueInt("CVEdit", "BackGroundColor", crBackColor);
	SetRegValueBool("CVEdit","DisplayAutoComplete", DisplayAutoComplete);
	SetRegValueInt("CVEdit","DisplayAutoCompleteAfter", DisplayAutoCompleteLength);
	SetRegValueBool("CVEdit","DwellDisplay", DwellDisplay);
	SetRegValueBool("CVEdit","DwellHelp", DwellHelp);
	SetRegValueInt("CVEdit","DwellDisplayTime", DwellDisplayTime);
	for (size_t i = 0; i < lPrefsList->size(); i++)
	{
		lPrefsList->at(i)->SetPrefsToReg();
	}
}

void CodeViewer::InitPreferences()
{
	for (int i = 0 ; i<16 ; ++i)
	{
		g_PrefCols[i] = 0;
	}
	crBackColor= RGB(255,255,255);
	lPrefsList = new vector<CVPrefrence*>();

	prefEverythingElse = new CVPrefrence();
	prefEverythingElse->FillCVPreference("EverythingElse", RGB(0,0,0), true, "EverythingElse",  STYLE_DEFAULT, 0 , IDC_CVP_BUT_COL_EVERYTHINGELSE, IDC_CVP_BUT_FONT_EVERYTHINGELSE);
	lPrefsList->push_back(prefEverythingElse);
	prefDefault = new CVPrefrence();
	prefDefault->FillCVPreference("Default", RGB(0,0,0), true, "Default", SCE_B_DEFAULT, 0 , 0, 0);
	lPrefsList->push_back(prefDefault);
	prefVBS = new CVPrefrence();
	prefVBS->FillCVPreference("VBs", RGB(0,0,160), true, "ShowVBS", SCE_B_KEYWORD, IDC_CVP_CHECKBOX_VBS, IDC_CVP_BUT_COL_VBS, IDC_CVP_BUT_FONT_VBS);
	lPrefsList->push_back(prefVBS);
	prefComps = new CVPrefrence();
	prefComps->FillCVPreference("Components", RGB(120,120,0), true, "ShowComponents", SCE_B_KEYWORD3, IDC_CVP_CHKB_COMP, IDC_CVP_BUT_COL_COMPS, IDC_CVP_BUT_FONT_COMPS);
	lPrefsList->push_back(prefComps);
	prefSubs = new CVPrefrence();
	prefSubs->FillCVPreference("SubFuns", RGB(120,0,120), true, "ShowSubs", SCE_B_KEYWORD2, IDC_CVP_CHKB_SUBS, IDC_CVP_BUT_COL_SUBS, IDC_CVP_BUT_FONT_SUBS);
	lPrefsList->push_back(prefSubs);
	prefComments = new CVPrefrence();
	prefComments->FillCVPreference("Remarks", RGB(0,120,0), true, "ShowRemarks", SCE_B_COMMENT, IDC_CVP_CHKB_COMMENTS, IDC_CVP_BUT_COL_COMMENTS, IDC_CVP_BUT_FONT_COMMENTS);
	lPrefsList->push_back(prefComments);
	prefLiterals = new CVPrefrence();
	prefLiterals->FillCVPreference("Literals", RGB(0,120,160), true, "ShowLierals", SCE_B_STRING, IDC_CVP_CHKB_LITERALS, IDC_CVP_BUT_COL_LITERALS, IDC_CVP_BUT_FONT_LITERALS);
	lPrefsList->push_back(prefLiterals);
	prefVPcore = new CVPrefrence();
	prefVPcore->FillCVPreference("VPcore", RGB(200,50,60), true, "ShowVPcore", SCE_B_KEYWORD4, IDC_CVP_CHKB_VPCORE, IDC_CVP_BUT_COL_VPCORE, IDC_CVP_BUT_FONT_VPCORE);
	lPrefsList->push_back(prefVPcore);
	for (size_t i = 0; i < lPrefsList->size(); ++i)
	{
		CVPrefrence* Pref = lPrefsList->at(i);
		Pref->SetDefaultFont(m_hwndMain);
	}
	// load prefs from registery
	UpdatePrefsfromReg();
}

void CodeViewer::Create()
{
   m_haccel = LoadAccelerators(g_hinst, MAKEINTRESOURCE(IDR_CODEVIEWACCEL));// Accelerator keys

   WNDCLASSEX wcex;
   ZeroMemory(&wcex, sizeof(WNDCLASSEX));
   wcex.cbSize = sizeof(WNDCLASSEX);
   wcex.style = CS_DBLCLKS;
   wcex.lpfnWndProc = (WNDPROC)CodeViewWndProc;
   wcex.hInstance = g_hinst;
   wcex.hIcon = LoadIcon(g_hinst, MAKEINTRESOURCE(IDI_SCRIPT));
   wcex.lpszClassName = "CVFrame";
   wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
   wcex.lpszMenuName = MAKEINTRESOURCE(IDR_SCRIPTMENU);//NULL;
   wcex.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
   RegisterClassEx(&wcex);

   const int x = GetRegIntWithDefault( "Editor", "CodeViewPosX", 0 );
   const int y = GetRegIntWithDefault( "Editor", "CodeViewPosY", 0 );
   const int w = GetRegIntWithDefault( "Editor", "CodeViewPosWidth", 640 );
   const int h = GetRegIntWithDefault( "Editor", "CodeViewPosHeight", 490 );

   m_hwndMain = CreateWindowEx(0, "CVFrame", "Script",
      WS_POPUP | WS_SIZEBOX | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
      x, y, w, h, m_hwndMain, NULL, g_hinst, 0);

   SetWindowLongPtr(m_hwndMain, GWLP_USERDATA, (size_t)this);


   m_hwndScintilla = CreateWindowEx(0, "Scintilla", "",
      WS_CHILD | ES_NOHIDESEL | WS_VISIBLE | ES_SUNKEN | WS_HSCROLL | WS_VSCROLL | ES_MULTILINE | ES_WANTRETURN,
      0, 10 + 32, w, h-10, m_hwndMain, NULL, g_hinst, 0);

	//if still using old dll load VB lexer insted
	//use SCI_SETLEXERLANGUAGE as SCI_GETLEXER doesn't return the correct value with SCI_SETLEXER
	SendMessage(m_hwndScintilla, SCI_SETLEXERLANGUAGE, 0 , (LPARAM)"vpscript");
	LRESULT lexVersion = SendMessage(m_hwndScintilla, SCI_GETLEXER, 0, 0);
	if (lexVersion != SCLEX_VPSCRIPT)
	{
		SendMessage(m_hwndScintilla, SCI_SETLEXER, (WPARAM)SCLEX_VBSCRIPT, 0);
	}
	char szValidChars[256] = {};

   SendMessage(m_hwndScintilla, SCI_GETWORDCHARS, 0, (LPARAM)szValidChars);
   ValidChars = string(szValidChars);
	VBValidChars = string("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_");
	StopErrorDisplay = false;
// Create new list of user functions & Collections- filled in ParseForFunction(), first called in LoadFromStrem()
	AutoCompList = new vector<string>();
	ComponentsDict = new vector<UserData>();
	PageConstructsDict = new vector<UserData>();
	VP_CoreDict = new vector<UserData>();
	CurrentMembers = new vector<UserData>();
   WordUnderCaret.lpstrText = NULL;
	WordUnderCaret.lpstrText = (char *) CaretTextBuff;
	CurrentConstruct.lpstrText = NULL;
	CurrentConstruct.lpstrText = (char *) ConstructTextBuff;
	// parse vb reserved words for auto complete.
	VBwordsDict = new vector<UserData>;
	int intWordFinish = -1; //skip space
	char WordChar= vbsReservedWords[0];
	char szWord[256];
	int wordlen=0;
	UserData VBWord;
	while (WordChar != 0) //Just make sure with chars, we reached EOL
	{
		memset(szWord,0,256);
		wordlen = 0;
		intWordFinish++; //skip space
		WordChar = vbsReservedWords[intWordFinish];
		while (WordChar != 0 && WordChar != ' ')
		{
			szWord[wordlen]=WordChar;
			intWordFinish++;
			wordlen++;
			WordChar= vbsReservedWords[intWordFinish];
		}
		VBWord.KeyName = string(szWord);
		VBWord.UniqueKey = VBWord.KeyName;
		FindOrInsertUD(VBwordsDict, VBWord);
	}
	vbsKeyWords.clear();// For colouring scintilla
	for (vector<UserData>::iterator i = VBwordsDict->begin();i != VBwordsDict->end(); ++i)
	{
		//make vbsKeyWords in order.
		vbsKeyWords += lowerCase( i->KeyName);
		vbsKeyWords += " ";
		//Then capitalise first letter
		WordChar = i->KeyName.at(0);
		if (WordChar >= 'a' && WordChar <= 'z' ) WordChar -= ('a'- 'A');
		i->KeyName.at(0) = WordChar;	
	}
	///// Preferences
	InitPreferences();

   SendMessage(m_hwndScintilla, SCI_SETMODEVENTMASK, SC_MOD_INSERTTEXT, 0);
   SendMessage(m_hwndScintilla, SCI_SETKEYWORDS, 0, (LPARAM)vbsReservedWords );
   SendMessage(m_hwndScintilla, SCI_SETTABWIDTH, 4, 0);


   // The null visibility policy is like Visual Studio - if a search goes
   // off the screen, the newly selected text is placed in the middle of the
   // screen
   SendMessage(m_hwndScintilla, SCI_SETVISIBLEPOLICY, 0, 0);
	//Set up line numbering
   SendMessage(m_hwndScintilla, SCI_SETMARGINTYPEN, 0, SC_MARGIN_NUMBER);
   SendMessage(m_hwndScintilla, SCI_SETMARGINSENSITIVEN, 1, 1);
   SendMessage(m_hwndScintilla, SCI_SETMARGINWIDTHN, 0, 40);
	//Cursor line dimmed
   SendMessage(m_hwndScintilla, SCI_SETCARETLINEVISIBLE, 1, 0);
   SendMessage(m_hwndScintilla, SCI_SETCARETLINEBACK, RGB(240, 240, 255), 0);
	//Highlight Errors
   SendMessage(m_hwndScintilla, SCI_INDICSETSTYLE, 0, INDIC_ROUNDBOX);
   SendMessage(m_hwndScintilla, SCI_SETINDICATORCURRENT, 0, 0);
   SendMessage(m_hwndScintilla, SCI_INDICSETFORE, 0, RGB(255, 0, 0));
   SendMessage(m_hwndScintilla, SCI_INDICSETALPHA, 0, 90);
	//Set up folding.
   SendMessage(m_hwndScintilla, SCI_SETPROPERTY, (WPARAM)"fold", (LPARAM)"1");
   SendMessage(m_hwndScintilla, SCI_SETPROPERTY, (WPARAM)"fold.compact", (LPARAM)"0");
	//Set up folding margin
   SendMessage(m_hwndScintilla, SCI_SETMARGINTYPEN, 1, SC_MARGIN_SYMBOL);
   SendMessage(m_hwndScintilla, SCI_SETMARGINMASKN, 1, SC_MASK_FOLDERS);
   SendMessage(m_hwndScintilla, SCI_SETMARGINWIDTHN, 1, 20);

   SendMessage(m_hwndScintilla, SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPEN, SC_MARK_MINUS);
	SendMessage(m_hwndScintilla, SCI_MARKERSETFORE, SC_MARKNUM_FOLDEROPEN, prefEverythingElse->rgb);
	SendMessage(m_hwndScintilla, SCI_MARKERSETBACK, SC_MARKNUM_FOLDEROPEN, crBackColor);
	//WIP markers
   SendMessage(m_hwndScintilla, SCI_MARKERDEFINE, SC_MARKNUM_FOLDER, SC_MARK_PLUS);
	SendMessage(m_hwndScintilla, SCI_MARKERSETFORE, SC_MARKNUM_FOLDER, prefEverythingElse->rgb);
	SendMessage(m_hwndScintilla, SCI_MARKERSETBACK, SC_MARKNUM_FOLDER, crBackColor);
   SendMessage(m_hwndScintilla, SCI_MARKERDEFINE, SC_MARKNUM_FOLDERSUB, SC_MARK_EMPTY);
	SendMessage(m_hwndScintilla, SCI_MARKERSETFORE, SC_MARKNUM_FOLDERSUB, prefEverythingElse->rgb);
	SendMessage(m_hwndScintilla, SCI_MARKERSETBACK, SC_MARKNUM_FOLDERSUB, crBackColor);
   SendMessage(m_hwndScintilla, SCI_MARKERDEFINE, SC_MARKNUM_FOLDERTAIL, SC_MARK_EMPTY);
	SendMessage(m_hwndScintilla, SCI_MARKERSETFORE, SC_MARKNUM_FOLDERTAIL, prefEverythingElse->rgb);
	SendMessage(m_hwndScintilla, SCI_MARKERSETBACK, SC_MARKNUM_FOLDERTAIL, crBackColor);
   SendMessage(m_hwndScintilla, SCI_MARKERDEFINE, SC_MARKNUM_FOLDEREND, SC_MARK_EMPTY);
	SendMessage(m_hwndScintilla, SCI_MARKERSETFORE, SC_MARKNUM_FOLDEREND, prefEverythingElse->rgb);
	SendMessage(m_hwndScintilla, SCI_MARKERSETBACK, SC_MARKNUM_FOLDEREND, crBackColor);
   SendMessage(m_hwndScintilla, SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPENMID, SC_MARK_EMPTY);
	SendMessage(m_hwndScintilla, SCI_MARKERSETFORE, SC_MARKNUM_FOLDEROPENMID, prefEverythingElse->rgb);
	SendMessage(m_hwndScintilla, SCI_MARKERSETBACK, SC_MARKNUM_FOLDEROPENMID, crBackColor);
   SendMessage(m_hwndScintilla, SCI_MARKERDEFINE, SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_EMPTY);
	SendMessage(m_hwndScintilla, SCI_MARKERSETFORE, SC_MARKNUM_FOLDERMIDTAIL, prefEverythingElse->rgb);
	SendMessage(m_hwndScintilla, SCI_MARKERSETBACK, SC_MARKNUM_FOLDERMIDTAIL, crBackColor);

   SendMessage(m_hwndScintilla, SCI_STYLESETSIZE, STYLE_DEFAULT, 10);
   SendMessage(m_hwndScintilla, SCI_STYLESETFONT, STYLE_DEFAULT, (LPARAM)"Courier");

   SendMessage(m_hwndScintilla, SCI_STYLESETFORE, SCE_B_PREPROCESSOR, RGB(255, 0, 0));
   SendMessage(m_hwndScintilla, SCI_STYLESETFORE, SCE_B_OPERATOR, RGB(0, 0, 160));
   SendMessage(m_hwndScintilla, SCI_STYLESETFORE, SCE_B_IDENTIFIER, RGB(0, 0, 0));
   SendMessage(m_hwndScintilla, SCI_STYLESETFORE, SCE_B_DATE, RGB(0, 0, 0));

	SendMessage(m_hwndScintilla, SCI_SETWORDCHARS, 0, (LPARAM)"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_");

	//SendMessage(m_hwndScintilla, SCI_SETMOUSEDOWNCAPTURES ,0,0); //send mouse events through scintilla.
	SendMessage(m_hwndScintilla, SCI_AUTOCSETIGNORECASE, TRUE, 0);
	SendMessage(m_hwndScintilla, SCI_AUTOCSETCASEINSENSITIVEBEHAVIOUR, SC_CASEINSENSITIVEBEHAVIOUR_IGNORECASE,0);

	SendMessage(m_hwndScintilla, SCI_AUTOCSETFILLUPS, 0,(LPARAM) "[]{}()");
	SendMessage(m_hwndScintilla, SCI_AUTOCSTOPS, 0,(LPARAM) " ");

	//////////////////////// Status Window (& Sizing Box)

   m_hwndStatus = CreateStatusWindow((WS_CHILD | WS_VISIBLE), "", m_hwndMain, 1);

   const int foo[4] = { 120, 320, 350, 400 };
   SendMessage(m_hwndStatus, SB_SETPARTS, 4, (size_t)foo);

   /////////////////// Compile / Find Buttons

   /////////////////// Item / Event Lists

   m_hwndItemList = CreateWindowEx(0, "ComboBox", "Objects",
      WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_SORT | WS_VSCROLL,
      5, 17, 180, 400, m_hwndMain, NULL, g_hinst, 0);
   SetWindowLongPtr(m_hwndItemList, GWL_ID, IDC_ITEMLIST);
   SendMessage(m_hwndItemList, WM_SETFONT, (size_t)GetStockObject(DEFAULT_GUI_FONT), 0);

   m_hwndItemText = CreateWindowEx(0, "Static", "ObjectsText",
		WS_CHILD | WS_VISIBLE | SS_SIMPLE, 5, 0, 180, 15, m_hwndMain, NULL, g_hinst, 0);
   SetWindowText(m_hwndItemText, "Table component:" );
   SendMessage(m_hwndItemText, WM_SETFONT, (size_t)GetStockObject(DEFAULT_GUI_FONT), 0);

	m_hwndEventList = CreateWindowEx(0, "ComboBox", "Events",
      WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_SORT | WS_VSCROLL,
      210 + 5, 17, 180, 400, m_hwndMain, NULL, g_hinst, 0);
   SetWindowLongPtr(m_hwndEventList, GWL_ID, IDC_EVENTLIST);
   SendMessage(m_hwndEventList, WM_SETFONT, (size_t)GetStockObject(DEFAULT_GUI_FONT), 0);

   m_hwndEventText = CreateWindowEx(0, "Static", "EventsText",
		WS_CHILD | WS_VISIBLE | SS_SIMPLE, 210 + 5, 0, 180, 15, m_hwndMain, NULL, g_hinst, 0);
   SetWindowText(m_hwndEventText, "Create Sub from component:" );
   SendMessage(m_hwndEventText, WM_SETFONT, (size_t)GetStockObject(DEFAULT_GUI_FONT), 0);
	
	m_hwndFunctionList = CreateWindowEx(0, "ComboBox", "Functions",
      WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
      430 + 5, 17, 180, 400, m_hwndMain, NULL, g_hinst, 0);
   SetWindowLongPtr(m_hwndFunctionList, GWL_ID, IDC_FUNCTIONLIST);
   SendMessage(m_hwndFunctionList, WM_SETFONT, (size_t)GetStockObject(DEFAULT_GUI_FONT), 0);

    m_hwndFunctionText = CreateWindowEx(0, "Static", "FunctionsText",
		WS_CHILD | WS_VISIBLE | SS_SIMPLE, 430 + 5, 0, 180, 15, m_hwndMain, NULL, g_hinst, 0);
   SetWindowText(m_hwndFunctionText, "Go to Sub/Function:" );
   SendMessage(m_hwndFunctionText, WM_SETFONT, (size_t)GetStockObject(DEFAULT_GUI_FONT), 0);
	ParseVPCore();
	UpdateScinFromPrefs();

	SendMessage(m_hwndMain, WM_SIZE, 0, 0); // Make our window relay itself out

}

void CodeViewer::Destroy()
{
	if (prefEverythingElse) delete prefEverythingElse;
	if (prefDefault) delete prefDefault;
	if (prefVBS) delete prefVBS;
	if (prefComps) delete prefComps;
	if (prefSubs) delete prefSubs;
	if (prefComments) delete prefComments;
	if (prefLiterals) delete prefLiterals;
	if (prefVPcore) delete prefVPcore;
	if (lPrefsList)
	{
		lPrefsList->clear();
		delete lPrefsList;
	}
	if(AutoCompList)
	{
		AutoCompList->clear();
		delete AutoCompList;
	}
	AutoCompList = 0;

	if(ComponentsDict)
	{
		ComponentsDict->clear();
		delete ComponentsDict;
	}
	ComponentsDict = 0;

	if(PageConstructsDict)
	{
		PageConstructsDict->clear();
		delete PageConstructsDict;
	}
	PageConstructsDict = 0;
	if(VBwordsDict)
	{
		VBwordsDict->clear();
		delete VBwordsDict;
	}
	VBwordsDict = 0;
	if(CurrentMembers)
	{
		CurrentMembers->clear();
		delete CurrentMembers;
	}
	CurrentMembers = 0;
	if(VP_CoreDict)
	{
		VP_CoreDict->clear();
		delete VP_CoreDict;
	}
	VP_CoreDict = 0;

	if (m_hwndFind) DestroyWindow(m_hwndFind);

   DestroyWindow(m_hwndMain);
}

STDMETHODIMP CodeViewer::GetItemInfo(LPCOLESTR pstrName, DWORD dwReturnMask,
   IUnknown **ppiunkItem, ITypeInfo **ppti)
{
   if (dwReturnMask & SCRIPTINFO_IUNKNOWN)
      *ppiunkItem = 0;
   if (dwReturnMask & SCRIPTINFO_ITYPEINFO)
      *ppti = 0;

   CodeViewDispatch *pcvd = m_vcvd.GetSortedElement((void *)pstrName);

   if (pcvd == NULL)
   {
      pcvd = m_vcvdTemp.GetSortedElement((void *)pstrName);
      if (pcvd == NULL)
         return E_FAIL;
   }

   if (dwReturnMask & SCRIPTINFO_IUNKNOWN)
   {
      if (*ppiunkItem = pcvd->m_punk)
         (*ppiunkItem)->AddRef();
   }

   if (dwReturnMask & SCRIPTINFO_ITYPEINFO)
   {
      IProvideClassInfo* pClassInfo;
      pcvd->m_punk->QueryInterface(IID_IProvideClassInfo,
         (LPVOID*)&pClassInfo);
      if (pClassInfo)
      {
         pClassInfo->GetClassInfo(ppti);
         pClassInfo->Release();
      }
   }

   return S_OK;
}

STDMETHODIMP CodeViewer::OnScriptError(IActiveScriptError *pscripterror)
{
   DWORD dwCookie;
   LONG nChar;
   ULONG nLine;
   BSTR bstr = 0;
   EXCEPINFO ei;
   ZeroMemory(&ei, sizeof(ei));
   pscripterror->GetSourcePosition(&dwCookie, &nLine, &nChar);
   pscripterror->GetSourceLineText(&bstr);
   pscripterror->GetExceptionInfo(&ei);
   nLine++;
   if (dwCookie == CONTEXTCOOKIE_DEBUG)
   {
      char *szT = MakeChar(ei.bstrDescription);
      AddToDebugOutput(szT);
      delete[] szT;
      SysFreeString( bstr );
      return S_OK;
   }

   m_fScriptError = fTrue;

   PinTable * const pt = g_pvp->GetActiveTable();
   if (pt != NULL && !pt->CheckPermissions(DISABLE_TABLEVIEW))
   {
      SetVisible(fTrue);
      ShowWindow(m_hwndMain, SW_RESTORE);
      ColorError(nLine, nChar);
   }

   OLECHAR wszOutput[MAX_LINE_LENGTH];
   swprintf_s(wszOutput, L"Line: %u\n%s",
      nLine, ei.bstrDescription);

   SysFreeString(bstr);
   SysFreeString(ei.bstrSource);
   SysFreeString(ei.bstrDescription);
   SysFreeString(ei.bstrHelpFile);

   if (g_pplayer)
      EnableWindow(g_pplayer->m_hwnd, FALSE);
   EnableWindow(g_pvp->m_hwnd, FALSE);

   /*const int result =*/ MessageBoxW(m_hwndMain,
      wszOutput,
      L"Script Error",
      MB_SETFOREGROUND);

   EnableWindow(g_pvp->m_hwnd, TRUE);

   if (pt != NULL && !pt->CheckPermissions(DISABLE_TABLEVIEW))
      SetFocus(m_hwndScintilla);

   return S_OK;
}

void CodeViewer::Compile()
{
   if (m_pScript)
   {
      const size_t cchar = SendMessage(m_hwndScintilla, SCI_GETTEXTLENGTH, 0, 0);

      char * const szText = new char[cchar + 1];
      WCHAR * const wzText = new WCHAR[cchar + 1];

      SendMessage(m_hwndScintilla, SCI_GETTEXT, cchar + 1, (size_t)szText);
      MultiByteToWideChar(CP_ACP, 0, szText, -1, wzText, (int)cchar);
      wzText[cchar] = L'\0';

      EXCEPINFO exception;
      ZeroMemory(&exception, sizeof(exception));
      m_pScript->SetScriptState(SCRIPTSTATE_INITIALIZED);

      /*const HRESULT hr =*/ m_pScript->AddTypeLib(LIBID_VPinballLib, 1, 0, 0);

      for (int i = 0; i < m_vcvd.Size(); ++i)
      {
         int flags = SCRIPTITEM_ISSOURCE | SCRIPTITEM_ISVISIBLE;
         if (m_vcvd.ElementAt(i)->m_fGlobal)
            flags |= SCRIPTITEM_GLOBALMEMBERS;
         m_pScript->AddNamedItem(m_vcvd.ElementAt(i)->m_wzName, flags);
      }

      m_pScriptParse->ParseScriptText(wzText, 0, 0, 0, CONTEXTCOOKIE_NORMAL, 0,
         SCRIPTTEXT_ISVISIBLE, 0, &exception);

      m_pScript->SetScriptState(SCRIPTSTATE_INITIALIZED);

      delete[] wzText;
      delete[] szText;
   }
}

void CodeViewer::Start()
{
   //ShowError("CodeViewer::Start"); //debug logging BDS
   if (m_pScript)
      m_pScript->SetScriptState(SCRIPTSTATE_CONNECTED);
}

void CodeViewer::EvaluateScriptStatement(const char * const szScript)
{
   EXCEPINFO exception;
   ZeroMemory(&exception, sizeof(exception));

   const int scriptlen = lstrlen(szScript);
   WCHAR * const wzScript = new WCHAR[scriptlen + 1];

   MultiByteToWideChar(CP_ACP, 0, szScript, -1, wzScript, scriptlen + 1);
   wzScript[scriptlen] = L'\0';

   m_pScriptParse->ParseScriptText(wzScript, L"Debug", 0, 0, CONTEXTCOOKIE_DEBUG, 0, 0, NULL, &exception);

   delete[] wzScript;
}

void CodeViewer::AddToDebugOutput(const char * const szText)
{
   SendMessage(g_pplayer->m_hwndDebugOutput, SCI_ADDTEXT, lstrlen(szText), (LPARAM)szText);
   SendMessage(g_pplayer->m_hwndDebugOutput, SCI_ADDTEXT, lstrlen("\n"), (LPARAM)"\n");

   const size_t pos = SendMessage(g_pplayer->m_hwndDebugOutput, SCI_GETCURRENTPOS, 0, 0);
   const size_t line = SendMessage(g_pplayer->m_hwndDebugOutput, SCI_LINEFROMPOSITION, pos, 0);
   SendMessage(g_pplayer->m_hwndDebugOutput, SCI_ENSUREVISIBLEENFORCEPOLICY, line, 0);
}

void CodeViewer::ShowFindDialog()
{
   if (m_hwndFind == NULL)
   {
		WordUnderCaret.lpstrText = szFindString;
 		GetWordUnderCaret();

		m_findreplacestruct.lStructSize = sizeof(FINDREPLACE);
		m_findreplacestruct.hwndOwner = m_hwndMain;
		m_findreplacestruct.hInstance = NULL;
		m_findreplacestruct.Flags = FR_DOWN | FR_HIDEWHOLEWORD;
		m_findreplacestruct.lpstrFindWhat = szFindString;
		m_findreplacestruct.lpstrReplaceWith = NULL;
      m_findreplacestruct.wFindWhatLen = 80;
      m_findreplacestruct.wReplaceWithLen = 0;
      m_findreplacestruct.lCustData = 0;
      m_findreplacestruct.lpfnHook = NULL;
      m_findreplacestruct.lpTemplateName = NULL;

      m_hwndFind = FindText(&m_findreplacestruct);
   }
}

void CodeViewer::ShowFindReplaceDialog()
{
   if (m_hwndFind == NULL)
   {
		WordUnderCaret.lpstrText = szFindString;
 		GetWordUnderCaret();

      m_findreplacestruct.lStructSize = sizeof(FINDREPLACE);
      m_findreplacestruct.hwndOwner = m_hwndMain;
      m_findreplacestruct.hInstance = NULL;
      m_findreplacestruct.Flags = FR_DOWN | FR_HIDEWHOLEWORD;
      m_findreplacestruct.lpstrFindWhat = szFindString;
      m_findreplacestruct.lpstrReplaceWith = szReplaceString;
      m_findreplacestruct.wFindWhatLen = 80;
      m_findreplacestruct.wReplaceWithLen = 80;
      m_findreplacestruct.lCustData = 0;
      m_findreplacestruct.lpfnHook = NULL;
      m_findreplacestruct.lpTemplateName = NULL;

      m_hwndFind = ReplaceText(&m_findreplacestruct);
   }
}

void CodeViewer::Find(const FINDREPLACE * const pfr)
{
   if (pfr->lStructSize == 0) // Our built-in signal that we are doing 'find next' and nothing has been searched for yet
      return;

   m_findreplaceold = *pfr;

   const size_t selstart = SendMessage(m_hwndScintilla, SCI_GETSELECTIONSTART, 0, 0);
   const size_t selend = SendMessage(m_hwndScintilla, SCI_GETSELECTIONEND, 0, 0);

   size_t startChar, stopChar;

   if (pfr->Flags & FR_DOWN)
   {
      const size_t len = SendMessage(m_hwndScintilla, SCI_GETTEXTLENGTH, 0, 0);
      startChar = selend;
      stopChar = len;
   }
   else
   {
      startChar = selstart - 1;
      stopChar = 0;
   }

   const int scinfindflags = ((pfr->Flags & FR_WHOLEWORD) ? SCFIND_WHOLEWORD : 0) |
      ((pfr->Flags & FR_MATCHCASE) ? SCFIND_MATCHCASE : 0) /*|
                                                           ((pfr->Flags & 0) ? SCFIND_REGEXP : 0)*/;

   SendMessage(m_hwndScintilla, SCI_SETTARGETSTART, startChar, 0);
   SendMessage(m_hwndScintilla, SCI_SETTARGETEND, stopChar, 0);
   SendMessage(m_hwndScintilla, SCI_SETSEARCHFLAGS, scinfindflags, 0);
   size_t posFind = SendMessage(m_hwndScintilla, SCI_SEARCHINTARGET, lstrlen(pfr->lpstrFindWhat), (LPARAM)pfr->lpstrFindWhat);

   BOOL fWrapped = fFalse;

   if (posFind == -1)
   {
      // Not found, try looping the document
      fWrapped = fTrue;
      if (pfr->Flags & FR_DOWN)
      {
         startChar = 0;
         stopChar = selstart;
      }
      else
      {
         const size_t len = SendMessage(m_hwndScintilla, SCI_GETTEXTLENGTH, 0, 0);
         startChar = len;
         stopChar = selend;
      }

      SendMessage(m_hwndScintilla, SCI_SETTARGETSTART, startChar, 0);
      SendMessage(m_hwndScintilla, SCI_SETTARGETEND, stopChar, 0);

      posFind = SendMessage(m_hwndScintilla, SCI_SEARCHINTARGET, lstrlen(pfr->lpstrFindWhat), (LPARAM)pfr->lpstrFindWhat);
   }

   if (posFind != -1)
   {
      const size_t start = SendMessage(m_hwndScintilla, SCI_GETTARGETSTART, 0, 0);
      const size_t end = SendMessage(m_hwndScintilla, SCI_GETTARGETEND, 0, 0);
      const size_t lineStart = SendMessage(m_hwndScintilla, SCI_LINEFROMPOSITION, min(start, end), 0);
      const size_t lineEnd = SendMessage(m_hwndScintilla, SCI_LINEFROMPOSITION, max(start, end), 0);
      for (size_t line = lineStart; line <= lineEnd; ++line)
         SendMessage(m_hwndScintilla, SCI_ENSUREVISIBLEENFORCEPOLICY, line, 0);
      SendMessage(m_hwndScintilla, SCI_SETSEL, start, end);

      if (!fWrapped)
         SendMessage(m_hwndStatus, SB_SETTEXT, 1 | 0, (size_t)"");
      else
      {
         LocalString ls(IDS_FINDLOOPED);
         SendMessage(m_hwndStatus, SB_SETTEXT, 1 | 0, (size_t)ls.m_szbuffer);
      }
   }
   else
   {
      char szT[MAX_PATH];
      LocalString ls(IDS_FINDFAILED);
      LocalString ls2(IDS_FINDFAILED2);
      lstrcpy(szT, ls.m_szbuffer);
      lstrcat(szT, pfr->lpstrFindWhat);
      lstrcat(szT, ls2.m_szbuffer);
      MessageBeep(MB_ICONEXCLAMATION);
      SendMessage(m_hwndStatus, SB_SETTEXT, 1 | 0, (size_t)szT);
   }
}

void CodeViewer::Replace(const FINDREPLACE * const pfr)
{
   const size_t selstart = SendMessage(m_hwndScintilla, SCI_GETSELECTIONSTART, 0, 0);
   const size_t selend = SendMessage(m_hwndScintilla, SCI_GETSELECTIONEND, 0, 0);

   const size_t len = SendMessage(m_hwndScintilla, SCI_GETTEXTLENGTH, 0, 0);

   FINDTEXTEX ft;
   ft.chrg.cpMax = (LONG)len;			// search through end of the text
   ft.chrg.cpMin = (LONG)selstart;
   if (!(pfr->Flags & (FR_REPLACE | FR_REPLACEALL)))
      ft.chrg.cpMin = (LONG)selend;
   ft.lpstrText = pfr->lpstrFindWhat;

   LONG cszReplaced = 0;
next:
   const size_t cpMatch = SendMessage(m_hwndScintilla, SCI_FINDTEXT, (WPARAM)(pfr->Flags), (LPARAM)&ft);
   if ((SSIZE_T)cpMatch < 0)
   {
      if (cszReplaced == 0)
      {
         char szT[MAX_PATH];
         LocalString ls(IDS_FINDFAILED);
         LocalString ls2(IDS_FINDFAILED2);
         lstrcpy(szT, ls.m_szbuffer);
         lstrcat(szT, ft.lpstrText);
         lstrcat(szT, ls2.m_szbuffer);
         MessageBeep(MB_ICONEXCLAMATION);
         SendMessage(m_hwndStatus, SB_SETTEXT, 1 | 0, (size_t)szT);
      }
      else
      {
         char szT[MAX_PATH];
         LocalString ls(IDS_REPLACEALL);
         LocalString ls2(IDS_REPLACEALL2);
         wsprintfA(szT, "%s %ld %s", ls.m_szbuffer, cszReplaced, ls2.m_szbuffer);
         MessageBeep(MB_ICONEXCLAMATION);
         SendMessage(m_hwndStatus, SB_SETTEXT, 1 | 0, (size_t)szT);
      }
      return;
   }

   ft.chrg.cpMin = ft.chrgText.cpMin;
   ft.chrg.cpMax = ft.chrgText.cpMax;
   SendMessage(m_hwndScintilla, SCI_SETSEL, ft.chrgText.cpMin, ft.chrgText.cpMax);
   if (((pfr->Flags & FR_REPLACE) && cszReplaced == 0) || (pfr->Flags & FR_REPLACEALL))
   {
      SendMessage(m_hwndScintilla, SCI_REPLACESEL, true, (LPARAM)pfr->lpstrReplaceWith);
      ft.chrg.cpMin = (LONG)(cpMatch + lstrlen(pfr->lpstrReplaceWith));
      ft.chrg.cpMax = (LONG)len;	// search through end of the text
      cszReplaced++;
      goto next;
   }
}

void CodeViewer::SaveToStream(IStream *pistream, const HCRYPTHASH hcrypthash, const HCRYPTKEY hcryptkey)
{
   size_t cchar = SendMessage(m_hwndScintilla, SCI_GETTEXTLENGTH, 0, 0);
   const size_t bufferSize = cchar + 32;
   char * const szText = new char[bufferSize + 1];
   SendMessage(m_hwndScintilla, SCI_GETTEXT, cchar + 1, (size_t)szText);

   // if there is a valid key, then encrypt the script text (now in szText)
   // (must be done before the hash is updated)
   if (hcryptkey != NULL)
   {
      // get the size of the data to encrypt
      DWORD cryptlen = (DWORD)cchar;

      // encrypt the script
      CryptEncrypt(hcryptkey,			// key to use
         0, 				// not hashing data at the same time
         TRUE, 				// last block (or only block)
         0, 				// no flags
         (BYTE *)szText,	// buffer to encrypt
         &cryptlen,			// size of data to encrypt
         (DWORD)bufferSize);		// maximum size of buffer (includes padding)

      /*const int foo =*/ GetLastError();	// purge any errors

      // update the size of the buffer to stream out (and create hash for)
      cchar = cryptlen;
   }

   ULONG writ = 0;
   pistream->Write(&cchar, (ULONG)sizeof(int), &writ);
   pistream->Write(szText, (ULONG)(cchar*sizeof(char)), &writ);

   CryptHashData(hcrypthash, (BYTE *)szText, (DWORD)cchar, 0);

   delete[] szText;
}

void CodeViewer::LoadFromStream(IStream *pistream, const HCRYPTHASH hcrypthash, const HCRYPTKEY hcryptkey)
{
   m_fIgnoreDirty = fTrue;

   ULONG read = 0;
   int cchar;
   pistream->Read(&cchar, sizeof(int), &read);

   BYTE * const szText = new BYTE[cchar + 1];

   pistream->Read(szText, cchar*(int)sizeof(char), &read);

   CryptHashData(hcrypthash, (BYTE *)szText, cchar, 0);

   // if there is a valid key, then decrypt the script text (now in szText)
   //(must be done after the hash is updated)
   if (hcryptkey != NULL)
   {
      // get the size of the data to decrypt
      DWORD cryptlen = cchar*(int)sizeof(char);

      // decrypt the script
      CryptDecrypt(hcryptkey,			// key to use
         0, 				// not hashing data at the same time
         TRUE, 				// last block (or only block)
         0, 				// no flags
         (BYTE *)szText,	// buffer to decrypt
         &cryptlen);		// size of data to decrypt

      /*const int foo =*/ GetLastError();	// purge any errors

      // update the size of the buffer
      cchar = cryptlen;
   }

   // ensure that the script is null terminated (as an encrypted script can leave junk at
   // the end of the buffer)
   szText[cchar] = L'\0';

   // check for bogus control characters
   for (int i = 0; i < cchar; ++i)
   {
      if (szText[i] < 9 || (szText[i] > 10 && szText[i] < 13) || (szText[i] > 13 && szText[i] < 32))
         szText[i] = ' ';
   }
   SendMessage(m_hwndScintilla, SCI_SETTEXT, 0, (size_t)szText);
   SendMessage(m_hwndScintilla, SCI_EMPTYUNDOBUFFER, 0, 0);
   delete[] szText;

   m_fIgnoreDirty = fFalse;
   m_sdsDirty = eSaveClean;
}

void CodeViewer::ColorLine(const int line)
{
   //!!
}

void CodeViewer::UncolorError()
{
   const size_t startChar = SendMessage(m_hwndScintilla, SCI_POSITIONFROMLINE, m_errorLineNumber, 0);
   const size_t length = SendMessage(m_hwndScintilla, SCI_LINELENGTH, m_errorLineNumber, 0);

   SendMessage(m_hwndScintilla, SCI_INDICATORCLEARRANGE, startChar, length);

   m_errorLineNumber = -1;
}

void CodeViewer::ColorError(const int line, const int nchar)
{
   m_errorLineNumber = line - 1;

   const size_t startChar = SendMessage(m_hwndScintilla, SCI_POSITIONFROMLINE, line - 1, 0);
   const size_t length = SendMessage(m_hwndScintilla, SCI_LINELENGTH, line - 1, 0);

   SendMessage(m_hwndScintilla, SCI_INDICATORFILLRANGE, startChar, length);
   SendMessage(m_hwndScintilla, SCI_GOTOLINE, line, 0);
}

STDMETHODIMP CodeViewer::OnEnterScript()
{
   return S_OK;
}

STDMETHODIMP CodeViewer::OnLeaveScript()
{
   return S_OK;
}

void CodeViewer::TellHostToSelectItem()
{
   const size_t index = SendMessage(m_hwndItemList, CB_GETCURSEL, 0, 0);
   IScriptable * const pscript = (IScriptable *)SendMessage(m_hwndItemList, CB_GETITEMDATA, index, 0);

   m_psh->SelectItem(pscript);
}

void CodeViewer::GetParamsFromEvent(const int iEvent, char * const szParams)
{
   szParams[0] = '\0';

   const size_t index = SendMessage(m_hwndItemList, CB_GETCURSEL, 0, 0);
   IScriptable * const pscript = (IScriptable *)SendMessage(m_hwndItemList, CB_GETITEMDATA, index, 0);
   IDispatch * const pdisp = pscript->GetDispatch();
   IProvideClassInfo* pClassInfo;
   pdisp->QueryInterface(IID_IProvideClassInfo, (void **)&pClassInfo);

   if (pClassInfo)
   {
      ITypeInfo *pti;
      pClassInfo->GetClassInfo(&pti);

      TYPEATTR *pta;
      pti->GetTypeAttr(&pta);

      for (int i = 0; i < pta->cImplTypes; ++i)
      {
         HREFTYPE href;
         pti->GetRefTypeOfImplType(i, &href);

         ITypeInfo *ptiChild;
         pti->GetRefTypeInfo(href, &ptiChild);

         TYPEATTR *ptaChild;
         ptiChild->GetTypeAttr(&ptaChild);

         if (ptaChild->wTypeFlags == 4096) // Events
         {
            FUNCDESC *pfd;
            ptiChild->GetFuncDesc(iEvent, &pfd);

            if (pfd->cParams != 0) // no parameters makes it easy
            {
               // Get parameter names
               BSTR * const rgstr = (BSTR *)CoTaskMemAlloc(6 * sizeof(BSTR));

               unsigned int cnames;
               /*const HRESULT hr =*/ ptiChild->GetNames(pfd->memid, rgstr, 6, &cnames);

               // Add enum string to combo control
               char szT[512];
               for (unsigned int l = 1; l < cnames; ++l)
               {
                  WideCharToMultiByte(CP_ACP, 0, rgstr[l], -1, szT, 512, NULL, NULL);
                  if (l > 1)
                  {
                     lstrcat(szParams, ", ");
                  }
                  lstrcat(szParams, szT);
               }

               for (unsigned int l = 0; l < cnames; l++)
                  SysFreeString(rgstr[l]);

               CoTaskMemFree(rgstr);
            }

            ptiChild->ReleaseFuncDesc(pfd);
         }

         ptiChild->ReleaseTypeAttr(ptaChild);

         ptiChild->Release();
      }

      pti->ReleaseTypeAttr(pta);

      pti->Release();
      pClassInfo->Release();
   }
}

void CodeViewer::ListEventsFromItem()
{
   // Clear old events
   SendMessage(m_hwndEventList, CB_RESETCONTENT, 0, 0);

   const size_t index = SendMessage(m_hwndItemList, CB_GETCURSEL, 0, 0);
   IScriptable * const pscript = (IScriptable *)SendMessage(m_hwndItemList, CB_GETITEMDATA, index, 0);
   IDispatch * const pdisp = pscript->GetDispatch();

   EnumEventsFromDispatch(pdisp, AddEventToList, (LPARAM)m_hwndEventList);
}

void CodeViewer::FindCodeFromEvent()
{
   bool fFound = false;

   char szItemName[512]; // Can't be longer than 32 chars, but use this buffer for concatenating
   char szEventName[512];
   size_t index = SendMessage(m_hwndItemList, CB_GETCURSEL, 0, 0);
   SendMessage(m_hwndItemList, CB_GETLBTEXT, index, (size_t)szItemName);
   index = SendMessage(m_hwndEventList, CB_GETCURSEL, 0, 0);
   SendMessage(m_hwndEventList, CB_GETLBTEXT, index, (size_t)szEventName);
   const size_t iEventIndex = SendMessage(m_hwndEventList, CB_GETITEMDATA, index, 0);
   lstrcat(szItemName, "_"); // VB Specific event names
   lstrcat(szItemName, szEventName);
   size_t codelen = SendMessage(m_hwndScintilla, SCI_GETTEXTLENGTH, 0, 0);
   size_t startChar = 0;
   size_t stopChar = codelen;
   SendMessage(m_hwndScintilla, SCI_TARGETWHOLEDOCUMENT, 0, 0);
	SendMessage(m_hwndScintilla, SCI_SETSEARCHFLAGS, SCFIND_WHOLEWORD, 0);
	size_t posFind;
   while ((posFind = SendMessage(m_hwndScintilla, SCI_SEARCHINTARGET, lstrlen(szItemName), (LPARAM)szItemName)) != -1)
   {
      const size_t line = SendMessage(m_hwndScintilla, SCI_LINEFROMPOSITION, posFind, 0);
      // Check for 'sub' and make sure we're not in a comment
      const size_t beginchar = SendMessage(m_hwndScintilla, SCI_POSITIONFROMLINE, line, 0);
      bool fGoodMatch = true;

      char szLine[MAX_LINE_LENGTH];
      SOURCE_TEXT_ATTR wzFormat[MAX_LINE_LENGTH];
      WCHAR wzText[MAX_LINE_LENGTH];

      const size_t cchar = SendMessage(m_hwndScintilla, SCI_GETLINE, line, (size_t)szLine);
      MultiByteToWideChar(CP_ACP, 0, szLine, -1, wzText, (int)cchar);
      m_pScriptDebug->GetScriptTextAttributes(wzText, (ULONG)cchar, NULL, 0, wzFormat);

      const size_t inamechar = posFind - beginchar - 1;

      int i;
      for (i = (int)inamechar; i >= 0; i--)
      {
         if (wzFormat[i] == SOURCETEXT_ATTR_KEYWORD)
            break;

         if (!FIsWhitespace(szLine[i]) /*&& (wzFormat[i] != 0 || wzFormat[i] != SOURCETEXT_ATTR_COMMENT)*/) //!!?
            fGoodMatch = false;
      }

      if (i < 2) // Can't fit the word 'sub' in here
      {
         fGoodMatch = false;
      }
      else
      {
         szLine[i + 1] = '\0';
         if (lstrcmpi(&szLine[i - 2], "sub")) //!! correct like this?
            fGoodMatch = false;
      }

      if (fGoodMatch)
      {
         // We found a real sub heading - move the cursor inside of it

         fFound = true;

         size_t ichar = SendMessage(m_hwndScintilla, SCI_POSITIONFROMLINE, line + 1, 0);
         if (ichar == -1)
         {
            // The function was declared as the last line of the script - rare but possible
            ichar = SendMessage(m_hwndScintilla, SCI_POSITIONFROMLINE, line, 0);
         }

         const size_t lineEvent = SendMessage(m_hwndScintilla, SCI_LINEFROMPOSITION, ichar, 0);
         SendMessage(m_hwndScintilla, SCI_ENSUREVISIBLEENFORCEPOLICY, lineEvent, 0);
         SendMessage(m_hwndScintilla, SCI_SETSEL, ichar, ichar);
      }

      if (fFound)
         break;

      startChar = posFind + 1;
      SendMessage(m_hwndScintilla, SCI_SETTARGETSTART, startChar, 0);
      SendMessage(m_hwndScintilla, SCI_SETTARGETEND, stopChar, 0);
   }

   if (!fFound)
   {
      char szNewCode[MAX_LINE_LENGTH];
      char szEnd[2];

      TEXTRANGE tr;
      tr.chrg.cpMax = (LONG)codelen;
      tr.chrg.cpMin = (LONG)codelen - 1;
      tr.lpstrText = szEnd;

      // Make sure there is at least a one space gap between the last function and this new one
      SendMessage(m_hwndScintilla, SCI_GETTEXT, 0, (size_t)&tr);

      if (szEnd[0] != '\n')
      {
         SendMessage(m_hwndScintilla, SCI_SETSEL, codelen, codelen);
         SendMessage(m_hwndScintilla, SCI_REPLACESEL, TRUE, (size_t)"\n");
         codelen++;
      }

      if (szEnd[1] != '\n')
      {
         SendMessage(m_hwndScintilla, SCI_SETSEL, codelen, codelen);
         SendMessage(m_hwndScintilla, SCI_REPLACESEL, TRUE, (size_t)"\n");
         codelen++;
      }

      // Add the new function at the end
      SendMessage(m_hwndScintilla, SCI_SETSEL, codelen, codelen);

      char szParams[MAX_LINE_LENGTH];

      GetParamsFromEvent(iEventIndex, szParams);

      lstrcpy(szNewCode, "Sub ");
      lstrcat(szNewCode, szItemName);
      lstrcat(szNewCode, "(");
      lstrcat(szNewCode, szParams);
      lstrcat(szNewCode, ")\n\t");
      const size_t subtitlelen = lstrlen(szNewCode);
      lstrcat(szNewCode, "\nEnd Sub");

      SendMessage(m_hwndScintilla, SCI_REPLACESEL, TRUE, (size_t)szNewCode);

      SendMessage(m_hwndScintilla, SCI_SETSEL, codelen + subtitlelen, codelen + subtitlelen);
   }

   SetFocus(m_hwndScintilla);
}

HRESULT STDMETHODCALLTYPE CodeViewer::GetSecurityId(
   BYTE *pbSecurityId,
   DWORD *pcbSecurityId,
   DWORD_PTR dwReserved)
{
   return S_OK;
}

HRESULT STDMETHODCALLTYPE CodeViewer::ProcessUrlAction(
   DWORD dwAction,
   BYTE __RPC_FAR *pPolicy,
   DWORD cbPolicy,
   BYTE __RPC_FAR *pContext,
   DWORD cbContext,
   DWORD dwFlags,
   DWORD dwReserved)
{

   *pPolicy = (dwAction == URLACTION_ACTIVEX_RUN && (g_pvp->m_securitylevel < eSecurityNoControls)) ?
   URLPOLICY_ALLOW : URLPOLICY_DISALLOW;

   return S_OK;
}

DEFINE_GUID(GUID_CUSTOM_CONFIRMOBJECTSAFETY, 0x10200490, 0xfa38, 0x11d0, 0xac, 0x0e, 0x00, 0xa0, 0xc9, 0x0f, 0xff, 0xc0);

HRESULT STDMETHODCALLTYPE CodeViewer::QueryCustomPolicy(
   REFGUID guidKey,
   BYTE __RPC_FAR *__RPC_FAR *ppPolicy,
   DWORD __RPC_FAR *pcbPolicy,
   BYTE __RPC_FAR *pContext,
   DWORD cbContext,
   DWORD dwReserved)
{
   DWORD * const ppolicy = (DWORD *)CoTaskMemAlloc(sizeof(DWORD));
   *ppolicy = URLPOLICY_DISALLOW;

   *ppPolicy = (BYTE *)ppolicy;

   *pcbPolicy = sizeof(DWORD);

   if (InlineIsEqualGUID(guidKey, GUID_CUSTOM_CONFIRMOBJECTSAFETY))
   {
      bool fSafe = false;
      CONFIRMSAFETY *pcs = (CONFIRMSAFETY *)pContext;

      if (g_pvp->m_securitylevel == eSecurityNone)
         fSafe = true;

      if (!fSafe && ((g_pvp->m_securitylevel == eSecurityWarnOnUnsafeType) || (g_pvp->m_securitylevel == eSecurityWarnOnType)))
         fSafe = (FControlAlreadyOkayed(pcs) != 0);

      if (!fSafe && (g_pvp->m_securitylevel <= eSecurityWarnOnUnsafeType))
         fSafe = (FControlMarkedSafe(pcs) != 0);

      if (!fSafe)
      {
         fSafe = (FUserManuallyOkaysControl(pcs) != 0);
         if (fSafe && ((g_pvp->m_securitylevel == eSecurityWarnOnUnsafeType) || (g_pvp->m_securitylevel == eSecurityWarnOnType)))
            AddControlToOkayedList(pcs);
      }

      if (fSafe)
         *ppolicy = URLPOLICY_ALLOW;
   }

   return S_OK;
}

BOOL CodeViewer::FControlAlreadyOkayed(CONFIRMSAFETY *pcs)
{
   if (g_pplayer)
   {
      for (int i = 0; i < g_pplayer->m_controlclsidsafe.Size(); ++i)
      {
         const CLSID * const pclsid = g_pplayer->m_controlclsidsafe.ElementAt(i);
         if (*pclsid == pcs->clsid)
            return fTrue;
      }
   }

   return fFalse;
}

void CodeViewer::AddControlToOkayedList(CONFIRMSAFETY *pcs)
{
   if (g_pplayer)
   {
      CLSID * const pclsid = new CLSID();
      *pclsid = pcs->clsid;
      g_pplayer->m_controlclsidsafe.AddElement(pclsid);
   }
}

BOOL CodeViewer::FControlMarkedSafe(CONFIRMSAFETY *pcs)
{
   BOOL fSafe = fFalse;
   IObjectSafety *pios = NULL;

   if (FAILED(pcs->pUnk->QueryInterface(IID_IObjectSafety, (void **)&pios)))
      goto LError;

   DWORD supported, enabled;

   if (FAILED(pios->GetInterfaceSafetyOptions(IID_IDispatch, &supported, &enabled)))
      goto LError;

   if (!(supported & INTERFACESAFE_FOR_UNTRUSTED_CALLER) || !(supported & INTERFACESAFE_FOR_UNTRUSTED_DATA))
      goto LError;

   if (!(enabled & INTERFACESAFE_FOR_UNTRUSTED_CALLER) || !(enabled & INTERFACESAFE_FOR_UNTRUSTED_DATA))
   {
      if (FAILED(pios->SetInterfaceSafetyOptions(IID_IDispatch, supported, INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA)))
         goto LError;
   }

   fSafe = fTrue;

LError:

   if (pios)
      pios->Release();

   return fSafe;
}

BOOL CodeViewer::FUserManuallyOkaysControl(CONFIRMSAFETY *pcs)
{
   OLECHAR *wzT;
   if (FAILED(OleRegGetUserType(pcs->clsid, USERCLASSTYPE_FULL, &wzT)))
      return fFalse;

   const int len = lstrlenW(wzT) + 1; // include null termination

   char * const szName = new char[len];

   WideCharToMultiByte(CP_ACP, 0, wzT, len, szName, len, NULL, NULL);

   LocalString ls1(IDS_UNSECURECONTROL1);
   LocalString ls2(IDS_UNSECURECONTROL2);

   char * const szT = new char[lstrlen(ls1.m_szbuffer) + lstrlen(szName) + lstrlen(ls2.m_szbuffer) + 1];
   lstrcpy(szT, ls1.m_szbuffer);
   lstrcat(szT, szName);
   lstrcat(szT, ls2.m_szbuffer);

   const int ans = MessageBox(m_hwndMain, szT, "Visual Pinball", MB_YESNO | MB_DEFBUTTON2);

   BOOL fSafe = fFalse;
   if (ans == IDYES)
      fSafe = fTrue;

   delete[] szName;
   delete[] szT;

   return fSafe;
}

HRESULT STDMETHODCALLTYPE CodeViewer::QueryService(
   REFGUID guidService,
   REFIID riid,
   void **ppv)
{
   const HRESULT hr = (riid == IID_IInternetHostSecurityManager) ? QueryInterface(riid /*IID_IInternetHostSecurityManager*/, ppv) : E_NOINTERFACE;

   return hr;
}

void CodeViewer::ShowAutoComplete(SCNotification *pSCN)
{
	if(!pSCN) return;
	char KeyPressed = pSCN->ch;
	if (KeyPressed != '.')
	{
		WordUnderCaret.lpstrText = CaretTextBuff;
		GetWordUnderCaret();
		const size_t intWordLen = strlen(WordUnderCaret.lpstrText);
		if ((int)intWordLen > DisplayAutoCompleteLength && intWordLen < MAX_FIND_LENGTH)
		{
			const char * McStr = AutoCompString.c_str();
			SendMessage(m_hwndScintilla, SCI_AUTOCSHOW, intWordLen, (LPARAM)McStr);
		}
	}
	else
	{
		//Get member construct

		LRESULT ConstructPos = SendMessage(m_hwndScintilla, SCI_GETCURRENTPOS, 0, 0 ) - 2;
		CurrentConstruct.chrg.cpMin = SendMessage(m_hwndScintilla, SCI_WORDSTARTPOSITION, ConstructPos, TRUE);
		CurrentConstruct.chrg.cpMax = SendMessage(m_hwndScintilla, SCI_WORDENDPOSITION, ConstructPos, TRUE);
		if (( CurrentConstruct.chrg.cpMax - CurrentConstruct.chrg.cpMin) > MAX_FIND_LENGTH) return;
		SendMessage(m_hwndScintilla, SCI_GETTEXTRANGE, 0, (LPARAM)&CurrentConstruct);
		//Check Core dict first
		CurrentConstruct.lpstrText = (char *) ConstructTextBuff;
		GetMembers(VP_CoreDict, string(CurrentConstruct.lpstrText) );
		//Check Table Script
		if (CurrentMembers->size() == 0)
		{
			CurrentConstruct.lpstrText = (char *) ConstructTextBuff;
			GetMembers(PageConstructsDict, string(CurrentConstruct.lpstrText) );	
		}
		//if no contruct (no children) exit
		if (CurrentMembers->size() == 0) return;
		//autocomp string  = members of construct
		AutoCompMembersString = "";
		for (vector<UserData>::iterator i = CurrentMembers->begin(); i != CurrentMembers->end(); ++i)
		{
			AutoCompMembersString.append(i->KeyName);
			AutoCompMembersString.append(" ");
		}
		//display
		const char * McStr = AutoCompMembersString.c_str();
		SendMessage(m_hwndScintilla, SCI_AUTOCSHOW, 0, (LPARAM)McStr);
	}
}

void CodeViewer::GetMembers(vector<UserData>* ListIn, const string &strIn)
{
	CurrentMembers->clear();
	const int idx = UDIndex(ListIn , strIn);
	if (idx != -1)
	{
		const UserData udParent = ListIn->at(idx);
		const size_t NumberOfMembers = udParent.Children.size();
		size_t i = 0;
		while (i < NumberOfMembers)
		{
			UserData UD = GetUDfromUniqueKey(ListIn, udParent.Children.at(i) );
			FindOrInsertUD(CurrentMembers, UD);
			++i;
		}
	}
}

bool CodeViewer::ShowTooltip(SCNotification *pSCN)
{
	//get word under pointer
	const int dwellpos = pSCN->position;
	LRESULT wordstart = SendMessage(m_hwndScintilla, SCI_WORDSTARTPOSITION, dwellpos, TRUE );
	LRESULT wordfinish = SendMessage(m_hwndScintilla, SCI_WORDENDPOSITION, dwellpos, TRUE );
	char Mess[MAX_LINE_LENGTH*4] = {}; int MessLen = 0;
	char szDwellWord[256] = {};
	char szLCDwellWord[256] = {};
	const int CurrentLineNo = (int)SendMessage(m_hwndScintilla, SCI_LINEFROMPOSITION, dwellpos, 0);
	//return if in a comment
	char text[MAX_LINE_LENGTH] = {0};
	SendMessage(m_hwndScintilla, SCI_GETLINE, CurrentLineNo, (LPARAM)text);
	if(text[0] != '\0')
	{
		const string strText = string(text);
		const size_t t = strText.find_first_of('\'', 0 );
		if (t != string::npos)
		{
			const LRESULT linestart = SendMessage(m_hwndScintilla, SCI_POSITIONFROMLINE, CurrentLineNo, 0 );
			if ( ((size_t)(wordstart - linestart)) >= t ) return false;
		}
	}

	// is it a valid 'word'
	if ((SendMessage(m_hwndScintilla, SCI_ISRANGEWORD, wordstart , wordfinish )) && ((wordfinish - wordstart) < 255))
	{
		//Retrieve the word
		GetRange(m_hwndScintilla, wordstart, wordfinish, szDwellWord);
		strcpy_s( szLCDwellWord, szDwellWord);
		szLower(szLCDwellWord);
		string DwellWord = szLCDwellWord;
		RemovePadding(DwellWord);
		RemoveNonVBSChars(DwellWord);
		if (DwellWord.size() == 0) return false;
		// Serarch for VBS reserved words
		// ToDo: Should be able get some MS help for better descriptions
		vector<UserData>::iterator i;
		int idx = 0;
		if (FindUD(VBwordsDict, DwellWord, i, idx) == 0)
		{
			string ptemp = i->KeyName;
			MessLen = sprintf_s(Mess, "VBS:%s",  ptemp.c_str() );
		}
		else if (MessLen == 0)
		{
			//Has function list been filled?
			StopErrorDisplay = true;
			if ( PageConstructsDict->size() == 0 ) ParseForFunction();

			//search subs/funcs
			if (FindUD(PageConstructsDict, DwellWord, i, idx) == 0)
			{
				idx = FindClosestUD(PageConstructsDict, CurrentLineNo, idx);
				const UserData* const Word = &PageConstructsDict->at(idx);
				string ptemp = Word->Description;
				ptemp += " (Line:" + to_string((long long)Word->LineNum + 1) + ")";
				if ( (Word->Comment.length() > 1) && DwellHelp )
				{
					ptemp += "\n" +  PageConstructsDict->at(idx).Comment;
				}
				MessLen = sprintf_s(Mess, "%s", ptemp.c_str() );
			}

			//Search VP core
			else if (FindUD(VP_CoreDict, DwellWord, i, idx) == 0)
			{
				idx = FindClosestUD(VP_CoreDict, CurrentLineNo, idx) ; 
				string ptemp = VP_CoreDict->at(idx).Description;
				if ( (VP_CoreDict->at(idx).Comment.length() >1) && DwellHelp )
				{
					ptemp += "\n" +  VP_CoreDict->at(idx).Comment;
				}
				MessLen = sprintf_s(Mess, "%s", ptemp.c_str() );
			}
			else if ( ( FindUD(ComponentsDict, DwellWord, i, idx)  == 0 ) )
			{
				MessLen = sprintf_s(Mess, "Component: %s", szDwellWord);
			}
		}
#ifdef _DEBUG
		if (MessLen == 0 )
		{
			MessLen = sprintf_s(Mess, "Test:%s", szDwellWord);
		}
#endif
	}
	if (MessLen > 0)
	{
		SendMessage(m_hwndScintilla,SCI_CALLTIPSHOW,dwellpos, (LPARAM)Mess );
		return true;
	}
	return false;
}

void CodeViewer::MarginClick(const int position, const int modifiers)
{
   const size_t lineClick = SendMessage(m_hwndScintilla, SCI_LINEFROMPOSITION, position, 0);
   if ((modifiers & SCMOD_SHIFT) && (modifiers & SCMOD_CTRL))
   {
      //FoldAll();
   }
   else
   {
      const size_t levelClick = SendMessage(m_hwndScintilla, SCI_GETFOLDLEVEL, lineClick, 0);
      if (levelClick & SC_FOLDLEVELHEADERFLAG)
      {
         if (modifiers & SCMOD_SHIFT)
         {
            // Ensure all children visible
            SendMessage(m_hwndScintilla, SCI_SETFOLDEXPANDED, lineClick, 1);
            //Expand(lineClick, true, true, 100, levelClick);
         }
         else if (modifiers & SCMOD_CTRL)
         {
            if (SendMessage(m_hwndScintilla, SCI_GETFOLDEXPANDED, lineClick, 0))
            {
               // Contract this line and all children
               SendMessage(m_hwndScintilla, SCI_SETFOLDEXPANDED, lineClick, 0);
               //Expand(lineClick, false, true, 0, levelClick);
            }
            else
            {
               // Expand this line and all children
               SendMessage(m_hwndScintilla, SCI_SETFOLDEXPANDED, lineClick, 1);
               //Expand(lineClick, true, true, 100, levelClick);
            }
         }
         else
         {
            // Toggle this line
            SendMessage(m_hwndScintilla, SCI_TOGGLEFOLD, lineClick, 0);
         }
      }
   }
}

void AddComment(const HWND m_hwndScintilla)
{
   char *comment = "'";

   size_t startSel = SendMessage(m_hwndScintilla, SCI_GETSELECTIONSTART, 0, 0);
   size_t endSel = SendMessage(m_hwndScintilla, SCI_GETSELECTIONEND, 0, 0);
   size_t pos = SendMessage(m_hwndScintilla, SCI_GETCURRENTPOS, 0, 0);

   size_t selStartLine = SendMessage(m_hwndScintilla, SCI_LINEFROMPOSITION, startSel, 0);
   size_t selEndLine = SendMessage(m_hwndScintilla, SCI_LINEFROMPOSITION, endSel, 0);
   size_t lines = selEndLine - selStartLine + 1;
   size_t posFromLine = SendMessage(m_hwndScintilla, SCI_POSITIONFROMLINE, selEndLine, 0);

   if (lines > 1 && endSel == posFromLine)
   {
      selEndLine--;
      lines--;
      endSel = SendMessage(m_hwndScintilla, SCI_GETLINEENDPOSITION, selEndLine, 0);
   }
   SendMessage(m_hwndScintilla, SCI_BEGINUNDOACTION, 0, 0);
   size_t lineStart = SendMessage(m_hwndScintilla, SCI_POSITIONFROMLINE, selStartLine, 0);
   if (lines <= 1)
   {
      // Only a single line was selected, so just append whitespace + end-comment at end of line if needed
      size_t lineEnd = SendMessage(m_hwndScintilla, SCI_GETLINEENDPOSITION, selEndLine, 0);
      SendMessage(m_hwndScintilla, SCI_INSERTTEXT, lineStart, (LPARAM)comment);
   }
   else
   {
      // More than one line selected, so insert middle_comments where needed
      for (size_t i = selStartLine; i < selEndLine + 1; ++i)
      {
         lineStart = SendMessage(m_hwndScintilla, SCI_POSITIONFROMLINE, i, 0);
         SendMessage(m_hwndScintilla, SCI_INSERTTEXT, lineStart, (LPARAM)comment);
      }
   }
   SendMessage(m_hwndScintilla, SCI_ENDUNDOACTION, 0, 0);
}

void RemoveComment(const HWND m_hwndScintilla)
{
   char *comment = "\b";
   size_t startSel = SendMessage(m_hwndScintilla, SCI_GETSELECTIONSTART, 0, 0);
   size_t endSel = SendMessage(m_hwndScintilla, SCI_GETSELECTIONEND, 0, 0);
   size_t pos = SendMessage(m_hwndScintilla, SCI_GETCURRENTPOS, 0, 0);

   size_t selStartLine = SendMessage(m_hwndScintilla, SCI_LINEFROMPOSITION, startSel, 0);
   size_t selEndLine = SendMessage(m_hwndScintilla, SCI_LINEFROMPOSITION, endSel, 0);
   size_t lines = selEndLine - selStartLine + 1;
   size_t posFromLine = SendMessage(m_hwndScintilla, SCI_POSITIONFROMLINE, selEndLine, 0);

   if (lines > 1 && endSel == posFromLine)
   {
      selEndLine--;
      lines--;
      endSel = SendMessage(m_hwndScintilla, SCI_GETLINEENDPOSITION, selEndLine, 0);
   }

   SendMessage(m_hwndScintilla, SCI_BEGINUNDOACTION, 0, 0);

   for (size_t i = selStartLine; i < selEndLine + 1; ++i)
   {
      size_t lineStart = SendMessage(m_hwndScintilla, SCI_POSITIONFROMLINE, i, 0);
      size_t lineEnd = SendMessage(m_hwndScintilla, SCI_GETLINEENDPOSITION, i, 0);
      char buf[MAX_LINE_LENGTH];
      if (lineEnd - lineStart < (MAX_LINE_LENGTH -1) )
      {
         GetRange(m_hwndScintilla, lineStart, lineEnd, buf);
         string line(buf);
         size_t idx = line.find_first_of("'");
         if (idx == 0)
         {
            SendMessage(m_hwndScintilla, SCI_SETSEL, lineStart, lineStart + 1);
            SendMessage(m_hwndScintilla, SCI_REPLACESEL, 0, (LPARAM)"");
         }
      }
   }
   SendMessage(m_hwndScintilla, SCI_ENDUNDOACTION, 0, 0);
}

string CodeViewer::upperCase(string input)
{
   for (string::iterator it = input.begin(); it != input.end(); ++it)
      *it = toupper(*it);
   return input;
}

string CodeViewer::lowerCase(string input)
{
   for (string::iterator it = input.begin(); it != input.end(); ++it)
      *it = tolower(*it);
   return input;
}

void CodeViewer::szLower(char * const incstr)
{
	char *pC = incstr;
	while (*pC)
	{
		if (*pC >= 'A' && *pC <= 'Z')
			*pC = *pC + ('a' - 'A');
		pC++;
	}
}

void CodeViewer::szUpper(char * const incstr)
{
	char *pC = incstr;
	while (*pC)
	{
		if (*pC >= 'a' && *pC <= 'z')
			*pC = *pC - ('a' - 'A' );
		pC++;
	}
}

// Makes sure what is found has only VBS Chars in..
int CodeViewer::SureFind(const string &LineIn, const string &ToFind)
{
	const size_t Pos = LineIn.find(ToFind);
	if (Pos == -1) return -1;
	const char EndChr = LineIn[Pos + ToFind.length() ];
	int IsValidVBChr = VBValidChars.find( EndChr );
	if (IsValidVBChr >= 0 )
	{// Extra char on end - not what we want
		return -1;
	}

	if (Pos > 0)
	{
		const char StartChr = LineIn[Pos -1];
		IsValidVBChr = VBValidChars.find( StartChr );
		if (IsValidVBChr >= 0 )
		{
			return -1;
		}
	}
	return Pos;
}

string CodeViewer::ParseRemoveLineComments(string *Line)
{
	const int commentIdx = Line->find("'");
	if (commentIdx == -1) return "";
	string RetVal = Line->substr(commentIdx+1, string::npos);
	RemovePadding(RetVal);
	if (commentIdx > 0)
	{
		*Line = string(Line->substr(0, commentIdx));
		return RetVal;
	}
	Line->clear();
	return RetVal;
}

bool CodeViewer::ParseOKLineLength(const int LineLen)
{
	if (LineLen > MAX_LINE_LENGTH)
	{
		char szText[256] = {};
		sprintf_s(szText, "The current maximum script line length is %d", MAX_LINE_LENGTH);
		char szCaption[256] = {};
		sprintf_s(szCaption, "Line too long on line %d", LineLen);
		MessageBox(m_hwndMain, szText,szCaption, MB_OK);
		return false;
	}
	if (LineLen <3) return false;
	return true;
}


//false is a fail/syntax error
bool CodeViewer::ParseStructureName(vector<UserData> *ListIn, UserData ud,
												const string &UCline, const string &line, const int Lineno)
{
	static int ParentLevel = 0;
	static string CurrentParentKey = "";
	const int endIdx = SureFind(UCline,"END"); 
	const int exitIdx = SureFind(UCline,"EXIT"); 
	RemoveNonVBSChars( ud.KeyName );

	if (endIdx == -1 && exitIdx == -1) 
	{
		if (ud.eTyping == eDim || ud.eTyping == eConst)
		{
			ud.UniqueKey = lowerCase( ud.KeyName ) + CurrentParentKey + "\0";
			ud.UniqueParent = CurrentParentKey;
			FindOrInsertUD(ListIn, ud);
			size_t iCurParent = GetUDPointerfromUniqueKey(ListIn, CurrentParentKey);
			if (CurrentParentKey.size() !=0 && ud.UniqueKey.size() != 0 && iCurParent<ListIn->size())
			{
				ListIn->at(iCurParent).Children.push_back(ud.UniqueKey);//add child to parent
			}
			string RemainingLine = line;
			int CommPos = UCline.find_first_of(',');
			while (CommPos != -1)
			{
				//Insert stuff after comma after cleaning up
				int NewCommPos = RemainingLine.find_first_of(',', CommPos+1);
				//get word @
				string crWord;
				crWord = RemainingLine.substr(CommPos+1, (NewCommPos == -1) ? string::npos : (NewCommPos - CommPos)-1 );
				RemoveByVal(crWord);
				RemovePadding(crWord);
				RemoveNonVBSChars(crWord);
				if (crWord.size() <= MAX_FIND_LENGTH && crWord.size() > 0) 
				{
					ud.KeyName = crWord;
					ud.UniqueKey = lowerCase(ud.KeyName) + CurrentParentKey + "\0";
					ud.UniqueParent = CurrentParentKey;
					FindOrInsertUD(ListIn, ud);
					if (CurrentParentKey.size() != 0 && iCurParent<ListIn->size())
					{
						ListIn->at(iCurParent).Children.push_back(ud.UniqueKey);//add child to parent
					}	
				}
				RemainingLine = RemainingLine.substr(CommPos+1, string::npos);
				CommPos = RemainingLine.find_first_of(',');
			}
			return false;
		}
		//Its something new and structrual and therefore we are now a parent
		if (ParentLevel == 0)// its a root
		{
			ud.UniqueKey = lowerCase(ud.KeyName) + "\0";
			ud.UniqueParent = "";
			size_t iCurParent = FindOrInsertUD( ListIn, ud);
			//if (iCurParent == -1)
			//{
			//	ShowError("Parent == -1");
			//}
			CurrentParentKey = ud.UniqueKey;
			++ParentLevel;
			// get construct autodims
			string RemainingLine = line;
			int CommPos = UCline.find_first_of('(');
			while (CommPos != -1)
			{
				//Insert stuff after comma after cleaning up
				int NewCommPos = RemainingLine.find_first_of(',', CommPos+1);
				//get word @
				string crWord;
				crWord = RemainingLine.substr(CommPos+1, (NewCommPos == -1) ? string::npos : (NewCommPos - CommPos)-1 );
				RemoveByVal(crWord);
				RemovePadding(crWord);
				RemoveNonVBSChars(crWord);
				if (crWord.size() <= MAX_FIND_LENGTH && crWord.size() > 0)
				{
					ud.KeyName = crWord;
					ud.eTyping = eDim;
					ud.UniqueKey = lowerCase(ud.KeyName) + CurrentParentKey + "\0";
					ud.UniqueParent = CurrentParentKey;
					FindOrInsertUD(ListIn, ud);
					if (CurrentParentKey.size() != 0)
					{
						ListIn->at(iCurParent).Children.push_back(ud.UniqueKey);//add child to parent
					}	
				}
				RemainingLine = RemainingLine.substr(CommPos+1, string::npos);
				CommPos = RemainingLine.find_first_of(',');
			}

		}
		else 
		{
			ud.UniqueParent = CurrentParentKey;
			ud.UniqueKey = lowerCase(ud.KeyName) + CurrentParentKey + "\0";
			FindOrInsertUD( ListIn, ud);
			int iUDIndx = UDKeyIndex( ListIn, CurrentParentKey);
			if (iUDIndx == -1)
			{
				ParentTreeInvalid = true;
				ParentLevel = 0;
				if (!StopErrorDisplay)
				{
					StopErrorDisplay = true;
					char szCaption[256] = {};
					sprintf_s(szCaption,"Parse error on line:%d", Lineno);
					MessageBox(m_hwndMain, "Construct not closed", szCaption, MB_OK);
				}
				return true;
			}
			UserData *iCurParent = &(ListIn->at(iUDIndx));
			iCurParent->Children.push_back(ud.UniqueKey);//add child to parent
			CurrentParentKey = ud.UniqueKey;
			++ParentLevel;
		}
	}
	else
	{
		if (endIdx >= 0)
		{
			if (ParentLevel == -1)
			{
				ParentTreeInvalid = true;
				ParentLevel = 0;
				if (!StopErrorDisplay)
				{
					StopErrorDisplay = true;
					char szCaption[256] = {};
					sprintf_s(szCaption,"Parse error on line:%d", Lineno);
					MessageBox(m_hwndMain, "Construct not opened", szCaption, MB_OK);
				}

				return true;
			}
			else
			{ 
				if (ParentLevel > 0)
				{//finished with child ===== END =====
					int iCurParent = UDKeyIndex(ListIn, CurrentParentKey);
					if (iCurParent != -1 )
					{
						UserData *CurrentParentUD = (&ListIn->at(iCurParent));
						int iGrandParent = UDKeyIndex(ListIn, CurrentParentUD->UniqueParent);
						if (iGrandParent != -1 )
						{
							UserData *CurrentGrandParentUD = (&ListIn->at(iGrandParent));
							CurrentParentKey = CurrentGrandParentUD->UniqueKey; 
						}
						else
						{
							CurrentParentKey.clear();
						}
						ParentLevel--;
						return false;
					}
					/// error - end without start
					CurrentParentKey.clear();
					ParentLevel--;
					return true;
				}
				else
				{	//Error - end without start
					ParentLevel = 0;
					return true;
				}
			}//if (ParentLevel == -1)
		}//if (endIdx >= 0)
	}
	return false;
}

void CodeViewer::ParseDelimtByColon(string *result, string *wholeline)
{
	*result = *wholeline;
	const int idx = result->find(":"); 
	if (idx == -1)
	{
		wholeline->clear();
		return;
	}
	if (idx > 0)
	{
		*result = wholeline->substr(0,idx);
		*wholeline = wholeline->substr(idx + 1, (wholeline->length()) - (result->length()) ) ;
		return;
	}
	wholeline->clear();
}

void CodeViewer::ParseFindConstruct(int &Pos, const string *UCLineIn, WordType &Type,int &ConstructSize)
{
	if ( (Pos = SureFind( *UCLineIn ,"DIM")) != -1 )
	{
		ConstructSize =3;
		Type = eDim;
		return;
	}
	if ( (Pos = SureFind( *UCLineIn ,"CONST")) != -1 )
	{
		ConstructSize =5;
		Type = eConst;
		return;
	}
	if ( (Pos = SureFind( *UCLineIn ,"SUB")) != -1 )
	{
		ConstructSize =3;
		Type = eSub;
		return;
	}
	if ( (Pos = SureFind( *UCLineIn ,"FUNCTION")) != -1)
	{
		ConstructSize = 8;
		Type = eFunction;
		return;
	}
	if ( (Pos = SureFind( *UCLineIn ,"CLASS")) != -1)
	{
		ConstructSize = 5;
		Type = eClass;
		return;
	}
	if ( (Pos = SureFind( *UCLineIn ,"PROPERTY")) != -1)
	{
		int GetLetPos;
		if ( (GetLetPos = SureFind( *UCLineIn ,"GET")) != -1)
		{
			if (Pos < GetLetPos)
			{
				Pos = GetLetPos;
				ConstructSize = 3;
				Type = ePropGet;
				return;
			}
		}
		if ( (GetLetPos = SureFind( *UCLineIn ,"LET")) != -1)
		{
			if (Pos < GetLetPos)
			{
				Pos = GetLetPos;
				ConstructSize = 3;
				Type = ePropLet;
				return;
			}
		}
		if ( (GetLetPos = SureFind( *UCLineIn ,"SET")) != -1)
		{
			if (Pos < GetLetPos)
			{
				Pos = GetLetPos;
				ConstructSize = 3;
				Type = ePropSet;
				return;
			}
		}
		ConstructSize = 8;
		return;
	}
	Pos = -1;
	return;
}

void CodeViewer::ReadLineToParseBrain(string wholeline, int linecount, vector<UserData> *ListIn)
{
		string CommentTmp = ParseRemoveLineComments(&wholeline);
		RemovePadding(CommentTmp);
		while (wholeline.length() > 1)
		{
			string line;
			ParseDelimtByColon(&line, &wholeline);
			RemovePadding(line);
			string UCline = upperCase(line);
			UserData UD;
			UD.eTyping = eUnknown;
			UD.LineNum = linecount;
			UD.Comment = CommentTmp;
			int SearchLength = 0 ;
			int idx = -1;
			ParseFindConstruct(idx, &UCline, UD.eTyping, SearchLength);
			if (idx == -1) continue;
			if ( idx >= 0) // Found something something structrual
			{
				const int doubleQuoteIdx = line.find("\"");
				if ((doubleQuoteIdx >= 0)  && (doubleQuoteIdx < idx)) continue; // in a string literal
				const string sSubName = ExtractWordOperand(line, (idx + SearchLength) );
				UD.Description = line;
				UD.KeyName = sSubName;
				//UserData ud(linecount, line, sSubName, Type);
				if (!ParseStructureName( ListIn, UD, UCline, line, linecount))
				{/*A critical brain error occured */}
			}// if ( idx >= 0)
		}// while (wholeline.length > 1) 
}

void CodeViewer::RemoveByVal(string &line)
{
	const int LL = line.length();
	const string SearchLine = lowerCase(line);
	int Pos = SureFind( SearchLine, "byval");
	if (Pos > -1)
	{
		Pos += 5;
		if ( (LL-Pos) < 0 ) return;
		line = line.substr(Pos, (LL-Pos) );
	}
}

void CodeViewer::RemovePadding(string &line)
{
	const size_t LL = line.length();
	size_t Pos = (line.find_first_not_of("\n\r\t ,"));
	if (Pos == -1)
	{
		line.clear();
		return;
	}
	if (Pos > 0)
	{
		if ( (LL-Pos) < 1 ) return;
		line = line.substr(Pos, (LL-Pos) );
	}

	Pos =  (line.find_last_not_of("\n\r\t ,"));
	if (Pos != -1)
	{
		if ( Pos < 1 ) return;
		line = line.erase(Pos+1);
	}
}

void CodeViewer::RemoveNonVBSChars(string &line)
{
	size_t LL = line.length();
	size_t Pos = (line.find_first_of(VBValidChars));
	if (Pos == -1)
	{
		line.clear();
		return;
	}
	if (Pos > 0)
	{
		if ( (LL-Pos) < 1 ) return;
		line = line.substr(Pos, (LL-Pos) );
	}

	Pos =  (line.find_last_of(VBValidChars));
	if (Pos != -1)
	{
		line = line.erase(Pos+1);
	}
}

void CodeViewer::ParseForFunction() // Subs & Collections WIP 
{
  char text[MAX_LINE_LENGTH];
   const int scriptLines = (int)SendMessage(m_hwndScintilla, SCI_GETLINECOUNT, 0, 0);
   SendMessage(m_hwndFunctionList, CB_RESETCONTENT, 0, 0);
	int ParentLevel = 0; //root
	ParentTreeInvalid = false;
	for (int linecount = 0; linecount < scriptLines; ++linecount) 
   {
		// Read line
      const int lineLength = (int)SendMessage(m_hwndScintilla, SCI_LINELENGTH, linecount, 0);
		if ( !ParseOKLineLength(lineLength) ) continue;
		memset(text, 0, MAX_LINE_LENGTH);
		SendMessage(m_hwndScintilla, SCI_GETLINE, linecount, (LPARAM)text);
		if(text[0] != '\0')
		{
		    string wholeline(text);
		    ReadLineToParseBrain( wholeline, linecount, PageConstructsDict);
		}
	}
   //Propergate subs&funcs in menu in order
	for (vector<UserData>::iterator i = PageConstructsDict->begin(); i != PageConstructsDict->end(); ++i) 
   {
		if (i->eTyping < eDim)
		{
			const char *c_str1 = i->KeyName.c_str ();
			SendMessage(m_hwndFunctionList, CB_ADDSTRING, 0, (LPARAM)(c_str1) );
		}
   }
	//Collect Objects/Components from the menu. (cheat!)
	int CBCount = (int)SendMessage(m_hwndItemList, CB_GETCOUNT, 0, 0)-1;//Zero Based
	char c_str1[256]={0};
	UserData ud;
	while (CBCount >= 0) 
   {
		memset(c_str1,0,256);
		SendMessage(m_hwndItemList, CB_GETLBTEXT, CBCount, (LPARAM)c_str1);
		if(strlen(c_str1)>1)
		{
			ud.KeyName = string(c_str1);
			ud.UniqueKey = ud.KeyName;
			FindOrInsertUD(ComponentsDict,ud);
		}
		CBCount--;
   }
	//Now merge the lot for Auto complete...
	AutoCompList->clear();
	for (vector<UserData>::iterator i = VBwordsDict->begin(); i != VBwordsDict->end(); ++i)
	{
		FindOrInsertStringIntoAutolist(AutoCompList,i->KeyName);
	}
	string strVPcoreWords = "";
	for (vector<UserData>::iterator i = VP_CoreDict->begin(); i != VP_CoreDict->end(); ++i)
	{
		if (FindOrInsertStringIntoAutolist(AutoCompList,i->KeyName))
		{
			strVPcoreWords.append(i->KeyName);
			strVPcoreWords.append(" ");
		}
	}
	string strCompOut ="";
	for (vector<UserData>::iterator i = ComponentsDict->begin(); i != ComponentsDict->end(); ++i)
	{
		if (FindOrInsertStringIntoAutolist(AutoCompList,i->KeyName))
		{
			strCompOut.append(i->KeyName);
			strCompOut.append(" ");
		}
	}
	string sSubFunOut = "";
	for (vector<UserData>::iterator i = PageConstructsDict->begin(); i != PageConstructsDict->end(); ++i)
	{
		if (FindOrInsertStringIntoAutolist(AutoCompList,i->KeyName))
		{
			sSubFunOut.append(i->KeyName);
			sSubFunOut.append(" ");
		}
	}
	AutoCompString = "";
	for (vector<string>::iterator i = AutoCompList->begin(); i != AutoCompList->end();++i)
	{
		AutoCompString.append(i->data());
		AutoCompString += " ";
	}
   //Send the collected func/subs to scintilla for highlighting - always lowercase as VBS is case insensitive.
	//TODO: Need to comune with scintilla closer (COM pointers??)
	sSubFunOut = lowerCase(sSubFunOut);
	SendMessage(m_hwndScintilla, SCI_SETKEYWORDS, 1 , (LPARAM)sSubFunOut.c_str());
	strCompOut = lowerCase(strCompOut);
	SendMessage(m_hwndScintilla, SCI_SETKEYWORDS, 2 , (LPARAM)strCompOut.c_str());
	strVPcoreWords = lowerCase(strVPcoreWords);
	SendMessage(m_hwndScintilla, SCI_SETKEYWORDS, 3 , (LPARAM)strVPcoreWords.c_str());
}

void CodeViewer::ParseVPCore()
{
   //Open file
   const string sPath = string(g_pvp->m_szMyPath) + "scripts\\core.vbs";
   FILE* fCore;
   if (fopen_s(&fCore, sPath.c_str(), "r") != 0)
	if (!fCore)
	{
      char szLoadDir[MAX_PATH] = { 0 };
	  strcpy_s(szLoadDir, g_pvp->m_currentTablePath);
	  strcat_s(szLoadDir, "\\core.vbs");
	  if (fopen_s(&fCore, szLoadDir, "r") != 0)
		  if (!fCore)
		  {
			  szLoadDir[0] = '\0';
			  const HRESULT hr = GetRegString("RecentDir", "LoadDir", szLoadDir, MAX_PATH);
			  strcat_s(szLoadDir, "\\core.vbs");
			  if (fopen_s(&fCore, szLoadDir, "r") != 0)
				  if (!fCore)
				  {
					  MessageBox(m_hwndMain, "Couldn't find core.vbs for code completion parsing!", "Script Parser Warning", MB_OK);
					  return;
				  }
		  }
	}
	char text[MAX_LINE_LENGTH] = {};
	//intalise Parent child
///////////////////////
	int ParentLevel = 0; //root
	StopErrorDisplay = true;/// WIP BRANDREW (was set to false)
	ParentTreeInvalid = false;
	int linecount = 0;
	while (!feof(fCore))
	{
		memset(text, 0, MAX_LINE_LENGTH);
		fgets(text, MAX_LINE_LENGTH, fCore);
		if(text[0] != '\0')
		{
		    string wholeline(text);
		    ++linecount;
		    const int lineLength = (int)wholeline.length();
		    if ( !ParseOKLineLength(lineLength) ) continue;
		    ReadLineToParseBrain( wholeline, linecount, VP_CoreDict);
		}
	}
	fclose(fCore);
}

string CodeViewer::ExtractWordOperand(const string &line, const int &StartPos)
{
	size_t Substart = StartPos;
	const size_t lineLength = line.size();
	char linechar = 0;
	linechar = line[Substart];
	while ((ValidChars.find(linechar) == -1) && (Substart < lineLength))
	{
		Substart++;
		linechar = line[Substart];
	}
	//scan for last valid char
	size_t Subfinish = Substart;
	while ((ValidChars.find(linechar) != -1) && (Subfinish < lineLength))
	{
		Subfinish++;
		linechar = line[Subfinish];
	}
	return line.substr(Substart,Subfinish-Substart);
}

static CodeViewer* GetCodeViewerPtr(HWND hwndDlg)
{
   return (CodeViewer *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
}

LRESULT CALLBACK CodeViewWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   if (uMsg == g_FindMsgString)
   {
      CodeViewer * const pcv = GetCodeViewerPtr(hwndDlg);
      FINDREPLACE * const pfr = (FINDREPLACE *)lParam;
      if (pfr->Flags & FR_DIALOGTERM)
      {
         pcv->m_hwndFind = NULL;
         const size_t selstart = SendMessage(pcv->m_hwndScintilla, SCI_GETSELECTIONSTART, 0, 0);
         const size_t selend = SendMessage(pcv->m_hwndScintilla, SCI_GETSELECTIONEND, 0, 0);
         ::SetFocus(pcv->m_hwndScintilla);
         SendMessage(pcv->m_hwndScintilla, SCI_SETSEL, selstart, selend);
         return 0;
      }
      if (pfr->Flags & FR_FINDNEXT)
         pcv->Find(pfr);
      if ((pfr->Flags & FR_REPLACE) || (pfr->Flags & FR_REPLACEALL))
         pcv->Replace(pfr);
   }
	SCNotification * const pscn = (SCNotification *)lParam;
   switch (uMsg)
   {
   case WM_DESTROY:
   {
   }
   break;
   case WM_ACTIVATE:
   {
      if (LOWORD(wParam) != WA_INACTIVE)
		{
         g_pvp->m_pcv = GetCodeViewerPtr(hwndDlg);
			CodeViewer * const pcv = g_pvp->m_pcv;
			pcv->StopErrorDisplay = false;
			if (!(pcv->StopErrorDisplay))
			{
				pcv->StopErrorDisplay = true; ///stop Error reporting WIP
				pcv->ParseForFunction();
			}
			//pcv->StopErrorDisplay = false;
		}
   }
   break;

   case WM_CLOSE:
   {
      CodeViewer * const pcv = GetCodeViewerPtr(hwndDlg);
      pcv->SetVisible(fFalse);
      return 0;
   }
   break;

   case WM_COMMAND:
   {
      const int code = HIWORD(wParam);
      const int id = LOWORD(wParam);

      switch (code)
      {

		case SCEN_SETFOCUS:
		{
         CodeViewer * const pcv = GetCodeViewerPtr(hwndDlg);
			pcv->ParseForFunction();
		}
		break;
      case SCEN_CHANGE:
      {
			// TODO: Line Parse Brain here?
         CodeViewer * const pcv = GetCodeViewerPtr(hwndDlg);
         if (pcv->m_errorLineNumber != -1)
            pcv->UncolorError();
         if (!pcv->m_fIgnoreDirty && (pcv->m_sdsDirty < eSaveDirty))
         {
            pcv->m_sdsDirty = eSaveDirty;
            pcv->m_psh->SetDirtyScript(eSaveDirty);
         }
		}
      break;

      case BN_CLICKED: // or menu
      {
         CodeViewer * const pcv = GetCodeViewerPtr(hwndDlg);
         switch (id)
         {
         case ID_COMPILE:
            pcv->Compile();
            // Setting the script to started, and the back to initialized will clear the script out so we can re-do it later
				// - is this behavior just random or is it the way it's supposed to work?
            //pcv->m_pScript->SetScriptState(SCRIPTSTATE_CLOSED /*SCRIPTSTATE_STARTED*/);
            pcv->EndSession();
            break;
			case ID_SCRIPT_PREFERENCES:			
				DialogBox(g_hinst, MAKEINTRESOURCE(IDD_CODEVIEW_PREFS), hwndDlg, CVPrefProc);
				break;
 			case ID_FIND:
            pcv->ShowFindDialog();
            break;
			case ID_REPLACE:
            pcv->ShowFindReplaceDialog();
            break;
			case ID_EDIT_FINDNEXT:
            pcv->Find(&pcv->m_findreplaceold);
            break;
         case ID_EDIT_UNDO:
            SendMessage(pcv->m_hwndScintilla, SCI_UNDO, 0, 0);
            break;
         case ID_EDIT_COPY:
            SendMessage(pcv->m_hwndScintilla, WM_COPY, 0, 0);
            break;
         case ID_EDIT_CUT:
            SendMessage(pcv->m_hwndScintilla, WM_CUT, 0, 0);
            break;
         case ID_EDIT_PASTE:
            SendMessage(pcv->m_hwndScintilla, WM_PASTE, 0, 0);
            break;
 			case ID_ADD_COMMENT:
            AddComment(pcv->m_hwndScintilla);
            break;
 			case ID_REMOVE_COMMENT:
            RemoveComment(pcv->m_hwndScintilla);
            break;
         }
      }
      break;
      case CBN_SETFOCUS:
      {
         if (id == IDC_FUNCTIONLIST)
			{
				CodeViewer * const pcv = GetCodeViewerPtr(hwndDlg);
				pcv->StopErrorDisplay = true;
				pcv->ParseForFunction();
			}
         break;
      }
      case CBN_SELCHANGE: // Or accelerator
      {
         CodeViewer * const pcv = GetCodeViewerPtr(hwndDlg);
         switch (id)
         {
         case ID_FIND: // accelerator
            pcv->ShowFindDialog();
            break;
         case ID_SAVE:
         case ID_TABLE_PLAY:
            pcv->m_psh->DoCodeViewCommand(id);
            break;
         case ID_EDIT_FINDNEXT:
            pcv->Find(&pcv->m_findreplaceold);
            break;
         case ID_REPLACE:
            pcv->ShowFindReplaceDialog();
            break;
         case ID_EDIT_UNDO:
            SendMessage(pcv->m_hwndScintilla, SCI_UNDO, 0, 0);
            break;
         case IDC_ITEMLIST:
         {
            pcv->ListEventsFromItem();
            pcv->TellHostToSelectItem();
         }
         break;

         case IDC_EVENTLIST:
         {
            pcv->FindCodeFromEvent();
         }
         break;
         case IDC_FUNCTIONLIST:
         {
            const size_t Listindex = SendMessage(pcv->m_hwndFunctionList, CB_GETCURSEL, 0, 0);
				if (Listindex != -1)
				{
					char ConstructName[MAX_FIND_LENGTH] = {0};
					size_t index = SendMessage(pcv->m_hwndFunctionList, CB_GETLBTEXT, Listindex, (LPARAM)ConstructName);
					vector<UserData>::iterator i;
					int idx = 0;
					int Pos = pcv->FindUD(pcv->PageConstructsDict, string(ConstructName), i, idx);
					SendMessage(pcv->m_hwndScintilla, SCI_GOTOLINE, i->LineNum, 0);
					SendMessage(pcv->m_hwndScintilla, SCI_GRABFOCUS, 0, 0);
				}
         }
         break;
         case ID_ADD_COMMENT:
         {
            AddComment(pcv->m_hwndScintilla);
            break;
         }
         case ID_REMOVE_COMMENT:
         {
            RemoveComment(pcv->m_hwndScintilla);
            break;
         }
			case ID_SHOWAUTOCOMPLETE:
			{
				pcv->ShowAutoComplete(pscn);
				break;
			}
         }//switch (id)
      }
      break;
      }
   }
   break;

   case WM_NOTIFY:
   {
      //int idCtrl = (int) wParam;
      NMHDR * const pnmh = (LPNMHDR)lParam;
      HWND hwndRE = pnmh->hwndFrom;
      const int code = pnmh->code;
      CodeViewer *pcv = GetCodeViewerPtr(hwndDlg);
		//SCNotification * const pscn = (SCNotification *)lParam;
      switch (code)
      {

			case SCN_SAVEPOINTREACHED:
			{
				if (pcv->m_sdsDirty > eSaveClean)
				{
					pcv->m_sdsDirty = eSaveClean;
					pcv->m_psh->SetDirtyScript(eSaveClean);
				}
			}
			break;
		  case SCN_DWELLSTART:
			{
				if (pcv->DwellDisplay)
					pcv->g_ToolTipActive = pcv->ShowTooltip(pscn);
			}
			break;

		  case SCN_DWELLEND:
			{
				if (pcv->g_ToolTipActive)
				{
					SendMessage(pcv->m_hwndScintilla,SCI_CALLTIPCANCEL , 0, 0 );
					pcv->g_ToolTipActive = false;
				}
			} break;

			case SCN_CHARADDED:
			{
				// Member selection
				if (pcv->DisplayAutoComplete)
					pcv->ShowAutoComplete(pscn);
			}
			break;

		  case SCN_UPDATEUI:
			{
				char szT[256];
				size_t pos = SendMessage(hwndRE, SCI_GETCURRENTPOS, 0, 0);
				const size_t line = SendMessage(hwndRE, SCI_LINEFROMPOSITION, pos, 0) + 1;
				const size_t column = SendMessage(hwndRE, SCI_GETCOLUMN, pos, 0);

				sprintf_s(szT, "Line %u, Col %u", (U32)line, (U32)column);
				SendMessage(pcv->m_hwndStatus, SB_SETTEXT, 0 | 0, (size_t)szT);
			}
			break;

			case SCN_DOUBLECLICK:
			{
				pcv->WordUnderCaret.lpstrText = CaretTextBuff;
				memset(CaretTextBuff, 0, MAX_FIND_LENGTH);
				pcv->GetWordUnderCaret();
				pcv->szLower(pcv->WordUnderCaret.lpstrText);
				// set back ground colour of all words on display
				SendMessage(pcv->m_hwndScintilla, SCI_STYLESETBACK, SCE_B_KEYWORD5, RGB(200,200,200) );
				SendMessage(pcv->m_hwndScintilla, SCI_SETKEYWORDS, 4 , (LPARAM)CaretTextBuff);
			}
			break;

			case SCN_MARGINCLICK:
			{
				if (pscn->margin == 1)
				{
					pcv->MarginClick(pscn->position, pscn->modifiers);
				}
			}
			break;
      }

      break;
   }

   //case RECOLOR_LINE:
   //{
   //   CodeViewer * const pcv = GetCodeViewerPtr(hwndDlg);

   //   for (size_t i = wParam; i <= (size_t)lParam; ++i)
   //      pcv->ColorLine(i);
   //}
   //break;

   case WM_SIZE:
   {
      RECT rc;
      GetClientRect(hwndDlg, &rc);
      CodeViewer * const pcv = GetCodeViewerPtr(hwndDlg);

      if (pcv && pcv->m_hwndStatus)
      {
         SendMessage(pcv->m_hwndStatus, WM_SIZE, wParam, lParam);

         RECT rcStatus;
         GetClientRect(pcv->m_hwndStatus, &rcStatus);
         const int statheight = rcStatus.bottom - rcStatus.top;

         const int buttonwidth = 0;

         SetWindowPos(pcv->m_hwndScintilla, NULL,
            0, 0, rc.right - rc.left - buttonwidth/* - 20*/, rc.bottom - rc.top - 10 - statheight - 30, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);
      }
   }
   break;
   }

   return DefWindowProc(hwndDlg, uMsg, wParam, lParam);
}

INT_PTR CALLBACK CVPrefProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   switch (uMsg)
   {
   case WM_INITDIALOG:
   {
      HWND hwndParent = GetParent(hwndDlg);
      RECT rcDlg;
      RECT rcMain;
      GetWindowRect(hwndParent, &rcMain);
      GetWindowRect(hwndDlg, &rcDlg);
      SetWindowPos(hwndDlg, NULL,
         (rcMain.right + rcMain.left) / 2 - (rcDlg.right - rcDlg.left) / 2,
         (rcMain.bottom + rcMain.top) / 2 - (rcDlg.bottom - rcDlg.top) / 2,
         0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
		CodeViewer* pcv = GetCodeViewerPtr(hwndParent);
		if (pcv->lPrefsList)
		{
			for (size_t i = 0; i < pcv->lPrefsList->size(); i++)
			{
				pcv->lPrefsList->at(i)->GetPrefsFromReg();
				pcv->lPrefsList->at(i)->SetCheckBox(hwndDlg);
			}
			pcv->crBackColor = GetRegIntWithDefault("CVEdit", "BackGroundColor", RGB(255,255,255));
			pcv->UpdateScinFromPrefs();
			HWND hChkBox = GetDlgItem(hwndDlg,IDC_CVP_CHKBOX_SHOWAUTOCOMPLETE);
			SNDMSG(hChkBox, BM_SETCHECK, pcv->DisplayAutoComplete ? BST_CHECKED : BST_UNCHECKED, 0L);
			hChkBox = GetDlgItem(hwndDlg, IDC_CVP_CHKBOX_DISPLAYDWELL);
			SNDMSG(hChkBox, BM_SETCHECK, pcv->DwellDisplay ? BST_CHECKED : BST_UNCHECKED, 0L);
			hChkBox = GetDlgItem(hwndDlg, IDC_CVP_CHKBOX_HELPWITHDWELL);
			SNDMSG(hChkBox, BM_SETCHECK, pcv->DwellHelp ? BST_CHECKED : BST_UNCHECKED, 0L);

			char foo[65] = {0};
			sprintf_s(foo,"%i", pcv->DisplayAutoCompleteLength );
			SetDlgItemText( hwndDlg, IDC_CVP_EDIT_AUTOCHARS, foo);
		
			sprintf_s(foo,"%i", pcv->DwellDisplayTime );
			SetDlgItemText( hwndDlg, IDC_CVP_EDIT_MOUSEDWELL, foo);
		}

		//SetFocus(hwndDlg);
		//#if !(defined(IMSPANISH) | defined(IMGERMAN) | defined(IMFRENCH))
		//      HWND hwndTransName = GetDlgItem(hwndDlg, IDC_TRANSNAME);
		//      ShowWindow(hwndTransName, SW_HIDE);
		//#endif
		//
		//#if !(defined(IMSPANISH))
		//      HWND hwndTransSite = GetDlgItem(hwndDlg, IDC_TRANSLATEWEBSITE);
		//      ShowWindow(hwndTransSite, SW_HIDE);
		//#endif
	}
   break; //case WM_INITDIALOG:

   case WM_COMMAND:
   {
      switch (HIWORD(wParam))
      {
      case BN_CLICKED:
			CodeViewer * const pcv = GetCodeViewerPtr(GetParent(hwndDlg));
			if (pcv)
			{
				int wParamLowWord = LOWORD(wParam);
				switch (wParamLowWord)
				{
				case IDC_CVP_BUT_CANCEL:
				{
					pcv->UpdatePrefsfromReg();
					pcv->UpdateScinFromPrefs();
					EndDialog(hwndDlg, FALSE);
				}
				break;
				case IDC_CVP_BUT_OK:
				{
					//Save to reg.
					pcv->DisplayAutoCompleteLength = GetDlgItemInt(hwndDlg, IDC_CVP_EDIT_AUTOCHARS, 0 , false);
					pcv->DwellDisplayTime = GetDlgItemInt(hwndDlg, IDC_CVP_EDIT_MOUSEDWELL, 0 , false);
					pcv->UpdateRegWithPrefs();
					pcv->UpdateScinFromPrefs();
					EndDialog(hwndDlg, TRUE);
				}
				break;
				case IDC_CVP_CHKBOX_DISPLAYDWELL:
				{
					if(IsDlgButtonChecked(hwndDlg, IDC_CVP_CHKBOX_DISPLAYDWELL) )
						{pcv->DwellDisplay = true;}
					else
						{pcv->DwellDisplay = false;}
					SetRegValueBool("CVEdit", "DwellDisplay", pcv->DwellDisplay );
				}
				break;
				case IDC_CVP_CHKBOX_HELPWITHDWELL:
				{
					if(IsDlgButtonChecked(hwndDlg, IDC_CVP_CHKBOX_HELPWITHDWELL) )
						{pcv->DwellHelp = true;}
					else
						{pcv->DwellHelp = false;}
					SetRegValueBool("CVEdit", "DwellHelp", pcv->DwellHelp );
				}
				break;
				case IDC_CVP_CHKBOX_SHOWAUTOCOMPLETE:
				{
					if(IsDlgButtonChecked(hwndDlg, IDC_CVP_CHKBOX_SHOWAUTOCOMPLETE) )
						{pcv->DisplayAutoComplete = true;}
					else
						{pcv->DisplayAutoComplete = false;}
					SetRegValueBool("CVEdit", "DisplayAutoComplete", pcv->DisplayAutoComplete );
				}
				break;
				//TODO: Impliment IDC_CVP_BUT_COL_BACKGROUND
/*				case IDC_CVP_BUT_COL_BACKGROUND:
				{
					CHOOSECOLOR cc;
					memset(&cc, 0, sizeof(CHOOSECOLOR));
					cc.lStructSize = sizeof(CHOOSECOLOR);
					cc.hwndOwner = hwndDlg;
					cc.rgbResult = pcv->crBackColor;
					cc.lpCustColors = pcv->g_PrefCols;
					cc.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;
					if (ChooseColor(&cc))
					{
						pcv->crBackColor = cc.rgbResult;
						pcv->UpdateScinFromPrefs();
						return DefWindowProc(hwndDlg, uMsg, wParam, lParam);
					}
				}
				break;*/

				case IDC_CVP_BUT_COL_EVERYTHINGELSE:
				{
					CHOOSECOLOR cc;
					memset(&cc, 0, sizeof(CHOOSECOLOR));
					cc.lStructSize = sizeof(CHOOSECOLOR);
					cc.hwndOwner = hwndDlg;
					cc.rgbResult = pcv->prefEverythingElse->rgb;
					cc.lpCustColors = pcv->g_PrefCols; 
					cc.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;
					if (ChooseColor(&cc))
					{
						pcv->prefEverythingElse->rgb = cc.rgbResult;
						pcv->UpdateScinFromPrefs();
						return DefWindowProc(hwndDlg, uMsg, wParam, lParam);
					}
				}
				break;
				case IDC_CVP_BUT_FONT_EVERYTHINGELSE:
				{
					pcv->prefEverythingElse->LogFont.lfHeight = pcv->prefEverythingElse->GetHeightFromPointSize(hwndDlg);
					CHOOSEFONT cf;
					memset(&cf, 0, sizeof(CHOOSEFONT));
					cf.lStructSize = sizeof(CHOOSEFONT);
					cf.Flags = CF_NOVERTFONTS | CF_EFFECTS | CF_INITTOLOGFONTSTRUCT ;
					cf.hDC = GetDC(hwndDlg);
					cf.hwndOwner = hwndDlg;
					cf.iPointSize = pcv->prefEverythingElse->PointSize * 10;
					cf.lpLogFont = &(pcv->prefEverythingElse->LogFont);
					cf.rgbColors = pcv->prefEverythingElse->rgb;
					if (ChooseFont(&cf))
					{
						pcv->prefEverythingElse->rgb = cf.rgbColors;
						pcv->prefEverythingElse->PointSize = cf.iPointSize / 10;
						pcv->UpdateScinFromPrefs();
						return DefWindowProc(hwndDlg, uMsg, wParam, lParam);
					}
				}
				break;
				default:
					//EverythingElse=0, default=1, consumed above
					for (size_t i = 2 ; i < pcv->lPrefsList->size(); ++i)
					{
						CVPrefrence* Pref = pcv->lPrefsList->at(i);
						if (Pref->IDC_ChkBox_code == wParamLowWord)// && Pref->IDC_ChkBox_code != 0)
						{
							Pref->ReadCheckBox(hwndDlg);
							pcv->UpdateScinFromPrefs();
							break;
						}

						if (Pref->IDC_ColorBut_code == wParamLowWord)
						{
							CHOOSECOLOR cc;
							memset(&cc, 0, sizeof(CHOOSECOLOR));
							cc.lStructSize = sizeof(CHOOSECOLOR);
							cc.hwndOwner = hwndDlg;
							cc.rgbResult = Pref->rgb;
							cc.lpCustColors = pcv->g_PrefCols;
							cc.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;
							if (ChooseColor(&cc))
							{
								Pref->rgb = cc.rgbResult;
								pcv->UpdateScinFromPrefs();
								return DefWindowProc(hwndDlg, uMsg, wParam, lParam);
							}
							break;
						}

						if (Pref->IDC_Font_code == wParamLowWord)
						{
							Pref->LogFont.lfHeight = Pref->GetHeightFromPointSize(hwndDlg);
							CHOOSEFONT cf;
							memset(&cf, 0, sizeof(CHOOSEFONT));
							cf.lStructSize = sizeof(CHOOSEFONT);
							cf.Flags = CF_NOVERTFONTS | CF_EFFECTS | CF_INITTOLOGFONTSTRUCT ;
							cf.hDC = GetDC(hwndDlg);
							cf.hwndOwner = hwndDlg;
							cf.iPointSize = Pref->PointSize * 10;
							cf.lpLogFont = &Pref->LogFont;
							cf.rgbColors = Pref->rgb;
							if (ChooseFont(&cf))
							{
								Pref->rgb = cf.rgbColors;
								Pref->PointSize = cf.iPointSize / 10;
								pcv->UpdateScinFromPrefs();
								return DefWindowProc(hwndDlg, uMsg, wParam, lParam);
								break;
							}
						}
					}// for pref
				}// end switch Button clicked
			}// end if (pcv->prefVBS)
      }// end switch Hiword Wparam
   }// case WM_COMMAND
   break;
   case WM_CLOSE:
      EndDialog(hwndDlg, TRUE);
   break;
   }
   return FALSE; // be selfish - consume all
   //return DefWindowProc(hwndDlg, uMsg, wParam, lParam);

}

void CodeViewer::UpdateScinFromPrefs()
{
   SendMessage(m_hwndScintilla, SCI_SETMOUSEDWELLTIME, DwellDisplayTime, 0);
	SendMessage(m_hwndScintilla, SCI_STYLESETBACK, prefEverythingElse->SciKeywordID, crBackColor);
	prefEverythingElse->ApplyPreferences(m_hwndScintilla, prefEverythingElse);//Update internally
	SendMessage(m_hwndScintilla,SCI_STYLECLEARALL,0,0);
	SendMessage(m_hwndScintilla, SCI_MARKERSETFORE, SC_MARKNUM_FOLDEROPEN, prefEverythingElse->rgb);
	SendMessage(m_hwndScintilla, SCI_MARKERSETBACK, SC_MARKNUM_FOLDEROPEN, crBackColor);
	prefDefault->ApplyPreferences(m_hwndScintilla, prefEverythingElse);
	prefVBS->ApplyPreferences(m_hwndScintilla, prefEverythingElse);
	prefSubs->ApplyPreferences(m_hwndScintilla, prefEverythingElse);
	prefComps->ApplyPreferences(m_hwndScintilla, prefEverythingElse);
	prefLiterals->ApplyPreferences(m_hwndScintilla, prefEverythingElse);
	prefComments->ApplyPreferences(m_hwndScintilla, prefEverythingElse);
	prefVPcore->ApplyPreferences(m_hwndScintilla, prefEverythingElse);
	SendMessage(m_hwndScintilla, SCI_STYLESETBACK, SCE_B_KEYWORD5, RGB(200,200,200) );
	SendMessage(m_hwndScintilla, SCI_SETKEYWORDS, 4 , (LPARAM)WordUnderCaret.lpstrText);

}

Collection::Collection()
{
   m_fFireEvents = fFalse;
   m_fStopSingleEvents = fFalse;

   int groupElementsCollection = GetRegIntWithDefault("Editor", "GroupElementsInCollection", 1);
   m_fGroupElements = groupElementsCollection;
}

STDMETHODIMP Collection::get_Name(BSTR *pVal)
{
   *pVal = SysAllocString((WCHAR *)m_wzName);

   return S_OK;
}

IDispatch *Collection::GetDispatch()
{
   return (IDispatch *)this;
}

ISelect *Collection::GetISelect()
{
   return NULL;
}

HRESULT Collection::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   BiffWriter bw(pstm, hcrypthash, hcryptkey);

   bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);

   for (int i = 0; i < m_visel.Size(); ++i)
   {
      IEditable * const piedit = m_visel.ElementAt(i)->GetIEditable();
      IScriptable * const piscript = piedit->GetScriptable();
      bw.WriteWideString(FID(ITEM), piscript->m_wzName);
   }

   bw.WriteBool(FID(EVNT), m_fFireEvents);
   bw.WriteBool(FID(SSNG), m_fStopSingleEvents);
   bw.WriteBool(FID(GREL), m_fGroupElements);

   bw.WriteTag(FID(ENDB));

   return S_OK;
}

HRESULT Collection::LoadData(IStream *pstm, PinTable *ppt, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   BiffReader br(pstm, this, ppt, version, hcrypthash, hcryptkey);

   br.Load();
   return S_OK;
}

BOOL Collection::LoadToken(int id, BiffReader *pbr)
{
   if (id == FID(NAME))
   {
      pbr->GetWideString((WCHAR *)m_wzName);
   }
   else if (id == FID(EVNT))
   {
      pbr->GetBool(&m_fFireEvents);
   }
   else if (id == FID(SSNG))
   {
      pbr->GetBool(&m_fStopSingleEvents);
   }
   else if (id == FID(GREL))
   {
      pbr->GetBool(&m_fGroupElements);
   }
   else if (id == FID(ITEM))
   {
      //!! BUG - item list must be up to date in table (loaded) for the reverse name lookup to work
      PinTable * const ppt = (PinTable *)pbr->m_pdata;

      WCHAR wzT[MAXNAMEBUFFER];
      pbr->GetWideString((WCHAR *)wzT);

      for (int i = 0; i < ppt->m_vedit.Size(); ++i)
      {
         IScriptable * const piscript = ppt->m_vedit.ElementAt(i)->GetScriptable();
         if (piscript) // skip decals
         {
            if (!WideStrCmp(piscript->m_wzName, wzT))
            {
               piscript->GetISelect()->GetIEditable()->m_vCollection.AddElement(this);
               piscript->GetISelect()->GetIEditable()->m_viCollection.AddElement((void *)m_visel.Size());
               m_visel.AddElement(piscript->GetISelect());
               return fTrue;
            }
         }
      }
   }

   return fTrue;
}

STDMETHODIMP Collection::get_Count(long __RPC_FAR *plCount)
{
   *plCount = m_visel.Size();
   return S_OK;
}

STDMETHODIMP Collection::get_Item(long index, IDispatch __RPC_FAR * __RPC_FAR *ppidisp)
{
   if (index < 0 || index >= m_visel.Size())
      return TYPE_E_OUTOFBOUNDS;

   IDispatch * const pdisp = m_visel.ElementAt(index)->GetDispatch();
   return pdisp->QueryInterface(IID_IDispatch, (void **)ppidisp);
}

STDMETHODIMP Collection::get__NewEnum(IUnknown** ppunk)
{
   CComObject<OMCollectionEnum> *pomenum;
   HRESULT hr = CComObject<OMCollectionEnum>::CreateInstance(&pomenum);

   if (SUCCEEDED(hr))
   {
      pomenum->Init(this);
      hr = pomenum->QueryInterface(IID_IEnumVARIANT, (void **)ppunk);
   }

   return hr;
}

OMCollectionEnum::OMCollectionEnum()
{
}

OMCollectionEnum::~OMCollectionEnum()
{
}

STDMETHODIMP OMCollectionEnum::Init(Collection *pcol)
{
   m_pcol = pcol;
   m_index = 0;
   return S_OK;
}

STDMETHODIMP OMCollectionEnum::Next(ULONG celt, VARIANT __RPC_FAR *rgVar, ULONG __RPC_FAR *pCeltFetched)
{
   int last;
   HRESULT hr;
   const int cwanted = celt;
   int creturned;

   if (m_index + cwanted > m_pcol->m_visel.Size())
   {
      hr = S_FALSE;
      last = m_pcol->m_visel.Size();
      creturned = m_pcol->m_visel.Size() - m_index;
   }
   else
   {
      hr = S_OK;
      last = m_index + cwanted;
      creturned = cwanted;
   }

   for (int i = m_index; i < last; ++i)
   {
      IDispatch * const pdisp = m_pcol->m_visel.ElementAt(i)->GetDispatch();

      pdisp->QueryInterface(IID_IDispatch, (void **)&pdisp);

      V_VT(&rgVar[i - m_index]) = VT_DISPATCH;
      V_DISPATCH(&rgVar[i - m_index]) = pdisp;
   }

   m_index += creturned;

   if (pCeltFetched)
      *pCeltFetched = creturned;

   return hr;
}

STDMETHODIMP OMCollectionEnum::Skip(ULONG celt)
{
   m_index += celt;
   return (m_index >= m_pcol->m_visel.Size()) ? S_FALSE : S_OK;
}

STDMETHODIMP OMCollectionEnum::Reset()
{
   m_index = 0;

   return S_OK;
}

STDMETHODIMP OMCollectionEnum::Clone(IEnumVARIANT __RPC_FAR *__RPC_FAR *ppEnum)
{
   IUnknown *punk;
   HRESULT hr = m_pcol->get__NewEnum(&punk);

   if (SUCCEEDED(hr))
   {
      hr = punk->QueryInterface(IID_IEnumVARIANT, (void **)ppEnum);

      punk->Release();
   }

   return hr;
}

void DebuggerModule::Init(CodeViewer * const pcv)
{
   m_pcv = pcv;
}

STDMETHODIMP DebuggerModule::Print(VARIANT *pvar)
{
   if (!g_pplayer->m_hwndDebugOutput)
      return S_OK;

   if (pvar->vt == VT_EMPTY || pvar->vt == VT_NULL || pvar->vt == VT_ERROR)
   {
      m_pcv->AddToDebugOutput("");
      return S_OK;
   }

   CComVariant varT;
   const HRESULT hr = VariantChangeType(&varT, pvar, 0, VT_BSTR);

   if (FAILED(hr))
   {
      LocalString ls(IDS_DEBUGNOCONVERT);
      m_pcv->AddToDebugOutput(ls.m_szbuffer);
      return S_OK;
   }

   WCHAR * const wzT = V_BSTR(&varT);
   const int len = lstrlenW(wzT);

   char * const szT = new char[len + 1];

   WideCharToMultiByte(CP_ACP, 0, wzT, -1, szT, len + 1, NULL, NULL);

   m_pcv->AddToDebugOutput(szT);

   delete[] szT;

   return S_OK;
}

IDispatch *DebuggerModule::GetDispatch()
{
   return (IDispatch *)this;
}

STDMETHODIMP DebuggerModule::get_Name(BSTR *pVal)
{
   *pVal = SysAllocString(L"Debug");

   return S_OK;
}
