// primitive.cpp: implementation of the Primitive class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h" 

////////////////////////////////////////////////////////////////////////////////

HitTarget::HitTarget()
{
   vertexBuffer = 0;
   vertexBufferRegenerate = true;
   indexBuffer = 0;
   m_d.m_meshFileName[0] = 0;
   m_d.m_staticRendering = false;
   m_d.m_depthBias = 0.0f;
   m_d.m_fSkipRendering = false;
   m_d.m_fReflectionEnabled = true;
   m_numGroupIndices = 0;
   m_numGroupVertices = 0;

   numIndices = 0;
   numVertices = 0;
   m_propPhysics = NULL;
   m_propPosition = NULL;
   m_propVisual = NULL;
   memset(m_d.m_szImage, 0, MAXTOKEN);
   memset(m_d.m_szMaterial, 0, 32);
}

HitTarget::~HitTarget()
{
   if (vertexBuffer)
      vertexBuffer->release();
   if (indexBuffer)
      indexBuffer->release();
}


HRESULT HitTarget::Init(PinTable *ptable, float x, float y, bool fromMouseClick)
{
   m_ptable = ptable;

   m_d.m_vPosition.x = x;
   m_d.m_vPosition.y = y;

   SetDefaults(false);

   InitVBA(fTrue, 0, NULL);

   UpdateEditorView();

   return S_OK;
}

void HitTarget::SetDefaults(bool fromMouseClick)
{
   static const char strKeyName[] = "DefaultProps\\HitTarget";

   HRESULT hr;

   m_d.m_meshFileName[0] = 0;
   // sides
   m_d.m_Sides = fromMouseClick ? GetRegIntWithDefault(strKeyName, "Sides", 4) : 4;
   if (m_d.m_Sides > Max_Primitive_Sides)
      m_d.m_Sides = Max_Primitive_Sides;

   // colors
   m_d.m_SideColor = fromMouseClick ? GetRegIntWithDefault(strKeyName, "SideColor", RGB(150, 150, 150)) : RGB(150, 150, 150);

   m_d.m_fVisible = fromMouseClick ? GetRegBoolWithDefault(strKeyName, "Visible", true) : true;
   m_d.m_staticRendering = fromMouseClick ? GetRegBoolWithDefault(strKeyName, "StaticRendering", true) : true;

   // Position (X and Y is already set by the click of the user)
   m_d.m_vPosition.z = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "Position_Z", 0.0f) : 0.0f;

   // Size
   m_d.m_vSize.x = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "Size_X", 100.0f) : 100.0f;
   m_d.m_vSize.y = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "Size_Y", 100.0f) : 100.0f;
   m_d.m_vSize.z = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "Size_Z", 100.0f) : 100.0f;

   // Rotation and Transposition
   m_d.m_rotX = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "RotX", 0.0f) : 0.0f;
   m_d.m_rotY = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "RotY", 0.0f) : 0.0f;
   m_d.m_rotZ = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "RotZ", 0.0f) : 0.0f;

   hr = GetRegString(strKeyName, "Image", m_d.m_szImage, MAXTOKEN);
   if ((hr != S_OK) && fromMouseClick)
      m_d.m_szImage[0] = 0;

   m_d.m_threshold = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "HitThreshold", 2.0f) : 2.0f;

   SetDefaultPhysics(fromMouseClick);


   m_d.m_fCollidable = fromMouseClick ? GetRegBoolWithDefault(strKeyName, "Collidable", true) : true;
   m_d.m_fDisableLighting = fromMouseClick ? GetRegBoolWithDefault(strKeyName, "DisableLighting", false) : false;
   m_d.m_fReflectionEnabled = fromMouseClick ? GetRegBoolWithDefault(strKeyName, "ReflectionEnabled", true) : true;
}

void HitTarget::WriteRegDefaults()
{
   static const char strKeyName[] = "DefaultProps\\HitTarget";

   SetRegValueInt(strKeyName, "SideColor", m_d.m_SideColor);
   SetRegValueBool(strKeyName, "Visible", m_d.m_fVisible);
   SetRegValueBool(strKeyName, "StaticRendering", m_d.m_staticRendering);

   SetRegValueFloat(strKeyName, "Position_Z", m_d.m_vPosition.z);

   SetRegValueFloat(strKeyName, "Size_X", m_d.m_vSize.x);
   SetRegValueFloat(strKeyName, "Size_Y", m_d.m_vSize.y);
   SetRegValueFloat(strKeyName, "Size_Z", m_d.m_vSize.z);

   SetRegValueFloat(strKeyName, "RotX", m_d.m_rotX);
   SetRegValueFloat(strKeyName, "RotY", m_d.m_rotY);
   SetRegValueFloat(strKeyName, "RotZ", m_d.m_rotZ);

   SetRegValueString(strKeyName, "Image", m_d.m_szImage);
   SetRegValueBool(strKeyName, "HitEvent", m_d.m_fHitEvent);
   SetRegValueFloat(strKeyName, "HitThreshold", m_d.m_threshold);
   SetRegValueFloat(strKeyName, "Elasticity", m_d.m_elasticity);
   SetRegValueFloat(strKeyName, "ElasticityFalloff", m_d.m_elasticityFalloff);
   SetRegValueFloat(strKeyName, "Friction", m_d.m_friction);
   SetRegValueFloat(strKeyName, "Scatter", m_d.m_scatter);


   SetRegValueBool(strKeyName, "Collidable", m_d.m_fCollidable);
   SetRegValueBool(strKeyName, "DisableLighting", m_d.m_fDisableLighting);
   SetRegValueBool(strKeyName, "ReflectionEnabled", m_d.m_fReflectionEnabled);
}

