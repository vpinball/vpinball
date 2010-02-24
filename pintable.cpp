// PinTable.cpp: implementation of the PinTable class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "main.h"
#include "buildnumber.h"

#define HASHLENGTH 16

const unsigned char TABLE_KEY[] = "Visual Pinball";
const unsigned char PARAPHRASE_KEY[] = { 0xB4, 0x0B, 0xBE, 0x37, 0xC3, 0x0C, 0x8E, 0xA1, 0x5A, 0x05, 0xDF, 0x1B, 0x2D, 0x02, 0xEF, 0x8D };

int CALLBACK ProgressProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK TableWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#define TIMER_ID_AUTOSAVE 12345

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void ScriptGlobalTable::Init(PinTable *pt)
	{
	m_pt = pt;

	// initialise the sound sequencer
	SeqSoundInit();
	}

STDMETHODIMP ScriptGlobalTable::Nudge(float Angle, float Force)
{
	if (g_pplayer && (g_pplayer->m_nudgetime == 0))
		{
		float sn,cs;
  		sn = (float)sin(ANGTORAD(Angle));
		cs = (float)cos(ANGTORAD(Angle));
		g_pplayer->m_NudgeX = -sn*Force;
		g_pplayer->m_NudgeY = cs*Force;
		g_pplayer->m_NudgeBackX = -g_pplayer->m_NudgeX;
		g_pplayer->m_NudgeBackY = -g_pplayer->m_NudgeY;
		g_pplayer->m_nudgetime = 10;
		}

	return S_OK;
}

STDMETHODIMP ScriptGlobalTable::PlaySound(BSTR bstr, long LoopCount, float volume)
{
	if (!g_pplayer || !g_pplayer->m_fPlaySound)
		{
		return S_OK;
		}

	m_pt->PlaySound(bstr, LoopCount, volume);

	return S_OK;
}

STDMETHODIMP ScriptGlobalTable::StopSound(BSTR Sound)
{
	if (!g_pplayer || !g_pplayer->m_fPlaySound)
		{
		return S_OK;
		}

	m_pt->StopSound(Sound);

	return S_OK;
}

STDMETHODIMP ScriptGlobalTable::PlayMusic(BSTR str)
{
	if (g_pplayer && g_pplayer->m_fPlayMusic)
		{
		if (g_pplayer->m_pxap || g_pplayer->m_pcsimpleplayer)
			{
			EndMusic();
			}

		char szT[512];
		char szPath[MAX_PATH+512];

		WideCharToMultiByte(CP_ACP, 0, g_pvp->m_wzMyPath, -1, szPath, MAX_PATH, NULL, NULL);
		WideCharToMultiByte(CP_ACP, 0, str, -1, szT, 512, NULL, NULL);

		char szextension[MAX_PATH];
		ExtensionFromFilename(szT, szextension);

	//ppi->m_ppb;// = new PinBinary();

		if (!lstrcmpi(szextension, "wma"))
			{
			WCHAR *wz;
			wz = new WCHAR[MAX_PATH + lstrlenW(str) + 1];
			//WCHAR wz[MAX_PATH];

			//MultiByteToWideChar(CP_ACP, 0, szPath, -1, wz, MAX_PATH);

			//lstrcpyW(wz, g_pvp->m_wzMyPath);
			WideStrCopy(g_pvp->m_wzMyPath, wz);

			WideStrCat(L"Music\\", wz);
			WideStrCat(str, wz);
			//lstrcatW(wz, L"Music\\");
			//lstrcatW(wz, str);

			g_pplayer->m_pcsimpleplayer = new CSimplePlayer();
			HRESULT hr;
			hr = g_pplayer->m_pcsimpleplayer->Play(wz, g_pplayer->m_hSongCompletionEvent, &hr, g_pplayer->m_MusicVolume);
			if (hr != S_OK)
				{
				g_pplayer->m_pcsimpleplayer->Release();
				g_pplayer->m_pcsimpleplayer = NULL;
				}

			delete wz;
			}
		else // mp3
			{
			lstrcat(szPath, "Music\\");

			//WideCharToMultiByte(CP_ACP, 0, str, -1, szT, 512, NULL, NULL);

			// We know that szT can't be more than 512 characters as this point, and that szPath can't be more than MAX_PATH
			lstrcat(szPath, szT);

			g_pplayer->m_pxap = new XAudPlayer();

			if (!g_pplayer->m_pxap->Init(szPath, g_pplayer->m_MusicVolume))
				{
				delete g_pplayer->m_pxap;
				g_pplayer->m_pxap = NULL;
				}
			}
		}

	return S_OK;
}

