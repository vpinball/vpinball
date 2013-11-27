// primitive.cpp: implementation of the Primitive class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h" 

Primitive::Primitive()
{
   vertexBuffer = 0;
   vertexBufferRegenerate = true;
   objMeshOrg=0;
   objMesh=0;
   indexList=0;
   indexListSize=0;
   m_d.use3DMesh=false;
   m_d.wasVisible=false;
   m_d.meshFileName[0]=0;
   m_d.useLighting=false;
   m_d.staticRendering=false;
   m_d.sphereMapping=false;
   m_d.m_aRotAndTraTypes[0] = RotX;
   m_d.m_aRotAndTraTypes[1] = RotY;
   m_d.m_aRotAndTraTypes[2] = RotZ;
   m_d.m_aRotAndTraTypes[3] = TraX;
   m_d.m_aRotAndTraTypes[4] = TraY;
   m_d.m_aRotAndTraTypes[5] = TraZ;
   m_d.m_aRotAndTraTypes[6] = ObjRotX;
   m_d.m_aRotAndTraTypes[7] = ObjRotY;
   m_d.m_aRotAndTraTypes[8] = ObjRotZ;
} 

Primitive::~Primitive() 
{
	if(vertexBuffer)
	{
		vertexBuffer->release();
		vertexBuffer = 0;
		//vertexBufferRegenerate = true;
	}
   if( objMeshOrg )
   {
      delete[] objMeshOrg;
   }
   if( objMesh )
   {
      delete[] objMesh;
   }
   if( indexList )
   {
      delete[] indexList;
   }
}

HRESULT Primitive::Init(PinTable *ptable, float x, float y, bool fromMouseClick)
{
   m_ptable = ptable;

   m_d.m_vPosition.x = x;
   m_d.m_vPosition.y = y;

   SetDefaults(false);

   InitVBA(fTrue, 0, NULL);

   return S_OK;
}

void Primitive::SetDefaults(bool fromMouseClick)
{
   HRESULT hr;
   int iTmp;
   float fTmp;

   m_d.use3DMesh=false;
   m_d.meshFileName[0]=0;

   // sides
   hr = GetRegInt("DefaultProps\\Primitive","Sides", &iTmp);
   m_d.m_Sides = (hr == S_OK) && fromMouseClick ? iTmp : 4;
   if(m_d.m_Sides > Max_Primitive_Sides)
      m_d.m_Sides = Max_Primitive_Sides;

   // colors
   hr = GetRegInt("DefaultProps\\Primitive", "TopColor", &iTmp);
   m_d.m_TopColor = (hr == S_OK) && fromMouseClick ? iTmp : RGB(100,100,100);
   hr = GetRegInt("DefaultProps\\Primitive", "SideColor", &iTmp);
   m_d.m_SideColor = (hr == S_OK) && fromMouseClick ? iTmp : RGB(150,150,150);

   // visible
   hr = GetRegInt("DefaultProps\\Primitive", "TopVisible", &iTmp);
   m_d.m_TopVisible = (hr == S_OK) && fromMouseClick ? (iTmp==1) : true;

   // lighting on/off
   hr = GetRegInt("DefaultProps\\Primitive", "UseLighting", &iTmp);
   m_d.useLighting = (hr == S_OK) && fromMouseClick ? (iTmp==1) : false;

   hr = GetRegInt("DefaultProps\\Primitive", "StaticRendering", &iTmp);
   m_d.staticRendering = (hr == S_OK) && fromMouseClick ? (iTmp==1) : false;

   hr = GetRegInt("DefaultProps\\Primitive", "SphereMapping", &iTmp);
   m_d.sphereMapping = (hr == S_OK) && fromMouseClick ? (iTmp==1) : false;

   // Draw Textures inside
   hr = GetRegInt("DefaultProps\\Primitive", "DrawTexturesInside", &iTmp);
   m_d.m_DrawTexturesInside = (hr == S_OK) && fromMouseClick ? (iTmp==1) : true;

   // Position (X and Y is already set by the click of the user)
   hr = GetRegStringAsFloat("DefaultProps\\Primitive","Position_Z", &fTmp);
   m_d.m_vPosition.z = (hr == S_OK) && fromMouseClick ? fTmp : 0;

   // Size
   hr = GetRegStringAsFloat("DefaultProps\\Primitive","Size_X", &fTmp);
   m_d.m_vSize.x = (hr == S_OK) && fromMouseClick ? fTmp : 100;
   hr = GetRegStringAsFloat("DefaultProps\\Primitive","Size_Y", &fTmp);
   m_d.m_vSize.y = (hr == S_OK) && fromMouseClick ? fTmp : 100;
   hr = GetRegStringAsFloat("DefaultProps\\Primitive","Size_Z", &fTmp);
   m_d.m_vSize.z = (hr == S_OK) && fromMouseClick ? fTmp : 100;

   // Axis Scale
   hr = GetRegStringAsFloat("DefaultProps\\Primitive","AxisScale_X_Y", &fTmp);
   m_d.m_vAxisScaleX.y = (hr == S_OK) && fromMouseClick ? fTmp : 1;
   hr = GetRegStringAsFloat("DefaultProps\\Primitive","AxisScale_X_Z", &fTmp);
   m_d.m_vAxisScaleX.z = (hr == S_OK) && fromMouseClick ? fTmp : 1;
   hr = GetRegStringAsFloat("DefaultProps\\Primitive","AxisScale_Y_X", &fTmp);
   m_d.m_vAxisScaleY.x = (hr == S_OK) && fromMouseClick ? fTmp : 1;
   hr = GetRegStringAsFloat("DefaultProps\\Primitive","AxisScale_Y_Z", &fTmp);
   m_d.m_vAxisScaleY.z = (hr == S_OK) && fromMouseClick ? fTmp : 1;
   hr = GetRegStringAsFloat("DefaultProps\\Primitive","AxisScale_Z_X", &fTmp);
   m_d.m_vAxisScaleZ.x = (hr == S_OK) && fromMouseClick ? fTmp : 1;
   hr = GetRegStringAsFloat("DefaultProps\\Primitive","AxisScale_Z_Y", &fTmp);
   m_d.m_vAxisScaleZ.y = (hr == S_OK) && fromMouseClick ? fTmp : 1;
   m_d.m_vAxisScaleX.x = 1.0f;
   m_d.m_vAxisScaleY.y = 1.0f;
   m_d.m_vAxisScaleZ.z = 1.0f;

   // Rotation and Transposition
   hr = GetRegStringAsFloat("DefaultProps\\Primitive","RotAndTra0", &fTmp);
   m_d.m_aRotAndTra[0] = (hr == S_OK) && fromMouseClick ? fTmp : 0;
   hr = GetRegStringAsFloat("DefaultProps\\Primitive","RotAndTra1", &fTmp);
   m_d.m_aRotAndTra[1] = (hr == S_OK) && fromMouseClick ? fTmp : 0;
   hr = GetRegStringAsFloat("DefaultProps\\Primitive","RotAndTra2", &fTmp);
   m_d.m_aRotAndTra[2] = (hr == S_OK) && fromMouseClick ? fTmp : 0;
   hr = GetRegStringAsFloat("DefaultProps\\Primitive","RotAndTra3", &fTmp);
   m_d.m_aRotAndTra[3] = (hr == S_OK) && fromMouseClick ? fTmp : 0;
   hr = GetRegStringAsFloat("DefaultProps\\Primitive","RotAndTra4", &fTmp);
   m_d.m_aRotAndTra[4] = (hr == S_OK) && fromMouseClick ? fTmp : 0;
   hr = GetRegStringAsFloat("DefaultProps\\Primitive","RotAndTra5", &fTmp);
   m_d.m_aRotAndTra[5] = (hr == S_OK) && fromMouseClick ? fTmp : 0;
   hr = GetRegStringAsFloat("DefaultProps\\Primitive","RotAndTra6", &fTmp);
   m_d.m_aRotAndTra[6] = (hr == S_OK) && fromMouseClick ? fTmp : 0;
   hr = GetRegStringAsFloat("DefaultProps\\Primitive","RotAndTra7", &fTmp);
   m_d.m_aRotAndTra[7] = (hr == S_OK) && fromMouseClick ? fTmp : 0;
   hr = GetRegStringAsFloat("DefaultProps\\Primitive","RotAndTra8", &fTmp);
   m_d.m_aRotAndTra[8] = (hr == S_OK) && fromMouseClick ? fTmp : 0;

//    hr = GetRegInt("DefaultProps\\Primitive","RotAndTraType0", &iTmp);
//    m_d.m_aRotAndTraTypes[0] = (hr == S_OK) && fromMouseClick ? (enum RotAndTraTypeEnum)iTmp : RotX;
//    hr = GetRegInt("DefaultProps\\Primitive","RotAndTraType1", &iTmp);
//    m_d.m_aRotAndTraTypes[1] = (hr == S_OK) && fromMouseClick ? (enum RotAndTraTypeEnum)iTmp : RotY;
//    hr = GetRegInt("DefaultProps\\Primitive","RotAndTraType2", &iTmp);
//    m_d.m_aRotAndTraTypes[2] = (hr == S_OK) && fromMouseClick ? (enum RotAndTraTypeEnum)iTmp : RotZ;
//    hr = GetRegInt("DefaultProps\\Primitive","RotAndTraType3", &iTmp);
//    m_d.m_aRotAndTraTypes[3] = (hr == S_OK) && fromMouseClick ? (enum RotAndTraTypeEnum)iTmp : TraX;
//    hr = GetRegInt("DefaultProps\\Primitive","RotAndTraType4", &iTmp);
//    m_d.m_aRotAndTraTypes[4] = (hr == S_OK) && fromMouseClick ? (enum RotAndTraTypeEnum)iTmp : TraY;
//    hr = GetRegInt("DefaultProps\\Primitive","RotAndTraType5", &iTmp);
//    m_d.m_aRotAndTraTypes[5] = (hr == S_OK) && fromMouseClick ? (enum RotAndTraTypeEnum)iTmp : TraZ;
//    hr = GetRegInt("DefaultProps\\Primitive","RotAndTraType6", &iTmp);
//    m_d.m_aRotAndTraTypes[6] = (hr == S_OK) && fromMouseClick ? (enum RotAndTraTypeEnum)iTmp : ObjRotX;
//    hr = GetRegInt("DefaultProps\\Primitive","RotAndTraType7", &iTmp);
//    m_d.m_aRotAndTraTypes[7] = (hr == S_OK) && fromMouseClick ? (enum RotAndTraTypeEnum)iTmp : ObjRotY;
//    hr = GetRegInt("DefaultProps\\Primitive","RotAndTraType8", &iTmp);
//    m_d.m_aRotAndTraTypes[8] = (hr == S_OK) && fromMouseClick ? (enum RotAndTraTypeEnum)iTmp : ObjRotZ;
   m_d.m_aRotAndTraTypes[0] = RotX;
   m_d.m_aRotAndTraTypes[1] = RotY;
   m_d.m_aRotAndTraTypes[2] = RotZ;
   m_d.m_aRotAndTraTypes[3] = TraX;
   m_d.m_aRotAndTraTypes[4] = TraY;
   m_d.m_aRotAndTraTypes[5] = TraZ;
   m_d.m_aRotAndTraTypes[6] = ObjRotX;
   m_d.m_aRotAndTraTypes[7] = ObjRotY;
   m_d.m_aRotAndTraTypes[8] = ObjRotZ;
   /*
   hr = GetRegStringAsFloat("DefaultProps\\Primitive","Rotation_X", &fTmp);
   m_d.m_vRotation.x = (hr == S_OK) ? fTmp : 0;
   hr = GetRegStringAsFloat("DefaultProps\\Primitive","Rotation_Y", &fTmp);
   m_d.m_vRotation.y = (hr == S_OK) ? fTmp : 0;
   hr = GetRegStringAsFloat("DefaultProps\\Primitive","Rotation_Z", &fTmp);
   m_d.m_vRotation.z = (hr == S_OK) ? fTmp : 0;

   //Transposition
   hr = GetRegStringAsFloat("DefaultProps\\Primitive","Transposition_X", &fTmp);
   m_d.m_vTransposition.x = (hr == S_OK) ? fTmp : 0;
   hr = GetRegStringAsFloat("DefaultProps\\Primitive","Transposition_Y", &fTmp);
   m_d.m_vTransposition.y = (hr == S_OK) ? fTmp : 0;
   hr = GetRegStringAsFloat("DefaultProps\\Primitive","Transposition_Z", &fTmp);
   m_d.m_vTransposition.z = (hr == S_OK) ? fTmp : 0;
   */

   hr = GetRegString("DefaultProps\\Primitive","Image", m_d.m_szImage, MAXTOKEN);
   if ((hr != S_OK) && fromMouseClick)
      m_d.m_szImage[0] = 0;
}

