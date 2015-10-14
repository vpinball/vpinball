#include "stdafx.h"

#include <initguid.h>

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


LRESULT CALLBACK CodeViewWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

WNDPROC g_RichEditProc;
bool g_ToolTipActive;
string vbsKeyWords;
bool FindOrInsertUD(list<UserData>* ListIn, UserData ud);
bool FindOrInsertStringIntoAutolist(list<string>* ListIn, string strIn);
list<UserData> *g_VBwords;
list<string> *g_AutoComp;
list<UserData> *g_UserFunc;
list<UserData> *g_Components;
string g_AutoCompList;


UserData::UserData()
{
	intLineNum=0;
	strDescription="";
	strKeyName="";
}

UserData::~UserData()
{
}

UserData::UserData(const int LineNo, const string &Desc, const string &Name)
{
	intLineNum=LineNo;
	strDescription=Desc;
	strKeyName=Name;
}


bool UserData::FuncSortUD (const UserData &first, const UserData &second)
{
  const string strF = CodeViewer::lowerCase(first.strKeyName);
  const string strS = CodeViewer::lowerCase(second.strKeyName);
  basic_string <char>::size_type i=0;
  while ( (i<strF.length()) && (i<strS.length() ) )
  {
	  if (strF[i]<strS[i]) return true;
	  else if (strF[i]>strS[i]) return false;
    ++i;
  }
  return ( strF.length() < strF.length() );
}


//Assumes case insensitive sorted list (found = false):
bool FindOrInsertUD(list<UserData>* ListIn, UserData ud)
{
	//First in the list
	if (ListIn->size() == 0)
	{
		ListIn->push_front(ud);
		return true;
	}
	list<UserData>::iterator i = ListIn->begin();
	int counter = (int)ListIn->size();
	int result = -1;
	const string strSearchData =  CodeViewer::lowerCase( ud.strKeyName );

	while (counter)
	{
		const string strTableData = CodeViewer::lowerCase(i->strKeyName);
		result = strTableData.compare(strSearchData);
		if (result < 0)
		{
			++i;
			counter--;
		}
		else break;
	}
	if (result == 0) return false;//Already Exists.
	if (result > 0 && (counter != 0))//Add new ud somewhere in middle.
	{
		ListIn->insert(i,ud);
		return true;
	}
	if (result > 0 && (counter == 0))	//It's new at and at the very bottom.
	{
		ListIn->push_back(ud);
		return true;
	}
	else //it's 1 above the last on the list.
	{
		ListIn->insert(i,ud);
		return true;
	}
	return false;//Oh pop poop, never should hit here.
}

bool FindOrInsertStringIntoAutolist(list<string>* ListIn, string strIn)
{
	//First in the list
	if (ListIn->empty())
	{
		ListIn->push_front(strIn);
		return true;
	}
	const string strLowerIn = CodeViewer::lowerCase(strIn);
	list<string>::iterator i = ListIn->begin();
	int counter = (int)ListIn->size();
	int result = -1;
	while (counter)
	{
		const string strLowerComp = CodeViewer::lowerCase(string(i->data()));
		result = strLowerComp.compare(strLowerIn);
		if (result < 0)
		{
			++i;
			counter--;
		}
		else break;
	}
	if (result == 0) return false;//Already Exists.
	if (result > 0 && (counter != 0))//Add new ud somewhere in middle.
	{
		ListIn->insert(i,strIn);
		return true;
	}
	if (result > 0 && (counter == 0))	//It's new at and at the very bottom.
	{
		ListIn->push_back(strIn);
		return true;
	}
	else //it's 1 above the last on the list.
	{
		ListIn->insert(i,strIn);
		return true;
	}
	return false;//Oh pop poop, never should hit here.
}

IScriptable::IScriptable()
{
   m_wzName[0] = 0;
}

