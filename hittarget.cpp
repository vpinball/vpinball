// primitive.cpp: implementation of the Primitive class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h" 
#include "meshes/dropTargetT2Mesh.h"
#include "meshes/dropTargetT3Mesh.h"
#include "meshes/dropTargetT4Mesh.h"
#include "meshes/hitTargetRoundMesh.h"
#include "meshes/hitTargetRectangleMesh.h"
#include "meshes/hitTargetFatRectangleMesh.h"
#include "meshes/hitTargetFatSquareMesh.h"

////////////////////////////////////////////////////////////////////////////////
const float HitTarget::DROP_TARGET_LIMIT = 52.0f;
HitTarget::HitTarget()
{
   vertexBuffer = 0;
   vertexBufferRegenerate = true;
   indexBuffer = 0;
   m_d.m_depthBias = 0.0f;
   m_d.m_fSkipRendering = false;
   m_d.m_fReflectionEnabled = true;

   numIndices = 0;
   numVertices = 0;
   m_propPhysics = NULL;
   m_propPosition = NULL;
   m_propVisual = NULL;
   memset(m_d.m_szImage, 0, MAXTOKEN);
   memset(m_d.m_szMaterial, 0, 32);
   m_vertices = NULL;
   m_indices = NULL;
   m_numIndices = 0;
   m_numVertices = 0;
   m_moveAnimation = false;
   m_moveDown = true;
   m_moveAnimationOffset = 0.0f;
   m_hitEvent = false;
   
}

HitTarget::~HitTarget()
{
   if (vertexBuffer)
      vertexBuffer->release();
   if (indexBuffer)
      indexBuffer->release();
}

void HitTarget::SetMeshType(const TargetType type)
{
    if (type == DropTargetBeveled)
    {
        m_vertices = hitTargetT2Mesh;
        m_indices = hitTargetT2Indices;
        m_numIndices = hitTargetT2NumFaces;
        m_numVertices = hitTargetT2Vertices;
    }
    if (type == DropTargetSimple)
    {
        m_vertices = hitTargetT3Mesh;
        m_indices = hitTargetT3Indices;
        m_numIndices = hitTargetT3NumFaces;
        m_numVertices = hitTargetT3Vertices;
    }
    if (type == DropTargetFlatSimple)
    {
        m_vertices = hitTargetT4Mesh;
        m_indices = hitTargetT4Indices;
        m_numIndices = hitTargetT4NumFaces;
        m_numVertices = hitTargetT4Vertices;
    }
    if (type == HitTargetRound)
    {
        m_vertices = hitTargetRoundMesh;
        m_indices = hitTargetRoundIndices;
        m_numIndices = hitTargetRoundNumFaces;
        m_numVertices = hitTargetRoundVertices;
    }
    if (type == HitTargetRectangle)
    {
        m_vertices = hitTargetRectangleMesh;
        m_indices = hitTargetRectangleIndices;
        m_numIndices = hitTargetRectangleNumFaces;
        m_numVertices = hitTargetRectangleVertices;
    }
    if (type == HitFatTargetRectangle)
    {
        m_vertices = hitFatTargetRectangleMesh;
        m_indices = hitFatTargetRectangleIndices;
        m_numIndices = hitFatTargetRectangleNumFaces;
        m_numVertices = hitFatTargetRectangleVertices;
    }
    if (type == HitFatTargetSquare)
    {
        m_vertices = hitFatTargetSquareMesh;
        m_indices = hitFatTargetSquareIndices;
        m_numIndices = hitFatTargetSquareNumFaces;
        m_numVertices = hitFatTargetSquareVertices;
    }
}

HRESULT HitTarget::Init(PinTable *ptable, float x, float y, bool fromMouseClick)
{
   m_ptable = ptable;

   m_d.m_vPosition.x = x;
   m_d.m_vPosition.y = y;

   SetDefaults(false);
   m_hitEvent = false;

   InitVBA(fTrue, 0, NULL);

   UpdateEditorView();

   return S_OK;
}

