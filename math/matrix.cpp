#include "stdafx.h"
#include "matrix.h"

void Matrix3D::Invert()
//void Gauss (RK8 ** a, RK8 ** b, int n)
{
	int ipvt[4];
	for (int i = 0; i < 4; ++i)
		ipvt[i] = i;

	for (int k = 0; k < 4; ++k)
	{
		float temp = 0.f;
		int l = k;
		for (int i = k; i < 4; ++i)
		{
			const float d = fabsf(m[k][i]);
			if (d > temp)
			{
				temp = d;
				l = i;
			}
		}
		if (l != k)
		{
			const int tmp = ipvt[k];
			ipvt[k] = ipvt[l];
			ipvt[l] = tmp;
			for (int j = 0; j < 4; ++j)
			{
				temp = m[j][k];
				m[j][k] = m[j][l];
				m[j][l] = temp;
			}
		}
		const float d = 1.0f / m[k][k];
		for (int j = 0; j < k; ++j)
		{
			const float c = m[j][k] * d;
			for (int i = 0; i < 4; ++i)
				m[j][i] -= m[k][i] * c;
			m[j][k] = c;
		}
		for (int j = k + 1; j < 4; ++j)
		{
			const float c = m[j][k] * d;
			for (int i = 0; i < 4; ++i)
				m[j][i] -= m[k][i] * c;
			m[j][k] = c;
		}
		for (int i = 0; i < 4; ++i)
			m[k][i] = -m[k][i] * d;
		m[k][k] = d;
	}

	Matrix3D mat3D;
	mat3D.m[ipvt[0]][0] = m[0][0]; mat3D.m[ipvt[0]][1] = m[0][1]; mat3D.m[ipvt[0]][2] = m[0][2]; mat3D.m[ipvt[0]][3] = m[0][3];
	mat3D.m[ipvt[1]][0] = m[1][0]; mat3D.m[ipvt[1]][1] = m[1][1]; mat3D.m[ipvt[1]][2] = m[1][2]; mat3D.m[ipvt[1]][3] = m[1][3];
	mat3D.m[ipvt[2]][0] = m[2][0]; mat3D.m[ipvt[2]][1] = m[2][1]; mat3D.m[ipvt[2]][2] = m[2][2]; mat3D.m[ipvt[2]][3] = m[2][3];
	mat3D.m[ipvt[3]][0] = m[3][0]; mat3D.m[ipvt[3]][1] = m[3][1]; mat3D.m[ipvt[3]][2] = m[3][2]; mat3D.m[ipvt[3]][3] = m[3][3];

	m[0][0] = mat3D.m[0][0]; m[0][1] = mat3D.m[0][1]; m[0][2] = mat3D.m[0][2]; m[0][3] = mat3D.m[0][3];
	m[1][0] = mat3D.m[1][0]; m[1][1] = mat3D.m[1][1]; m[1][2] = mat3D.m[1][2]; m[1][3] = mat3D.m[1][3];
	m[2][0] = mat3D.m[2][0]; m[2][1] = mat3D.m[2][1]; m[2][2] = mat3D.m[2][2]; m[2][3] = mat3D.m[2][3];
	m[3][0] = mat3D.m[3][0]; m[3][1] = mat3D.m[3][1]; m[3][2] = mat3D.m[3][2]; m[3][3] = mat3D.m[3][3];
}