void Primitive::WriteRegDefaults()
{
   char strTmp[40];
   int iTmp;

   SetRegValue("DefaultProps\\Primitive","TopColor",REG_DWORD,&m_d.m_TopColor,4);
   SetRegValue("DefaultProps\\Primitive","SideColor",REG_DWORD,&m_d.m_SideColor,4);
   iTmp = (m_d.m_TopVisible) ? 1 : 0;
   SetRegValue("DefaultProps\\Primitive","TopVisible",REG_DWORD,&iTmp,4);
   iTmp = (m_d.useLighting) ? 1 : 0;
   SetRegValue("DefaultProps\\Primitive","UseLighting",REG_DWORD,&iTmp,4);
   iTmp = (m_d.staticRendering) ? 1 : 0;
   SetRegValue("DefaultProps\\Primitive","StaticRendering",REG_DWORD,&iTmp,4);
   iTmp = (m_d.sphereMapping) ? 1 : 0;
   SetRegValue("DefaultProps\\Primitive","SphereMapping",REG_DWORD,&iTmp,4);

   iTmp = (m_d.m_DrawTexturesInside) ? 1 : 0;
   SetRegValue("DefaultProps\\Primitive","DrawTexturesInside",REG_DWORD,&iTmp,4);

   sprintf_s(strTmp, 40, "%f", m_d.m_vPosition.z);
   SetRegValue("DefaultProps\\Primitive","Position_Z", REG_SZ, &strTmp,strlen(strTmp));	

   sprintf_s(strTmp, 40, "%f", m_d.m_vSize.x);
   SetRegValue("DefaultProps\\Primitive","Size_X", REG_SZ, &strTmp,strlen(strTmp));	
   sprintf_s(strTmp, 40, "%f", m_d.m_vSize.y);
   SetRegValue("DefaultProps\\Primitive","Size_Y", REG_SZ, &strTmp,strlen(strTmp));	
   sprintf_s(strTmp, 40, "%f", m_d.m_vSize.z);
   SetRegValue("DefaultProps\\Primitive","Size_Z", REG_SZ, &strTmp,strlen(strTmp));	

   sprintf_s(strTmp, 40, "%f", m_d.m_vAxisScaleX.y);
   SetRegValue("DefaultProps\\Primitive","AxisScaleXY", REG_SZ, &strTmp,strlen(strTmp));	
   sprintf_s(strTmp, 40, "%f", m_d.m_vAxisScaleX.z);
   SetRegValue("DefaultProps\\Primitive","AxisScaleXZ", REG_SZ, &strTmp,strlen(strTmp));	
   sprintf_s(strTmp, 40, "%f", m_d.m_vAxisScaleY.x);
   SetRegValue("DefaultProps\\Primitive","AxisScaleYX", REG_SZ, &strTmp,strlen(strTmp));	
   sprintf_s(strTmp, 40, "%f", m_d.m_vAxisScaleY.z);
   SetRegValue("DefaultProps\\Primitive","AxisScaleYZ", REG_SZ, &strTmp,strlen(strTmp));	
   sprintf_s(strTmp, 40, "%f", m_d.m_vAxisScaleZ.x);
   SetRegValue("DefaultProps\\Primitive","AxisScaleZX", REG_SZ, &strTmp,strlen(strTmp));	
   sprintf_s(strTmp, 40, "%f", m_d.m_vAxisScaleZ.y);
   SetRegValue("DefaultProps\\Primitive","AxisScaleZY", REG_SZ, &strTmp,strlen(strTmp));	

   sprintf_s(strTmp, 40, "%f", m_d.m_aRotAndTra[0]);
   SetRegValue("DefaultProps\\Primitive","RotAndTra0", REG_SZ, &strTmp,strlen(strTmp));	
   sprintf_s(strTmp, 40, "%f", m_d.m_aRotAndTra[1]);
   SetRegValue("DefaultProps\\Primitive","RotAndTra1", REG_SZ, &strTmp,strlen(strTmp));	
   sprintf_s(strTmp, 40, "%f", m_d.m_aRotAndTra[2]);
   SetRegValue("DefaultProps\\Primitive","RotAndTra2", REG_SZ, &strTmp,strlen(strTmp));	
   sprintf_s(strTmp, 40, "%f", m_d.m_aRotAndTra[3]);
   SetRegValue("DefaultProps\\Primitive","RotAndTra3", REG_SZ, &strTmp,strlen(strTmp));	
   sprintf_s(strTmp, 40, "%f", m_d.m_aRotAndTra[4]);
   SetRegValue("DefaultProps\\Primitive","RotAndTra4", REG_SZ, &strTmp,strlen(strTmp));	
   sprintf_s(strTmp, 40, "%f", m_d.m_aRotAndTra[5]);
   SetRegValue("DefaultProps\\Primitive","RotAndTra5", REG_SZ, &strTmp,strlen(strTmp));	
   sprintf_s(strTmp, 40, "%f", m_d.m_aRotAndTra[6]);
   SetRegValue("DefaultProps\\Primitive","RotAndTra6", REG_SZ, &strTmp,strlen(strTmp));	
   sprintf_s(strTmp, 40, "%f", m_d.m_aRotAndTra[7]);
   SetRegValue("DefaultProps\\Primitive","RotAndTra7", REG_SZ, &strTmp,strlen(strTmp));	
   sprintf_s(strTmp, 40, "%f", m_d.m_aRotAndTra[8]);
   SetRegValue("DefaultProps\\Primitive","RotAndTra8", REG_SZ, &strTmp,strlen(strTmp));	

   SetRegValue("DefaultProps\\Primitive","RotAndTraType0",REG_DWORD,&m_d.m_aRotAndTraTypes[0],4);
   SetRegValue("DefaultProps\\Primitive","RotAndTraType1",REG_DWORD,&m_d.m_aRotAndTraTypes[1],4);
   SetRegValue("DefaultProps\\Primitive","RotAndTraType2",REG_DWORD,&m_d.m_aRotAndTraTypes[2],4);
   SetRegValue("DefaultProps\\Primitive","RotAndTraType3",REG_DWORD,&m_d.m_aRotAndTraTypes[3],4);
   SetRegValue("DefaultProps\\Primitive","RotAndTraType4",REG_DWORD,&m_d.m_aRotAndTraTypes[4],4);
   SetRegValue("DefaultProps\\Primitive","RotAndTraType5",REG_DWORD,&m_d.m_aRotAndTraTypes[5],4);
   SetRegValue("DefaultProps\\Primitive","RotAndTraType6",REG_DWORD,&m_d.m_aRotAndTraTypes[6],4);
   SetRegValue("DefaultProps\\Primitive","RotAndTraType7",REG_DWORD,&m_d.m_aRotAndTraTypes[7],4);
   SetRegValue("DefaultProps\\Primitive","RotAndTraType8",REG_DWORD,&m_d.m_aRotAndTraTypes[8],4);
   /*
   sprintf_s(strTmp, 40, "%f", m_d.m_vRotation.x);
   SetRegValue("DefaultProps\\Primitive","Rotation_X", REG_SZ, &strTmp,strlen(strTmp));	
   sprintf_s(strTmp, 40, "%f", m_d.m_vRotation.y);
   SetRegValue("DefaultProps\\Primitive","Rotation_Y", REG_SZ, &strTmp,strlen(strTmp));	
   sprintf_s(strTmp, 40, "%f", m_d.m_vRotation.z);
   SetRegValue("DefaultProps\\Primitive","Rotation_Z", REG_SZ, &strTmp,strlen(strTmp));	

   sprintf_s(strTmp, 40, "%f", m_d.m_vTransposition.x);
   SetRegValue("DefaultProps\\Primitive","Transposition_X", REG_SZ, &strTmp,strlen(strTmp));	
   sprintf_s(strTmp, 40, "%f", m_d.m_vTransposition.y);
   SetRegValue("DefaultProps\\Primitive","Transposition_Y", REG_SZ, &strTmp,strlen(strTmp));	
   sprintf_s(strTmp, 40, "%f", m_d.m_vTransposition.z);
   SetRegValue("DefaultProps\\Primitive","Transposition_Z", REG_SZ, &strTmp,strlen(strTmp));	
   */
   SetRegValue("DefaultProps\\Primitive","Image", REG_SZ, &m_d.m_szImage,strlen(m_d.m_szImage));
}