void HitTarget::SetDefaults(bool fromMouseClick)
{
   static const char strKeyName[] = "DefaultProps\\HitTarget";
   HRESULT hr;
   int iTmp;

   m_d.m_tdr.m_fTimerEnabled = fromMouseClick ? GetRegBoolWithDefault(strKeyName, "TimerEnabled", false) : false;
   m_d.m_tdr.m_TimerInterval = fromMouseClick ? GetRegIntWithDefault(strKeyName, "TimerInterval", 100) : 100;
   m_d.m_fUseHitEvent = fromMouseClick ? GetRegBoolWithDefault(strKeyName, "HitEvent", true) : true;
   m_d.m_fVisible = fromMouseClick ? GetRegBoolWithDefault(strKeyName, "Visible", true) : true;
   m_d.m_isDropped = fromMouseClick ? GetRegBoolWithDefault(strKeyName, "IsDropped", false) : false;
   
   // Position (X and Y is already set by the click of the user)
   m_d.m_vPosition.z = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "Position_Z", 0.0f) : 0.0f;

   // Size
   m_d.m_vSize.x = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "ScaleX", 32.0f) : 32.0f;
   m_d.m_vSize.y = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "ScaleY", 32.0f) : 32.0f;
   m_d.m_vSize.z = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "ScaleZ", 32.0f) : 32.0f;

   // Rotation and Transposition
   m_d.m_rotZ = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "Orientation", 0.0f) : 0.0f;

   hr = GetRegString(strKeyName, "Image", m_d.m_szImage, MAXTOKEN);
   if ((hr != S_OK) && fromMouseClick)
      m_d.m_szImage[0] = 0;

   hr = GetRegInt(strKeyName, "TargetType", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
       m_d.m_targetType = (enum TargetType)iTmp;
   else
       m_d.m_targetType = DropTargetSimple;

   m_d.m_threshold = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "HitThreshold", 2.0f) : 2.0f;
   if (m_d.m_targetType == DropTargetBeveled || m_d.m_targetType == DropTargetSimple || m_d.m_targetType == DropTargetFlatSimple )
       m_d.m_dropSpeed = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "DropSpeed", 0.5f) : 0.5f;
   else
       m_d.m_dropSpeed = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "DropSpeed", 0.2f) : 0.2f;

   SetDefaultPhysics(fromMouseClick);

   m_d.m_fCollidable = fromMouseClick ? GetRegBoolWithDefault(strKeyName, "Collidable", true) : true;
   m_d.m_fDisableLighting = fromMouseClick ? GetRegBoolWithDefault(strKeyName, "DisableLighting", false) : false;
   m_d.m_fReflectionEnabled = fromMouseClick ? GetRegBoolWithDefault(strKeyName, "ReflectionEnabled", true) : true;
}

void HitTarget::WriteRegDefaults()
{
   static const char strKeyName[] = "DefaultProps\\HitTarget";

   SetRegValueBool(strKeyName, "TimerEnabled", m_d.m_tdr.m_fTimerEnabled);
   SetRegValueInt(strKeyName, "TimerInterval", m_d.m_tdr.m_TimerInterval);
   SetRegValueBool(strKeyName, "Visible", m_d.m_fVisible);
   SetRegValueBool(strKeyName, "IsDropped", m_d.m_isDropped);

   SetRegValueFloat(strKeyName, "Position_Z", m_d.m_vPosition.z);
   SetRegValueFloat(strKeyName, "DropSpeed", m_d.m_dropSpeed);

   SetRegValueFloat(strKeyName, "ScaleX", m_d.m_vSize.x);
   SetRegValueFloat(strKeyName, "ScaleY", m_d.m_vSize.y);
   SetRegValueFloat(strKeyName, "ScaleZ", m_d.m_vSize.z);

   SetRegValueFloat(strKeyName, "Orientation", m_d.m_rotZ);

   SetRegValueString(strKeyName, "Image", m_d.m_szImage);
   SetRegValueBool(strKeyName, "HitEvent", m_d.m_fUseHitEvent);
   SetRegValueFloat(strKeyName, "HitThreshold", m_d.m_threshold);
   SetRegValueFloat(strKeyName, "Elasticity", m_d.m_elasticity);
   SetRegValueFloat(strKeyName, "ElasticityFalloff", m_d.m_elasticityFalloff);
   SetRegValueFloat(strKeyName, "Friction", m_d.m_friction);
   SetRegValueFloat(strKeyName, "Scatter", m_d.m_scatter);

   SetRegValue(strKeyName, "TargetType", REG_DWORD, &m_d.m_targetType, 4);

   SetRegValueBool(strKeyName, "Collidable", m_d.m_fCollidable);
   SetRegValueBool(strKeyName, "DisableLighting", m_d.m_fDisableLighting);
   SetRegValueBool(strKeyName, "ReflectionEnabled", m_d.m_fReflectionEnabled);
}