void HitTarget::GetTimers(Vector<HitTimer> * const pvht)
{
   IEditable::BeginPlay();
}

void HitTarget::GetHitShapes(Vector<HitObject> * const pvho)
{
   RecalculateMatrices();
   TransformVertices(); //!! could also only do this for the optional reduced variant!

    std::set< std::pair<unsigned, unsigned> > addedEdges;

    // add collision triangles and edges
    for (unsigned i = 0; i < m_mesh.NumIndices(); i += 3)
    {
        const unsigned int i0 = m_mesh.m_indices[i];
        const unsigned int i1 = m_mesh.m_indices[i + 1];
        const unsigned int i2 = m_mesh.m_indices[i + 2];

        Vertex3Ds rgv3D[3];
        // NB: HitTriangle wants CCW vertices, but for rendering we have them in CW order
        rgv3D[0] = vertices[i0];
        rgv3D[1] = vertices[i2];
        rgv3D[2] = vertices[i1];
        SetupHitObject(pvho, new HitTriangle(rgv3D));

        AddHitEdge(pvho, addedEdges, i0, i1, rgv3D[0], rgv3D[2]);
        AddHitEdge(pvho, addedEdges, i1, i2, rgv3D[2], rgv3D[1]);
        AddHitEdge(pvho, addedEdges, i2, i0, rgv3D[1], rgv3D[0]);
    }

    // add collision vertices
    for (unsigned i = 0; i < m_mesh.NumVertices(); ++i)
        SetupHitObject(pvho, new HitPoint(vertices[i]));
}

void HitTarget::GetHitShapesDebug(Vector<HitObject> * const pvho)
{
}

void HitTarget::AddHitEdge(Vector<HitObject> * pvho, std::set< std::pair<unsigned, unsigned> >& addedEdges, const unsigned i, const unsigned j, const Vertex3Ds &vi, const Vertex3Ds &vj)
{
   // create pair uniquely identifying the edge (i,j)
   std::pair<unsigned, unsigned> p(std::min(i, j), std::max(i, j));

   if (addedEdges.count(p) == 0)   // edge not yet added?
   {
      addedEdges.insert(p);
      SetupHitObject(pvho, new HitLine3D(vi, vj));
   }
}

void HitTarget::SetupHitObject(Vector<HitObject> * pvho, HitObject * obj)
{
   obj->m_elasticity = m_d.m_elasticity;
   obj->m_elasticityFalloff = m_d.m_elasticityFalloff;
   obj->SetFriction(m_d.m_friction);
   obj->m_scatter = ANGTORAD(m_d.m_scatter);
   obj->m_threshold = m_d.m_threshold;
   obj->m_ObjType = ePrimitive;
   obj->m_fEnabled = m_d.m_fCollidable;
   if (m_d.m_fHitEvent)
      obj->m_pfe = (IFireEvents *)this;
   obj->m_pe = this;

   pvho->AddElement(obj);
   m_vhoCollidable.AddElement(obj);	//remember hit components of primitive
}

void HitTarget::EndPlay()
{
   m_vhoCollidable.RemoveAllElements();

   if (vertexBuffer)
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
   m_d.m_fSkipRendering = false;
}

//////////////////////////////
// Calculation
//////////////////////////////

void HitTarget::RecalculateMatrices()
{
   // scale matrix
   Matrix3D Smatrix;
   Smatrix.SetScaling(m_d.m_vSize.x, m_d.m_vSize.y, m_d.m_vSize.z);

   // translation matrix
   Matrix3D Tmatrix;
   Tmatrix.SetTranslation(m_d.m_vPosition.x, m_d.m_vPosition.y, m_d.m_vPosition.z + m_ptable->m_tableheight);

   // translation + rotation matrix
   Matrix3D RTmatrix, tempMatrix;

   tempMatrix.RotateZMatrix(ANGTORAD(m_d.m_rotX));
   tempMatrix.Multiply(RTmatrix, RTmatrix);
   tempMatrix.RotateYMatrix(ANGTORAD(m_d.m_rotY));
   tempMatrix.Multiply(RTmatrix, RTmatrix);
   tempMatrix.RotateXMatrix(ANGTORAD(m_d.m_rotZ));
   tempMatrix.Multiply(RTmatrix, RTmatrix);

   fullMatrix = Smatrix;
   RTmatrix.Multiply(fullMatrix, fullMatrix);
   Tmatrix.Multiply(fullMatrix, fullMatrix);        // fullMatrix = Smatrix * RTmatrix * Tmatrix
   Smatrix.SetScaling(1.0f, 1.0f, m_ptable->m_BG_scalez[m_ptable->m_BG_current_set]);
   Smatrix.Multiply(fullMatrix, fullMatrix);
}