void Primitive::GetTimers(Vector<HitTimer> * const pvht)
{
   IEditable::BeginPlay();
}

void Primitive::GetHitShapes(Vector<HitObject> * const pvho)
{
   //!! Here the hitshapes have to be added... lets look at other implementations.
   // OK, i need a hitprimitive class and a hitanimobject class.
   // the hitprimitive class should add itself to the HitObjectVector.
   // i think i have to look at easy hit objects and then at ramps hitobjects.
   HitPrimitive * pHitPrimitive = new HitPrimitive();

   pHitPrimitive->m_primitiveAnim.m_pprimitive = this; //!! as a temporary(?) workaround use this to recalculate

   pvho->AddElement(pHitPrimitive);
}

void Primitive::GetHitShapesDebug(Vector<HitObject> * const pvho)
{
}

void Primitive::EndPlay()
{
	if(vertexBuffer)
	{
		vertexBuffer->release();
		vertexBuffer = 0;
		vertexBufferRegenerate = true;
	}

	m_d.wasVisible = false;

	if(objMesh)
	{
		delete [] objMesh;
		objMesh = 0;
	}
}

//////////////////////////////
// Calculation
//////////////////////////////

void Primitive::RecalculateMatrices()
{
   // scale matrix
   Matrix3D Smatrix;
   Smatrix.SetIdentity();
   Smatrix._11 = m_d.m_vSize.x;
   Smatrix._22 = m_d.m_vSize.y;
   Smatrix._33 = m_d.m_vSize.z;

   // transform matrix
   Matrix3D Tmatrix;
   Tmatrix.SetIdentity();
   Tmatrix._41 = m_d.m_vPosition.x;
   Tmatrix._42 = m_d.m_vPosition.y;
   Tmatrix._43 = m_d.m_vPosition.z;

   Matrix3D tempMatrix;
   Matrix3D RTmatrix;
   RTmatrix.SetIdentity();

   tempMatrix.SetIdentity();
   tempMatrix._41 = m_d.m_aRotAndTra[3];
   tempMatrix._42 = m_d.m_aRotAndTra[4];
   tempMatrix._43 = m_d.m_aRotAndTra[5];
   tempMatrix.Multiply(RTmatrix, RTmatrix);
   tempMatrix.RotateZMatrix(ANGTORAD(m_d.m_aRotAndTra[2]));
   tempMatrix.Multiply(RTmatrix, RTmatrix);
   tempMatrix.RotateYMatrix(ANGTORAD(m_d.m_aRotAndTra[1]));
   tempMatrix.Multiply(RTmatrix, RTmatrix);
   tempMatrix.RotateXMatrix(ANGTORAD(m_d.m_aRotAndTra[0]));
   tempMatrix.Multiply(RTmatrix, RTmatrix);
 
   tempMatrix.RotateZMatrix(ANGTORAD(m_d.m_aRotAndTra[8]));
   tempMatrix.Multiply(RTmatrix, RTmatrix);
   tempMatrix.RotateYMatrix(ANGTORAD(m_d.m_aRotAndTra[7]));
   tempMatrix.Multiply(RTmatrix, RTmatrix);
   tempMatrix.RotateXMatrix(ANGTORAD(m_d.m_aRotAndTra[6]));
   tempMatrix.Multiply(RTmatrix, RTmatrix);

   rotMatrix = RTmatrix;
   fullMatrix = Smatrix;

   RTmatrix.Multiply(fullMatrix, fullMatrix);
   Tmatrix.Multiply(fullMatrix, fullMatrix);
}

void Primitive::RecalculateVertices() 
{
   for( int i=0;i<verticesTop.Size();i++ )
      delete verticesTop.ElementAt(i);

   for( int i=0;i<verticesBottom.Size();i++ )
      delete verticesBottom.ElementAt(i);

   verticesTop.RemoveAllElements();
   verticesBottom.RemoveAllElements();

   if( !m_d.use3DMesh )
   {
      const float outerRadius = -0.5f/cosf((float)M_PI/(float)m_d.m_Sides);
      const float addAngle = (float)(2.0*M_PI)/(float)m_d.m_Sides;
	  const float offsAngle = (float)M_PI/(float)m_d.m_Sides;
      for (int i = 0; i < m_d.m_Sides; ++i)
      {
         Vertex3Ds * const topVert = new Vertex3Ds();
         Vertex3Ds * const bottomVert = new Vertex3Ds();

		 const float currentAngle = addAngle*(float)i + offsAngle;
         topVert->x = sinf(currentAngle)*outerRadius;
         topVert->y = cosf(currentAngle)*outerRadius * (1.0f+(m_d.m_vAxisScaleX.y - 1.0f)*(topVert->x+0.5f));
         topVert->z =                           0.5f * (1.0f+(m_d.m_vAxisScaleX.z - 1.0f)*(topVert->x+0.5f));

         topVert->x *= 1.0f+(m_d.m_vAxisScaleY.x - 1.0f)*(topVert->y+0.5f);
         topVert->z *= 1.0f+(m_d.m_vAxisScaleY.z - 1.0f)*(topVert->y+0.5f);
         bottomVert->z = -topVert->z;

         const float tmp = topVert->x;
         topVert->x    = tmp * (1.0f+(m_d.m_vAxisScaleZ.x - 1.0f)*(topVert->z+0.5f));
         bottomVert->x = tmp * (1.0f+(m_d.m_vAxisScaleZ.x - 1.0f)*(0.5f-topVert->z));

         const float tmp2 = topVert->y;
         topVert->y    = tmp2 * (1.0f+(m_d.m_vAxisScaleZ.y - 1.0f)*(topVert->z+0.5f));
         bottomVert->y = tmp2 * (1.0f+(m_d.m_vAxisScaleZ.y - 1.0f)*(0.5f-topVert->z));

         fullMatrix.MultiplyVector(topVert->x, topVert->y, topVert->z, topVert);
         fullMatrix.MultiplyVector(bottomVert->x, bottomVert->y, bottomVert->z, bottomVert);
         verticesTop.AddElement(topVert);
         verticesBottom.AddElement(bottomVert);
      }
   }
   else
   {
      for( int i=0;i<numVertices;i++ )
      {
         Vertex3Ds *const vert = new Vertex3Ds();
         vert->x = objMeshOrg[i].x;
         vert->y = objMeshOrg[i].y;
         vert->z = objMeshOrg[i].z;
         fullMatrix.MultiplyVector(vert->x, vert->y, vert->z, vert);
         verticesTop.AddElement(vert);
      }
   }
}

//////////////////////////////
// Rendering
//////////////////////////////

//2d
void Primitive::PreRender(Sur * const psur)
{
   /*
   psur->SetBorderColor(-1,false,0);
   psur->SetFillColor(m_d.m_TopColor);
   psur->SetObject(this);

   psur->Line(m_d.m_vPosition.x -10.0f, m_d.m_vPosition.y,m_d.m_vPosition.x +10.0f, m_d.m_vPosition.y);
   psur->Line(m_d.m_vPosition.x, m_d.m_vPosition.y -10.0f,m_d.m_vPosition.x, m_d.m_vPosition.y +10.0f);
   */
}

void Primitive::Render(Sur * const psur)
{
   RecalculateMatrices();
   RecalculateVertices();
   //psur->SetBorderColor(RGB(0,0,0),false,2);
   psur->SetLineColor(RGB(0,0,0),false,1);
   //psur->SetFillColor(-1);
   psur->SetObject(this);
   //psur->SetObject(NULL);
   if( !m_d.use3DMesh )
   {
      for (int i = 0; i < m_d.m_Sides; i++)
      {
         const int inext = ((i+1) == m_d.m_Sides) ? 0 : i+1;
         const Vertex3Ds * const topVert = verticesTop.ElementAt(i);
         const Vertex3Ds * const nextTopVert = verticesTop.ElementAt(inext);
         psur->Line(topVert->x, topVert->y, nextTopVert->x, nextTopVert->y);
         const Vertex3Ds * const bottomVert = verticesBottom.ElementAt(i);
         const Vertex3Ds * const nextBottomVert = verticesBottom.ElementAt(inext);
         psur->Line(bottomVert->x, bottomVert->y, nextBottomVert->x, nextBottomVert->y);
         psur->Line(bottomVert->x, bottomVert->y, topVert->x, topVert->y);
      }
      psur->Line(m_d.m_vPosition.x -20.0f, m_d.m_vPosition.y,m_d.m_vPosition.x +20.0f, m_d.m_vPosition.y);
      psur->Line(m_d.m_vPosition.x, m_d.m_vPosition.y -20.0f,m_d.m_vPosition.x, m_d.m_vPosition.y +20.0f);
   }
   else
   {
      //just draw a simple mesh layout not the entire mesh for performance reasons
      for( int i=0;i<indexListSize;i+=3 )
      {
         const Vertex3Ds * const A = verticesTop.ElementAt(indexList[i]);
         const Vertex3Ds * const B = verticesTop.ElementAt(indexList[i+1]);
         psur->Line( A->x,A->y,B->x,B->y);
         //psur->Line( B->x,B->y,C->x,C->y);
         //psur->Line( C->x,C->y,A->x,A->y);
      }
   }
}

//static const WORD rgiPrimStatic0[5] = {0,1,2,3,4};
static const WORD rgiPrimStatic1[5] = {4,3,2,1,0};

