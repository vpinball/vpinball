#include "stdafx.h"



float sz2f(char *sz)
	{
	WCHAR wzT[256];
	MultiByteToWideChar(CP_ACP, 0, sz, -1, wzT, 256);

	CComVariant var = wzT;

	VariantChangeType(&var, &var, 0, VT_R8); 

	GPINFLOAT r;
	r = V_R8(&var);

	return float(r);

	}

void f2sz(float f, char *sz)
	{
	CComVariant var = f;

	VariantChangeType(&var, &var, 0, VT_BSTR); 

	WCHAR *wzT;
	wzT = V_BSTR(&var);

	WideCharToMultiByte(CP_ACP, 0, wzT, -1, sz, 256, NULL, NULL);
	}

void Matrix3::CreateSkewSymmetric(const Vertex3D * const pv3D)
	{
	m_d[0][0] = 0; m_d[0][1] = -pv3D->z; m_d[0][2] = pv3D->y;
	m_d[1][0] = pv3D->z; m_d[1][1] = 0; m_d[1][2] = -pv3D->x;
	m_d[2][0] = -pv3D->y; m_d[2][1] = pv3D->x; m_d[2][2] = 0;
	}

void Matrix3::CreateSkewSymmetric(const Vertex3Ds * const pv3D)
	{
	m_d[0][0] = 0; m_d[0][1] = -pv3D->z; m_d[0][2] = pv3D->y;
	m_d[1][0] = pv3D->z; m_d[1][1] = 0; m_d[1][2] = -pv3D->x;
	m_d[2][0] = -pv3D->y; m_d[2][1] = pv3D->x; m_d[2][2] = 0;
	}

void Matrix3::MultiplyScalar(const float scalar)
	{
	for (int i=0;i<3;i++)
		{
		for (int l=0;l<3;l++)
			{
			m_d[i][l] *= scalar;
			}
		}
	}

void Matrix3::MultiplyVector(const Vertex3D * const pv3D, Vertex3D * const pv3DOut)
	{
    float ans[3];

    for(int i = 0;i < 3;i++)
		{
        float value = 0.0f;
      
        for(int l = 0;l < 3;l++)
			{
            value += m_d[i][l] *
                    pv3D->m_d[l];
			}

        ans[i] = value;
		}

	// Copy the final values over later.  This makes it so pv3D and pv3DOut can
	// point to the same vertex.
    for(int i = 0;i < 3;i++)
		{
        pv3DOut->m_d[i] = ans[i];
		}
	}

void Matrix3::MultiplyVector(const Vertex3Ds * const pv3D, Vertex3Ds * const pv3DOut)
	{
    float ans[3];

    for(int i = 0;i < 3;i++)
		{
        float value = 0.0f;
      
        for(int l = 0;l < 3;l++)
			{
            value += m_d[i][l] *
                    pv3D->m_d[l];
			}

        ans[i] = value;
		}

	// Copy the final values over later.  This makes it so pv3D and pv3DOut can
	// point to the same vertex.
    for(int i = 0;i < 3;i++)
		{
        pv3DOut->m_d[i] = ans[i];
		}
	}

void Matrix3::MultiplyMatrix(const Matrix3 * const pmat1, const Matrix3 * const pmat2)
	{
	Matrix3 matans;

    for(int i=0;i<3;i++)
    {
        for(int l=0;l<3;l++)
        {
            float value = 0.0f;
          
            for(int m=0;m<3;m++)
            {
                value += pmat1->m_d[i][m] *
                        pmat2->m_d[m][l];
            }

            matans.m_d[i][l] = value;
        }
    }

	// Copy the final values over later.  This makes it so pmat1 and pmat2 can
	// point to the same matrix.

    for(int i=0;i<3;i++)
		{
		for (int l=0;l<3;l++)
			{
			m_d[i][l] = matans.m_d[i][l];
			}
		}
	}