CodeViewDispatch::CodeViewDispatch()
{
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

void GetRange(HWND m_hwndScintilla, int start, int end, char *text)
{
   Sci_TextRange tr;
   tr.chrg.cpMin = start;
   tr.chrg.cpMax = end;
   tr.lpstrText = text;
   SendMessage(m_hwndScintilla, SCI_GETTEXTRANGE, 0, (LPARAM)&tr);
}

void CodeViewer::SetClean(SaveDirtyState sds)
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

HRESULT CodeViewer::AddTemporaryItem(BSTR bstr, IDispatch *pdisp)
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

HRESULT CodeViewer::AddItem(IScriptable *piscript, BOOL fGlobal)
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

void CodeViewer::RemoveItem(IScriptable *piscript)
{
   CComBSTR bstr;
   piscript->get_Name(&bstr);

   CodeViewDispatch *pcvd;

   int idx = m_vcvd.GetSortedIndex(bstr);

   if (idx == -1)
      return;

   pcvd = m_vcvd.ElementAt(idx);

   _ASSERTE(pcvd);

   m_vcvd.RemoveElementAt(idx);

   // Remove item from dropdown
   char szT[64]; // Names can only be 32 characters (plus terminator)
   WideCharToMultiByte(CP_ACP, 0, bstr, -1, szT, 64, NULL, NULL);
   size_t index = SendMessage(m_hwndItemList, CB_FINDSTRINGEXACT, ~0u, (size_t)szT);
   SendMessage(m_hwndItemList, CB_DELETESTRING, index, 0);

   delete pcvd;
}

void CodeViewer::SelectItem(IScriptable *piscript)
{
   CComBSTR bstr;
   piscript->get_Name(&bstr);

   char szT[64]; // Names can only be 32 characters (plus terminator)
   WideCharToMultiByte(CP_ACP, 0, bstr, -1, szT, 64, NULL, NULL);

   const size_t index = SendMessage(m_hwndItemList, CB_FINDSTRINGEXACT, ~0u, (size_t)szT);
   SendMessage(m_hwndItemList, CB_SETCURSEL, index, 0);

   ListEventsFromItem();
}

HRESULT CodeViewer::ReplaceName(IScriptable *piscript, WCHAR *wzNew)
{
   if (m_vcvd.GetSortedIndex(wzNew) != -1)
      return E_FAIL;

   CComBSTR bstr;
   piscript->get_Name(&bstr);

   int idx = m_vcvd.GetSortedIndex(bstr);
   if (idx == -1)
      return E_FAIL;

   CodeViewDispatch * const pcvd = m_vcvd.ElementAt(idx);

   _ASSERTE(pcvd);

   m_vcvd.RemoveElementAt(idx);

   lstrcpyW(pcvd->m_wzName, wzNew);

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

void CodeViewer::SetVisible(BOOL fVisible)
{
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

   if (fVisible)
      SetWindowPos(m_hwndMain, HWND_TOP,
      0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

void CodeViewer::SetEnabled(BOOL fEnabled)
{
   SendMessage(m_hwndScintilla, SCI_SETREADONLY, !fEnabled, 0);

   EnableWindow(m_hwndItemList, fEnabled);
   EnableWindow(m_hwndEventList, fEnabled);
}

void CodeViewer::SetCaption(char *szCaption)
{
   char szT[_MAX_PATH];
   strcpy_s(szT, sizeof(szT), szCaption);
   LocalString ls(IDS_SCRIPT);
   strcat_s(szT, sizeof(szT), " ");
   strcat_s(szT, sizeof(szT), ls.m_szbuffer);
   SetWindowText(m_hwndMain, szT);
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
   m_hwndMain = CreateWindowEx(0, "CVFrame", "Script",
      WS_POPUP | WS_SIZEBOX | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
      10, 10, 300, 300, m_hwndMain, NULL, g_hinst, 0);

   SetWindowLongPtr(m_hwndMain, GWLP_USERDATA, (size_t)this);

   SetWindowPos(m_hwndMain, NULL,
      0, 0, 640, 490, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);

   m_hwndScintilla = CreateWindowEx(0, "Scintilla", "",
      WS_CHILD | ES_NOHIDESEL | WS_VISIBLE | ES_SUNKEN | WS_HSCROLL | WS_VSCROLL | ES_MULTILINE | ES_WANTRETURN,
      0, 10 + 32, 300, 290, m_hwndMain, NULL, g_hinst, 0);
// Create new list of user functions & Collections- filled in ParseForFunction(), first called in LoadFromStrem()
	g_AutoComp = new list<string>();
	g_Components = new list<UserData>();
	g_UserFunc = new list<UserData>(); 
// parse vb reserved words for auto complete.
	g_VBwords = new list<UserData>;
	int intWordFinish =-1; //skip space
	char WordChar= vbsReservedWords[0];
	char szWord[256];
	int wordlen=0;
	while (WordChar != 0) //Just make sure with chars, we reached EOL
	{
		memset(szWord,0,256);
		wordlen=0;
		intWordFinish++; //skip space
		WordChar= vbsReservedWords[intWordFinish];
		while (WordChar != 0 && WordChar != ' ')
		{
			szWord[wordlen]=WordChar;
			intWordFinish++;
			wordlen++;
			WordChar= vbsReservedWords[intWordFinish];
		}
		UserData ud;
		ud.strKeyName = string(szWord);
		g_VBwords->push_front(ud);
	}
	g_VBwords->sort(UserData::FuncSortUD);
	vbsKeyWords = new char[strlen(vbsReservedWords)+2];
	vbsKeyWords.clear();
	for (list<UserData>::iterator i = g_VBwords->begin();i != g_VBwords->end(); ++i)
	{
		//make vbsKeyWords in order. (the cat has been kicked out :)
		vbsKeyWords += lowerCase( i->strKeyName);
		vbsKeyWords += " ";
		//Then capitalise first letter
		WordChar = i->strKeyName.at(0);
		if (WordChar >= 'a' && WordChar <= 'z' ) WordChar -= ('a'- 'A');
		i->strKeyName.at(0) = WordChar;	
	}
   SendMessage(m_hwndScintilla, SCI_SETLEXER, (WPARAM)SCLEX_VBSCRIPT, 0);
   
	SendMessage(m_hwndScintilla, SCI_SETKEYWORDS, 0, (LPARAM)vbsReservedWords );
   SendMessage(m_hwndScintilla, SCI_SETTABWIDTH, 4, 0);

   SendMessage(m_hwndScintilla, SCI_SETMODEVENTMASK, SC_MOD_INSERTTEXT, 0);
   // AndyS 17/9/15 - Turn On & Set Dwell to 1 sec
   SendMessage(m_hwndScintilla,SCI_SETMOUSEDWELLTIME,1000,0);

   // The null visibility policy is like Visual Studio - if a search goes
   // off the screen, the newly selected text is placed in the middle of the
   // screen
   SendMessage(m_hwndScintilla, SCI_SETVISIBLEPOLICY, 0, 0);

   SendMessage(m_hwndScintilla, SCI_SETMARGINTYPEN, 0, SC_MARGIN_NUMBER);
   SendMessage(m_hwndScintilla, SCI_SETMARGINSENSITIVEN, 1, 1);
   SendMessage(m_hwndScintilla, SCI_SETMARGINWIDTHN, 0, 40);

   SendMessage(m_hwndScintilla, SCI_SETCARETLINEVISIBLE, 1, 0);
   SendMessage(m_hwndScintilla, SCI_SETCARETLINEBACK, RGB(240, 240, 255), 0);

   SendMessage(m_hwndScintilla, SCI_INDICSETSTYLE, 0, INDIC_ROUNDBOX);
   SendMessage(m_hwndScintilla, SCI_SETINDICATORCURRENT, 0, 0);
   SendMessage(m_hwndScintilla, SCI_INDICSETFORE, 0, RGB(255, 0, 0));
   SendMessage(m_hwndScintilla, SCI_INDICSETALPHA, 0, 90);

   SendMessage(m_hwndScintilla, SCI_SETPROPERTY, (WPARAM)"fold", (LPARAM)"1");
   SendMessage(m_hwndScintilla, SCI_SETPROPERTY, (WPARAM)"fold.compact", (LPARAM)"0");
   SendMessage(m_hwndScintilla, SCI_SETMARGINTYPEN, 1, SC_MARGIN_SYMBOL);
   SendMessage(m_hwndScintilla, SCI_SETMARGINMASKN, 1, SC_MASK_FOLDERS);
   SendMessage(m_hwndScintilla, SCI_SETMARGINWIDTHN, 1, 20);

   SendMessage(m_hwndScintilla, SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPEN, SC_MARK_MINUS);
   SendMessage(m_hwndScintilla, SCI_MARKERSETFORE, SC_MARKNUM_FOLDEROPEN, RGB(0, 0, 0));
   SendMessage(m_hwndScintilla, SCI_MARKERSETBACK, SC_MARKNUM_FOLDEROPEN, RGB(255, 255, 255));

   SendMessage(m_hwndScintilla, SCI_MARKERDEFINE, SC_MARKNUM_FOLDER, SC_MARK_PLUS);
   SendMessage(m_hwndScintilla, SCI_MARKERDEFINE, SC_MARKNUM_FOLDERSUB, SC_MARK_EMPTY);
   SendMessage(m_hwndScintilla, SCI_MARKERDEFINE, SC_MARKNUM_FOLDERTAIL, SC_MARK_EMPTY);
   SendMessage(m_hwndScintilla, SCI_MARKERDEFINE, SC_MARKNUM_FOLDEREND, SC_MARK_EMPTY);
   SendMessage(m_hwndScintilla, SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPENMID, SC_MARK_EMPTY);
   SendMessage(m_hwndScintilla, SCI_MARKERDEFINE, SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_EMPTY);

   SendMessage(m_hwndScintilla, SCI_STYLESETSIZE, 32, 10);
   SendMessage(m_hwndScintilla, SCI_STYLESETFONT, 32, (LPARAM)"Courier");
   SendMessage(m_hwndScintilla, SCI_STYLESETFORE, SCE_B_DEFAULT, RGB(255, 0, 0));
   SendMessage(m_hwndScintilla, SCI_STYLESETFORE, SCE_B_COMMENT, RGB(0, 130, 0));
   SendMessage(m_hwndScintilla, SCI_STYLESETFORE, SCE_B_NUMBER, RGB(0, 100, 100));
   SendMessage(m_hwndScintilla, SCI_STYLESETFORE, SCE_B_KEYWORD, RGB(0, 0, 160));
   SendMessage(m_hwndScintilla, SCI_STYLESETFORE, SCE_B_STRING, RGB(0, 100, 100));
   SendMessage(m_hwndScintilla, SCI_STYLESETFORE, SCE_B_PREPROCESSOR, RGB(255, 0, 0));
   SendMessage(m_hwndScintilla, SCI_STYLESETFORE, SCE_B_OPERATOR, RGB(0, 0, 160));
   SendMessage(m_hwndScintilla, SCI_STYLESETFORE, SCE_B_IDENTIFIER, RGB(0, 0, 0));
   SendMessage(m_hwndScintilla, SCI_STYLESETFORE, SCE_B_DATE, RGB(0, 0, 0));

	SendMessage(m_hwndScintilla, SCI_SETWORDCHARS, 0,(LPARAM) "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_");
	//SendMessage(m_hwndScintilla, SCI_SETWHITESPACECHARS,0,(LPARAM) ".");
	//SendMessage(m_hwndScintilla, SCI_SETPUNCTUATIONCHARS,0 ,(LPARAM)".");
	SendMessage(m_hwndScintilla, SCI_STYLESETFORE, SCE_B_KEYWORD2, RGB(128, 0, 128));
   SendMessage(m_hwndScintilla, SCI_STYLESETFORE, SCE_B_KEYWORD3, RGB(128, 128, 0));

	SendMessage(m_hwndScintilla, SCI_AUTOCSETIGNORECASE, TRUE, 0);
	SendMessage(m_hwndScintilla, SCI_AUTOCSETCASEINSENSITIVEBEHAVIOUR, SC_CASEINSENSITIVEBEHAVIOUR_IGNORECASE,0);

	SendMessage(m_hwndScintilla, SCI_AUTOCSETFILLUPS, 0,(LPARAM) "[]{}(). ");
	SendMessage(m_hwndScintilla, SCI_AUTOCSTOPS, 0,(LPARAM) " ");

	//////////////////////// Status Window (& Sizing Box)

   m_hwndStatus = CreateStatusWindow((WS_CHILD | WS_VISIBLE), "", m_hwndMain, 1);

   int foo[4] = { 120, 320, 350, 400 };
   SendMessage(m_hwndStatus, SB_SETPARTS, 4, (size_t)foo);

   /////////////////// Compile / Find Buttons

   /////////////////// Item / Event Lists

   m_hwndItemList = CreateWindowEx(0, "ComboBox", "Objects",
      WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_SORT | WS_VSCROLL,
      5, 17, 150, 400, m_hwndMain, NULL, g_hinst, 0);
   SetWindowLong(m_hwndItemList, GWL_ID, IDC_ITEMLIST);
   SendMessage(m_hwndItemList, WM_SETFONT, (size_t)GetStockObject(DEFAULT_GUI_FONT), 0);

   m_hwndItemText = CreateWindowEx(0, "Static", "ObjectsText",
		WS_CHILD | WS_VISIBLE | SS_SIMPLE, 5, 0, 150, 15, m_hwndMain, NULL, g_hinst, 0);
	SetWindowText(m_hwndItemText, "Table component:" );
   SendMessage(m_hwndItemText, WM_SETFONT, (size_t)GetStockObject(DEFAULT_GUI_FONT), 0);

	m_hwndEventList = CreateWindowEx(0, "ComboBox", "Events",
      WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_SORT | WS_VSCROLL,
      180 + 5, 17, 150, 400, m_hwndMain, NULL, g_hinst, 0);
   SetWindowLong(m_hwndEventList, GWL_ID, IDC_EVENTLIST);
   SendMessage(m_hwndEventList, WM_SETFONT, (size_t)GetStockObject(DEFAULT_GUI_FONT), 0);

   m_hwndEventText = CreateWindowEx(0, "Static", "EventsText",
		WS_CHILD | WS_VISIBLE | SS_SIMPLE, 180 + 5, 0, 150, 15, m_hwndMain, NULL, g_hinst, 0);
	SetWindowText(m_hwndEventText, "Create Sub from component:" );
   SendMessage(m_hwndEventText, WM_SETFONT, (size_t)GetStockObject(DEFAULT_GUI_FONT), 0);
	
	m_hwndFunctionList = CreateWindowEx(0, "ComboBox", "Functions",
      WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
      360 + 5, 17, 150, 400, m_hwndMain, NULL, g_hinst, 0);
   SetWindowLong(m_hwndFunctionList, GWL_ID, IDC_FUNCTIONLIST);
   SendMessage(m_hwndFunctionList, WM_SETFONT, (size_t)GetStockObject(DEFAULT_GUI_FONT), 0);

    m_hwndFunctionText = CreateWindowEx(0, "Static", "FunctionsText",
		WS_CHILD | WS_VISIBLE | SS_SIMPLE, 360 + 5, 0, 150, 15, m_hwndMain, NULL, g_hinst, 0);
	SetWindowText(m_hwndFunctionText, "Go to Sub/Function:" );
   SendMessage(m_hwndFunctionText, WM_SETFONT, (size_t)GetStockObject(DEFAULT_GUI_FONT), 0);

	SendMessage(m_hwndMain, WM_SIZE, 0, 0); // Make our window relay itself out
}

void CodeViewer::Destroy()
{
	if(g_VBwords)
	{
		g_VBwords->clear();
		delete g_VBwords;
	}
	if(g_UserFunc)
	{
		g_UserFunc->clear();
		delete g_UserFunc;
	}
   if (m_hwndFind)
      DestroyWindow(m_hwndFind);

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

   OLECHAR wszOutput[1024];
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

void CodeViewer::EvaluateScriptStatement(char *szScript)
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

void CodeViewer::AddToDebugOutput(char *szText)
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

void CodeViewer::Find(FINDREPLACE *pfr)
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

void CodeViewer::Replace(FINDREPLACE *pfr)
{
   const size_t selstart = SendMessage(m_hwndScintilla, SCI_GETSELECTIONSTART, 0, 0);
   const size_t selend = SendMessage(m_hwndScintilla, SCI_GETSELECTIONEND, 0, 0);

   const size_t len = SendMessage(m_hwndScintilla, SCI_GETTEXTLENGTH, 0, 0);

   FINDTEXTEX ft;
   ft.chrg.cpMax = len;			// search through end of the text
   ft.chrg.cpMin = selstart;
   if (!(pfr->Flags & (FR_REPLACE | FR_REPLACEALL)))
      ft.chrg.cpMin = selend;
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
      ft.chrg.cpMin = cpMatch + lstrlen(pfr->lpstrReplaceWith);
      ft.chrg.cpMax = len;	// search through end of the text
      cszReplaced++;
      goto next;
   }
}

void CodeViewer::SaveToStream(IStream *pistream, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
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


void CodeViewer::LoadFromStream(IStream *pistream, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   m_fIgnoreDirty = fTrue;

   ULONG read = 0;
   int cchar;
   pistream->Read(&cchar, sizeof(int), &read);

   BYTE * const szText = new BYTE[cchar + 1];

   pistream->Read(szText, (cchar)*sizeof(char), &read);

   CryptHashData(hcrypthash, (BYTE *)szText, cchar, 0);

   // if there is a valid key, then decrypt the script text (now in szText)
   //(must be done after the hash is updated)
   if (hcryptkey != NULL)
   {
      // get the size of the data to decrypt
      DWORD cryptlen = cchar*sizeof(char);

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
	//initalise autocomplete, tooltips etc. - AndyS
	ParseForFunction();


   m_fIgnoreDirty = fFalse;
   m_sdsDirty = eSaveClean;
}


void CodeViewer::ColorLine(int line)
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

void CodeViewer::ColorError(int line, int nchar)
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

void CodeViewer::GetParamsFromEvent(int iEvent, char *szParams)
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
   // AndyS Gain a few ticks!

   size_t codelen = SendMessage(m_hwndScintilla, SCI_GETTEXTLENGTH, 0, 0);
   size_t startChar = 0;
   size_t stopChar = codelen;
   //SendMessage(m_hwndScintilla, SCI_SETTARGETSTART, startChar, 0); 
   //SendMessage(m_hwndScintilla, SCI_SETTARGETEND, stopChar, 0);
   SendMessage(m_hwndScintilla, SCI_TARGETWHOLEDOCUMENT, 0, 0);
   //AndyS - Finish
   SendMessage(m_hwndScintilla, SCI_SETSEARCHFLAGS, SCFIND_WHOLEWORD, 0);

   size_t posFind;
   while ((posFind = SendMessage(m_hwndScintilla, SCI_SEARCHINTARGET, lstrlen(szItemName), (LPARAM)szItemName)) != -1)
   {
      const size_t line = SendMessage(m_hwndScintilla, SCI_LINEFROMPOSITION, posFind, 0);
      // Check for 'sub' and make sure we're not in a comment
      const size_t beginchar = SendMessage(m_hwndScintilla, SCI_POSITIONFROMLINE, line, 0);
      bool fGoodMatch = true;

      char szLine[1024];
      SOURCE_TEXT_ATTR wzFormat[1024];
      WCHAR wzText[1024];

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
      char szNewCode[1024];
      char szEnd[2];

      TEXTRANGE tr;
      tr.chrg.cpMax = codelen;
      tr.chrg.cpMin = codelen - 1;
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

      char szParams[1024];

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
//Ajs
void CodeViewer::ShowAutoComplete(SCNotification *pSCN)
{
	// 1. get current word
	int CurPos = SendMessage(m_hwndScintilla,SCI_GETCURRENTPOS,0,0);
	int wordstart = SendMessage(m_hwndScintilla,SCI_WORDSTARTPOSITION,CurPos, TRUE);
	int wordfinish = SendMessage(m_hwndScintilla,SCI_WORDENDPOSITION,CurPos, TRUE);
	// 2. if word length >3 start auto complete
	char pcWord[256] = {};
	GetRange(m_hwndScintilla,wordstart,wordfinish,pcWord);
	int intWordLen = strlen(pcWord);
	if (intWordLen > 3)
	{
		const char * McStr = g_AutoCompList.c_str();
		SendMessage(m_hwndScintilla,SCI_AUTOCSHOW,intWordLen,(LPARAM)McStr);
	}
}

bool CodeViewer::ShowTooltip(SCNotification *pSCN)
{
	//get word under pointer
	int dwellpos = pSCN->position;
	int wordstart = SendMessage(m_hwndScintilla,SCI_WORDSTARTPOSITION,dwellpos, FALSE );
	int wordfinish = SendMessage(m_hwndScintilla,SCI_WORDENDPOSITION,dwellpos, FALSE );
	char Mess[256] = {}; int MessLen = 0;
	char DwellWord[256] = {};
	// is it a valid 'word'
	if ((SendMessage(m_hwndScintilla, SCI_ISRANGEWORD, wordstart , wordfinish )) && ((wordfinish - wordstart) < 255))
	{
		//Retrieve the word
		GetRange(m_hwndScintilla,wordstart,wordfinish,DwellWord);
		szLower(DwellWord);
		// Serarch for VBS reserved words
		// ToDo: Should be able get some MS help for better descriptions
		const int RetIndex = vbsKeyWords.find( DwellWord );
		if (RetIndex != -1)
		{
			MessLen = sprintf_s(Mess, "VBS:%s", DwellWord);
		}

		// Search subs list
		if (MessLen == 0)
		{
			/// has function list been filled?
			if ( g_UserFunc->size() == 0 ) ParseForFunction();
			//now search
			int iTemp= 0;
			for (list<UserData>::iterator i = g_UserFunc->begin();i != g_UserFunc->end(); ++i) 
			{
				if (i->strKeyName == (string(DwellWord)))
				{
					const char *ptemp = (i->strDescription.c_str());
					if (*ptemp)
					{
						MessLen = sprintf_s(Mess, "%s", ptemp);
						iTemp = g_UserFunc->size(); 
					}
				}
			}
		}
		//ajs will be used!
		if (MessLen == 0 )
		{
			MessLen = sprintf_s(Mess, "Test:%s", DwellWord);
		}
	}
	if (MessLen > 0)
	{
		SendMessage(m_hwndScintilla,SCI_CALLTIPSHOW,dwellpos, (size_t)Mess );
		return true;
	}
	return false;
}

void CodeViewer::MarginClick(int position, int modifiers)
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

void AddComment(HWND m_hwndScintilla)
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



void RemoveComment(HWND m_hwndScintilla)
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
      char buf[1024];
      if (lineEnd - lineStart < 1023)
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

void CodeViewer::szLower(char * incstr)
{
	char *pC = incstr;
	while (*pC)
	{
		if (*pC >= 'A' && *pC <= 'Z')
			*pC = *pC + ('a' - 'A');
		pC++;
	}
}

void CodeViewer::szUpper(char * incstr)
{
	char *pC = incstr;
	while (*pC)
	{
		if (*pC >= 'a' && *pC <= 'z')
			*pC = *pC - ('a' - 'A' );
		pC++;
	}
}
void CodeViewer::ParseForFunction() // & Subs & Collections AndyS - WIP - Totally overhaul of this!
{
   char text[1024];
   size_t scriptLines = SendMessage(m_hwndScintilla, SCI_GETLINECOUNT, 0, 0);
   SendMessage(m_hwndFunctionList, CB_RESETCONTENT, 0, 0);
	char szValidChars[256] = {};  //AndyS - 191 valid from UK locale 
   SendMessage(m_hwndScintilla, SCI_GETWORDCHARS, 0, (LPARAM)szValidChars);//Lparam or size_t ?????
   string ValidChars(szValidChars);
	//g_UserFunc->clear(); //is refilled below
	string str ="";
	for (size_t i = 0; i < scriptLines; ++i) // i i captin'
   {
		
      const size_t lineLength = SendMessage(m_hwndScintilla, SCI_LINELENGTH, i, 0);
      if (lineLength > 1023 || lineLength < 4) 
			continue;
      memset(text, 0, 1024);
		SendMessage(m_hwndScintilla, SCI_GETLINE, i, (LPARAM)text);
      string line(text);
		size_t idx = line.find(":"); 
		if (idx > 0)
		{
			line = line.substr(0,idx);
		}
		int SearchLength =3;
      idx = upperCase(line).find("SUB");
		if (idx == -1)
		{
			idx = upperCase(line).find("FUNCTION");
			SearchLength =8;
		}
      if ((SSIZE_T)idx >= 0)
      {
         const size_t endIdx = upperCase(line).find("END");
         const size_t exitIdx = upperCase(line).find("EXIT");
         const size_t commentIdx = upperCase(line).find("'");
         const size_t doubleQuoteIdx = upperCase(line).find("\""); //AndyS
         if (endIdx == -1 && exitIdx == -1)
         {
				if ((commentIdx >= 0)  && (commentIdx < idx)) continue;
				if ((doubleQuoteIdx >= 0)  && (doubleQuoteIdx < idx)) continue;//AndyS - combine?
				string sSubName;
				size_t Substart = idx + SearchLength;
				char linechar = 0;
				//scan for first valid char of sub name (should loop at least once)
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
				sSubName = line.substr(Substart,Subfinish-Substart);
				const UserData ud(i ,line.substr( 0, line.length()-2 ) , sSubName );
				FindOrInsertUD( g_UserFunc , ud );
         }
      }
	}
   //Propergate subs&funcs in menu in order
	for (list<UserData>::iterator i = g_UserFunc->begin(); i != g_UserFunc->end(); ++i) 
   {
		const char *c_str1 = i->strKeyName.c_str ();
		SendMessage(m_hwndFunctionList, CB_ADDSTRING, 0, (LPARAM)(c_str1) );
   }
	//Collect Objects/Components from the menu.
	int CBCount;
	CBCount = SendMessage(m_hwndItemList, CB_GETCOUNT, 0, 0)-1;//Zero Based
	char c_str1[256]={0};
	UserData ud;
	while (CBCount >= 0) 
   {
		memset(c_str1,0,256);
		SendMessage(m_hwndItemList, CB_GETLBTEXT, CBCount, (LPARAM)c_str1);
		if(strlen(c_str1)>1)
		{
			ud.strKeyName = string(c_str1);
			FindOrInsertUD(g_Components,ud);
		}
		CBCount--;
   }
	//Now merge the lot for Auto complete...
	g_AutoComp->clear();
	string strCompOut ="";
	for (list<UserData>::iterator i = g_Components->begin(); i != g_Components->end(); ++i)
	{
		FindOrInsertStringIntoAutolist(g_AutoComp,i->strKeyName);
		strCompOut.append(i->strKeyName);
		strCompOut.append(" ");
	}
	for (list<UserData>::iterator i = g_VBwords->begin(); i != g_VBwords->end(); ++i)
	{
		FindOrInsertStringIntoAutolist(g_AutoComp,i->strKeyName);
	}
	string sSubFunOut = "";
	for (list<UserData>::iterator i = g_UserFunc->begin(); i != g_UserFunc->end(); ++i)
	{
		FindOrInsertStringIntoAutolist(g_AutoComp,i->strKeyName);
		sSubFunOut.append(i->strKeyName);
		sSubFunOut.append(" ");
	}
	g_AutoCompList = "";
	for (list<string>::iterator i = g_AutoComp->begin(); i != g_AutoComp->end();++i)
	{
		g_AutoCompList.append(i->data());
		g_AutoCompList += " ";
	}
   //Send the collected func/subs to scintilla for highlighting - always lowercase as VBS is case insensitive.
	sSubFunOut = lowerCase(sSubFunOut);
	const char * strOut = sSubFunOut.c_str();
	SendMessage(m_hwndScintilla, SCI_SETKEYWORDS, 1, (LPARAM)strOut);
	//Send Components to Scintilla for highlighting
	strCompOut = lowerCase(strCompOut);
	strOut = strCompOut.c_str();
	SendMessage(m_hwndScintilla, SCI_SETKEYWORDS, 2 , (LPARAM)strOut);
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

   switch (uMsg)
   {
   case WM_DESTROY:
   {
   }
   break;

   case WM_ACTIVATE:
   {
      if (LOWORD(wParam) != WA_INACTIVE)
         g_pvp->m_pcv = GetCodeViewerPtr(hwndDlg);
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
      //HWND hwndControl = (HWND)lParam;

      switch (code)
      {

      case SCEN_CHANGE:
      {
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
         {
            AddComment(pcv->m_hwndScintilla);
            break;
         }
         case ID_REMOVE_COMMENT:
         {
            RemoveComment(pcv->m_hwndScintilla);
            break;
         }
         }
      }
      break;
      case CBN_SETFOCUS:
      {
         if (id == IDC_FUNCTIONLIST)
			{
	        CodeViewer * const pcv = GetCodeViewerPtr(hwndDlg);
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
            const size_t index = SendMessage(pcv->m_hwndFunctionList, CB_GETCURSEL, 0, 0);
				if (index != -1)
				{
					list<UserData>::iterator i = g_UserFunc->begin();
					for (size_t t = 0; t < index; t++)	++i;
					SendMessage(pcv->m_hwndScintilla, SCI_GOTOLINE, i->intLineNum, 0);
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
         }
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

      switch (code)
      {

      case SCN_SAVEPOINTREACHED:
      {
         CodeViewer *pcv = GetCodeViewerPtr(hwndDlg);
         if (pcv->m_sdsDirty > eSaveClean)
         {
            pcv->m_sdsDirty = eSaveClean;
            pcv->m_psh->SetDirtyScript(eSaveClean);
         }
      }
      break;
	  case SCN_DWELLSTART:
		{
			SCNotification * const pscn = (SCNotification *)lParam;
         CodeViewer *pcv = GetCodeViewerPtr(hwndDlg);
			g_ToolTipActive = pcv->ShowTooltip(pscn);
		} break;

	  case SCN_DWELLEND:
		{
			if (g_ToolTipActive)
			{
				CodeViewer *pcv = GetCodeViewerPtr(hwndDlg);
				SendMessage(pcv->m_hwndScintilla,SCI_CALLTIPCANCEL , 0, 0 );
				g_ToolTipActive = false;
			}
		} break;

		case SCN_CHARADDED:
		{
			CodeViewer *pcv = GetCodeViewerPtr(hwndDlg);
			SCNotification * const pSCN = (SCNotification *)lParam;
			pcv->ShowAutoComplete(pSCN);
		}
		break;
	  case SCN_UPDATEUI:
      {
         SCNotification * const pscn = (SCNotification *)lParam;

         char szT[256];
         CodeViewer * const pcv = GetCodeViewerPtr(hwndDlg);
         size_t pos = SendMessage(hwndRE, SCI_GETCURRENTPOS, 0, 0);
         const size_t line = SendMessage(hwndRE, SCI_LINEFROMPOSITION, pos, 0) + 1;
         const size_t column = SendMessage(hwndRE, SCI_GETCOLUMN, pos, 0);

         sprintf_s(szT, "Line %u, Col %u", (U32)line, (U32)column);
         SendMessage(pcv->m_hwndStatus, SB_SETTEXT, 0 | 0, (size_t)szT);
      }
      break;
      case SCN_MARGINCLICK:
      {
         SCNotification * const pscn = (SCNotification *)lParam;
         CodeViewer * const pcv = GetCodeViewerPtr(hwndDlg);
         if (pscn->margin == 1)
         {
            pcv->MarginClick(pscn->position, pscn->modifiers);
         }
      }
      break;

      }

      break;
   }

   case RECOLOR_LINE:
   {
      CodeViewer * const pcv = GetCodeViewerPtr(hwndDlg);

      for (size_t i = wParam; i <= (size_t)lParam; ++i)
         pcv->ColorLine(i);
   }
   break;

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

Collection::Collection()
{
   m_fFireEvents = fFalse;
   m_fStopSingleEvents = fFalse;
   m_fGroupElements = fTrue;
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

void DebuggerModule::Init(CodeViewer *pcv)
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
