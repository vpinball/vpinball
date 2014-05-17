// primitive.cpp: implementation of the Primitive class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h" 

Primitive::Primitive()
{
   vertexBuffer = 0;
   vertexBufferRegenerate = true;
   indexBuffer = 0;
   m_d.use3DMesh=false;
   m_d.meshFileName[0]=0;
   m_d.useLighting=false;
   m_d.staticRendering=false;
   m_d.sphereMapping=false;
   m_d.m_depthBias = 0.0f;
   numIndices = 0;
   numVertices = 0;
} 

Primitive::~Primitive() 
{
    if(vertexBuffer)
    {
        vertexBuffer->release();
        vertexBuffer = 0;
    }
    if (indexBuffer)
        indexBuffer->release();
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

   hr = GetRegStringAsFloat("DefaultProps\\Primitive","HitThreshold", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_threshold = fTmp;
   else
      m_d.m_threshold = 2.0f;

   hr = GetRegStringAsFloat("DefaultProps\\Primitive","Elasticity", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_elasticity = fTmp;
   else
      m_d.m_elasticity = 0.3f;

   hr = GetRegStringAsFloat("DefaultProps\\Primitive","Friction", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_friction = fTmp;
   else
      m_d.m_friction = 0;	//zero uses global value

   hr = GetRegStringAsFloat("DefaultProps\\Primitive","Scatter", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_scatter = fTmp;
   else
      m_d.m_scatter = 0;	//zero uses global value

   hr = GetRegInt("DefaultProps\\Primitive","Collidable", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fCollidable = iTmp == 0 ? false : true;
   else
      m_d.m_fCollidable = false;
   hr = GetRegInt("DefaultProps\\Primitive","IsToy", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fToy= iTmp == 0 ? false : true;
   else
      m_d.m_fToy = true;
}

void Primitive::WriteRegDefaults()
{
   SetRegValue("DefaultProps\\Primitive","TopColor",REG_DWORD,&m_d.m_TopColor,4);
   SetRegValue("DefaultProps\\Primitive","SideColor",REG_DWORD,&m_d.m_SideColor,4);
   SetRegValueBool("DefaultProps\\Primitive","TopVisible", m_d.m_TopVisible);
   SetRegValueBool("DefaultProps\\Primitive","UseLighting", m_d.useLighting);
   SetRegValueBool("DefaultProps\\Primitive","StaticRendering", m_d.staticRendering);
   SetRegValueBool("DefaultProps\\Primitive","SphereMapping", m_d.sphereMapping);
   SetRegValueBool("DefaultProps\\Primitive","DrawTexturesInside", m_d.m_DrawTexturesInside);

   SetRegValueFloat("DefaultProps\\Primitive","Position_Z", m_d.m_vPosition.z);

   SetRegValueFloat("DefaultProps\\Primitive","Size_X", m_d.m_vSize.x);
   SetRegValueFloat("DefaultProps\\Primitive","Size_Y", m_d.m_vSize.y);
   SetRegValueFloat("DefaultProps\\Primitive","Size_Z", m_d.m_vSize.z);

   SetRegValueFloat("DefaultProps\\Primitive","AxisScaleXY", m_d.m_vAxisScaleX.y);
   SetRegValueFloat("DefaultProps\\Primitive","AxisScaleXZ", m_d.m_vAxisScaleX.z);
   SetRegValueFloat("DefaultProps\\Primitive","AxisScaleYX", m_d.m_vAxisScaleY.x);
   SetRegValueFloat("DefaultProps\\Primitive","AxisScaleYZ", m_d.m_vAxisScaleY.z);
   SetRegValueFloat("DefaultProps\\Primitive","AxisScaleZX", m_d.m_vAxisScaleZ.x);
   SetRegValueFloat("DefaultProps\\Primitive","AxisScaleZY", m_d.m_vAxisScaleZ.y);

   SetRegValueFloat("DefaultProps\\Primitive","RotAndTra0", m_d.m_aRotAndTra[0]);
   SetRegValueFloat("DefaultProps\\Primitive","RotAndTra1", m_d.m_aRotAndTra[1]);
   SetRegValueFloat("DefaultProps\\Primitive","RotAndTra2", m_d.m_aRotAndTra[2]);
   SetRegValueFloat("DefaultProps\\Primitive","RotAndTra3", m_d.m_aRotAndTra[3]);
   SetRegValueFloat("DefaultProps\\Primitive","RotAndTra4", m_d.m_aRotAndTra[4]);
   SetRegValueFloat("DefaultProps\\Primitive","RotAndTra5", m_d.m_aRotAndTra[5]);
   SetRegValueFloat("DefaultProps\\Primitive","RotAndTra6", m_d.m_aRotAndTra[6]);
   SetRegValueFloat("DefaultProps\\Primitive","RotAndTra7", m_d.m_aRotAndTra[7]);
   SetRegValueFloat("DefaultProps\\Primitive","RotAndTra8", m_d.m_aRotAndTra[8]);
   /*
   SetRegValueFloat("DefaultProps\\Primitive","Rotation_X", m_d.m_vRotation.x);
   SetRegValueFloat("DefaultProps\\Primitive","Rotation_Y", m_d.m_vRotation.y);
   SetRegValueFloat("DefaultProps\\Primitive","Rotation_Z", m_d.m_vRotation.z);

   SetRegValueFloat("DefaultProps\\Primitive","Transposition_X", m_d.m_vTransposition.x);
   SetRegValueFloat("DefaultProps\\Primitive","Transposition_Y", m_d.m_vTransposition.y);
   SetRegValueFloat("DefaultProps\\Primitive","Transposition_Z", m_d.m_vTransposition.z);
   */
   SetRegValue("DefaultProps\\Primitive","Image", REG_SZ, &m_d.m_szImage,strlen(m_d.m_szImage));
   SetRegValueBool("DefaultProps\\Primitive","HitEvent", !!m_d.m_fHitEvent);
   SetRegValueFloat("DefaultProps\\Primitive","HitThreshold", m_d.m_threshold);
   SetRegValueFloat("DefaultProps\\Primitive","Elasticity", m_d.m_elasticity);
   SetRegValueFloat("DefaultProps\\Primitive","Friction", m_d.m_friction);
   SetRegValueFloat("DefaultProps\\Primitive","Scatter", m_d.m_scatter);
   SetRegValueBool("DefaultProps\\Primitive","Collidable", m_d.m_fCollidable);
   SetRegValueBool("DefaultProps\\Primitive","IsToy", m_d.m_fToy);
}

void Primitive::GetTimers(Vector<HitTimer> * const pvht)
{
   IEditable::BeginPlay();
}

void Primitive::GetHitShapes(Vector<HitObject> * const pvho)
{
   if( !m_d.use3DMesh || m_d.m_fToy )
      return;

   RecalculateMatrices();
   RecalculateVertices();
   HitTriangle *ph3dpolyOld = NULL;

   for( unsigned i=0; i<indexList.size(); i+=3 )
   {
      Vertex3Ds rgv3D[3];
      rgv3D[0] = vertices[ indexList[i  ] ];
      rgv3D[1] = vertices[ indexList[i+1] ];
      rgv3D[2] = vertices[ indexList[i+2] ];
      HitTriangle * const ph3dpoly = new HitTriangle(rgv3D); //!! this is not efficient at all, use native triangle-soup directly somehow
      ph3dpoly->m_elasticity = m_d.m_elasticity;
      ph3dpoly->SetFriction(m_d.m_friction);
      ph3dpoly->m_scatter = ANGTORAD(m_d.m_scatter);
      ph3dpoly->m_fVisible=fTrue;
      ph3dpoly->m_threshold = m_d.m_threshold;
      ph3dpoly->m_fEnabled=m_d.m_fCollidable;
      ph3dpoly->m_ObjType = ePrimitive;
      if ( m_d.m_fHitEvent )
         ph3dpoly->m_pfe = (IFireEvents *)this;
      else  
         ph3dpoly->m_pfe = 0;
      pvho->AddElement( ph3dpoly );

      if (ph3dpolyOld)
        CheckJoint(pvho, ph3dpolyOld, ph3dpoly);

      ph3dpolyOld = ph3dpoly;
   }
}

void Primitive::GetHitShapesDebug(Vector<HitObject> * const pvho)
{
}

void Primitive::CheckJoint(Vector<HitObject> * const pvho, const HitTriangle * const ph3d1, const HitTriangle * const ph3d2)
{
   Vertex3Ds vjointnormal = CrossProduct(ph3d1->normal, ph3d2->normal);
   //vjointnormal.x = ph3d1->normal.x + ph3d2->normal.x;
   //vjointnormal.y = ph3d1->normal.y + ph3d2->normal.y;
   //vjointnormal.z = ph3d1->normal.z + ph3d2->normal.z;

   const float sqrlength = vjointnormal.x * vjointnormal.x + vjointnormal.y * vjointnormal.y + vjointnormal.z * vjointnormal.z;
   if (sqrlength < 1.0e-8f) return;

   const float inv_length = 1.0f/sqrtf(sqrlength);
   vjointnormal.x *= inv_length;
   vjointnormal.y *= inv_length;
   vjointnormal.z *= inv_length;

   // By convention of the calling function, points 1 [0] and 2 [1] of the second polygon will
   // be the common-edge points

   Hit3DCylinder * const ph3dc = new Hit3DCylinder(&ph3d2->m_rgv[0], &ph3d2->m_rgv[1], &vjointnormal);
   ph3dc->m_elasticity = m_d.m_elasticity;
   ph3dc->SetFriction(m_d.m_friction);
   ph3dc->m_scatter = ANGTORAD(m_d.m_scatter);
   ph3dc->m_threshold = m_d.m_threshold;
   ph3dc->m_ObjType = ePrimitive;
   if ( m_d.m_fHitEvent )
      ph3dc->m_pfe = (IFireEvents *)this;
   else
      ph3dc->m_pfe = 0;
   pvho->AddElement(ph3dc);

   m_vhoCollidable.AddElement(ph3dc);	//remember hit components of primitive
   ph3dc->m_fEnabled = m_d.m_fCollidable;
}

void Primitive::EndPlay()
{
   m_vhoCollidable.RemoveAllElements();
	if(vertexBuffer)
	{
		vertexBuffer->release();
		vertexBuffer = 0;
		vertexBufferRegenerate = true;
	}
    if (indexBuffer)
    {
        indexBuffer->release();
        indexBuffer = 0;
    }

    objMesh.clear();
}

//////////////////////////////
// Calculation
//////////////////////////////

void Primitive::RecalculateMatrices()
{
   // scale matrix
   Matrix3D Smatrix;
   Smatrix.SetScaling( m_d.m_vSize.x, m_d.m_vSize.y, m_d.m_vSize.z );

   // translation matrix
   Matrix3D Tmatrix;
   Tmatrix.SetTranslation(m_d.m_vPosition.x, m_d.m_vPosition.y, m_d.m_vPosition.z);

   // translation + rotation matrix
   Matrix3D RTmatrix;
   RTmatrix.SetTranslation( m_d.m_aRotAndTra[3], m_d.m_aRotAndTra[4], m_d.m_aRotAndTra[5]);

   Matrix3D tempMatrix;
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
   Tmatrix.Multiply(fullMatrix, fullMatrix);        // fullMatrix = Smatrix * RTmatrix * Tmatrix
}

// recalculate vertices for editor display
void Primitive::RecalculateVertices()
{
   vertices.clear();

   if( !m_d.use3DMesh )
   {
      vertices.resize(2 * m_d.m_Sides);

      const float outerRadius = -0.5f/cosf((float)M_PI/(float)m_d.m_Sides);
      const float addAngle = (float)(2.0*M_PI)/(float)m_d.m_Sides;
	  const float offsAngle = (float)M_PI/(float)m_d.m_Sides;
      for (int i = 0; i < m_d.m_Sides; ++i)
      {
         Vertex3Ds topVert, bottomVert;

		 const float currentAngle = addAngle*(float)i + offsAngle;
         topVert.x = sinf(currentAngle)*outerRadius;
         topVert.y = cosf(currentAngle)*outerRadius * (1.0f+(m_d.m_vAxisScaleX.y - 1.0f)*(topVert.x+0.5f));
         topVert.z =                           0.5f * (1.0f+(m_d.m_vAxisScaleX.z - 1.0f)*(topVert.x+0.5f));

         topVert.x *= 1.0f+(m_d.m_vAxisScaleY.x - 1.0f)*(topVert.y+0.5f);
         topVert.z *= 1.0f+(m_d.m_vAxisScaleY.z - 1.0f)*(topVert.y+0.5f);
         bottomVert.z = -topVert.z;

         const float tmp = topVert.x;
         topVert.x    = tmp * (1.0f+(m_d.m_vAxisScaleZ.x - 1.0f)*(topVert.z+0.5f));
         bottomVert.x = tmp * (1.0f+(m_d.m_vAxisScaleZ.x - 1.0f)*(0.5f-topVert.z));

         const float tmp2 = topVert.y;
         topVert.y    = tmp2 * (1.0f+(m_d.m_vAxisScaleZ.y - 1.0f)*(topVert.z+0.5f));
         bottomVert.y = tmp2 * (1.0f+(m_d.m_vAxisScaleZ.y - 1.0f)*(0.5f-topVert.z));

         fullMatrix.MultiplyVector(topVert, topVert);
         fullMatrix.MultiplyVector(bottomVert, bottomVert);
         vertices[i] = topVert;
         vertices[i + m_d.m_Sides] = bottomVert;
      }
   }
   else
   {
      vertices.resize( objMeshOrg.size() );
      for( unsigned i=0; i<objMeshOrg.size(); i++ )
      {
         fullMatrix.MultiplyVector(objMeshOrg[i], vertices[i]);
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
         const Vertex3Ds * const topVert = &vertices[i];
         const Vertex3Ds * const nextTopVert = &vertices[inext];
         psur->Line(topVert->x, topVert->y, nextTopVert->x, nextTopVert->y);
         const Vertex3Ds * const bottomVert = &vertices[i + m_d.m_Sides];
         const Vertex3Ds * const nextBottomVert = &vertices[inext + m_d.m_Sides];
         psur->Line(bottomVert->x, bottomVert->y, nextBottomVert->x, nextBottomVert->y);
         psur->Line(bottomVert->x, bottomVert->y, topVert->x, topVert->y);
      }
      psur->Line(m_d.m_vPosition.x -20.0f, m_d.m_vPosition.y,m_d.m_vPosition.x +20.0f, m_d.m_vPosition.y);
      psur->Line(m_d.m_vPosition.x, m_d.m_vPosition.y -20.0f,m_d.m_vPosition.x, m_d.m_vPosition.y +20.0f);
   }
   else
   {
      //just draw a simple mesh layout not the entire mesh for performance reasons
      for( unsigned i=0; i<indexList.size(); i+=3 )
      {
         const Vertex3Ds * const A = &vertices[ indexList[i]  ];
         const Vertex3Ds * const B = &vertices[ indexList[i+1]];
         psur->Line( A->x,A->y,B->x,B->y);
         //psur->Line( B->x,B->y,C->x,C->y);
         //psur->Line( C->x,C->y,A->x,A->y);
      }
   }
}

void Primitive::CalculateBuiltinOriginal()
{
   // this recalculates the Original Vertices -> should be only called, when sides are altered.
   const float outerRadius = -0.5f/(cosf((float)M_PI/(float)m_d.m_Sides));
   const float addAngle = (float)(2.0*M_PI)/(float)m_d.m_Sides;
   const float offsAngle = (float)M_PI/(float)m_d.m_Sides;
   float minX = FLT_MAX;
   float minY = FLT_MAX;
   float maxX = -FLT_MAX;
   float maxY = -FLT_MAX;

   objMeshOrg.resize(4*m_d.m_Sides + 2);

   Vertex3D_NoTex2 *middle;
   middle = &objMeshOrg[0]; // middle point top
   middle->x = 0.0f;
   middle->y = 0.0f;
   middle->z = 0.5f;
   middle = &objMeshOrg[m_d.m_Sides+1]; // middle point bottom
   middle->x = 0.0f;
   middle->y = 0.0f;
   middle->z = -0.5f;
   for (int i = 0; i < m_d.m_Sides; ++i)
   {
      // calculate Top
      Vertex3D_NoTex2 * const topVert = &objMeshOrg[i+1]; // top point at side
      const float currentAngle = addAngle*(float)i + offsAngle;
      topVert->x = sinf(currentAngle)*outerRadius;
      topVert->y = cosf(currentAngle)*outerRadius;		
      topVert->z = 0.5f;

      // calculate bottom
      Vertex3D_NoTex2 * const bottomVert = &objMeshOrg[i+1 + m_d.m_Sides+1]; // bottompoint at side
      bottomVert->x = topVert->x;
      bottomVert->y = topVert->y;
      bottomVert->z = -0.5f;

      // calculate sides
      objMeshOrg[m_d.m_Sides*2 + 2 + i] = *topVert; // sideTopVert
      objMeshOrg[m_d.m_Sides*3 + 2 + i] = *bottomVert; // sideBottomVert

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
   middle = &objMeshOrg[0]; // middle point top
   middle->tu = 0.25f;   // /4
   middle->tv = 0.25f;   // /4
   middle = &objMeshOrg[m_d.m_Sides+1]; // middle point bottom
   middle->tu = (float)(0.25*3.); // /4*3
   middle->tv = 0.25f;   // /4
   const float invx = 0.5f/(maxX-minX);
   const float invy = 0.5f/(maxY-minY);
   const float invs = 1.0f/(float)m_d.m_Sides;
   for (int i = 0; i < m_d.m_Sides; i++)
   {
      Vertex3D_NoTex2 * const topVert = &objMeshOrg[i+1]; // top point at side
      topVert->tu = (topVert->x - minX)*invx;
      topVert->tv = (topVert->y - minY)*invy;

      Vertex3D_NoTex2 * const bottomVert = &objMeshOrg[i+1 + m_d.m_Sides+1]; // bottompoint at side
      bottomVert->tu = topVert->tu+0.5f;
      bottomVert->tv = topVert->tv;

      Vertex3D_NoTex2 * const sideTopVert = &objMeshOrg[m_d.m_Sides*2 + 2 + i];
      Vertex3D_NoTex2 * const sideBottomVert = &objMeshOrg[m_d.m_Sides*3 + 2 + i];

      sideTopVert->tu = (float)i*invs;
      sideTopVert->tv = 0.5f;
      sideBottomVert->tu = sideTopVert->tu;
      sideBottomVert->tv = 1.0f;
   }

   // So how many indices are needed?
   // 3 per Triangle top - we have m_sides triangles -> 0, 1, 2, 0, 2, 3, 0, 3, 4, ...
   // 3 per Triangle bottom - we have m_sides triangles
   // 6 per Side at the side (two triangles form a rectangle) - we have m_sides sides
   // == 12 * m_sides
   // * 2 for both cullings (m_DrawTexturesInside == true)
   // == 24 * m_sides
   // this will also be the initial sorting, when depths, Vertices and Indices are recreated, because calculateRealTimeOriginal is called.

   // 2 restore indices
   //   check if anti culling is enabled:
   if (m_d.m_DrawTexturesInside)
   {
      indexList.resize(m_d.m_Sides*24);
      // yes: draw everything twice
      // restore indices
      for (int i = 0; i < m_d.m_Sides; i++)
      {
         const int tmp = (i == m_d.m_Sides-1) ? 1 : (i+2); // wrapping around
         // top
         indexList[i*6  ] = 0;
         indexList[i*6+1] = i + 1;
         indexList[i*6+2] = tmp;
         indexList[i*6+3] = 0;
         indexList[i*6+4] = tmp;
         indexList[i*6+5] = i + 1;

         const int tmp2 = tmp+1;
         // bottom
         indexList[6 * (i + m_d.m_Sides)    ] = m_d.m_Sides + 1;
         indexList[6 * (i + m_d.m_Sides) + 1] = m_d.m_Sides + tmp2;
         indexList[6 * (i + m_d.m_Sides) + 2] = m_d.m_Sides + 2 + i;
         indexList[6 * (i + m_d.m_Sides) + 3] = m_d.m_Sides + 1;
         indexList[6 * (i + m_d.m_Sides) + 4] = m_d.m_Sides + 2 + i;
         indexList[6 * (i + m_d.m_Sides) + 5] = m_d.m_Sides + tmp2;
         // sides
         indexList[12 * (i + m_d.m_Sides)    ] = m_d.m_Sides*2 + tmp2;
         indexList[12 * (i + m_d.m_Sides) + 1] = m_d.m_Sides*2 + 2 + i;
         indexList[12 * (i + m_d.m_Sides) + 2] = m_d.m_Sides*3 + 2 + i;
         indexList[12 * (i + m_d.m_Sides) + 3] = m_d.m_Sides*2 + tmp2;
         indexList[12 * (i + m_d.m_Sides) + 4] = m_d.m_Sides*3 + 2 + i;
         indexList[12 * (i + m_d.m_Sides) + 5] = m_d.m_Sides*3 + tmp2;
         indexList[12 * (i + m_d.m_Sides) + 6] = m_d.m_Sides*2 + tmp2;
         indexList[12 * (i + m_d.m_Sides) + 7] = m_d.m_Sides*3 + 2 + i;
         indexList[12 * (i + m_d.m_Sides) + 8] = m_d.m_Sides*2 + 2 + i;
         indexList[12 * (i + m_d.m_Sides) + 9] = m_d.m_Sides*2 + tmp2;
         indexList[12 * (i + m_d.m_Sides) + 10]= m_d.m_Sides*3 + tmp2;
         indexList[12 * (i + m_d.m_Sides) + 11]= m_d.m_Sides*3 + 2 + i;
      }
   } else {
      // no: only out-facing polygons
      // restore indices
      indexList.resize(m_d.m_Sides*12);
      for (int i = 0; i < m_d.m_Sides; i++)
      {
         const int tmp = (i == m_d.m_Sides-1) ? 1 : (i+2); // wrapping around
         // top
         indexList[i*3  ] = 0;
         indexList[i*3+2] = i + 1;
         indexList[i*3+1] = tmp;

         const int tmp2 = tmp+1;
         // bottom
         indexList[3 * (i + m_d.m_Sides)    ] = m_d.m_Sides + 1;
         indexList[3 * (i + m_d.m_Sides) + 1] = m_d.m_Sides + 2 + i;
         indexList[3 * (i + m_d.m_Sides) + 2] = m_d.m_Sides + tmp2;

         // sides
         indexList[6 * (i + m_d.m_Sides)    ] = m_d.m_Sides*2 + tmp2;
         indexList[6 * (i + m_d.m_Sides) + 1] = m_d.m_Sides*3 + 2 + i;
         indexList[6 * (i + m_d.m_Sides) + 2] = m_d.m_Sides*2 + 2 + i;
         indexList[6 * (i + m_d.m_Sides) + 3] = m_d.m_Sides*2 + tmp2;
         indexList[6 * (i + m_d.m_Sides) + 4] = m_d.m_Sides*3 + tmp2;
         indexList[6 * (i + m_d.m_Sides) + 5] = m_d.m_Sides*3 + 2 + i;
      }
   }
}

void Primitive::UpdateMeshBuiltin()
{
   // 1 copy vertices
   objMesh = objMeshOrg;

   // 2 apply matrix trafo
   // BUG: SetNormal only works for plane polygons
   SetNormal( &objMesh[0], &indexList[0], indexList.size(), NULL, NULL, 0 );

   for (int i = 0; i < (m_d.m_Sides*4 + 2); i++)
   {
      TransformVertex( objMesh[i] );
   }

   // store in vertex buffer
   Vertex3D_NoTex2 *buf;
   vertexBuffer->lock(0,0,(void**)&buf, VertexBuffer::WRITEONLY);
   memcpy( buf, &objMesh[0], sizeof(Vertex3D_NoTex2)*objMesh.size() );
   vertexBuffer->unlock();

#if 0   // depth sorting of polygons: this probably serves no practical purpose, better to keep the index buffer constant
   // 3 depth calculation / sorting

   // I need m_sides values at top
   // I need m_sides values at bottom
   // I need m_sides values at the side, since i use only one depth value for each side instead of two.
   // in the implementation i will use shell sort like implemented at wikipedia.
   // Other algorithms are better at presorted things, but i will have some reverse sorted elements between the presorted here. 
   // That's why insertion or bubble sort does not work fast here...
   // input: an array a of length n with array elements numbered 0 to n ? 1

   const float zMultiplicator = cosf(ANGTORAD(m_ptable->m_inclination));
   const float yMultiplicator = sinf(ANGTORAD(m_ptable->m_inclination));

   // depth calculation
   // Since we are compiling with SSE, I'll use Floating points for comparison.
   // I need m_sides values at top
   // I need m_sides values at bottom
   // I need m_sides * 2 values at the side
   // in the implementation i will use shell sort like implemented at wikipedia.
   // Other algorithms are better at presorted things, but i will have some reverse sorted elements between the presorted here. 
   // That's why insertion or bubble sort does not work fast here...
   std::vector<float> builtin_depth(m_d.m_Sides * 4);

   // get depths
   if (!m_d.m_DrawTexturesInside)
   {
      // top and bottom
      for (int i = 0; i < m_d.m_Sides * 2; i++)
      {
         //!! this is wrong!
         builtin_depth[i] = 
            zMultiplicator*objMesh[indexList[i*3  ]].z+
            zMultiplicator*objMesh[indexList[i*3+1]].z+
            zMultiplicator*objMesh[indexList[i*3+2]].z+
            yMultiplicator*objMesh[indexList[i*3  ]].y+
            yMultiplicator*objMesh[indexList[i*3+1]].y+
            yMultiplicator*objMesh[indexList[i*3+2]].y;
      }
   } else {
      const float zM13 = (float)(1.0/3.0) * zMultiplicator;
      const float yM13 = (float)(1.0/3.0) * yMultiplicator;
      // top and bottom
      for (int i = 0; i < m_d.m_Sides * 2; i++)
      {
         builtin_depth[i] = 
            (objMesh[indexList[i*6  ]].z+
             objMesh[indexList[i*6+1]].z+
             objMesh[indexList[i*6+2]].z) 
            * zM13 +
            (objMesh[indexList[i*6  ]].y+
             objMesh[indexList[i*6+1]].y+
             objMesh[indexList[i*6+2]].y) 
            * yM13;
      }

      const float zM05 = 0.5f * zMultiplicator;
      const float yM05 = 0.5f * yMultiplicator;
      for (int i = m_d.m_Sides; i < m_d.m_Sides * 2; i++)
      {
         builtin_depth[i*2] = 
         builtin_depth[i*2+1] = 
            (objMesh[indexList[i*12  ]].z+
             objMesh[indexList[i*12+1]].z)
            * zM05 +
            (objMesh[indexList[i*12  ]].y+
             objMesh[indexList[i*12+1]].y)
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
               tempIndices[tempI] = indexList[i*6 + tempI];

            int j = i;
            while ((j >= inc) && (builtin_depth[j-inc] > tempDepth))
            {
               builtin_depth[j] = builtin_depth[j-inc];
               for (int tempI = 0; tempI < 6; tempI++)
                  indexList[j*6+tempI] = indexList[(j-inc)*6 + tempI];
               j -= inc;
            }

            builtin_depth[j] = tempDepth;
            for (int tempI = 0; tempI < 6; tempI++)
               indexList[j*6+tempI] = tempIndices[tempI];
         }

         if(inc == 2)
            inc = 1;
         else
            inc = (int)((float)inc*(float)(1.0/2.2));
      }
   } //else { //!! this is missing completely!!???
   //}
#endif
}

void Primitive::TransformVertex(Vertex3D_NoTex2& v) const
{
    // NB: most transformations are now handled by the
    // proper world transform matrix, only shear scaling
    // stays here for now.
    v.y *= 1.0f + (m_d.m_vAxisScaleX.y - 1.0f)*(v.x+0.5f);
    v.z *= 1.0f + (m_d.m_vAxisScaleX.z - 1.0f)*(v.x+0.5f);
    v.x *= 1.0f + (m_d.m_vAxisScaleY.x - 1.0f)*(v.y+0.5f);
    v.z *= 1.0f + (m_d.m_vAxisScaleY.z - 1.0f)*(v.y+0.5f);
    v.x *= 1.0f + (m_d.m_vAxisScaleZ.x - 1.0f)*(v.z+0.5f);
    v.y *= 1.0f + (m_d.m_vAxisScaleZ.y - 1.0f)*(v.z+0.5f);
    v.z *= m_ptable->m_zScale;
}

void Primitive::UpdateMesh()
{
   objMesh = objMeshOrg;
   if ( m_d.sphereMapping )
   {
      Matrix3D matView = g_pplayer->m_pin3d.GetViewTransform();
      matView.Multiply(rotMatrix, rotMatrix);
      // TODO/BUG: this should compute the inverse transpose of the rotational part
   }

   for (unsigned i = 0; i < objMeshOrg.size(); i++)
   {
      Vertex3D_NoTex2 * const tempVert = &objMesh[i];
      if ( m_d.sphereMapping )
      {
         Vertex3Ds norm(tempVert->nx, tempVert->ny, tempVert->nz);
         rotMatrix.MultiplyVectorNoTranslate(norm, norm);
         tempVert->tu = 0.5f + norm.x*0.5f;
         tempVert->tv = 0.5f + norm.y*0.5f;
      }
      TransformVertex(*tempVert);
      /* HACK/VP9COMPAT:
       * In VP9, all the normals are the wrong way around, so we also
       * have to flip them on imported meshes for now.
       */
      if (!m_d.sphereMapping)
      {
          tempVert->nx *= -1.0f;
          tempVert->ny *= -1.0f;
          tempVert->nz *= -1.0f;
      }
   }

   Vertex3D_NoTex2 *buf;
   vertexBuffer->lock(0,0,(void**)&buf, VertexBuffer::WRITEONLY);
   memcpy( buf, &objMesh[0], sizeof(Vertex3D_NoTex2)*objMesh.size() );
   vertexBuffer->unlock();
}

void Primitive::RenderObject( RenderDevice *pd3dDevice )
{
   RecalculateMatrices();

   if (m_d.m_TopVisible)
   {
      Texture * const pin = m_ptable->GetImage(m_d.m_szImage);

      if (pin)
      {
         pin->CreateAlphaChannel();
         pin->Set( ePictureTexture );
         g_pplayer->m_pin3d.EnableAlphaBlend(1, fFalse);
         g_pplayer->m_pin3d.SetTextureFilter(ePictureTexture, TEXTURE_MODE_TRILINEAR);
      }
      else
      {
         g_pplayer->m_pin3d.SetTexture(NULL);
         if( vertexBufferRegenerate )
            material.setColor( 1.0f, m_d.m_TopColor );
      }

      pd3dDevice->SetMaterial(material);

      if (vertexBufferRegenerate)
      {
         vertexBufferRegenerate = false;

         if( m_d.use3DMesh )
            UpdateMesh();
         else
            UpdateMeshBuiltin();
      }

      if ( !m_d.useLighting )
      {
          // disable lighting is a default setting
          // it could look odd if you switch lighting on on non mesh primitives
          pd3dDevice->SetRenderState( RenderDevice::LIGHTING, FALSE );
          // VP9COMPAT: in VP10, the following should be enabled
          //pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_COLORARG2, D3DTA_TFACTOR);
          //pd3dDevice->SetRenderState(RenderDevice::TEXTUREFACTOR, COLORREF_to_D3DCOLOR(m_d.m_TopColor));
      }

      // set transform
      Matrix3D matOrig, matNew;
      matOrig = g_pplayer->m_pin3d.GetWorldTransform();
      //matTemp.SetScaling(1.0f, 1.0f, m_ptable->m_zScale); // TODO: z-scaling? causes distortions
      //matNew.Multiply(matTemp, matNew);
      matOrig.Multiply(fullMatrix, matNew);
      pd3dDevice->SetTransform(TRANSFORMSTATE_WORLD, &matNew);
      
      // draw the mesh
      pd3dDevice->DrawIndexedPrimitiveVB( D3DPT_TRIANGLELIST, vertexBuffer, 0, objMeshOrg.size(), indexBuffer, 0, indexList.size() );

      // reset transform
      pd3dDevice->SetTransform(TRANSFORMSTATE_WORLD, &matOrig);

      // reset render states
      if ( !m_d.useLighting )
      {
          pd3dDevice->SetRenderState(RenderDevice::LIGHTING, TRUE);
          pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
          pd3dDevice->SetRenderState(RenderDevice::TEXTUREFACTOR, 0xffffffff);
      }

      g_pplayer->m_pin3d.DisableAlphaBlend();
   }
}

// Always called each frame to render over everything else (along with alpha ramps)
void Primitive::PostRenderStatic(const RenderDevice* _pd3dDevice)
{
    TRACE_FUNCTION();
   if ( m_d.staticRendering )
      return;

   RenderDevice* pd3dDevice=(RenderDevice*)_pd3dDevice;
   RenderObject( pd3dDevice );
}

// defined in objloader.cpp
extern bool WaveFrontObj_Load( char *filename, bool flipTv, bool convertToLeftHanded );
extern void WaveFrontObj_GetVertices( std::vector<Vertex3D_NoTex2>& objMesh );
extern void WaveFrontObj_GetIndices( std::vector<WORD>& list );
extern void WaveFrontObj_Save( char *filename, Primitive *mesh );
//

void Primitive::RenderSetup( const RenderDevice* _pd3dDevice )
{
   RenderDevice* pd3dDevice=(RenderDevice*)_pd3dDevice;

   if( !m_d.use3DMesh )
      CalculateBuiltinOriginal();

   if( !vertexBuffer )
      pd3dDevice->CreateVertexBuffer( objMeshOrg.size(), 0, MY_D3DFVF_NOTEX2_VERTEX, &vertexBuffer );

   indexBuffer = pd3dDevice->CreateAndFillIndexBuffer( indexList );

   // make sure alpha channel is set up
   Texture * const tex = m_ptable->GetImage(m_d.m_szImage);
   if (tex)
       tex->CreateAlphaChannel();
}

void Primitive::RenderStatic(const RenderDevice* _pd3dDevice)
{
   if( m_d.staticRendering )
   {
      RenderDevice *pd3dDevice = (RenderDevice*)_pd3dDevice;
      RenderObject(pd3dDevice);
      g_pplayer->m_pin3d.SetTexture(NULL);
   }
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
   /*
    * Someone decided that it was a good idea to write these vectors including
    * the fourth padding float that they used to have, so now we have to write
    * them padded to 4 floats to maintain compatibility.
    */
   bw.WriteVector3Padded(FID(VPOS), &m_d.m_vPosition);
   bw.WriteVector3Padded(FID(VSIZ), &m_d.m_vSize);
   bw.WriteVector3Padded(FID(AXSX), &m_d.m_vAxisScaleX);
   bw.WriteVector3Padded(FID(AXSY), &m_d.m_vAxisScaleY);
   bw.WriteVector3Padded(FID(AXSZ), &m_d.m_vAxisScaleZ);
   bw.WriteFloat(FID(RTV0), m_d.m_aRotAndTra[0]);
   bw.WriteFloat(FID(RTV1), m_d.m_aRotAndTra[1]);
   bw.WriteFloat(FID(RTV2), m_d.m_aRotAndTra[2]);
   bw.WriteFloat(FID(RTV3), m_d.m_aRotAndTra[3]);
   bw.WriteFloat(FID(RTV4), m_d.m_aRotAndTra[4]);
   bw.WriteFloat(FID(RTV5), m_d.m_aRotAndTra[5]);
   bw.WriteFloat(FID(RTV6), m_d.m_aRotAndTra[6]);
   bw.WriteFloat(FID(RTV7), m_d.m_aRotAndTra[7]);
   bw.WriteFloat(FID(RTV8), m_d.m_aRotAndTra[8]);
   bw.WriteString(FID(IMAG), m_d.m_szImage);
   bw.WriteInt(FID(SIDS), m_d.m_Sides);
   bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);
   bw.WriteInt(FID(TCOL), m_d.m_TopColor);
   bw.WriteInt(FID(SCOL), m_d.m_SideColor);
   bw.WriteInt(FID(TVIS), (m_d.m_TopVisible) ? 1 : 0);
   bw.WriteInt(FID(DTXI), (m_d.m_DrawTexturesInside) ? 1 : 0);
   bw.WriteBool(FID(HTEV), m_d.m_fHitEvent);
   bw.WriteFloat(FID(THRS), m_d.m_threshold);
   bw.WriteFloat(FID(ELAS), m_d.m_elasticity);
   bw.WriteFloat(FID(RFCT), m_d.m_friction);
   bw.WriteFloat(FID(RSCT), m_d.m_scatter);
   bw.WriteBool(FID(CLDRP), m_d.m_fCollidable);
   bw.WriteBool(FID(ISTO), m_d.m_fToy);
   bw.WriteInt(FID(ENLI), (m_d.useLighting) ? 1 : 0);
   bw.WriteInt(FID(U3DM), (m_d.use3DMesh) ? 1 : 0 );
   bw.WriteInt(FID(STRE), (m_d.staticRendering) ? 1 : 0 );
   bw.WriteInt(FID(EVMP), (m_d.sphereMapping) ? 1 : 0 );
   if( m_d.use3DMesh )
   {
      bw.WriteString( FID(M3DN), m_d.meshFileName);
      bw.WriteInt( FID(M3VN), (int)objMeshOrg.size() );
      bw.WriteStruct( FID(M3DX), &objMeshOrg[0], sizeof(Vertex3D_NoTex2)*objMeshOrg.size());
      bw.WriteInt( FID(M3FN), indexList.size() );
      bw.WriteStruct( FID(M3DI), &indexList[0], sizeof(WORD)*indexList.size() );
   }
   bw.WriteFloat(FID(PIDB), m_d.m_depthBias);

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
      pbr->GetVector3Padded(&m_d.m_vPosition);
   }
   else if (id == FID(VSIZ))
   {
      pbr->GetVector3Padded(&m_d.m_vSize);
   }
   else if (id == FID(AXSX))
   {
      pbr->GetVector3Padded(&m_d.m_vAxisScaleX);
   }
   else if (id == FID(AXSY))
   {
      pbr->GetVector3Padded(&m_d.m_vAxisScaleY);
   }
   else if (id == FID(AXSZ))
   {
      pbr->GetVector3Padded(&m_d.m_vAxisScaleZ);
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
   else if (id == FID(HTEV))
   {
      pbr->GetBool(&m_d.m_fHitEvent);
   }
   else if (id == FID(THRS))
   {
      pbr->GetFloat(&m_d.m_threshold);
   }
   else if (id == FID(ELAS))
   {
      pbr->GetFloat(&m_d.m_elasticity);
   }
   else if (id == FID(RFCT))
   {
      pbr->GetFloat(&m_d.m_friction);
   }
   else if (id == FID(RSCT))
   {
      pbr->GetFloat(&m_d.m_scatter);
   }
   else if (id == FID(CLDRP))
   {
      BOOL iTmp;
      pbr->GetBool(&iTmp);
      m_d.m_fCollidable = (iTmp==1);
   }
   else if (id == FID(ISTO))
   {
      BOOL iTmp;
      pbr->GetBool(&iTmp);
      m_d.m_fToy = (iTmp==1);
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
      objMesh.clear();
      objMeshOrg.clear();
      objMeshOrg.resize(numVertices);
      pbr->GetStruct( &objMeshOrg[0], sizeof(Vertex3D_NoTex2)*numVertices);
   }
   else if( id == FID(M3FN) )
   {
      pbr->GetInt( &numIndices );
   }
   else if( id == FID(M3DI) )
   {
      indexList.resize( numIndices );
      pbr->GetStruct( &indexList[0], sizeof(WORD)*numIndices);
   }
   else if (id == FID(PIDB))
   {
      pbr->GetFloat(&m_d.m_depthBias);
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
   char szFoo[MAX_PATH];
   if (hr == S_OK)
   {
      ofn.lpstrInitialDir = szInitialDir;
   }
   else
   {
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
   objMeshOrg.clear();
   objMesh.clear();
   numVertices = numIndices = 0;
   indexList.clear();
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
   if ( WaveFrontObj_Load(ofn.lpstrFile, flipTV, convertToLeftHanded) )
   {
      m_d.use3DMesh=true;
      WaveFrontObj_GetVertices( objMeshOrg );
      WaveFrontObj_GetIndices( indexList );
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
   char szFoo[MAX_PATH];
   if (hr == S_OK)
   {
      ofn.lpstrInitialDir = szInitialDir;
   }
   else
   {
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
   WaveFrontObj_Save( ofn.lpstrFile, this );
}

bool Primitive::IsTransparent()
{
    Texture *tex = m_ptable->GetImage(m_d.m_szImage);
    return tex && tex->m_fTransparent;
}

float Primitive::GetDepth(const Vertex3Ds& viewDir)
{
    return m_d.m_depthBias + m_d.m_vPosition.Dot( viewDir );
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
   if(m_d.m_DrawTexturesInside != VBTOF(newVal))
   {
	   STARTUNDO

	   m_d.m_DrawTexturesInside = VBTOF(newVal);
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
    return get_RotX(pVal);
}

STDMETHODIMP Primitive::put_RotAndTra0(float newVal)
{
    return put_RotX(newVal);
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

	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Primitive::get_RotAndTra1(float *pVal)
{
    return get_RotY(pVal);
}

STDMETHODIMP Primitive::put_RotAndTra1(float newVal)
{
    return put_RotY(newVal);
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

	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Primitive::get_RotAndTra2(float *pVal)
{
    return get_RotZ(pVal);
}

STDMETHODIMP Primitive::put_RotAndTra2(float newVal)
{
    return put_RotZ(newVal);
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

	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Primitive::get_RotAndTra3(float *pVal)
{
    return get_TransX(pVal);
}

STDMETHODIMP Primitive::put_RotAndTra3(float newVal)
{
    return put_TransX(newVal);
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

	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Primitive::get_RotAndTra4(float *pVal)
{
    return get_TransY(pVal);
}

STDMETHODIMP Primitive::put_RotAndTra4(float newVal)
{
    return put_TransY(newVal);
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

	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Primitive::get_RotAndTra5(float *pVal)
{
    return get_TransZ(pVal);
}

STDMETHODIMP Primitive::put_RotAndTra5(float newVal)
{
    return put_TransZ(newVal);
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

	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Primitive::get_RotAndTra6(float *pVal)
{
    return get_ObjRotX(pVal);
}

STDMETHODIMP Primitive::put_RotAndTra6(float newVal)
{
    return put_ObjRotX(newVal);
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

	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Primitive::get_RotAndTra7(float *pVal)
{
    return get_ObjRotY(pVal);
}

STDMETHODIMP Primitive::put_RotAndTra7(float newVal)
{
    return put_ObjRotY(newVal);
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

	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Primitive::get_RotAndTra8(float *pVal)
{
    return get_ObjRotZ(pVal);
}

STDMETHODIMP Primitive::put_RotAndTra8(float newVal)
{
    return put_ObjRotZ(newVal);
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

STDMETHODIMP Primitive::get_HasHitEvent(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fHitEvent);

   return S_OK;
}

STDMETHODIMP Primitive::put_HasHitEvent(VARIANT_BOOL newVal)
{
   STARTUNDO

   m_d.m_fHitEvent = VBTOF(newVal);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Primitive::get_Threshold(float *pVal)
{
   *pVal = m_d.m_threshold;

   return S_OK;
}

STDMETHODIMP Primitive::put_Threshold(float newVal)
{
   STARTUNDO

   m_d.m_threshold = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Primitive::get_Elasticity(float *pVal)
{
   *pVal = m_d.m_elasticity;

   return S_OK;
}

STDMETHODIMP Primitive::put_Elasticity(float newVal)
{
   STARTUNDO

   m_d.m_elasticity = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Primitive::get_Friction(float *pVal)
{
   *pVal = m_d.m_friction;

   return S_OK;
}

STDMETHODIMP Primitive::put_Friction(float newVal)
{
   STARTUNDO

   if (newVal > 1.0f) newVal = 1.0f;
      else if (newVal < 0.f) newVal = 0.f;

   m_d.m_friction = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Primitive::get_Scatter(float *pVal)
{
   *pVal = m_d.m_scatter;

   return S_OK;
}

STDMETHODIMP Primitive::put_Scatter(float newVal)
{
   STARTUNDO

   m_d.m_scatter = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Primitive::get_Collidable(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB((!g_pplayer) ? m_d.m_fCollidable : m_vhoCollidable.ElementAt(0)->m_fEnabled);

   return S_OK;
}

STDMETHODIMP Primitive::put_Collidable(VARIANT_BOOL newVal)
{
   BOOL fNewVal = VBTOF(newVal);	
   if (!g_pplayer)
   {	
      STARTUNDO

      m_d.m_fCollidable = !!fNewVal;		

      STOPUNDO
   }
   else
      for (int i=0;i < m_vhoCollidable.Size();i++)
         m_vhoCollidable.ElementAt(i)->m_fEnabled = fNewVal;	//copy to hit checking on enities composing the object 

   return S_OK;
}

STDMETHODIMP Primitive::get_IsToy(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fToy);

   return S_OK;
}

STDMETHODIMP Primitive::put_IsToy(VARIANT_BOOL newVal)
{
   STARTUNDO

      m_d.m_fToy = VBTOF(newVal);		

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Primitive::get_UpdateRegions(VARIANT_BOOL *pVal)
{
   //!! deprecated
   *pVal = (VARIANT_BOOL)FTOVB(false);

   return S_OK;
}

STDMETHODIMP Primitive::put_UpdateRegions(VARIANT_BOOL newVal)
{
   //!! deprecated
   return S_OK;
}

STDMETHODIMP Primitive::TriggerSingleUpdate() 
{
   //!! deprecated
   return S_OK;
}

void Primitive::GetDialogPanes(Vector<PropertyPane> *pvproppane)
{
   PropertyPane *pproppane;

   pproppane = new PropertyPane(IDD_PROP_NAME, NULL);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPPRIMITIVE_VISUALS, IDS_VISUALS);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPPRIMITIVE_POSITION, IDS_POSITION_TRANSLATION);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPPRIMITIVE_PHYSICS, IDS_PHYSICS);
   pvproppane->AddElement(pproppane);
}

STDMETHODIMP Primitive::get_DepthBias(float *pVal)
{
   *pVal = m_d.m_depthBias;

   return S_OK;
}

STDMETHODIMP Primitive::put_DepthBias(float newVal)
{
   if(m_d.m_depthBias != newVal)
   {
      STARTUNDO

      m_d.m_depthBias = newVal;

      STOPUNDO
   }

   return S_OK;
}