void Matrix3::AddMatrix(const Matrix3 * const pmat1, const Matrix3 * const pmat2)
	{
	for (int i=0;i<3;i++)
		{
		for (int l=0;l<3;l++)
			{
			m_d[i][l] = pmat1->m_d[i][l] + pmat2->m_d[i][l];
			}
		}
	}

void Matrix3::OrthoNormalize()
	{
	Vertex3Ds vX;
	Vertex3Ds vY;
	Vertex3Ds vZ;

	vX.Set(m_d[0][0], m_d[1][0], m_d[2][0]);
	vY.Set(m_d[0][1], m_d[1][1], m_d[2][1]);

	vX.Normalize();
	CrossProduct(&vX, &vY, &vZ);
	vZ.Normalize();
	CrossProduct(&vZ, &vX, &vY);
	vY.Normalize();

	m_d[0][0] = vX.m_d[0]; m_d[0][1] = vY.m_d[0]; m_d[0][2] = vZ.m_d[0];
	m_d[1][0] = vX.m_d[1]; m_d[1][1] = vY.m_d[1]; m_d[1][2] = vZ.m_d[1];
	m_d[2][0] = vX.m_d[2]; m_d[2][1] = vY.m_d[2]; m_d[2][2] = vZ.m_d[2];
	}

void Matrix3::Transpose(Matrix3 * const pmatOut) const
	{
	for(int i = 0;i < 3;i++)
		{
        pmatOut->m_d[0][i] = m_d[i][0];
        pmatOut->m_d[1][i] = m_d[i][1];
        pmatOut->m_d[2][i] = m_d[i][2];
		}
	}

void WideStrCopy(WCHAR *wzin, WCHAR *wzout)
	{
	while (*wzin) {*wzout++ = *wzin++;}
	*wzout = 0;
	}

void WideStrCat(WCHAR *wzin, WCHAR *wzout)
	{
	wzout += lstrlenW(wzout);
	while (*wzin) {*wzout++ = *wzin++;}
	*wzout = 0;
	}

int WideStrCmp(WCHAR *wz1, WCHAR *wz2)
	{
	while (*wz1 != L'\0')
		{
		if (*wz1 != *wz2)
			{
			if (*wz1 > *wz2)
				{
				return 1; // If *wz2 == 0, then wz1 will return as higher, which is correct
				}
			else if (*wz1 < *wz2)
				{
				return -1;
				}
			}
		*wz1++;
		*wz2++;
		}
	if (*wz2 != L'\0')
		{
		return -1; // wz2 is longer - and therefore higher
		}
	return 0;
	}

int WzSzStrCmp(WCHAR *wz1, char *sz2)
	{
	while (*wz1 != L'\0')
		{
		if (*wz1++ != *sz2++)
			{
			return 1;
			}
		}
	if (*sz2 != L'\0')
		{
		return 1;
		}
	return 0;
	}

LocalString::LocalString(int resid)
	{
	int cchar = LoadString(g_hinstres, resid, m_szbuffer, 256);
	if (cchar == 0 && g_hinstres != g_hinst)
		{
		// string not in resource dll, check main dll
		cchar = LoadString(g_hinst, resid, m_szbuffer, 256);
		}
	}

WCHAR *MakeWide(char *sz)
	{
	int len = lstrlen(sz);
	WCHAR *wzT = new WCHAR[len+1];
	MultiByteToWideChar(CP_ACP, 0, sz, -1, wzT, len+1);

	return wzT;
	}

char *MakeChar(WCHAR *wz)
	{
	int len = lstrlenW(wz);
	char *szT = new char[len+1];
	WideCharToMultiByte(CP_ACP, 0, wz, -1, szT, len+1, NULL, NULL);

	return szT;
	}