void HitTarget::GetHitShapes(Vector<HitObject> * const pvho)
{
   TransformVertices(); //!! could also only do this for the optional reduced variant!

    std::set< std::pair<unsigned, unsigned> > addedEdges;

    // add collision triangles and edges
    for (unsigned i = 0; i < m_numIndices; i += 3)
    {
        const unsigned int i0 = m_indices[i];
        const unsigned int i1 = m_indices[i + 1];
        const unsigned int i2 = m_indices[i + 2];

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
    for (unsigned i = 0; i < m_numVertices; ++i)
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
   obj->m_ObjType = eHitTarget;
   obj->m_fEnabled = m_d.m_fCollidable;
   if (m_d.m_fUseHitEvent)
      obj->m_pfe = (IFireEvents *)this;
   obj->m_objHitEvent = this;
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

void HitTarget::GenerateMesh(Vertex3D_NoTex2 *buf)
{
   Matrix3D fullMatrix,tempMatrix;
   SetMeshType(m_d.m_targetType);
   fullMatrix.SetIdentity();
   tempMatrix.SetIdentity();
   tempMatrix.RotateZMatrix(ANGTORAD(m_d.m_rotZ));
   tempMatrix.Multiply(fullMatrix, fullMatrix);

   for (unsigned int i = 0; i < m_numVertices; i++)
   {
      Vertex3Ds vert(m_vertices[i].x, m_vertices[i].y, m_vertices[i].z);
      vert = fullMatrix.MultiplyVector(vert);

      buf[i].x = vert.x*m_d.m_vSize.x + m_d.m_vPosition.x;
      buf[i].y = vert.y*m_d.m_vSize.y + m_d.m_vPosition.y;
      buf[i].z = vert.z*m_d.m_vSize.z*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set] + m_d.m_vPosition.z + m_ptable->m_tableheight;
      vert = Vertex3Ds(m_vertices[i].nx, m_vertices[i].ny, m_vertices[i].nz);
      vert = fullMatrix.MultiplyVectorNoTranslate(vert);
      buf[i].nx = vert.x;
      buf[i].ny = vert.y;
      buf[i].nz = vert.z;
      buf[i].tu = m_vertices[i].tu;
      buf[i].tv = m_vertices[i].tv;
   }
}

// recalculate vertices for editor display
void HitTarget::TransformVertices()
{
   Matrix3D fullMatrix, tempMatrix;
   
   SetMeshType(m_d.m_targetType);
   vertices.resize(m_numIndices);
   normals.resize(m_numVertices);
   fullMatrix.SetIdentity();
   tempMatrix.SetIdentity();
   tempMatrix.RotateZMatrix(ANGTORAD(m_d.m_rotZ));
   tempMatrix.Multiply(fullMatrix, fullMatrix);

   for (unsigned i = 0; i < m_numVertices; i++)
   {
      Vertex3Ds vert(m_vertices[i].x, m_vertices[i].y, m_vertices[i].z);
      vert = fullMatrix.MultiplyVector(vert);

      vertices[i].x = vert.x*m_d.m_vSize.x + m_d.m_vPosition.x;
      vertices[i].y = vert.y*m_d.m_vSize.y + m_d.m_vPosition.y;
      vertices[i].z = vert.z*m_d.m_vSize.z*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set] + m_d.m_vPosition.z + m_ptable->m_tableheight;
      vert = Vertex3Ds(m_vertices[i].nx, m_vertices[i].ny, m_vertices[i].nz);
      vert = fullMatrix.MultiplyVectorNoTranslate(vert);
      // only z is needed for collision
      normals[i] = vert.z;

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

    for (unsigned i = 0; i < m_numIndices; i += 3)
    {
       const Vertex3Ds * const A = &vertices[m_indices[i]];
       const Vertex3Ds * const B = &vertices[m_indices[i + 1]];
       const Vertex3Ds * const C = &vertices[m_indices[i + 2]];
       psur->Line(A->x, A->y, B->x, B->y);
       psur->Line(B->x, B->y, C->x, C->y);
       psur->Line(C->x, C->y, A->x, A->y);
    }

    if (m_selectstate == eNotSelected)
       return;

    const float radangle = ANGTORAD(m_d.m_rotZ-180.0f);
    const float halflength = 50.0f;
    const float len1 = halflength *0.5f;
    const float len2 = len1*0.5f;
    Vertex2D tmp;
    {
       const float sn = sinf(radangle);
       const float cs = cosf(radangle);

       // Draw Arrow
       psur->SetLineColor(RGB(255, 0, 0), false, 1);

       tmp.x = m_d.m_vPosition.x + sn*len1;
       tmp.y = m_d.m_vPosition.y - cs*len1;

       psur->Line(tmp.x, tmp.y,
          m_d.m_vPosition.x, m_d.m_vPosition.y);
       {
          const float arrowang = radangle + 0.6f;
          const float sn = sinf(arrowang);
          const float cs = cosf(arrowang);

          psur->Line(tmp.x, tmp.y,  m_d.m_vPosition.x + sn*len2, m_d.m_vPosition.y - cs*len2);
       }
      {
         const float arrowang = ANGTORAD(m_d.m_rotZ-180.0f) - 0.6f;
         const float sn = sinf(arrowang);
         const float cs = cosf(arrowang);

         psur->Line(tmp.x, tmp.y,
            m_d.m_vPosition.x + sn*len2, m_d.m_vPosition.y - cs*len2);
      }

    }
   // draw center marker
//    psur->SetLineColor(RGB(128, 128, 128), false, 1);
//    psur->Line(m_d.m_vPosition.x - 10.0f, m_d.m_vPosition.y, m_d.m_vPosition.x + 10.0f, m_d.m_vPosition.y);
//    psur->Line(m_d.m_vPosition.x, m_d.m_vPosition.y - 10.0f, m_d.m_vPosition.x, m_d.m_vPosition.y + 10.0f);
}

void HitTarget::UpdateEditorView()
{
   TransformVertices();
}

void HitTarget::UpdateAnimation(RenderDevice *pd3dDevice)
{
    const U32 old_time_msec = (m_d.m_time_msec < g_pplayer->m_time_msec) ? m_d.m_time_msec : g_pplayer->m_time_msec;
    m_d.m_time_msec = g_pplayer->m_time_msec;
    const float diff_time_msec = (float)(g_pplayer->m_time_msec - old_time_msec);

    if (m_hitEvent)
    {
        if (!m_d.m_isDropped)
        {
            m_moveDown = true;
        }
        m_moveAnimation = true;
        m_hitEvent = false;
    }
    if (m_d.m_targetType == DropTargetBeveled || m_d.m_targetType == DropTargetSimple || m_d.m_targetType == DropTargetFlatSimple)
    {
        if (m_moveAnimation)
        {
            float step = m_d.m_dropSpeed*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];
            const float limit = DROP_TARGET_LIMIT*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];
            if (m_moveDown)
                step = -step;
            m_moveAnimationOffset += step*diff_time_msec;
            if (m_moveDown)
            {
                if (m_moveAnimationOffset <= -limit)
                {
                    m_moveAnimationOffset = -limit;
                    m_moveDown = false;
                    m_d.m_isDropped = true;
                    m_moveAnimation = false;
                }
            }
            else
            {
                if (m_moveAnimationOffset >= 0.0f)
                {
                    m_moveAnimationOffset = 0.0f;
                    m_moveAnimation = false;
                    m_d.m_isDropped = false;
                }
            }
            UpdateTarget(pd3dDevice);
        }
    }
    else
    {
        if (m_moveAnimation)
        {
            float step = m_d.m_dropSpeed*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];
            const float limit = 13.0f*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];
            if (!m_moveDown)
                step = -step;
            m_moveAnimationOffset += step*diff_time_msec;
            if (m_moveDown)
            {
                if (m_moveAnimationOffset <= limit)
                {
                    m_moveAnimationOffset = limit;
                    m_moveDown = false;
                }
            }
            else
            {
                if (m_moveAnimationOffset <= 0.0f)
                {
                    m_moveAnimationOffset = 0.0f;
                    m_moveAnimation = false;
                }
            }
            UpdateTarget(pd3dDevice);
        }
    }
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

   UpdateAnimation(pd3dDevice);

   // draw the mesh
   pd3dDevice->basicShader->Begin(0);
   pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, vertexBuffer, 0, m_numVertices, indexBuffer, 0, m_numIndices);
   pd3dDevice->basicShader->End();

