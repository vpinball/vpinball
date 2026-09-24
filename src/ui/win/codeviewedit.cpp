// license:GPLv3+

#include "core/stdafx.h"
#include "codeviewedit.h"

#include "codeview.h"
#include "core/VPApp.h"

UserData::UserData()
   : m_lineNum(0),
     eTyping(eUnknown)
{
}

UserData::UserData(const int LineNo, const string &Desc, const string &Name, const WordType TypeIn)
   : m_lineNum(LineNo),
     m_keyName(Name),
     eTyping(TypeIn),
     m_description(Desc)
{
}

// CodeViewer Preferences
CVPreference::CVPreference(const COLORREF crTextColor, const bool bDisplay, const string& registryName,
                           const int szScintillaKeyword, const int IDC_ChkBox, const int IDC_ColorBut, const int IDC_Font)
   : m_rgb(crTextColor),
     m_sciKeywordID(szScintillaKeyword),
     IDC_ChkBox_code(IDC_ChkBox),
     IDC_ColorBut_code(IDC_ColorBut),
     IDC_Font_code(IDC_Font),
     m_regName(registryName),
     m_highlight(bDisplay)
{
}

void CVPreference::SetCheckBox(const HWND hwndDlg)
{
   SNDMSG(GetDlgItem(hwndDlg, IDC_ChkBox_code), BM_SETCHECK, m_highlight ? BST_CHECKED : BST_UNCHECKED, 0L);
}

void CVPreference::ReadCheckBox(const HWND hwndDlg)
{
   m_highlight = !!IsDlgButtonChecked(hwndDlg, IDC_ChkBox_code);
}

void CVPreference::GetPrefsFromReg()
{
   m_highlight = g_settingsService.GetAppSettings().GetBool(Settings::GetRegistry().GetPropertyId("CVEdit"s, m_regName).value());
   m_rgb = g_settingsService.GetAppSettings().GetInt(Settings::GetRegistry().GetPropertyId("CVEdit"s, m_regName + "_color").value());
   m_pointSize = g_settingsService.GetAppSettings().GetInt(Settings::GetRegistry().GetPropertyId("CVEdit"s, m_regName + "_FontPointSize").value());
   string tmp = g_settingsService.GetAppSettings().GetString(Settings::GetRegistry().GetPropertyId("CVEdit"s, m_regName + "_Font").value());
   strncpy_s(m_logFont.lfFaceName, std::size(m_logFont.lfFaceName), tmp.c_str());
   m_logFont.lfWeight = g_settingsService.GetAppSettings().GetInt(Settings::GetRegistry().GetPropertyId("CVEdit"s, m_regName + "_FontWeight").value());
   m_logFont.lfItalic = g_settingsService.GetAppSettings().GetBool(Settings::GetRegistry().GetPropertyId("CVEdit"s, m_regName + "_FontItalic").value());
   m_logFont.lfUnderline = g_settingsService.GetAppSettings().GetBool(Settings::GetRegistry().GetPropertyId("CVEdit"s, m_regName + "_FontUnderline").value());
   m_logFont.lfStrikeOut = g_settingsService.GetAppSettings().GetBool(Settings::GetRegistry().GetPropertyId("CVEdit"s, m_regName + "_FontStrike").value());
}

void CVPreference::SetPrefsToReg()
{
   g_settingsService.GetAppSettings().Set(Settings::GetRegistry().GetPropertyId("CVEdit"s, m_regName).value(), m_highlight, false);
   g_settingsService.GetAppSettings().Set(Settings::GetRegistry().GetPropertyId("CVEdit"s, m_regName + "_color").value(), (int)m_rgb, false);
   g_settingsService.GetAppSettings().Set(Settings::GetRegistry().GetPropertyId("CVEdit"s, m_regName + "_FontPointSize").value(), m_pointSize, false);
   g_settingsService.GetAppSettings().Set(Settings::GetRegistry().GetPropertyId("CVEdit"s, m_regName + "_Font").value(), string(m_logFont.lfFaceName), false);
   g_settingsService.GetAppSettings().Set(Settings::GetRegistry().GetPropertyId("CVEdit"s, m_regName + "_FontWeight").value(), (int)m_logFont.lfWeight, false);
   g_settingsService.GetAppSettings().Set(Settings::GetRegistry().GetPropertyId("CVEdit"s, m_regName + "_FontItalic").value(), m_logFont.lfItalic, false);
   g_settingsService.GetAppSettings().Set(Settings::GetRegistry().GetPropertyId("CVEdit"s, m_regName + "_FontUnderline").value(), m_logFont.lfUnderline, false);
   g_settingsService.GetAppSettings().Set(Settings::GetRegistry().GetPropertyId("CVEdit"s, m_regName + "_FontStrike").value(), m_logFont.lfStrikeOut, false);
}

void CVPreference::SetDefaultFont(const HWND hwndDlg)
{
	LOGFONT* const plfont = &m_logFont;
	memset(plfont, 0, sizeof(LOGFONT));
	HFONT hFont = (HFONT)GetStockObject(ANSI_FIXED_FONT);
	if (hFont == nullptr)
		hFont = (HFONT)GetStockObject(SYSTEM_FONT);
	GetObject(hFont, sizeof(LOGFONT), plfont);
	m_pointSize = 10;
	GetHeightFromPointSize(hwndDlg);
}

int CVPreference::GetHeightFromPointSize(const HWND hwndDlg)
{
	const CClientDC clientDC(hwndDlg);
	const int Height = -MulDiv(m_pointSize, clientDC.GetDeviceCaps(LOGPIXELSY), 72);
	return Height;
}

void CVPreference::ApplyPreferences(const HWND hwndScin, const CVPreference* DefaultPref)
{
	const int id = m_sciKeywordID;
	const bool HL = m_highlight;
	SendMessage(hwndScin, SCI_STYLESETFORE,      id, HL ? (LPARAM)m_rgb : (LPARAM)DefaultPref->m_rgb);
	SendMessage(hwndScin, SCI_STYLESETFONT,      id, HL ? (LPARAM)m_logFont.lfFaceName : (LPARAM)DefaultPref->m_logFont.lfFaceName);
	SendMessage(hwndScin, SCI_STYLESETSIZE,      id, HL ? (LPARAM)m_pointSize : (LPARAM)DefaultPref->m_pointSize);
	SendMessage(hwndScin, SCI_STYLESETWEIGHT,    id, HL ? (LPARAM)m_logFont.lfWeight : (LPARAM)DefaultPref->m_logFont.lfWeight);
	SendMessage(hwndScin, SCI_STYLESETITALIC,    id, HL ? (LPARAM)m_logFont.lfItalic : (LPARAM)DefaultPref->m_logFont.lfItalic);
	SendMessage(hwndScin, SCI_STYLESETUNDERLINE, id, HL ? (LPARAM)m_logFont.lfUnderline : (LPARAM)DefaultPref->m_logFont.lfUnderline);
	// There is no strike through in Scintilla (yet!)
}
