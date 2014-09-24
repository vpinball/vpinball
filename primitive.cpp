// primitive.cpp: implementation of the Primitive class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h" 

// defined in objloader.cpp
extern bool WaveFrontObj_Load(const char *filename, bool flipTv, bool convertToLeftHanded );
extern void WaveFrontObj_GetVertices( std::vector<Vertex3D_NoTex2>& verts );
extern void WaveFrontObj_GetIndices( std::vector<WORD>& list );
extern void WaveFrontObj_Save(const char *filename, const char *description, const Mesh& mesh);
//


void Mesh::Clear()
{
    m_vertices.clear();
    m_indices.clear();
}

bool Mesh::LoadWavefrontObj(const char *fname, bool flipTV, bool convertToLeftHanded)
{
    Clear();

    if (WaveFrontObj_Load(fname, flipTV, convertToLeftHanded))
    {
        WaveFrontObj_GetVertices(m_vertices);
        WaveFrontObj_GetIndices(m_indices);
        return true;
    }
    else
        return false;
}

void Mesh::SaveWavefrontObj(const char *fname, const char *description)
{
    if (description == NULL)
        description = fname;

    WaveFrontObj_Save(fname, description, *this);
}


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

   if( !m_d.use3DMesh )
      CalculateBuiltinOriginal();

   UpdateEditorView();

   return S_OK;
}

void Primitive::SetDefaults(bool fromMouseClick)
{
   static const char strKeyName[] = "DefaultProps\\Primitive";

   HRESULT hr;

   m_d.use3DMesh=false;
   m_d.meshFileName[0]=0;

   // sides
   m_d.m_Sides = fromMouseClick ? GetRegIntWithDefault(strKeyName,"Sides", 4) : 4;
   if(m_d.m_Sides > Max_Primitive_Sides)
      m_d.m_Sides = Max_Primitive_Sides;

   // colors
   m_d.m_SideColor = fromMouseClick ? GetRegIntWithDefault(strKeyName, "SideColor", RGB(150,150,150)) : RGB(150,150,150);

   m_d.m_fVisible = fromMouseClick ? GetRegBoolWithDefault(strKeyName, "Visible", true) : true;
   m_d.useLighting = fromMouseClick ? GetRegBoolWithDefault(strKeyName, "UseLighting", false) : false;
   m_d.staticRendering = fromMouseClick ? GetRegBoolWithDefault(strKeyName, "StaticRendering", true) : true;
   m_d.sphereMapping = fromMouseClick ? GetRegBoolWithDefault(strKeyName, "SphereMapping", false) : false;
   m_d.m_DrawTexturesInside = fromMouseClick ? GetRegBoolWithDefault(strKeyName, "DrawTexturesInside", false) : false;

   // Position (X and Y is already set by the click of the user)
   m_d.m_vPosition.z = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "Position_Z", 0.0f) : 0.0f;

   // Size
   m_d.m_vSize.x = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "Size_X", 100.0f) : 100.0f;
   m_d.m_vSize.y = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "Size_Y", 100.0f) : 100.0f;
   m_d.m_vSize.z = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "Size_Z", 100.0f) : 100.0f;

   // Axis Scale
   m_d.m_vAxisScaleX.x = 1.0f;
   m_d.m_vAxisScaleY.y = 1.0f;
   m_d.m_vAxisScaleZ.z = 1.0f;
   m_d.m_vAxisScaleX.y = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "AxisScale_X_Y", 1.0f) : 1.0f;
   m_d.m_vAxisScaleX.z = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "AxisScale_X_Z", 1.0f) : 1.0f;
   m_d.m_vAxisScaleY.x = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "AxisScale_Y_X", 1.0f) : 1.0f;
   m_d.m_vAxisScaleY.z = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "AxisScale_Y_Z", 1.0f) : 1.0f;
   m_d.m_vAxisScaleZ.x = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "AxisScale_Z_X", 1.0f) : 1.0f;
   m_d.m_vAxisScaleZ.y = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "AxisScale_Z_Y", 1.0f) : 1.0f;

   // Rotation and Transposition
   m_d.m_aRotAndTra[0] = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName,"RotAndTra0", 0.0f) : 0.0f;
   m_d.m_aRotAndTra[1] = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName,"RotAndTra1", 0.0f) : 0.0f;
   m_d.m_aRotAndTra[2] = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName,"RotAndTra2", 0.0f) : 0.0f;
   m_d.m_aRotAndTra[3] = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName,"RotAndTra3", 0.0f) : 0.0f;
   m_d.m_aRotAndTra[4] = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName,"RotAndTra4", 0.0f) : 0.0f;
   m_d.m_aRotAndTra[5] = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName,"RotAndTra5", 0.0f) : 0.0f;
   m_d.m_aRotAndTra[6] = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName,"RotAndTra6", 0.0f) : 0.0f;
   m_d.m_aRotAndTra[7] = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName,"RotAndTra7", 0.0f) : 0.0f;
   m_d.m_aRotAndTra[8] = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName,"RotAndTra8", 0.0f) : 0.0f;

   /*
   hr = GetRegStringAsFloat(strKeyName,"Rotation_X", &fTmp);
   m_d.m_vRotation.x = (hr == S_OK) ? fTmp : 0;
   hr = GetRegStringAsFloat(strKeyName,"Rotation_Y", &fTmp);
   m_d.m_vRotation.y = (hr == S_OK) ? fTmp : 0;
   hr = GetRegStringAsFloat(strKeyName,"Rotation_Z", &fTmp);
   m_d.m_vRotation.z = (hr == S_OK) ? fTmp : 0;

   //Transposition
   hr = GetRegStringAsFloat(strKeyName,"Transposition_X", &fTmp);
   m_d.m_vTransposition.x = (hr == S_OK) ? fTmp : 0;
   hr = GetRegStringAsFloat(strKeyName,"Transposition_Y", &fTmp);
   m_d.m_vTransposition.y = (hr == S_OK) ? fTmp : 0;
   hr = GetRegStringAsFloat(strKeyName,"Transposition_Z", &fTmp);
   m_d.m_vTransposition.z = (hr == S_OK) ? fTmp : 0;
   */

   hr = GetRegString(strKeyName,"Image", m_d.m_szImage, MAXTOKEN);
   if ((hr != S_OK) && fromMouseClick)
      m_d.m_szImage[0] = 0;

   m_d.m_threshold = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName,"HitThreshold", 2.0f) : 2.0f;
   m_d.m_elasticity = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName,"Elasticity", 0.3f) : 0.3f;
   m_d.m_friction = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName,"Friction", 0.3f) : 0.3f;
   m_d.m_scatter = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName,"Scatter", 0) : 0;

   m_d.m_fCollidable = fromMouseClick ? GetRegBoolWithDefault(strKeyName,"Collidable", true) : true;
   m_d.m_fToy = fromMouseClick ? GetRegBoolWithDefault(strKeyName,"IsToy", false) : false;
}