// recalculate vertices for editor display
void HitTarget::TransformVertices()
{
   vertices.resize(m_mesh.NumVertices());
   normals.resize(m_mesh.NumVertices());

   for (unsigned i = 0; i < m_mesh.NumVertices(); i++)
   {
      fullMatrix.MultiplyVector(m_mesh.m_vertices[i], vertices[i]);
      Vertex3Ds n;
      fullMatrix.MultiplyVectorNoTranslateNormal(m_mesh.m_vertices[i], n);
      n.Normalize();
      normals[i] = n.z;
   }
}

//////////////////////////////
// Rendering
//////////////////////////////

//2d
void HitTarget::PreRender(Sur * const psur)
{
}

void HitTarget::Render(Sur * const psur)
{
   psur->SetLineColor(RGB(0, 0, 0), false, 1);
   psur->SetObject(this);

    for (unsigned i = 0; i < m_mesh.NumIndices(); i += 3)
    {
    const Vertex3Ds * const A = &vertices[m_mesh.m_indices[i]];
    const Vertex3Ds * const B = &vertices[m_mesh.m_indices[i + 1]];
    const Vertex3Ds * const C = &vertices[m_mesh.m_indices[i + 2]];
    psur->Line(A->x, A->y, B->x, B->y);
    psur->Line(B->x, B->y, C->x, C->y);
    psur->Line(C->x, C->y, A->x, A->y);
    }

   // draw center marker
   psur->SetLineColor(RGB(128, 128, 128), false, 1);
   psur->Line(m_d.m_vPosition.x - 10.0f, m_d.m_vPosition.y, m_d.m_vPosition.x + 10.0f, m_d.m_vPosition.y);
   psur->Line(m_d.m_vPosition.x, m_d.m_vPosition.y - 10.0f, m_d.m_vPosition.x, m_d.m_vPosition.y + 10.0f);
}