void Primitive::CalculateBuiltinOriginal()
{
   Texture * const pin = m_ptable->GetImage(m_d.m_szImage);
   float maxtu,maxtv;
   if (pin) 
   {
      DDSURFACEDESC2 ddsd;
      ddsd.dwSize = sizeof(ddsd);

      pin->m_pdsBuffer->GetSurfaceDesc(&ddsd);
      maxtu = (float)pin->m_width / (float)ddsd.dwWidth;
      maxtv = (float)pin->m_height / (float)ddsd.dwHeight;
   }
   else
      maxtu = maxtv = 1.f;

   // this recalculates the Original Vertices -> should be only called, when sides are altered.
   const float outerRadius = -0.5f/(cosf((float)M_PI/(float)m_d.m_Sides));
   const float addAngle = (float)(2.0*M_PI)/(float)m_d.m_Sides;
   const float offsAngle = (float)M_PI/(float)m_d.m_Sides;
   float minX = FLT_MAX;
   float minY = FLT_MAX;
   float maxX = -FLT_MAX;
   float maxY = -FLT_MAX;

   Vertex3D_NoTex2 *middle;
   middle = &builtin_rgvOriginal[0]; // middle point top
   middle->x = 0.0f;
   middle->y = 0.0f;
   middle->z = 0.5f;
   middle = &builtin_rgvOriginal[m_d.m_Sides+1]; // middle point bottom
   middle->x = 0.0f;
   middle->y = 0.0f;
   middle->z = -0.5f;
   for (int i = 0; i < m_d.m_Sides; ++i)
   {
      // calculate Top
      Vertex3D_NoTex2 * const topVert = &builtin_rgvOriginal[i+1]; // top point at side
      const float currentAngle = addAngle*(float)i + offsAngle;
      topVert->x = sinf(currentAngle)*outerRadius;
      topVert->y = cosf(currentAngle)*outerRadius;		
      topVert->z = 0.5f;

      // calculate bottom
      Vertex3D_NoTex2 * const bottomVert = &builtin_rgvOriginal[i+1 + m_d.m_Sides+1]; // bottompoint at side
      bottomVert->x = topVert->x;
      bottomVert->y = topVert->y;
      bottomVert->z = -0.5f;

      // calculate sides
      builtin_rgvOriginal[m_d.m_Sides*2 + 2 + i] = *topVert; // sideTopVert
      builtin_rgvOriginal[m_d.m_Sides*3 + 2 + i] = *bottomVert; // sideBottomVert

      // calculate bounds for X and Y
      if (topVert->x < minX)
         minX = topVert->x;
      if (topVert->x > maxX)
         maxX = topVert->x;
      if (topVert->y < minY)
         minY = topVert->y;
      if (topVert->y > maxY)
         maxY = topVert->y;
   }

   // these have to be replaced for image mapping
   middle = &builtin_rgvOriginal[0]; // middle point top
   middle->tu = maxtu*0.25f;   // /4
   middle->tv = maxtv*0.25f;   // /4
   middle = &builtin_rgvOriginal[m_d.m_Sides+1]; // middle point bottom
   middle->tu = maxtu*(float)(0.25*3.); // /4*3
   middle->tv = maxtv*0.25f;   // /4
   const float invx = (maxtu*0.5f)/(maxX-minX);
   const float invy = (maxtv*0.5f)/(maxY-minY);
   const float invs = maxtu/(float)m_d.m_Sides;
   for (int i = 0; i < m_d.m_Sides; i++)
   {
      Vertex3D_NoTex2 * const topVert = &builtin_rgvOriginal[i+1]; // top point at side
      topVert->tu = (topVert->x - minX)*invx;
      topVert->tv = (topVert->y - minY)*invy;

      Vertex3D_NoTex2 * const bottomVert = &builtin_rgvOriginal[i+1 + m_d.m_Sides+1]; // bottompoint at side
      bottomVert->tu = topVert->tu+0.5f*maxtu;
      bottomVert->tv = topVert->tv;

      Vertex3D_NoTex2 * const sideTopVert = &builtin_rgvOriginal[m_d.m_Sides*2 + 2 + i];
      Vertex3D_NoTex2 * const sideBottomVert = &builtin_rgvOriginal[m_d.m_Sides*3 + 2 + i];

      sideTopVert->tu = (float)i*invs;
      sideTopVert->tv = 0.5f*maxtv;
      sideBottomVert->tu = sideTopVert->tu;
      sideBottomVert->tv = /*1.0f**/maxtv;
   }
   // 2 restore indices
   //   check if anti culling is enabled:
   if (m_d.m_DrawTexturesInside)
   {
      // yes: draw everything twice
      // restore indices
      for (int i = 0; i < m_d.m_Sides; i++)
      {
         const int tmp = (i == m_d.m_Sides-1) ? 1 : (i+2); // wrapping around
         // top
         builtin_indices[i*6  ] = 0;
         builtin_indices[i*6+1] = i + 1;
         builtin_indices[i*6+2] = tmp;
         builtin_indices[i*6+3] = 0;
         builtin_indices[i*6+4] = tmp;
         builtin_indices[i*6+5] = i + 1;

         const int tmp2 = tmp+1;
         // bottom
         builtin_indices[6 * (i + m_d.m_Sides)    ] = m_d.m_Sides + 1;
         builtin_indices[6 * (i + m_d.m_Sides) + 1] = m_d.m_Sides + tmp2;
         builtin_indices[6 * (i + m_d.m_Sides) + 2] = m_d.m_Sides + 2 + i;
         builtin_indices[6 * (i + m_d.m_Sides) + 3] = m_d.m_Sides + 1;
         builtin_indices[6 * (i + m_d.m_Sides) + 4] = m_d.m_Sides + 2 + i;
         builtin_indices[6 * (i + m_d.m_Sides) + 5] = m_d.m_Sides + tmp2;
         // sides
         builtin_indices[12 * (i + m_d.m_Sides)    ] = m_d.m_Sides*2 + tmp2;
         builtin_indices[12 * (i + m_d.m_Sides) + 1] = m_d.m_Sides*2 + 2 + i;
         builtin_indices[12 * (i + m_d.m_Sides) + 2] = m_d.m_Sides*3 + 2 + i;
         builtin_indices[12 * (i + m_d.m_Sides) + 3] = m_d.m_Sides*2 + tmp2;
         builtin_indices[12 * (i + m_d.m_Sides) + 4] = m_d.m_Sides*3 + 2 + i;
         builtin_indices[12 * (i + m_d.m_Sides) + 5] = m_d.m_Sides*3 + tmp2;
         builtin_indices[12 * (i + m_d.m_Sides) + 6] = m_d.m_Sides*2 + tmp2;
         builtin_indices[12 * (i + m_d.m_Sides) + 7] = m_d.m_Sides*3 + 2 + i;
         builtin_indices[12 * (i + m_d.m_Sides) + 8] = m_d.m_Sides*2 + 2 + i;
         builtin_indices[12 * (i + m_d.m_Sides) + 9] = m_d.m_Sides*2 + tmp2;
         builtin_indices[12 * (i + m_d.m_Sides) + 10]= m_d.m_Sides*3 + tmp2;
         builtin_indices[12 * (i + m_d.m_Sides) + 11]= m_d.m_Sides*3 + 2 + i;
      }
   } else {
      // yes: draw everything twice
      // restore indices
      for (int i = 0; i < m_d.m_Sides; i++)
      {
         const int tmp = (i == m_d.m_Sides-1) ? 1 : (i+2); // wrapping around
         // top
         builtin_indices[i*3  ] = 0;
         builtin_indices[i*3+2] = i + 1;
         builtin_indices[i*3+1] = tmp;

         const int tmp2 = tmp+1;
         // bottom
         builtin_indices[3 * (i + m_d.m_Sides)    ] = m_d.m_Sides + 1;
         builtin_indices[3 * (i + m_d.m_Sides) + 1] = m_d.m_Sides + 2 + i;
         builtin_indices[3 * (i + m_d.m_Sides) + 2] = m_d.m_Sides + tmp2;

         // sides
         builtin_indices[6 * (i + m_d.m_Sides)    ] = m_d.m_Sides*2 + tmp2;
         builtin_indices[6 * (i + m_d.m_Sides) + 1] = m_d.m_Sides*3 + 2 + i;
         builtin_indices[6 * (i + m_d.m_Sides) + 2] = m_d.m_Sides*2 + 2 + i;
         builtin_indices[6 * (i + m_d.m_Sides) + 3] = m_d.m_Sides*2 + tmp2;
         builtin_indices[6 * (i + m_d.m_Sides) + 4] = m_d.m_Sides*3 + tmp2;
         builtin_indices[6 * (i + m_d.m_Sides) + 5] = m_d.m_Sides*3 + 2 + i;
      }
   }

}