STDMETHODIMP ScriptGlobalTable::EndMusic()
{
	if (g_pplayer && g_pplayer->m_fPlayMusic)
		{
		if (g_pplayer->m_pcsimpleplayer)
			{
			g_pplayer->m_pcsimpleplayer->Stop();
			g_pplayer->m_pcsimpleplayer->Release();
			g_pplayer->m_pcsimpleplayer = NULL;
			}
		if (g_pplayer->m_pxap)
			{
			g_pplayer->m_pxap->End();
			delete g_pplayer->m_pxap;
			g_pplayer->m_pxap = NULL;
			}
		}

	return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_Name(BSTR *pVal)
{
	*pVal = SysAllocString(L"Global");

	return S_OK;
}

IDispatch *ScriptGlobalTable::GetDispatch()
	{
	return (IDispatch *)this;
	}

STDMETHODIMP ScriptGlobalTable::get_LeftFlipperKey(long *pVal)
{
	*pVal = g_pplayer->m_rgKeys[eLeftFlipperKey];

	return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_RightFlipperKey(long *pVal)
{
	*pVal = g_pplayer->m_rgKeys[eRightFlipperKey];

	return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_LeftTiltKey(long *pVal)
{
	*pVal = g_pplayer->m_rgKeys[eLeftTiltKey];

	return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_RightTiltKey(long *pVal)
{
	*pVal = g_pplayer->m_rgKeys[eRightTiltKey];

	return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_CenterTiltKey(long *pVal)
{
	*pVal = g_pplayer->m_rgKeys[eCenterTiltKey];

	return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_PlungerKey(long *pVal)
{
	*pVal = g_pplayer->m_rgKeys[ePlungerKey];

	return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_StartGameKey(long *pVal)
{
	*pVal = g_pplayer->m_rgKeys[eStartGameKey];

	return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_AddCreditKey(long *pVal)
{
	*pVal = g_pplayer->m_rgKeys[eAddCreditKey];

	return S_OK;
}

BOOL ScriptGlobalTable::GetTextFileFromDirectory(char *szfilename, char *dirname, BSTR *pContents)
	{
	//char szPath[MAX_PATH];
	char *szPath;
	szPath = new char[MAX_PATH + lstrlen(szfilename)];
	BOOL fSuccess = fFalse;

	if (dirname != NULL)
		{
		lstrcpy(szPath, g_pvp->m_szMyPath);
		lstrcat(szPath, dirname);
		}
	else
		{
		// Current directory
		szPath[0] = '\0';
		}
	lstrcat(szPath, szfilename);

	int len;
	BYTE *szContents;

	if (RawReadFromFile(szPath, &len, (char **)&szContents))
		{
		BYTE *szDataStart = szContents;
		int encoding = CP_ACP;

		if (szContents[0] == 0xEF && szContents[1] == 0xBB && szContents[2] == 0xBF)
			{
			encoding = CP_UTF8;
			szDataStart += 3;
			len -= 3;
			}

		if (szContents[0] == 0xFF && szContents[1] == 0xFE)
			{
			// Unicode
			szDataStart += 2;
			len -= 2;
			*pContents = SysAllocString((WCHAR *)szDataStart);
			}
		else
			{
			WCHAR *wzContents = new WCHAR[len+1];

			MultiByteToWideChar(encoding, 0, (char *)szDataStart, len, wzContents, len+1);
			wzContents[len] = L'\0';

			*pContents = SysAllocString(wzContents);
			delete wzContents;
			}

		delete szContents;

		fSuccess = fTrue;
		}

	delete szPath;

	return fSuccess;
	}

STDMETHODIMP ScriptGlobalTable::GetTextFile(BSTR FileName, BSTR *pContents)
{
	BOOL fSuccess;
	char szFileName[MAX_PATH];

	WideCharToMultiByte(CP_ACP, 0, FileName, -1, szFileName, MAX_PATH, NULL, NULL);

	// try to load the scripts from the current directory
	fSuccess = GetTextFileFromDirectory(szFileName, NULL, pContents);

	// if that fails, try the User, Scripts and Tables sub-directorys under where VP was loaded from
		if (!fSuccess)
		{
		fSuccess = GetTextFileFromDirectory(szFileName, "User\\", pContents);
		}

	if (!fSuccess)
		{
		fSuccess = GetTextFileFromDirectory(szFileName, "Scripts\\", pContents);
		}

	if (!fSuccess)
		{
		fSuccess = GetTextFileFromDirectory(szFileName, "Tables\\", pContents);
		}

	return (fSuccess) ? S_OK : E_FAIL;
}

STDMETHODIMP ScriptGlobalTable::get_UserDirectory(BSTR *pVal)
{
	WCHAR wzPath[MAX_PATH];
	WideStrCopy(g_pvp->m_wzMyPath, wzPath);
	WideStrCat(L"User\\", wzPath);
	*pVal = SysAllocString(wzPath);

	return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_GetPlayerHWnd(long *pVal)
{
	if (!g_pplayer)
		{
		*pVal = NULL;
		return E_FAIL;
		}
	else
		{
		*pVal = (long)g_pplayer->m_hwnd;
		}

	return S_OK;
}

STDMETHODIMP ScriptGlobalTable::AddObject(BSTR Name, IDispatch *pdisp)
{
	if (!g_pplayer)
		{
		return E_FAIL;
		}

	g_pplayer->m_ptable->m_pcv->AddTemporaryItem(Name, pdisp);

	return S_OK;
}

STDMETHODIMP ScriptGlobalTable::SaveValue(BSTR TableName, BSTR ValueName, VARIANT Value)
{
	IStorage* pstgRoot;
	IStorage* pstgTable;
	IStream* pstmValue;

	HRESULT hr = S_OK;

	WCHAR wzPath[MAX_PATH];
	WideStrCopy(g_pvp->m_wzMyPath, wzPath);
	WideStrCat(L"User\\VPReg.stg", wzPath);

	if (FAILED(hr = StgOpenStorage(wzPath, NULL, STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, NULL, 0, &pstgRoot)))
		{
		// Registry file does not exist - create it
		if (FAILED(hr = StgCreateDocfile(wzPath, STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, &pstgRoot)))
			{
			return hr;
			}
		}

	if (FAILED(hr = pstgRoot->OpenStorage(TableName, NULL, STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, NULL, 0, &pstgTable)))
		{
		// Table file does not exist
		if (FAILED(hr = pstgRoot->CreateStorage(TableName, STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstgTable)))
			{
			pstgRoot->Release();
			return hr;
			}
		}

	if (FAILED(hr = pstgTable->CreateStream(ValueName, STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmValue)))
		{
		pstgTable->Release();
		pstgRoot->Release();
		return hr;
		}

	BSTR bstr = BstrFromVariant(&Value, 0x409);

	DWORD writ;

	hr = pstmValue->Write((WCHAR *)bstr, lstrlenW((WCHAR *)bstr) * sizeof(WCHAR), &writ);

	SysFreeString(bstr);

	pstmValue->Release();

	pstgTable->Commit(STGC_DEFAULT);
	pstgTable->Release();

	pstgRoot->Commit(STGC_DEFAULT);
	pstgRoot->Release();

	return S_OK;
}

STDMETHODIMP ScriptGlobalTable::LoadValue(BSTR TableName, BSTR ValueName, VARIANT *Value)
{
	IStorage* pstgRoot;
	IStorage* pstgTable;
	IStream* pstmValue;

	HRESULT hr = S_OK;

	WCHAR wzPath[MAX_PATH];
	WideStrCopy(g_pvp->m_wzMyPath, wzPath);
	WideStrCat(L"User\\VPReg.stg", wzPath);

	if (FAILED(hr = StgOpenStorage(wzPath, NULL, STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, NULL, 0, &pstgRoot)))
		{
		SetVarBstr(Value, SysAllocString(L""));
		return S_OK;
		}

	if (FAILED(hr = pstgRoot->OpenStorage(TableName, NULL, STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, NULL, 0, &pstgTable)))
		{
		SetVarBstr(Value, SysAllocString(L""));
		pstgRoot->Release();
		return S_OK;
		}

	if (FAILED(hr = pstgTable->OpenStream(ValueName, 0, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmValue)))
		{
		SetVarBstr(Value, SysAllocString(L""));
		pstgTable->Release();
		pstgRoot->Release();
		return S_OK;
		}

	STATSTG statstg;

	pstmValue->Stat(&statstg, STATFLAG_NONAME);

	WCHAR *wzT;

	int size = statstg.cbSize.LowPart / sizeof(WCHAR);

	wzT = new WCHAR[size + 1];

	DWORD read;

	hr = pstmValue->Read(wzT, size * sizeof(WCHAR), &read);
	wzT[size] = L'\0';

	pstmValue->Release();

	pstgTable->Commit(STGC_DEFAULT);
	pstgTable->Release();

	pstgRoot->Commit(STGC_DEFAULT);
	pstgRoot->Release();

	SetVarBstr(Value, SysAllocString(wzT));

	delete wzT;

	return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_ActiveBall(IBall **pVal)
{
	if (!g_pplayer)
		{
		return E_POINTER;
		}

	if (!g_pplayer->m_pactiveball)
		{
		return E_POINTER;
		//*pVal = NULL;
		//return S_OK;
		}

	BallEx *pballex = g_pplayer->m_pactiveball->m_pballex;

	pballex->QueryInterface(IID_IBall, (void **)pVal);

	return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_GameTime(long *pVal)
{
	if (!g_pplayer)
		{
		return E_POINTER;
		}

	*pVal = g_pplayer->m_timeCur;

	return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_VPBuildVersion(long *pVal)
{
	if (!g_pplayer)
		{
		return E_POINTER;
		}
	*pVal = BUILDNUMBER;
	return S_OK;
}

PinTable::PinTable()
	{
	//m_pselcur = this;
	m_vmultisel.AddElement((ISelect *)this);
	m_undo.m_ptable = this;
	//m_hbmBackdrop = NULL;
	m_fGrid = fTrue;
	m_fBackdrop = fTrue;
	m_fRenderShadows = fTrue;
	m_PhysicsType = PhysicsVP;

	CComObject<CodeViewer>::CreateInstance(&m_pcv);
	m_pcv->AddRef();
	m_pcv->Init((IScriptableHost*)this);
	m_pcv->Create(g_pvp->m_hwnd);

	CComObject<ScriptGlobalTable>::CreateInstance(&m_psgt);
	m_psgt->AddRef();
	m_psgt->Init(this);

	int i;
	for (i=0;i<eItemTypeCount;i++)
		{
		m_suffixcount[i] = 1;
		}

	m_sdsDirtyProp = eSaveClean;
	m_sdsDirtyScript = eSaveClean;
	m_sdsNonUndoableDirty = eSaveClean;
	m_sdsCurrentDirtyState = eSaveClean;

	// set up default protection security descripter
	ResetProtectionBlock();

	m_szTableName = NULL;
	m_szAuthor = NULL;
	m_szVersion = NULL;
	m_szReleaseDate = NULL;
	m_szAuthorEMail = NULL;
	m_szWebSite = NULL;
	m_szBlurb = NULL;
	m_szDescription = NULL;
	m_szRules = NULL;

	m_pbTempScreenshot = NULL;

	//m_protectionData.flags |= DISABLE_TABLE_SAVE;
	//m_protectionData.flags |= DISABLE_TABLE_SAVEPROT;
	//m_protectionData.flags |= DISABLE_SCRIPT_EDITING;
	//m_protectionData.flags |= DISABLE_OPEN_MANAGERS;
	//m_protectionData.flags |= DISABLE_CUTCOPYPASTE;
	//m_protectionData.flags |= DISABLE_TABLEVIEW;
	//m_protectionData.flags |= DISABLE_EVERYTHING;

	m_plungerNormalize = 100;  //Mech-Plunger component adjustment or weak spring, aging
	m_plungerFilter = fTrue;

	HRESULT hr;
	int tmp;
	
	m_tblAccelerometer = fTrue;							// true if electronic accelerometer enabled
	hr = GetRegInt("Player", "PinballWizard", &m_tblAccelerometer);
	m_tblAccelerometer = m_tblAccelerometer != fFalse;

	m_tblAccelNormalMount = fTrue;						// true is normal mounting (left hand coordinates)
	hr = GetRegInt("Player", "PinballWizardNormalMount", &m_tblAccelNormalMount);
	m_tblAccelNormalMount = m_tblAccelNormalMount != fFalse;

	m_tblAccelAngle = 0.0f;			// 0 degrees rotated counterclockwise (GUI is lefthand coordinates)
	hr = GetRegInt("Player", "PinballWizardRotation", &tmp);
	if (hr == S_OK) m_tblAccelAngle = (float)tmp;

	m_tblAccelAmp = 1.5f;								// Accelerometer gain 
	hr = GetRegInt("Player", "PinballWizardAccelGain", &tmp);
	if (hr == S_OK) m_tblAccelAmp = (float)tmp/100.0f;

	m_tblAccelManualAmp = 3.5f;							// manual input gain, generally from joysticks
	hr = GetRegInt("Player", "JoystickGain", &tmp);
	if (hr == S_OK) m_tblAccelManualAmp = (float)tmp/100.0f;

	}

PinTable::~PinTable()
	{
	int i;

	for (i=0;i<m_vedit.Size();i++)
		{
		m_vedit.ElementAt(i)->Release();
		}

	ClearOldSounds();

	for (i=0;i<m_vsound.Size();i++)
		{
		//m_vsound.ElementAt(i)->m_pDSBuffer->Release();
		delete m_vsound.ElementAt(i);
		}

	for (i=0;i<m_vimage.Size();i++)
		{
		delete m_vimage.ElementAt(i);
		}

	for (i=0;i<m_vfont.Size();i++)
		{
		m_vfont.ElementAt(i)->UnRegister();
		delete m_vfont.ElementAt(i);
		}

	for (i=0;i<m_vcollection.Size();i++)
		{
		m_vcollection.ElementAt(i)->Release();
		}

	for (i=0;i<m_vCustomInfoTag.Size();i++)
		{
		delete m_vCustomInfoTag.ElementAt(i);
		delete m_vCustomInfoContent.ElementAt(i);
		}

	m_pcv->Release();

	m_psgt->Release();

	if (IsWindow(m_hwnd))
		{
		//DestroyWindow(m_hwnd);
		SendMessage(g_pvp->m_hwndWork, WM_MDIDESTROY, (DWORD)m_hwnd, 0);
		}

	if (m_hbmOffScreen)
		{
		DeleteObject(m_hbmOffScreen);
		}

	SAFE_DELETE(m_szTableName);
	SAFE_DELETE(m_szAuthor);
	SAFE_DELETE(m_szVersion);
	SAFE_DELETE(m_szReleaseDate);
	SAFE_DELETE(m_szAuthorEMail);
	SAFE_DELETE(m_szWebSite);
	SAFE_DELETE(m_szBlurb);
	SAFE_DELETE(m_szDescription);
	SAFE_DELETE(m_szRules);
	}

BOOL PinTable::FVerifySaveToClose()
	{
	if (m_vAsyncHandles.Size() > 0)
		{
		DWORD wait = WaitForMultipleObjects(m_vAsyncHandles.Size(), (HANDLE *)m_vAsyncHandles.GetArray(), TRUE, INFINITE);
		//MessageBox(NULL, "Async work items not done", NULL, 0);

		// Close the remaining handles here, since the window messages will never be processed
		int i;
		for (i=0;i<m_vAsyncHandles.Size();i++)
			{
			CloseHandle(m_vAsyncHandles.ElementAt(i));
			}

		g_pvp->SetActionCur("");
		}

	return fTrue;
	}

BOOL PinTable::CheckPermissions(unsigned long flag)
	{
	if ( ((m_protectionData.flags & DISABLE_EVERYTHING) == DISABLE_EVERYTHING) ||
		 ((m_protectionData.flags & flag) 				== flag) 				)
		{
		return fTrue;	// cannot perform this operation
		}
	else
		{
		return fFalse;
		}
	}

BOOL PinTable::IsTableProtected()
	{
	if (m_protectionData.flags != 0)
		{
		return fTrue;	// table is protected
		}
	else
		{
		return fFalse;
		}
	}

void PinTable::ResetProtectionBlock()
	{
	// set up default protection security descripter
	memset (&m_protectionData, 0x00, sizeof(m_protectionData));
	m_protectionData.fileversion = PROT_DATA_VERSION;
	m_protectionData.size = sizeof(m_protectionData);
	}

BOOL PinTable::SetupProtectionBlock(unsigned char *pPassword, unsigned long flags)
	{
	int 		foo;
	BOOL		rc 		= fTrue;
	HCRYPTPROV  hcp		= NULL;
	HCRYPTKEY  	hkey	= NULL;
	HCRYPTHASH 	hchkey	= NULL;

	_protectionData *pProtectionData = &m_protectionData;

	// acquire a crypto context
	foo = CryptAcquireContext(&hcp, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_NEWKEYSET/* | CRYPT_SILENT*/);
	foo = GetLastError();
	// create a hash
	foo = CryptCreateHash(hcp, CALG_MD5, NULL, 0, &hchkey);
	foo = GetLastError();
	// hash the password
	foo = CryptHashData(hchkey, pPassword, strlen((char *)pPassword), 0);
	foo = GetLastError();
	// Create a block cipher session key based on the hash of the password.
	foo = CryptDeriveKey(hcp, CALG_RC2, hchkey, CRYPT_EXPORTABLE, &hkey);
	foo = GetLastError();

	// copy our paraphrase key into the key field
	memcpy(pProtectionData->paraphrase, PARAPHRASE_KEY, sizeof(PARAPHRASE_KEY));

	// encypt this with the key generated with the password
	DWORD cryptlen;
	// get the size of the data to encrypt
	cryptlen = PROT_PASSWORD_LENGTH;

	// encrypt the paraphrase
    foo = CryptEncrypt( hkey,									// key to use
						0, 										// not hashing data at the same time
						TRUE, 									// last block (or only block)
						0, 										// no flags
						(BYTE *)pProtectionData->paraphrase,	// buffer to encrypt
						&cryptlen,								// size of data to encrypt
						sizeof(pProtectionData->paraphrase));	// maximum size of buffer (includes any padding)

	foo = GetLastError();		// purge any errors

	// set up the flags
	pProtectionData->flags = flags;

	// destroy our cryto
	foo = CryptDestroyHash(hchkey);
	foo = CryptDestroyKey(hkey);
	foo = CryptReleaseContext(hcp, 0);

	return(rc);
	}

BOOL PinTable::UnlockProtectionBlock(unsigned char *pPassword)
	{
	int 			foo;
	BOOL			rc 		= fFalse;
	HCRYPTPROV  	hcp		= NULL;
	HCRYPTKEY  		hkey	= NULL;
	HCRYPTHASH 		hchkey	= NULL;
	unsigned char	paraphrase[PROT_CIPHER_LENGTH];
	_protectionData *pProtectionData = &m_protectionData;

	// acquire a crypto context
	foo = CryptAcquireContext(&hcp, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_NEWKEYSET/* | CRYPT_SILENT*/);
	foo = GetLastError();
	// create a hash
	foo = CryptCreateHash(hcp, CALG_MD5, NULL, 0, &hchkey);
	foo = GetLastError();
	// hash the password
	foo = CryptHashData(hchkey, pPassword, strlen((char *)pPassword), 0);
	foo = GetLastError();
	// Create a block cipher session key based on the hash of the password.
	foo = CryptDeriveKey(hcp, CALG_RC2, hchkey, CRYPT_EXPORTABLE, &hkey);
	foo = GetLastError();

	// copy the paraphase from the protection block to some local memory
	memcpy(paraphrase, pProtectionData->paraphrase, sizeof(paraphrase));

	// decypt this with the key generated with the password
	DWORD cryptlen;
	// get the size of the data to encrypt
	cryptlen = PROT_CIPHER_LENGTH;

	// encrypt the paraphrase
    foo = CryptDecrypt( hkey,									// key to use
						0, 										// not hashing data at the same time
						TRUE, 									// last block (or only block)
						0, 										// no flags
						(BYTE *)paraphrase,						// buffer to encrypt
						&cryptlen);								// size of data to decrypt

	// destroy our cryto
	foo = CryptDestroyHash(hchkey);
	foo = CryptDestroyKey(hkey);
	foo = CryptReleaseContext(hcp, 0);

	// if the decrypted data matches the original paraphrase then unlock the table
	if ( (cryptlen == PROT_PASSWORD_LENGTH) &&
		 (memcmp(paraphrase, PARAPHRASE_KEY, sizeof(PARAPHRASE_KEY)) == 0) )
		{
		ResetProtectionBlock();
		rc = fTrue;
		}

	return(rc);
	}

	// Now, create a real storage for VBA to use (the mem one crashes for some reason when trying to actually use VBA)
	/*if (SUCCEEDED(hr = StgCreateDocfile(L"c:\\foobar.vpt", STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE,
				0, &m_pStg)))
		{
		if (SUCCEEDED(hr = ApcProject.Save(m_pStg, FALSE)))
			{
			if (SUCCEEDED(hr = ApcProject.SaveCompleted(m_pStg)))
				{
				m_pStg->Commit(STGC_DEFAULT);
				}
			else
				{
				_ASSERTE("Initial storage not created.  Not good.");
				m_pStg->Revert();
				}
			}
		}*/

	// Release previous storage
	//pis->Release();

#define NEWFROMRES 1

void PinTable::Init(VPinball *pvp)
	{
	m_pvp = pvp;

#ifdef NEWFROMRES
	// Get our new table resource, get it to be opened as a storage, and open it like a normal file
	HRSRC hrsrc = FindResource(NULL, MAKEINTRESOURCE(IDR_TABLE_NEW), "TABLE");
	HGLOBAL hglobal = LoadResource(NULL, hrsrc);
	char *pchar = (char *)LockResource(hglobal);
	DWORD size = SizeofResource(NULL, hrsrc);
	HGLOBAL hcopiedmem = GlobalAlloc(GMEM_MOVEABLE, size);
	char *pcopied = (char *)GlobalLock(hcopiedmem);
	memcpy(pcopied, pchar, size);
	GlobalUnlock(hcopiedmem);

	ILockBytes *pilb;
	HRESULT hr = CreateILockBytesOnHGlobal(hcopiedmem, TRUE, &pilb);
	IStorage *pis;

	hr = StgOpenStorageOnILockBytes(pilb,NULL,STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,NULL,0,&pis);

	m_glassheight = 210;

	int i;
	for (i=0;i<16;i++)
		{
		rgcolorcustom[i] = RGB(0,0,0);
		}

#ifdef VBA
	m_pStg = pis;
#endif

	//pilb->Release();

	//LoadGameFromFilename("d:\\gdk\\data\\tables\\newsave\\basetable6.vpt");

	char szSuffix[32];

	LocalString ls(IDS_TABLE);
	lstrcpy(m_szTitle, ls.m_szbuffer/*"Table"*/);
	_itoa(g_pvp->m_NextTableID, szSuffix, 10);
	lstrcat(m_szTitle, szSuffix);
	g_pvp->m_NextTableID++;
	m_szFileName[0] = '\0';
	//lstrcpy(m_szFileName, m_szFileName);

	LoadGameFromStorage(pis);

	m_angletilt = 6;

	//MAKE_WIDEPTR_FROMANSI(wszFileName, m_szFileName);
	//ApcProject->APC_PUT(DisplayName)(wszFileName);

	InitPostLoad(pvp);

	SetCaption(m_szTitle);
#else

	m_szFileName[0] = 0;
	m_szBlueprintFileName[0] = 0;
	m_gridsize = 50;

	//m_ptinfoCls = NULL;
	//m_ptinfoInt = NULL;

	m_left = 0;
	m_top = 0;
	m_right = 1000;
	m_bottom = 2000;

	m_inclination = 43;
	m_FOV = 45;

	/*m_offsetx = (m_left+m_right) / 2;
	m_offsety = (m_top+m_bottom) / 2;
	m_zoom = 0.5;*/
	SetDefaultView();

	m_szImage[0] = 0;
	m_szImageBackdrop[0] = 0;

	m_colorplayfield = RGB(128,128,128);
	m_colorbackdrop = RGB(128,128,128);

	InitVBA();

	CreateTableWindow();

	//m_psur = new Surface();
	/*CComObject<Surface> *psur;
	CComObject<Surface>::CreateInstance(&psur);
	if (psur)
		{
		psur->AddRef();
		psur->Init(this, 500, 1000);
		m_vedit.AddElement((IEditable *)psur);

		psur->m_d.m_fInner = fFalse;
		}*/

	SetMyScrollInfo();
#endif
	}

void PinTable::SetDefaultView()
	{
	FRect frect;
	GetViewRect(&frect);

	m_offsetx = (frect.left+frect.right) / 2;
	m_offsety = (frect.top+frect.bottom) / 2;
	m_zoom = 0.5;
	}

void PinTable::SetCaption(char *szCaption)
	{
	SetWindowText(m_hwnd, szCaption);
	m_pcv->SetCaption(szCaption);
	}

void PinTable::InitPostLoad(VPinball *pvp)
	{
	m_pvp = pvp;

	m_hbmOffScreen = NULL;
	m_fDirtyDraw = fTrue;

	m_left = 0;
	m_top = 0;
	//m_right = 1000;

	//m_offsetx = (m_left+m_right) / 2;
	//m_offsety = (m_top+m_bottom) / 2;
	//m_zoom = 0.5;
	SetDefaultView();

	m_szBlueprintFileName[0] = 0;

	CreateGDIBackdrop();

	//InitVBA();
	m_pcv->AddItem(this, fFalse);
	m_pcv->AddItem(m_psgt, fTrue);
	m_pcv->AddItem(m_pcv->m_pdm, fFalse);

	CreateTableWindow();

	SetMyScrollInfo();
	}

BOOL FWzEqual(WCHAR *wz1, WCHAR *wz2)
	{
	while (*wz1 != 0 || *wz2 != 0)
		{
		if (*wz1++ != *wz2++)
			{
			return fFalse;
			}
		}
	return fTrue;
	}

void PinTable::GetUniqueName(int type, WCHAR *wzUniqueName)
	{
	int suffix = 1;
	//int i;
	BOOL fFound = fFalse;
	WCHAR wzRoot[128];
	WCHAR wzName[128];
	WCHAR wzSuffix[10];

	LocalString ls(rgTypeStringIndex[type]);

	MultiByteToWideChar(CP_ACP, 0, ls.m_szbuffer, -1, wzRoot, 128);

	while (!fFound)
		{
		//lstrcpyW(szName, rgwzTypeName[type]);
		WideStrCopy(wzRoot, wzName);
		_itow(suffix, wzSuffix, 10);
		WideStrCat(wzSuffix, wzName);
		//lstrcatW(szName, szSuffix);

		/*for (i=0;i<m_vedit.Size();i++)
			{
			CComBSTR bstr;
			if (m_vedit.ElementAt(i)->GetScriptable())
				{
				m_vedit.ElementAt(i)->GetScriptable()->get_Name(&bstr);
				if (FWzEqual(bstr, wzName))
					{
					break;
					}
				}
			}*/

		//if (i >= m_vedit.Size())
		if (m_pcv->m_vcvd.GetSortedIndex(wzName) == -1)
			{
			fFound = fTrue;
			}
		else
			{
			suffix += 1;
			}
		}

	//lstrcpyW(wzUniqueName, szName);
	WideStrCopy(wzName, wzUniqueName);
	}

void PinTable::Render(Sur *psur)
	{
	RECT rc;
	GetClientRect(m_hwnd, &rc);
	psur->SetFillColor(RGB(255,255,255));
	psur->SetBorderColor(-1,fFalse,0);
	int i;

	FRect frect;
	GetViewRect(&frect);
	//SelectObject(psur->m_hdc, GetStockObject(WHITE_BRUSH));
	//PatBlt(psur->m_hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, PATCOPY);

	//PinImage *ppi = GetImage((char *)m_szImage);
	//float maxtu,maxtv;

	/*if (ppi)
		{
		HDC hdcImage;
		ppi->m_pdsBuffer->GetDC(&hdcImage);

		//StretchBlt(psur->m_hdc, 0, 0, ppi->m_width, ppi->m_height, hdcImage, 0, 0, ppi->m_width, ppi->m_height, SRCCOPY);

		psur->Image(m_left, m_top, m_right, m_bottom, hdcImage, ppi->m_width, ppi->m_height);

		ppi->m_pdsBuffer->ReleaseDC(hdcImage);
		}*/

	psur->Rectangle2(rc.left, rc.top, rc.right, rc.bottom);

	// can we view the table elements?? if not then draw a box for the table outline
	// got to give the punters at least something to know that the table has loaded
	if (CheckPermissions(DISABLE_TABLEVIEW) == fTrue)
		{
		float rleft, rtop, rright, rbottom;

		psur->ScreenToSurface(rc.left, rc.top, &rleft, &rtop);
		psur->ScreenToSurface(rc.right, rc.bottom, &rright, &rbottom);
		rleft = max(rleft, frect.left);
		rtop = max(rtop, frect.top);
		rright = min(rright, frect.right);
		rbottom = min(rbottom, frect.bottom);

		psur->SetObject(NULL); 							// Don't hit test edgelines

		psur->SetLineColor(RGB(0,0,0), fFalse, 0);		// black outline

		psur->Line(rleft, rtop, rright, rtop);
		psur->Line(rright, rtop, rright, rbottom);
		psur->Line(rleft, rbottom, rright, rbottom);
		psur->Line(rleft, rtop, rleft, rbottom);

		return;
		}
//<<<

	/*if (m_hbmBackdrop && m_fBackdrop)
		{
		HDC hdcScreen;
		HDC hdcNew;
		HBITMAP hbmOld;

		PinImage *ppi = GetImage(m_szImage);

		if (ppi)
			{
			hdcScreen = GetDC(NULL);
			hdcNew = CreateCompatibleDC(hdcScreen);
			hbmOld = (HBITMAP)SelectObject(hdcNew, m_hbmBackdrop);

			psur->Image(m_left, m_top, m_right, m_bottom, hdcNew, ppi->m_width, ppi->m_height);

			SelectObject(hdcNew, hbmOld);
			DeleteDC(hdcNew);
			ReleaseDC(NULL, hdcScreen);
			}
		}*/

	if (m_fBackdrop)
			{
			HDC hdcScreen;
			HDC hdcNew;
			HBITMAP hbmOld;
			PinImage *ppi;

			if (!g_pvp->m_fBackglassView)
				{
				ppi = GetImage(m_szImage);
				}
			else
				{
				ppi = GetImage(m_szImageBackdrop);
				}

			if (ppi)
				{
				ppi->EnsureHBitmap();
				if (ppi->m_hbmGDIVersion)
					{
					hdcScreen = GetDC(NULL);
					hdcNew = CreateCompatibleDC(hdcScreen);
					hbmOld = (HBITMAP)SelectObject(hdcNew, ppi->m_hbmGDIVersion);

					psur->Image(frect.left, frect.top, frect.right, frect.bottom, hdcNew, ppi->m_width, ppi->m_height);

					SelectObject(hdcNew, hbmOld);
					DeleteDC(hdcNew);
					ReleaseDC(NULL, hdcScreen);
					}
				}
			}

	if (g_pvp->m_fBackglassView)
		{
		Render3DProjection(psur);
		}

	for (i=0;i<m_vedit.Size();i++)
		{
		if (m_vedit.ElementAt(i)->m_fBackglass == g_pvp->m_fBackglassView)
			{
			m_vedit.ElementAt(i)->PreRender(psur);
			}
		}

	if (m_fGrid)
		{
		float rleft, rtop, rright, rbottom;

		psur->ScreenToSurface(rc.left, rc.top, &rleft, &rtop);
		psur->ScreenToSurface(rc.right, rc.bottom, &rright, &rbottom);

		rleft = max(rleft, frect.left);
		rtop = max(rtop, frect.top);
		rright = min(rright, frect.right);
		rbottom = min(rbottom, frect.bottom);

		int beginx = (int)((rleft / m_gridsize));
		float lenx = (rright - rleft) / m_gridsize;//(((rc.right - rc.left)/m_zoom));
		int beginy = (int)((rtop / m_gridsize));
		float leny = (rbottom - rtop) / m_gridsize;//(((rc.bottom - rc.top)/m_zoom));

		//beginx -= lenx*0.5;
		//beginy -= leny*0.5;

		/*beginx = max(beginx, (int)(m_left / m_gridsize));
		beginy = max(beginy, (int)(m_top / m_gridsize));

		rleft = max(rleft, m_left);
		rtop = max(rtop, m_top);
		rright = min(rright, m_right);
		rbottom = min(rbottom, m_bottom);

		lenx = min((m_right-beginx) / m_gridsize, lenx);
		leny = min((m_bottom-beginy) / m_gridsize, leny);*/

		psur->SetObject(NULL); // Don't hit test gridlines

		psur->SetLineColor(RGB(190,220,240), fFalse, 0);
		for (i=0;i<(lenx+1);i+=1)
			{
			float x;
			x = (beginx+i)*m_gridsize;
			psur->Line(x, rtop, x, rbottom);
			}

		for (i=0;i<(leny+1);i+=1)
			{
			float y;
			y = (beginy+i)*m_gridsize;
			psur->Line(rleft, y, rright, y);
			}
		}

	for (i=0;i<m_vedit.Size();i++)
		{
		if (m_vedit.ElementAt(i)->m_fBackglass == g_pvp->m_fBackglassView)
			{
			m_vedit.ElementAt(i)->Render(psur);
			}
		}

	if (g_pvp->m_fBackglassView) // Outline of the view, for when the grid is off
		{
		psur->SetObject(NULL);
		psur->SetFillColor(-1);
		psur->SetBorderColor(RGB(0,0,0), fFalse, 1);
		psur->Rectangle(0,0,1000,750);
		}

	if (m_fDragging)
		{
		psur->SetFillColor(-1);
		psur->SetBorderColor(RGB(0,0,0),fTrue,0);
		psur->Rectangle(m_rcDragRect.left, m_rcDragRect.top, m_rcDragRect.right, m_rcDragRect.bottom);
		}
	}

void PinTable::Render3DProjection(Sur *psur)
	{
	int i;
	PinProjection pinproj;
	float rotation = 0;
	float inclination = ANGTORAD(m_inclination);

	Vector<Vertex3D> vvertex3D;

	for (i=0;i<m_vedit.Size();i++)
		{
		m_vedit.ElementAt(i)->GetBoundingVertices(&vvertex3D);
		}

	// dummy coordinate system for backdrop view
	pinproj.m_rcviewport.left = 0;
	pinproj.m_rcviewport.top = 0;
	pinproj.m_rcviewport.right = 1000;
	pinproj.m_rcviewport.bottom = 750;

	double aspect = ((double)1000)/750;

	float realFOV = m_FOV;

	if (realFOV <= 0)
		{
		realFOV = 0.001f; // Can't have a real zero FOV, but this will look the same
		}

	pinproj.FitCameraToVertices(&vvertex3D/*rgv*/, vvertex3D.Size(), aspect, rotation, inclination, realFOV);
	pinproj.SetFieldOfView(realFOV, aspect, pinproj.m_rznear, pinproj.m_rzfar);
	pinproj.Translate(-pinproj.m_vertexcamera.x,-pinproj.m_vertexcamera.y,-pinproj.m_vertexcamera.z);
	pinproj.Rotate(inclination,0,rotation);
	pinproj.CacheTransform();

	for (i=0;i<vvertex3D.Size();i++)
		{
		delete vvertex3D.ElementAt(i);
		}

	psur->SetFillColor(RGB(200,200,200));
	psur->SetBorderColor(-1,fFalse,0);

	Vertex3D rgvIn[6];
	Vertex3D rgvOut[6];

	rgvIn[0].Set(m_left, m_top, 50);
	rgvIn[1].Set(m_right, m_top, 50);
	rgvIn[2].Set(m_right, m_bottom, 50);
	rgvIn[3].Set(m_right, m_bottom, 0);
	rgvIn[4].Set(m_left, m_bottom, 0);
	rgvIn[5].Set(m_left, m_bottom, 50);

	Vertex rgv[6];

	pinproj.TransformVertices(rgvIn, NULL, 6, rgvOut);

	for (i=0;i<6;i++)
		{
		rgv[i].x = rgvOut[i].x;
		rgv[i].y = rgvOut[i].y;
		}

	psur->Polygon(rgv, 6);
	}

void PinTable::Paint(HDC hdc)
	{
	Sur *psur;
	//HBITMAP hbmOffScreen;
	HBITMAP hbmOld;
	HDC hdc2;

	RECT rc;

	//HRGN hrgn = CreateRectRgn(0, 0, 10, 10);

	//GetRandomRgn(hdc, hrgn, SYSRGN);

	//SelectClipRgn(hdc, NULL);

	GetClientRect(m_hwnd, &rc);

	//SelectObject(hdc, GetStockObject(BLACK_BRUSH));
	//PatBlt(hdc, 0/*rc.left*/, 0/*rc.top*/, 1000/*rc.right - rc.left*/, 1000/*rc.bottom - rc.top*/, PATCOPY);

	if (m_fDirtyDraw)
		{
		if (m_hbmOffScreen)
			{
			DeleteObject(m_hbmOffScreen);
			}
		m_hbmOffScreen = CreateCompatibleBitmap(hdc, rc.right - rc.left, rc.bottom - rc.top);
		}

	hdc2 = CreateCompatibleDC(hdc);

	hbmOld = (HBITMAP)SelectObject(hdc2, m_hbmOffScreen);

	if (m_fDirtyDraw)
		{
		psur = new PaintSur(hdc2, m_zoom, m_offsetx, m_offsety, rc.right - rc.left, rc.bottom - rc.top, m_vmultisel.ElementAt(0)/*m_pselcur*/);
		Render(psur);

		delete psur;
		}

	BitBlt(hdc, rc.left, rc.top, rc.right, rc.bottom, hdc2, 0, 0, SRCCOPY);

	SelectObject (hdc2, hbmOld);

	DeleteDC(hdc2);

	m_fDirtyDraw = fFalse;
	//DeleteObject(hbmOffScreen);
	}

ISelect *PinTable::HitTest(int x, int y)
	{
	HitSur *phs;
	HDC hdc;
	ISelect *pisel;

	hdc = GetDC(m_hwnd);

	RECT rc;
	GetClientRect(m_hwnd, &rc);

	phs = new HitSur(hdc, m_zoom, m_offsetx, m_offsety, rc.right - rc.left, rc.bottom - rc.top, x, y, this);

	Render(phs);

	pisel = phs->m_pselected;

	//SetSel(phs->m_pselected);

	delete phs;

	ReleaseDC(m_hwnd, hdc);

	return pisel;
	}

void PinTable::SetDirtyDraw()
	{
	m_fDirtyDraw = fTrue;
	InvalidateRect(m_hwnd, NULL, fFalse);
	}

HANDLE hmyfont;

void PinTable::Play()
	{
	short foo = 1;
	char szLoadDir[MAX_PATH];

	//int fonts = AddFontResource("d:\\gdk\\vbatest\\fonts\\LED.ttf");

	/*OFSTRUCT ofstruct;

	ofstruct.cBytes = sizeof(OFSTRUCT);

	HANDLE hFontFile = CreateFile("d:\\gdk\\vbatest\\fonts\\LED.ttf",
		GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	DWORD filesize = GetFileSize(hFontFile, NULL);

	BOOL fFoo;

	char *pchar = new char[filesize];

	DWORD cfonts;

	DWORD read;

	fFoo = ReadFile(hFontFile, pchar, filesize, &read, NULL);

	HANDLE hmyfont = AddFontMemResourceEx(pchar, filesize, 0, &cfonts);

	delete [] pchar;

	fFoo = CloseHandle(hFontFile);*/

	if (g_pplayer)
		{
		return; // Can't play twice
		}
		
	EndAutoSaveCounter();

	// get the load path from the table filename
	PathFromFilename(m_szFileName, szLoadDir);
	// make sure the load directory is the active directory
	DWORD err = SetCurrentDirectory(szLoadDir);
	if (err == 0)
		{
		err = GetLastError();
		}

	HWND hwndProgressDialog = NULL;
	//hwndProgressDialog = CreateDialog(g_hinstres, MAKEINTRESOURCE(IDD_PROGRESS), g_pvp->m_hwnd, ProgressProc);
	ResetEvent(g_hProgressWindowStarted);
	// Create the progress dialog on a different thread so that
	// it will get paint messages while this thread is working.
	g_pvp->PostWorkToWorkerThread(CREATE_PROGRESS_WINDOW, (LPARAM)&hwndProgressDialog);
	if (WaitForSingleObject(g_hProgressWindowStarted, 2000) == WAIT_TIMEOUT)
		{
		}
			
	// TEXT
	//SetWindowText(hwndProgressDialog, "Preparing Table");
	ShowWindow(hwndProgressDialog, SW_SHOW);

	HWND hwndProgressBar = GetDlgItem(hwndProgressDialog, IDC_PROGRESS2);
	HWND hwndStatusName = GetDlgItem(hwndProgressDialog, IDC_STATUSNAME);

	SendMessage(hwndProgressBar, PBM_SETPOS, 0, 0);
	// TEXT
	SetWindowText(hwndStatusName, "Compiling Script...");

#ifdef VBA
	ApcProject.GetApcProject()->Compile(&foo);
#endif

	SendMessage(hwndProgressBar, PBM_SETPOS, 20, 0);
	SetWindowText(hwndStatusName, "Backing Up Table State...");
	// Need to check dirty state for cache before allowing running
	// table to make the table look dirty.
	// Only cache disk versions of tables so invalidation
	// can work off of time stamps.
	
	int rendercache;
	HRESULT hr = GetRegInt("Player", "RenderCache", &rendercache);
	if (hr != S_OK)
		{
		rendercache = fTrue; // The default
		}
	BOOL fCheckForCache = (rendercache != 0) && !FDirty();
	BackupForPlay();

	g_fKeepUndoRecords = fFalse;

	m_pcv->m_fScriptError = fFalse;
	m_pcv->Compile();

	if (!m_pcv->m_fScriptError)
		{
		//m_fScriptDirtyBeforePlay = m_fDirtyScript;

		g_pplayer = new Player();
		HRESULT hr = g_pplayer->Init(this, hwndProgressBar, hwndStatusName, fCheckForCache);

		//m_pcv->SetVisible(fFalse);

		c_plungerNormalize  = m_plungerNormalize/1300.0f; 
		c_plungerFilter  = m_plungerFilter == fTrue;

		m_pcv->SetEnabled(fFalse); // Can't edit script while playing

		g_pvp->SetEnableToolbar();
		g_pvp->SetEnableMenuItems();	//>>> added as part of table protection

		if (!m_pcv->m_fScriptError && (hr == S_OK))
			{
			ShowWindow(g_pvp->m_hwndWork, SW_HIDE);
			}
		else
			{
			SendMessage(g_pplayer->m_hwnd, WM_CLOSE, 0, 0);
			}
		}
	else
		{
		RestoreBackup();
		g_fKeepUndoRecords = fTrue;
		m_pcv->EndSession();
		//delete g_pplayer;
		//g_pplayer = NULL;
		}

	g_pvp->PostWorkToWorkerThread(DESTROY_PROGRESS_WINDOW, (LPARAM)hwndProgressDialog);
	//DestroyWindow(hwndProgressDialog);
	//EnableWindow(g_pvp->m_hwndWork, fFalse); // Go modal in our main app window
	}

void PinTable::StopPlaying()
	{
	// Unhook script connections
	//m_pcv->m_pScript->SetScriptState(SCRIPTSTATE_INITIALIZED);

	m_pcv->SetEnabled(fTrue);

	// Stop all sounds
	// In case we were playing any of the main buffers
	int i;
	for (i=0;i<m_vsound.Size();i++)
		{
		m_vsound.ElementAt(i)->m_pDSBuffer->Stop();
		}
	// The usual case - copied sounds
	for (i=0;i<m_voldsound.Size();i++)
		{
		PinSoundCopy *ppsc = m_voldsound.ElementAt(i);
		ppsc->m_pDSBuffer->Stop();
		}
	ClearOldSounds();

	m_pcv->EndSession();

	ShowWindow(g_pvp->m_hwndWork, SW_SHOW);
	//EnableWindow(g_pvp->m_hwndWork, fTrue); // Disable modal state after game ends

	RestoreBackup();

	g_fKeepUndoRecords = fTrue;
	
	BeginAutoSaveCounter();

	//if (m_fScriptDirtyBeforePlay != m_fDirtyScript)
		//{
		//UnsetDirtyScript(); // There might have been changes because of script error (coloring), but the code itself should not have changed
		//}

	//BOOL fFoo = RemoveFontResource("d:\\gdk\\vbatest\\fonts\\LED.ttf");
	}

void PinTable::CreateTableWindow()
	{

	WNDCLASSEX wcex;

	memset(&wcex, 0, sizeof(WNDCLASSEX));

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;//CS_NOCLOSE | CS_OWNDC;
	wcex.lpfnWndProc = TableWndProc;
	wcex.hInstance = g_hinst;
	wcex.lpszClassName = "PinTable";
	wcex.hIcon = LoadIcon(g_hinst, MAKEINTRESOURCE(IDI_TABLEICON));
	wcex.hCursor = NULL;//LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = NULL;
	//wcex.lpszMenuName = MAKEINTRESOURCE(IDR_APPMENU);

	RegisterClassEx(&wcex);

	m_hwnd = ::CreateWindowEx(WS_EX_MDICHILD /*| WS_EX_OVERLAPPEDWINDOW*/,"PinTable",m_szFileName,WS_HSCROLL | WS_VSCROLL | WS_MAXIMIZE | WS_VISIBLE | WS_CHILD | WS_OVERLAPPEDWINDOW/* | WS_MAXIMIZE*/,
			20,20,400,400,m_pvp->m_hwndWork,NULL,g_hinst,0);

	BeginAutoSaveCounter();

	SetWindowLong(m_hwnd, GWL_USERDATA, (long)this);
	}

HRESULT PinTable::InitVBA()
	{
	HRESULT hr = S_OK;
#ifdef VBA
	if (SUCCEEDED(hr = StgCreateDocfile(NULL, STGM_TRANSACTED | STGM_READWRITE
			| STGM_SHARE_EXCLUSIVE | STGM_CREATE | STGM_DELETEONRELEASE, 0, &m_pStg)))
		{
		if (SUCCEEDED(hr = ApcProject.Create(m_pvp->ApcHost, axProjectNormal,
				L"Table")) && SUCCEEDED(hr = ApcProject.InitNew(m_pStg)))
				{
				}
		}

	if (hr != S_OK)
		{
		ShowError("Could not create VBA Project.");
		}

	hr = ApcProjectItem.Define(ApcProject, GetDispatch(), axTypeHostProjectItem, L"Table", NULL);

	if (hr != S_OK)
		{
		ShowError("Could not create VBA ProjectItem Table.");
		}

#endif
	return hr;
	}

void PinTable::CloseVBA()
	{
	//CHECK_SIGNATURE(SIG_Game);
#ifdef VBA
	ApcProject.Close();
#endif
	}

HRESULT PinTable::TableSave()
	{
	BOOL fSaveAs = (!m_szFileName[0]);
	return Save(fSaveAs);
	}

HRESULT PinTable::SaveAs()
	{
	return Save(fTrue);
	}

HRESULT PinTable::ApcProject_Save()
	{
	BOOL fSaveAs = (!m_szFileName[0]);
	return Save(fSaveAs);
	}

void PinTable::BeginAutoSaveCounter()
	{
	if (g_pvp->m_autosaveTime > 0)
		{
		SetTimer(m_hwnd, TIMER_ID_AUTOSAVE, g_pvp->m_autosaveTime, NULL);
		}
	}
	
void PinTable::EndAutoSaveCounter()
	{
	KillTimer(m_hwnd, TIMER_ID_AUTOSAVE);
	}

void PinTable::AutoSave()
	{
	if ((m_sdsCurrentDirtyState <= eSaveAutosaved) || CheckPermissions(DISABLE_TABLE_SAVE))
		{
		return;
		}

	KillTimer(m_hwnd, TIMER_ID_AUTOSAVE);
	//MessageBox(m_hwnd, "Auto Saving", NULL, 0);

	HRESULT hr;

	{
		LocalString ls(IDS_AUTOSAVING);
		g_pvp->SetActionCur(ls.m_szbuffer);
		g_pvp->SetCursorCur(NULL, IDC_WAIT);
	}
	
	/*ILockBytes *pilb;
	HRESULT hr = CreateILockBytesOnHGlobal(NULL, TRUE, &pilb);
	IStorage *pstgroot;

	hr = StgCreateDocfileOnILockBytes(pilb,STGM_DIRECT | STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,0,&pstgroot);*/

	FastIStorage *pstgroot;

	pstgroot = new FastIStorage();
	pstgroot->AddRef();
	
	hr = SaveToStorage(pstgroot);

	//MessageBox(m_hwnd, "Done", NULL, 0);

	m_undo.SetCleanPoint(min(m_sdsDirtyProp, eSaveAutosaved));
	m_pcv->SetClean(min(m_sdsDirtyScript, eSaveAutosaved));
	SetNonUndoableDirty(min(m_sdsNonUndoableDirty, eSaveAutosaved));

	AutoSavePackage *pasp = new AutoSavePackage();
	pasp->pstg = pstgroot;
	pasp->tableindex = g_pvp->m_vtable.IndexOf(this);
	pasp->HwndTable = m_hwnd;
	
	HANDLE hEvent;

	if (hr == S_OK)
		{
		hEvent = g_pvp->PostWorkToWorkerThread(COMPLETE_AUTOSAVE, (LPARAM)pasp);
		m_vAsyncHandles.AddElement(hEvent);

		g_pvp->SetActionCur("Completing AutoSave");
		}
	else
		{
		g_pvp->SetActionCur("");
		}

	g_pvp->SetCursorCur(NULL, IDC_ARROW);

	//pstgroot->Release();
	//pilb->Release();
	}

HRESULT PinTable::Save(BOOL fSaveAs)
	{
	IStorage* pstgRoot;
	HRESULT hr = S_OK;

#ifdef VBA
	pstgRoot = m_pStg;
#endif

	// Get file name if needed
	if(fSaveAs)
		{
		//need to get a file name
		OPENFILENAME ofn;
		memset(&ofn, 0, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hInstance = g_hinst;
		ofn.hwndOwner = g_pvp->m_hwnd;
		// TEXT
		ofn.lpstrFilter = "Visual Pinball Tables (*.vpt)\0*.vpt\0";
		ofn.lpstrFile = m_szFileName;
		ofn.nMaxFile = _MAX_PATH;
		ofn.lpstrDefExt = "vpt";
		ofn.Flags = OFN_OVERWRITEPROMPT;

		char szInitialDir[1024];
		HRESULT hr = GetRegString("RecentDir","LoadDir", szInitialDir, 1024);
		if (hr == S_OK)
			{
			ofn.lpstrInitialDir = szInitialDir;
			}
		else
			{
			char szFoo[MAX_PATH];
			lstrcpy(szFoo, g_pvp->m_szMyPath);
			lstrcat(szFoo, "Tables");
			ofn.lpstrInitialDir = szFoo;
			}

/*#ifdef VBA
		g_pvp->ApcHost->BeginModalDialog();
#endif*/
		int ret = GetSaveFileName(&ofn);
/*#ifdef VBA
		g_pvp->ApcHost->EndModalDialog();
#endif*/

		// user cancelled
		if(ret == 0)
			return S_FALSE;

		strcpy(szInitialDir, m_szFileName);
		szInitialDir[ofn.nFileOffset] = 0;
		hr = SetRegValue("RecentDir","LoadDir", REG_SZ, szInitialDir, strlen(szInitialDir));

			{
			MAKE_WIDEPTR_FROMANSI(wszCodeFile, m_szFileName);
			if(FAILED(hr = StgCreateDocfile(wszCodeFile, STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE,
				0, &pstgRoot)))
				{
				LocalString ls(IDS_SAVEERROR);
				MessageBox(m_hwnd, ls.m_szbuffer, "Visual Pinball", MB_ICONERROR);
				goto Error;
				}
			}

		TitleFromFilename(m_szFileName, m_szTitle);
		SetCaption(m_szTitle);
		}
	else
		{
/*#ifdef VBA
		pstgRoot = m_pStg;
#else*/
			{
			MAKE_WIDEPTR_FROMANSI(wszCodeFile, m_szFileName);
			if(FAILED(hr = StgCreateDocfile(wszCodeFile, STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE,
				0, &pstgRoot)))
				{
				LocalString ls(IDS_SAVEERROR);
				MessageBox(m_hwnd, ls.m_szbuffer, "Visual Pinball", MB_ICONERROR);
				goto Error;
				}
			}
//#endif
		}

	{
		LocalString ls(IDS_SAVING);
		g_pvp->SetActionCur(ls.m_szbuffer);
		g_pvp->SetCursorCur(NULL, IDC_WAIT);
	}

	hr = SaveToStorage(pstgRoot);

	if(FAILED(hr)) goto Error;

/*#ifdef VBA
	// Save App Data

	if (SUCCEEDED(hr = ApcProject.Save(pstgRoot, !fSaveAs)))
		{
		if (SUCCEEDED(hr = ApcProject.SaveCompleted(fSaveAs ? pstgRoot : NULL)))
			{
			pstgRoot->Commit(STGC_DEFAULT);
			}
		else
			{
			pstgRoot->Revert();
			}
		}

	if(FAILED(hr)) goto Error;

	if(pstgRoot != m_pStg)
		{
		m_pStg->Release();
		m_pStg = pstgRoot;
		}
#else*/
	pstgRoot->Commit(STGC_DEFAULT);
	pstgRoot->Release();
//#endif

	g_pvp->SetActionCur("");
	g_pvp->SetCursorCur(NULL, IDC_ARROW);

/*#ifdef VBA
	MAKE_WIDEPTR_FROMANSI(wszFileName, m_szFileName);
	BSTR bstrFileName = SysAllocString(wszFileName);
	ApcProject->APC_PUT(DisplayName)(bstrFileName);
	SysFreeString(bstrFileName);
#endif*/

	m_undo.SetCleanPoint(eSaveClean);
	m_pcv->SetClean(eSaveClean);
	SetNonUndoableDirty(eSaveClean);

Error:
/*#ifdef VBA
	if(FAILED(hr))
	{
		g_pvp->ApcHost->APC_RAW(BeginModalDialog)();
		g_pvp->ApcHost->APC_RAW(ShowError)(hr);
		g_pvp->ApcHost->APC_RAW(EndModalDialog)();
	}
#endif*/

	return hr;
	}

HRESULT PinTable::SaveToStorage(IStorage *pstgRoot)
	{
	IStorage *pstgData, *pstgInfo;
	IStream *pstmGame, *pstmItem;
	HRESULT hr;

	RECT rc;
	SendMessage(g_pvp->m_hwndStatusBar, SB_GETRECT, 2, (long)&rc);

	HWND hwndProgressBar = CreateWindowEx(0, PROGRESS_CLASS, (LPSTR) NULL,
		WS_CHILD | WS_VISIBLE, rc.left,
		rc.top,
		rc.right-rc.left, rc.bottom-rc.top,
		g_pvp->m_hwndStatusBar, (HMENU) 0, g_hinst, NULL);

	SendMessage(hwndProgressBar, PBM_SETPOS, 1, 0);

	//////////////// Begin Encryption
	HCRYPTPROV hcp;
	HCRYPTHASH hch;
	HCRYPTKEY  hkey;
	HCRYPTHASH hchkey;

	int foo;

	hcp = NULL;
	hch = NULL;
	hkey = NULL;
	hchkey = NULL;

	foo = CryptAcquireContext(&hcp, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_NEWKEYSET/* | CRYPT_SILENT*/);

	//foo = CryptGenKey(hcp, CALG_RC2, CRYPT_EXPORTABLE, &hkey);

	foo = GetLastError();

	foo = CryptCreateHash(hcp, CALG_MD2/*CALG_MAC*//*CALG_HMAC*/, NULL/*hkey*/, 0, &hch);

	foo = GetLastError();

	BYTE hashval[256];
	DWORD hashlen;
	hashlen = 256;

	foo = CryptHashData(hch, (BYTE *)TABLE_KEY, 14, 0);

	foo = GetLastError();

	// create a key hash (we have to use a second hash as deriving a key from the
	// integrity hash actually modifies it and thus it calculates the wrong hash)
	foo = CryptCreateHash(hcp, CALG_MD5, NULL, 0, &hchkey);
	foo = GetLastError();
	// hash the password
	foo = CryptHashData(hchkey, (BYTE *)TABLE_KEY, 14, 0);
	foo = GetLastError();
	// Create a block cipher session key based on the hash of the password.
	foo = CryptDeriveKey(hcp, CALG_RC2, hchkey, CRYPT_EXPORTABLE | 0x00280000, &hkey);
	foo = GetLastError();

	////////////// End Encryption

	int ctotalitems = m_vedit.Size() + m_vsound.Size() + m_vimage.Size() + m_vfont.Size() + m_vcollection.Size();
	int csaveditems = 0;

	SendMessage(hwndProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, ctotalitems));

	//first save our own data
	if(SUCCEEDED(hr = pstgRoot->CreateStorage(L"GameStg", STGM_DIRECT/*STGM_TRANSACTED*/ | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstgData)))
		{
		if(SUCCEEDED(hr = pstgData->CreateStream(L"GameData", STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmGame)))
			{
			if(SUCCEEDED(hr = pstgData->CreateStream(L"Version", STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmItem)))
				{
				ULONG writ;
				int version = CURRENT_FILE_FORMAT_VERSION; //0.3
				CryptHashData(hch, (BYTE *)&version, sizeof(version), 0);
				pstmItem->Write(&version, sizeof(version), &writ);
				pstmItem->Release();
				pstmItem = NULL;
				//if(FAILED(hr)) goto Error;
				}

			if(SUCCEEDED(hr = pstgRoot->CreateStorage(L"TableInfo",STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstgInfo)))
				{
				SaveInfo(pstgInfo, hch);

				if(SUCCEEDED(hr = pstgData->CreateStream(L"CustomInfoTags", STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmItem)))
					{
					SaveCustomInfo(pstgInfo, pstmItem, hch);
					pstmItem->Release();
					pstmItem = NULL;
					}

				pstgInfo->Release();
				}

			if(SUCCEEDED(hr = SaveData(pstmGame, hch, hkey)))
				{
				char szSuffix[32], szStmName[64];

				for(int i = 0; i < m_vedit.Size(); i++)
					{
					strcpy(szStmName, "GameItem");
					_itoa(i, szSuffix, 10);
					strcat(szStmName, szSuffix);

					MAKE_WIDEPTR_FROMANSI(wszStmName, szStmName);

					if(SUCCEEDED(hr = pstgData->CreateStream(wszStmName, STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmItem)))
						{
						ULONG writ;
						IEditable *piedit = m_vedit.ElementAt(i);
						ItemTypeEnum type = piedit->GetItemType();
						pstmItem->Write(&type, sizeof(int), &writ);
						hr = piedit->SaveData(pstmItem, hch, hkey);
						pstmItem->Release();
						pstmItem = NULL;
						//if(FAILED(hr)) goto Error;
						}

					csaveditems++;
					SendMessage(hwndProgressBar, PBM_SETPOS, csaveditems, 0);
					}

				for (int i=0;i<m_vsound.Size();i++)
					{
					strcpy(szStmName, "Sound");
					_itoa(i, szSuffix, 10);
					strcat(szStmName, szSuffix);

					MAKE_WIDEPTR_FROMANSI(wszStmName, szStmName);

					if(SUCCEEDED(hr = pstgData->CreateStream(wszStmName, STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmItem)))
						{
						SaveSoundToStream(m_vsound.ElementAt(i), pstmItem);
						pstmItem->Release();
						pstmItem = NULL;
						}

					csaveditems++;
					SendMessage(hwndProgressBar, PBM_SETPOS, csaveditems, 0);
					}

				for (int i=0;i<m_vimage.Size();i++)
					{
					strcpy(szStmName, "Image");
					_itoa(i, szSuffix, 10);
					strcat(szStmName, szSuffix);

					MAKE_WIDEPTR_FROMANSI(wszStmName, szStmName);

					if(SUCCEEDED(hr = pstgData->CreateStream(wszStmName, STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmItem)))
						{
						m_vimage.ElementAt(i)->SaveToStream(pstmItem, this);
						//SaveImageToStream(m_vimage.ElementAt(i), pstmItem);
						pstmItem->Release();
						pstmItem = NULL;
						}

					csaveditems++;
					SendMessage(hwndProgressBar, PBM_SETPOS, csaveditems, 0);
					}

				for (int i=0;i<m_vfont.Size();i++)
					{
					strcpy(szStmName, "Font");
					_itoa(i, szSuffix, 10);
					strcat(szStmName, szSuffix);

					MAKE_WIDEPTR_FROMANSI(wszStmName, szStmName);

					if(SUCCEEDED(hr = pstgData->CreateStream(wszStmName, STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmItem)))
						{
						m_vfont.ElementAt(i)->SaveToStream(pstmItem);
						pstmItem->Release();
						pstmItem = NULL;
						}

					csaveditems++;
					SendMessage(hwndProgressBar, PBM_SETPOS, csaveditems, 0);
					}

				for (int i=0;i<m_vcollection.Size();i++)
					{
					strcpy(szStmName, "Collection");
					_itoa(i, szSuffix, 10);
					strcat(szStmName, szSuffix);

					MAKE_WIDEPTR_FROMANSI(wszStmName, szStmName);

					if(SUCCEEDED(hr = pstgData->CreateStream(wszStmName, STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmItem)))
						{
						m_vcollection.ElementAt(i)->SaveData(pstmItem, hch, hkey);
						pstmItem->Release();
						pstmItem = NULL;
						}

					csaveditems++;
					SendMessage(hwndProgressBar, PBM_SETPOS, csaveditems, 0);
					}
				}
			pstmGame->Release();
			}

		// Authentication block
		foo = CryptGetHashParam(hch, HP_HASHSIZE, hashval, &hashlen, 0);

		hashlen = 256;
		foo = CryptGetHashParam(hch, HP_HASHVAL, hashval, &hashlen, 0);

		if(SUCCEEDED(hr = pstgData->CreateStream(L"MAC", STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmItem)))
			{
			ULONG writ;
			int version = CURRENT_FILE_FORMAT_VERSION; //0.3
			pstmItem->Write(hashval, hashlen, &writ);
			pstmItem->Release();
			pstmItem = NULL;
			//if(FAILED(hr)) goto Error;

			//CryptExportKey(hkey, NULL, PUBLICKEYBLOB, 0, BYTE *pbData, DWORD *pdwDataLen);
			}

		foo = CryptDestroyHash(hch);

		foo = CryptDestroyHash(hchkey);

		foo = CryptDestroyKey(hkey);

		foo = CryptReleaseContext(hcp, 0);
		//////// End Authentication block

		if(SUCCEEDED(hr))
			pstgData->Commit(STGC_DEFAULT);
		else
			{
			pstgData->Revert();
			pstgData->Release();
			pstgRoot->Revert();
			LocalString ls(IDS_SAVEERROR);
			MessageBox(m_hwnd, ls.m_szbuffer, "Visual Pinball", MB_ICONERROR);
			goto Error;
			}
		pstgData->Release();
		}

Error:

	DestroyWindow(hwndProgressBar);

	return hr;
	}

HRESULT PinTable::SaveSoundToStream(PinSound *pps, IStream *pstm)
	{
	int len;
	ULONG writ = 0;
	HRESULT hr = S_OK;

	len = lstrlen(pps->m_szName);

	if(FAILED(hr = pstm->Write(&len, sizeof(int), &writ)))
		return hr;

	if(FAILED(hr = pstm->Write(pps->m_szName, len, &writ)))
		return hr;

	len = lstrlen(pps->m_szPath);

	if(FAILED(hr = pstm->Write(&len, sizeof(int), &writ)))
		return hr;

	if(FAILED(hr = pstm->Write(pps->m_szPath, len, &writ)))
		return hr;

	len = lstrlen(pps->m_szInternalName);

	if(FAILED(hr = pstm->Write(&len, sizeof(int), &writ)))
		return hr;

	if(FAILED(hr = pstm->Write(pps->m_szInternalName, len, &writ)))
		return hr;

	WAVEFORMATEX wfx;

	pps->m_pDSBuffer->GetFormat(&wfx, sizeof(wfx), NULL);

	if(FAILED(hr = pstm->Write(&wfx, sizeof(wfx), &writ)))
		return hr;

	if(FAILED(hr = pstm->Write(&pps->m_cdata, sizeof(int), &writ)))
		return hr;

	//LZWWriter lzwwriter(pstm, (int *)pps->m_pdata, pps->m_cdata, 1, pps->m_cdata);

	//lzwwriter.CompressBits(8+1);

	if(FAILED(hr = pstm->Write(pps->m_pdata, pps->m_cdata, &writ)))
		return hr;

	return S_OK;
	}

HRESULT PinTable::LoadSoundFromStream(IStream *pstm)
	{
	PinSound *pps;
	int len;
	ULONG read = 0;
	HRESULT hr = S_OK;
	WAVEFORMATEX wfx;

	pps = new PinSound();

	if(FAILED(hr = pstm->Read(&len, sizeof(len), &read)))
		return hr;

	if(FAILED(hr = pstm->Read(pps->m_szName, len, &read)))
		return hr;

	pps->m_szName[len] = 0;

	if(FAILED(hr = pstm->Read(&len, sizeof(len), &read)))
		return hr;

	if(FAILED(hr = pstm->Read(pps->m_szPath, len, &read)))
		return hr;

	pps->m_szPath[len] = 0;

	if(FAILED(hr = pstm->Read(&len, sizeof(len), &read)))
		return hr;

	if(FAILED(hr = pstm->Read(pps->m_szInternalName, len, &read)))
		return hr;

	pps->m_szInternalName[len] = 0;

	if(FAILED(hr = pstm->Read(&wfx, sizeof(wfx), &read)))
		return hr;

	if(FAILED(hr = pstm->Read(&pps->m_cdata, sizeof(int), &read)))
		return hr;

	pps->m_pdata = new char [pps->m_cdata];

	//LZWReader lzwreader(pstm, (int *)pps->m_pdata, pps->m_cdata, 1, pps->m_cdata);

	//lzwreader.Decoder();

	if(FAILED(hr = pstm->Read(pps->m_pdata, pps->m_cdata, &read)))
		return hr;

	if (g_pvp->m_pds.CreateDirectFromNative(pps, &wfx) == S_OK)
		{
		m_vsound.AddElement(pps);
		}

	return S_OK;
	}

HRESULT PinTable::WriteInfoValue(IStorage* pstg, WCHAR *wzName, char *szValue, HCRYPTHASH hcrypthash)
	{
	HRESULT hr;
	IStream *pstm;

	if(szValue && SUCCEEDED(hr = pstg->CreateStream(wzName, STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstm)))
		{
		ULONG writ;
		BiffWriter bw(pstm, hcrypthash, NULL);

		int len = lstrlen(szValue);
		WCHAR *wzT = new WCHAR[len+1];
		MultiByteToWideChar(CP_ACP, 0, szValue, -1, wzT, len+1);

		bw.WriteBytes(wzT, len*sizeof(WCHAR), &writ);
		//delete bw;
		//pstm->Write(szValue, lstrlen(szValue), &writ);
		delete wzT;
		pstm->Release();
		pstm = NULL;
		}

	return S_OK;
	}

HRESULT PinTable::SaveInfo(IStorage* pstg, HCRYPTHASH hcrypthash)
	{
	WriteInfoValue(pstg, L"TableName", m_szTableName, hcrypthash);
	WriteInfoValue(pstg, L"AuthorName", m_szAuthor, hcrypthash);
	WriteInfoValue(pstg, L"TableVersion", m_szVersion, hcrypthash);
	WriteInfoValue(pstg, L"ReleaseDate", m_szReleaseDate, hcrypthash);
	WriteInfoValue(pstg, L"AuthorEmail", m_szAuthorEMail, hcrypthash);
	WriteInfoValue(pstg, L"AuthorWebSite", m_szWebSite, hcrypthash);
	WriteInfoValue(pstg, L"TableBlurb", m_szBlurb, hcrypthash);
	WriteInfoValue(pstg, L"TableDescription", m_szDescription, hcrypthash);
	WriteInfoValue(pstg, L"TableRules", m_szRules, hcrypthash);

	PinImage *pin = GetImage(m_szScreenShot);
	if (pin != NULL && pin->m_ppb != NULL)
		{
		IStream *pstm;
		HRESULT hr;
		ULONG writ;

		if(SUCCEEDED(hr = pstg->CreateStream(L"Screenshot", STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstm)))
			{
			BiffWriter bw(pstm, hcrypthash, NULL);
			bw.WriteBytes(pin->m_ppb->m_pdata, pin->m_ppb->m_cdata, &writ);
			pstm->Release();
			pstm = NULL;
			}
		}

	pstg->Commit(STGC_DEFAULT);

	return S_OK;
	}

HRESULT PinTable::SaveCustomInfo(IStorage* pstg, IStream *pstmTags, HCRYPTHASH hcrypthash)
	{
	int i;
	BiffWriter bw(pstmTags, hcrypthash, NULL);

	for (i=0;i<m_vCustomInfoTag.Size();i++)
		{
		bw.WriteString(FID(CUST), m_vCustomInfoTag.ElementAt(i));
		}


	bw.WriteTag(FID(ENDB));

	for (i=0;i<m_vCustomInfoTag.Size();i++)
		{
		char *szName = m_vCustomInfoTag.ElementAt(i);
		int len = lstrlen(szName);
		WCHAR *wzName = new WCHAR[len+1];
		MultiByteToWideChar(CP_ACP, 0, szName, -1, wzName, len+1);

		WriteInfoValue(pstg, wzName, m_vCustomInfoContent.ElementAt(i), hcrypthash);

		delete wzName;
		}

	pstg->Commit(STGC_DEFAULT);

	return S_OK;
	}

HRESULT PinTable::ReadInfoValue(IStorage* pstg, WCHAR *wzName, char **pszValue, HCRYPTHASH hcrypthash)
	{
	HRESULT hr;
	IStream *pstm;

	if(SUCCEEDED(hr = pstg->OpenStream(wzName, NULL, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstm)))
		{
		STATSTG ss;
		pstm->Stat(&ss, STATFLAG_NONAME);
		int len = ss.cbSize.LowPart / sizeof(WCHAR);

		WCHAR *wzT = new WCHAR[len+1];
		*pszValue = new char[len+1];

		ULONG read;
		BiffReader br(pstm, NULL, NULL, 0, hcrypthash, NULL);
		br.ReadBytes(wzT, ss.cbSize.LowPart, &read);
		wzT[len] = L'\0';

		WideCharToMultiByte(CP_ACP, 0, wzT, -1, *pszValue, len+1, NULL, NULL);

		//delete br;
		//pstm->Read(*pszValue, ss.cbSize.LowPart, &read);
		
		delete wzT;
		pstm->Release();
		}

	return S_OK;
	}

HRESULT PinTable::LoadInfo(IStorage* pstg, HCRYPTHASH hcrypthash, int version)
	{
	ReadInfoValue(pstg, L"TableName", &m_szTableName, hcrypthash);
	ReadInfoValue(pstg, L"AuthorName", &m_szAuthor, hcrypthash);
	ReadInfoValue(pstg, L"TableVersion", &m_szVersion, hcrypthash);
	ReadInfoValue(pstg, L"ReleaseDate", &m_szReleaseDate, hcrypthash);
	ReadInfoValue(pstg, L"AuthorEmail", &m_szAuthorEMail, hcrypthash);
	ReadInfoValue(pstg, L"AuthorWebSite", &m_szWebSite, hcrypthash);
	ReadInfoValue(pstg, L"TableBlurb", &m_szBlurb, hcrypthash);
	ReadInfoValue(pstg, L"TableDescription", &m_szDescription, hcrypthash);
	ReadInfoValue(pstg, L"TableRules", &m_szRules, hcrypthash);

	HRESULT hr;
	IStream *pstm;

	if(SUCCEEDED(hr = pstg->OpenStream(L"Screenshot", NULL, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstm)))
		{
		STATSTG ss;
		pstm->Stat(&ss, STATFLAG_NONAME);
		//char *pdata = new char[ss.cbSize.LowPart];
		m_pbTempScreenshot = new PinBinary();

		m_pbTempScreenshot->m_cdata = ss.cbSize.LowPart;
		m_pbTempScreenshot->m_pdata = new char[m_pbTempScreenshot->m_cdata];

		//m_pbTempScreenshot->LoadFromStream(pstm, version);

		ULONG read;
		BiffReader br(pstm, NULL, NULL, 0, hcrypthash, NULL);
		br.ReadBytes(m_pbTempScreenshot->m_pdata, m_pbTempScreenshot->m_cdata, &read);

		//delete pdata;

		pstm->Release();
		}

	return S_OK;
	}

HRESULT PinTable::LoadCustomInfo(IStorage* pstg, IStream *pstmTags, HCRYPTHASH hcrypthash, int version)
	{
	BiffReader br(pstmTags, this, NULL, version, hcrypthash, NULL);
	br.Load();

	int i;
	for (i=0;i<m_vCustomInfoTag.Size();i++)
		{
		char *szName = m_vCustomInfoTag.ElementAt(i);
		char *szValue;
		int len = lstrlen(szName);
		WCHAR *wzName = new WCHAR[len+1];
		MultiByteToWideChar(CP_ACP, 0, szName, -1, wzName, len+1);

		ReadInfoValue(pstg, wzName, &szValue, hcrypthash);
		m_vCustomInfoContent.AddElement(szValue);

		delete wzName;
		}

	return S_OK;
	}

HRESULT PinTable::SaveData(IStream* pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
	BiffWriter bw(pstm, hcrypthash, hcryptkey);

#ifdef VBA
	bw.WriteInt(FID(PIID), ApcProjectItem.ID());
#endif

/////////////////
	bw.WriteInt(FID(MPGC), m_plungerNormalize);
	bw.WriteBool(FID(MPDF), m_plungerFilter);

	bw.WriteBool(FID(ACEL), m_tblAccelerometer);
	bw.WriteBool(FID(AORD), m_tblAccelNormalMount);
	bw.WriteFloat(FID(AANG), m_tblAccelAngle);
	bw.WriteFloat(FID(AAMP), m_tblAccelAmp);
	bw.WriteFloat(FID(AMAMP), m_tblAccelManualAmp);

/////////////////

	bw.WriteFloat(FID(GRSZ), m_gridsize);
	bw.WriteFloat(FID(LEFT), m_left);
	bw.WriteFloat(FID(TOPX), m_top);
	bw.WriteFloat(FID(RGHT), m_right);
	bw.WriteFloat(FID(BOTM), m_bottom);

	bw.WriteFloat(FID(OFFX), m_offsetx);
	bw.WriteFloat(FID(OFFY), m_offsety);

	bw.WriteFloat(FID(ZOOM), m_zoom);

	bw.WriteFloat(FID(INCL), m_inclination);
	bw.WriteFloat(FID(FOVX), m_FOV);
	bw.WriteFloat(FID(SLOP), m_angletilt);

	bw.WriteString(FID(IMAG), m_szImage);
	bw.WriteString(FID(BIMG), m_szImageBackdrop);
	bw.WriteString(FID(BLIM), m_szBallImage);
	bw.WriteString(FID(BLIF), m_szBallImageFront);
	bw.WriteString(FID(BLIB), m_szBallImageBack);

	bw.WriteString(FID(SSHT), m_szScreenShot);
	

	bw.WriteBool(FID(FGRD), m_fGrid);
	bw.WriteBool(FID(FBCK), m_fBackdrop);

	bw.WriteFloat(FID(GLAS), m_glassheight);

	bw.WriteInt(FID(COLR), m_colorplayfield);
	bw.WriteInt(FID(BCLR), m_colorbackdrop);

	bw.WriteBool(FID(DSHD), m_fRenderShadows);
	
	bw.WriteInt(FID(PHYS), m_PhysicsType);

	// HACK!!!! - Don't save special values when copying for undo.  For instance, don't reset the code.
	// Someday save these values into there own stream, used only when saving to file.
	if (hcrypthash != 0)
		{
		bw.WriteInt(FID(SEDT), m_vedit.Size());
		bw.WriteInt(FID(SSND), m_vsound.Size());
		bw.WriteInt(FID(SIMG), m_vimage.Size());
		bw.WriteInt(FID(SFNT), m_vfont.Size());
		bw.WriteInt(FID(SCOL), m_vcollection.Size());

		bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);

		bw.WriteStruct(FID(CCUS), rgcolorcustom, sizeof(COLORREF)*16);

		bw.WriteStruct(FID(SECB), &m_protectionData, sizeof(_protectionData));

		// save the script source code
		bw.WriteTag(FID(CODE));
		// if the script is protected then we pass in the proper cyptokey into the code savestream
		if (CheckPermissions(DISABLE_SCRIPT_EDITING) == fTrue)
			{
			m_pcv->SaveToStream(pstm, hcrypthash, hcryptkey);
			}
		else
			{
			m_pcv->SaveToStream(pstm, hcrypthash, NULL);
			}
		}

	bw.WriteTag(FID(ENDB));

	return S_OK;
	/*ULONG writ = 0;
	HRESULT hr = S_OK;

	int size = m_vedit.Size();

	if(FAILED(hr = pstm->Write(&size, sizeof(int), &writ)))
		return hr;

	if(FAILED(hr = pstm->Write(&m_gridsize, sizeof m_gridsize, &writ)))
		return hr;

	DWORD dwID = ApcProjectItem.ID();
	if(FAILED(hr = pstm->Write(&dwID, sizeof dwID, &writ)))
		return hr;

	if(FAILED(hr = pstm->Write(&m_left, sizeof m_left, &writ)))
		return hr;

	if(FAILED(hr = pstm->Write(&m_top, sizeof m_top, &writ)))
		return hr;

	if(FAILED(hr = pstm->Write(&m_right, sizeof m_right, &writ)))
		return hr;

	if(FAILED(hr = pstm->Write(&m_bottom, sizeof m_bottom, &writ)))
		return hr;

	if(FAILED(hr = pstm->Write(&m_offsetx, sizeof m_offsetx, &writ)))
		return hr;

	if(FAILED(hr = pstm->Write(&m_offsety, sizeof m_offsety, &writ)))
		return hr;

	if(FAILED(hr = pstm->Write(&m_zoom, sizeof m_zoom, &writ)))
		return hr;

	if(FAILED(hr = pstm->Write(&m_inclination, sizeof m_inclination, &writ)))
		return hr;

	if(FAILED(hr = pstm->Write(&m_FOV, sizeof m_FOV, &writ)))
		return hr;

	size = m_vsound.Size();

	if(FAILED(hr = pstm->Write(&size, sizeof size, &writ)))
		return hr;

	size = m_vimage.Size();

	if(FAILED(hr = pstm->Write(&size, sizeof size, &writ)))
		return hr;

	if(FAILED(hr = pstm->Write(m_szImage, MAXTOKEN, &writ)))
		return hr;

	if(FAILED(hr = pstm->Write(&m_fGrid, sizeof(m_fGrid), &writ)))
		return hr;

	if(FAILED(hr = pstm->Write(&m_fBackdrop, sizeof(m_fBackdrop), &writ)))
		return hr;

	return hr;*/
}

HRESULT PinTable::LoadGameFromFilename(char *szFileName)
	{
	IStorage* pstgRoot;
	HRESULT hr = S_OK;

	//ASSERT(*szFileName, "Empty File Name String!");

	strcpy(m_szFileName, szFileName);
		{
		MAKE_WIDEPTR_FROMANSI(wszCodeFile, szFileName);
		if(FAILED(hr = StgOpenStorage(wszCodeFile, NULL, STGM_TRANSACTED | STGM_READ | STGM_SHARE_EXCLUSIVE, NULL, 0, &pstgRoot)))
			{
			// TEXT
			MessageBox(g_pvp->m_hwnd, "Error loading file.", "Load Error", 0);
			return hr;
			}
		}

	return LoadGameFromStorage(pstgRoot);
	}

HRESULT PinTable::LoadGameFromStorage(IStorage *pstgRoot)
{
	IStorage *pstgData, *pstgInfo;
	IStream *pstmGame, *pstmItem, *pstmVersion;
	HRESULT hr = S_OK;

	int ctotalitems;
	int cloadeditems;

	/*HWND hwndProgressDialog;
	hwndProgressDialog = CreateDialog(g_hinst, MAKEINTRESOURCE(IDD_PROGRESS), g_pvp->m_hwnd, ProgressProc);
	SetWindowText(hwndProgressDialog, "Loading Table");
	ShowWindow(hwndProgressDialog, SW_SHOW);
	HWND hwndProgressBar = GetDlgItem(hwndProgressDialog, IDC_PROGRESS2);
	HWND hwndStatusName = GetDlgItem(hwndProgressDialog, IDC_STATUSNAME);*/

	RECT rc;
	SendMessage(g_pvp->m_hwndStatusBar, SB_GETRECT, 2, (long)&rc);

	HWND hwndProgressBar = CreateWindowEx(0, PROGRESS_CLASS, (LPSTR) NULL,
        WS_CHILD | WS_VISIBLE, rc.left,
        rc.top,
        rc.right-rc.left, rc.bottom-rc.top,
        g_pvp->m_hwndStatusBar, (HMENU) 0, g_hinst, NULL);

	//SetWindowText(hwndStatusName, "Opening File...");
	SendMessage(hwndProgressBar, PBM_SETPOS, 1, 0);

	LocalString ls(IDS_LOADING);
	g_pvp->SetActionCur(ls.m_szbuffer);
	g_pvp->SetCursorCur(NULL, IDC_WAIT);

	HCRYPTPROV hcp = NULL;
	HCRYPTHASH hch = NULL;
	HCRYPTHASH hchkey = NULL;
	HCRYPTKEY  hkey = NULL;

	///////// Begin MAC
	int foo;

	foo = CryptAcquireContext(&hcp, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_NEWKEYSET/* | CRYPT_SILENT*/);

	//foo = CryptGenKey(hcp, CALG_RC2, CRYPT_EXPORTABLE, &hkey);

	foo = GetLastError();

	foo = CryptCreateHash(hcp, CALG_MD2/*CALG_MAC*//*CALG_HMAC*/, NULL/*hkey*/, 0, &hch);

	foo = GetLastError();

	BYTE hashval[256];
	DWORD hashlen = 256;

	foo = CryptHashData(hch, (BYTE *)TABLE_KEY, 14, 0);

	foo = GetLastError();

	// create a key hash (we have to use a second hash as deriving a key from the
	// integrity hash actually modifies it and thus it calculates the wrong hash)
	foo = CryptCreateHash(hcp, CALG_MD5, NULL, 0, &hchkey);
	foo = GetLastError();
	// hash the password
	foo = CryptHashData(hchkey, (BYTE *)TABLE_KEY, 14, 0);
	foo = GetLastError();
	// Create a block cipher session key based on the hash of the password.
	// We need to figure out the file verison before we can create the key

	////////////// End MAC

	//load our stuff first
	if(SUCCEEDED(hr = pstgRoot->OpenStorage(L"GameStg", NULL, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, NULL, 0, &pstgData)))
		{
		if(SUCCEEDED(hr = pstgData->OpenStream(L"GameData", NULL, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmGame)))
			{
			int csubobj = 0;
			int csounds = 0;
			int ctextures = 0;
			int cfonts = 0;
			int ccollection = 0;
			int version = CURRENT_FILE_FORMAT_VERSION; //.3

			if(SUCCEEDED(hr = pstgData->OpenStream(L"Version", NULL, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmVersion)))
				{
				ULONG read;
				hr = pstmVersion->Read(&version, sizeof(int), &read);
				CryptHashData(hch, (BYTE *)&version, sizeof(int), 0);
				pstmVersion->Release();
				if (version >= BEYOND_FILE_FORMAT_VERSION)
					{
					LocalString ls(IDS_WRONGFILEVERSION);
					ShowError(ls.m_szbuffer);
					pstgRoot->Release();
					pstmGame->Release();
					pstgData->Release();
					DestroyWindow(hwndProgressBar);
					g_pvp->SetCursorCur(NULL, IDC_ARROW);
					}

				// Create a block cipher session key based on the hash of the password.
				if (version == 600)
					{
					CryptDeriveKey(hcp, CALG_RC2, hchkey, CRYPT_EXPORTABLE, &hkey);
					}
				else
					{
					CryptDeriveKey(hcp, CALG_RC2, hchkey, CRYPT_EXPORTABLE | 0x00280000, &hkey);
					}
				}

			if(SUCCEEDED(hr = pstgRoot->OpenStorage(L"TableInfo", NULL, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, NULL, 0, &pstgInfo)))
				{
				LoadInfo(pstgInfo, hch, version);
				if(SUCCEEDED(hr = pstgData->OpenStream(L"CustomInfoTags", NULL, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmItem)))
					{
					LoadCustomInfo(pstgInfo, pstmItem, hch, version);
					pstmItem->Release();
					pstmItem = NULL;
					}
				pstgInfo->Release();
				}

			if(SUCCEEDED(hr = LoadData(pstmGame, csubobj, csounds, ctextures, cfonts, ccollection, version, hch, hkey)))
				{

				ctotalitems = csubobj + csounds + ctextures + cfonts;
				cloadeditems = 0;
				SendMessage(hwndProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, ctotalitems));

				//SetWindowText(hwndStatusName, "Loading Objects...");

				int i;
				for(i = 0; i < csubobj; i++)
					{
					char szSuffix[32], szStmName[64];
					strcpy(szStmName, "GameItem");
					_itoa(i, szSuffix, 10);
					strcat(szStmName, szSuffix);

					MAKE_WIDEPTR_FROMANSI(wszStmName, szStmName);

					if(SUCCEEDED(hr = pstgData->OpenStream(wszStmName, NULL, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmItem)))
						{
						ULONG read;
						ItemTypeEnum type;
						IEditable *piedit;
						hr = pstmItem->Read(&type, sizeof(int), &read);
						CreateIEditableFromType(type, &piedit);

						piedit->AddRef();

						//AddSpriteProjItem();
						int id; // VBA id for this item
						hr = piedit->InitLoad(pstmItem, this, &id, version, hch, hkey);
						piedit->InitVBA(fFalse, id, NULL);
						pstmItem->Release();
						pstmItem = NULL;
						if(FAILED(hr)) break;

						m_vedit.AddElement(piedit);

						//hr = piedit->InitPostLoad();
						}
					cloadeditems++;
					SendMessage(hwndProgressBar, PBM_SETPOS, cloadeditems, 0);
					}

				for(i = 0; i < csounds; i++)
					{
					char szSuffix[32], szStmName[64];
					strcpy(szStmName, "Sound");
					_itoa(i, szSuffix, 10);
					strcat(szStmName, szSuffix);

					MAKE_WIDEPTR_FROMANSI(wszStmName, szStmName);

					if(SUCCEEDED(hr = pstgData->OpenStream(wszStmName, NULL, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmItem)))
						{
						LoadSoundFromStream(pstmItem);
						pstmItem->Release();
						pstmItem = NULL;
						}
					cloadeditems++;
					SendMessage(hwndProgressBar, PBM_SETPOS, cloadeditems, 0);
					}

				for(i = 0; i < ctextures; i++)
					{
					char szSuffix[32], szStmName[64];
					strcpy(szStmName, "Image");
					_itoa(i, szSuffix, 10);
					strcat(szStmName, szSuffix);

					MAKE_WIDEPTR_FROMANSI(wszStmName, szStmName);

					if(SUCCEEDED(hr = pstgData->OpenStream(wszStmName, NULL, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmItem)))
						{
						LoadImageFromStream(pstmItem, version);
						pstmItem->Release();
						pstmItem = NULL;
						}
					cloadeditems++;
					SendMessage(hwndProgressBar, PBM_SETPOS, cloadeditems, 0);
					}

				for(i = 0; i < cfonts; i++)
					{
					char szSuffix[32], szStmName[64];
					strcpy(szStmName, "Font");
					_itoa(i, szSuffix, 10);
					strcat(szStmName, szSuffix);

					MAKE_WIDEPTR_FROMANSI(wszStmName, szStmName);

					if(SUCCEEDED(hr = pstgData->OpenStream(wszStmName, NULL, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmItem)))
						{
						PinFont *ppf;
						ppf = new PinFont();
						ppf->LoadFromStream(pstmItem, version);
						m_vfont.AddElement(ppf);
						ppf->Register();
						pstmItem->Release();
						pstmItem = NULL;
						}
					cloadeditems++;
					SendMessage(hwndProgressBar, PBM_SETPOS, cloadeditems, 0);
					}

				for(i = 0; i < ccollection; i++)
					{
					char szSuffix[32], szStmName[64];
					strcpy(szStmName, "Collection");
					_itoa(i, szSuffix, 10);
					strcat(szStmName, szSuffix);

					MAKE_WIDEPTR_FROMANSI(wszStmName, szStmName);

					if(SUCCEEDED(hr = pstgData->OpenStream(wszStmName, NULL, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmItem)))
						{
						CComObject<Collection> *pcol;
						CComObject<Collection>::CreateInstance(&pcol);
						pcol->AddRef();
						pcol->LoadData(pstmItem, this, version, hch, hkey);
						m_vcollection.AddElement(pcol);
						m_pcv->AddItem((IScriptable *)pcol, fFalse);
						pstmItem->Release();
						pstmItem = NULL;
						}
					cloadeditems++;
					SendMessage(hwndProgressBar, PBM_SETPOS, cloadeditems, 0);
					}

				for (i=0;i<m_vedit.Size();i++)
					{
					IEditable *piedit;
					piedit = m_vedit.ElementAt(i);
					piedit->InitPostLoad();
					}
				}
			pstmGame->Release();

			// Authentication block

			BYTE hashvalOld[256];
			DWORD hashlenOld = 256;

			if (version > 40)
				{
				if(SUCCEEDED(hr = pstgData->OpenStream(L"MAC", NULL, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmVersion)))
					{
					ULONG read;
					hr = pstmVersion->Read(&hashvalOld, HASHLENGTH, &read);
					}
				else
					{
					// Error
					hr = E_ACCESSDENIED;
					}

				foo = CryptGetHashParam(hch, HP_HASHSIZE, hashval, &hashlen, 0);

				hashlen = 256;
				foo = CryptGetHashParam(hch, HP_HASHVAL, hashval, &hashlen, 0);

				foo = CryptDestroyHash(hch);

				foo = CryptDestroyHash(hchkey);

				foo = CryptDestroyKey(hkey);

				foo = CryptReleaseContext(hcp, 0);

				int i;
				for (i=0;i<HASHLENGTH;i++)
					{
					if (hashval[i] != hashvalOld[i])
						{
						hr = E_ACCESSDENIED;
						}
					}
				}
			//////// End Authentication block
			}
		pstgData->Release();
		}

	if (m_pbTempScreenshot) // For some reason, no image picked up the screenshot.  Not good; but we'll dump it to make sure it gets cleaned up
		{
		delete m_pbTempScreenshot;
		m_pbTempScreenshot = NULL;
		}

	DestroyWindow(hwndProgressBar);
	//DestroyWindow(hwndProgressDialog);

#ifdef VBA
	if(FAILED(hr)) return hr;

	//Now do the vba stuff
	if(SUCCEEDED(hr = ApcProject.Open(g_pvp->ApcHost, axProjectNormal)))
		{
		if(SUCCEEDED(hr = ApcProject.Load(pstgRoot)))
			{
			if(!SUCCEEDED(hr = ApcProject.FinishLoading()))
				{
				ShowError("Some parts of the table could not be loaded.");
				hr = S_OK; // Load the parts that were okay
				}
			m_pStg = pstgRoot;

			MAKE_WIDEPTR_FROMANSI(wszFileName, m_szFileName);
			BSTR bstrFileName = SysAllocString(wszFileName);
			ApcProject->APC_PUT(DisplayName)(bstrFileName);
			SysFreeString(bstrFileName);
			}
		}
#else
	pstgRoot->Release();
#endif

	g_pvp->SetActionCur("");

	return hr;
}

HRESULT PinTable::CreateIEditableFromType(int type, IEditable **piedit)
	{
	switch (type)
		{
		case eItemSurface:
			CComObject<Surface> *psurface;
			CComObject<Surface>::CreateInstance(&psurface);
			*piedit = psurface;
			break;

		case eItemFlipper:
			CComObject<Flipper> *pflipper;
			CComObject<Flipper>::CreateInstance(&pflipper);
			*piedit = pflipper;
			break;

		case eItemTimer:
			CComObject<Timer> *ptimer;
			CComObject<Timer>::CreateInstance(&ptimer);
			*piedit = ptimer;
			break;

		case eItemPlunger:
			CComObject<Plunger> *pplunger;
			CComObject<Plunger>::CreateInstance(&pplunger);
			*piedit = pplunger;
			break;

		case eItemTextbox:
			CComObject<Textbox> *ptextbox;
			CComObject<Textbox>::CreateInstance(&ptextbox);
			*piedit = ptextbox;
			break;

		case eItemComControl:
			CComObject<PinComControl> *pcomcontrol;
			CComObject<PinComControl>::CreateInstance(&pcomcontrol);
			*piedit = pcomcontrol;
			break;

		case eItemDispReel:
			CComObject<DispReel> *pdispreel;
			CComObject<DispReel>::CreateInstance(&pdispreel);
			*piedit = pdispreel;
			break;

		case eItemLightSeq:
			CComObject<LightSeq> *plightseq;
			CComObject<LightSeq>::CreateInstance(&plightseq);
			*piedit = plightseq;
			break;

		case eItemBumper:
			CComObject<Bumper> *pbumper;
			CComObject<Bumper>::CreateInstance(&pbumper);
			*piedit = pbumper;
			break;

		case eItemTrigger:
			CComObject<Trigger> *ptrigger;
			CComObject<Trigger>::CreateInstance(&ptrigger);
			*piedit = ptrigger;
			break;

		case eItemLight:
			CComObject<Light> *plight;
			CComObject<Light>::CreateInstance(&plight);
			*piedit = plight;
			break;

		case eItemKicker:
			CComObject<Kicker> *pkicker;
			CComObject<Kicker>::CreateInstance(&pkicker);
			*piedit = pkicker;
			break;

		case eItemDecal:
			CComObject<Decal> *pdecal;
			CComObject<Decal>::CreateInstance(&pdecal);
			*piedit = pdecal;
			break;

		case eItemGate:
			CComObject<Gate> *pgate;
			CComObject<Gate>::CreateInstance(&pgate);
			*piedit = pgate;
			break;

		case eItemSpinner:
			CComObject<Spinner> *pspinner;
			CComObject<Spinner>::CreateInstance(&pspinner);
			*piedit = pspinner;
			break;

		case eItemRamp:
			CComObject<Ramp> *pramp;
			CComObject<Ramp>::CreateInstance(&pramp);
			*piedit = pramp;
			break;

		default:
			_ASSERTE(fFalse);
			break;
		}

	return S_OK;
	}

void PinTable::SetLoadDefaults()
	{
	m_szImageBackdrop[0] = 0;
	m_szBallImage[0] = 0;
	m_szBallImageFront[0] = 0;
	m_szBallImageBack[0] = 0;

	m_szScreenShot[0] = 0;

	m_colorplayfield = RGB(128,128,128);
	m_colorbackdrop = RGB(0x62,0x6E,0x8E);

	m_angletilt = 4.5;
	}

HRESULT PinTable::LoadData(IStream* pstm, int& csubobj, int& csounds, int& ctextures, int& cfonts, int& ccollection, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
#ifndef OLDLOAD
	int rgi[6];
	int i;

	SetLoadDefaults();

	for (i=0;i<6;i++)
		{
		rgi[i] = 0;
		}

	BiffReader br(pstm, this, rgi, version, hcrypthash, hcryptkey);

	br.Load();

#ifdef VBA
	ApcProjectItem.Register(ApcProject, GetDispatch(), rgi[0]);
#endif

	csubobj = rgi[1];
	csounds = rgi[2];
	ctextures = rgi[3];
	cfonts = rgi[4];
	ccollection = rgi[5];

	return S_OK;
#else
	ULONG read = 0;
	HRESULT hr = S_OK;

	int count;

	if(FAILED(hr = pstm->Read(&count, sizeof(int), &read)))
		return hr;

	csubobj = count;

	if(FAILED(hr = pstm->Read(&m_gridsize, sizeof(m_gridsize), &read)))
		return hr;

	DWORD dwID;
	if(FAILED(hr = pstm->Read(&dwID, sizeof dwID, &read)))
		return hr;

	if(FAILED(hr = pstm->Read(&m_left, sizeof m_left, &read)))
		return hr;

	if(FAILED(hr = pstm->Read(&m_top, sizeof m_top, &read)))
		return hr;

	if(FAILED(hr = pstm->Read(&m_right, sizeof m_right, &read)))
		return hr;

	if(FAILED(hr = pstm->Read(&m_bottom, sizeof m_bottom, &read)))
		return hr;

	if(FAILED(hr = pstm->Read(&m_offsetx, sizeof m_offsetx, &read)))
		return hr;

	if(FAILED(hr = pstm->Read(&m_offsety, sizeof m_offsety, &read)))
		return hr;

	if(FAILED(hr = pstm->Read(&m_zoom, sizeof m_zoom, &read)))
		return hr;

	if(FAILED(hr = pstm->Read(&m_inclination, sizeof m_inclination, &read)))
		return hr;

	if(FAILED(hr = pstm->Read(&m_FOV, sizeof m_FOV, &read)))
		return hr;

	if(FAILED(hr = pstm->Read(&count, sizeof(int), &read)))
		return hr;

	if (read)
		{
		csounds = count;
		}

	if(FAILED(hr = pstm->Read(&count, sizeof(int), &read)))
		return hr;

	if (read)
		{
		ctextures = count;
		}

	if(FAILED(hr = pstm->Read(m_szImage, MAXTOKEN, &read)))
		return hr;

	if(FAILED(hr = pstm->Read(&m_fGrid, sizeof(m_fGrid), &read)))
		return hr;

	if(FAILED(hr = pstm->Read(&m_fBackdrop, sizeof(m_fBackdrop), &read)))
		return hr;

	ApcProjectItem.Register(ApcProject, GetDispatch(), dwID);

	return hr;
#endif
}

BOOL PinTable::LoadToken(int id, BiffReader *pbr)
	{
	if( id == FID(MPGC))
	{
		pbr->GetInt(&m_plungerNormalize);	
		HRESULT hr = GetRegInt("Player", "PlungerNormalize", &m_plungerNormalize);	
		}
	else if( id == FID(MPDF))
	{
		int tmp;
		pbr->GetBool(&tmp);	
		HRESULT hr = GetRegInt("Player", "PlungerFilter", &tmp);
		if(hr == S_OK) m_plungerFilter = tmp != 0;		
	}
	else if (id == FID(ACEL)) //////////////////
	{		
		pbr->GetBool(&m_tblAccelerometer);
		GetRegInt("Player", "PinballWizardEnabled", &m_tblAccelerometer);
		m_tblAccelerometer = m_tblAccelerometer != fFalse;
	}
	else if (id == FID(AORD))
	{
		pbr->GetBool(&m_tblAccelNormalMount);
		GetRegInt("Player", "PinballWizardNormalMount", &m_tblAccelNormalMount);
		m_tblAccelNormalMount = m_tblAccelNormalMount != fFalse;
	}
	else if (id == FID(AANG))
	{
		pbr->GetFloat(&m_tblAccelAngle);
		int tmp;
		HRESULT hr = GetRegInt("Player", "PinballWizardRotation", &tmp);
		if (hr == S_OK) m_tblAccelAngle = (float)tmp;		
	}
	else if (id == FID(AAMP))
	{
		pbr->GetFloat(&m_tblAccelAmp);
		int tmp;
		HRESULT hr = GetRegInt("Player", "PinballWizardAccelGain", &tmp);
		if (hr == S_OK) m_tblAccelAmp = (float)tmp/100.0f;		
	}
	else if (id == FID(AMAMP))
	{
		pbr->GetFloat(&m_tblAccelManualAmp);
		int tmp;
		HRESULT hr = GetRegInt("Player", "JoystickGain", &tmp);
		if (hr == S_OK) m_tblAccelManualAmp = (float)tmp/100.0f;		
	}	
	else if (id == FID(PIID))
		{
		pbr->GetInt(&((int *)pbr->m_pdata)[0]);
		}
	else if (id == FID(GRSZ))
		{
		pbr->GetFloat(&m_gridsize);
		}
	else if (id == FID(LEFT))
		{
		pbr->GetFloat(&m_left);
		}
	else if (id == FID(TOPX))
		{
		pbr->GetFloat(&m_top);
		}
	else if (id == FID(RGHT))
		{
		pbr->GetFloat(&m_right);
		}
	else if (id == FID(BOTM))
		{
		pbr->GetFloat(&m_bottom);
		}
	else if (id == FID(OFFX))
		{
		pbr->GetFloat(&m_offsetx);
		}
	else if (id == FID(OFFY))
		{
		pbr->GetFloat(&m_offsety);
		}
	else if (id == FID(ZOOM))
		{
		pbr->GetFloat(&m_zoom);
		}
	else if (id == FID(INCL))
		{
		pbr->GetFloat(&m_inclination);
		}
	else if (id == FID(FOVX))
		{
		pbr->GetFloat(&m_FOV);
		}
	else if (id == FID(SLOP))
		{
		pbr->GetFloat(&m_angletilt);
		}
	else if (id == FID(GLAS))
		{
		pbr->GetFloat(&m_glassheight);
		}
	else if (id == FID(IMAG))
		{
		pbr->GetString(m_szImage);
		}
	else if (id == FID(BLIM))
		{
		pbr->GetString(m_szBallImage);
		}
	else if (id == FID(BLIF))
		{
		pbr->GetString(m_szBallImageFront);
		}
	else if (id == FID(BLIB))
		{
		pbr->GetString(m_szBallImageBack);
		}
	else if (id == FID(SSHT))
		{
		pbr->GetString(m_szScreenShot);
		}
	else if (id == FID(FGRD))
		{
		pbr->GetBool(&m_fGrid);
		}
	else if (id == FID(FBCK))
		{
		pbr->GetBool(&m_fBackdrop);
		}
	else if (id == FID(SEDT))
		{
		pbr->GetInt(&((int *)pbr->m_pdata)[1]);
		}
	else if (id == FID(SSND))
		{
		pbr->GetInt(&((int *)pbr->m_pdata)[2]);
		}
	else if (id == FID(SIMG))
		{
		pbr->GetInt(&((int *)pbr->m_pdata)[3]);
		}
	else if (id == FID(SFNT))
		{
		pbr->GetInt(&((int *)pbr->m_pdata)[4]);
		}
	else if (id == FID(SCOL))
		{
		pbr->GetInt(&((int *)pbr->m_pdata)[5]);
		}
	else if (id == FID(NAME))
		{
		pbr->GetWideString((WCHAR *)m_wzName);
		}
	else if (id == FID(BIMG))
		{
		pbr->GetString(m_szImageBackdrop);
		}
	else if (id == FID(COLR))
		{
		pbr->GetInt(&m_colorplayfield);
		}
	else if (id == FID(BCLR))
		{
		pbr->GetInt(&m_colorbackdrop);
		}
	else if (id == FID(SECB))
		{
		pbr->GetStruct(&m_protectionData, sizeof(_protectionData));
		if (pbr->m_version < 700 && m_protectionData.flags != 0)
			{
			m_protectionData.flags |= DISABLE_DEBUGGER;
			}
		}
	else if (id == FID(CODE))
		{
		// if the script is protected then we pass in the proper cyptokey into the code loadstream
		if (CheckPermissions(DISABLE_SCRIPT_EDITING) == fTrue)
			{
			m_pcv->LoadFromStream(pbr->m_pistream, pbr->m_hcrypthash, pbr->m_hcryptkey);
			}
		else
			{
			m_pcv->LoadFromStream(pbr->m_pistream, pbr->m_hcrypthash, NULL);
			}
		}
	else if (id == FID(CCUS))
		{
		pbr->GetStruct(rgcolorcustom, sizeof(COLORREF)*16);
		}
	else if (id == FID(DSHD))
		{
		pbr->GetBool(&m_fRenderShadows);
		}
	else if (id == FID(PHYS))
		{
		pbr->GetInt(&m_PhysicsType);
		}
	else if (id == FID(CUST))
		{
		char szT[1024];  //maximum length of tagnames right now
		pbr->GetString(szT);
		char *szName;
		szName = new char[lstrlen(szT)+1];
		lstrcpy(szName,szT);
		m_vCustomInfoTag.AddElement(szName);
		}


	return fTrue;
	}

void PinTable::ReImportSound(HWND hwndListView, PinSound *pps, char *filename, BOOL fPlay)
	{
	PinSound *ppsNew;
	ppsNew = g_pvp->m_pds.LoadWaveFile(filename);

	if (ppsNew == NULL)
		{
		return;
		}

	PinSound psT = *pps;
	*pps = *ppsNew;
	*ppsNew = psT;

	lstrcpy(pps->m_szName, ppsNew->m_szName);
	lstrcpy(pps->m_szInternalName, ppsNew->m_szInternalName);

	// make sure sound data doesn't get deleted twice
	psT.m_pdata = NULL;
	psT.m_pDSBuffer = NULL;
	delete ppsNew;

	if (fPlay)
		{
		pps->m_pDSBuffer->Play( 0, 0, 0 );
		}
	}

void PinTable::ImportSound(HWND hwndListView, char *szfilename, BOOL fPlay)
	{
	PinSound *pps;
	pps = g_pvp->m_pds.LoadWaveFile(szfilename);

	if (pps == NULL)
		{
		return;
		}

	if (fPlay)
		{
		pps->m_pDSBuffer->Play( 0, 0, 0 );
		}

	m_vsound.AddElement(pps);

	int index = AddListSound(hwndListView, pps);

	ListView_SetItemState(hwndListView, index, LVIS_SELECTED, LVIS_SELECTED);
	}

void PinTable::ListSounds(HWND hwndListView)
	{
	int i;
	for (i=0;i<m_vsound.Size();i++)
		{
		AddListSound(hwndListView, m_vsound.ElementAt(i));
		}
	}

int PinTable::AddListSound(HWND hwndListView, PinSound *pps)
	{
		LVITEM lvitem;

		lvitem.mask = LVIF_DI_SETITEM | LVIF_TEXT | LVIF_PARAM;
		lvitem.iItem = 0;
		lvitem.iSubItem = 0;
		lvitem.pszText = pps->m_szName;
		lvitem.lParam = (long)pps;

		int index = ListView_InsertItem(hwndListView, &lvitem);

		ListView_SetItemText(hwndListView, index, 1, pps->m_szPath);

		return index;
	}

/*void PinTable::RemoveListSound(HWND hwndListView, PinSound *pps)
	{
	}*/

void PinTable::RemoveSound(PinSound *pps)
	{
	//pps->m_pDSBuffer->Release();
	m_vsound.RemoveElement(pps);
	delete pps;
	}

void PinTable::ImportFont(HWND hwndListView, char *filename)
	{
	PinFont *ppb;

	ppb = new PinFont();

	ppb->ReadFromFile(filename);

	if (ppb->m_pdata != NULL)
		{
		//DumpNameTable(filename, ppb->m_szFontName);

		m_vfont.AddElement(ppb);

		int index = AddListBinary(hwndListView, ppb);

		ListView_SetItemState(hwndListView, index, LVIS_SELECTED, LVIS_SELECTED);

		ppb->Register();
		}
	}

void PinTable::RemoveFont(PinFont *ppf)
	{
	m_vfont.RemoveElement(ppf);
	ppf->UnRegister();
	delete ppf;
	}

void PinTable::ListFonts(HWND hwndListView)
	{
	int i;
	for (i=0;i<m_vfont.Size();i++)
		{
		AddListBinary(hwndListView, m_vfont.ElementAt(i));
		}
	}

int PinTable::AddListBinary(HWND hwndListView, PinBinary *ppb)
	{
		LVITEM lvitem;

		lvitem.mask = LVIF_DI_SETITEM | LVIF_TEXT | LVIF_PARAM;
		lvitem.iItem = 0;
		lvitem.iSubItem = 0;
		lvitem.pszText = ppb->m_szName;
		lvitem.lParam = (long)ppb;

		int index = ListView_InsertItem(hwndListView, &lvitem);

		ListView_SetItemText(hwndListView, index, 1, ppb->m_szPath);

		return index;
	}

void PinTable::NewCollection(HWND hwndListView, BOOL fFromSelection)
	{
	WCHAR wzT[128];

	CComObject<Collection> *pcol;
	CComObject<Collection>::CreateInstance(&pcol);
	pcol->AddRef();

	GetUniqueName(eItemCollection, wzT);

	WideStrCopy(wzT, pcol->m_wzName);

	if (fFromSelection && (m_vmultisel.ElementAt(0) != this))
		{
		int i;
		for (i=0;i<m_vmultisel.Size();i++)
			{
			ISelect *pisel = m_vmultisel.ElementAt(i);
			IEditable *piedit = pisel->GetIEditable();
			if (piedit->GetISelect() == pisel) // Do this check so we don't put walls in a collection when we only have the control point selected
				{
				if (piedit && piedit->GetScriptable()) // check for scriptable because can't add decals to a collection - they have no name
					{
					piedit->m_vCollection.AddElement(pcol);
					piedit->m_viCollection.AddElement((void *)pcol->m_visel.Size());
					pcol->m_visel.AddElement(m_vmultisel.ElementAt(i));
					}
				}
			}
		}

	int index = AddListCollection(hwndListView, pcol);

	ListView_SetItemState(hwndListView, index, LVIS_SELECTED, LVIS_SELECTED);

	m_vcollection.AddElement(pcol);
	m_pcv->AddItem((IScriptable *)pcol, fFalse);
	}

int PinTable::AddListCollection(HWND hwndListView, CComObject<Collection> *pcol)
	{
	char szT[MAX_PATH];

	WideCharToMultiByte(CP_ACP, 0, pcol->m_wzName, -1, szT, MAX_PATH, NULL, NULL);

	LVITEM lvitem;

	lvitem.mask = LVIF_DI_SETITEM | LVIF_TEXT | LVIF_PARAM;
	lvitem.iItem = 0;
	lvitem.iSubItem = 0;
	lvitem.pszText = szT;
	lvitem.lParam = (long)pcol;

	int index = ListView_InsertItem(hwndListView, &lvitem);

	return index;
	}

void PinTable::ListCollections(HWND hwndListView)
	{
	//ListView_DeleteAllItems(hwndListView);

	int i;
	for (i=0;i<m_vcollection.Size();i++)
		{
		CComObject<Collection> *pcol = m_vcollection.ElementAt(i);

		AddListCollection(hwndListView, pcol);
		}
	}

void PinTable::RemoveCollection(CComObject<Collection> *pcol)
	{
	m_pcv->RemoveItem((IScriptable *)pcol);
	m_vcollection.RemoveElement(pcol);
	pcol->Release();
	}

void PinTable::SetCollectionName(Collection *pcol, char *szName, HWND hwndList, int index)
	{
	WCHAR wzT[1024];
	MultiByteToWideChar(CP_ACP, 0, szName, -1, wzT, 1024);
	if (m_pcv->ReplaceName((IScriptable *)pcol, wzT) == S_OK)
		{
		if (hwndList)
			{
			ListView_SetItemText(hwndList, index, 0, szName);
			}
		WideStrCopy(wzT, pcol->m_wzName);
		}
	}

/*STDMETHODIMP PinTable::get_Application(IVisualPinball **lppaReturn)
	{
	return m_pvp->QueryInterface(IID_IVisualPinball, (void **)lppaReturn);
	}

STDMETHODIMP PinTable::get_Parent(IVisualPinball **lppaReturn)
	{
	return m_pvp->QueryInterface(IID_IVisualPinball, (void **)lppaReturn);
	}*/

STDMETHODIMP PinTable::get_GridSize(float *pgs)
	{
	*pgs = m_gridsize;
	return S_OK;
	}

STDMETHODIMP PinTable::put_GridSize(float gs)
	{
	/*if (gs <= 0)
		{
		return E_FAIL;
		}*/

	STARTUNDO

	if (gs < 1)
		{
		gs = 1;
		}

	m_gridsize = gs;

	SetDirtyDraw();

	STOPUNDO

	return S_OK;
	}

/*void PinTable::FireVoidEvent(int dispid)
	{
	DISPPARAMS dispparams  = {
		NULL,
		NULL,
		0,
		0
		};

	FireDispID(dispid, &dispparams);
	}*/

void PinTable::SetZoom(float zoom)
	{
	m_zoom = zoom;
	SetMyScrollInfo();
	}

void PinTable::GetViewRect(FRect *pfrect)
	{
	if (!g_pvp->m_fBackglassView)
		{
		pfrect->left = m_left;
		pfrect->top = m_top;
		pfrect->right = m_right;
		pfrect->bottom = m_bottom;
		}
	else
		{
		pfrect->left = 0;
		pfrect->top = 0;
		pfrect->right = 1000;
		pfrect->bottom = 750;
		}
	}

void PinTable::SetMyScrollInfo()
	{
	HitSur *phs;

	FRect frect;
	GetViewRect(&frect);

	RECT rc;
	GetClientRect(m_hwnd, &rc);
	Vertex rgv[2];

	phs = new HitSur(NULL, m_zoom, m_offsetx, m_offsety, rc.right - rc.left, rc.bottom - rc.top, 0, 0, NULL);

	phs->ScreenToSurface(rc.left, rc.top, &rgv[0].x, &rgv[0].y);
	phs->ScreenToSurface(rc.right, rc.bottom, &rgv[1].x, &rgv[1].y);

	delete phs;

	SCROLLINFO si;

	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
	si.nMin = (int)min(frect.left, rgv[0].x);
	si.nMax = (int)max(frect.right, rgv[1].x);
	si.nPage = (int)(rgv[1].x - rgv[0].x);
	si.nPos = (int)(rgv[0].x);

	SetScrollInfo(m_hwnd, SB_HORZ, &si, fTrue);

	si.nMin = (int)min(frect.top, rgv[0].y);
	si.nMax = (int)max(frect.bottom, rgv[1].y);
	si.nPage = (int)(rgv[1].y - rgv[0].y);
	si.nPos = (int)(rgv[0].y);

	SetScrollInfo(m_hwnd, SB_VERT, &si, fTrue);
	}

void PinTable::FireKeyEvent(int dispid, int keycode)
	{
	CComVariant rgvar[1] = {  CComVariant(keycode)};

	DISPPARAMS dispparams  = {
		rgvar,
		NULL,
		1,
		0
		};

#ifdef LOG
		if (g_pplayer)
			{
			if (dispid == DISPID_GameEvents_KeyDown)
				{
				fprintf(g_pplayer->m_flog, "Key Down %d\n", keycode);
				}
			else
				{
				fprintf(g_pplayer->m_flog, "Key Up %d\n", keycode);
				}
			}
#endif

	FireDispID(dispid, &dispparams);
	}

void PinTable::DoLButtonDown(int x,int y)
	{
	int ksshift, ksctrl;
	ksshift = GetKeyState(VK_SHIFT);
	ksctrl = GetKeyState(VK_CONTROL);

	// set the focus of the window so all keyboard and mouse inputs are processed.
	// (this fixes the problem of selecting a element on the properties dialog, clicking on a table
	// object and not being able to use the cursor keys/wheely mouse
	SetFocus(g_pvp->m_hwndWork);

	if ((g_pvp->m_ToolCur == IDC_MAGNIFY) || (ksctrl & 0x80000000))
		{
		if (m_zoom < MAX_ZOOM)
			{
			Vertex v;
			TransformPoint(x,y,&v);
			m_offsetx = v.x;
			m_offsety = v.y;
			SetZoom(m_zoom * 2);
			SetDirtyDraw();
			}
		}

	// if disabling table view then don't allow the table to be selected (thus bringing up table properties)
	else if (CheckPermissions(DISABLE_TABLEVIEW) == fFalse)
	// Normal click
		{
		ISelect *pisel = HitTest(x,y);
		// Null selection not allowed
		/*if (!(ksshift & 0x80000000))
			{
			SetSel(pisel);
			}
		else
			{
			AddMultiSel(pisel);
			}*/

		BOOL fAdd = (ksshift & 0x80000000) != 0;

		if (pisel == (ISelect *)this && fAdd == fTrue)
			{
			// Can not include the table in multi-select
			// and table will not be unselected, because the
			// user might be drawing a box around other objects
			// to add them to the selection group
			OnLButtonDown(x,y); // Start the band select
			return;
			}

		AddMultiSel(pisel, fAdd, fTrue);

		//m_pselcur->OnLButtonDown(x,y);
		int i;
		for (i=0;i<m_vmultisel.Size();i++)
			{
			m_vmultisel.ElementAt(i)->OnLButtonDown(x,y);
			}
		}
	}

void PinTable::DoLButtonUp(int x,int y)
	{
	int i;
	//m_pselcur->OnLButtonUp(x,y);

	if (!m_fDragging) // Not doing band select
		{
		for (i=0;i<m_vmultisel.Size();i++)
			{
			m_vmultisel.ElementAt(i)->OnLButtonUp(x,y);
			}
		}
	else
		{
		OnLButtonUp(x,y);
		}
	}

void PinTable::DoRButtonDown(int x,int y)
	{
	int ks;
	ks = GetKeyState(VK_CONTROL);

	if ((g_pvp->m_ToolCur == IDC_MAGNIFY) || (ks & 0x80000000))
		{
		if (m_zoom > MIN_ZOOM)
			{
			Vertex v;
			TransformPoint(x,y,&v);
			m_offsetx = v.x;
			m_offsety = v.y;
			SetZoom(m_zoom * 0.5f);
			SetDirtyDraw();
			}
		}
	else
		{
		//SetSel(HitTest(x,y));
		AddMultiSel(HitTest(x,y), fFalse, fTrue);
		//m_pselcur->OnRButtonDown(x,y,m_hwnd);
		/*if (m_vmultisel.Size() > 1)
			{
			DoContextMenu(x, y, IDR_MULTIMENU, this);
			}
		else
			{
			if (m_vmultisel.ElementAt(0) != this)
				{
				// No right click menu for main table object
				DoContextMenu(x, y, m_vmultisel.ElementAt(0)->m_menuid, m_vmultisel.ElementAt(0));
				//m_vmultisel.ElementAt(0)->OnRButtonDown(x,y,m_hwnd);
				}
			else
				{
				DoContextMenu(x, y, IDR_TABLEMENU, m_vmultisel.ElementAt(0));
				}
			}*/
		}
	}

void PinTable::DoContextMenu(int x, int y, int menuid, ISelect *psel)
	{
	HMENU hmenumain;
	HMENU hmenu;
	POINT pt;
	int icmd;

	//(m_vmultisel.ElementAt(0) != this)*/
	/*if (menuid == -1 && GetIEditable() == NULL)
		{
		return;
		}*/

	pt.x = x;
	pt.y = y;
	ClientToScreen(m_hwnd, &pt);

	if (menuid != -1)
		{
		hmenumain = LoadMenu(g_hinstres, MAKEINTRESOURCE(menuid));

		hmenu = GetSubMenu(hmenumain, 0);
		}
	else
		{
		hmenu = CreatePopupMenu();
		}

	psel->EditMenu(hmenu);

	if (menuid != IDR_POINTMENU && menuid != IDR_TABLEMENU) //psel->GetIEditable() != NULL)
		{
		if (GetMenuItemCount(hmenu) > 0)
			{
			AppendMenu(hmenu, MF_SEPARATOR, -1, "");
			}
		// TEXT
		LocalString ls1(IDS_DRAWINFRONT);
		LocalString ls2(IDS_DRAWINBACK);
		AppendMenu(hmenu, MF_STRING, ID_DRAWINFRONT, ls1.m_szbuffer);
		AppendMenu(hmenu, MF_STRING, ID_DRAWINBACK, ls2.m_szbuffer);

		LocalString ls3(IDS_LOCK);
		AppendMenu(hmenu, MF_STRING, ID_LOCK, ls3.m_szbuffer);

		BOOL fLocked = psel->m_fLocked;
		// HACK
		if (psel == this) // multi-select case
			{
			fLocked = FMutilSelLocked();
			}

		CheckMenuItem(hmenu, ID_LOCK, MF_BYCOMMAND | (fLocked ? MF_CHECKED : MF_UNCHECKED));
		}

	icmd = TrackPopupMenuEx(hmenu, TPM_RETURNCMD,
			pt.x, pt.y, m_hwnd, NULL);

	if (icmd != 0)
		{
		psel->DoCommand(icmd, x, y);
		}

	DestroyMenu(hmenu);

	if (menuid != -1)
		{
		DestroyMenu(hmenumain);
		}

	return;
	}

BOOL PinTable::FMutilSelLocked()
	{
	BOOL fLocked = fFalse;

	int i;
	for (i=0;i<m_vmultisel.Size();i++)
		{
		if (m_vmultisel.ElementAt(i)->m_fLocked)
			{
			fLocked = fTrue;
			break;
			}
		}

	return fLocked;
	}

/*void PinTable::DoCommandToObject(int x, int y, int command, ISelect *psel)
	{
	IEditable *piedit = psel->GetIEditable();

	switch (command)
		{
		case ID_DRAWINFRONT:
			GetPTable()->m_vedit.RemoveElement(piedit);
			GetPTable()->m_vedit.AddElement(piedit);
			GetPTable()->SetDirtyDraw();
			break;
		case ID_DRAWINBACK:
			GetPTable()->m_vedit.RemoveElement(piedit);
			GetPTable()->m_vedit.InsertElementAt(piedit, 0);
			GetPTable()->SetDirtyDraw();
			break;
		default:
			psel->DoCommand(command, x, y);
			break;
		}
	}*/

void PinTable::DoCommand(int icmd, int x, int y)
	{
	switch (icmd)
		{
		case ID_DRAWINFRONT:
		case ID_DRAWINBACK:
			{
			int i;
			for (i=0;i<m_vmultisel.Size();i++)
				{
				ISelect *psel;
				psel = m_vmultisel.ElementAt(i);
				_ASSERTE(psel != this); // Would make an infinite loop
				psel->DoCommand(icmd, x, y);
				}
			}
			break;

		case ID_LOCK:
			{
			BeginUndo();
			BOOL fLock = FMutilSelLocked() ? fFalse : fTrue;
			int i;
			for (i=0;i<m_vmultisel.Size();i++)
				{
				ISelect *psel;
				psel = m_vmultisel.ElementAt(i);
				psel->GetIEditable()->MarkForUndo();
				psel->m_fLocked = fLock;
				}
			EndUndo();
			SetDirtyDraw();
			}
			break;

		case ID_WALLMENU_FLIP:
			{
			Vertex vCenter;

			GetCenter(&vCenter);

			FlipY(&vCenter);
			}
			break;

		case ID_WALLMENU_MIRROR:
			{
			Vertex vCenter;

			GetCenter(&vCenter);

			FlipX(&vCenter);
			}
			break;

		case IDC_PASTEAT:
			//g_pvp->ParseCommand(icmd, g_pvp->m_hwnd, 0);
			Paste(fTrue, x, y);
			break;


		/*case ID_WALLMENU_MIRROR:
			{
			int i;
			//float totalx, totaly;
			Vertex vCenter;
			/*totalx = 0;
			totaly = 0;

			for (i=0;i<m_vmultisel.Size();i++)
				{
				ISelect *psel;
				psel = m_vmultisel.ElementAt(i);
				psel->GetCenter(&vCenter);
				totalx += vCenter.x;
				totaly += vCenter.y;
				}

			totalx /= m_vmultisel.Size();
			totaly /= m_vmultisel.Size();*/

			/*float minx, maxx, miny, maxy;

			minx = FLT_MAX;
			maxx = -FLT_MAX;
			miny = FLT_MAX;
			maxy = -FLT_MAX;

			for (i=0;i<m_vmultisel.Size();i++)
				{
				ISelect *psel;
				psel = m_vmultisel.ElementAt(i);
				psel->GetCenter(&vCenter);

				minx = min(minx, vCenter.x);
				maxx = max(maxx, vCenter.x);
				miny = min(miny, vCenter.y);
				maxy = max(maxy, vCenter.y);
				//tx += m_vdpoint.ElementAt(i)->m_v.x;
				//ty += m_vdpoint.ElementAt(i)->m_v.y;
				}

			vCenter.x = (maxx+minx)/2;
			vCenter.y = (maxy+miny)/2;

			if (icmd == ID_WALLMENU_FLIP)
				{
				for (i=0;i<m_vmultisel.Size();i++)
					{
					m_vmultisel.ElementAt(i)->FlipY(&vCenter);
					}
				}
			else
				{
				for (i=0;i<m_vmultisel.Size();i++)
					{
					m_vmultisel.ElementAt(i)->FlipX(&vCenter);
					}
				}
			}
			break;*/

		case ID_WALLMENU_ROTATE:
			{
#ifdef VBA
			g_pvp->ApcHost->BeginModalDialog();
#endif
			DialogBoxParam(g_hinstres, MAKEINTRESOURCE(IDD_ROTATE),
				g_pvp->m_hwnd, RotateProc, (long)(ISelect *)this);
#ifdef VBA
			g_pvp->ApcHost->EndModalDialog();
#endif
			}
			break;

		case ID_WALLMENU_SCALE:
			{
#ifdef VBA
			g_pvp->ApcHost->BeginModalDialog();
#endif
			DialogBoxParam(g_hinstres, MAKEINTRESOURCE(IDD_SCALE),
				g_pvp->m_hwnd, ScaleProc, (long)(ISelect *)this);
#ifdef VBA
			g_pvp->ApcHost->EndModalDialog();
#endif
			}
			break;

		case ID_WALLMENU_TRANSLATE:
			{
#ifdef VBA
			g_pvp->ApcHost->BeginModalDialog();
#endif
			DialogBoxParam(g_hinstres, MAKEINTRESOURCE(IDD_TRANSLATE),
				g_pvp->m_hwnd, TranslateProc, (long)(ISelect *)this);
#ifdef VBA
			g_pvp->ApcHost->EndModalDialog();
#endif
			}
			break;
		}
	}

void PinTable::FlipY(Vertex *pvCenter)
	{
	int i;

	BeginUndo();

	for (i=0;i<m_vmultisel.Size();i++)
		{
		m_vmultisel.ElementAt(i)->FlipY(pvCenter);
		}

	EndUndo();
	}

void PinTable::FlipX(Vertex *pvCenter)
	{
	int i;

	BeginUndo();

	for (i=0;i<m_vmultisel.Size();i++)
		{
		m_vmultisel.ElementAt(i)->FlipX(pvCenter);
		}

	EndUndo();
	}

void PinTable::Rotate(float ang, Vertex *pvCenter)
	{
	int i;

	BeginUndo();

	for (i=0;i<m_vmultisel.Size();i++)
		{
		m_vmultisel.ElementAt(i)->Rotate(ang, pvCenter);
		}

	EndUndo();
	}

void PinTable::Scale(float scalex, float scaley, Vertex *pvCenter)
	{
	int i;

	BeginUndo();

	for (i=0;i<m_vmultisel.Size();i++)
		{
		m_vmultisel.ElementAt(i)->Scale(scalex, scaley, pvCenter);
		}

	EndUndo();
	}

void PinTable::Translate(Vertex *pvOffset)
	{
	int i;

	BeginUndo();

	for (i=0;i<m_vmultisel.Size();i++)
		{
		m_vmultisel.ElementAt(i)->Translate(pvOffset);
		}

	EndUndo();
	}

void PinTable::GetCenter(Vertex *pv)
	{
	int i;
	Vertex vCenter;

	float minx, maxx, miny, maxy;

	minx = FLT_MAX;
	maxx = -FLT_MAX;
	miny = FLT_MAX;
	maxy = -FLT_MAX;

	for (i=0;i<m_vmultisel.Size();i++)
		{
		ISelect *psel;
		psel = m_vmultisel.ElementAt(i);
		psel->GetCenter(&vCenter);

		minx = min(minx, vCenter.x);
		maxx = max(maxx, vCenter.x);
		miny = min(miny, vCenter.y);
		maxy = max(maxy, vCenter.y);
		//tx += m_vdpoint.ElementAt(i)->m_v.x;
		//ty += m_vdpoint.ElementAt(i)->m_v.y;
		}

	pv->x = (maxx+minx)/2;
	pv->y = (maxy+miny)/2;
	}

void PinTable::PutCenter(Vertex *pv)
	{
	}

void PinTable::DoRButtonUp(int x,int y)
	{
	m_vmultisel.ElementAt(0)->OnRButtonUp(x,y);

	int ks;
	ks = GetKeyState(VK_CONTROL);

	// Only bring up context menu if we weren't in magnify mode
	if (!((g_pvp->m_ToolCur == IDC_MAGNIFY) || (ks & 0x80000000)))
		{
		//SetSel(HitTest(x,y));
		//AddMultiSel(HitTest(x,y), fFalse);
		//m_pselcur->OnRButtonDown(x,y,m_hwnd);
		if (m_vmultisel.Size() > 1)
			{
			DoContextMenu(x, y, IDR_MULTIMENU, this);
			}
		else
			{
			if (m_vmultisel.ElementAt(0) != this)
				{
				// No right click menu for main table object
				DoContextMenu(x, y, m_vmultisel.ElementAt(0)->m_menuid, m_vmultisel.ElementAt(0));
				//m_vmultisel.ElementAt(0)->OnRButtonDown(x,y,m_hwnd);
				}
			else
				{
				DoContextMenu(x, y, IDR_TABLEMENU, m_vmultisel.ElementAt(0));
				}
			}
		}
	}

void PinTable::DoMouseMove(int x,int y)
	{
	Vertex v;

	TransformPoint(x,y,&v);

	g_pvp->SetPosCur(v.x, v.y);

	if (!m_fDragging) // Not doing band select
		{
		int i;
		for (i=0;i<m_vmultisel.Size();i++)
			{
			m_vmultisel.ElementAt(i)->OnMouseMove(x,y);
			}
		}
	else
		{
		OnMouseMove(x,y);
		}
	}

void PinTable::DoLDoubleClick(int x, int y)
	{
#ifdef VBA
	IApcProjectItem *papi;
	papi = m_pselcur->GetIApcProjectItem();
	if (papi)
		{
		papi->ViewEventHandler(NULL);
		}
	else // No code, but if its a control then it will have properties
		{
		IApcControl *pac;
		pac = m_pselcur->GetIApcControl();
		if (pac)
			{
			IApcPropertiesWindow *papw;
			g_pvp->ApcHost->get_PropertiesWindow(&papw);
			papw->put_Visible(VARIANT_TRUE);
			}
		}
#else
	//g_pvp->m_sb.SetVisible(fTrue);
	//SendMessage(g_pvp->m_hwnd, WM_SIZE, 0, 0);
#endif
	}

void PinTable::ExportBlueprint()
	{
	BOOL fSaveAs = fTrue;
	//char *c;
	int i,l;
	//HRESULT hr;

	//IStorage *pstgRoot;
	HANDLE hfile;

	if(fSaveAs)
		{
		//need to get a file name
		OPENFILENAME ofn;
		memset(&ofn, 0, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hInstance = g_hinst;
		ofn.hwndOwner = g_pvp->m_hwnd;
		// TEXT
		ofn.lpstrFilter = "Bitmap (*.bmp)\0*.bmp\0";
		ofn.lpstrFile = m_szBlueprintFileName;
		ofn.nMaxFile = _MAX_PATH;
		ofn.lpstrDefExt = "bmp";
		ofn.Flags = OFN_OVERWRITEPROMPT;

#ifdef VBA
		g_pvp->ApcHost->BeginModalDialog();
#endif
		int ret = GetSaveFileName(&ofn);
#ifdef VBA
		g_pvp->ApcHost->EndModalDialog();
#endif

		// user cancelled
		if(ret == 0)
			return;// S_FALSE;
		}

	hfile = CreateFile(m_szBlueprintFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL,
						CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);


	BITMAPFILEHEADER bmfh;
	DWORD foo;

	BITMAPINFO bmi;

	memset(&bmi, 0, sizeof(bmi));

	float tableheight, tablewidth;

	if (g_pvp->m_fBackglassView)
		{
		tablewidth = 1000;
		tableheight = 750;
		}
	else
		{
		tablewidth = m_right - m_left;
		tableheight = m_bottom - m_top;
		}

	int bmwidth, bmheight;

	if (tableheight > tablewidth)
		{
		bmheight = 1024;
		bmwidth = (int)((tablewidth/tableheight) * bmheight + 0.5);
		}
	else
		{
		bmwidth = 1024;
		bmheight = (int)((tableheight/tablewidth) * bmwidth + 0.5);
		}

	int totallinebytes = bmwidth * 3;
	totallinebytes = (((totallinebytes-1)/4)+1)*4; // make multiple of four
	int bmlinebuffer = totallinebytes - (bmwidth * 3);

	memset(&bmfh, 0, sizeof(bmfh));

	bmfh.bfType = 'M'<<8 | 'B';
	bmfh.bfSize = sizeof(bmfh) + sizeof(BITMAPINFOHEADER) + totallinebytes*bmheight;
	bmfh.bfOffBits = sizeof(bmfh) + sizeof(BITMAPINFOHEADER);	

	WriteFile(hfile, &bmfh, sizeof(bmfh), &foo, NULL);

	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = bmwidth;
	bmi.bmiHeader.biHeight = bmheight;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 24;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = totallinebytes*bmheight;

	WriteFile(hfile, &bmi, sizeof(BITMAPINFOHEADER), &foo, NULL);

	HBITMAP hdib;
	HDC hdcScreen;
	HDC hdc2;
	char *pbits;

	hdcScreen = GetDC(NULL);
	hdc2 = CreateCompatibleDC(hdcScreen);

	hdib = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, (void **)&pbits, NULL, 0);

	HBITMAP hbmOld = (HBITMAP)SelectObject(hdc2, hdib);

	PaintSur *psur;
	psur = new PaintSur(hdc2, bmwidth/tablewidth, tablewidth/2, tableheight/2, bmwidth, bmheight, NULL);

	SelectObject(hdc2, GetStockObject(WHITE_BRUSH));
	PatBlt(hdc2, 0, 0, bmwidth, bmheight, PATCOPY);

	if (g_pvp->m_fBackglassView)
		{
		Render3DProjection(psur);
		}

	for (i=0;i<m_vedit.Size();i++)
		{
		if (m_vedit.ElementAt(i)->m_fBackglass == g_pvp->m_fBackglassView)
			{
			m_vedit.ElementAt(i)->RenderBlueprint(psur);
			}
		}

	//Render(psur);

	delete psur;

	//BitBlt(hdcScreen, 0, 0, 512, 1024, hdc2, 0, 0, SRCCOPY);

	for (i=0;i<bmheight;i++)
		{
		//for (l=0;l<bmwidth;l++)
			//{
			//c = (pbits + ((l + (i*bmwidth)) * 3));
			//WriteFile(hfile, c, 3, &foo, NULL);
			//}
		WriteFile(hfile, (pbits + ((i*bmwidth) * 3)), bmwidth*3, &foo, NULL);
		}

	// For some reason to make our bitmap compatible with all programs,
	// We need to write out dummy bytes as if our totalwidthbytes had been
	// a multiple of 4.
	for (i=0;i<bmheight;i++)
		{
		for (l=0;l<bmlinebuffer;l++)
			{
			WriteFile(hfile, pbits, 1, &foo, NULL);
			}
		}

	DeleteDC(hdc2);
	ReleaseDC(NULL, hdcScreen);

	DeleteObject(hdib);

	CloseHandle(hfile);

	//pstgRoot->Release();
	}

/*#ifdef VBA
HRESULT PinTable::ApcProject_ModuleDirtyChange(IApcProjectItem* pProjectItem, VARIANT_BOOL fDirty)
	{
	if (fDirty)
		{
		m_undo.m_fDirty = fTrue;
		m_undo.m_fDirtyPermanent = fTrue; // Since we are complex enough to determine when VBA might undo itself out of a dirty state
		SetDirty();
		}

	return S_OK;
	}
#endif*/

void PinTable::SelectItem(IScriptable *piscript)
	{
	ISelect *pisel = piscript->GetISelect();
	if (pisel)
		{
		AddMultiSel(pisel, fFalse, fTrue);
		}
	}

void PinTable::DoCodeViewCommand(int command)
	{
	switch (command)
		{
		case ID_SAVE:
			// added by chris as part of table protection
			if (CheckPermissions(DISABLE_TABLE_SAVE) == fFalse)
				{
				TableSave();
				}
			break;

		case ID_TABLE_PLAY:
			Play();
			break;
		}
	}

void PinTable::SetDirtyScript(SaveDirtyState sds)
	{
	m_sdsDirtyScript = sds;
	CheckDirty();
	}

void PinTable::SetDirty(SaveDirtyState sds)
	{
	m_sdsDirtyProp = sds;
	CheckDirty();
	}

void PinTable::SetNonUndoableDirty(SaveDirtyState sds)
	{
	m_sdsNonUndoableDirty = sds;
	CheckDirty();
	}

void PinTable::CheckDirty()
	{
	/*BOOL fIsDirty = FDirty();

	if (fIsDirty)
		{
		m_fDirtyAutoSave = fTrue; // Autosave might be marked as clean even when the table is not, because we might have autosaved and then been idle until the next autosave time.
		}*/

	SaveDirtyState sdsNewDirtyState = max(max(m_sdsDirtyProp, m_sdsDirtyScript), m_sdsNonUndoableDirty);

	// Odd design?  If the user goes back to clean on one attribute but is autosaved on the others, we still need to autosave
	/*if ((m_sdsDirtyProp == eSaveAutosaved || m_sdsDirtyScript == eSaveAutosaved || m_sdsNonUndoableDirty == eSaveAutosaved) &&
		(m_sdsDirtyProp == eSaveClean || m_sdsDirtyScript == eSaveClean || m_sdsNonUndoableDirty == eSaveClean))
		{
		
		sdsNewDirtyState = eSaveDirty;
		}*/

	if (sdsNewDirtyState != m_sdsCurrentDirtyState)
		{
		if (sdsNewDirtyState > eSaveClean)
			{
			char szWindowName[_MAX_PATH];
			lstrcpy(szWindowName, m_szTitle);
			lstrcat(szWindowName, "*");

			SetCaption(szWindowName);
			}
		else
			{
			SetCaption(m_szTitle);
			}
		}

	m_sdsCurrentDirtyState = sdsNewDirtyState;
	}

BOOL PinTable::FDirty()
	{
	//return m_fDirtyProp | m_fDirtyScript | m_fNonUndoableDirty;
	return (m_sdsCurrentDirtyState > eSaveClean);
	}

void PinTable::BeginUndo()
	{
	m_undo.BeginUndo();
	}

void PinTable::EndUndo()
	{
	m_undo.EndUndo();
	}

void PinTable::Undo()
	{
	m_undo.Undo();

	SetDirtyDraw();
	g_pvp->m_sb.RefreshProperties();
	}

void PinTable::Uncreate(IEditable *pie)
	{
	//int index = m_vmultisel.IndexOf(psel);
	/*if (m_pselcur == pie->GetISelect())
		{
		SetSel(this);
		}*/

	if (pie->GetISelect()->m_selectstate != eNotSelected)
		{
		AddMultiSel(pie->GetISelect(), fTrue, fTrue); // Remove the item from the multi-select list
		}

	pie->GetISelect()->Uncreate();
	pie->Release();
	}

void PinTable::Undelete(IEditable *pie)
	{
	m_vedit.AddElement(pie);
	pie->Undelete();
	SetDirtyDraw();
	}

void PinTable::BackupForPlay()
	{
#ifndef PERFTEST
	int i;

	m_undo.BeginUndo();

	m_undo.MarkForUndo((IEditable *)this);
	for (i=0;i<m_vedit.Size();i++)
		{
		m_undo.MarkForUndo(m_vedit.ElementAt(i));
		}

	m_undo.EndUndo();
#endif //PERFTEST
	}

void PinTable::RestoreBackup()
	{
	m_undo.Undo();
	}

void PinTable::Copy()
	{
	//IStorage* pstg;
	//IStream* pstm;
	//HRESULT hr;
	int i;
	Vector<IStream> vstm;
	ULONG writ = 0;

	if (m_vmultisel.ElementAt(0) == (ISelect *)this) // Can't copy table
		{
		return;
		}

	//m_vstmclipboard

	for (i=0;i<m_vmultisel.Size();i++)
		{
		IStream *pstm;
		HGLOBAL hglobal = GlobalAlloc(GMEM_MOVEABLE, 1);

		CreateStreamOnHGlobal(hglobal, TRUE, &pstm);

		//////// BUG!  With multi-select, if you have multiple dragpoints on
		//////// a surface selected, the surface will get copied multiple times
		int type = m_vmultisel.ElementAt(i)->GetIEditable()->GetItemType();
		pstm->Write(&type, sizeof(int), &writ);

		m_vmultisel.ElementAt(i)->GetIEditable()->SaveData(pstm, NULL, NULL);

		vstm.AddElement(pstm);
		}

	/*hr = StgCreateDocfile(NULL,STGM_TRANSACTED | STGM_READWRITE
			| STGM_SHARE_EXCLUSIVE | STGM_CREATE | STGM_DELETEONRELEASE, 0, &pstg);
	hr = pstg->CreateStream(L"Clipboard", STGM_DIRECT | STGM_READWRITE |
			STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstm);*/

								// Go back to beginning of stream to load
								/*LARGE_INTEGER foo;
								foo.QuadPart = 0;
								pstm->Seek(foo, STREAM_SEEK_SET, NULL);*/

	//pstm->Release();

	//g_pvp->SetClipboard(pstg);

	g_pvp->SetClipboard(&vstm);
	}

// BUG - in sync with list in ISelect.h
int rgItemViewAllowed[] =
	{
	1,
	1,
	3,
	1,
	2,
	1,
	1,
	3,
	1,
	3,
	1,
	1,
	1,
	0,
	0,
	0,
	0,
	2,
	0,
	0,
	2,
	};

void PinTable::Paste(BOOL fAtLocation, int x, int y)
	{
	IEditable *peditNew;
	int id;
	int i;
	BOOL fError = fFalse;
	int viewflag;
	int cpasted = 0;

	if (CheckPermissions(DISABLE_CUTCOPYPASTE) == fTrue)
		{
		g_pvp->ShowPermissionError();
		return;
		}

	if (g_pvp->m_fBackglassView)
		{
		viewflag = 2;
		}
	else
		{
		viewflag = 1;
		}

	IStream* pstm;

	// Do a backwards loop, so that the primary selection we had when
	// copying will again be the primary selection, since it will be
	// selected last.  Purely cosmetic.
	for (i=(g_pvp->m_vstmclipboard.Size()-1);i>=0;i--)
	//for (i=0;i<g_pvp->m_vstmclipboard.Size();i++)
		{
		pstm = g_pvp->m_vstmclipboard.ElementAt(i);

		// Go back to beginning of stream to load
		LARGE_INTEGER foo;
		foo.QuadPart = 0;
		pstm->Seek(foo, STREAM_SEEK_SET, NULL);

		ULONG writ = 0;
		int type;
		HRESULT hr = pstm->Read(&type, sizeof(int), &writ);

		if (!(rgItemViewAllowed[type] & viewflag))
			{
			fError = fTrue;
			}
		else
			{
			CreateIEditableFromType(type, &peditNew);
			peditNew->AddRef();

			peditNew->InitLoad(pstm, this, &id, CURRENT_FILE_FORMAT_VERSION, NULL, NULL);
			peditNew->InitVBA(fTrue, 0, NULL);
			m_vedit.AddElement(peditNew);
			peditNew->InitPostLoad();
			peditNew->m_fBackglass = g_pvp->m_fBackglassView;

			AddMultiSel(peditNew->GetISelect(), (i == g_pvp->m_vstmclipboard.Size()-1) ? fFalse : fTrue, fTrue);
			cpasted++;
			}
		}

	// Center view on newly created objects, if they are off the screen
	if (cpasted > 0)
		{
		Vertex vcenter;
		GetCenter(&vcenter);
		}

	if ((cpasted > 0) && fAtLocation)
		{
		Vertex vcenter;
		GetCenter(&vcenter);

		Vertex vPos;

		TransformPoint(x,y,&vPos);

		Vertex vOffset;
		vOffset.x = vPos.x-vcenter.x;
		vOffset.y = vPos.y-vcenter.y;
		Translate(&vOffset);
		}

	if (fError)
		{
		LocalString ls(IDS_NOPASTEINVIEW);
		MessageBox(m_hwnd, ls.m_szbuffer, "Visual Pinball", 0);
		}
	}

/*void PinTable::SetSel(ISelect *psel)
	{
	int i;

	if (m_pselcur == psel || m_vmultisel.IndexOf(psel) != -1)
		{
		// If this item is already part of a multi-select, allow
		// the user to drag them or whatever without holding down
		// shift
		return;
		}

	m_pselcur->m_selectstate = eNotSelected;

	m_pselcur = psel;

	m_pselcur->m_selectstate = eSelected;

	for (i=0;i<m_vmultisel.Size();i++)
		{
		m_vmultisel.ElementAt(i)->m_selectstate = eNotSelected;
		}

	m_vmultisel.RemoveAllElements();

	SetDirtyDraw();

	g_pvp->SetPropSel(psel);

#ifdef VBA
	IApcProjectItem *papi;
	papi = psel->GetIApcProjectItem();
	if (papi)
		{
		papi->Activate();
		}
	else
		{
		IApcControl *pac;
		pac = psel->GetIApcControl();
		if (pac)
			{
			pac->Activate();
			}
		}
#endif
	}*/

void PinTable::PreRender(Sur *psur)
	{
	}

ItemTypeEnum PinTable::GetItemType()
	{
	return eItemTable;
	}

HRESULT PinTable::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
	{
	SetDefaults();

	int csubobj, csounds, ctextures, cfonts, ccollection;

	LoadData(pstm, csubobj, csounds, ctextures, cfonts, ccollection, version, hcrypthash, hcryptkey);

	return S_OK;
	}

HRESULT PinTable::InitPostLoad()
	{
	return S_OK;
	}

HRESULT PinTable::InitVBA(BOOL fNew, int id, WCHAR *wzName)
	{
	return S_OK;
	}

ISelect *PinTable::GetISelect()
	{
	return (ISelect *)this;
	}

void PinTable::SetDefaults()
	{
	}

IScriptable *PinTable::GetScriptable()
	{
	return (IScriptable *)this;
	}

// fUpdate tells us whether to go ahead and change the UI
// based on the new selection, or whether more stuff is coming
// down the pipe (speeds up drag-selection)
void PinTable::AddMultiSel(ISelect *psel, BOOL fAdd, BOOL fUpdate)
	{
	int index = m_vmultisel.IndexOf(psel);

	//_ASSERTE(m_vmultisel.ElementAt(0)->m_selectstate == eSelected);

	if (index == -1) // If we aren't selected yet, do that
		{
		_ASSERTE(psel->m_selectstate == eNotSelected);
		// If we non-shift click on an element outside the multi-select group, delete the old group
		// If the table is currently selected, deselect it - the table can not be part of a multi-select
		if (!fAdd || (m_vmultisel.ElementAt(0) == (ISelect *)this))
			{
			int i;
			for (i=0;i<m_vmultisel.Size();i++)
				{
				m_vmultisel.ElementAt(i)->m_selectstate = eNotSelected;
				}

			m_vmultisel.RemoveAllElements();

			m_vmultisel.AddElement(psel);
			}
		else
			{
			// Make this new selection the primary one for the group
			m_vmultisel.ElementAt(0)->m_selectstate = eMultiSelected;

			m_vmultisel.InsertElementAt(psel, 0);
			}

		psel->m_selectstate = eSelected;
		SetDirtyDraw();
		}
	else if (fAdd) // Take the element off the list
		{
		_ASSERTE(psel->m_selectstate != eNotSelected);
		m_vmultisel.RemoveElementAt(index);
		psel->m_selectstate = eNotSelected;
		if (m_vmultisel.Size() == 0)
			{
			// Have to have something selected
			m_vmultisel.AddElement((ISelect *)this);
			}
		// The main element might have changed
		m_vmultisel.ElementAt(0)->m_selectstate = eSelected;
		SetDirtyDraw();
		}
	else if (m_vmultisel.ElementAt(0) != psel) // Object already in list - no change to selection, only to primary
		{
		_ASSERTE(psel->m_selectstate != eNotSelected);
		// Make this new selection the primary one for the group
		m_vmultisel.ElementAt(0)->m_selectstate = eMultiSelected;

		m_vmultisel.RemoveElementAt(index);

		m_vmultisel.InsertElementAt(psel, 0);

		psel->m_selectstate = eSelected;
		SetDirtyDraw();
		}

	if (fUpdate)
		{
		g_pvp->SetPropSel(&m_vmultisel);
		}

	if (m_vmultisel.ElementAt(0)->GetIEditable() && m_vmultisel.ElementAt(0)->GetIEditable()->GetScriptable())
		{
		m_pcv->SelectItem(m_vmultisel.ElementAt(0)->GetIEditable()->GetScriptable());
		}
	/*else if (m_vmultisel.ElementAt(0) == ((ISelect *)this))
		{
		m_pcv->SelectItem((IScriptable *)this);
		}*/
	}

void PinTable::OnDelete()
	{
	int i;
	Vector<ISelect> m_vseldelete;

	for (i=0;i<m_vmultisel.Size();i++)
		{
		// Can't delete these items yet - AddMultiSel will try to mark them as unselected
		m_vseldelete.AddElement(m_vmultisel.ElementAt(i));
		}

	AddMultiSel((ISelect *)this, fFalse, fTrue); // Will get rid of the multi-selection

	for (i=0;i<m_vseldelete.Size();i++)
		{
		m_vseldelete.ElementAt(i)->Delete();
		}
	}

void PinTable::OnKeyDown(int key)
	{
	int fShift, fCtrl, fAlt;
	fShift = GetKeyState(VK_SHIFT) & 0x8000;
	fCtrl = GetKeyState(VK_CONTROL) & 0x8000;
	fAlt = GetKeyState(VK_MENU) & 0x8000;

	switch (key)
		{
		//m_pselcur->Delete();
		case VK_DELETE:
			{
			int i;
			Vector<ISelect> m_vseldelete;

			for (i=0;i<m_vmultisel.Size();i++)
				{
				// Can't delete these items yet - AddMultiSel will try to mark them as unselected
				m_vseldelete.AddElement(m_vmultisel.ElementAt(i));
				}

			AddMultiSel((ISelect *)this, fFalse, fTrue); // Will get rid of the multi-selection

			for (i=0;i<m_vseldelete.Size();i++)
				{
				m_vseldelete.ElementAt(i)->Delete();
				}
			//SetSel(this);
			//m_pselcur = this;
			//SetDirtyDraw();
			}
			break;

		case VK_LEFT:
		case VK_RIGHT:
		case VK_UP:
		case VK_DOWN:
			{
			BeginUndo();
			int distance = fShift ? 10 : 1;
			int i;
			for (i=0;i<m_vmultisel.Size();i++)
				{
				ISelect *pisel = m_vmultisel.ElementAt(i);
				if (!pisel->GetIEditable()->GetISelect()->m_fLocked) // control points get lock info from parent - UNDONE - make this code snippet be in one place
					{
					switch (key)
						{
						case VK_LEFT:
							pisel->GetIEditable()->MarkForUndo();
							pisel->MoveOffset(-distance/m_zoom,0);
							break;

						case VK_RIGHT:
							pisel->GetIEditable()->MarkForUndo();
							pisel->MoveOffset(distance/m_zoom,0);
							break;

						case VK_UP:
							pisel->GetIEditable()->MarkForUndo();
							pisel->MoveOffset(0,-distance/m_zoom);
							break;

						case VK_DOWN:
							pisel->GetIEditable()->MarkForUndo();
							pisel->MoveOffset(0,distance/m_zoom);
							break;
						}
					}
				}
			EndUndo();
			}
			break;

		/*case VK_LEFT:
			{
			int i;
			i = m_vedit.IndexOf(m_pselcur->GetIEditable());
			if (i == -1)
				{
				i = m_vedit.Size();
				}
			i--;

			if (i < 0)
				{
				SetSel(this);
				}
			else
				{
				SetSel(m_vedit.ElementAt(i)->GetISelect());
				}
			}
			break;

		case VK_RIGHT:
			{
			int i;
			i = m_vedit.IndexOf(m_pselcur->GetIEditable());
			// if i == -1, the table itself is selected
			i++;
			if (i >= m_vedit.Size())
				{
				SetSel(this);
				}
			else
				{
				SetSel(m_vedit.ElementAt(i)->GetISelect());
				}
			}
			break;*/

		/*case 'C':
			if (fCtrl)
				{
				Copy();
				}
			break;
		case 'V':
			if (fCtrl)
				{
				Paste();
				}
			break;*/
		}
	}

void PinTable::UseTool(int x,int y,int tool)
	{
	Vertex v;
	TransformPoint(x,y,&v);
	IEditable *pie;

	pie = NULL;

	switch (tool)
		{
		case IDC_WALL:
			CComObject<Surface> *psur;
			CComObject<Surface>::CreateInstance(&psur);
			if (psur)
				{
				psur->AddRef();
				psur->Init(this, v.x, v.y);
				pie = (IEditable *)psur;
				}
			break;
		case IDC_TARGET:
			{
			CComObject<Surface> *psur;
			CComObject<Surface>::CreateInstance(&psur);
			if (psur)
				{
				psur->AddRef();
				psur->InitTarget(this, v.x, v.y);
				pie = (IEditable *)psur;
				}
			}
			break;
		case IDC_FLIPPER:
			CComObject<Flipper> *pflipper;
			CComObject<Flipper>::CreateInstance(&pflipper);
			if (pflipper)
				{
				pflipper->AddRef();
				pflipper->Init(this, v.x, v.y);
				pie = (IEditable *)pflipper;
				}
			break;
		case IDC_TIMER:
			CComObject<Timer> *ptimer;
			CComObject<Timer>::CreateInstance(&ptimer);
			if (ptimer)
				{
				ptimer->AddRef();
				ptimer->Init(this, v.x, v.y);
				pie = (IEditable *)ptimer;
				}
			break;
		case IDC_PLUNGER:
			CComObject<Plunger> *pplunger;
			CComObject<Plunger>::CreateInstance(&pplunger);
			if (pplunger)
				{
				pplunger->AddRef();
				pplunger->Init(this, v.x, v.y);
				pie = (IEditable *)pplunger;
				}
			break;
		case IDC_TEXTBOX:
			CComObject<Textbox> *ptextbox;
			CComObject<Textbox>::CreateInstance(&ptextbox);
			if (ptextbox)
				{
				ptextbox->AddRef();
				ptextbox->Init(this, v.x, v.y);
				pie = (IEditable *)ptextbox;
				}
			break;
		case IDC_COMCONTROL:
			CComObject<PinComControl> *pcomcontrol;
			CComObject<PinComControl>::CreateInstance(&pcomcontrol);
			if (pcomcontrol)
				{
				pcomcontrol->AddRef();
				HRESULT hr = pcomcontrol->Init(this, v.x, v.y);
				if (hr == E_FAIL)
					{
					pie = NULL;
					pcomcontrol->Release();
					}
				else
					{
					pie = (IEditable *)pcomcontrol;
					}
				}
			break;
		case IDC_BUMPER:
			CComObject<Bumper> *pbumper;
			CComObject<Bumper>::CreateInstance(&pbumper);
			if (pbumper)
				{
				pbumper->AddRef();
				pbumper->Init(this, v.x, v.y);
				pie = (IEditable *)pbumper;
				}
			break;
		case IDC_TRIGGER:
			CComObject<Trigger> *ptrigger;
			CComObject<Trigger>::CreateInstance(&ptrigger);
			if (ptrigger)
				{
				ptrigger->AddRef();
				ptrigger->Init(this, v.x, v.y);
				pie = (IEditable *)ptrigger;
				}
			break;
		case IDC_LIGHT:
			CComObject<Light> *plight;
			CComObject<Light>::CreateInstance(&plight);
			if (plight)
				{
				plight->AddRef();
				plight->Init(this, v.x, v.y);
				pie = (IEditable *)plight;
				}
			//IApcPropertiesWindow *pfoo;
			//g_pvp->ApcHost->get_PropertiesWindow(&pfoo);
			//pfoo->put_Visible(VARIANT_TRUE);
			break;
		case IDC_KICKER:
			CComObject<Kicker> *pkicker;
			CComObject<Kicker>::CreateInstance(&pkicker);
			if (pkicker)
				{
				pkicker->AddRef();
				pkicker->Init(this, v.x, v.y);
				pie = (IEditable *)pkicker;
				}
			break;
		case IDC_DECAL:
			CComObject<Decal> *pdecal;
			CComObject<Decal>::CreateInstance(&pdecal);
			if (pdecal)
				{
				pdecal->AddRef();
				pdecal->Init(this, v.x, v.y);
				pie = (IEditable *)pdecal;
				}
			break;
		case IDC_GATE:
			CComObject<Gate> *pgate;
			CComObject<Gate>::CreateInstance(&pgate);
			if (pgate)
				{
				pgate->AddRef();
				pgate->Init(this, v.x, v.y);
				pie = (IEditable *)pgate;
				}
			break;
		case IDC_SPINNER:
			CComObject<Spinner> *pspinner;
			CComObject<Spinner>::CreateInstance(&pspinner);
			if (pspinner)
				{
				pspinner->AddRef();
				pspinner->Init(this, v.x, v.y);
				pie = (IEditable *)pspinner;
				}
			break;
		case IDC_RAMP:
			CComObject<Ramp> *pramp;
			CComObject<Ramp>::CreateInstance(&pramp);
			if (pramp)
				{
				pramp->AddRef();
				pramp->Init(this, v.x, v.y);
				pie = (IEditable *)pramp;
				}
			break;
		case IDC_DISPREEL:
			CComObject<DispReel> *pdispreel;
			CComObject<DispReel>::CreateInstance(&pdispreel);
			if (pdispreel)
				{
				pdispreel->AddRef();
				pdispreel->Init(this, v.x, v.y);
				pie = (IEditable *)pdispreel;
				}
			break;
		case IDC_LIGHTSEQ:
			CComObject<LightSeq> *plightseq;
			CComObject<LightSeq>::CreateInstance(&plightseq);
			if (plightseq)
				{
				plightseq->AddRef();
				plightseq->Init(this, v.x, v.y);
				pie = (IEditable *)plightseq;
				}
			break;
		}

	if (pie)
		{
		pie->m_fBackglass = g_pvp->m_fBackglassView;
		m_vedit.AddElement(pie);
		//SetSel(pie->GetISelect()); // Set dirty draw so the thing gets drawn at all
		AddMultiSel(pie->GetISelect(), fFalse, fTrue);
		BeginUndo();
		m_undo.MarkForCreate(pie);
		EndUndo();
		}

	g_pvp->ParseCommand(IDC_SELECT, g_pvp->m_hwnd, 0);
	}

void PinTable::TransformPoint(int x, int y, Vertex *pv)
	{
	HitSur *phs;

	RECT rc;
	GetClientRect(m_hwnd, &rc);

	phs = new HitSur(NULL, m_zoom, m_offsetx, m_offsety, rc.right - rc.left, rc.bottom - rc.top, 0, 0, NULL);

	phs->ScreenToSurface(x, y, &pv->x, &pv->y);

	delete phs;
	}

void PinTable::OnLButtonDown(int x, int y)
	{
	Vertex v;

	TransformPoint(x,y,&v);

	m_rcDragRect.left = v.x;
	m_rcDragRect.right = v.x;
	m_rcDragRect.top = v.y;
	m_rcDragRect.bottom = v.y;

	m_fDragging = fTrue;

	SetCapture(GetPTable()->m_hwnd);

	SetDirtyDraw();
	}

void PinTable::OnLButtonUp(int x, int y)
	{
	if (m_fDragging)
		{
		m_fDragging = fFalse;
		ReleaseCapture();
		if ((m_rcDragRect.left != m_rcDragRect.right) || (m_rcDragRect.top != m_rcDragRect.bottom))
			{
			HitRectSur *phrs;
			HDC hdc;
			Vector<ISelect> vsel;
			int i;

			hdc = GetDC(m_hwnd);

			RECT rc;
			GetClientRect(m_hwnd, &rc);

			phrs = new HitRectSur(hdc, m_zoom, m_offsetx, m_offsety, rc.right - rc.left, rc.bottom - rc.top, &m_rcDragRect, &vsel);

			// Just want one rendering pass (no PreRender) so we don't select things twice
			/*for (i=0;i<m_vedit.Size();i++)
				{
				m_vedit.ElementAt(i)->Render(phrs);
				}*/
			Render(phrs);

			int ksshift;
			ksshift = GetKeyState(VK_SHIFT);
			BOOL fAdd = (ksshift & 0x80000000) != 0;

			int minlevel = 999;

			for (i=0;i<vsel.Size();i++)
				{
				minlevel = min(minlevel, vsel.ElementAt(i)->GetSelectLevel());
				}

			if (vsel.Size() > 0)
				{
				BOOL fFirstAdd = fTrue;

				for (i=0;i<vsel.Size();i++)
					{
					if (vsel.ElementAt(i)->GetSelectLevel() == minlevel)
						{
						if (fFirstAdd)
							{
							// If the shift key is not down, adding the first
							// element will clear the previously selected group
							AddMultiSel(vsel.ElementAt(i), fAdd, fFalse);
							fFirstAdd = fFalse;
							}
						else
							{
							AddMultiSel(vsel.ElementAt(i), fTrue, fFalse);
							}
						}
					}
					
				// We told the UI to not update in AddMultiSel because
				// we were potentially adding a bunch of things.
				// So we have to update it manually here.
				g_pvp->SetPropSel(&m_vmultisel);
				}

			delete phrs;

			ReleaseDC(m_hwnd, hdc);
			}
		}

	SetDirtyDraw();
	}

void PinTable::OnMouseMove(int x, int y)
	{
	Vertex v;

	TransformPoint(x,y,&v);

	m_rcDragRect.right = v.x;
	m_rcDragRect.bottom = v.y;

	/*g_pvp->SetPosCur(v.x, v.y);*/

	if (m_fDragging)
		{
		SetDirtyDraw();
		}
	}

HRESULT PinTable::GetTypeName(BSTR *pVal)
	{
	WCHAR wzName[128];

	int stringid;

	if (!g_pvp->m_fBackglassView)
		{
		stringid = IDS_TABLE;
		}
	else
		{
		stringid = IDS_TB_BACKGLASS;
		}

	LocalString ls(stringid);

	MultiByteToWideChar(CP_ACP, 0, ls.m_szbuffer, -1, wzName, 128);

	*pVal = SysAllocString(wzName);

	return S_OK;
	}

/*int PinTable::GetDialogID()
	{
	if (!g_pvp->m_fBackglassView)
		{
		return IDD_PROPTABLE;
		}
	else
		{
		return IDD_PROPBACKGLASS;
		}
	}*/

void PinTable::GetDialogPanes(Vector<PropertyPane> *pvproppane)
	{
	if (!g_pvp->m_fBackglassView)
		{
		PropertyPane *pproppane;

		pproppane = new PropertyPane(IDD_PROP_NAME, NULL);
		pvproppane->AddElement(pproppane);

		pproppane = new PropertyPane(IDD_PROPTABLE_EDITOR, IDS_EDITOR);
		pvproppane->AddElement(pproppane);

		pproppane = new PropertyPane(IDD_PROPTABLE_VISUALS, IDS_VISUALS);
		pvproppane->AddElement(pproppane);

		pproppane = new PropertyPane(IDD_PROPTABLE_PHYSICS, IDS_PHYSICS);
		pvproppane->AddElement(pproppane);

		pproppane = new PropertyPane(IDD_PROPTABLE_BALL, IDS_DEFAULTBALL);
		pvproppane->AddElement(pproppane);
		}
	else
		{
		PropertyPane *pproppane;

		pproppane = new PropertyPane(IDD_PROPTABLE_EDITOR, IDS_EDITOR);
		pvproppane->AddElement(pproppane);

		pproppane = new PropertyPane(IDD_PROPBACKGLASS_VISUALS, IDS_VISUALS);
		pvproppane->AddElement(pproppane);
		}
	}

LRESULT CALLBACK TableWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	HDC hdc;
	PAINTSTRUCT ps;
	CComObject<PinTable> *pt;

	switch (uMsg)
		{
		case WM_CLOSE:
			// Scary!!!!
			pt = (CComObject<PinTable> *)GetWindowLong(hwnd, GWL_USERDATA);
			//DefMDIChildProc(hwnd, uMsg, wParam, lParam);
			pt->m_pvp->CloseTable(pt);
			//DestroyWindow(hwnd);
			return 0;
			break;

		case WM_TIMER:
			{
			pt = (CComObject<PinTable> *)GetWindowLong(hwnd, GWL_USERDATA);
			switch (wParam)
				{
				case TIMER_ID_AUTOSAVE:
					pt->AutoSave();
					break;
				}
			}
			break;

		case WM_SETCURSOR:
			{
			if (LOWORD(lParam) == HTCLIENT)
				{
				HCURSOR hcursor;
				char *cursorid;
				HINSTANCE hinst = g_hinst;
				switch (g_pvp->m_ToolCur)
					{
					default:
					case IDC_SELECT:
						hinst = NULL;
						cursorid = IDC_ARROW;
						break;

					case IDC_MAGNIFY:
						cursorid = MAKEINTRESOURCE(IDC_CUR_MAGNIFY);
						break;

					case IDC_WALL:
						cursorid = MAKEINTRESOURCE(IDC_CUR_WALL);
						break;

					case IDC_LIGHT:
						cursorid = MAKEINTRESOURCE(IDC_CUR_LIGHT);
						break;

					case IDC_FLIPPER:
						cursorid = MAKEINTRESOURCE(IDC_CUR_FLIPPER);
						break;

					case IDC_TRIGGER:
						cursorid = MAKEINTRESOURCE(IDC_CUR_TRIGGER);
						break;

					case IDC_GATE:
						cursorid = MAKEINTRESOURCE(IDC_CUR_GATE);
						break;

					case IDC_TIMER:
						cursorid = MAKEINTRESOURCE(IDC_CUR_TIMER);
						break;

					case IDC_PLUNGER:
						cursorid = MAKEINTRESOURCE(IDC_CUR_PLUNGER);
						break;

					case IDC_TEXTBOX:
						cursorid = MAKEINTRESOURCE(IDC_CUR_TEXTBOX);
						break;

					case IDC_COMCONTROL:
						cursorid = MAKEINTRESOURCE(IDC_CUR_TEXTBOX);
						break;

					case IDC_BUMPER:
						cursorid = MAKEINTRESOURCE(IDC_CUR_BUMPER);
						break;

					case IDC_KICKER:
						cursorid = MAKEINTRESOURCE(IDC_CUR_KICKER);
						break;

					case IDC_TARGET:
						cursorid = MAKEINTRESOURCE(IDC_CUR_TARGET);
						break;

					case IDC_DECAL:
						cursorid = MAKEINTRESOURCE(IDC_CUR_DECAL);
						break;

					case IDC_SPINNER:
						cursorid = MAKEINTRESOURCE(IDC_CUR_SPINNER);
						break;

					case IDC_RAMP:
						cursorid = MAKEINTRESOURCE(IDC_CUR_RAMP);
						break;

					case IDC_DISPREEL:
						cursorid = MAKEINTRESOURCE(IDC_CUR_DISPREEL);
						break;

					case IDC_LIGHTSEQ:
					cursorid = MAKEINTRESOURCE(IDC_CUR_LIGHTSEQ);
					break;
					}
				hcursor = LoadCursor(hinst, cursorid);
				SetCursor(hcursor);
				return TRUE;
				}
			}
			break;

		case WM_PAINT:
			hdc = BeginPaint(hwnd,&ps);
			pt = (CComObject<PinTable> *)GetWindowLong(hwnd, GWL_USERDATA);
			pt->Paint(hdc);
			EndPaint(hwnd,&ps);
			break;

		case WM_MOUSEACTIVATE:
		case WM_ACTIVATE:
			if (LOWORD(wParam) != WA_INACTIVE)
				{
				pt = (CComObject<PinTable> *)GetWindowLong(hwnd, GWL_USERDATA);
				pt->m_pvp->m_ptableActive = pt;

				// re-evaluate the toolbar/menuitems depending on table permissions
				g_pvp->SetEnableToolbar();
				g_pvp->SetEnableMenuItems();
				}
			break;

		case WM_LBUTTONDOWN:
			pt = (CComObject<PinTable> *)GetWindowLong(hwnd, GWL_USERDATA);
			short x,y;
			x = (short)(lParam & 0xffff);
			y = (short)((lParam>>16) & 0xffff);

			if ((g_pvp->m_ToolCur == IDC_SELECT) || (g_pvp->m_ToolCur == IDC_MAGNIFY))
				{
				pt->DoLButtonDown(x,y);
				}
			else
				{
				pt->UseTool(x,y,g_pvp->m_ToolCur);
				}
			break;

		case WM_LBUTTONDBLCLK:
			pt = (CComObject<PinTable> *)GetWindowLong(hwnd, GWL_USERDATA);
			x = (short)(lParam & 0xffff);
			y = (short)((lParam>>16) & 0xffff);
			pt->DoLDoubleClick(x,y);
			break;

		case WM_LBUTTONUP:
			{
			short x,y;
			x = (short)(lParam & 0xffff);
			y = (short)((lParam>>16) & 0xffff);
			pt = (CComObject<PinTable> *)GetWindowLong(hwnd, GWL_USERDATA);
			pt->DoLButtonUp(x,y);
			}
			break;

		case WM_MOUSEMOVE:
			{
			short x,y;
			x = (short)(lParam & 0xffff);
			y = (short)((lParam>>16) & 0xffff);
			pt = (CComObject<PinTable> *)GetWindowLong(hwnd, GWL_USERDATA);
			pt->DoMouseMove(x,y);
			}
			break;

		case WM_RBUTTONDOWN:
			pt = (CComObject<PinTable> *)GetWindowLong(hwnd, GWL_USERDATA);
			x = (short)(lParam & 0xffff);
			y = (short)((lParam>>16) & 0xffff);

			pt->DoRButtonDown(x,y);
			break;

		case WM_RBUTTONUP:
			pt = (CComObject<PinTable> *)GetWindowLong(hwnd, GWL_USERDATA);
			x = (short)(lParam & 0xffff);
			y = (short)((lParam>>16) & 0xffff);
			pt->DoRButtonUp(x,y);
			break;

		case WM_KEYDOWN:
			pt = (CComObject<PinTable> *)GetWindowLong(hwnd, GWL_USERDATA);
			pt->OnKeyDown(wParam);
			break;

		case WM_HSCROLL:
			{
			pt = (CComObject<PinTable> *)GetWindowLong(hwnd, GWL_USERDATA);
			SCROLLINFO si;
			si.cbSize = sizeof(SCROLLINFO);
			si.fMask = SIF_ALL;
			GetScrollInfo(hwnd, SB_HORZ, &si);
			switch (LOWORD(wParam))
				{
				case SB_LINELEFT:
					pt->m_offsetx -= si.nPage/10;
					break;
				case SB_LINERIGHT:
					pt->m_offsetx += si.nPage/10;
					break;
				case SB_PAGELEFT:
					pt->m_offsetx -= si.nPage/2;
					break;
				case SB_PAGERIGHT:
					pt->m_offsetx += si.nPage/2;
					break;
				case SB_THUMBTRACK:
					int delta;
					delta = (int)(pt->m_offsetx - si.nPos);
					pt->m_offsetx = (float)((short)HIWORD(wParam) + delta);
					break;
				}
			pt->SetDirtyDraw();
			pt->SetMyScrollInfo();
			return 0;
			}
			break;

		case WM_VSCROLL:
			{
			pt = (CComObject<PinTable> *)GetWindowLong(hwnd, GWL_USERDATA);
			SCROLLINFO si;
			si.cbSize = sizeof(SCROLLINFO);
			si.fMask = SIF_ALL;
			GetScrollInfo(hwnd, SB_VERT, &si);
			switch (LOWORD(wParam))
				{
				case SB_LINEUP:
					pt->m_offsety -= si.nPage/10;
					break;
				case SB_LINEDOWN:
					pt->m_offsety += si.nPage/10;
					break;
				case SB_PAGEUP:
					pt->m_offsety -= si.nPage/2;
					break;
				case SB_PAGEDOWN:
					pt->m_offsety += si.nPage/2;
					break;
				case SB_THUMBTRACK:
					int delta;
					delta = (int)(pt->m_offsety - si.nPos);
					pt->m_offsety = (float)((short)HIWORD(wParam) + delta);
					break;
				}
			pt->SetDirtyDraw();
			pt->SetMyScrollInfo();
			return 0;
			}
			break;

	    case WM_MOUSEWHEEL:
			{
			pt = (CComObject<PinTable> *)GetWindowLong(hwnd, GWL_USERDATA);
			short zDelta = (short) HIWORD(wParam);    // wheel rotation
			pt->m_offsety -= zDelta / pt->m_zoom;	// change to orientation to match windows default
			pt->SetDirtyDraw();
			pt->SetMyScrollInfo();
			return 0;
			}
			break;

		case WM_SIZE:
			pt = (CComObject<PinTable> *)GetWindowLong(hwnd, GWL_USERDATA);
			if (pt) // Window might have just been created
				{
				pt->SetMyScrollInfo();
				pt->m_fDirtyDraw = fTrue;
				// this window command is called whenever the MDI window changes over
				// re-evaluate the toolbar/menuitems depending on table permissions
				g_pvp->SetEnableToolbar();
				g_pvp->SetEnableMenuItems();
				}
			break;

		case WM_COMMAND:
			int blah;
			blah = 1;
			break;

		case DONE_AUTOSAVE:
			{
			pt = (CComObject<PinTable> *)GetWindowLong(hwnd, GWL_USERDATA);
			if (lParam == S_OK)
				{
				g_pvp->SetActionCur("");
				}
			else
				{
				g_pvp->SetActionCur("Autosave Failed");
				}
			pt->BeginAutoSaveCounter();
			HANDLE hEvent = (HANDLE)wParam;
			pt->m_vAsyncHandles.RemoveElement((int)hEvent);
			CloseHandle(hEvent);
			}
			break;
		}
	return DefMDIChildProc(hwnd, uMsg, wParam, lParam);
	//return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

STDMETHODIMP PinTable::get_Name(BSTR *pVal)
{
	//CComBSTR bstr;
	//GetIApcProjectItem()->get_Name(&bstr);
	*pVal = SysAllocString((WCHAR *)m_wzName);

	return S_OK;
}

STDMETHODIMP PinTable::put_Name(BSTR newVal)
{
	//GetIApcProjectItem()->put_Name(newVal);

	STARTUNDO

	if (lstrlenW(newVal) > 32)
		{
		return E_FAIL;
		}

	if (m_pcv->ReplaceName((IScriptable *)this, newVal) == S_OK)
		{
		WideStrCopy(newVal, (WCHAR *)m_wzName);
		//lstrcpyW((WCHAR *)m_wzName, newVal);
		}

#ifdef VBA
	GetIApcProjectItem()->put_Name(newVal);
#endif

	STOPUNDO

	return S_OK;
}

STDMETHODIMP PinTable::get_Inclination(float *pVal)
{
	*pVal = m_inclination;

	return S_OK;
}

STDMETHODIMP PinTable::put_Inclination(float newVal)
{
	STARTUNDO

	m_inclination = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP PinTable::get_FieldOfView(float *pVal)
{
	*pVal = m_FOV;

	return S_OK;
}

STDMETHODIMP PinTable::put_FieldOfView(float newVal)
{
	STARTUNDO

	m_FOV = newVal;

	STOPUNDO

	return S_OK;
}

void PinTable::ClearOldSounds()
	{
	int i;
	DWORD status;

	for (i=0;i<m_voldsound.Size();i++)
		{
		//LPDIRECTSOUNDBUFFER pdsbOld = (LPDIRECTSOUNDBUFFER)(m_voldsound.ElementAt(i));
		PinSoundCopy *ppsc = m_voldsound.ElementAt(i);
		ppsc->m_pDSBuffer->GetStatus(&status);
		if (!(status & DSBSTATUS_PLAYING)) //sound is done, we can throw it away now
			{
			ppsc->m_pDSBuffer->Release();
			m_voldsound.RemoveElementAt(i);
			delete ppsc;
			i--; // elements will shift, check this one again
			}
		}
	}

HRESULT PinTable::StopSound(BSTR Sound)
	{
	int i;

	MAKE_ANSIPTR_FROMWIDE(szName, Sound);
	CharLowerBuff(szName, lstrlen(szName));

	// In case we were playing any of the main buffers
	for (i=0;i<m_vsound.Size();i++)
		{
		if (!lstrcmp(m_vsound.ElementAt(i)->m_szInternalName, szName))
			{
			m_vsound.ElementAt(i)->m_pDSBuffer->Stop();
			break;
			}
		}

	for (i=0;i<m_voldsound.Size();i++)
		{
		PinSoundCopy *ppsc = m_voldsound.ElementAt(i);
		if (!lstrcmp(ppsc->m_ppsOriginal->m_szInternalName, szName))
			{
			ppsc->m_pDSBuffer->Stop();
			break;
			}
		}

	return S_OK;
	}

STDMETHODIMP PinTable::PlaySound(BSTR bstr, int loopcount, float volume)
{
	MAKE_ANSIPTR_FROMWIDE(szName, bstr);
	CharLowerBuff(szName, lstrlen(szName));

	int i;

	for (i=0;i<m_vsound.Size();i++)
		{
		if (!lstrcmp(m_vsound.ElementAt(i)->m_szInternalName, szName))
			{
			break;
			}
		}

	if (i==m_vsound.Size()) // did not find it
		{
		return S_OK;
		}

	ClearOldSounds();

	int flags = (loopcount == -1) ? DSBPLAY_LOOPING : 0;
	float totalvolume = ((float)g_pplayer->m_SoundVolume)*volume;
	int decibelvolume = (int)(((log((float)totalvolume)/log(10.0f))*1000) - 2000); // 10 volume = -10Db

	DWORD status;

	LPDIRECTSOUNDBUFFER pdsb = m_vsound.ElementAt(i)->m_pDSBuffer;

	//LPDIRECTSOUNDBUFFER pdsbNew;

	PinSoundCopy *ppsc = new PinSoundCopy();

	g_pvp->m_pds.m_pDS->DuplicateSoundBuffer(pdsb, &ppsc->m_pDSBuffer/*&pdsbNew*/);

	if (ppsc->m_pDSBuffer)
		{
		ppsc->m_pDSBuffer->SetVolume(decibelvolume);

		ppsc->m_pDSBuffer->Play(0,0,flags);
		ppsc->m_ppsOriginal = m_vsound.ElementAt(i);
		m_voldsound.AddElement(ppsc);
		//pdsbNew->Release();
		}
	else // Couldn't create a copy - just play the original and hope it doesn't get played again before it finishes
		{
		delete ppsc;

		pdsb->GetStatus(&status);

		if (!(status & DSBSTATUS_PLAYING))
			{
			pdsb->SetVolume(decibelvolume);
			pdsb->Play(0,0,flags);
			}
		else // Okay, it got played again before it finished.  Well, just start it over.
			{
			pdsb->SetCurrentPosition(0);
			}
		}

	return S_OK;
}

PinImage *PinTable::GetImage(char *szName)
	{
	CharLowerBuff(szName, lstrlen(szName));

	int i;

	for (i=0;i<m_vimage.Size();i++)
		{
		if (!lstrcmp(m_vimage.ElementAt(i)->m_szInternalName, szName))
			{
			return m_vimage.ElementAt(i);
			}
		}

	return NULL;
	}

void PinTable::GetTVTU(PinImage *ppi, float *pmaxtu, float *pmaxtv)
	{
	DDSURFACEDESC2 ddsd;
	ddsd.dwSize = sizeof(ddsd);

	ppi->m_pdsBuffer->GetSurfaceDesc(&ddsd);

	*pmaxtu = (float)ppi->m_width / (float)ddsd.dwWidth;
	*pmaxtv = (float)ppi->m_height / (float)ddsd.dwHeight;
	}

void PinTable::CreateGDIBackdrop()
	{
	/*if (m_hbmBackdrop)
		{
		DeleteObject(m_hbmBackdrop);
		m_hbmBackdrop = NULL;
		}

	PinImage *ppi = GetImage(m_szImage);

	if (ppi)
		{
		HDC hdcImage;
		ppi->m_pdsBuffer->GetDC(&hdcImage);

		//StretchBlt(psur->m_hdc, 0, 0, ppi->m_width, ppi->m_height, hdcImage, 0, 0, ppi->m_width, ppi->m_height, SRCCOPY);

		HDC hdcScreen = GetDC(NULL);
		m_hbmBackdrop = CreateCompatibleBitmap(hdcScreen, ppi->m_width, ppi->m_height);
		HDC hdcNew = CreateCompatibleDC(hdcScreen);
		HBITMAP hbmOld = (HBITMAP)SelectObject(hdcNew, m_hbmBackdrop);

		StretchBlt(hdcNew, 0, 0, ppi->m_width, ppi->m_height, hdcImage, 0, 0, ppi->m_width, ppi->m_height, SRCCOPY);

		SelectObject(hdcNew, hbmOld);
		DeleteDC(hdcNew);
		ReleaseDC(NULL,hdcScreen);

		ppi->m_pdsBuffer->ReleaseDC(hdcImage);
		}*/
	}

void PinTable::ReImportImage(HWND hwndListView, PinImage *ppi, char *filename)
	{
	PinImage piT;

	char szextension[MAX_PATH];
	ExtensionFromFilename(filename, szextension);

	BOOL fBinary;
	PinBinary *ppb;

	//ppi->m_ppb;// = new PinBinary();

	if (!lstrcmpi(szextension, "bmp"))
		{
		fBinary = fFalse;
		}
	else if (!lstrcmpi(szextension, "jpg") || !lstrcmpi(szextension, "jpeg"))// jpeg
		{
		fBinary = fTrue;
		}
	else
		{
		ShowError("Unknown image type.");
		}

	if (fBinary)
		{
		ppb = new PinBinary();
		ppb->ReadFromFile(filename);
		}

	// Make sure we can import the new file before blowing away anything we had before
	if (!fBinary)
		{
		piT.m_pdsBuffer = g_pvp->m_pdd.CreateFromFile(filename, &ppi->m_width, &ppi->m_height);
		}
	else
		{
		piT.m_pdsBuffer = g_pvp->m_pdd.DecompressJPEG(ppi/*filename*/, ppb, &ppi->m_width, &ppi->m_height);
		}

	if (piT.m_pdsBuffer == NULL)
		{
		//delete ppi;
		return;
		}

	ppi->FreeStuff();

	if (fBinary)
		{
		ppi->m_ppb = ppb;
		}

	//SAFE_RELEASE(ppi->m_pdsBuffer);

	ppi->m_pdsBuffer = piT.m_pdsBuffer;
	ppi->m_pdsBuffer->AddRef();

	lstrcpy(ppi->m_szPath, filename);

	g_pvp->m_pdd.SetOpaque(ppi->m_pdsBuffer, ppi->m_width, ppi->m_height);
	g_pvp->m_pdd.CreateNextMipMapLevel(ppi->m_pdsBuffer);

	ppi->EnsureMaxTextureCoordinates();
	}

void PinTable::ImportImage(HWND hwndListView, char *filename)
	{
	PinImage *ppi;

	ppi = new PinImage();

	ReImportImage(hwndListView, ppi, filename);

	if (ppi->m_pdsBuffer == NULL)
		{
		delete ppi;
		return;
		}

	// The first time we import a file, parse the name of the texture from the filename

	int begin, end;

	int len = lstrlen(filename);

	for (begin=len;begin>=0;begin--)
		{
		if (filename[begin] == '\\')
			{
			begin++;
			break;
			}
		}

	for (end=len;end>=0;end--)
		{
		if (filename[end] == '.')
			{
			break;
			}
		}

	if (end == 0)
		{
		end = len-1;
		}

	lstrcpy(ppi->m_szName, &filename[begin]);

	ppi->m_szName[end-begin] = 0;

	lstrcpy(ppi->m_szInternalName, ppi->m_szName);

	CharLowerBuff(ppi->m_szInternalName, lstrlen(ppi->m_szInternalName));
	m_vimage.AddElement(ppi);

	int index = AddListImage(hwndListView, ppi);

	ListView_SetItemState(hwndListView, index, LVIS_SELECTED, LVIS_SELECTED);
	}

void PinTable::ListImages(HWND hwndListView)
	{
	int i;
	for (i=0;i<m_vimage.Size();i++)
		{
		AddListImage(hwndListView, m_vimage.ElementAt(i));
		}
	}

int PinTable::AddListImage(HWND hwndListView, PinImage *ppi)
	{
	LVITEM lvitem;

	lvitem.mask = LVIF_DI_SETITEM | LVIF_TEXT | LVIF_PARAM;
	lvitem.iItem = 0;
	lvitem.iSubItem = 0;
	lvitem.pszText = ppi->m_szName;
	lvitem.lParam = (long)ppi;

	int index = ListView_InsertItem(hwndListView, &lvitem);

	//lvitem.iSubItem = 1;
	//lvitem.pszText = "d:\\foo\\lah";

	ListView_SetItemText(hwndListView, index, 1, ppi->m_szPath);

	return index;
	}

void PinTable::RemoveImage(PinImage *ppi)
	{
	m_vimage.RemoveElement(ppi);
	delete ppi;
	}

int PinTable::GetImageLink(PinImage *ppi)
	{
	if (!lstrcmp(ppi->m_szInternalName, m_szScreenShot))
		{
		return 1;
		}

	return 0;
	}

PinBinary *PinTable::GetImageLinkBinary(int id)
	{
	switch (id)
		{
		case 1: //Screenshot
			// Transfer ownership of the screenshot pinbary blob to the image
			PinBinary *pbT = m_pbTempScreenshot;
			m_pbTempScreenshot = NULL;
			return pbT;
			break;
		}

	return NULL;
	}

void PinTable::ListCustomInfo(HWND hwndListView)
	{
	int i;
	for (i=0;i<m_vCustomInfoTag.Size();i++)
		{
		AddListItem(hwndListView, m_vCustomInfoTag.ElementAt(i), m_vCustomInfoContent.ElementAt(i), NULL);
		}
	}

int PinTable::AddListItem(HWND hwndListView, char *szName, char *szValue1, LPARAM lparam)
	{
	LVITEM lvitem;

	lvitem.mask = LVIF_DI_SETITEM | LVIF_TEXT | LVIF_PARAM;
	lvitem.iItem = 0;
	lvitem.iSubItem = 0;
	lvitem.pszText = szName;
	lvitem.lParam = lparam;

	int index = ListView_InsertItem(hwndListView, &lvitem);

	ListView_SetItemText(hwndListView, index, 1, szValue1);

	return index;
	}

/*HRESULT PinTable::SaveImageToStream(PinImage *ppi, IStream *pstm)
	{
	int len;
	ULONG writ = 0;
	HRESULT hr = S_OK;

	len = lstrlen(ppi->m_szName);

	if(FAILED(hr = pstm->Write(&len, sizeof(int), &writ)))
		return hr;

	if(FAILED(hr = pstm->Write(ppi->m_szName, len, &writ)))
		return hr;

	len = lstrlen(ppi->m_szPath);

	if(FAILED(hr = pstm->Write(&len, sizeof(int), &writ)))
		return hr;

	if(FAILED(hr = pstm->Write(ppi->m_szPath, len, &writ)))
		return hr;

	len = lstrlen(ppi->m_szInternalName);

	if(FAILED(hr = pstm->Write(&len, sizeof(int), &writ)))
		return hr;

	if(FAILED(hr = pstm->Write(ppi->m_szInternalName, len, &writ)))
		return hr;

	DDSURFACEDESC2 ddsd;
	ddsd.dwSize = sizeof(ddsd);

	hr = ppi->m_pdsBuffer->Lock(NULL, &ddsd, DDLOCK_READONLY | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);

	if(FAILED(hr = pstm->Write(&ppi->m_width, sizeof(ppi->m_width), &writ)))
		return hr;

	if(FAILED(hr = pstm->Write(&ppi->m_height, sizeof(ppi->m_height), &writ)))
		return hr;

	//unsigned int i;
	char *pch = (char *)ddsd.lpSurface;

	// 32-bit picture
	LZWWriter lzwwriter(pstm, (int *)ddsd.lpSurface, ppi->m_width*4, ppi->m_height, ddsd.lPitch);

	lzwwriter.CompressBits(8+1);
	/*for (i=0;i<ddsd.dwHeight;i++)
		{
		if(FAILED(hr = pstm->Write(pch, ddsd.dwWidth*4, &writ)))
			return hr;

		pch += ddsd.lPitch;
		}*/

	/*if(FAILED(hr = pstm->Write(&ppi->m_rgbTransparent, sizeof(ppi->m_rgbTransparent), &writ)))
		return hr;

	ppi->m_pdsBuffer->Unlock(NULL);

	return S_OK;
	}*/

HRESULT PinTable::LoadImageFromStream(IStream *pstm, int version)
	{
	if (version < 100) // Tech Beta 3 and below
		{
		PinImage *ppi;
		int len;
		ULONG read = 0;
		HRESULT hr = S_OK;
		int width, height;

		ppi = new PinImage();

		if(FAILED(hr = pstm->Read(&len, sizeof(len), &read)))
			return hr;

		if(FAILED(hr = pstm->Read(ppi->m_szName, len, &read)))
			return hr;

		ppi->m_szName[len] = 0;

		if(FAILED(hr = pstm->Read(&len, sizeof(len), &read)))
			return hr;

		if(FAILED(hr = pstm->Read(ppi->m_szPath, len, &read)))
			return hr;

		ppi->m_szPath[len] = 0;

		if(FAILED(hr = pstm->Read(&len, sizeof(len), &read)))
			return hr;

		if(FAILED(hr = pstm->Read(ppi->m_szInternalName, len, &read)))
			return hr;

		ppi->m_szInternalName[len] = 0;

		if(FAILED(hr = pstm->Read(&width, sizeof(int), &read)))
			return hr;

		if(FAILED(hr = pstm->Read(&height, sizeof(int), &read)))
			return hr;

		ppi->m_width = width;
		ppi->m_height = height;
		ppi->m_pdsBuffer = g_pvp->m_pdd.CreateTextureOffscreen(width, height);

		if (ppi->m_pdsBuffer == NULL)
			{
			delete ppi;
			return E_FAIL;
			}

		DDSURFACEDESC2 ddsd;
		ddsd.dwSize = sizeof(ddsd);

		hr = ppi->m_pdsBuffer->Lock(NULL, &ddsd, DDLOCK_READONLY | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);

		//int i;
		char *pch = (char *)ddsd.lpSurface;

		// 32-bit picture
		LZWReader lzwreader(pstm, (int *)ddsd.lpSurface, width*4, height, ddsd.lPitch);

		lzwreader.Decoder();

		/*for (i=0;i<height;i++)
			{
			if(FAILED(hr = pstm->Read(pch, width*4, &read)))
				return hr;

			pch += ddsd.lPitch;
			}*/

		ppi->m_pdsBuffer->Unlock(NULL);

		if(FAILED(hr = pstm->Read(&ppi->m_rgbTransparent, sizeof(ppi->m_rgbTransparent), &read)))
			{
			ppi->m_rgbTransparent = RGB(255,255,255);
			}

		g_pvp->m_pdd.SetOpaque(ppi->m_pdsBuffer, width, height);
		g_pvp->m_pdd.CreateNextMipMapLevel(ppi->m_pdsBuffer);

		m_vimage.AddElement(ppi);
		}
	else
		{
		PinImage *ppi;
		ppi = new PinImage();

		if (ppi->LoadFromStream(pstm, version, this) == S_OK)
			{
			g_pvp->m_pdd.SetOpaque(ppi->m_pdsBuffer, ppi->m_width, ppi->m_height);
			g_pvp->m_pdd.CreateNextMipMapLevel(ppi->m_pdsBuffer);

			m_vimage.AddElement(ppi);
			}
		else
			{
			delete ppi;
			}
		}

	return S_OK;
	}

STDMETHODIMP PinTable::get_Image(BSTR *pVal)
{
	OLECHAR wz[512];

	MultiByteToWideChar(CP_ACP, 0, m_szImage, -1, wz, 32);
	*pVal = SysAllocString(wz);

	return S_OK;
}

STDMETHODIMP PinTable::put_Image(BSTR newVal)
{
	STARTUNDO

	WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_szImage, 32, NULL, NULL);

	if (!g_pplayer)
		{
		CreateGDIBackdrop();
		SetDirtyDraw();
		}

	STOPUNDO

	return S_OK;
}

STDMETHODIMP PinTable::GetPredefinedStrings(DISPID dispID, CALPOLESTR *pcaStringsOut, CADWORD *pcaCookiesOut)
	{
	return GetPredefinedStrings(dispID, pcaStringsOut, pcaCookiesOut, NULL);
	}

STDMETHODIMP PinTable::GetPredefinedValue(DISPID dispID, DWORD dwCookie, VARIANT *pVarOut)
	{
	return GetPredefinedValue(dispID, dwCookie, pVarOut, NULL);
	}

STDMETHODIMP PinTable::GetPredefinedStrings(DISPID dispID, CALPOLESTR *pcaStringsOut, CADWORD *pcaCookiesOut, IEditable *piedit)
	{
	if (pcaStringsOut == NULL || pcaCookiesOut == NULL)
		return E_POINTER;

	WCHAR *wzDst;
	int cvar;
	WCHAR **rgstr;
	DWORD *rgdw;

	switch (dispID)
		{

		case DISPID_Image:
		case DISPID_Image2:
		case DISPID_Image3:
		case DISPID_Image4:
		case DISPID_Image5:
			{
			cvar = m_vimage.Size();
			int ivar;

			rgstr = (WCHAR **) CoTaskMemAlloc((cvar+1) * sizeof(WCHAR *));
			rgdw = (DWORD *) CoTaskMemAlloc((cvar+1) * sizeof DWORD);

			wzDst = (WCHAR *) CoTaskMemAlloc(7*sizeof(WCHAR));
			// TEXT
			LocalString ls(IDS_NONE);
			MultiByteToWideChar(CP_ACP, 0, ls.m_szbuffer, -1, wzDst, 7);
			rgstr[0] = wzDst;
			rgdw[0] = -1;

			for (ivar = 0 ; ivar < cvar ; ivar++)
				{
				char *szSrc = m_vimage.ElementAt(ivar)->m_szName;
				DWORD cwch = lstrlen(szSrc)+1;
				wzDst = (WCHAR *) CoTaskMemAlloc(cwch*sizeof(WCHAR));
				if (wzDst == NULL)
					{
					ShowError("Damn");
					}

				MultiByteToWideChar(CP_ACP, 0, szSrc, -1, wzDst, cwch);

				//MsoWzCopy(szSrc,szDst);
				rgstr[ivar+1] = wzDst;
				rgdw[ivar+1] = ivar;
				}
			cvar++;
			}
			break;

        case DISPID_Sound:
			{
            cvar = m_vsound.Size();
			int ivar;

			rgstr = (WCHAR **) CoTaskMemAlloc((cvar+1) * sizeof(WCHAR *));
			rgdw = (DWORD *) CoTaskMemAlloc((cvar+1) * sizeof DWORD);

			wzDst = (WCHAR *) CoTaskMemAlloc(7*sizeof(WCHAR));
			// TEXT
			MultiByteToWideChar(CP_ACP, 0, "<None>", -1, wzDst, 7);
			rgstr[0] = wzDst;
			rgdw[0] = -1;

			for (ivar = 0 ; ivar < cvar ; ivar++)
				{
                char *szSrc = m_vsound.ElementAt(ivar)->m_szName;
				DWORD cwch = lstrlen(szSrc)+1;
				wzDst = (WCHAR *) CoTaskMemAlloc(cwch*sizeof(WCHAR));
				if (wzDst == NULL)
					{
					ShowError("Damn");
					}

				MultiByteToWideChar(CP_ACP, 0, szSrc, -1, wzDst, cwch);

				//MsoWzCopy(szSrc,szDst);
				rgstr[ivar+1] = wzDst;
				rgdw[ivar+1] = ivar;
				}
			cvar++;
			}
			break;

        case DISPID_Collection:
			{
            cvar = m_vcollection.Size();
			int ivar;

			rgstr = (WCHAR **) CoTaskMemAlloc((cvar+1) * sizeof(WCHAR *));
			rgdw = (DWORD *) CoTaskMemAlloc((cvar+1) * sizeof DWORD);

			wzDst = (WCHAR *) CoTaskMemAlloc(7*sizeof(WCHAR));
			// TEXT
			MultiByteToWideChar(CP_ACP, 0, "<None>", -1, wzDst, 7);
			rgstr[0] = wzDst;
			rgdw[0] = -1;

			for (ivar = 0 ; ivar < cvar ; ivar++)
				{
				DWORD cwch = sizeof(m_vcollection.ElementAt(ivar)->m_wzName)+sizeof(DWORD);
				wzDst = (WCHAR *) CoTaskMemAlloc(cwch);
				if (wzDst == NULL)
					{
					ShowError("DOH!");
					}
				else
					{
					memcpy (wzDst, m_vcollection.ElementAt(ivar)->m_wzName, cwch);
					}
				rgstr[ivar+1] = wzDst;
				rgdw[ivar+1] = ivar;
				}
			cvar++;
			}
			break;

		case DISPID_Surface:
			{
			cvar = 0;
			int ivar;

			BOOL fRamps = fTrue;/*(piedit->GetItemType() == eItemTrigger);*/

			for (ivar = 0;ivar < m_vedit.Size();ivar++)
				{
				if (m_vedit.ElementAt(ivar)->GetItemType() == eItemSurface || (fRamps && m_vedit.ElementAt(ivar)->GetItemType() == eItemRamp))
					{
					cvar++;
					}
				}

			rgstr = (WCHAR **) CoTaskMemAlloc((cvar+1) * sizeof(WCHAR *));
			rgdw = (DWORD *) CoTaskMemAlloc((cvar+1) * sizeof DWORD);

			cvar = 0;

			wzDst = (WCHAR *) CoTaskMemAlloc(7*sizeof(WCHAR));
			//MultiByteToWideChar(CP_ACP, 0, "None", -1, wzDst, 5);
			// TEXT
			WideStrCopy(L"<None>", wzDst);
			rgstr[cvar] = wzDst;
			rgdw[cvar] = -1;
			cvar++;

			for (ivar = 0;ivar < m_vedit.Size();ivar++)
				{
				if (m_vedit.ElementAt(ivar)->GetItemType() == eItemSurface|| (fRamps && m_vedit.ElementAt(ivar)->GetItemType() == eItemRamp))
					{
					CComBSTR bstr;
#ifdef VBA
					m_vedit.ElementAt(ivar)->GetIApcProjectItem()->get_Name(&bstr);
#else
					m_vedit.ElementAt(ivar)->GetScriptable()->get_Name(&bstr);
#endif

					DWORD cwch = lstrlenW(bstr)+1;
					//wzDst = ::SysAllocString(bstr);

					wzDst = (WCHAR *) CoTaskMemAlloc(cwch*sizeof(WCHAR));
					if (wzDst == NULL)
						{
						ShowError("Damn");
						}

					WideStrCopy(bstr, wzDst);

					//MultiByteToWideChar(CP_ACP, 0, "Hello", -1, wzDst, cwch);

					//MsoWzCopy(szSrc,szDst);
					rgstr[cvar] = wzDst;
					rgdw[cvar] = ivar;
					cvar++;
					}
				}
			}
			break;

        default:
			return E_NOTIMPL;
		}

	pcaStringsOut->cElems = cvar;
	pcaStringsOut->pElems = rgstr;

	pcaCookiesOut->cElems = cvar;
	pcaCookiesOut->pElems = rgdw;

	return S_OK;
	}

STDMETHODIMP PinTable::GetPredefinedValue(DISPID dispID, DWORD dwCookie, VARIANT *pVarOut, IEditable *piedit)
	{
	WCHAR *wzDst;

	switch (dispID)
		{
		default:
			return E_NOTIMPL;

		case DISPID_Image:
		case DISPID_Image2:
		case DISPID_Image3:
		case DISPID_Image4:
		case DISPID_Image5:
            {
			if (dwCookie == -1)
				{
				wzDst = (WCHAR *) CoTaskMemAlloc(1*sizeof(WCHAR));
				wzDst[0] = L'\0';
				}
			else
				{
				char *szSrc = m_vimage.ElementAt(dwCookie)->m_szName;
				DWORD cwch = lstrlen(szSrc)+1;
				wzDst = (WCHAR *) CoTaskMemAlloc(cwch*sizeof(WCHAR));

				MultiByteToWideChar(CP_ACP, 0, szSrc, -1, wzDst, cwch);
				}
			}
			break;
        case DISPID_Sound:
            {
			if (dwCookie == -1)
				{
				wzDst = (WCHAR *) CoTaskMemAlloc(1*sizeof(WCHAR));
				wzDst[0] = L'\0';
				}
			else
				{
                char *szSrc = m_vsound.ElementAt(dwCookie)->m_szName;
				DWORD cwch = lstrlen(szSrc)+1;
				wzDst = (WCHAR *) CoTaskMemAlloc(cwch*sizeof(WCHAR));

				MultiByteToWideChar(CP_ACP, 0, szSrc, -1, wzDst, cwch);
				}
			}
			break;
        case DISPID_Collection:
            {
			if (dwCookie == -1)
				{
				wzDst = (WCHAR *) CoTaskMemAlloc(1*sizeof(WCHAR));
				wzDst[0] = L'\0';
				}
			else
				{
   				DWORD cwch = sizeof(m_vcollection.ElementAt(dwCookie)->m_wzName)+sizeof(DWORD);
				wzDst = (WCHAR *) CoTaskMemAlloc(cwch);
				if (wzDst == NULL)
					{
					ShowError("DOH!");
					}
				else
					{
					memcpy (wzDst, m_vcollection.ElementAt(dwCookie)->m_wzName, cwch);
					}
				}
			}
			break;
		case DISPID_Surface:
			{
			if (dwCookie == -1)
				{
				wzDst = (WCHAR *) CoTaskMemAlloc(1*sizeof(WCHAR));
				wzDst[0] = L'\0';
				}
			else
				{
				CComBSTR bstr;
#ifdef VBA
				m_vedit.ElementAt(dwCookie)->GetIApcProjectItem()->get_Name(&bstr);
#else
				m_vedit.ElementAt(dwCookie)->GetScriptable()->get_Name(&bstr);
#endif

				DWORD cwch = lstrlenW(bstr)+1;
				//wzDst = ::SysAllocString(bstr);

				wzDst = (WCHAR *) CoTaskMemAlloc(cwch*sizeof(WCHAR));
				if (wzDst == NULL)
					{
					ShowError("Damn");
					}

				WideStrCopy(bstr, wzDst);
				}
			}
			break;
		}

	CComVariant var(wzDst);

	CoTaskMemFree(wzDst);

	return var.Detach(pVarOut);
	}

float PinTable::GetSurfaceHeight(char *szName, float x, float y)
	{
	int i;
	for (i=0;i<m_vedit.Size();i++)
		{
		if (m_vedit.ElementAt(i)->GetItemType() == eItemSurface || m_vedit.ElementAt(i)->GetItemType() == eItemRamp)
			{
			CComBSTR bstr;
#ifdef VBA
			m_vedit.ElementAt(i)->GetIApcProjectItem()->get_Name(&bstr);
#else
			m_vedit.ElementAt(i)->GetScriptable()->get_Name(&bstr);
#endif
			if (!WzSzStrCmp(bstr, szName))
				{
				IEditable *piedit = m_vedit.ElementAt(i);
				switch (piedit->GetItemType())
					{
					case eItemSurface:
						return ((Surface *)piedit)->m_d.m_heighttop;
						break;

					case eItemRamp:
						Ramp *pramp = (Ramp *)piedit;
						int cvertex;
						Vertex *rgv;
						Vertex v, vOut;
						int iSeg;

						v.x = x;
						v.y = y;

						Vector<RenderVertex> vvertex;
						pramp->GetRgVertex(&vvertex);

						cvertex = vvertex.Size();
						rgv = new Vertex[cvertex];
						
						int i;
						for (i=0;i<vvertex.Size();i++)
							{
							rgv[i] = *((Vertex *)vvertex.ElementAt(i));
							}

						ClosestPointOnPolygon(rgv, cvertex, &v, &vOut, &iSeg, fFalse);

						// Go through vertices (including iSeg itself) counting control points until iSeg
						float totallength = 0;
						float startlength = 0;
						float len;

						float zheight = 0;

						if (iSeg == -1)
							{
							//zheight = 0;
							goto HeightError;
							//return 0; // Object is not on ramp path
							}

						float dx,dy;
						for (i=1;i<cvertex;i++)
							{
							dx = (float)rgv[i].x - (float)rgv[i-1].x;
							dy = (float)rgv[i].y - (float)rgv[i-1].y;
							len = (float)sqrt(dx*dx + dy*dy);
							if (i <= iSeg)
								{
								startlength += len;
								}
							totallength += len;
							}

						dx = (float)vOut.x - (float)rgv[iSeg].x;
						dy = (float)vOut.y - (float)rgv[iSeg].y;
						len = (float)sqrt(dx*dx + dy*dy);
						startlength += len; // Add the distance the object is between the two closest polyline segments.  Matters mostly for straight edges.

						zheight = ((startlength/totallength) * (pramp->m_d.m_heighttop - pramp->m_d.m_heightbottom)) + pramp->m_d.m_heightbottom;

HeightError:
						for (i=0;i<vvertex.Size();i++)
							{
							delete vvertex.ElementAt(i);
							}

						delete rgv;
						return zheight;
						break;
					}
				}
			}
		}
	return 0;
	}

STDMETHODIMP PinTable::get_DisplayGrid(VARIANT_BOOL *pVal)
{
	*pVal = FTOVB(m_fGrid);

	return S_OK;
}

STDMETHODIMP PinTable::put_DisplayGrid(VARIANT_BOOL newVal)
{
	STARTUNDO

	m_fGrid = VBTOF(newVal);

	SetDirtyDraw();

	STOPUNDO

	return S_OK;
}

STDMETHODIMP PinTable::get_DisplayBackdrop(VARIANT_BOOL *pVal)
{
	*pVal = FTOVB(m_fBackdrop);

	return S_OK;
}

STDMETHODIMP PinTable::put_DisplayBackdrop(VARIANT_BOOL newVal)
{
	STARTUNDO

	m_fBackdrop = VBTOF(newVal);

	SetDirtyDraw();

	STOPUNDO

	return S_OK;
}

int CALLBACK ProgressProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	switch (uMsg)
		{
		case WM_INITDIALOG:
			{
			RECT rcProgress;
			RECT rcMain;
			GetWindowRect(g_pvp->m_hwnd, &rcMain);
			GetWindowRect(hwndDlg, &rcProgress);

			SetWindowPos(hwndDlg, NULL,
			(rcMain.right + rcMain.left)/2 - (rcProgress.right - rcProgress.left)/2,
			(rcMain.bottom + rcMain.top)/2 - (rcProgress.bottom - rcProgress.top)/2,
			0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE/* | SWP_NOMOVE*/);
			return TRUE;
			}
			break;
		}

	return FALSE;
	}

STDMETHODIMP PinTable::Nudge(float Angle, float Force)
{
	if (g_pplayer && (g_pplayer->m_nudgetime == 0))
		{
		float sn,cs;
		sn = (float)sin(ANGTORAD(Angle));
		cs = (float)cos(ANGTORAD(Angle));
		g_pplayer->m_NudgeX = -sn*Force;
		g_pplayer->m_NudgeY = cs*Force;
		g_pplayer->m_NudgeBackX = -g_pplayer->m_NudgeX;
		g_pplayer->m_NudgeBackY = -g_pplayer->m_NudgeY;
		g_pplayer->m_nudgetime = 10;
		}

	return S_OK;
}

STDMETHODIMP PinTable::get_GlassHeight(float *pVal)
{
	*pVal = m_glassheight;

	return S_OK;
}

STDMETHODIMP PinTable::put_GlassHeight(float newVal)
{
	STARTUNDO

	m_glassheight = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP PinTable::get_Width(float *pVal)
{
	*pVal = m_right;

	return S_OK;
}

STDMETHODIMP PinTable::put_Width(float newVal)
{
	STARTUNDO

	m_right = newVal;

	SetDirtyDraw();

	SetMyScrollInfo();

	STOPUNDO

	return S_OK;
}

STDMETHODIMP PinTable::get_Height(float *pVal)
{
	*pVal = m_bottom;

	return S_OK;
}

STDMETHODIMP PinTable::put_Height(float newVal)
{
	STARTUNDO

	m_bottom = newVal;

	SetDirtyDraw();

	SetMyScrollInfo();

	STOPUNDO

	return S_OK;
}

STDMETHODIMP PinTable::get_PlayfieldColor(OLE_COLOR *pVal)
{
	*pVal = m_colorplayfield;

	return S_OK;
}

STDMETHODIMP PinTable::put_PlayfieldColor(OLE_COLOR newVal)
{
	STARTUNDO

	m_colorplayfield = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP PinTable::get_BackdropColor(OLE_COLOR *pVal)
{
	*pVal = m_colorbackdrop;

	return S_OK;
}

STDMETHODIMP PinTable::put_BackdropColor(OLE_COLOR newVal)
{
	STARTUNDO

	m_colorbackdrop = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP PinTable::get_BackdropImage(BSTR *pVal)
{
	OLECHAR wz[512];

	MultiByteToWideChar(CP_ACP, 0, m_szImageBackdrop, -1, wz, 32);
	*pVal = SysAllocString(wz);

	return S_OK;
}

STDMETHODIMP PinTable::put_BackdropImage(BSTR newVal)
{
	STARTUNDO

	WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_szImageBackdrop, 32, NULL, NULL);

	if (!g_pplayer)
		{
		CreateGDIBackdrop();
		SetDirtyDraw();
		}

	STOPUNDO

	return S_OK;
}

STDMETHODIMP PinTable::get_Slope(float *pVal)
{
	*pVal = m_angletilt;

	return S_OK;
}

STDMETHODIMP PinTable::put_Slope(float newVal)
{
	STARTUNDO

	m_angletilt = newVal;

	if (g_pplayer)
		{
		g_pplayer->m_mainlevel.m_gravity.y = (float)(sin(ANGTORAD(g_pplayer->m_ptable->m_angletilt))*GRAVITY); //0.06f;
		// Old Phys
		g_pplayer->m_gravityz = (float)(sin(ANGTORAD(90 - (g_pplayer->m_ptable->m_angletilt)))*GRAVITY);
		// New Phys
		//g_pplayer->m_mainlevel.m_gravity.z = (float)g_pplayer->m_gravityz;
		}

	STOPUNDO

	return S_OK;
}

STDMETHODIMP PinTable::get_BallImage(BSTR *pVal)
{
	OLECHAR wz[512];

	MultiByteToWideChar(CP_ACP, 0, m_szBallImage, -1, wz, 32);
	*pVal = SysAllocString(wz);

	return S_OK;
}

STDMETHODIMP PinTable::put_BallImage(BSTR newVal)
{
	STARTUNDO

	WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_szBallImage, 32, NULL, NULL);

	STOPUNDO

	return S_OK;
}

STDMETHODIMP PinTable::get_YieldTime(long *pVal)
{
	if (!g_pplayer)
		{
		*pVal = NULL;
		return E_FAIL;
		}
	else
		{
		*pVal = g_pplayer->m_sleeptime;
		}

	return S_OK;
}

STDMETHODIMP PinTable::put_YieldTime(long newVal)
{
	if (!g_pplayer)
		{
		return E_FAIL;
		}
	else
		{
		g_pplayer->m_sleeptime = newVal;
		if (g_pplayer->m_sleeptime > 1000)
			{
			g_pplayer->m_sleeptime = 1000;
			}
		}

	return S_OK;
}

STDMETHODIMP PinTable::get_RenderShadows(VARIANT_BOOL *pVal)
{
	*pVal = FTOVB(m_fRenderShadows);

	return S_OK;
}

STDMETHODIMP PinTable::put_RenderShadows(VARIANT_BOOL newVal)
{
	STARTUNDO
	m_fRenderShadows = VBTOF(newVal);
	STOPUNDO

	return S_OK;
}

STDMETHODIMP PinTable::get_PhysicsType(PhysicsType *pVal)
	{
	*pVal = m_PhysicsType;
	return S_OK;
	}

STDMETHODIMP PinTable::put_PhysicsType(PhysicsType newVal)
	{
	if (g_pplayer != NULL)
		{
		return E_FAIL;
		}
		
	STARTUNDO
	m_PhysicsType = newVal;
	STOPUNDO
		
	return S_OK;
	}

// Accelerometer 

STDMETHODIMP PinTable::get_Accelerometer(VARIANT_BOOL *pVal)//
{
	if (g_pplayer) *pVal = FTOVB(g_pplayer->m_fAccelerometer);	//VB Script
	else *pVal = FTOVB(m_tblAccelerometer);						//VP Editor

	return S_OK;
}

STDMETHODIMP PinTable::put_Accelerometer(VARIANT_BOOL newVal)
{
	if (g_pplayer) g_pplayer->m_fAccelerometer = VBTOF(newVal); //VB Script
	else
	{														//VP Editor
		HRESULT hr = GetRegInt("Player", "PinballWizardEnabled", &m_tblAccelerometer);
		if (hr == S_OK) m_tblAccelerometer = (BOOL)m_tblAccelerometer != fFalse;
		else
		{
			STARTUNDO
				m_tblAccelerometer = VBTOF(newVal);
			STOPUNDO
		}
	}

	return S_OK;
}

STDMETHODIMP PinTable::get_AccelNormalMount(VARIANT_BOOL *pVal)
{
	if (g_pplayer) *pVal = FTOVB(g_pplayer->m_AccelNormalMount); //VB Script
	else *pVal = FTOVB(m_tblAccelNormalMount);					//VP Editor

	return S_OK;
}

STDMETHODIMP PinTable::put_AccelNormalMount(VARIANT_BOOL newVal)
{
	if (g_pplayer) g_pplayer->m_AccelNormalMount = VBTOF(newVal); //VB Script
	else 
	{			
		HRESULT hr = GetRegInt("Player", "PinballWizardNormalMount", &m_tblAccelNormalMount);
		if (hr == S_OK) m_tblAccelNormalMount = (BOOL)m_tblAccelNormalMount != fFalse;
		else
		{
			STARTUNDO
				m_tblAccelNormalMount = VBTOF(newVal);
			STOPUNDO
		}
	}
	return S_OK;
}

STDMETHODIMP PinTable::get_AccelerometerAngle(float *pVal)
{
	if (g_pplayer) *pVal = g_pplayer->m_AccelAngle * 180.0f/PI;			//VB Script convert to radians
	else *pVal = m_tblAccelAngle;										//VP Editor in degrees
	return S_OK;
}

STDMETHODIMP PinTable::put_AccelerometerAngle(float newVal)
{
	if (g_pplayer) g_pplayer->m_AccelAngle = newVal * PI/180.0f;	//VB Script conert to radians
	else
	{	//VP Editor in degrees
		int tmp;
		HRESULT hr = GetRegInt("Player", "PinballWizardRotation", &tmp);
		if (hr == S_OK) m_tblAccelAngle = (float)tmp;
		else 
		{		
			STARTUNDO
				m_tblAccelAngle = newVal;
			STOPUNDO
		}
	}
	return S_OK;
}

STDMETHODIMP PinTable::get_AccelerometerAmp(float *pVal)
{
	if (g_pplayer) *pVal = g_pplayer->m_AccelAmp; //VB Script
	else *pVal = m_tblAccelAmp;						//VP Editor

	return S_OK;
}

STDMETHODIMP PinTable::put_AccelerometerAmp(float newVal)
{
	if (g_pplayer) g_pplayer->m_AccelAmp = newVal; //VB Script
	else
	{						//VP Editor
		int tmp;
		HRESULT hr = GetRegInt("Player", "PinballWizardAccelGain", &tmp);
		if (hr == S_OK) m_tblAccelAmp = (float)tmp/100.0f;
		else 
		{
			STARTUNDO
				m_tblAccelAmp = newVal;
			STOPUNDO
		}
	}
	return S_OK;
}

STDMETHODIMP PinTable::get_AccelerManualAmp(float *pVal)
{
	if (g_pplayer) *pVal = g_pplayer->m_AccelMAmp; //VB Script
	else *pVal = m_tblAccelManualAmp;						//VP Editor

	return S_OK;
}

STDMETHODIMP PinTable::put_AccelerManualAmp(float newVal)
{
	if (g_pplayer) g_pplayer->m_AccelMAmp = newVal; //VB Script
	else
	{						//VP Editor		
		int tmp;
		HRESULT hr = GetRegInt("Player", "JoystickGain", &tmp);
		if (hr == S_OK) m_tblAccelManualAmp = (float)tmp/100.0f;
		else
		{
			STARTUNDO
				m_tblAccelManualAmp = newVal;
			STOPUNDO
		}
	}
	return S_OK;
}
STDMETHODIMP PinTable::get_PlungerNormalize(int *pVal)
{
	*pVal = m_plungerNormalize;

	return S_OK;
}

STDMETHODIMP PinTable::put_PlungerNormalize(int newVal )
{
	STARTUNDO

		m_plungerNormalize = newVal;
	GetRegInt("Player", "PlungerNormalize", &m_plungerNormalize);

	STOPUNDO

		return S_OK;
}

STDMETHODIMP PinTable::get_PlungerFilter(VARIANT_BOOL *pVal)
{
	*pVal = FTOVB(m_plungerFilter);
	return S_OK;
}

STDMETHODIMP PinTable::put_PlungerFilter(VARIANT_BOOL newVal )
{
	STARTUNDO

		BOOL tmp;
	tmp = VBTOF(newVal);
	GetRegInt("Player", "PlungerFilter", &tmp);
	m_plungerFilter = tmp != 0;

	STOPUNDO
		return S_OK;
}


/************************\
**						**
** Sound Sequencer Code	**
**						**
\************************/

void ScriptGlobalTable::SeqSoundInit(void)
{

}

STDMETHODIMP ScriptGlobalTable::SeqSoundPlay(/*[in]*/ long Channel, /*[in]*/ BSTR Sound, /*[in]*/ long LoopCount, /*[in]*/ float Volume, long Delay)
{
	return S_OK;
}


STDMETHODIMP ScriptGlobalTable::SeqSoundStop(/*[in]*/ long Channel, /*[in]*/ BSTR Sound, /*[in]*/ float Volume, /*[in]*/ long Delay)
{
	return S_OK;
}


STDMETHODIMP ScriptGlobalTable::SeqSoundFlush(/*[in]*/ long Channel)
{
	return S_OK;
}

STDMETHODIMP PinTable::get_BallFrontDecal(BSTR *pVal)
{
	OLECHAR wz[512];

	MultiByteToWideChar(CP_ACP, 0, m_szBallImageFront, -1, wz, 32);
	*pVal = SysAllocString(wz);

	return S_OK;
}

STDMETHODIMP PinTable::put_BallFrontDecal(BSTR newVal)
{
	STARTUNDO

	WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_szBallImageFront, 32, NULL, NULL);

	STOPUNDO

	return S_OK;
}

STDMETHODIMP PinTable::get_BallBackDecal(BSTR *pVal)
{
	OLECHAR wz[512];

	MultiByteToWideChar(CP_ACP, 0, m_szBallImageBack, -1, wz, 32);
	*pVal = SysAllocString(wz);

	return S_OK;
}

STDMETHODIMP PinTable::put_BallBackDecal(BSTR newVal)
{
	STARTUNDO

	WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_szBallImageBack, 32, NULL, NULL);

	STOPUNDO

	return S_OK;
}
