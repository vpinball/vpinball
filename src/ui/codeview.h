#pragma once

#include <commdlg.h>
#include <atlcom.h>
#include "codeviewedit.h"

#ifndef __STANDALONE__
#include "ui/dialogs/ScriptErrorDialog.h"
#include "scintilla.h"
#endif

#include "core/ScriptInterpreter.h"

#ifndef OVERRIDE
#ifndef __STANDALONE__
   #define OVERRIDE override
#else
   #define OVERRIDE
#endif
#endif

#define MAX_FIND_LENGTH 81 // from MS docs: The buffer should be at least 80 characters long (for find/replace)

class CodeViewer : public CWnd
{
public:
   class IScriptableHost
   {
   public:
      virtual void SelectItem(IScriptable *piscript) = 0;
      virtual void SetDirtyScript(SaveDirtyState sds) = 0;
      virtual void DoCodeViewCommand(int command) = 0;
   };

   CodeViewer(IScriptableHost *psh);
   ~CodeViewer() OVERRIDE;

   void SetVisible(const bool visible);

   void SetEnabled(const bool enabled);

   void SetClean(const SaveDirtyState sds);

   HRESULT AddItem(IScriptable * const piscript, const bool global);
   void RemoveItem(IScriptable * const piscript);
   HRESULT ReplaceName(IScriptable * const piscript, const wstring& wzNew);
   void SelectItem(IScriptable * const piscript);

   void Compile(PinTable *table, const bool message);

   void OnScriptError(ScriptInterpreter::ErrorType type, int line, int column, const string &description, const vector<string> &stackDump);

   void UncolorError();
   void ParseForFunction();

   void ShowFindDialog();
   void ShowFindReplaceDialog();
   void Find(const FINDREPLACE * const pfr);
   void Replace(const FINDREPLACE * const pfr);
   void SaveToStream(IStream *pistream, HCRYPTHASH const hcrypthash);
   void LoadFromStream(IStream *pistream, HCRYPTHASH const hcrypthash, const HCRYPTKEY hcryptkey); // incl. table protection
   void LoadFromFile(const string& filename);
   void SetCaption(const string& szCaption);

   bool ShowTooltipOrGoToDefinition(const SCNotification *pSCN, const bool tooltip);
   void ShowAutoComplete(const SCNotification *pSCN);

   void UpdateRegWithPrefs();
   void UpdatePrefsfromReg();

   size_t GetWordUnderCaret(char *buf);

   void ListEventsFromItem();
   void FindCodeFromEvent();
   void TellHostToSelectItem();

   void UpdateScinFromPrefs();

#ifndef __STANDALONE__
   void MarginClick(const Sci_Position position, const int modifiers);
#endif

   void AddToDebugOutput(const string& szText);

   BOOL PreTranslateMessage(MSG& msg) OVERRIDE;

   string GetScript() const;
   void SetScript(const string& script);

   IScriptableHost *m_psh;

   class CodeViewDispatch final
   {
   public:
      CodeViewDispatch() { }
      ~CodeViewDispatch() { }

      wstring m_wName;
      IUnknown *m_punk = nullptr;
      IDispatch *m_pdisp = nullptr;
      bool m_global = false;
   };

   VectorSortString<CodeViewDispatch *> m_vcvd;

   COLORREF m_prefCols[16];
   COLORREF m_bgColor;
   COLORREF m_bgSelColor;
   CVPreference *m_prefEverythingElse;
   vector<CVPreference*> *m_lPrefsList;

   int m_displayAutoCompleteLength;

   SaveDirtyState m_sdsDirty = eSaveClean;
   bool m_ignoreDirty = false;

   bool m_warn_on_dupes = false;

   bool m_visible = false;
   bool m_minimized = false;

   bool m_displayAutoComplete;
   bool m_toolTipActive;
   bool m_stopErrorDisplay;

   bool m_dwellHelp;
   bool m_dwellDisplay;
   int m_dwellDisplayTime;

   fi_vector<UserData> m_pageConstructsDict;
#ifndef __STANDALONE__
   Sci_TextRange m_wordUnderCaret;
#endif

   HWND m_hwndMain = nullptr;
   HWND m_hwndScintilla = nullptr;
   HWND m_hwndFind = nullptr;
   HWND m_hwndStatus = nullptr;
   HWND m_hwndFunctionList = nullptr;

