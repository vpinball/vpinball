#pragma once

#ifndef __DEF_H__
#define __DEF_H__

#ifdef VBA
using namespace MSAPC;
#endif

#include <xmmintrin.h>

//#define OLDLOAD 1

#define fTrue 1
#define fFalse 0

#define BOOL int

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

	inline Vertex2D() {}
	inline Vertex2D(const float _x, const float _y) : x(_x), y(_y) {}

	inline void Normalize()
		{
		const float oneoverlength = 1.0f/sqrtf(x*x + y*y);
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
	D3DVALUE x; 
	D3DVALUE y; 
	D3DVALUE z;

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
	inline float Dot(const Vertex3D &pv) const
	{
		return x*pv.x + y*pv.y + z*pv.z;
	}
	inline float DistanceSquared(const Vertex3D &pv) const
	{
		const float dx = pv.x - x;
		const float dy = pv.y - y;
		const float dz = pv.z - z;
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
	inline void Add(const Vertex3D &pv)
	{
		x += pv.x;
		y += pv.y;
		z += pv.z;
	}
	};

__declspec(align(16))
class Vertex3Ds
	{
public:
	union {
	  struct {
		float x; 
		float y; 
		float z;
		// dummy value to help with 16-byte alignment of Vertex3Ds objects
		float _dummy;
	  };
	  __m128 xyz;
	};

	inline Vertex3Ds() {}
	inline Vertex3Ds(const float _x, const float _y, const float _z) : x(_x), y(_y), z(_z) {}

	inline void Set(const float a, const float b, const float c) {x=a; y=b; z=c;}
	inline void SetDouble(const double a, const double b, const double c) {x=(float)a; y=(float)b; z=(float)c;}
	inline void Normalize()
		{
		const float oneoverlength = 1.0f/sqrtf(x*x + y*y + z*z);
		x *= oneoverlength;
		y *= oneoverlength;
		z *= oneoverlength;
		}
	inline void Normalize(const float scalar)
		{
		const float oneoverlength = scalar/sqrtf(x*x + y*y + z*z);
		x *= oneoverlength;
		y *= oneoverlength;
		z *= oneoverlength;
		}
	inline float Dot(const Vertex3Ds &pv) const
		{
		return x*pv.x + y*pv.y + z*pv.z;
		}
	inline float Dot(const Vertex3D &pv) const
		{
		return x*pv.x + y*pv.y + z*pv.z;
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
	inline void Add(const Vertex3Ds &pv)
		{
		x += pv.x;
		y += pv.y;
		z += pv.z;
		}
	};

inline Vertex2D Calc2DNormal(const Vertex2D &pv1, const Vertex2D &pv2)
	{
	const Vertex2D vT(pv1.x - pv2.x, pv1.y - pv2.y);
	// Set up line normal
	const float inv_length = 1.0f/sqrtf(vT.x * vT.x + vT.y * vT.y);
	return Vertex2D(vT.y * inv_length, -vT.x * inv_length);
	}

inline void ClosestPointOnPolygon(const Vertex2D * const rgv, const int count, const Vertex2D &pvin, Vertex2D * const pvout, int * const piseg, const BOOL fClosed)
	{
	float mindist = FLT_MAX;
	int seg = -1;
	*piseg = -1; // in case we are not next to the line

	int cloop = count;
	if (!fClosed)
		{
		--cloop; // Don't check segment running from the end point to the beginning point
		}

	// Go through line segment, calculate distance from point to the line
	// then pick the shortest distance
	for (int i=0; i<cloop; ++i)
		{
		const int p2 = (i < count-1) ? (i+1) : 0;

		const float A = rgv[i].y - rgv[p2].y;
		const float B = rgv[p2].x - rgv[i].x;
		const float C = -(A*rgv[i].x + B*rgv[i].y);

		const float dist = fabsf(A*pvin.x + B*pvin.y + C) / sqrtf(A*A + B*B);

		if (dist < mindist)
			{
			// Assuming we got a segment that we are closet to, calculate the intersection
			// of the line with the perpenticular line projected from the point,
			// to find the closest point on the line
			const float D = -B;
			const float F = -(D*pvin.x + A*pvin.y);
			
			const float det = A*A - B*D;
			const float inv_det = (det != 0.0f) ? 1.0f/det : 0.0f;
			const float intersectx = (B*F-A*C)*inv_det;
			const float intersecty = (C*D-A*F)*inv_det;

			// If the intersect point lies on the polygon segment
			// (not out in space), then make this the closest known point
			if (intersectx >= (min(rgv[i].x, rgv[p2].x) - 0.1f) &&
				intersectx <= (max(rgv[i].x, rgv[p2].x) + 0.1f) &&
				intersecty >= (min(rgv[i].y, rgv[p2].y) - 0.1f) &&
				intersecty <= (max(rgv[i].y, rgv[p2].y) + 0.1f))
				{
				mindist = dist;
				seg = i;
				pvout->x = intersectx;
				pvout->y = intersecty;
				*piseg = seg;
				}
			}
		}
	}

inline void RotateAround(const Vertex3Ds &pvAxis, Vertex3D * const pvPoint, const int count, const float angle)
	{
	const float rsin = sinf(angle);
	const float rcos = cosf(angle);

	// Matrix for rotating around an arbitrary vector

	float matrix[3][3];
	matrix[0][0] = pvAxis.x*pvAxis.x + rcos*(1.0f-pvAxis.x*pvAxis.x);
	matrix[1][0] = pvAxis.x*pvAxis.y*(1.0f-rcos) - pvAxis.z*rsin;
	matrix[2][0] = pvAxis.z*pvAxis.x*(1.0f-rcos) + pvAxis.y*rsin;

	matrix[0][1] = pvAxis.x*pvAxis.y*(1.0f-rcos) + pvAxis.z*rsin;
	matrix[1][1] = pvAxis.y*pvAxis.y + rcos*(1.0f-pvAxis.y*pvAxis.y);
	matrix[2][1] = pvAxis.y*pvAxis.z*(1.0f-rcos) - pvAxis.x*rsin;

	matrix[0][2] = pvAxis.z*pvAxis.x*(1.0f-rcos) - pvAxis.y*rsin;
	matrix[1][2] = pvAxis.y*pvAxis.z*(1.0f-rcos) + pvAxis.x*rsin;
	matrix[2][2] = pvAxis.z*pvAxis.z + rcos*(1.0f-pvAxis.z*pvAxis.z);

	for (int i=0; i<count; ++i)
		{
		const float result[3] = {
			matrix[0][0]*pvPoint[i].x + matrix[0][1]*pvPoint[i].y + matrix[0][2]*pvPoint[i].z,
			matrix[1][0]*pvPoint[i].x + matrix[1][1]*pvPoint[i].y + matrix[1][2]*pvPoint[i].z,
			matrix[2][0]*pvPoint[i].x + matrix[2][1]*pvPoint[i].y + matrix[2][2]*pvPoint[i].z};

		pvPoint[i].x = result[0];
		pvPoint[i].y = result[1];
		pvPoint[i].z = result[2];

		const float resultn[3] = {
			matrix[0][0]*pvPoint[i].nx + matrix[0][1]*pvPoint[i].ny + matrix[0][2]*pvPoint[i].nz,
			matrix[1][0]*pvPoint[i].nx + matrix[1][1]*pvPoint[i].ny + matrix[1][2]*pvPoint[i].nz,
			matrix[2][0]*pvPoint[i].nx + matrix[2][1]*pvPoint[i].ny + matrix[2][2]*pvPoint[i].nz};

		pvPoint[i].nx = resultn[0];
		pvPoint[i].ny = resultn[1];
		pvPoint[i].nz = resultn[2];
		}
	}

inline void RotateAround(const Vertex3Ds &pvAxis, Vertex3Ds * const pvPoint, const int count, const float angle)
	{
	const float rsin = sinf(angle);
	const float rcos = cosf(angle);

	// Matrix for rotating around an arbitrary vector

	float matrix[3][3];
	matrix[0][0] = pvAxis.x*pvAxis.x + rcos*(1.0f-pvAxis.x*pvAxis.x);
	matrix[1][0] = pvAxis.x*pvAxis.y*(1.0f-rcos) - pvAxis.z*rsin;
	matrix[2][0] = pvAxis.z*pvAxis.x*(1.0f-rcos) + pvAxis.y*rsin;

	matrix[0][1] = pvAxis.x*pvAxis.y*(1.0f-rcos) + pvAxis.z*rsin;
	matrix[1][1] = pvAxis.y*pvAxis.y + rcos*(1.0f-pvAxis.y*pvAxis.y);
	matrix[2][1] = pvAxis.y*pvAxis.z*(1.0f-rcos) - pvAxis.x*rsin;

	matrix[0][2] = pvAxis.z*pvAxis.x*(1.0f-rcos) - pvAxis.y*rsin;
	matrix[1][2] = pvAxis.y*pvAxis.z*(1.0f-rcos) + pvAxis.x*rsin;
	matrix[2][2] = pvAxis.z*pvAxis.z + rcos*(1.0f-pvAxis.z*pvAxis.z);

	for (int i=0; i<count; ++i)
		{
		const float result[3] = {
			matrix[0][0]*pvPoint[i].x + matrix[0][1]*pvPoint[i].y + matrix[0][2]*pvPoint[i].z,
			matrix[1][0]*pvPoint[i].x + matrix[1][1]*pvPoint[i].y + matrix[1][2]*pvPoint[i].z,
			matrix[2][0]*pvPoint[i].x + matrix[2][1]*pvPoint[i].y + matrix[2][2]*pvPoint[i].z};

		pvPoint[i].x = result[0];
		pvPoint[i].y = result[1];
		pvPoint[i].z = result[2];
		}
	}

inline Vertex3Ds RotateAround(const Vertex3Ds &pvAxis, const Vertex2D &pvPoint, const float angle)
	{
	const float rsin = sinf(angle);
	const float rcos = cosf(angle);

	// Matrix for rotating around an arbitrary vector

	float matrix[3][2];
	matrix[0][0] = pvAxis.x*pvAxis.x + rcos*(1.0f-pvAxis.x*pvAxis.x);
	matrix[1][0] = pvAxis.x*pvAxis.y*(1.0f-rcos) - pvAxis.z*rsin;
	matrix[2][0] = pvAxis.z*pvAxis.x*(1.0f-rcos) + pvAxis.y*rsin;

	matrix[0][1] = pvAxis.x*pvAxis.y*(1.0f-rcos) + pvAxis.z*rsin;
	matrix[1][1] = pvAxis.y*pvAxis.y + rcos*(1.0f-pvAxis.y*pvAxis.y);
	matrix[2][1] = pvAxis.y*pvAxis.z*(1.0f-rcos) - pvAxis.x*rsin;

	return Vertex3Ds(matrix[0][0]*pvPoint.x + matrix[0][1]*pvPoint.y,
					 matrix[1][0]*pvPoint.x + matrix[1][1]*pvPoint.y,
					 matrix[2][0]*pvPoint.x + matrix[2][1]*pvPoint.y);
	}

inline Vertex3Ds CrossProduct(const Vertex3Ds &pv1, const Vertex3Ds &pv2)
	{
	return Vertex3Ds(pv1.y * pv2.z - pv1.z * pv2.y,
		             pv1.z * pv2.x - pv1.x * pv2.z,
					 pv1.x * pv2.y - pv1.y * pv2.x);
	}

class Matrix3
	{
public:

	inline void scaleX(const float factor)
	{
		m_d[0][0] *= factor;
	}
	inline void scaleY(const float factor)
	{
		m_d[1][1] *= factor;
	}
	inline void scaleZ(const float factor)
	{
		m_d[2][2] *= factor;
	}

	inline void CreateSkewSymmetric(const Vertex3Ds &pv3D)
	{
	m_d[0][0] = 0; m_d[0][1] = -pv3D.z; m_d[0][2] = pv3D.y;
	m_d[1][0] = pv3D.z; m_d[1][1] = 0; m_d[1][2] = -pv3D.x;
	m_d[2][0] = -pv3D.y; m_d[2][1] = pv3D.x; m_d[2][2] = 0;
	}

	inline void MultiplyScalar(const float scalar)
	{
	for (int i=0; i<3; ++i)
		for (int l=0; l<3; ++l)
			m_d[i][l] *= scalar;
	}

	inline Vertex3Ds MultiplyVector(const Vertex3D &pv3D) const
	{
	return Vertex3Ds(m_d[0][0] * pv3D.x
				   + m_d[0][1] * pv3D.y
			       + m_d[0][2] * pv3D.z,
					 m_d[1][0] * pv3D.x
				   + m_d[1][1] * pv3D.y
				   + m_d[1][2] * pv3D.z,
					 m_d[2][0] * pv3D.x
				   + m_d[2][1] * pv3D.y
				   + m_d[2][2] * pv3D.z);
	}

	inline Vertex3Ds MultiplyVector(const Vertex3Ds &pv3D) const
	{
	return Vertex3Ds(m_d[0][0] * pv3D.x
				   + m_d[0][1] * pv3D.y
			       + m_d[0][2] * pv3D.z,
					 m_d[1][0] * pv3D.x
				   + m_d[1][1] * pv3D.y
				   + m_d[1][2] * pv3D.z,
					 m_d[2][0] * pv3D.x
				   + m_d[2][1] * pv3D.y
				   + m_d[2][2] * pv3D.z);
	}

	inline void MultiplyMatrix(const Matrix3 * const pmat1, const Matrix3 * const pmat2)
	{
	Matrix3 matans;
    for(int i=0; i<3; ++i)
        for(int l=0; l<3; ++l)
            matans.m_d[i][l] = pmat1->m_d[i][0] * pmat2->m_d[0][l] +
					           pmat1->m_d[i][1] * pmat2->m_d[1][l] +
						       pmat1->m_d[i][2] * pmat2->m_d[2][l];

	// Copy the final values over later.  This makes it so pmat1 and pmat2 can
	// point to the same matrix.
    for(int i=0; i<3; ++i)
		for (int l=0; l<3; ++l)
			m_d[i][l] = matans.m_d[i][l];
	}

	inline void AddMatrix(const Matrix3 * const pmat1, const Matrix3 * const pmat2)
	{
	for (int i=0; i<3; ++i)
		for (int l=0; l<3; ++l)
			m_d[i][l] = pmat1->m_d[i][l] + pmat2->m_d[i][l];
	}

	inline void OrthoNormalize()
	{
	Vertex3Ds vX(m_d[0][0], m_d[1][0], m_d[2][0]);
	Vertex3Ds vY(m_d[0][1], m_d[1][1], m_d[2][1]);
	vX.Normalize();
	Vertex3Ds vZ = CrossProduct(vX, vY);
	vZ.Normalize();
	vY = CrossProduct(vZ, vX);
	vY.Normalize();

	m_d[0][0] = vX.x; m_d[0][1] = vY.x; m_d[0][2] = vZ.x;
	m_d[1][0] = vX.y; m_d[1][1] = vY.y; m_d[1][2] = vZ.y;
	m_d[2][0] = vX.z; m_d[2][1] = vY.z; m_d[2][2] = vZ.z;
	}

	inline void Transpose(Matrix3 * const pmatOut) const
	{
	for(int i=0; i<3; ++i)
		{
        pmatOut->m_d[0][i] = m_d[i][0];
        pmatOut->m_d[1][i] = m_d[i][1];
        pmatOut->m_d[2][i] = m_d[i][2];
		}
	}

	inline void Identity(const float value = 1.0f)
	{
		m_d[0][0] = m_d[1][1] = m_d[2][2] = value;
		m_d[0][1] = m_d[0][2] = 
		m_d[1][0] = m_d[1][2] = 
		m_d[2][0] = m_d[2][1] = 0.0f;
	}
private:
	float m_d[3][3];
	};

class FRect
	{
public:
	union {
	struct {
	float left, top, right, bottom;
	};
	struct {
	__m128 ltrb;
	};
	};
	};

class FRect3D
	{
public:
	union {
	struct {
	float left, top, right, bottom, zlow, zhigh;
	};
	struct {
	__m128 ltrb,zlzh;
	};
	};
	};

class LocalString
	{
public:
	LocalString(const int resid);

	char m_szbuffer[256];
	};

#define M_PI 3.1415926535897932384626433832795

#define ANGTORAD(x) ((x) *(float)(M_PI/180.0))
#define RADTOANG(x) ((x) *(float)(180.0/M_PI))

#define VBTOF(x) ((x) ? fTrue : fFalse)
#define FTOVB(x) ((x) ? -1 : 0)

const WORD rgi0123[4] = {0,1,2,3};

#include "HELPERS.H"

inline __m128 rcpps(const __m128 &T) //Newton Raphson
{
	const __m128 TRCP = _mm_rcp_ps(T);
	return _mm_sub_ps(_mm_add_ps(TRCP,TRCP),_mm_mul_ps(_mm_mul_ps(TRCP,T),TRCP));
}

inline __m128 rsqrtps(const __m128 &T) //Newton Raphson
{
	const __m128 TRSQRT = _mm_rsqrt_ps(T);
	return _mm_mul_ps(_mm_mul_ps(_mm_set1_ps(0.5f),TRSQRT), _mm_sub_ps(_mm_set1_ps(3.0f),_mm_mul_ps(_mm_mul_ps(TRSQRT,T),TRSQRT)));
}

inline __m128 rsqrtss(const __m128 &T) //Newton Raphson
{
	const __m128 TRSQRT = _mm_rsqrt_ss(T);
	return _mm_mul_ss(_mm_mul_ss(_mm_set_ss(0.5f),TRSQRT), _mm_sub_ss(_mm_set_ss(3.0f),_mm_mul_ss(_mm_mul_ss(TRSQRT,T),TRSQRT)));
}

inline __m128 sseHorizontalAdd(const __m128 &a) // could use dp instruction on SSE4
{
	const __m128 ftemp = _mm_add_ps(a, _mm_movehl_ps(a, a));
	return _mm_add_ss(ftemp,_mm_shuffle_ps(ftemp, ftemp, 1));
}

float sz2f(char *sz);
void f2sz(const float f, char *sz);

void WideStrCopy(WCHAR *wzin, WCHAR *wzout);
int WideStrCmp(WCHAR *wz1, WCHAR *wz2);
int WzSzStrCmp(WCHAR *wz1, char *sz2);
void WideStrCat(WCHAR *wzin, WCHAR *wzout);
int WzSzStrnCmp(WCHAR *wz1, char *sz2, int count);

//void DumpNameTable (char *pszFile, char *pszName);

HRESULT OpenURL(char *szURL);

WCHAR *MakeWide(char *sz);
char *MakeChar(WCHAR *wz);

#endif/* !__DEF_H__ */