#ifdef TWOSIDED_TRANSPARENCY
   if(mat->m_bOpacityActive)
   {
       pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);
       pd3dDevice->basicShader->Begin(0);
       pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, vertexBuffer, 0, m_numVertices, indexBuffer, 0, m_numIndices);
       pd3dDevice->basicShader->End();
   }
#endif


   pd3dDevice->SetTextureAddressMode(0, RenderDevice::TEX_CLAMP);
   //g_pplayer->m_pin3d.DisableAlphaBlend(); //!! not necessary anymore
   if (m_d.m_fDisableLighting)
      pd3dDevice->basicShader->SetDisableLighting(false);
}

void HitTarget::UpdateTarget(RenderDevice *pd3dDevice)
{
   Vertex3D_NoTex2 *buf;
   vertexBuffer->lock(0, 0, (void**)&buf, VertexBuffer::DISCARDCONTENTS);
   if (m_d.m_targetType == DropTargetBeveled || m_d.m_targetType == DropTargetSimple || m_d.m_targetType == DropTargetFlatSimple)
   {
       for (unsigned int i = 0; i < m_numVertices; i++)
       {
           buf[i].x = transformedVertices[i].x;
           buf[i].y = transformedVertices[i].y;
           buf[i].z = transformedVertices[i].z + m_moveAnimationOffset;
           buf[i].nx = transformedVertices[i].nx;
           buf[i].ny = transformedVertices[i].ny;
           buf[i].nz = transformedVertices[i].nz;
           buf[i].tu = transformedVertices[i].tu;
           buf[i].tv = transformedVertices[i].tv;
       }
   }
   else
   {
       Matrix3D fullMatrix, tempMatrix;
       fullMatrix.SetIdentity();
       tempMatrix.SetIdentity();
       tempMatrix.RotateXMatrix(ANGTORAD(m_moveAnimationOffset));
       tempMatrix.Multiply(fullMatrix, fullMatrix);
       tempMatrix.RotateZMatrix(ANGTORAD(m_d.m_rotZ));
       tempMatrix.Multiply(fullMatrix, fullMatrix);

       for (unsigned int i = 0; i < m_numVertices; i++)
       {
           Vertex3Ds vert(m_vertices[i].x, m_vertices[i].y, m_vertices[i].z);
           vert = fullMatrix.MultiplyVector(vert);

           buf[i].x = vert.x*m_d.m_vSize.x + m_d.m_vPosition.x;
           buf[i].y = vert.y*m_d.m_vSize.y + m_d.m_vPosition.y;
           buf[i].z = vert.z*m_d.m_vSize.z*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set] + m_d.m_vPosition.z + m_ptable->m_tableheight;
           vert = Vertex3Ds(m_vertices[i].nx, m_vertices[i].ny, m_vertices[i].nz);
           vert = fullMatrix.MultiplyVectorNoTranslate(vert);
           buf[i].nx = vert.x;
           buf[i].ny = vert.y;
           buf[i].nz = vert.z;
           buf[i].tu = m_vertices[i].tu;
           buf[i].tv = m_vertices[i].tv;
       }
   }
   vertexBuffer->unlock();
}

