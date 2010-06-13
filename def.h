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

class Vertex2D
	{
public:
	float x;
	float y;

	inline void Normalize()
		{
		const float oneoverlength = 1.0f/sqrtf(x*x + y*y);
		x *= oneoverlength;
		y *= oneoverlength;
		}
	};

class Vertex3Ds
	{
public:
	union
		{
			struct {
			D3DVALUE x; 
			D3DVALUE y; 
			D3DVALUE z;
			};
			D3DVALUE m_d[3];
		};

	inline void Set(const float a, const float b, const float c) {x=a; y=b; z=c;}
	inline void SetDouble(const double a, const double b, const double c) {x=(float)a; y=(float)b; z=(float)c;}
	inline void Normalize()
		{
		const float oneoverlength = 1.0f/sqrtf(x*x + y*y + z*z);
		x *= oneoverlength;
		y *= oneoverlength;
		z *= oneoverlength;
		}
	inline float Dot(const Vertex3Ds * const pv) const
		{
		return x*pv->x + y*pv->y + z*pv->z;
		}
	inline float LengthSquared() const
		{
		return x*x + y*y + z*z;
		}
	inline void MultiplyScalar(const float scalar)
		{
		x *= scalar;
		y *= scalar;
		z *= scalar;
		}
	inline void Add(const Vertex3Ds * const pv)
		{
		x += pv->x;
		y += pv->y;
		z += pv->z;
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

	inline void Set(const float a, const float b, const float c) {x=a; y=b; z=c;}
	inline void SetDouble(const double a, const double b, const double c) {x=(float)a; y=(float)b; z=(float)c;}
	inline void Normalize()
	{
		const float oneoverlength = 1.0f/sqrtf(x*x + y*y + z*z);
		x *= oneoverlength;
		y *= oneoverlength;
		z *= oneoverlength;
	}
	inline void NormalizeNormal()
	{
		const float oneoverlength = 1.0f/sqrtf(nx*nx + ny*ny + nz*nz);
		nx *= oneoverlength;
		ny *= oneoverlength;
		nz *= oneoverlength;
	}
	inline float Dot(const Vertex3D * const pv) const
	{
		return x*pv->x + y*pv->y + z*pv->z;
	}
	inline float DistanceSquared(const Vertex3D * const pv) const
	{
		const float dx = (pv->x - x);
		const float dy = (pv->y - y);
		const float dz = (pv->z - z);
		return dx*dx + dy*dy + dz*dz;
	}
	inline float LengthSquared() const
	{
		return x*x + y*y + z*z;
	}
	inline void MultiplyScalar(const float scalar)
	{
		x *= scalar;
		y *= scalar;
		z *= scalar;
	}
	inline void Add(const Vertex3D * const pv)
	{
		x += pv->x;
		y += pv->y;
		z += pv->z;
	}
	};

class Matrix3
	{
public:
	void CreateSkewSymmetric(const Vertex3D * const pv3D);
	void CreateSkewSymmetric(const Vertex3Ds * const pv3D);
	void MultiplyScalar(const float scalar);
	void MultiplyVector(const Vertex3D * const pv3D, Vertex3D * const pv3DOut);
	void MultiplyVector(const Vertex3Ds * const pv3D, Vertex3Ds * const pv3DOut);
	void MultiplyMatrix(const Matrix3 * const pmat1, const Matrix3 * const pmat2);
	void AddMatrix(const Matrix3 * const pmat1, const Matrix3 * const pmat2);
	void OrthoNormalize();
	void Transpose(Matrix3 * const pmatOut) const;

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

#define M_PI 3.1415926535897932384626433832795

#define ANGTORAD(x) ((x) *(float)(M_PI/180.0))
#define RADTOANG(x) ((x) *(float)(180.0/M_PI))

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