void HitTarget::CalculateBuiltinOriginal()
{
   // this recalculates the Original Vertices -> should be only called, when sides are altered.
   const float outerRadius = -0.5f / (cosf((float)M_PI / (float)m_d.m_Sides));
   const float addAngle = (float)(2.0*M_PI) / (float)m_d.m_Sides;
   const float offsAngle = (float)M_PI / (float)m_d.m_Sides;
   float minX = FLT_MAX;
   float minY = FLT_MAX;
   float maxX = -FLT_MAX;
   float maxY = -FLT_MAX;

   m_mesh.m_vertices.resize(4 * m_d.m_Sides + 2);

   Vertex3D_NoTex2 *middle;
   middle = &m_mesh.m_vertices[0]; // middle point top
   middle->x = 0.0f;
   middle->y = 0.0f;
   middle->z = 0.5f;
   middle = &m_mesh.m_vertices[m_d.m_Sides + 1]; // middle point bottom
   middle->x = 0.0f;
   middle->y = 0.0f;
   middle->z = -0.5f;
   for (int i = 0; i < m_d.m_Sides; ++i)
   {
      // calculate Top
      Vertex3D_NoTex2 * const topVert = &m_mesh.m_vertices[i + 1]; // top point at side
      const float currentAngle = addAngle*(float)i + offsAngle;
      topVert->x = sinf(currentAngle)*outerRadius;
      topVert->y = cosf(currentAngle)*outerRadius;
      topVert->z = 0.5f;

      // calculate bottom
      Vertex3D_NoTex2 * const bottomVert = &m_mesh.m_vertices[i + 1 + m_d.m_Sides + 1]; // bottompoint at side
      bottomVert->x = topVert->x;
      bottomVert->y = topVert->y;
      bottomVert->z = -0.5f;

      // calculate sides
      m_mesh.m_vertices[m_d.m_Sides * 2 + 2 + i] = *topVert; // sideTopVert
      m_mesh.m_vertices[m_d.m_Sides * 3 + 2 + i] = *bottomVert; // sideBottomVert

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
   middle = &m_mesh.m_vertices[m_d.m_Sides + 1]; // middle point bottom
   middle->tu = (float)(0.25*3.); // /4*3
   middle->tv = 0.25f;   // /4
   const float invx = 0.5f / (maxX - minX);
   const float invy = 0.5f / (maxY - minY);
   const float invs = 1.0f / (float)m_d.m_Sides;
   for (int i = 0; i < m_d.m_Sides; i++)
   {
      Vertex3D_NoTex2 * const topVert = &m_mesh.m_vertices[i + 1]; // top point at side
      topVert->tu = (topVert->x - minX)*invx;
      topVert->tv = (topVert->y - minY)*invy;

      Vertex3D_NoTex2 * const bottomVert = &m_mesh.m_vertices[i + 1 + m_d.m_Sides + 1]; // bottompoint at side
      bottomVert->tu = topVert->tu + 0.5f;
      bottomVert->tv = topVert->tv;

      Vertex3D_NoTex2 * const sideTopVert = &m_mesh.m_vertices[m_d.m_Sides * 2 + 2 + i];
      Vertex3D_NoTex2 * const sideBottomVert = &m_mesh.m_vertices[m_d.m_Sides * 3 + 2 + i];

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

    // no: only out-facing polygons
    // restore indices
    m_mesh.m_indices.resize(m_d.m_Sides * 12);
    for (int i = 0; i < m_d.m_Sides; i++)
    {
        const int tmp = (i == m_d.m_Sides - 1) ? 1 : (i + 2); // wrapping around
        // top
        m_mesh.m_indices[i * 3] = 0;
        m_mesh.m_indices[i * 3 + 2] = i + 1;
        m_mesh.m_indices[i * 3 + 1] = tmp;

        //SetNormal(&m_mesh.m_vertices[0], &m_mesh.m_indices[i+3], 3); // see below

        const int tmp2 = tmp + 1;
        // bottom
        m_mesh.m_indices[3 * (i + m_d.m_Sides)] = m_d.m_Sides + 1;
        m_mesh.m_indices[3 * (i + m_d.m_Sides) + 1] = m_d.m_Sides + 2 + i;
        m_mesh.m_indices[3 * (i + m_d.m_Sides) + 2] = m_d.m_Sides + tmp2;

        //SetNormal(&m_mesh.m_vertices[0], &m_mesh.m_indices[3*(i+m_d.m_Sides)], 3); // see below

        // sides
        m_mesh.m_indices[6 * (i + m_d.m_Sides)] = m_d.m_Sides * 2 + tmp2;
        m_mesh.m_indices[6 * (i + m_d.m_Sides) + 1] = m_d.m_Sides * 3 + 2 + i;
        m_mesh.m_indices[6 * (i + m_d.m_Sides) + 2] = m_d.m_Sides * 2 + 2 + i;
        m_mesh.m_indices[6 * (i + m_d.m_Sides) + 3] = m_d.m_Sides * 2 + tmp2;
        m_mesh.m_indices[6 * (i + m_d.m_Sides) + 4] = m_d.m_Sides * 3 + tmp2;
        m_mesh.m_indices[6 * (i + m_d.m_Sides) + 5] = m_d.m_Sides * 3 + 2 + i;
    }

   //SetNormal(&m_mesh.m_vertices[0], &m_mesh.m_indices[0], m_mesh.NumIndices()); // SetNormal only works for plane polygons
   ComputeNormals(m_mesh.m_vertices, m_mesh.m_indices);
}

void HitTarget::UpdateEditorView()
{
   RecalculateMatrices();
   TransformVertices();
}

void HitTarget::RenderObject(RenderDevice *pd3dDevice)
{
   Material *mat = m_ptable->GetMaterial(m_d.m_szMaterial);
   pd3dDevice->basicShader->SetMaterial(mat);

   pd3dDevice->SetRenderState(RenderDevice::DEPTHBIAS, 0);
   pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);
#ifdef TWOSIDED_TRANSPARENCY
   pd3dDevice->SetRenderState(RenderDevice::CULLMODE, mat->m_bOpacityActive ? D3DCULL_CW : D3DCULL_CCW);
#else
   pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);
#endif

   if (m_d.m_fDisableLighting)
      pd3dDevice->basicShader->SetDisableLighting(m_d.m_fDisableLighting);

   Texture * const pin = m_ptable->GetImage(m_d.m_szImage);
   if (pin)
   {
      pd3dDevice->basicShader->SetTechnique("basic_with_texture");
      pd3dDevice->basicShader->SetTexture("Texture0", pin);
      pd3dDevice->basicShader->SetAlphaTestValue(pin->m_alphaTestValue * (float)(1.0 / 255.0));

      //g_pplayer->m_pin3d.SetTextureFilter(0, TEXTURE_MODE_TRILINEAR);
      // accomodate models with UV coords outside of [0,1]
      pd3dDevice->SetTextureAddressMode(0, RenderDevice::TEX_WRAP);
   }
   else
      pd3dDevice->basicShader->SetTechnique("basic_without_texture");

   // set transform
   g_pplayer->UpdateBasicShaderMatrix(fullMatrix);

   // draw the mesh
   pd3dDevice->basicShader->Begin(0);
   pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, vertexBuffer, 0, m_numGroupVertices, indexBuffer, 0, m_numGroupIndices);
   pd3dDevice->basicShader->End();