// Always called each frame to render over everything else (along with alpha ramps)
void HitTarget::PostRenderStatic(RenderDevice* pd3dDevice)
{
   TRACE_FUNCTION();

   if (!m_d.m_fVisible || m_d.m_fSkipRendering)
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

   SetMeshType(m_d.m_targetType);
   pd3dDevice->CreateVertexBuffer((unsigned int)m_numVertices, 0, MY_D3DFVF_NOTEX2_VERTEX, &vertexBuffer);

   if (indexBuffer)
      indexBuffer->release();
   indexBuffer = pd3dDevice->CreateAndFillIndexBuffer(m_numIndices, m_indices);

   transformedVertices = new Vertex3D_NoTex2[m_numVertices];
   GenerateMesh(transformedVertices);
   if (m_d.m_targetType == DropTargetBeveled || m_d.m_targetType == DropTargetSimple || m_d.m_targetType == DropTargetFlatSimple)
   {
       if (m_d.m_isDropped)
       {
           m_moveDown = false;
           m_moveAnimationOffset = -DROP_TARGET_LIMIT*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];
           UpdateTarget(pd3dDevice);
           return;
       }
   }
   Vertex3D_NoTex2 *buf;
   vertexBuffer->lock(0, 0, (void**)&buf, 0);
   memcpy(buf, transformedVertices, m_numVertices*sizeof(Vertex3D_NoTex2));
   vertexBuffer->unlock();
}