void Primitive::WriteRegDefaults()
{
   static const char strKeyName[] = "DefaultProps\\Primitive";

   SetRegValueInt(strKeyName,"SideColor", m_d.m_SideColor);
   SetRegValueBool(strKeyName,"Visible", m_d.m_fVisible);
   SetRegValueBool(strKeyName,"UseLighting", m_d.useLighting);
   SetRegValueBool(strKeyName,"StaticRendering", m_d.staticRendering);
   SetRegValueBool(strKeyName,"SphereMapping", m_d.sphereMapping);
   SetRegValueBool(strKeyName,"DrawTexturesInside", m_d.m_DrawTexturesInside);

   SetRegValueFloat(strKeyName,"Position_Z", m_d.m_vPosition.z);

   SetRegValueFloat(strKeyName,"Size_X", m_d.m_vSize.x);
   SetRegValueFloat(strKeyName,"Size_Y", m_d.m_vSize.y);
   SetRegValueFloat(strKeyName,"Size_Z", m_d.m_vSize.z);

   SetRegValueFloat(strKeyName,"AxisScaleXY", m_d.m_vAxisScaleX.y);
   SetRegValueFloat(strKeyName,"AxisScaleXZ", m_d.m_vAxisScaleX.z);
   SetRegValueFloat(strKeyName,"AxisScaleYX", m_d.m_vAxisScaleY.x);
   SetRegValueFloat(strKeyName,"AxisScaleYZ", m_d.m_vAxisScaleY.z);
   SetRegValueFloat(strKeyName,"AxisScaleZX", m_d.m_vAxisScaleZ.x);
   SetRegValueFloat(strKeyName,"AxisScaleZY", m_d.m_vAxisScaleZ.y);

   SetRegValueFloat(strKeyName,"RotAndTra0", m_d.m_aRotAndTra[0]);
   SetRegValueFloat(strKeyName,"RotAndTra1", m_d.m_aRotAndTra[1]);
   SetRegValueFloat(strKeyName,"RotAndTra2", m_d.m_aRotAndTra[2]);
   SetRegValueFloat(strKeyName,"RotAndTra3", m_d.m_aRotAndTra[3]);
   SetRegValueFloat(strKeyName,"RotAndTra4", m_d.m_aRotAndTra[4]);
   SetRegValueFloat(strKeyName,"RotAndTra5", m_d.m_aRotAndTra[5]);
   SetRegValueFloat(strKeyName,"RotAndTra6", m_d.m_aRotAndTra[6]);
   SetRegValueFloat(strKeyName,"RotAndTra7", m_d.m_aRotAndTra[7]);
   SetRegValueFloat(strKeyName,"RotAndTra8", m_d.m_aRotAndTra[8]);

   SetRegValueString(strKeyName,"Image", m_d.m_szImage);
   SetRegValueBool(strKeyName,"HitEvent", !!m_d.m_fHitEvent);
   SetRegValueFloat(strKeyName,"HitThreshold", m_d.m_threshold);
   SetRegValueFloat(strKeyName,"Elasticity", m_d.m_elasticity);
   SetRegValueFloat(strKeyName,"Friction", m_d.m_friction);
   SetRegValueFloat(strKeyName,"Scatter", m_d.m_scatter);
   SetRegValueBool(strKeyName,"Collidable", m_d.m_fCollidable);
   SetRegValueBool(strKeyName,"IsToy", m_d.m_fToy);
}