#ifdef TWOSIDED_TRANSPARENCY
   if(mat->m_bOpacityActive)
   {
       pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);
       pd3dDevice->basicShader->Begin(0);
       if (m_d.m_fGroupdRendering)
          pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, vertexBuffer, 0, m_numGroupVertices, indexBuffer, 0, m_numGroupIndices);
       else
          pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, vertexBuffer, 0, (DWORD)m_mesh.NumVertices(), indexBuffer, 0, (DWORD)m_mesh.NumIndices());
       pd3dDevice->basicShader->End();
   }
#endif

   // reset transform
      g_pplayer->UpdateBasicShaderMatrix();

   pd3dDevice->SetTextureAddressMode(0, RenderDevice::TEX_CLAMP);
   //g_pplayer->m_pin3d.DisableAlphaBlend(); //!! not necessary anymore
   if (m_d.m_fDisableLighting)
      pd3dDevice->basicShader->SetDisableLighting(false);
}

// Always called each frame to render over everything else (along with alpha ramps)
void HitTarget::PostRenderStatic(RenderDevice* pd3dDevice)
{
   TRACE_FUNCTION();

   if (m_d.m_staticRendering || !m_d.m_fVisible || m_d.m_fSkipRendering)
      return;
   if (m_ptable->m_fReflectionEnabled && !m_d.m_fReflectionEnabled)
      return;

   RenderObject(pd3dDevice);
}

void HitTarget::RenderSetup(RenderDevice* pd3dDevice)
{
   if (m_d.m_fSkipRendering)
      return;

   if (vertexBuffer)
      vertexBuffer->release();

   pd3dDevice->CreateVertexBuffer((unsigned int)m_mesh.NumVertices(), 0, MY_D3DFVF_NOTEX2_VERTEX, &vertexBuffer);

   if (indexBuffer)
      indexBuffer->release();
   indexBuffer = pd3dDevice->CreateAndFillIndexBuffer(m_mesh.m_indices);
}

void HitTarget::RenderStatic(RenderDevice* pd3dDevice)
{
   if (m_d.m_staticRendering && m_d.m_fVisible)
   {
      if (m_ptable->m_fReflectionEnabled && !m_d.m_fReflectionEnabled)
         return;

      RenderObject(pd3dDevice);
   }
}

//////////////////////////////
// Positioning
//////////////////////////////

void HitTarget::SetObjectPos()
{
   g_pvp->SetObjectPosCur(m_d.m_vPosition.x, m_d.m_vPosition.y);
}

void HitTarget::MoveOffset(const float dx, const float dy)
{
   m_d.m_vPosition.x += dx;
   m_d.m_vPosition.y += dy;

   UpdateEditorView();
   m_ptable->SetDirtyDraw();
}

void HitTarget::GetCenter(Vertex2D * const pv) const
{
   pv->x = m_d.m_vPosition.x;
   pv->y = m_d.m_vPosition.y;
}

void HitTarget::PutCenter(const Vertex2D * const pv)
{
   m_d.m_vPosition.x = pv->x;
   m_d.m_vPosition.y = pv->y;

   UpdateEditorView();
   m_ptable->SetDirtyDraw();
}

//////////////////////////////
// Save and Load
//////////////////////////////

HRESULT HitTarget::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   BiffWriter bw(pstm, hcrypthash, hcryptkey);

   /*
    * Someone decided that it was a good idea to write these vectors including
    * the fourth padding float that they used to have, so now we have to write
    * them padded to 4 floats to maintain compatibility.
    */
   bw.WriteVector3Padded(FID(VPOS), &m_d.m_vPosition);
   bw.WriteVector3Padded(FID(VSIZ), &m_d.m_vSize);
   bw.WriteFloat(FID(ROTX), m_d.m_rotX);
   bw.WriteFloat(FID(ROTY), m_d.m_rotY);
   bw.WriteFloat(FID(ROTZ), m_d.m_rotZ);
   bw.WriteString(FID(IMAG), m_d.m_szImage);
   bw.WriteInt(FID(SIDS), m_d.m_Sides);
   bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);
   bw.WriteString(FID(MATR), m_d.m_szMaterial);
   bw.WriteInt(FID(SCOL), m_d.m_SideColor);
   bw.WriteBool(FID(TVIS), m_d.m_fVisible);
   bw.WriteBool(FID(HTEV), m_d.m_fHitEvent);
   bw.WriteFloat(FID(THRS), m_d.m_threshold);
   bw.WriteFloat(FID(ELAS), m_d.m_elasticity);
   bw.WriteFloat(FID(ELFO), m_d.m_elasticityFalloff);
   bw.WriteFloat(FID(RFCT), m_d.m_friction);
   bw.WriteFloat(FID(RSCT), m_d.m_scatter);
   bw.WriteBool(FID(CLDRP), m_d.m_fCollidable);
   bw.WriteBool(FID(STRE), m_d.m_staticRendering);
   bw.WriteBool(FID(DILI), m_d.m_fDisableLighting);
   bw.WriteBool(FID(REEN), m_d.m_fReflectionEnabled);
   bw.WriteFloat(FID(PIDB), m_d.m_depthBias);

   ISelect::SaveData(pstm, hcrypthash, hcryptkey);

   bw.WriteTag(FID(ENDB));

   return S_OK;
}

