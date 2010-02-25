#pragma once

#ifndef __DEF_H__
#define __DEF_H__

#ifdef VBA
using namespace MSAPC;
#endif

//#define OLDLOAD 1

#define fTrue 1
#define fFalse 0

#define BOOL int

//#define PINFLOAT double //rlc change back to single percision
#define PINFLOAT float

#define MAXSTRING 1024
#define MAXTOKEN 32*4

#define SZTHISFILE
#define ASSERT(fTest, err)
#define FAIL(err)

#define CCO(x) CComObject<x>

#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

#define hrNotImplemented      ResultFromScode(E_NOTIMPL)

#ifdef MYDEBUG
#define Assert(x) _ASSERTE(x)
#else
#define Assert(x)
#endif

enum SaveDirtyState
	{
	eSaveClean,
	eSaveAutosaved,
	eSaveDirty
	};

class Vertex
	{
public:
	float x;
	float y;

	void Normalize()
		{
		float oneoverlength = (float)(1/(sqrt(x*x + y*y)));
		x *= oneoverlength;
		y *= oneoverlength;
		}
	};

class VertexD
	{
public:
	PINFLOAT x;
	PINFLOAT y;

	void Normalize()
		{
		PINFLOAT oneoverlength = (PINFLOAT)(1/(sqrt(x*x + y*y)));
		x *= oneoverlength;
		y *= oneoverlength;
		}
	};

#define MY_D3DFVF_VERTEX (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2)

#define MY_D3DTRANSFORMED_VERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX2)

class Vertex3D
	{
public:
	// Position
	union
		{
			struct {
			D3DVALUE x; 
			D3DVALUE y; 
			D3DVALUE z;
			};
			D3DVALUE m_d[3];
		};

	// Normals
	union
		{
		D3DVALUE nx;
		D3DVALUE rhw;
		};
	
	union
		{
		D3DVALUE ny;
		D3DCOLOR color;
		};
	
	union
		{
		D3DVALUE nz;
		D3DCOLOR specular;
		};
	
	// Texture coordinates
	D3DVALUE tu;
	D3DVALUE tv;

	D3DVALUE tu2;
	D3DVALUE tv2;

	inline void Set(float a, float b, float c) {x=a; y=b; z=c;}
	inline void SetDouble(double a, double b, double c) {x=(float)a; y=(float)b; z=(float)c;}
	void Normalize();
	void NormalizeNormal();
	float Dot(Vertex3D *pv);
	float DistanceSquared(Vertex3D *pv);
	float LengthSquared();
	void MultiplyScalar(float scalar);
	void Add(Vertex3D *pv);
	};

class Matrix3
	{
public:
	void CreateSkewSymmetric(Vertex3D *pv3D);
	void MultiplyScalar(float scalar);
	void MultiplyVector(Vertex3D *pv3D, Vertex3D *pv3DOut);
	void MultiplyMatrix(Matrix3 *pmat1, Matrix3 *pmat2);
	void AddMatrix(Matrix3 *pmat1, Matrix3 *pmat2);
	void OrthoNormalize();
	void Transpose(Matrix3 *pmatOut);

	float m_d[3][3];
	};

class FRect
	{
public:
	float left, top, right, bottom;
	};

class FRect3D
	{
public:
	float left, top, right, bottom, zlow, zhigh;
	};

class LocalString
	{
public:
	LocalString(int resid);

	char m_szbuffer[256];
	};

#define PI 3.141592654f

#define ANGTORAD(x) ((x) *PI/180.0f)
#define RADTOANG(x) ((x) *180.0f/PI)

#define VBTOF(x) ((x) ? fTrue : fFalse)
#define FTOVB(x) ((x) ? -1 : 0)

#include "HELPERS.H"

float sz2f(char *sz);
void f2sz(float f, char *sz);

void WideStrCopy(WCHAR *wzin, WCHAR *wzout);
int WideStrCmp(WCHAR *wz1, WCHAR *wz2);
int WzSzStrCmp(WCHAR *wz1, char *sz2);
void WideStrCat(WCHAR *wzin, WCHAR *wzout);

//void DumpNameTable (char *pszFile, char *pszName);

HRESULT OpenURL(char *szURL);

WCHAR *MakeWide(char *sz);
char *MakeChar(WCHAR *wz);

#endif/* !__DEF_H__ */