void Primitive::GetTimers(Vector<HitTimer> * const pvht)
{
   IEditable::BeginPlay();
}

void Primitive::GetHitShapes(Vector<HitObject> * const pvho)
{
   if (m_d.m_fToy)
      return;

   RecalculateMatrices();
   TransformVertices();
   HitTriangle *ph3dpolyOld = NULL;

   for( unsigned i=0; i<m_mesh.NumIndices(); i+=3 )
   {
      Vertex3Ds rgv3D[3];
      rgv3D[0] = vertices[ m_mesh.m_indices[i  ] ];
      rgv3D[1] = vertices[ m_mesh.m_indices[i+1] ];
      rgv3D[2] = vertices[ m_mesh.m_indices[i+2] ];
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
   const Vertex3Ds vjointnormal = CrossProduct(ph3d1->normal, ph3d2->normal);
   if (vjointnormal.LengthSquared() < 1e-8f)
       return;  // coplanar triangles need no joints

   // By convention of the calling function, points 1 [0] and 2 [1] of the second polygon will
   // be the common-edge points
   //!! BUG/TODO: this is wrong! This code was blindly copy-pasted from the ramp code without
   // checking this assumption, which is not true for a general triangle mesh.

   HitLine3D * const ph3dc = new HitLine3D(ph3d2->m_rgv[0], ph3d2->m_rgv[1]);
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
void Primitive::TransformVertices()
{
    vertices.resize( m_mesh.NumVertices() );

    for( unsigned i=0; i<m_mesh.NumVertices(); i++ )
    {
        fullMatrix.MultiplyVector(m_mesh.m_vertices[i], vertices[i]);
    }
}

//////////////////////////////
// Rendering
//////////////////////////////

//2d
void Primitive::PreRender(Sur * const psur)
{
}

void Primitive::Render(Sur * const psur)
{
   psur->SetLineColor(RGB(0,0,0),false,1);
   psur->SetObject(this);

   if( m_mesh.NumVertices() <= 100)     // small mesh: draw all triangles
   {
      for( unsigned i=0; i<m_mesh.NumIndices(); i+=3 )
      {
         const Vertex3Ds * const A = &vertices[m_mesh.m_indices[i]  ];
         const Vertex3Ds * const B = &vertices[m_mesh.m_indices[i+1]];
         const Vertex3Ds * const C = &vertices[m_mesh.m_indices[i+2]];
         psur->Line(A->x,A->y, B->x,B->y);
         psur->Line(B->x,B->y, C->x,C->y);
         psur->Line(C->x,C->y, A->x,A->y);
      }
   }
   else     // large mesh: draw a simplified mesh for performance reasons
   {
       const size_t numPts = m_mesh.NumIndices() / 3 + 1;
       m_drawVertices.clear();
       m_drawVertices.reserve(numPts);

       if (numPts > 0)
       {
           const Vertex3Ds * const A = &vertices[m_mesh.m_indices[0]];
           m_drawVertices.push_back(A->xy());
       }

       for (unsigned i=0; i<m_mesh.NumIndices(); i+=3)
       {
           const Vertex3Ds * const A = &vertices[m_mesh.m_indices[i]  ];
           const Vertex3Ds * const B = &vertices[m_mesh.m_indices[i+1]];
           m_drawVertices.push_back(B->xy());
       }

       psur->Polyline(&m_drawVertices[0], m_drawVertices.size());
   }

   // draw center marker
   psur->SetLineColor(RGB(128,128,128),false,1);
   psur->Line(m_d.m_vPosition.x -10.0f, m_d.m_vPosition.y,m_d.m_vPosition.x +10.0f, m_d.m_vPosition.y);
   psur->Line(m_d.m_vPosition.x, m_d.m_vPosition.y -10.0f,m_d.m_vPosition.x, m_d.m_vPosition.y +10.0f);
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

   m_mesh.m_vertices.resize(4*m_d.m_Sides + 2);

   Vertex3D_NoTex2 *middle;
   middle = &m_mesh.m_vertices[0]; // middle point top
   middle->x = 0.0f;
   middle->y = 0.0f;
   middle->z = 0.5f;
   middle = &m_mesh.m_vertices[m_d.m_Sides+1]; // middle point bottom
   middle->x = 0.0f;
   middle->y = 0.0f;
   middle->z = -0.5f;
   for (int i = 0; i < m_d.m_Sides; ++i)
   {
      // calculate Top
      Vertex3D_NoTex2 * const topVert = &m_mesh.m_vertices[i+1]; // top point at side
      const float currentAngle = addAngle*(float)i + offsAngle;
      topVert->x = sinf(currentAngle)*outerRadius;
      topVert->y = cosf(currentAngle)*outerRadius;		
      topVert->z = 0.5f;

      // calculate bottom
      Vertex3D_NoTex2 * const bottomVert = &m_mesh.m_vertices[i+1 + m_d.m_Sides+1]; // bottompoint at side
      bottomVert->x = topVert->x;
      bottomVert->y = topVert->y;
      bottomVert->z = -0.5f;

      // calculate sides
      m_mesh.m_vertices[m_d.m_Sides*2 + 2 + i] = *topVert; // sideTopVert
      m_mesh.m_vertices[m_d.m_Sides*3 + 2 + i] = *bottomVert; // sideBottomVert

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
   middle = &m_mesh.m_vertices[0]; // middle point top
   middle->tu = 0.25f;   // /4
   middle->tv = 0.25f;   // /4
   middle = &m_mesh.m_vertices[m_d.m_Sides+1]; // middle point bottom
   middle->tu = (float)(0.25*3.); // /4*3
   middle->tv = 0.25f;   // /4
   const float invx = 0.5f/(maxX-minX);
   const float invy = 0.5f/(maxY-minY);
   const float invs = 1.0f/(float)m_d.m_Sides;
   for (int i = 0; i < m_d.m_Sides; i++)
   {
      Vertex3D_NoTex2 * const topVert = &m_mesh.m_vertices[i+1]; // top point at side
      topVert->tu = (topVert->x - minX)*invx;
      topVert->tv = (topVert->y - minY)*invy;

      Vertex3D_NoTex2 * const bottomVert = &m_mesh.m_vertices[i+1 + m_d.m_Sides+1]; // bottompoint at side
      bottomVert->tu = topVert->tu+0.5f;
      bottomVert->tv = topVert->tv;

      Vertex3D_NoTex2 * const sideTopVert = &m_mesh.m_vertices[m_d.m_Sides*2 + 2 + i];
      Vertex3D_NoTex2 * const sideBottomVert = &m_mesh.m_vertices[m_d.m_Sides*3 + 2 + i];

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
      m_mesh.m_indices.resize(m_d.m_Sides*24);
      // yes: draw everything twice
      // restore indices
      for (int i = 0; i < m_d.m_Sides; i++)
      {
         const int tmp = (i == m_d.m_Sides-1) ? 1 : (i+2); // wrapping around
         // top
         m_mesh.m_indices[i*6  ] = 0;
         m_mesh.m_indices[i*6+1] = i + 1;
         m_mesh.m_indices[i*6+2] = tmp;
         m_mesh.m_indices[i*6+3] = 0;
         m_mesh.m_indices[i*6+4] = tmp;
         m_mesh.m_indices[i*6+5] = i + 1;

         const int tmp2 = tmp+1;
         // bottom
         m_mesh.m_indices[6 * (i + m_d.m_Sides)    ] = m_d.m_Sides + 1;
         m_mesh.m_indices[6 * (i + m_d.m_Sides) + 1] = m_d.m_Sides + tmp2;
         m_mesh.m_indices[6 * (i + m_d.m_Sides) + 2] = m_d.m_Sides + 2 + i;
         m_mesh.m_indices[6 * (i + m_d.m_Sides) + 3] = m_d.m_Sides + 1;
         m_mesh.m_indices[6 * (i + m_d.m_Sides) + 4] = m_d.m_Sides + 2 + i;
         m_mesh.m_indices[6 * (i + m_d.m_Sides) + 5] = m_d.m_Sides + tmp2;

         // sides
         m_mesh.m_indices[12 * (i + m_d.m_Sides)    ] = m_d.m_Sides*2 + tmp2;
         m_mesh.m_indices[12 * (i + m_d.m_Sides) + 1] = m_d.m_Sides*2 + 2 + i;
         m_mesh.m_indices[12 * (i + m_d.m_Sides) + 2] = m_d.m_Sides*3 + 2 + i;
         m_mesh.m_indices[12 * (i + m_d.m_Sides) + 3] = m_d.m_Sides*2 + tmp2;
         m_mesh.m_indices[12 * (i + m_d.m_Sides) + 4] = m_d.m_Sides*3 + 2 + i;
         m_mesh.m_indices[12 * (i + m_d.m_Sides) + 5] = m_d.m_Sides*3 + tmp2;
         m_mesh.m_indices[12 * (i + m_d.m_Sides) + 6] = m_d.m_Sides*2 + tmp2;
         m_mesh.m_indices[12 * (i + m_d.m_Sides) + 7] = m_d.m_Sides*3 + 2 + i;
         m_mesh.m_indices[12 * (i + m_d.m_Sides) + 8] = m_d.m_Sides*2 + 2 + i;
         m_mesh.m_indices[12 * (i + m_d.m_Sides) + 9] = m_d.m_Sides*2 + tmp2;
         m_mesh.m_indices[12 * (i + m_d.m_Sides) + 10]= m_d.m_Sides*3 + tmp2;
         m_mesh.m_indices[12 * (i + m_d.m_Sides) + 11]= m_d.m_Sides*3 + 2 + i;
      }
   } else {
      // no: only out-facing polygons
      // restore indices
      m_mesh.m_indices.resize(m_d.m_Sides*12);
      for (int i = 0; i < m_d.m_Sides; i++)
      {
         const int tmp = (i == m_d.m_Sides-1) ? 1 : (i+2); // wrapping around
         // top
         m_mesh.m_indices[i*3  ] = 0;
         m_mesh.m_indices[i*3+2] = i + 1;
         m_mesh.m_indices[i*3+1] = tmp;

         SetNormal(&m_mesh.m_vertices[0], &m_mesh.m_indices[i+3], 3);

         const int tmp2 = tmp+1;
         // bottom
         m_mesh.m_indices[3 * (i + m_d.m_Sides)    ] = m_d.m_Sides + 1;
         m_mesh.m_indices[3 * (i + m_d.m_Sides) + 1] = m_d.m_Sides + 2 + i;
         m_mesh.m_indices[3 * (i + m_d.m_Sides) + 2] = m_d.m_Sides + tmp2;

         SetNormal(&m_mesh.m_vertices[0], &m_mesh.m_indices[3*(i+m_d.m_Sides)], 3);

         // sides
         m_mesh.m_indices[6 * (i + m_d.m_Sides)    ] = m_d.m_Sides*2 + tmp2;
         m_mesh.m_indices[6 * (i + m_d.m_Sides) + 1] = m_d.m_Sides*3 + 2 + i;
         m_mesh.m_indices[6 * (i + m_d.m_Sides) + 2] = m_d.m_Sides*2 + 2 + i;
         m_mesh.m_indices[6 * (i + m_d.m_Sides) + 3] = m_d.m_Sides*2 + tmp2;
         m_mesh.m_indices[6 * (i + m_d.m_Sides) + 4] = m_d.m_Sides*3 + tmp2;
         m_mesh.m_indices[6 * (i + m_d.m_Sides) + 5] = m_d.m_Sides*3 + 2 + i;
      }
   }

   // BUG: SetNormal only works for plane polygons
   //SetNormal(&m_mesh.m_vertices[0], &m_mesh.m_indices[0], m_mesh.NumIndices());
}

#if 0
void Primitive::UpdateMeshBuiltin()
{
   // 1 copy vertices
   objMesh = m_mesh.m_vertices;

   // 2 apply matrix trafo
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
            zMultiplicator*objMesh[m_mesh.m_indices[i*3  ]].z+
            zMultiplicator*objMesh[m_mesh.m_indices[i*3+1]].z+
            zMultiplicator*objMesh[m_mesh.m_indices[i*3+2]].z+
            yMultiplicator*objMesh[m_mesh.m_indices[i*3  ]].y+
            yMultiplicator*objMesh[m_mesh.m_indices[i*3+1]].y+
            yMultiplicator*objMesh[m_mesh.m_indices[i*3+2]].y;
      }
   } else {
      const float zM13 = (float)(1.0/3.0) * zMultiplicator;
      const float yM13 = (float)(1.0/3.0) * yMultiplicator;
      // top and bottom
      for (int i = 0; i < m_d.m_Sides * 2; i++)
      {
         builtin_depth[i] = 
            (objMesh[m_mesh.m_indices[i*6  ]].z+
             objMesh[m_mesh.m_indices[i*6+1]].z+
             objMesh[m_mesh.m_indices[i*6+2]].z) 
            * zM13 +
            (objMesh[m_mesh.m_indices[i*6  ]].y+
             objMesh[m_mesh.m_indices[i*6+1]].y+
             objMesh[m_mesh.m_indices[i*6+2]].y) 
            * yM13;
      }

      const float zM05 = 0.5f * zMultiplicator;
      const float yM05 = 0.5f * yMultiplicator;
      for (int i = m_d.m_Sides; i < m_d.m_Sides * 2; i++)
      {
         builtin_depth[i*2] = 
         builtin_depth[i*2+1] = 
            (objMesh[m_mesh.m_indices[i*12  ]].z+
             objMesh[m_mesh.m_indices[i*12+1]].z)
            * zM05 +
            (objMesh[m_mesh.m_indices[i*12  ]].y+
             objMesh[m_mesh.m_indices[i*12+1]].y)
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
               tempIndices[tempI] = m_mesh.m_indices[i*6 + tempI];

            int j = i;
            while ((j >= inc) && (builtin_depth[j-inc] > tempDepth))
            {
               builtin_depth[j] = builtin_depth[j-inc];
               for (int tempI = 0; tempI < 6; tempI++)
                  m_mesh.m_indices[j*6+tempI] = m_mesh.m_indices[(j-inc)*6 + tempI];
               j -= inc;
            }

            builtin_depth[j] = tempDepth;
            for (int tempI = 0; tempI < 6; tempI++)
               m_mesh.m_indices[j*6+tempI] = tempIndices[tempI];
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
#endif

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
   objMesh = m_mesh.m_vertices;

   if ( m_d.sphereMapping )
   {
      Matrix3D matView = g_pplayer->m_pin3d.GetViewTransform();
      matView.Multiply(rotMatrix, rotMatrix);
      //!! TODO/BUG: this should compute the inverse transpose of the rotational part
   }

   for (unsigned i = 0; i < m_mesh.NumVertices(); i++)
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
   }

   Vertex3D_NoTex2 *buf;
   vertexBuffer->lock(0,0,(void**)&buf, VertexBuffer::WRITEONLY);
   memcpy( buf, &objMesh[0], sizeof(Vertex3D_NoTex2)*objMesh.size() );
   vertexBuffer->unlock();
}

void Primitive::UpdateEditorView()
{
    RecalculateMatrices();
    TransformVertices();
}

void Primitive::RenderObject( RenderDevice *pd3dDevice )
{
    RecalculateMatrices();

    if (vertexBufferRegenerate)
    {
        vertexBufferRegenerate = false;
        UpdateMesh();
    }
    pd3dDevice->SetVertexDeclaration( pd3dDevice->m_pVertexNormalTexelDeclaration );

    Material *mat = m_ptable->GetMaterial( m_d.m_szMaterial);
    pd3dDevice->basicShader->SetMaterial(mat);
    pd3dDevice->basicShader->Core()->SetFloat("fmaterialAlpha",1.0f);

    Texture * const pin = m_ptable->GetImage(m_d.m_szImage);

    if (pin)
    {
        pin->CreateAlphaChannel();
        pd3dDevice->basicShader->SetTexture("Texture0", pin);
        pd3dDevice->basicShader->Core()->SetTechnique("basic_with_texture");
        pd3dDevice->SetRenderState(RenderDevice::ALPHABLENDENABLE, TRUE);
        pd3dDevice->SetRenderState(RenderDevice::SRCBLEND,  D3DBLEND_SRCALPHA);
        pd3dDevice->basicShader->Core()->SetBool("bPerformAlphaTest", true);
        pd3dDevice->basicShader->Core()->SetFloat("fAlphaTestValue", 128.0f/255.0f);

        g_pplayer->m_pin3d.SetTextureFilter(ePictureTexture, TEXTURE_MODE_TRILINEAR);
        // accomodate models with UV coords outside of [0,1]
        pd3dDevice->SetTextureAddressMode(ePictureTexture, RenderDevice::TEX_WRAP);
    }
    else
    {
        pd3dDevice->basicShader->Core()->SetTechnique("basic_without_texture");
    }

    // set transform
    g_pplayer->UpdateBasicShaderMatrix(fullMatrix);

    pd3dDevice->basicShader->Begin(0);
    // draw the mesh
    pd3dDevice->DrawIndexedPrimitiveVB( D3DPT_TRIANGLELIST, vertexBuffer, 0, m_mesh.NumVertices(), indexBuffer, 0, m_mesh.NumIndices() );
    pd3dDevice->basicShader->End();

    // reset transform
    g_pplayer->UpdateBasicShaderMatrix();

    pd3dDevice->SetTextureAddressMode(ePictureTexture, RenderDevice::TEX_CLAMP);
    pd3dDevice->basicShader->Core()->SetBool("bPerformAlphaTest", false);
    pd3dDevice->SetRenderState(RenderDevice::ALPHABLENDENABLE, FALSE);

}

// Always called each frame to render over everything else (along with alpha ramps)
void Primitive::PostRenderStatic(RenderDevice* pd3dDevice)
{
    TRACE_FUNCTION();
   if ( m_d.staticRendering || !m_d.m_fVisible )
      return;

   RenderObject( pd3dDevice );
}

void Primitive::RenderSetup( RenderDevice* pd3dDevice )
{
   if( !vertexBuffer )
      pd3dDevice->CreateVertexBuffer( m_mesh.NumVertices(), 0, MY_D3DFVF_NOTEX2_VERTEX, &vertexBuffer );

   indexBuffer = pd3dDevice->CreateAndFillIndexBuffer( m_mesh.m_indices );

   // make sure alpha channel is set up
   Texture * const tex = m_ptable->GetImage(m_d.m_szImage);
   if (tex)
       tex->CreateAlphaChannel();
}

void Primitive::RenderStatic(RenderDevice* pd3dDevice)
{
   if( m_d.staticRendering )
   {
      RenderObject(pd3dDevice);
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

   UpdateEditorView();
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

   UpdateEditorView();
   m_ptable->SetDirtyDraw();
}

//////////////////////////////
// Save and Load
//////////////////////////////

HRESULT Primitive::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   BiffWriter bw(pstm, hcrypthash, hcryptkey);

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
   bw.WriteString(FID(MATR), m_d.m_szMaterial);
   bw.WriteInt(FID(SCOL), m_d.m_SideColor);
   bw.WriteBool(FID(TVIS), m_d.m_fVisible);
   bw.WriteBool(FID(DTXI), m_d.m_DrawTexturesInside);
   bw.WriteBool(FID(HTEV), m_d.m_fHitEvent);
   bw.WriteFloat(FID(THRS), m_d.m_threshold);
   bw.WriteFloat(FID(ELAS), m_d.m_elasticity);
   bw.WriteFloat(FID(RFCT), m_d.m_friction);
   bw.WriteFloat(FID(RSCT), m_d.m_scatter);
   bw.WriteBool(FID(CLDRP), m_d.m_fCollidable);
   bw.WriteBool(FID(ISTO), m_d.m_fToy);
   bw.WriteBool(FID(ENLI), m_d.useLighting);
   bw.WriteBool(FID(U3DM), m_d.use3DMesh);
   bw.WriteBool(FID(STRE), m_d.staticRendering);
   bw.WriteBool(FID(EVMP), m_d.sphereMapping);
   if( m_d.use3DMesh )
   {
      bw.WriteString( FID(M3DN), m_d.meshFileName);
      bw.WriteInt( FID(M3VN), (int)m_mesh.NumVertices() );
      bw.WriteStruct( FID(M3DX), &m_mesh.m_vertices[0], (int)(sizeof(Vertex3D_NoTex2)*m_mesh.NumVertices()) );
      bw.WriteInt( FID(M3FN), (int)m_mesh.NumIndices() );
      bw.WriteStruct( FID(M3DI), &m_mesh.m_indices[0], (int)(sizeof(WORD)*m_mesh.NumIndices()) );
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
   UpdateEditorView();
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
   else if (id == FID(MATR))
   {
      pbr->GetString(m_d.m_szMaterial);
   }
   else if (id == FID(SCOL))
   {
      pbr->GetInt(&m_d.m_SideColor);
   }
   else if (id == FID(TVIS))
   {
      pbr->GetBool(&m_d.m_fVisible);
   }
   else if (id == FID(DTXI))
   {
      pbr->GetBool(&m_d.m_DrawTexturesInside);
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
      pbr->GetBool(&m_d.m_fCollidable);
   }
   else if (id == FID(ISTO))
   {
      pbr->GetBool(&m_d.m_fToy);
   }
   else if (id == FID(ENLI))
   {
      pbr->GetBool(&m_d.useLighting);
   }
   else if (id == FID(STRE))
   {
      pbr->GetBool(&m_d.staticRendering);
   }
   else if (id == FID(EVMP))
   {
      pbr->GetBool(&m_d.sphereMapping);
   }
   else if ( id == FID(U3DM))
   {
      pbr->GetBool(&m_d.use3DMesh);
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
      m_mesh.m_vertices.clear();
      m_mesh.m_vertices.resize(numVertices);
      pbr->GetStruct( &m_mesh.m_vertices[0], sizeof(Vertex3D_NoTex2)*numVertices);
   }
   else if( id == FID(M3FN) )
   {
      pbr->GetInt( &numIndices );
   }
   else if( id == FID(M3DI) )
   {
      m_mesh.m_indices.resize( numIndices );
      pbr->GetStruct( &m_mesh.m_indices[0], sizeof(WORD)*numIndices);
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
    if( !m_d.use3DMesh )
        CalculateBuiltinOriginal();

    UpdateEditorView();

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

   const int ret = GetOpenFileName(&ofn);
   string filename(ofn.lpstrFile);
   size_t index = filename.find_last_of("\\");
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
   m_mesh.Clear();
   objMesh.clear();
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
   if (m_mesh.LoadWavefrontObj(ofn.lpstrFile, flipTV, convertToLeftHanded))
   {
      m_d.use3DMesh=true;
      UpdateEditorView();
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

   const int ret = GetSaveFileName(&ofn);
   if(ret == 0)
   {
      return;
   }
   m_mesh.SaveWavefrontObj(ofn.lpstrFile, m_d.use3DMesh ? m_d.meshFileName : "Primitive");
}

bool Primitive::IsTransparent()
{
    const Texture * const tex = m_ptable->GetImage(m_d.m_szImage);
    return tex && tex->m_fTransparent;
}

float Primitive::GetDepth(const Vertex3Ds& viewDir) const
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
    if (newVal > Max_Primitive_Sides)
        newVal = Max_Primitive_Sides;

    if (m_d.m_Sides != newVal)
    {
        STARTUNDO

        m_d.m_Sides = newVal;
        if (!m_d.use3DMesh)
        {
            vertexBufferRegenerate = true;
            CalculateBuiltinOriginal();
            RecalculateMatrices();
            TransformVertices();
        }

        STOPUNDO
    }

    return S_OK;
}

STDMETHODIMP Primitive::get_Material(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_szMaterial, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Primitive::put_Material(BSTR newVal)
{
   STARTUNDO
   WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szMaterial, 32, NULL, NULL);
   STOPUNDO

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
	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Primitive::get_Visible(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fVisible);

   return S_OK;
}

STDMETHODIMP Primitive::put_Visible(VARIANT_BOOL newVal)
{
   STARTUNDO
   m_d.m_fVisible = VBTOF(newVal);
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

       if (!g_pplayer)
           UpdateEditorView();
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

       if (!g_pplayer)
           UpdateEditorView();
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

       if (!g_pplayer)
           UpdateEditorView();
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

       if (!g_pplayer)
           UpdateEditorView();
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

       if (!g_pplayer)
           UpdateEditorView();
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

       if (!g_pplayer)
           UpdateEditorView();
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

       if (!g_pplayer)
           UpdateEditorView();
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

       if (!g_pplayer)
           UpdateEditorView();
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

       if (!g_pplayer)
           UpdateEditorView();
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

       if (!g_pplayer)
           UpdateEditorView();
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

       if (!g_pplayer)
           UpdateEditorView();
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

       if (!g_pplayer)
           UpdateEditorView();
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

       if (!g_pplayer)
           UpdateEditorView();
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

       if (!g_pplayer)
           UpdateEditorView();
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

       if (!g_pplayer)
           UpdateEditorView();
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

       if (!g_pplayer)
           UpdateEditorView();
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

       if (!g_pplayer)
           UpdateEditorView();
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

       if (!g_pplayer)
           UpdateEditorView();
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

       if (!g_pplayer)
           UpdateEditorView();
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

       if (!g_pplayer)
           UpdateEditorView();
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

       if (!g_pplayer)
           UpdateEditorView();
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