   HACCEL m_haccel = nullptr; // Accelerator keys

   int m_errorLineNumber = -1;

   FINDREPLACE m_findreplaceold; // the last thing found/replaced

   string external_script_name;  // loaded from external .vbs?
   vector<char> original_table_script; // if yes, then this one stores the original table script

   // otherwise Scintilla owns the text
   string m_script_text;

protected:
   void PreCreate(CREATESTRUCT& cs) final;
   void PreRegisterClass(WNDCLASS& wc) final;
   int  OnCreate(CREATESTRUCT& cs) final;
   LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam) final;
   BOOL OnCommand(WPARAM wparam, LPARAM lparam) final;
   LRESULT OnNotify(WPARAM wparam, LPARAM lparam) final;
   void Destroy() final;

private:
   CodeViewer* GetCodeViewerPtr();
   BOOL ParseClickEvents(const int id, const SCNotification *pSCN);
   BOOL ParseSelChangeEvent(const int id, const SCNotification *pSCN);

   string ParseDelimtByColon(string &wholeline);
   void ParseFindConstruct(size_t &Pos, const string &UCLine, WordType &Type, int &ConstructSize);
   bool ParseStructureName(fi_vector<UserData> &ListIn, const UserData &ud, const string &UCline, const string &line, const int Lineno);

   size_t SureFind(const string &LineIn, const string &ToFind);
   void RemoveByVal(string &line); 
   void RemoveNonVBSChars(string &line);
   string ExtractWordOperand(const string &line, const size_t StartPos) const;

   void ColorLine(const int line);
   void ColorError(const int line, const int nchar);

   void ParseVPCore();

   void ReadLineToParseBrain(string wholeline, const int linecount, fi_vector<UserData> &ListIn);

   void GetMembers(const fi_vector<UserData> &ListIn, const string &StrIn);

   void InitPreferences();

   string GetParamsFromEvent(const UINT iEvent);

   /**
    * Resizes the Scintilla widget (the text editor) and the last error widget (if it's visible)
    * 
    * This is called when the window is resized (when we get a WM_SIZE message)
    * or when the last error widget is toggled (since that appears below the text editor)
    */
   void ResizeScintillaAndLastError();

   /**
    * Sets the visibility of the last error information, shown below the Scintilla text editor.
    */
   void SetLastErrorVisibility(bool show);
   void SetLastErrorTextW(const LPCWSTR text);
   void AppendLastErrorTextW(const wstring& text);

   FINDREPLACE m_findreplacestruct;
   char szFindString[MAX_FIND_LENGTH];
   char szReplaceString[MAX_FIND_LENGTH];
   char szCaretTextBuff[MAX_FIND_LENGTH];

#ifndef __STANDALONE__
   UINT m_findMsgString; // Windows message for the FindText dialog
#endif

   string m_validChars;

   // CodeViewer Preferences
   CVPreference *prefDefault = nullptr;
   CVPreference *prefVBS = nullptr;
   CVPreference *prefComps = nullptr;
   CVPreference *prefSubs = nullptr;
   CVPreference *prefComments = nullptr;
   CVPreference *prefLiterals = nullptr;
   CVPreference *prefVPcore = nullptr;

   int m_parentLevel = 0;
   string m_currentParentKey; // always lower case
   //bool m_parentTreeInvalid;
   //TODO: int TabStop;

   // Dictionaries
   fi_vector<UserData> m_VBwordsDict;
   fi_vector<UserData> m_componentsDict;
   fi_vector<UserData> m_VPcoreDict;
   fi_vector<UserData> m_currentMembers;
   string m_autoCompString;
   string m_autoCompMembersString;

   HWND m_hwndItemList;
   HWND m_hwndItemText;
   HWND m_hwndEventList;
   HWND m_hwndEventText;
   HWND m_hwndFunctionText;

   /**
    * Whether the last error widget is visible
    */
   bool m_lastErrorWidgetVisible = false;

   /**
    * If true, error dialogs will be suppressed for the play session
    * 
    * This gets reset to false whenever the script is started
    */
   bool m_suppressErrorDialogs = false;

   /**
    * Handle for the last error widget
    * 
    * The last error widget is a read-only text area that appears below the Scintilla text editor, with the contents of
    * the last reported compile or runtime error.
    */
   HWND m_hwndLastErrorTextArea;
};
