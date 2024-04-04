#include "stdafx.h"
#include "codeviewedit.h"

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

////////////////Preferences
CVPreference::CVPreference(
		const COLORREF crTextColor,
		const bool bDisplay, const string& szRegistryName,
		const int szScintillaKeyword, const int IDC_ChkBox,
		const int IDC_ColorBut, const int IDC_Font)
   : m_rgb(crTextColor),
     m_sciKeywordID(szScintillaKeyword),
     IDC_ChkBox_code(IDC_ChkBox),
     IDC_ColorBut_code(IDC_ColorBut),
     IDC_Font_code(IDC_Font),
     m_szRegName(szRegistryName),
     m_highlight(bDisplay)
{
	memset(&m_logFont, 0, sizeof(LOGFONT));
}

void CVPreference::SetCheckBox(const HWND hwndDlg)
{
#ifndef __STANDALONE__
	SNDMSG(GetDlgItem(hwndDlg, IDC_ChkBox_code), BM_SETCHECK, m_highlight ? BST_CHECKED : BST_UNCHECKED, 0L);
#endif
}

void CVPreference::ReadCheckBox(const HWND hwndDlg)
{
#ifndef __STANDALONE__
	m_highlight = !!IsDlgButtonChecked(hwndDlg, IDC_ChkBox_code);
#endif
}

void CVPreference::GetPrefsFromReg()
{
	m_highlight = g_pvp->m_settings.LoadValueWithDefault(Settings::CVEdit, m_szRegName, m_highlight);
	m_rgb = g_pvp->m_settings.LoadValueWithDefault(Settings::CVEdit, m_szRegName + "_color", (int)m_rgb);
	m_pointSize = g_pvp->m_settings.LoadValueWithDefault(Settings::CVEdit, m_szRegName + "_FontPointSize", m_pointSize);

	char bakupFaceName[LF_FACESIZE]; // to save the default font name, in case the corresponding registry entry is empty
	strncpy_s(bakupFaceName, m_logFont.lfFaceName, sizeof(bakupFaceName)-1);
	if (!g_pvp->m_settings.LoadValue(Settings::CVEdit, m_szRegName + "_Font", m_logFont.lfFaceName, LF_FACESIZE))
		strncpy_s(m_logFont.lfFaceName, bakupFaceName, sizeof(m_logFont.lfFaceName)-1);

	m_logFont.lfWeight = g_pvp->m_settings.LoadValueWithDefault(Settings::CVEdit, m_szRegName + "_FontWeight", (int)m_logFont.lfWeight);
	m_logFont.lfItalic = g_pvp->m_settings.LoadValueWithDefault(Settings::CVEdit, m_szRegName + "_FontItalic", m_logFont.lfItalic);
	m_logFont.lfUnderline = g_pvp->m_settings.LoadValueWithDefault(Settings::CVEdit, m_szRegName + "_FontUnderline", m_logFont.lfUnderline);
	m_logFont.lfStrikeOut = g_pvp->m_settings.LoadValueWithDefault(Settings::CVEdit, m_szRegName + "_FontStrike", m_logFont.lfStrikeOut);
}

void CVPreference::SetPrefsToReg()
{
	g_pvp->m_settings.SaveValue(Settings::CVEdit, m_szRegName, m_highlight);
	g_pvp->m_settings.SaveValue(Settings::CVEdit, m_szRegName + "_color", (int)m_rgb);
	g_pvp->m_settings.SaveValue(Settings::CVEdit, m_szRegName + "_FontPointSize", m_pointSize);
	g_pvp->m_settings.SaveValue(Settings::CVEdit, m_szRegName + "_Font", m_logFont.lfFaceName);
	g_pvp->m_settings.SaveValue(Settings::CVEdit, m_szRegName + "_FontWeight", (int)m_logFont.lfWeight);
	g_pvp->m_settings.SaveValue(Settings::CVEdit, m_szRegName + "_FontItalic", m_logFont.lfItalic);
	g_pvp->m_settings.SaveValue(Settings::CVEdit, m_szRegName + "_FontUnderline", m_logFont.lfUnderline);
	g_pvp->m_settings.SaveValue(Settings::CVEdit, m_szRegName + "_FontStrike", m_logFont.lfStrikeOut);
}

void CVPreference::SetDefaultFont(const HWND hwndDlg)
{
#ifndef __STANDALONE__
	LOGFONT* const plfont = &m_logFont;
	memset(plfont, 0, sizeof(LOGFONT));
	HFONT hFont = (HFONT)GetStockObject(ANSI_FIXED_FONT);
	if (hFont == nullptr)
		hFont = (HFONT)GetStockObject(SYSTEM_FONT);
	GetObject(hFont, sizeof(LOGFONT), plfont);
	m_pointSize = 10;
	GetHeightFromPointSize(hwndDlg);
#endif
}

int CVPreference::GetHeightFromPointSize(const HWND hwndDlg)
{
#ifndef __STANDALONE__
	const CClientDC clientDC(hwndDlg);
	const int Height = -MulDiv(m_pointSize, clientDC.GetDeviceCaps(LOGPIXELSY), 72);
	return Height;
#else
	return 0;
#endif
}

void CVPreference::ApplyPreferences(const HWND hwndScin, const CVPreference* DefaultPref)
{
#ifndef __STANDALONE__
	const int id = m_sciKeywordID;
	const bool HL = m_highlight;
	SendMessage(hwndScin, SCI_STYLESETFORE,      id, HL ? (LPARAM)m_rgb : (LPARAM)DefaultPref->m_rgb);
	SendMessage(hwndScin, SCI_STYLESETFONT,      id, HL ? (LPARAM)m_logFont.lfFaceName : (LPARAM)DefaultPref->m_logFont.lfFaceName);
	SendMessage(hwndScin, SCI_STYLESETSIZE,      id, HL ? (LPARAM)m_pointSize : (LPARAM)DefaultPref->m_pointSize);
	SendMessage(hwndScin, SCI_STYLESETWEIGHT,    id, HL ? (LPARAM)m_logFont.lfWeight : (LPARAM)DefaultPref->m_logFont.lfWeight);
	SendMessage(hwndScin, SCI_STYLESETITALIC,    id, HL ? (LPARAM)m_logFont.lfItalic : (LPARAM)DefaultPref->m_logFont.lfItalic);
	SendMessage(hwndScin, SCI_STYLESETUNDERLINE, id, HL ? (LPARAM)m_logFont.lfUnderline : (LPARAM)DefaultPref->m_logFont.lfUnderline);
	// There is no strike through in Scintilla (yet!)
#endif
}