HRESULT HitTarget::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   SetDefaults(false);

   BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);

   m_ptable = ptable;

   br.Load();

   UpdateEditorView();
   return S_OK;
}

BOOL HitTarget::LoadToken(int id, BiffReader *pbr)
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
   else if (id == FID(ROTX))
   {
      pbr->GetFloat(&m_d.m_rotX);
   }
   else if (id == FID(ROTY))
   {
      pbr->GetFloat(&m_d.m_rotY);
   }
   else if (id == FID(ROTZ))
   {
      pbr->GetFloat(&m_d.m_rotZ);
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
   else if (id == FID(REEN))
   {
      pbr->GetBool(&m_d.m_fReflectionEnabled);
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
   else if (id == FID(ELFO))
   {
      pbr->GetFloat(&m_d.m_elasticityFalloff);
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
   else if (id == FID(STRE))
   {
      pbr->GetBool(&m_d.m_staticRendering);
   }
   else if (id == FID(DILI))
   {
      pbr->GetBool(&m_d.m_fDisableLighting);
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

HRESULT HitTarget::InitPostLoad()
{
   UpdateEditorView();

   return S_OK;
}

//////////////////////////////
// Standard methods
//////////////////////////////

STDMETHODIMP HitTarget::get_Image(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_szImage, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP HitTarget::put_Image(BSTR newVal)
{
   char szImage[MAXTOKEN];
   WideCharToMultiByte(CP_ACP, 0, newVal, -1, szImage, 32, NULL, NULL);
   const Texture * const tex = m_ptable->GetImage(szImage);
   if(tex && tex->IsHDR())
   {
       ShowError("Cannot use a HDR image (.exr/.hdr) here");
       return E_FAIL;
   }

   STARTUNDO

   strcpy_s(m_d.m_szImage,szImage);

   STOPUNDO

   return S_OK;
}


bool HitTarget::IsTransparent()
{
   if (m_d.m_fSkipRendering)
      return false;

   Material *mat = m_ptable->GetMaterial(m_d.m_szMaterial);
   return mat->m_bOpacityActive;
}

float HitTarget::GetDepth(const Vertex3Ds& viewDir)
{
   return m_d.m_depthBias + m_d.m_vPosition.Dot(viewDir);
}

STDMETHODIMP HitTarget::get_Material(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_szMaterial, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP HitTarget::put_Material(BSTR newVal)
{
   STARTUNDO
      WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szMaterial, 32, NULL, NULL);
   STOPUNDO

      return S_OK;
}

STDMETHODIMP HitTarget::get_SideColor(OLE_COLOR *pVal)
{
   *pVal = m_d.m_SideColor;

   return S_OK;
}

STDMETHODIMP HitTarget::put_SideColor(OLE_COLOR newVal)
{
   if (m_d.m_SideColor != newVal)
   {
      STARTUNDO
         m_d.m_SideColor = newVal;
      STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP HitTarget::get_Visible(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fVisible);

   return S_OK;
}

STDMETHODIMP HitTarget::put_Visible(VARIANT_BOOL newVal)
{
   STARTUNDO
      m_d.m_fVisible = VBTOF(newVal);
   STOPUNDO
      return S_OK;
}

STDMETHODIMP HitTarget::get_X(float *pVal)
{
   *pVal = m_d.m_vPosition.x;

   return S_OK;
}

STDMETHODIMP HitTarget::put_X(float newVal)
{
   if (m_d.m_vPosition.x != newVal)
   {
      STARTUNDO
         m_d.m_vPosition.x = newVal;
      STOPUNDO

         if (!g_pplayer)
            UpdateEditorView();
   }

   return S_OK;
}

STDMETHODIMP HitTarget::get_Y(float *pVal)
{
   *pVal = m_d.m_vPosition.y;

   return S_OK;
}

STDMETHODIMP HitTarget::put_Y(float newVal)
{
   if (m_d.m_vPosition.y != newVal)
   {
      STARTUNDO
         m_d.m_vPosition.y = newVal;
      STOPUNDO

         if (!g_pplayer)
            UpdateEditorView();
   }

   return S_OK;
}

STDMETHODIMP HitTarget::get_Z(float *pVal)
{
   *pVal = m_d.m_vPosition.z;

   return S_OK;
}

STDMETHODIMP HitTarget::put_Z(float newVal)
{
   if (m_d.m_vPosition.z != newVal)
   {
      STARTUNDO
         m_d.m_vPosition.z = newVal;
      STOPUNDO

         if (!g_pplayer)
            UpdateEditorView();
   }

   return S_OK;
}

STDMETHODIMP HitTarget::get_Size_X(float *pVal)
{
   *pVal = m_d.m_vSize.x;

   return S_OK;
}

STDMETHODIMP HitTarget::put_Size_X(float newVal)
{
   if (m_d.m_vSize.x != newVal)
   {
      STARTUNDO
         m_d.m_vSize.x = newVal;
      STOPUNDO

         if (!g_pplayer)
            UpdateEditorView();
   }

   return S_OK;
}

STDMETHODIMP HitTarget::get_Size_Y(float *pVal)
{
   *pVal = m_d.m_vSize.y;

   return S_OK;
}

STDMETHODIMP HitTarget::put_Size_Y(float newVal)
{
   if (m_d.m_vSize.y != newVal)
   {
      STARTUNDO
         m_d.m_vSize.y = newVal;
      STOPUNDO

         if (!g_pplayer)
            UpdateEditorView();
   }

   return S_OK;
}

STDMETHODIMP HitTarget::get_Size_Z(float *pVal)
{
   *pVal = m_d.m_vSize.z;

   return S_OK;
}

STDMETHODIMP HitTarget::put_Size_Z(float newVal)
{
   if (m_d.m_vSize.z != newVal)
   {
      STARTUNDO
         m_d.m_vSize.z = newVal;
      STOPUNDO

         if (!g_pplayer)
            UpdateEditorView();
   }

   return S_OK;
}


STDMETHODIMP HitTarget::get_RotX(float *pVal)
{
   *pVal = m_d.m_rotX;
   return S_OK;
}

STDMETHODIMP HitTarget::put_RotX(float newVal)
{
   if (m_d.m_rotX != newVal)
   {
      STARTUNDO
         m_d.m_rotX = newVal;
      STOPUNDO

         if (!g_pplayer)
            UpdateEditorView();
   }

   return S_OK;
}

STDMETHODIMP HitTarget::get_RotY(float *pVal)
{
   *pVal = m_d.m_rotY;
   return S_OK;
}

STDMETHODIMP HitTarget::put_RotY(float newVal)
{
   if (m_d.m_rotY != newVal)
   {
      STARTUNDO
          m_d.m_rotY = newVal;
      STOPUNDO

         if (!g_pplayer)
            UpdateEditorView();
   }

   return S_OK;
}

STDMETHODIMP HitTarget::get_RotZ(float *pVal)
{
    *pVal = m_d.m_rotZ;
   return S_OK;
}

STDMETHODIMP HitTarget::put_RotZ(float newVal)
{
   if (m_d.m_rotZ != newVal)
   {
      STARTUNDO
         m_d.m_rotZ = newVal;
      STOPUNDO

         if (!g_pplayer)
            UpdateEditorView();
   }

   return S_OK;
}

STDMETHODIMP HitTarget::get_EnableStaticRendering(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_staticRendering);

   return S_OK;
}

STDMETHODIMP HitTarget::put_EnableStaticRendering(VARIANT_BOOL newVal)
{
   STARTUNDO

      m_d.m_staticRendering = VBTOF(newVal);

   STOPUNDO

      return S_OK;
}

STDMETHODIMP HitTarget::get_HasHitEvent(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fHitEvent);

   return S_OK;
}

STDMETHODIMP HitTarget::put_HasHitEvent(VARIANT_BOOL newVal)
{
   STARTUNDO

      m_d.m_fHitEvent = VBTOF(newVal);

   STOPUNDO

      return S_OK;
}

STDMETHODIMP HitTarget::get_Threshold(float *pVal)
{
   *pVal = m_d.m_threshold;

   return S_OK;
}

STDMETHODIMP HitTarget::put_Threshold(float newVal)
{
   STARTUNDO

      m_d.m_threshold = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP HitTarget::get_Elasticity(float *pVal)
{
   *pVal = m_d.m_elasticity;

   return S_OK;
}

STDMETHODIMP HitTarget::put_Elasticity(float newVal)
{
   STARTUNDO
      m_d.m_elasticity = newVal;
   STOPUNDO

      return S_OK;
}

STDMETHODIMP HitTarget::get_ElasticityFalloff(float *pVal)
{
   *pVal = m_d.m_elasticityFalloff;

   return S_OK;
}

STDMETHODIMP HitTarget::put_ElasticityFalloff(float newVal)
{
   STARTUNDO
      m_d.m_elasticityFalloff = newVal;
   STOPUNDO

      return S_OK;
}

STDMETHODIMP HitTarget::get_Friction(float *pVal)
{
   *pVal = m_d.m_friction;

   return S_OK;
}

STDMETHODIMP HitTarget::put_Friction(float newVal)
{
   STARTUNDO

      if (newVal > 1.0f) newVal = 1.0f;
      else if (newVal < 0.f) newVal = 0.f;

      m_d.m_friction = newVal;

      STOPUNDO

         return S_OK;
}

STDMETHODIMP HitTarget::get_Scatter(float *pVal)
{
   *pVal = m_d.m_scatter;

   return S_OK;
}

STDMETHODIMP HitTarget::put_Scatter(float newVal)
{
   STARTUNDO

      m_d.m_scatter = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP HitTarget::get_Collidable(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB((!g_pplayer) ? m_d.m_fCollidable : m_vhoCollidable.ElementAt(0)->m_fEnabled);

   return S_OK;
}

STDMETHODIMP HitTarget::put_Collidable(VARIANT_BOOL newVal)
{
   BOOL fNewVal = VBTOF(newVal);
   if (!g_pplayer)
   {
      STARTUNDO

         m_d.m_fCollidable = !!fNewVal;

      STOPUNDO
   }
   else
      for (int i = 0; i < m_vhoCollidable.Size(); i++)
         m_vhoCollidable.ElementAt(i)->m_fEnabled = VBTOF(fNewVal);	//copy to hit checking on entities composing the object 

   return S_OK;
}

STDMETHODIMP HitTarget::get_DisableLighting(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fDisableLighting);

   return S_OK;
}

STDMETHODIMP HitTarget::put_DisableLighting(VARIANT_BOOL newVal)
{
   STARTUNDO

      m_d.m_fDisableLighting = VBTOF(newVal);

   STOPUNDO

      return S_OK;
}

STDMETHODIMP HitTarget::get_ReflectionEnabled(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fReflectionEnabled);

   return S_OK;
}

STDMETHODIMP HitTarget::put_ReflectionEnabled(VARIANT_BOOL newVal)
{
   STARTUNDO

      m_d.m_fReflectionEnabled = VBTOF(newVal);

   STOPUNDO

      return S_OK;
}

void HitTarget::GetDialogPanes(Vector<PropertyPane> *pvproppane)
{
   PropertyPane *pproppane;

   pproppane = new PropertyPane(IDD_PROP_NAME, NULL);
   pvproppane->AddElement(pproppane);

   m_propVisual = new PropertyPane(IDD_PROPHITTARGET_VISUALS, IDS_VISUALS);
   pvproppane->AddElement(m_propVisual);

   m_propPosition = new PropertyPane(IDD_PROPHITTARGET_POSITION, IDS_POSITION_TRANSLATION);
   pvproppane->AddElement(m_propPosition);

   m_propPhysics = new PropertyPane(IDD_PROPHITTARGET_PHYSICS, IDS_PHYSICS);
   pvproppane->AddElement(m_propPhysics);
}

void HitTarget::UpdatePropertyPanes()
{
    /*
   if (m_propVisual == NULL || m_propPosition == NULL || m_propPhysics == NULL)
      return;

   if (m_d.m_use3DMesh) {
      EnableWindow(GetDlgItem(m_propVisual->dialogHwnd, 106), FALSE);
      EnableWindow(GetDlgItem(m_propVisual->dialogHwnd, 101), FALSE);
   }
   else {
      EnableWindow(GetDlgItem(m_propVisual->dialogHwnd, 106), TRUE);
      EnableWindow(GetDlgItem(m_propVisual->dialogHwnd, 101), TRUE);
   }

   if (m_d.m_fToy || !m_d.m_fCollidable)
   {
      EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 34), FALSE);
      EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 33), FALSE);
      EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 110), FALSE);
      EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 112), FALSE);
      if (m_d.m_fToy)
      {
         EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 481), FALSE);
         EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 111), FALSE);
      }
      else
      {
         EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 481), TRUE);
         EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 111), TRUE);
      }

      EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 114), FALSE);
      EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 115), FALSE);
   }
   else if (!m_d.m_fToy && m_d.m_fCollidable)
   {
      EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 34), TRUE);
      EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 111), TRUE);
      EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 481), TRUE);
      if (m_d.m_fHitEvent)
         EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 33), TRUE);
      else
         EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 33), FALSE);

      EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 110), TRUE);
      EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 112), TRUE);
      EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 114), TRUE);
      EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 115), TRUE);
   }
   */
}

void HitTarget::SetDefaultPhysics(bool fromMouseClick)
{
   static const char strKeyName[] = "DefaultProps\\HitTarget";
   m_d.m_elasticity = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "Elasticity", 0.3f) : 0.3f;
   m_d.m_elasticityFalloff = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "ElasticityFalloff", 0.5f) : 0.5f;
   m_d.m_friction = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "Friction", 0.3f) : 0.3f;
   m_d.m_scatter = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "Scatter", 0) : 0;
}

STDMETHODIMP HitTarget::get_DepthBias(float *pVal)
{
   *pVal = m_d.m_depthBias;

   return S_OK;
}

STDMETHODIMP HitTarget::put_DepthBias(float newVal)
{
   if (m_d.m_depthBias != newVal)
   {
      STARTUNDO

         m_d.m_depthBias = newVal;

      STOPUNDO
   }

   return S_OK;
}