#include "Intshcut.h"
HRESULT OpenURL(char *szURL)
	{
	IUniformResourceLocator* pURL;

	HRESULT hres = CoCreateInstance(CLSID_InternetShortcut, NULL, CLSCTX_INPROC_SERVER, IID_IUniformResourceLocator, (void**) &pURL);
	if (!SUCCEEDED(hres))
		{
		return hres;
		}

	hres = pURL->SetURL(szURL, IURL_SETURL_FL_GUESS_PROTOCOL);

	if (!SUCCEEDED(hres))
		{
		pURL->Release();
		return hres;
		}

	//Open the URL by calling InvokeCommand
	URLINVOKECOMMANDINFO ivci;
	ivci.dwcbSize = sizeof(URLINVOKECOMMANDINFO);
	ivci.dwFlags = IURL_INVOKECOMMAND_FL_ALLOW_UI;
	ivci.hwndParent = g_pvp->m_hwnd;
	ivci.pcszVerb = "open";
	hres = pURL->InvokeCommand(&ivci);
	pURL->Release();
	return (hres);
	}

#ifdef FONTMANAGER

#include	<fcntl.h>
#include	<io.h>
#include	<errno.h>
#include	<dos.h>
#include	"fscdefs.h"
#include	"sfnt.h"

void DumpNameTable (char *pszFile, char *pszName)
{
  unsigned            i;
  char		      namebuf[255];
  int                 fp;
  unsigned short      numNames;
  long		      curseek;
  unsigned            cTables;
  sfnt_OffsetTable    OffsetTable;
  sfnt_DirectoryEntry Table;
  sfnt_NamingTable    NamingTable;
  sfnt_NameRecord     NameRecord;

  if ((fp = open (pszFile, O_RDONLY | O_BINARY)) == -1)
    return;

  /* First off, read the initial directory header on the TTF.  We're only
   * interested in the "numOffsets" variable to tell us how many tables
   * are present in this file.  
   *
   * Remember to always convert from Motorola format (Big Endian to 
   * Little Endian).
   */
  read (fp, &OffsetTable, sizeof (OffsetTable) - sizeof (sfnt_DirectoryEntry));
  cTables = (int) SWAPW (OffsetTable.numOffsets);

  for ( i = 0; i < cTables && i < 40; i++)
  {
    if ((read (fp, &Table, sizeof (Table))) != sizeof(Table)) 
	{
		//printf("Read failed on table #%d\n", i);
		//exit(-1);
		return;
	}
	int foo = tag_NamingTable;
    if (Table.tag == tag_NamingTable)	/* defined in sfnt_en.h */
    {
	/* Now that we've found the entry for the name table, seek to that
	 * position in the file and read in the initial header for this
	 * particular table.  See "True Type Font Files" for information
	 * on this record layout.
	 */
	lseek (fp, SWAPL (Table.offset), SEEK_SET);
	read (fp, &NamingTable, sizeof (NamingTable));
	numNames = SWAPW(NamingTable.count);
	while (numNames--) {
		read (fp, &NameRecord, sizeof (NameRecord));
		curseek = tell(fp);

/* Undefine this next section if you'd like a little bit more info 
 * during the parsing of this particular name table.
 */
#ifdef SPAM 
printf("(%ld) platform=%u, specific=%u, lang=%x, name=%u (%u, %u)\n", curseek,
		SWAPW(NameRecord.platformID),
		SWAPW(NameRecord.specificID),
		SWAPW(NameRecord.languageID),
		SWAPW(NameRecord.nameID),
		SWAPW(NameRecord.length),
		SWAPW(NameRecord.offset));
#endif
		if (SWAPW(NameRecord.platformID) == 1 &&
				SWAPW(NameRecord.nameID) == 4) {
			lseek (fp, SWAPW (NameRecord.offset) + 
				SWAPW(NamingTable.stringOffset) + 
				SWAPL(Table.offset), SEEK_SET);
			read (fp, &namebuf, SWAPW(NameRecord.length));
			namebuf[SWAPW(NameRecord.length)] = '\0';
			//printf("%s: FullFontName = %s\n", pszFile, namebuf);
			lstrcpy(pszName, namebuf);
			lseek (fp, curseek, SEEK_SET);
		}
	}
	goto cleanup;
    }

  }
  printf("%s: ** No name table found **\n", pszFile);

cleanup:
  close (fp);  
}
#endif FONTMANAGER