void Primitive::CalculateBuiltin()
{
   // 1 copy vertices
   memcpy(builtin_rgv,builtin_rgvOriginal,(m_d.m_Sides*4 + 2)*sizeof(Vertex3D_NoTex2));

   // 2 apply matrix trafo

   SetNormal( builtin_rgv, builtin_indices,  m_d.m_DrawTexturesInside ? 24*m_d.m_Sides : 12*m_d.m_Sides ,NULL,NULL,0 );

   // could be optimized, if not everything is drawn.
   for (int i = 0; i < (m_d.m_Sides*4 + 2); i++)
   {
      Vertex3D_NoTex2 * const tempVert = &builtin_rgv[i];
      tempVert->y *= 1.0f+(m_d.m_vAxisScaleX.y - 1.0f)*(tempVert->x+0.5f);
      tempVert->z *= 1.0f+(m_d.m_vAxisScaleX.z - 1.0f)*(tempVert->x+0.5f);
      tempVert->x *= 1.0f+(m_d.m_vAxisScaleY.x - 1.0f)*(tempVert->y+0.5f);
      tempVert->z *= 1.0f+(m_d.m_vAxisScaleY.z - 1.0f)*(tempVert->y+0.5f);
      tempVert->x *= 1.0f+(m_d.m_vAxisScaleZ.x - 1.0f)*(tempVert->z+0.5f);
      tempVert->y *= 1.0f+(m_d.m_vAxisScaleZ.y - 1.0f)*(tempVert->z+0.5f);
      fullMatrix.MultiplyVector(tempVert->x, tempVert->y, tempVert->z, tempVert);
   }

   // 3 depth calculation / sorting

   // Since we are compiling with SSE, I'll use Floating points for comparison.
   // I need m_sides values at top
   // I need m_sides values at bottom
   // I need m_sides values at the side, since i use only one depth value for each side instead of two.
   // in the implementation i will use shell sort like implemented at wikipedia.
   // Other algorithms are better at presorted things, but i will have some reverse sorted elements between the presorted here. 
   // That's why insertion or bubble sort does not work fast here...
   // input: an array a of length n with array elements numbered 0 to n ? 1

   const float zMultiplicator = cosf(ANGTORAD(m_ptable->m_inclination));
   const float yMultiplicator = sinf(ANGTORAD(m_ptable->m_inclination));

   // get depths
   if (!m_d.m_DrawTexturesInside)
   {
      // top and bottom
      for (int i = 0; i < m_d.m_Sides * 2; i++)
      {
         //!! this is wrong!
         builtin_depth[i] = 
            zMultiplicator*builtin_rgv[builtin_indices[i*3  ]].z+
            zMultiplicator*builtin_rgv[builtin_indices[i*3+1]].z+
            zMultiplicator*builtin_rgv[builtin_indices[i*3+2]].z+
            yMultiplicator*builtin_rgv[builtin_indices[i*3  ]].y+
            yMultiplicator*builtin_rgv[builtin_indices[i*3+1]].y+
            yMultiplicator*builtin_rgv[builtin_indices[i*3+2]].y;
      }
   } else {
      const float zM13 = (float)(1.0/3.0) * zMultiplicator;
      const float yM13 = (float)(1.0/3.0) * yMultiplicator;
      // top and bottom
      for (int i = 0; i < m_d.m_Sides * 2; i++)
      {
         builtin_depth[i] = 
            (builtin_rgv[builtin_indices[i*6  ]].z+
             builtin_rgv[builtin_indices[i*6+1]].z+
             builtin_rgv[builtin_indices[i*6+2]].z) 
            * zM13 +
            (builtin_rgv[builtin_indices[i*6  ]].y+
             builtin_rgv[builtin_indices[i*6+1]].y+
             builtin_rgv[builtin_indices[i*6+2]].y) 
            * yM13;
      }

      const float zM05 = 0.5f * zMultiplicator;
      const float yM05 = 0.5f * yMultiplicator;
      for (int i = m_d.m_Sides; i < m_d.m_Sides * 2; i++)
      {
         builtin_depth[i*2] = 
            (builtin_rgv[builtin_indices[i*12  ]].z+
             builtin_rgv[builtin_indices[i*12+1]].z)
            * zM05 +
            (builtin_rgv[builtin_indices[i*12  ]].y+
             builtin_rgv[builtin_indices[i*12+1]].y)
            * yM05;
         builtin_depth[i*2+1] = 
            (builtin_rgv[builtin_indices[i*12  ]].z+
             builtin_rgv[builtin_indices[i*12+1]].z)
            * zM05 +
            (builtin_rgv[builtin_indices[i*12  ]].y+
             builtin_rgv[builtin_indices[i*12+1]].y)
            * yM05;
      }
   }

   // now shell sort using 2.2 gaps
   if (m_d.m_DrawTexturesInside)
   {
      int inc = (m_d.m_Sides*4)/2;
      while (inc > 0)
      {
         for (int i = inc; i < m_d.m_Sides*4; i++)
         {
            // store temp
            const float tempDepth = builtin_depth[i];
            int tempIndices[6];
            for (int tempI = 0; tempI < 6; tempI++)
               tempIndices[tempI] = builtin_indices[i*6 + tempI];

            int j = i;
            while ((j >= inc) && (builtin_depth[j-inc] > tempDepth))
            {
               builtin_depth[j] = builtin_depth[j-inc];
               for (int tempI = 0; tempI < 6; tempI++)
                  builtin_indices[j*6+tempI] = builtin_indices[(j-inc)*6 + tempI];
               j -= inc;
            }

            builtin_depth[j] = tempDepth;
            for (int tempI = 0; tempI < 6; tempI++)
               builtin_indices[j*6+tempI] = tempIndices[tempI];
         }

         if(inc == 2)
            inc = 1;
         else
            inc = (int)((float)inc*(float)(1.0/2.2));
      }
   } //else { //!! this is missing completely!!???
   //}

   // 4 update the bounding box for the primitive to tell the renderer where to update the back buffer
   g_pplayer->m_pin3d.ClearExtents(&m_d.boundRectangle,NULL,NULL);
   g_pplayer->m_pin3d.ExpandExtents(&m_d.boundRectangle, builtin_rgv, NULL, NULL, numVertices, fFalse);

   // 5 store in vertexbuffer
   Vertex3D_NoTex2 *buf;
   vertexBuffer->lock(0,0,(void**)&buf, VertexBuffer::WRITEONLY | VertexBuffer::NOOVERWRITE );
   memcpy( buf, builtin_rgv, sizeof(Vertex3D_NoTex2)*numVertices );
   vertexBuffer->unlock();
}

void Primitive::UpdateMesh()
{
   memcpy(objMesh,objMeshOrg,numVertices*sizeof(Vertex3D_NoTex2));
   if ( m_d.sphereMapping )
   {
      Matrix3D matWorld;
      g_pplayer->m_pin3d.m_pd3dDevice->GetTransform( D3DTRANSFORMSTATE_WORLD, &matWorld );
      matWorld.Multiply(rotMatrix, rotMatrix);
   }
   // could be optimized, if not everything is drawn.
   for (int i = 0; i < numVertices; i++)
   {
      Vertex3D_NoTex2 * const tempVert = &objMesh[i];
      if ( m_d.sphereMapping )
      {
         Vertex3Ds norm;
         norm.x = tempVert->nx;
         norm.y = tempVert->ny;
         norm.z = tempVert->nz;
         norm = rotMatrix.MultiplyVectorNoTranslate(norm);
         tempVert->tu = 0.5f+ norm.x*0.5f;
         tempVert->tv = 0.5f+ norm.y*0.5f;
      }
      tempVert->y *= 1.0f+(m_d.m_vAxisScaleX.y - 1.0f)*(tempVert->x+0.5f);
      tempVert->z *= 1.0f+(m_d.m_vAxisScaleX.z - 1.0f)*(tempVert->x+0.5f);
      tempVert->x *= 1.0f+(m_d.m_vAxisScaleY.x - 1.0f)*(tempVert->y+0.5f);
      tempVert->z *= 1.0f+(m_d.m_vAxisScaleY.z - 1.0f)*(tempVert->y+0.5f);
      tempVert->x *= 1.0f+(m_d.m_vAxisScaleZ.x - 1.0f)*(tempVert->z+0.5f);
      tempVert->y *= 1.0f+(m_d.m_vAxisScaleZ.y - 1.0f)*(tempVert->z+0.5f);
      fullMatrix.MultiplyVector(tempVert->x, tempVert->y, tempVert->z, tempVert);
   }
   // update the bounding box for the primitive to tell the renderer where to update the back buffer
   g_pplayer->m_pin3d.ClearExtents(&m_d.boundRectangle,NULL,NULL);
   g_pplayer->m_pin3d.ExpandExtents(&m_d.boundRectangle, objMesh, NULL, NULL, numVertices, fFalse);

   Vertex3D_NoTex2 *buf;
   vertexBuffer->lock(0,0,(void**)&buf, VertexBuffer::WRITEONLY | VertexBuffer::NOOVERWRITE );
   memcpy( buf, objMesh, sizeof(Vertex3D_NoTex2)*numVertices );
   vertexBuffer->unlock();
}

void Primitive::RenderObject( RenderDevice *pd3dDevice )
{
   if (m_d.m_TopVisible)
   {
      Texture * const pin = m_ptable->GetImage(m_d.m_szImage);

      if (pin)
      {
         // OK, Top is visible, and we have a image
         //lets draw
         pin->CreateAlphaChannel();
         pin->Set( ePictureTexture );
         pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);
         g_pplayer->m_pin3d.EnableAlphaBlend(1,false);

         g_pplayer->m_pin3d.SetColorKeyEnabled(TRUE);
         pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);

         g_pplayer->m_pin3d.SetTextureFilter(ePictureTexture, TEXTURE_MODE_TRILINEAR);
      }
      else
      {
         if( vertexBufferRegenerate )
            material.setColor( 1.0f, m_d.m_TopColor );
      }

      material.set();
      if(vertexBufferRegenerate)
      {
         vertexBufferRegenerate = false;

         RecalculateMatrices();

         if( m_d.use3DMesh )
            UpdateMesh();
         else
            CalculateBuiltin();
      }

	  if ( !m_d.useLighting )
      {
         // disable lighting is a default setting
         // it could look odd if you switch lighting on on non mesh primitives
         pd3dDevice->SetRenderState( RenderDevice::LIGHTING, FALSE );
      }
      
	  if( m_d.use3DMesh )
         pd3dDevice->renderPrimitive( D3DPT_TRIANGLELIST, vertexBuffer, 0, numVertices, indexList, indexListSize, 0 );
      else
         pd3dDevice->renderPrimitive( D3DPT_TRIANGLELIST, vertexBuffer, 0, numVertices, builtin_indices, m_d.m_DrawTexturesInside ? 24*m_d.m_Sides : 12*m_d.m_Sides, 0 );

      if ( !m_d.useLighting )
         pd3dDevice->SetRenderState( RenderDevice::LIGHTING, TRUE );

	  if (pin)
         pin->Unset(ePictureTexture);
   }
}

// Always called each frame to render over everything else (along with alpha ramps)
void Primitive::PostRenderStatic(const RenderDevice* _pd3dDevice)
{
   if ( m_d.staticRendering )
      return;

   RenderDevice* pd3dDevice=(RenderDevice*)_pd3dDevice;
   RenderObject( pd3dDevice );
}

extern bool loadWavefrontObj( char *filename, bool flipTv, bool convertToLeftHanded );
extern Vertex3D_NoTex2 *GetVertices( int &numVertices );
extern WORD *GetIndexList( int &indexListSize );
extern void SaveOBJ( char *filename, Primitive *mesh );

void Primitive::RenderSetup( const RenderDevice* _pd3dDevice )
{
   RenderDevice* pd3dDevice=(RenderDevice*)_pd3dDevice;
   if( m_d.use3DMesh )
   {
	  if( !objMesh )
          objMesh = new Vertex3D_NoTex2[numVertices];         
   }
   else
      numVertices = m_d.m_Sides*4+2;

   if( !vertexBuffer )
   {
      pd3dDevice->createVertexBuffer( numVertices, 0, MY_D3DFVF_NOTEX2_VERTEX, &vertexBuffer );
      NumVideoBytes += numVertices*sizeof(Vertex3D_NoTex2); //!! never cleared up again here
   }

   if( !m_d.use3DMesh )
      CalculateBuiltinOriginal();

   g_pplayer->m_pin3d.ClearExtents(&m_d.boundRectangle,NULL,NULL);
}

void Primitive::RenderStatic(const RenderDevice* _pd3dDevice)
{
   if( m_d.staticRendering )
   {
      RenderDevice *pd3dDevice = (RenderDevice*)_pd3dDevice;
      RenderObject(pd3dDevice);
   }
}