void HitTarget::RenderStatic(RenderDevice* pd3dDevice)
{
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
   bw.WriteFloat(FID(ROTZ), m_d.m_rotZ);
   bw.WriteString(FID(IMAG), m_d.m_szImage);
   bw.WriteInt(FID(TRTY), m_d.m_targetType);
   bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);
   bw.WriteString(FID(MATR), m_d.m_szMaterial);
   bw.WriteBool(FID(TVIS), m_d.m_fVisible);
   bw.WriteBool(FID(HTEV), m_d.m_fUseHitEvent);
   bw.WriteFloat(FID(THRS), m_d.m_threshold);
   bw.WriteFloat(FID(ELAS), m_d.m_elasticity);
   bw.WriteFloat(FID(ELFO), m_d.m_elasticityFalloff);
   bw.WriteFloat(FID(RFCT), m_d.m_friction);
   bw.WriteFloat(FID(RSCT), m_d.m_scatter);
   bw.WriteBool(FID(CLDRP), m_d.m_fCollidable);
   bw.WriteBool(FID(DILI), m_d.m_fDisableLighting);
   bw.WriteBool(FID(REEN), m_d.m_fReflectionEnabled);
   bw.WriteFloat(FID(PIDB), m_d.m_depthBias);
   bw.WriteBool(FID(ISDR), m_d.m_isDropped);
   bw.WriteFloat(FID(DRSP), m_d.m_dropSpeed);
   bw.WriteBool(FID(TMON), m_d.m_tdr.m_fTimerEnabled);
   bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
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
   else if (id == FID(ROTZ))
   {
      pbr->GetFloat(&m_d.m_rotZ);
   }
   else if (id == FID(IMAG))
   {
      pbr->GetString(m_d.m_szImage);
   }
   else if (id == FID(TRTY))
   {
       pbr->GetInt(&m_d.m_targetType);
   }
   else if (id == FID(NAME))
   {
      pbr->GetWideString((WCHAR *)m_wzName);
   }
   else if (id == FID(MATR))
   {
      pbr->GetString(m_d.m_szMaterial);
   }
   else if (id == FID(TVIS))
   {
      pbr->GetBool(&m_d.m_fVisible);
   }
   else if (id == FID(ISDR))
   {
      pbr->GetBool(&m_d.m_isDropped);
   }
   else if (id == FID(DRSP))
   {
      pbr->GetFloat(&m_d.m_dropSpeed);
   }
   else if (id == FID(REEN))
   {
      pbr->GetBool(&m_d.m_fReflectionEnabled);
   }
   else if (id == FID(HTEV))
   {
      pbr->GetBool(&m_d.m_fUseHitEvent);
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
   else if (id == FID(DILI))
   {
      pbr->GetBool(&m_d.m_fDisableLighting);
   }
   else if (id == FID(PIDB))
   {
      pbr->GetFloat(&m_d.m_depthBias);
   }
   else if (id == FID(TMON))
   {
       pbr->GetBool(&m_d.m_tdr.m_fTimerEnabled);
   }
   else if (id == FID(TMIN))
   {
       pbr->GetInt(&m_d.m_tdr.m_TimerInterval);
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

STDMETHODIMP HitTarget::get_ScaleX(float *pVal)
{
   *pVal = m_d.m_vSize.x;

   return S_OK;
}

STDMETHODIMP HitTarget::put_ScaleX(float newVal)
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

STDMETHODIMP HitTarget::get_ScaleY(float *pVal)
{
   *pVal = m_d.m_vSize.y;

   return S_OK;
}

STDMETHODIMP HitTarget::put_ScaleY(float newVal)
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

STDMETHODIMP HitTarget::get_ScaleZ(float *pVal)
{
   *pVal = m_d.m_vSize.z;

   return S_OK;
}

STDMETHODIMP HitTarget::put_ScaleZ(float newVal)
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


STDMETHODIMP HitTarget::get_Orientation(float *pVal)
{
    *pVal = m_d.m_rotZ;
   return S_OK;
}

STDMETHODIMP HitTarget::put_Orientation(float newVal)
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

STDMETHODIMP HitTarget::get_HasHitEvent(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fUseHitEvent);

   return S_OK;
}

STDMETHODIMP HitTarget::put_HasHitEvent(VARIANT_BOOL newVal)
{
   STARTUNDO

      m_d.m_fUseHitEvent = VBTOF(newVal);

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

   pproppane = new PropertyPane(IDD_PROP_TIMER, IDS_MISC);
   pvproppane->AddElement(pproppane);

}

void HitTarget::UpdatePropertyPanes()
{
    
   if (m_propVisual == NULL || m_propPosition == NULL || m_propPhysics == NULL)
      return;

   if (!m_d.m_fCollidable)
   {
      EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 34), FALSE);
      EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 33), FALSE);
      EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 110), FALSE);
      EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 112), FALSE);
      EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 481), TRUE);
      EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 111), TRUE);

      EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 114), FALSE);
      EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 115), FALSE);
   }
   else if (m_d.m_fCollidable)
   {
      EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 34), TRUE);
      EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 111), TRUE);
      EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 481), TRUE);
      if (m_d.m_fUseHitEvent)
         EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 33), TRUE);
      else
         EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 33), FALSE);

      EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 110), TRUE);
      EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 112), TRUE);
      EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 114), TRUE);
      EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 115), TRUE);
   }

   if (m_d.m_targetType == HitTargetRectangle || m_d.m_targetType == HitTargetRound || m_d.m_targetType == HitFatTargetRectangle || m_d.m_targetType == HitFatTargetSquare)
       EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, IDC_TARGET_ISDROPPED_CHECK), FALSE);
   else
       EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, IDC_TARGET_ISDROPPED_CHECK), TRUE);

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
STDMETHODIMP HitTarget::get_DropSpeed(float *pVal)
{
   *pVal = m_d.m_dropSpeed;

   return S_OK;
}