void Primitive::RenderMovers(const RenderDevice* pd3dDevice)
{
}

//////////////////////////////
// Positioning
//////////////////////////////

void Primitive::SetObjectPos()
{
   g_pvp->SetObjectPosCur(m_d.m_vPosition.x, m_d.m_vPosition.y);
}

void Primitive::MoveOffset(const float dx, const float dy)
{
   m_d.m_vPosition.x += dx;
   m_d.m_vPosition.y += dy;

   m_ptable->SetDirtyDraw();
}

void Primitive::GetCenter(Vertex2D * const pv) const
{
   pv->x = m_d.m_vPosition.x;
   pv->y = m_d.m_vPosition.y;
}

void Primitive::PutCenter(const Vertex2D * const pv)
{
   m_d.m_vPosition.x = pv->x;
   m_d.m_vPosition.y = pv->y;

   m_ptable->SetDirtyDraw();
}

//////////////////////////////
// Save and Load
//////////////////////////////

HRESULT Primitive::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   BiffWriter bw(pstm, hcrypthash, hcryptkey);

#ifdef VBA
   bw.WriteInt(FID(PIID), ApcControl.ID());
#endif
   bw.WriteStruct(FID(VPOS), &m_d.m_vPosition, sizeof(Vertex3Ds));
   bw.WriteStruct(FID(VSIZ), &m_d.m_vSize, sizeof(Vertex3Ds));
   bw.WriteStruct(FID(AXSX), &m_d.m_vAxisScaleX, sizeof(Vertex3Ds));
   bw.WriteStruct(FID(AXSY), &m_d.m_vAxisScaleY, sizeof(Vertex3Ds));
   bw.WriteStruct(FID(AXSZ), &m_d.m_vAxisScaleZ, sizeof(Vertex3Ds));
   bw.WriteFloat(FID(RTV0), m_d.m_aRotAndTra[0]);
   bw.WriteFloat(FID(RTV1), m_d.m_aRotAndTra[1]);
   bw.WriteFloat(FID(RTV2), m_d.m_aRotAndTra[2]);
   bw.WriteFloat(FID(RTV3), m_d.m_aRotAndTra[3]);
   bw.WriteFloat(FID(RTV4), m_d.m_aRotAndTra[4]);
   bw.WriteFloat(FID(RTV5), m_d.m_aRotAndTra[5]);
   bw.WriteFloat(FID(RTV6), m_d.m_aRotAndTra[6]);
   bw.WriteFloat(FID(RTV7), m_d.m_aRotAndTra[7]);
   bw.WriteFloat(FID(RTV8), m_d.m_aRotAndTra[8]);
//    int iTmp = m_d.m_aRotAndTraTypes[0];
//    bw.WriteInt(FID(RTT0), iTmp);
//    iTmp = m_d.m_aRotAndTraTypes[1];
//    bw.WriteInt(FID(RTT1), iTmp);
//    iTmp = m_d.m_aRotAndTraTypes[2];
//    bw.WriteInt(FID(RTT2), iTmp);
//    iTmp = m_d.m_aRotAndTraTypes[3];
//    bw.WriteInt(FID(RTT3), iTmp);
//    iTmp = m_d.m_aRotAndTraTypes[4];
//    bw.WriteInt(FID(RTT4), iTmp);
//    iTmp = m_d.m_aRotAndTraTypes[5];
//    bw.WriteInt(FID(RTT5), iTmp);
//    iTmp = m_d.m_aRotAndTraTypes[6];
//    bw.WriteInt(FID(RTT6), iTmp);
//    iTmp = m_d.m_aRotAndTraTypes[7];
//    bw.WriteInt(FID(RTT7), iTmp);
//    iTmp = m_d.m_aRotAndTraTypes[8];
//    bw.WriteInt(FID(RTT8), iTmp);
   bw.WriteString(FID(IMAG), m_d.m_szImage);
   bw.WriteInt(FID(SIDS), m_d.m_Sides);
   bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);
   bw.WriteInt(FID(TCOL), m_d.m_TopColor);
   bw.WriteInt(FID(SCOL), m_d.m_SideColor);
   bw.WriteInt(FID(TVIS), (m_d.m_TopVisible) ? 1 : 0);
   bw.WriteInt(FID(DTXI), (m_d.m_DrawTexturesInside) ? 1 : 0);
   bw.WriteInt(FID(ENLI), (m_d.useLighting) ? 1 : 0);
   bw.WriteInt(FID(U3DM), (m_d.use3DMesh) ? 1 : 0 );
   bw.WriteInt(FID(STRE), (m_d.staticRendering) ? 1 : 0 );
   bw.WriteInt(FID(EVMP), (m_d.sphereMapping) ? 1 : 0 );
   if( m_d.use3DMesh )
   {
      bw.WriteString( FID(M3DN), m_d.meshFileName);
      bw.WriteInt( FID(M3VN), numVertices );
      bw.WriteStruct( FID(M3DX), objMeshOrg, sizeof(Vertex3D_NoTex2)*numVertices);
      bw.WriteInt( FID(M3FN), indexListSize );
      bw.WriteStruct( FID(M3DI), indexList, sizeof(WORD)*indexListSize );
   }

   ISelect::SaveData(pstm, hcrypthash, hcryptkey);

   bw.WriteTag(FID(ENDB));

   return S_OK;
}

HRESULT Primitive::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   SetDefaults(false);

   BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);

   m_ptable = ptable;

   br.Load();
   return S_OK;
}

BOOL Primitive::LoadToken(int id, BiffReader *pbr)
{
   if (id == FID(PIID))
   {
      pbr->GetInt((int *)pbr->m_pdata);
   }
   else if (id == FID(VPOS))
   {
      pbr->GetStruct(&m_d.m_vPosition, sizeof(Vertex3Ds));
   }
   else if (id == FID(VSIZ))
   {
      pbr->GetStruct(&m_d.m_vSize, sizeof(Vertex3Ds));
   }
   else if (id == FID(AXSX))
   {
      pbr->GetStruct(&m_d.m_vAxisScaleX, sizeof(Vertex3Ds));
   }
   else if (id == FID(AXSY))
   {
      pbr->GetStruct(&m_d.m_vAxisScaleY, sizeof(Vertex3Ds));
   }
   else if (id == FID(AXSZ))
   {
      pbr->GetStruct(&m_d.m_vAxisScaleZ, sizeof(Vertex3Ds));
   }
   else if (id == FID(RTV0))
   {
      pbr->GetFloat(&m_d.m_aRotAndTra[0]);
   }
   else if (id == FID(RTV1))
   {
      pbr->GetFloat(&m_d.m_aRotAndTra[1]);
   }
   else if (id == FID(RTV2))
   {
      pbr->GetFloat(&m_d.m_aRotAndTra[2]);
   }
   else if (id == FID(RTV3))
   {
      pbr->GetFloat(&m_d.m_aRotAndTra[3]);
   }
   else if (id == FID(RTV4))
   {
      pbr->GetFloat(&m_d.m_aRotAndTra[4]);
   }
   else if (id == FID(RTV5))
   {
      pbr->GetFloat(&m_d.m_aRotAndTra[5]);
   }
   else if (id == FID(RTV6))
   {
      pbr->GetFloat(&m_d.m_aRotAndTra[6]);
   }
   else if (id == FID(RTV7))
   {
      pbr->GetFloat(&m_d.m_aRotAndTra[7]);
   }
   else if (id == FID(RTV8))
   {
      pbr->GetFloat(&m_d.m_aRotAndTra[8]);
   }
   else if (id == FID(IMAG))
   {
      pbr->GetString(m_d.m_szImage);
   }
   else if (id == FID(SIDS))
   {
      pbr->GetInt(&m_d.m_Sides);
   }
   else if (id == FID(NAME))
   {
      pbr->GetWideString((WCHAR *)m_wzName);
   }
   else if (id == FID(TCOL))
   {
      pbr->GetInt(&m_d.m_TopColor);
   }
   else if (id == FID(SCOL))
   {
      pbr->GetInt(&m_d.m_SideColor);
   }
   else if (id == FID(TVIS))
   {
      int iTmp;
      pbr->GetInt(&iTmp);
      m_d.m_TopVisible = (iTmp==1);
   }
   else if (id == FID(DTXI))
   {
      int iTmp;
      pbr->GetInt(&iTmp);
      m_d.m_DrawTexturesInside = (iTmp==1);
   }
   else if (id == FID(ENLI))
   {
      int iTmp;
      pbr->GetInt(&iTmp);
      m_d.useLighting = (iTmp==1);
   }
   else if (id == FID(STRE))
   {
      int iTmp;
      pbr->GetInt(&iTmp);
      m_d.staticRendering = (iTmp==1);
   }
   else if (id == FID(EVMP))
   {
      int iTmp;
      pbr->GetInt(&iTmp);
      m_d.sphereMapping = (iTmp==1);
   }
   else if ( id == FID(U3DM))
   {
      int iTmp;
      pbr->GetInt(&iTmp);
      m_d.use3DMesh = (iTmp==1);
   }
   else if ( id == FID(M3DN))
   {
      pbr->GetWideString((WCHAR *)m_d.meshFileName);
   }
   else if( id == FID(M3VN) )
   {
      pbr->GetInt( &numVertices );
   }
   else if( id == FID(M3DX) )
   {
      if( objMesh )
      {
         delete[] objMesh;
		 objMesh = 0;
      }
      if( objMeshOrg )
      {
         delete[] objMeshOrg;
      }
      objMeshOrg = new Vertex3D_NoTex2[numVertices];
      pbr->GetStruct( objMeshOrg, sizeof(Vertex3D_NoTex2)*numVertices);
   }
   else if( id == FID(M3FN) )
   {
      pbr->GetInt( &indexListSize );
   }
   else if( id == FID(M3DI) )
   {
      if( indexList )
      {
         delete[] indexList;
      }
      indexList = new WORD[indexListSize];
      pbr->GetStruct( indexList, sizeof(WORD)*indexListSize);
   }
   else
   {
      ISelect::LoadToken(id, pbr);
   }

   return fTrue;
}

HRESULT Primitive::InitPostLoad()
{
   return S_OK;
}

bool Primitive::BrowseFor3DMeshFile()
{
   char szFileName[1024];
   char szInitialDir[1024];
   szFileName[0] = '\0';

   OPENFILENAME ofn;
   ZeroMemory(&ofn, sizeof(OPENFILENAME));
   ofn.lStructSize = sizeof(OPENFILENAME);
   ofn.hInstance = g_hinst;
   ofn.hwndOwner = g_pvp->m_hwnd;
   // TEXT
   ofn.lpstrFilter = "Wavefront obj file (*.obj)\0*.obj\0";
   ofn.lpstrFile = szFileName;
   ofn.nMaxFile = _MAX_PATH;
   ofn.lpstrDefExt = "obj";
   ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

   const HRESULT hr = GetRegString("RecentDir","LoadDir", szInitialDir, 1024);
   if (hr == S_OK)
   {
      ofn.lpstrInitialDir = szInitialDir;
   }
   else
   {
      char szFoo[MAX_PATH];
      lstrcpy(szFoo, "c:\\");
      ofn.lpstrInitialDir = szFoo;
   }

#ifdef VBA
   ApcHost->BeginModalDialog();
#endif
   const int ret = GetOpenFileName(&ofn);
#ifdef VBA
   ApcHost->EndModalDialog();
#endif
   string filename(ofn.lpstrFile);
   int index = filename.find_last_of("\\");
   if( index!=-1 )
   {
      index++;
      string name = filename.substr(index, filename.length()-index);
      strcpy_s( m_d.meshFileName, name.c_str());
   }
   if(ret == 0)
   {
      return false;
   }
   if( objMeshOrg )
   {
      delete[] objMeshOrg;
	  objMeshOrg = 0;
   }
   if( objMesh )
   {
      delete[] objMesh;
	  objMesh = 0;
   }
   numVertices=0;
   if( indexList )
   {
      delete[] indexList;
	  indexList = 0;
   }
   indexListSize=0;
   m_d.use3DMesh=false;
   if( vertexBuffer )
   {
      vertexBuffer->release();
      vertexBuffer=0;
   }
   bool flipTV=false;
   bool convertToLeftHanded=false;
   int ans = MessageBox(g_pvp->m_hwnd, "Do you want to mirror the object?", "Convert coordinate system?", MB_YESNO | MB_DEFBUTTON2);
   if (ans == IDYES)
   {
      convertToLeftHanded=true;
   }
   else
   {
      ans = MessageBox(g_pvp->m_hwnd, "Do you want to convert texture coordinates?", "Confirm", MB_YESNO | MB_DEFBUTTON2);
      if (ans == IDYES)
      {
         flipTV=true;
      }
   }
   if ( loadWavefrontObj(ofn.lpstrFile, flipTV, convertToLeftHanded) )
   {
      m_d.use3DMesh=true;
      objMeshOrg = GetVertices( numVertices );
      indexList = GetIndexList( indexListSize );
      return true;
   }
   return false;
}

//////////////////////////////
// Standard methods
//////////////////////////////

STDMETHODIMP Primitive::get_Image(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_szImage, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Primitive::put_Image(BSTR newVal)
{
   STARTUNDO

   WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szImage, 32, NULL, NULL);
   
   STOPUNDO

   return S_OK;
}

STDMETHODIMP Primitive::get_MeshFileName(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.meshFileName, -1, wz, 256);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Primitive::put_MeshFileName(BSTR newVal)
{
   STARTUNDO

   WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.meshFileName, 256, NULL, NULL);
   vertexBufferRegenerate = true;

   STOPUNDO
   return S_OK;
}

bool Primitive::LoadMesh()
{
   bool result=false;
   STARTUNDO

   result = BrowseFor3DMeshFile();
   vertexBufferRegenerate = true;

   STOPUNDO

   return result;
}

void Primitive::ExportMesh()
{
   char szFileName[1024];
   char szInitialDir[1024];
   szFileName[0] = '\0';

   if ( !m_d.use3DMesh )
   {
      ShowError("This primitive isn't a 3D Mesh!");
      return;
   }
   OPENFILENAME ofn;
   ZeroMemory(&ofn, sizeof(OPENFILENAME));
   ofn.lStructSize = sizeof(OPENFILENAME);
   ofn.hInstance = g_hinst;
   ofn.hwndOwner = g_pvp->m_hwnd;
   // TEXT
   ofn.lpstrFilter = "Wavefront obj file (*.obj)\0*.obj\0";
   ofn.lpstrFile = szFileName;
   ofn.nMaxFile = _MAX_PATH;
   ofn.lpstrDefExt = "obj";
   ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

   const HRESULT hr = GetRegString("RecentDir","LoadDir", szInitialDir, 1024);
   if (hr == S_OK)
   {
      ofn.lpstrInitialDir = szInitialDir;
   }
   else
   {
      char szFoo[MAX_PATH];
      lstrcpy(szFoo, "c:\\");
      ofn.lpstrInitialDir = szFoo;
   }

#ifdef VBA
   ApcHost->BeginModalDialog();
#endif
   const int ret = GetSaveFileName(&ofn);
#ifdef VBA
   ApcHost->EndModalDialog();
#endif
   if(ret == 0)
   {
      return;
   }
   SaveOBJ( ofn.lpstrFile, this );
}

STDMETHODIMP Primitive::get_Sides(int *pVal)
{
   *pVal = m_d.m_Sides;

   return S_OK;
}