STDMETHODIMP HitTarget::put_DropSpeed(float newVal)
{
   if (m_d.m_dropSpeed != newVal)
   {
      STARTUNDO

         m_d.m_dropSpeed = newVal;

      STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP HitTarget::get_IsDropped(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_isDropped);

   return S_OK;
}

STDMETHODIMP HitTarget::put_IsDropped(VARIANT_BOOL newVal)
{
   STARTUNDO
      const bool val = VBTOF(newVal);
   if (g_pplayer && m_d.m_isDropped != val)
   {
      if (val)
      {
         m_moveAnimation = true;
         m_moveAnimationOffset = 0.0f;
         m_moveDown = true;
      }
      else
      {
         m_moveAnimation = true;
         m_moveAnimationOffset = -DROP_TARGET_LIMIT*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];
         m_moveDown = false;
      }
   }
   else
      m_d.m_isDropped = val;

   STOPUNDO
      return S_OK;
}

STDMETHODIMP HitTarget::get_DrawStyle(TargetType *pVal)
{
    *pVal = m_d.m_targetType;

    return S_OK;
}

STDMETHODIMP HitTarget::put_DrawStyle(TargetType newVal)
{
    STARTUNDO

        m_d.m_targetType = newVal;
        UpdateEditorView();
    STOPUNDO

        return S_OK;
}

void HitTarget::GetTimers(Vector<HitTimer> * const pvht)
{
    IEditable::BeginPlay();

    HitTimer * const pht = new HitTimer();
    pht->m_interval = m_d.m_tdr.m_TimerInterval;
    pht->m_nextfire = pht->m_interval;
    pht->m_pfe = (IFireEvents *)this;

    m_phittimer = pht;

    if (m_d.m_tdr.m_fTimerEnabled)
        pvht->AddElement(pht);
}