STDMETHODIMP Primitive::put_Sides(int newVal)
{
   if ((newVal <= Max_Primitive_Sides) && (m_d.m_Sides != newVal))
   {
      STARTUNDO

      m_d.m_Sides = newVal;
      vertexBufferRegenerate = true;
      RecalculateMatrices();
	  RecalculateVertices();

      STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Primitive::get_TopColor(OLE_COLOR *pVal)
{
   *pVal = m_d.m_TopColor;

   return S_OK;
}

STDMETHODIMP Primitive::put_TopColor(OLE_COLOR newVal)
{
   if(m_d.m_TopColor != newVal)
   {
	   STARTUNDO

	   m_d.m_TopColor = newVal;
	   vertexBufferRegenerate = true;

	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Primitive::get_SideColor(OLE_COLOR *pVal)
{
   *pVal = m_d.m_SideColor;

   return S_OK;
}

STDMETHODIMP Primitive::put_SideColor(OLE_COLOR newVal)
{
   if( m_d.m_SideColor != newVal)
   {
	   STARTUNDO

	   m_d.m_SideColor = newVal;
	   vertexBufferRegenerate = true;

	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Primitive::get_TopVisible(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_TopVisible);

   return S_OK;
}

STDMETHODIMP Primitive::put_TopVisible(VARIANT_BOOL newVal)
{
   STARTUNDO

   if( m_d.m_TopVisible && !VBTOF(newVal) )
      m_d.wasVisible=true;
   m_d.m_TopVisible = VBTOF(newVal);
   
   STOPUNDO
   return S_OK;
}

STDMETHODIMP Primitive::get_DrawTexturesInside(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_DrawTexturesInside);

   return S_OK;
}

STDMETHODIMP Primitive::put_DrawTexturesInside(VARIANT_BOOL newVal)
{
   if( m_d.m_DrawTexturesInside != VBTOF(newVal))
   {
	   STARTUNDO

	   m_d.m_DrawTexturesInside = VBTOF(newVal);
	   vertexBufferRegenerate = true;

	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Primitive::get_Z(float *pVal)
{
   *pVal = m_d.m_vPosition.z;

   return S_OK;
}

STDMETHODIMP Primitive::put_Z(float newVal)
{
   if(m_d.m_vPosition.z != newVal)
   {
	   STARTUNDO

	   m_d.m_vPosition.z = newVal;
	   vertexBufferRegenerate = true;

	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Primitive::get_X(float *pVal)
{
   *pVal = m_d.m_vPosition.x;

   return S_OK;
}

STDMETHODIMP Primitive::put_X(float newVal)
{
   if(m_d.m_vPosition.x != newVal)
   {
	   STARTUNDO

	   m_d.m_vPosition.x = newVal;
	   vertexBufferRegenerate = true;

	   STOPUNDO
   }
   return S_OK;
}

STDMETHODIMP Primitive::get_Y(float *pVal)
{
   *pVal = m_d.m_vPosition.y;

   return S_OK;
}

STDMETHODIMP Primitive::put_Y(float newVal)
{
   if(m_d.m_vPosition.y != newVal)
   {
	   STARTUNDO

	   m_d.m_vPosition.y = newVal;
	   vertexBufferRegenerate = true;

	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Primitive::get_Size_X(float *pVal)
{
   *pVal = m_d.m_vSize.x;

   return S_OK;
}

STDMETHODIMP Primitive::put_Size_X(float newVal)
{
   if(m_d.m_vSize.x != newVal)
   {
	   STARTUNDO

	   m_d.m_vSize.x = newVal;
	   vertexBufferRegenerate = true;

	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Primitive::get_Size_Y(float *pVal)
{
   *pVal = m_d.m_vSize.y;

   return S_OK;
}

STDMETHODIMP Primitive::put_Size_Y(float newVal)
{
   if(m_d.m_vSize.y != newVal)
   {
	   STARTUNDO

	   m_d.m_vSize.y = newVal;
	   vertexBufferRegenerate = true;

	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Primitive::get_Size_Z(float *pVal)
{
   *pVal = m_d.m_vSize.z;

   return S_OK;
}

STDMETHODIMP Primitive::put_Size_Z(float newVal)
{
   if(m_d.m_vSize.z != newVal)
   {
	   STARTUNDO

	   m_d.m_vSize.z = newVal;
	   vertexBufferRegenerate = true;

	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Primitive::get_AxisScaleX_Y(float *pVal)
{
   *pVal = m_d.m_vAxisScaleX.y;

   return S_OK;
}

STDMETHODIMP Primitive::put_AxisScaleX_Y(float newVal)
{
   if(m_d.m_vAxisScaleX.y != newVal)
   {
	   STARTUNDO

	   m_d.m_vAxisScaleX.y = newVal;
	   vertexBufferRegenerate = true;

	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Primitive::get_AxisScaleX_Z(float *pVal)
{
   *pVal = m_d.m_vAxisScaleX.z;

   return S_OK;
}

STDMETHODIMP Primitive::put_AxisScaleX_Z(float newVal)
{
   if(m_d.m_vAxisScaleX.z != newVal)
   {
	   STARTUNDO

	   m_d.m_vAxisScaleX.z = newVal;
	   vertexBufferRegenerate = true;

	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Primitive::get_AxisScaleY_X(float *pVal)
{
   *pVal = m_d.m_vAxisScaleY.x;

   return S_OK;
}

STDMETHODIMP Primitive::put_AxisScaleY_X(float newVal)
{
   if(m_d.m_vAxisScaleY.x != newVal)
   {
	   STARTUNDO

	   m_d.m_vAxisScaleY.x = newVal;
	   vertexBufferRegenerate = true;

	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Primitive::get_AxisScaleY_Z(float *pVal)
{
   *pVal = m_d.m_vAxisScaleY.z;

   return S_OK;
}

STDMETHODIMP Primitive::put_AxisScaleY_Z(float newVal)
{
   if(m_d.m_vAxisScaleY.z != newVal)
   {
	   STARTUNDO

	   m_d.m_vAxisScaleY.z = newVal;
	   vertexBufferRegenerate = true;

	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Primitive::get_AxisScaleZ_X(float *pVal)
{
   *pVal = m_d.m_vAxisScaleZ.x;

   return S_OK;
}

STDMETHODIMP Primitive::put_AxisScaleZ_X(float newVal)
{
   if(m_d.m_vAxisScaleZ.x != newVal)
   {
	   STARTUNDO

	   m_d.m_vAxisScaleZ.x = newVal;
	   vertexBufferRegenerate = true;

	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Primitive::get_AxisScaleZ_Y(float *pVal)
{
   *pVal = m_d.m_vAxisScaleZ.y;

   return S_OK;
}

STDMETHODIMP Primitive::put_AxisScaleZ_Y(float newVal)
{
   if(m_d.m_vAxisScaleZ.y != newVal)
   {
	   STARTUNDO

	   m_d.m_vAxisScaleZ.y = newVal;
	   vertexBufferRegenerate = true;

	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Primitive::get_RotAndTra0(float *pVal)
{
   *pVal = m_d.m_aRotAndTra[0];
   return S_OK;
}

STDMETHODIMP Primitive::put_RotAndTra0(float newVal)
{
   if(m_d.m_aRotAndTra[0] != newVal)
   {
	   STARTUNDO

	   m_d.m_aRotAndTra[0] = newVal;
	   vertexBufferRegenerate = true;

	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Primitive::get_RotX(float *pVal)
{
   *pVal = m_d.m_aRotAndTra[0];
   return S_OK;
}

STDMETHODIMP Primitive::put_RotX(float newVal)
{
   if(m_d.m_aRotAndTra[0] != newVal)
   {
	   STARTUNDO

	   m_d.m_aRotAndTra[0] = newVal;
	   vertexBufferRegenerate = true;

	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Primitive::get_RotAndTra1(float *pVal)
{
   *pVal = m_d.m_aRotAndTra[1];

   return S_OK;
}

STDMETHODIMP Primitive::put_RotAndTra1(float newVal)
{
   if(m_d.m_aRotAndTra[1] != newVal)
   {
	   STARTUNDO

	   m_d.m_aRotAndTra[1] = newVal;
	   vertexBufferRegenerate = true;

	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Primitive::get_RotY(float *pVal)
{
   *pVal = m_d.m_aRotAndTra[1];

   return S_OK;
}

STDMETHODIMP Primitive::put_RotY(float newVal)
{
   if(m_d.m_aRotAndTra[1] != newVal)
   {
	   STARTUNDO

	   m_d.m_aRotAndTra[1] = newVal;
	   vertexBufferRegenerate = true;

	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Primitive::get_RotAndTra2(float *pVal)
{
   *pVal = m_d.m_aRotAndTra[2];

   return S_OK;
}

STDMETHODIMP Primitive::put_RotAndTra2(float newVal)
{
   if(m_d.m_aRotAndTra[2] != newVal)
   {
	   STARTUNDO

	   m_d.m_aRotAndTra[2] = newVal;
	   vertexBufferRegenerate = true;

	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Primitive::get_RotZ(float *pVal)
{
   *pVal = m_d.m_aRotAndTra[2];

   return S_OK;
}

STDMETHODIMP Primitive::put_RotZ(float newVal)
{
   if(m_d.m_aRotAndTra[2] != newVal)
   {
	   STARTUNDO

	   m_d.m_aRotAndTra[2] = newVal;
	   vertexBufferRegenerate = true;

	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Primitive::get_RotAndTra3(float *pVal)
{
   *pVal = m_d.m_aRotAndTra[3];

   return S_OK;
}

STDMETHODIMP Primitive::put_RotAndTra3(float newVal)
{
   if(m_d.m_aRotAndTra[3] != newVal)
   {
	   STARTUNDO

	   m_d.m_aRotAndTra[3] = newVal;
	   vertexBufferRegenerate = true;

	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Primitive::get_TransX(float *pVal)
{
   *pVal = m_d.m_aRotAndTra[3];

   return S_OK;
}

STDMETHODIMP Primitive::put_TransX(float newVal)
{
   if(m_d.m_aRotAndTra[3] != newVal)
   {
	   STARTUNDO

	   m_d.m_aRotAndTra[3] = newVal;
	   vertexBufferRegenerate = true;

	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Primitive::get_RotAndTra4(float *pVal)
{
   *pVal = m_d.m_aRotAndTra[4];

   return S_OK;
}

STDMETHODIMP Primitive::put_RotAndTra4(float newVal)
{
   if(m_d.m_aRotAndTra[4] != newVal)
   {
	   STARTUNDO

	   m_d.m_aRotAndTra[4] = newVal;
	   vertexBufferRegenerate = true;

	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Primitive::get_TransY(float *pVal)
{
   *pVal = m_d.m_aRotAndTra[4];

   return S_OK;
}

STDMETHODIMP Primitive::put_TransY(float newVal)
{
   if(m_d.m_aRotAndTra[4] != newVal)
   {
	   STARTUNDO

	   m_d.m_aRotAndTra[4] = newVal;
	   vertexBufferRegenerate = true;

	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Primitive::get_RotAndTra5(float *pVal)
{
   *pVal = m_d.m_aRotAndTra[5];

   return S_OK;
}

STDMETHODIMP Primitive::put_RotAndTra5(float newVal)
{
   if(m_d.m_aRotAndTra[5] != newVal)
   {
	   STARTUNDO

	   m_d.m_aRotAndTra[5] = newVal;
	   vertexBufferRegenerate = true;

	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Primitive::get_TransZ(float *pVal)
{
   *pVal = m_d.m_aRotAndTra[5];

   return S_OK;
}

STDMETHODIMP Primitive::put_TransZ(float newVal)
{
   if(m_d.m_aRotAndTra[5] != newVal)
   {
	   STARTUNDO

	   m_d.m_aRotAndTra[5] = newVal;
	   vertexBufferRegenerate = true;

	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Primitive::get_RotAndTra6(float *pVal)
{
   *pVal = m_d.m_aRotAndTra[6];

   return S_OK;
}

STDMETHODIMP Primitive::put_RotAndTra6(float newVal)
{
   if(m_d.m_aRotAndTra[6] != newVal)
   {
	   STARTUNDO

	   m_d.m_aRotAndTra[6] = newVal;
	   vertexBufferRegenerate = true;

	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Primitive::get_ObjRotX(float *pVal)
{
   *pVal = m_d.m_aRotAndTra[6];

   return S_OK;
}

STDMETHODIMP Primitive::put_ObjRotX(float newVal)
{
   if(m_d.m_aRotAndTra[6] != newVal)
   {
	   STARTUNDO

	   m_d.m_aRotAndTra[6] = newVal;
	   vertexBufferRegenerate = true;

	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Primitive::get_RotAndTra7(float *pVal)
{
   *pVal = m_d.m_aRotAndTra[7];

   return S_OK;
}

STDMETHODIMP Primitive::put_RotAndTra7(float newVal)
{
   if(m_d.m_aRotAndTra[7] != newVal)
   {
	   STARTUNDO

	   m_d.m_aRotAndTra[7] = newVal;
	   vertexBufferRegenerate = true;

	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Primitive::get_ObjRotY(float *pVal)
{
   *pVal = m_d.m_aRotAndTra[7];

   return S_OK;
}

STDMETHODIMP Primitive::put_ObjRotY(float newVal)
{
   if(m_d.m_aRotAndTra[7] != newVal)
   {
	   STARTUNDO

	   m_d.m_aRotAndTra[7] = newVal;
	   vertexBufferRegenerate = true;

	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Primitive::get_RotAndTra8(float *pVal)
{
   *pVal = m_d.m_aRotAndTra[8];

   return S_OK;
}

STDMETHODIMP Primitive::put_RotAndTra8(float newVal)
{
   if(m_d.m_aRotAndTra[8] != newVal)
   {
	   STARTUNDO

	   m_d.m_aRotAndTra[8] = newVal;
	   vertexBufferRegenerate = true;

	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Primitive::get_ObjRotZ(float *pVal)
{
   *pVal = m_d.m_aRotAndTra[8];

   return S_OK;
}

STDMETHODIMP Primitive::put_ObjRotZ(float newVal)
{
   if(m_d.m_aRotAndTra[8] != newVal)
   {
	   STARTUNDO

	   m_d.m_aRotAndTra[8] = newVal;
	   vertexBufferRegenerate = true;

	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Primitive::get_EnableLighting(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.useLighting);

   return S_OK;
}

STDMETHODIMP Primitive::put_EnableLighting(VARIANT_BOOL newVal)
{
   STARTUNDO

   m_d.useLighting = VBTOF(newVal);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Primitive::get_EnableStaticRendering(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.staticRendering);

   return S_OK;
}

STDMETHODIMP Primitive::put_EnableStaticRendering(VARIANT_BOOL newVal)
{
   STARTUNDO

   m_d.staticRendering = VBTOF(newVal);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Primitive::get_EnableSphereMapping(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.sphereMapping);

   return S_OK;
}

STDMETHODIMP Primitive::put_EnableSphereMapping(VARIANT_BOOL newVal)
{
   STARTUNDO

   m_d.sphereMapping = VBTOF(newVal);

   STOPUNDO

   return S_OK;
}

void Primitive::GetDialogPanes(Vector<PropertyPane> *pvproppane)
{
   PropertyPane *pproppane;

   pproppane = new PropertyPane(IDD_PROP_NAME, NULL);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPPRIMITIVE_VISUALS, IDS_VISUALS);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPPRIMITIVE_POSITION, IDS_POSITION);
   pvproppane->AddElement(pproppane);
}
