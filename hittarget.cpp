// primitive.cpp: implementation of the Primitive class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h" 
#include "objloader.h"
#include "meshes/dropTargetT2Mesh.h"
#include "meshes/dropTargetT3Mesh.h"
#include "meshes/dropTargetT4Mesh.h"
#include "meshes/hitTargetRoundMesh.h"
#include "meshes/hitTargetRectangleMesh.h"
#include "meshes/hitTargetFatRectangleMesh.h"
#include "meshes/hitTargetFatSquareMesh.h"
#include "meshes/hitTargetT1SlimMesh.h"
#include "meshes/hitTargetT2SlimMesh.h"


////////////////////////////////////////////////////////////////////////////////
const float HitTarget::DROP_TARGET_LIMIT = 52.0f;


HitTarget::HitTarget()
{
   m_vertexBuffer = 0;
   m_indexBuffer = 0;
   m_d.m_depthBias = 0.0f;
   m_d.m_reflectionEnabled = true;

   m_propPhysics = NULL;
   m_propPosition = NULL;
   m_propVisual = NULL;
   m_d.m_overwritePhysics = true;
   m_vertices = NULL;
   m_indices = NULL;
   m_numIndices = 0;
   m_numVertices = 0;
   m_moveAnimation = false;
   m_moveDown = true;
   m_moveAnimationOffset = 0.0f;
   m_hitEvent = false;
   m_ptable = NULL;
   m_timeStamp = 0;
}

HitTarget::~HitTarget()
{
   if (m_vertexBuffer)
      m_vertexBuffer->release();
   if (m_indexBuffer)
      m_indexBuffer->release();
}

void HitTarget::SetMeshType(const TargetType type)
{
    if (type == DropTargetBeveled)
    {
        m_vertices = hitTargetT2Mesh;
        m_indices = hitTargetT2Indices;
        m_numIndices = hitTargetT2NumIndices;
        m_numVertices = hitTargetT2Vertices;
    }
    else if (type == DropTargetSimple)
    {
        m_vertices = hitTargetT3Mesh;
        m_indices = hitTargetT3Indices;
        m_numIndices = hitTargetT3NumIndices;
        m_numVertices = hitTargetT3Vertices;
    }
    else if (type == DropTargetFlatSimple)
    {
        m_vertices = hitTargetT4Mesh;
        m_indices = hitTargetT4Indices;
        m_numIndices = hitTargetT4NumIndices;
        m_numVertices = hitTargetT4Vertices;
    }
    else if (type == HitTargetRound)
    {
        m_vertices = hitTargetRoundMesh;
        m_indices = hitTargetRoundIndices;
        m_numIndices = hitTargetRoundNumIndices;
        m_numVertices = hitTargetRoundVertices;
    }
    else if (type == HitTargetRectangle)
    {
        m_vertices = hitTargetRectangleMesh;
        m_indices = hitTargetRectangleIndices;
        m_numIndices = hitTargetRectangleNumIndices;
        m_numVertices = hitTargetRectangleVertices;
    }
    else if (type == HitFatTargetRectangle)
    {
        m_vertices = hitFatTargetRectangleMesh;
        m_indices = hitFatTargetRectangleIndices;
        m_numIndices = hitFatTargetRectangleNumIndices;
        m_numVertices = hitFatTargetRectangleVertices;
    }
    else if (type == HitFatTargetSquare)
    {
        m_vertices = hitFatTargetSquareMesh;
        m_indices = hitFatTargetSquareIndices;
        m_numIndices = hitFatTargetSquareNumIndices;
        m_numVertices = hitFatTargetSquareVertices;
    }
    else if (type == HitTargetSlim)
    {
        m_vertices = hitTargetT1SlimMesh;
        m_indices = hitTargetT1SlimIndices;
        m_numIndices = hitTargetT1SlimNumIndices;
        m_numVertices = hitTargetT1SlimVertices;
    }
    else if (type == HitFatTargetSlim)
    {
        m_vertices = hitTargetT2SlimMesh;
        m_indices = hitTargetT2SlimIndices;
        m_numIndices = hitTargetT2SlimNumIndices;
        m_numVertices = hitTargetT2SlimVertices;
    }
    else
    {
        // just in case if something went wrong with the target type
        // set it to the standard simple drop target to prevent crashing
        m_vertices = hitTargetT3Mesh;
        m_indices = hitTargetT3Indices;
        m_numIndices = hitTargetT3NumIndices;
        m_numVertices = hitTargetT3Vertices;
        m_d.m_targetType = DropTargetSimple;
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

   UpdateStatusBarInfo();

   return S_OK;
}

void HitTarget::SetDefaults(bool fromMouseClick)
{
   static const char strKeyName[] = "DefaultProps\\HitTarget";

   m_d.m_legacy = fromMouseClick ? LoadValueBoolWithDefault(strKeyName, "LegacyMode", false) : false;
   m_d.m_tdr.m_TimerEnabled = fromMouseClick ? LoadValueBoolWithDefault(strKeyName, "TimerEnabled", false) : false;
   m_d.m_tdr.m_TimerInterval = fromMouseClick ? LoadValueIntWithDefault(strKeyName, "TimerInterval", 100) : 100;
   m_d.m_useHitEvent = fromMouseClick ? LoadValueBoolWithDefault(strKeyName, "HitEvent", true) : true;
   m_d.m_visible = fromMouseClick ? LoadValueBoolWithDefault(strKeyName, "Visible", true) : true;
   m_d.m_isDropped = fromMouseClick ? LoadValueBoolWithDefault(strKeyName, "IsDropped", false) : false;
   
   // Position (X and Y is already set by the click of the user)
   m_d.m_vPosition.z = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "Position_Z", 0.0f) : 0.0f;

   // Size
   m_d.m_vSize.x = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "ScaleX", 32.0f) : 32.0f;
   m_d.m_vSize.y = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "ScaleY", 32.0f) : 32.0f;
   m_d.m_vSize.z = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "ScaleZ", 32.0f) : 32.0f;

   // Rotation and Transposition
   m_d.m_rotZ = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "Orientation", 0.0f) : 0.0f;

   char buf[MAXTOKEN] = { 0 };
   const HRESULT hr = LoadValueString(strKeyName, "Image", buf, MAXTOKEN);
   if ((hr != S_OK) && fromMouseClick)
      m_d.m_szImage.clear();
   else
      m_d.m_szImage = buf;

   m_d.m_targetType = fromMouseClick ? (TargetType)LoadValueIntWithDefault(strKeyName, "TargetType", DropTargetSimple) : DropTargetSimple;
   m_d.m_threshold = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "HitThreshold", 2.0f) : 2.0f;
   if (m_d.m_targetType == DropTargetBeveled || m_d.m_targetType == DropTargetSimple || m_d.m_targetType == DropTargetFlatSimple)
       m_d.m_dropSpeed = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "DropSpeed", 0.5f) : 0.5f;
   else
       m_d.m_dropSpeed = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "DropSpeed", 0.2f) : 0.2f;

   SetDefaultPhysics(fromMouseClick);

   m_d.m_collidable = fromMouseClick ? LoadValueBoolWithDefault(strKeyName, "Collidable", true) : true;
   m_d.m_disableLightingTop = dequantizeUnsigned<8>(fromMouseClick ? LoadValueIntWithDefault(strKeyName, "DisableLighting", 0) : 0); // stored as uchar for backward compatibility
   m_d.m_disableLightingBelow = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "DisableLightingBelow", 0.f) : 0.f;
   m_d.m_reflectionEnabled = fromMouseClick ? LoadValueBoolWithDefault(strKeyName, "ReflectionEnabled", true) : true;
   m_d.m_raiseDelay = fromMouseClick ? LoadValueIntWithDefault(strKeyName, "RaiseDelay", 100) : 100;

}

void HitTarget::WriteRegDefaults()
{
   static const char strKeyName[] = "DefaultProps\\HitTarget";

   SaveValueBool(strKeyName, "LegacyMode", m_d.m_legacy);
   SaveValueBool(strKeyName, "TimerEnabled", m_d.m_tdr.m_TimerEnabled);
   SaveValueInt(strKeyName, "TimerInterval", m_d.m_tdr.m_TimerInterval);
   SaveValueBool(strKeyName, "Visible", m_d.m_visible);
   SaveValueBool(strKeyName, "IsDropped", m_d.m_isDropped);

   SaveValueFloat(strKeyName, "Position_Z", m_d.m_vPosition.z);
   SaveValueFloat(strKeyName, "DropSpeed", m_d.m_dropSpeed);

   SaveValueFloat(strKeyName, "ScaleX", m_d.m_vSize.x);
   SaveValueFloat(strKeyName, "ScaleY", m_d.m_vSize.y);
   SaveValueFloat(strKeyName, "ScaleZ", m_d.m_vSize.z);

   SaveValueFloat(strKeyName, "Orientation", m_d.m_rotZ);

   SaveValueString(strKeyName, "Image", m_d.m_szImage);
   SaveValueBool(strKeyName, "HitEvent", m_d.m_useHitEvent);
   SaveValueFloat(strKeyName, "HitThreshold", m_d.m_threshold);
   SaveValueFloat(strKeyName, "Elasticity", m_d.m_elasticity);
   SaveValueFloat(strKeyName, "ElasticityFalloff", m_d.m_elasticityFalloff);
   SaveValueFloat(strKeyName, "Friction", m_d.m_friction);
   SaveValueFloat(strKeyName, "Scatter", m_d.m_scatter);

   SaveValueInt(strKeyName, "TargetType", m_d.m_targetType);

   SaveValueBool(strKeyName, "Collidable", m_d.m_collidable);
   const int tmp = quantizeUnsigned<8>(clamp(m_d.m_disableLightingTop, 0.f, 1.f));
   SaveValueInt(strKeyName, "DisableLighting", (tmp == 1) ? 0 : tmp); // backwards compatible saving
   SaveValueFloat(strKeyName, "DisableLightingBelow", m_d.m_disableLightingBelow);
   SaveValueBool(strKeyName, "ReflectionEnabled", m_d.m_reflectionEnabled);
   SaveValueInt(strKeyName, "RaiseDelay", m_d.m_raiseDelay);

}

//
// license:GPLv3+
// Ported at: VisualPinball.Engine/VPT/HitTarget/HitTargetHitGenerator.cs
//

const unsigned int num_dropTargetHitPlaneVertices = 16;
const Vertex3Ds dropTargetHitPlaneVertices[num_dropTargetHitPlaneVertices] =
{
   Vertex3Ds(-0.300000f, 0.001737f, -0.160074f),
   Vertex3Ds(-0.300000f, 0.001738f, 0.439926f),
   Vertex3Ds(0.300000f, 0.001738f, 0.439926f),
   Vertex3Ds(0.300000f, 0.001737f, -0.160074f),
   Vertex3Ds(-0.500000f, 0.001738f, 0.439926f),
   Vertex3Ds(-0.500000f, 0.001738f, 1.789926f),
   Vertex3Ds(0.500000f, 0.001738f, 1.789926f),
   Vertex3Ds(0.500000f, 0.001738f, 0.439926f),
   Vertex3Ds(-0.535355f, 0.001738f, 0.454570f),
   Vertex3Ds(-0.535355f, 0.001738f, 1.775281f),
   Vertex3Ds(-0.550000f, 0.001738f, 0.489926f),
   Vertex3Ds(-0.550000f, 0.001738f, 1.739926f),
   Vertex3Ds(0.535355f, 0.001738f, 0.454570f),
   Vertex3Ds(0.535355f, 0.001738f, 1.775281f),
   Vertex3Ds(0.550000f, 0.001738f, 0.489926f),
   Vertex3Ds(0.550000f, 0.001738f, 1.739926f)
};

const unsigned int num_dropTargetHitPlaneIndices = 42;
const WORD dropTargetHitPlaneIndices[num_dropTargetHitPlaneIndices] =
{
   0, 1, 2, 2, 3, 0, 1, 4, 5, 6, 7, 2, 5, 6, 1,
   2, 1, 6, 4, 8, 9, 9, 5, 4, 8, 10, 11, 11, 9, 8,
   6, 12, 7, 12, 6, 13, 12, 13, 14, 13, 15, 14
};

// Ported at: VisualPinball.Engine/VPT/HitTarget/HitTargetHitGenerator.cs

void HitTarget::GetHitShapes(vector<HitObject*> &pvho)
{
    TransformVertices();

    if (m_d.m_targetType == DropTargetBeveled || m_d.m_targetType == DropTargetFlatSimple || m_d.m_targetType == DropTargetSimple)
    {
       std::set< std::pair<unsigned, unsigned> > addedEdges;

       Matrix3D fullMatrix, tempMatrix;
       fullMatrix.SetIdentity();
       tempMatrix.SetIdentity();
       tempMatrix.RotateZMatrix(ANGTORAD(m_d.m_rotZ));
       tempMatrix.Multiply(fullMatrix, fullMatrix);

       // add the normal drop target as collidable but without hit event
       for (unsigned i = 0; i < m_numIndices; i += 3)
       {
          const unsigned int i0 = m_indices[i];
          const unsigned int i1 = m_indices[i + 1];
          const unsigned int i2 = m_indices[i + 2];

          const Vertex3Ds rgv3D[3] = {
          // NB: HitTriangle wants CCW vertices, but for rendering we have them in CW order
             Vertex3Ds(m_hitUIVertices[i0].x, m_hitUIVertices[i0].y, m_hitUIVertices[i0].z),
             Vertex3Ds(m_hitUIVertices[i2].x, m_hitUIVertices[i2].y, m_hitUIVertices[i2].z),
             Vertex3Ds(m_hitUIVertices[i1].x, m_hitUIVertices[i1].y, m_hitUIVertices[i1].z) };
          SetupHitObject(pvho, new HitTriangle(rgv3D), m_d.m_legacy);

          AddHitEdge(pvho, addedEdges, i0, i1, rgv3D[0], rgv3D[2], m_d.m_legacy);
          AddHitEdge(pvho, addedEdges, i1, i2, rgv3D[2], rgv3D[1], m_d.m_legacy);
          AddHitEdge(pvho, addedEdges, i2, i0, rgv3D[1], rgv3D[0], m_d.m_legacy);
       }

       // add collision vertices
       for (unsigned i = 0; i < m_numVertices; ++i)
          SetupHitObject(pvho, new HitPoint(m_hitUIVertices[i]), m_d.m_legacy);

       if (!m_d.m_legacy)
       {
          Vertex3Ds rgv3D[num_dropTargetHitPlaneVertices];
          float hitShapeOffset = 0.18f;
          if (m_d.m_targetType == DropTargetBeveled)
             hitShapeOffset = 0.25f;
          if (m_d.m_targetType == DropTargetFlatSimple)
             hitShapeOffset = 0.13f;

          // now create a special hit shape with hit event enabled to prevent a hit event when hit from behind
          for (unsigned i = 0; i < num_dropTargetHitPlaneVertices; i++)
          {
             Vertex3Ds vert(dropTargetHitPlaneVertices[i].x, dropTargetHitPlaneVertices[i].y+hitShapeOffset, dropTargetHitPlaneVertices[i].z);
             vert.x *= m_d.m_vSize.x;
             vert.y *= m_d.m_vSize.y;
             vert.z *= m_d.m_vSize.z;
             vert = fullMatrix.MultiplyVector(vert);

             rgv3D[i].x = vert.x + m_d.m_vPosition.x;
             rgv3D[i].y = vert.y + m_d.m_vPosition.y;
             rgv3D[i].z = vert.z*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set] + m_d.m_vPosition.z + m_ptable->m_tableheight;
          }

          for (unsigned int i = 0; i < num_dropTargetHitPlaneIndices; i += 3)
          {
             const unsigned int i0 = dropTargetHitPlaneIndices[i];
             const unsigned int i1 = dropTargetHitPlaneIndices[i + 1];
             const unsigned int i2 = dropTargetHitPlaneIndices[i + 2];

             const Vertex3Ds rgv3D2[3] = {
             // NB: HitTriangle wants CCW vertices, but for rendering we have them in CW order
                 rgv3D[i0],
                 rgv3D[i2],
                 rgv3D[i1] };
             SetupHitObject(pvho, new HitTriangle(rgv3D2), true);

             AddHitEdge(pvho, addedEdges, i0, i1, rgv3D2[0], rgv3D2[2]);
             AddHitEdge(pvho, addedEdges, i1, i2, rgv3D2[2], rgv3D2[1]);
             AddHitEdge(pvho, addedEdges, i2, i0, rgv3D2[1], rgv3D2[0]);
          }

          // add collision vertices
          for (unsigned i = 0; i < num_dropTargetHitPlaneVertices; ++i)
             SetupHitObject(pvho, new HitPoint(rgv3D[i]), true);
       }
    }
    else
    {
       std::set< std::pair<unsigned, unsigned> > addedEdges;
       // add collision triangles and edges
       for (unsigned i = 0; i < m_numIndices; i += 3)
       {
          const unsigned int i0 = m_indices[i];
          const unsigned int i1 = m_indices[i + 1];
          const unsigned int i2 = m_indices[i + 2];

          const Vertex3Ds rgv3D[3] = {
          // NB: HitTriangle wants CCW vertices, but for rendering we have them in CW order
              Vertex3Ds(m_hitUIVertices[i0].x, m_hitUIVertices[i0].y, m_hitUIVertices[i0].z),
              Vertex3Ds(m_hitUIVertices[i2].x, m_hitUIVertices[i2].y, m_hitUIVertices[i2].z),
              Vertex3Ds(m_hitUIVertices[i1].x, m_hitUIVertices[i1].y, m_hitUIVertices[i1].z) };
          SetupHitObject(pvho, new HitTriangle(rgv3D), true);

          AddHitEdge(pvho, addedEdges, i0, i1, rgv3D[0], rgv3D[2]);
          AddHitEdge(pvho, addedEdges, i1, i2, rgv3D[2], rgv3D[1]);
          AddHitEdge(pvho, addedEdges, i2, i0, rgv3D[1], rgv3D[0]);
       }

       // add collision vertices
       for (unsigned i = 0; i < m_numVertices; ++i)
          SetupHitObject(pvho, new HitPoint(m_hitUIVertices[i]), true);
    }
}

void HitTarget::GetHitShapesDebug(vector<HitObject*> &pvho)
{
}

void HitTarget::AddHitEdge(vector<HitObject*> &pvho, std::set< std::pair<unsigned, unsigned> >& addedEdges, const unsigned i, const unsigned j, const Vertex3Ds &vi, const Vertex3Ds &vj, const bool setHitObject)
{
   // create pair uniquely identifying the edge (i,j)
   std::pair<unsigned, unsigned> p(std::min(i, j), std::max(i, j));

   if (addedEdges.count(p) == 0)   // edge not yet added?
   {
      addedEdges.insert(p);
      SetupHitObject(pvho, new HitLine3D(vi, vj), setHitObject);
   }
}

// Ported at: VisualPinball.Engine/Physics/HitObject.cs

void HitTarget::SetupHitObject(vector<HitObject*> &pvho, HitObject * obj, const bool setHitObject)
{
   const Material * const mat = m_ptable->GetMaterial(m_d.m_szPhysicsMaterial);
   if (!m_d.m_overwritePhysics)
   {
      obj->m_elasticity = mat->m_fElasticity;
      obj->m_elasticityFalloff = mat->m_fElasticityFalloff;
      obj->SetFriction(mat->m_fFriction);
      obj->m_scatter = ANGTORAD(mat->m_fScatterAngle);
   }
   else
   {
      obj->m_elasticity = m_d.m_elasticity;
      obj->m_elasticityFalloff = m_d.m_elasticityFalloff;
      obj->SetFriction(m_d.m_friction);
      obj->m_scatter = ANGTORAD(m_d.m_scatter);
   }
   obj->m_threshold = m_d.m_threshold;
   obj->m_enabled = m_d.m_collidable;
   obj->m_ObjType = eHitTarget;
   obj->m_obj = (IFireEvents*)this;
   obj->m_fe = setHitObject && m_d.m_useHitEvent;

   pvho.push_back(obj);
   m_vhoCollidable.push_back(obj);	//remember hit components of primitive
}

//
// end of license:GPLv3+, back to 'old MAME'-like
//

void HitTarget::EndPlay()
{
   m_vhoCollidable.clear();

   if (m_vertexBuffer)
   {
      m_vertexBuffer->release();
      m_vertexBuffer = 0;
   }
   if (m_indexBuffer)
   {
      m_indexBuffer->release();
      m_indexBuffer = 0;
   }

   IEditable::EndPlay();
}

//////////////////////////////
// Calculation
//////////////////////////////

void HitTarget::GenerateMesh(std::vector<Vertex3D_NoTex2> &buf)
{
   SetMeshType(m_d.m_targetType);

   Matrix3D fullMatrix, tempMatrix;
   fullMatrix.SetIdentity();
   tempMatrix.SetIdentity();
   tempMatrix.RotateZMatrix(ANGTORAD(m_d.m_rotZ));
   tempMatrix.Multiply(fullMatrix, fullMatrix);

   for (unsigned int i = 0; i < m_numVertices; i++)
   {
      Vertex3Ds vert(m_vertices[i].x, m_vertices[i].y, m_vertices[i].z);
      vert.x *= m_d.m_vSize.x;
      vert.y *= m_d.m_vSize.y;
      vert.z *= m_d.m_vSize.z;
      vert = fullMatrix.MultiplyVector(vert);

      buf[i].x = vert.x + m_d.m_vPosition.x;
      buf[i].y = vert.y + m_d.m_vPosition.y;
      buf[i].z = vert.z*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set] + m_d.m_vPosition.z + m_ptable->m_tableheight;

      vert = Vertex3Ds(m_vertices[i].nx, m_vertices[i].ny, m_vertices[i].nz);
      vert = fullMatrix.MultiplyVectorNoTranslate(vert);
      buf[i].nx = vert.x;
      buf[i].ny = vert.y;
      buf[i].nz = vert.z;
      buf[i].tu = m_vertices[i].tu;
      buf[i].tv = m_vertices[i].tv;
   }
}

// recalculate vertices for editor display or hit shapes
void HitTarget::TransformVertices()
{
   SetMeshType(m_d.m_targetType);

   Matrix3D fullMatrix, tempMatrix;
   m_hitUIVertices.resize(m_numVertices);
   fullMatrix.SetIdentity();
   tempMatrix.SetIdentity();
   tempMatrix.RotateZMatrix(ANGTORAD(m_d.m_rotZ));
   tempMatrix.Multiply(fullMatrix, fullMatrix);

   for (unsigned i = 0; i < m_numVertices; i++)
   {
      Vertex3Ds vert(m_vertices[i].x, m_vertices[i].y, m_vertices[i].z);
      vert.x *= m_d.m_vSize.x;
      vert.y *= m_d.m_vSize.y;
      vert.z *= m_d.m_vSize.z;
      vert = fullMatrix.MultiplyVector(vert);

      m_hitUIVertices[i].x = vert.x + m_d.m_vPosition.x;
      m_hitUIVertices[i].y = vert.y + m_d.m_vPosition.y;
      m_hitUIVertices[i].z = vert.z*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set] + (m_d.m_vPosition.z + m_ptable->m_tableheight);
   }
}

void HitTarget::ExportMesh(FILE *f)
{
   char name[sizeof(m_wzName)/sizeof(m_wzName[0])];
   WideCharToMultiByte(CP_ACP, 0, m_wzName, -1, name, sizeof(name), NULL, NULL);

   SetMeshType(m_d.m_targetType);

   m_transformedVertices.resize(m_numVertices);

   WaveFrontObj_WriteObjectName(f, name);

   GenerateMesh(m_transformedVertices);

   WaveFrontObj_WriteVertexInfo(f, m_transformedVertices.data(), m_numVertices);
   const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);
   WaveFrontObj_WriteMaterial(m_d.m_szMaterial, string(), mat);
   WaveFrontObj_UseTexture(f, m_d.m_szMaterial);
   WaveFrontObj_WriteFaceInfoList(f, m_indices, m_numIndices);
   WaveFrontObj_UpdateFaceOffset(m_numVertices);
}


//////////////////////////////
// Rendering
//////////////////////////////

// 2D

void HitTarget::UIRenderPass1(Sur * const psur)
{
}

void HitTarget::UIRenderPass2(Sur * const psur)
{
   psur->SetLineColor(RGB(0, 0, 0), false, 1);
   psur->SetObject(this);

    for (unsigned i = 0; i < m_numIndices; i += 3)
    {
       const Vertex3Ds * const A = &m_hitUIVertices[m_indices[i]];
       const Vertex3Ds * const B = &m_hitUIVertices[m_indices[i + 1]];
       const Vertex3Ds * const C = &m_hitUIVertices[m_indices[i + 2]];
       psur->Line(A->x, A->y, B->x, B->y);
       psur->Line(B->x, B->y, C->x, C->y);
       psur->Line(C->x, C->y, A->x, A->y);
    }

    if (m_selectstate == eNotSelected)
       return;

    const float radangle = ANGTORAD(m_d.m_rotZ-180.0f);
    const float halflength = 50.0f;
    const float len1 = halflength * 0.5f;
    const float len2 = len1 * 0.5f;
    Vertex2D tmp;
    {
       // Draw Arrow
       psur->SetLineColor(RGB(255, 0, 0), false, 1);

       {
       const float sn = sinf(radangle);
       const float cs = cosf(radangle);

       tmp.x = m_d.m_vPosition.x + sn*len1;
       tmp.y = m_d.m_vPosition.y - cs*len1;
       }

       psur->Line(tmp.x, tmp.y, m_d.m_vPosition.x, m_d.m_vPosition.y);
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

void HitTarget::UpdateStatusBarInfo()
{
   TransformVertices();
}

//
// license:GPLv3+
// Ported at: VisualPinball.Unity/VisualPinball.Unity/VPT/HitTarget/HitTargetAnimationSystem.cs
//

void HitTarget::UpdateAnimation()
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
            else if ((m_d.m_time_msec - m_timeStamp) < (unsigned int)m_d.m_raiseDelay)
            {
                step = 0.0f;
            }
            m_moveAnimationOffset += step*diff_time_msec;
            if (m_moveDown)
            {
                if (m_moveAnimationOffset <= -limit)
                {
                    m_moveAnimationOffset = -limit;
                    m_moveDown = false;
                    m_d.m_isDropped = true;
                    m_moveAnimation = false;
                    m_timeStamp = 0;
                    if (m_d.m_useHitEvent)
                        FireGroupEvent(DISPID_TargetEvents_Dropped);
                }
            }
            else 
            {                 
                if (m_moveAnimationOffset >= 0.0f)
                {
                    m_moveAnimationOffset = 0.0f;
                    m_moveAnimation = false;
                    m_d.m_isDropped = false;
                    if (m_d.m_useHitEvent)
                        FireGroupEvent(DISPID_TargetEvents_Raised);
                }
            }
            UpdateTarget();
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
                if (m_moveAnimationOffset >= limit)
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
            UpdateTarget();
        }
    }
}

//
// end of license:GPLv3+, back to 'old MAME'-like
//

void HitTarget::RenderObject()
{
   UpdateAnimation();

   RenderDevice * const pd3dDevice = g_pplayer->m_pin3d.m_pd3dPrimaryDevice;

   const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);
   pd3dDevice->basicShader->SetMaterial(mat);

   pd3dDevice->SetRenderState(RenderDevice::DEPTHBIAS, 0);
   pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, RenderDevice::RS_TRUE);
#ifdef TWOSIDED_TRANSPARENCY
   pd3dDevice->SetRenderState(RenderDevice::CULLMODE, mat->m_bOpacityActive ? RenderDevice::CULL_CW : RenderDevice::CULL_CCW);
#else
   pd3dDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_CCW);
#endif

   if (m_d.m_disableLightingTop != 0.f || m_d.m_disableLightingBelow != 0.f)
   {
      const vec4 tmp(m_d.m_disableLightingTop,m_d.m_disableLightingBelow, 0.f,0.f);
      pd3dDevice->basicShader->SetDisableLighting(tmp);
   }

   Texture * const pin = m_ptable->GetImage(m_d.m_szImage);
   if (pin)
   {
      pd3dDevice->basicShader->SetTechnique(mat->m_bIsMetal ? "basic_with_texture_isMetal" : "basic_with_texture_isNotMetal");
      pd3dDevice->basicShader->SetTexture("Texture0", pin, false);
      pd3dDevice->basicShader->SetAlphaTestValue(pin->m_alphaTestValue * (float)(1.0 / 255.0));

      //g_pplayer->m_pin3d.SetPrimaryTextureFilter(0, TEXTURE_MODE_TRILINEAR);
      // accomodate models with UV coords outside of [0,1]
      pd3dDevice->SetTextureAddressMode(0, RenderDevice::TEX_WRAP);
   }
   else
      pd3dDevice->basicShader->SetTechnique(mat->m_bIsMetal ? "basic_without_texture_isMetal" : "basic_without_texture_isNotMetal");

   // draw the mesh
   pd3dDevice->basicShader->Begin(0);
   pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, m_vertexBuffer, 0, m_numVertices, m_indexBuffer, 0, m_numIndices);
   pd3dDevice->basicShader->End();

#ifdef TWOSIDED_TRANSPARENCY
   if (mat->m_bOpacityActive)
   {
       pd3dDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_CCW);
       pd3dDevice->basicShader->Begin(0);
       pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, vertexBuffer, 0, m_numVertices, indexBuffer, 0, m_numIndices);
       pd3dDevice->basicShader->End();
   }
#endif

   pd3dDevice->SetTextureAddressMode(0, RenderDevice::TEX_CLAMP);
   //g_pplayer->m_pin3d.DisableAlphaBlend(); //!! not necessary anymore
   if (m_d.m_disableLightingTop != 0.f || m_d.m_disableLightingBelow != 0.f)
   {
      const vec4 tmp(0.f,0.f, 0.f,0.f);
      pd3dDevice->basicShader->SetDisableLighting(tmp);
   }
}

void HitTarget::UpdateTarget()
{
   Vertex3D_NoTex2 *buf;
   m_vertexBuffer->lock(0, 0, (void**)&buf, VertexBuffer::DISCARDCONTENTS);
   if (m_d.m_targetType == DropTargetBeveled || m_d.m_targetType == DropTargetSimple || m_d.m_targetType == DropTargetFlatSimple)
   {
       //TODO Update object Matrix instead
       for (unsigned int i = 0; i < m_numVertices; i++)
       {
           buf[i].x = m_transformedVertices[i].x;
           buf[i].y = m_transformedVertices[i].y;
           buf[i].z = m_transformedVertices[i].z + m_moveAnimationOffset;
           buf[i].nx = m_transformedVertices[i].nx;
           buf[i].ny = m_transformedVertices[i].ny;
           buf[i].nz = m_transformedVertices[i].nz;
           buf[i].tu = m_transformedVertices[i].tu;
           buf[i].tv = m_transformedVertices[i].tv;
       }
   }
   else
   {
       Matrix3D fullMatrix, tempMatrix;
       fullMatrix.RotateXMatrix(ANGTORAD(m_moveAnimationOffset));
       tempMatrix.RotateZMatrix(ANGTORAD(m_d.m_rotZ));
       tempMatrix.Multiply(fullMatrix, fullMatrix);

       Matrix3D vertMatrix;
       vertMatrix.SetScaling(m_d.m_vSize.x, m_d.m_vSize.y, m_d.m_vSize.z);
       fullMatrix.Multiply(vertMatrix, vertMatrix);
       tempMatrix.SetScaling(1.f, 1.f, m_ptable->m_BG_scalez[m_ptable->m_BG_current_set]);
       tempMatrix.Multiply(vertMatrix, vertMatrix);
       tempMatrix.SetTranslation(m_d.m_vPosition.x, m_d.m_vPosition.y, m_d.m_vPosition.z + m_ptable->m_tableheight);
       tempMatrix.Multiply(vertMatrix, vertMatrix);

       //TODO Update object Matrix instead
       for (unsigned int i = 0; i < m_numVertices; i++)
       {
           Vertex3Ds vert(m_vertices[i].x, m_vertices[i].y, m_vertices[i].z);
           vert = vertMatrix.MultiplyVector(vert);           
           buf[i].x = vert.x;
           buf[i].y = vert.y;
           buf[i].z = vert.z;

           vert = Vertex3Ds(m_vertices[i].nx, m_vertices[i].ny, m_vertices[i].nz);
           vert = fullMatrix.MultiplyVectorNoTranslate(vert);
           buf[i].nx = vert.x;
           buf[i].ny = vert.y;
           buf[i].nz = vert.z;
           buf[i].tu = m_vertices[i].tu;
           buf[i].tv = m_vertices[i].tv;
       }
   }
   m_vertexBuffer->unlock();
}

// Always called each frame to render over everything else (along with alpha ramps)
void HitTarget::RenderDynamic()
{
   TRACE_FUNCTION();

   if (!m_d.m_visible)
      return;
   if (m_ptable->m_reflectionEnabled && !m_d.m_reflectionEnabled)
      return;

   RenderObject();
}

void HitTarget::RenderSetup()
{
   RenderDevice * const pd3dDevice = g_pplayer->m_pin3d.m_pd3dPrimaryDevice;

   if (m_vertexBuffer)
      m_vertexBuffer->release();

   SetMeshType(m_d.m_targetType);
   pd3dDevice->CreateVertexBuffer((unsigned int)m_numVertices, USAGE_DYNAMIC, MY_D3DFVF_NOTEX2_VERTEX, &m_vertexBuffer);

   if (m_indexBuffer)
      m_indexBuffer->release();
   m_indexBuffer = pd3dDevice->CreateAndFillIndexBuffer(m_numIndices, m_indices);

   m_transformedVertices.resize(m_numVertices);

   GenerateMesh(m_transformedVertices);
   m_moveAnimationOffset = 0.0f;
   if (m_d.m_targetType == DropTargetBeveled || m_d.m_targetType == DropTargetSimple || m_d.m_targetType == DropTargetFlatSimple)
   {
       if (m_d.m_isDropped)
       {
           m_moveDown = false;
           m_moveAnimationOffset = -DROP_TARGET_LIMIT*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];
           UpdateTarget();
           return;
       }
   }
   Vertex3D_NoTex2 *buf;
   m_vertexBuffer->lock(0, 0, (void**)&buf, VertexBuffer::DISCARDCONTENTS);
   memcpy(buf, m_transformedVertices.data(), m_numVertices*sizeof(Vertex3D_NoTex2));
   m_vertexBuffer->unlock();
   m_d.m_time_msec = g_pplayer->m_time_msec;
}

void HitTarget::RenderStatic()
{
}

//////////////////////////////
// Positioning
//////////////////////////////

void HitTarget::SetObjectPos()
{
    m_vpinball->SetObjectPosCur(m_d.m_vPosition.x, m_d.m_vPosition.y);
}

void HitTarget::MoveOffset(const float dx, const float dy)
{
   m_d.m_vPosition.x += dx;
   m_d.m_vPosition.y += dy;

   UpdateStatusBarInfo();
}

Vertex2D HitTarget::GetCenter() const
{
   return Vertex2D(m_d.m_vPosition.x, m_d.m_vPosition.y);
}

void HitTarget::PutCenter(const Vertex2D& pv)
{
   m_d.m_vPosition.x = pv.x;
   m_d.m_vPosition.y = pv.y;

   UpdateStatusBarInfo();
}

//////////////////////////////
// Save and Load
//////////////////////////////

HRESULT HitTarget::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, const bool backupForPlay)
{
   BiffWriter bw(pstm, hcrypthash);

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
   bw.WriteWideString(FID(NAME), m_wzName);
   bw.WriteString(FID(MATR), m_d.m_szMaterial);
   bw.WriteBool(FID(TVIS), m_d.m_visible);
   bw.WriteBool(FID(LEMO), m_d.m_legacy);
   bw.WriteBool(FID(HTEV), m_d.m_useHitEvent);
   bw.WriteFloat(FID(THRS), m_d.m_threshold);
   bw.WriteFloat(FID(ELAS), m_d.m_elasticity);
   bw.WriteFloat(FID(ELFO), m_d.m_elasticityFalloff);
   bw.WriteFloat(FID(RFCT), m_d.m_friction);
   bw.WriteFloat(FID(RSCT), m_d.m_scatter);
   bw.WriteBool(FID(CLDR), m_d.m_collidable);
   const int tmp = quantizeUnsigned<8>(clamp(m_d.m_disableLightingTop, 0.f, 1.f));
   bw.WriteInt(FID(DILI), (tmp == 1) ? 0 : tmp); // backwards compatible saving
   bw.WriteFloat(FID(DILB), m_d.m_disableLightingBelow);
   bw.WriteBool(FID(REEN), m_d.m_reflectionEnabled);
   bw.WriteFloat(FID(PIDB), m_d.m_depthBias);
   bw.WriteBool(FID(ISDR), m_d.m_isDropped);
   bw.WriteFloat(FID(DRSP), m_d.m_dropSpeed);
   bw.WriteBool(FID(TMON), m_d.m_tdr.m_TimerEnabled);
   bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
   bw.WriteInt(FID(RADE), m_d.m_raiseDelay);
   bw.WriteString(FID(MAPH), m_d.m_szPhysicsMaterial);
   bw.WriteBool(FID(OVPH), m_d.m_overwritePhysics);

   ISelect::SaveData(pstm, hcrypthash);

   bw.WriteTag(FID(ENDB));

   return S_OK;
}

HRESULT HitTarget::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   SetDefaults(false);

   BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);

   m_ptable = ptable;

   br.Load();

   UpdateStatusBarInfo();
   return S_OK;
}

bool HitTarget::LoadToken(const int id, BiffReader * const pbr)
{
   switch(id)
   {
   case FID(PIID): pbr->GetInt((int *)pbr->m_pdata); break;
   case FID(VPOS): pbr->GetVector3Padded(&m_d.m_vPosition); break;
   case FID(VSIZ): pbr->GetVector3Padded(&m_d.m_vSize); break;
   case FID(ROTZ): pbr->GetFloat(&m_d.m_rotZ); break;
   case FID(IMAG): pbr->GetString(m_d.m_szImage); break;
   case FID(TRTY): pbr->GetInt(&m_d.m_targetType); break;
   case FID(NAME): pbr->GetWideString(m_wzName); break;
   case FID(MATR): pbr->GetString(m_d.m_szMaterial); break;
   case FID(TVIS): pbr->GetBool(&m_d.m_visible); break;
   case FID(LEMO): pbr->GetBool(&m_d.m_legacy); break;
   case FID(ISDR): pbr->GetBool(&m_d.m_isDropped); break;
   case FID(DRSP): pbr->GetFloat(&m_d.m_dropSpeed); break;
   case FID(REEN): pbr->GetBool(&m_d.m_reflectionEnabled); break;
   case FID(HTEV): pbr->GetBool(&m_d.m_useHitEvent); break;
   case FID(THRS): pbr->GetFloat(&m_d.m_threshold); break;
   case FID(ELAS): pbr->GetFloat(&m_d.m_elasticity); break;
   case FID(ELFO): pbr->GetFloat(&m_d.m_elasticityFalloff); break;
   case FID(RFCT): pbr->GetFloat(&m_d.m_friction); break;
   case FID(RSCT): pbr->GetFloat(&m_d.m_scatter); break;
   case FID(CLDR): pbr->GetBool(&m_d.m_collidable); break;
   case FID(DILI):
   {
      int tmp;
      pbr->GetInt(&tmp);
      m_d.m_disableLightingTop = (tmp == 1) ? 1.f : dequantizeUnsigned<8>(tmp); // backwards compatible hacky loading!
      break;
   }
   case FID(DILB): pbr->GetFloat(&m_d.m_disableLightingBelow); break;
   case FID(PIDB): pbr->GetFloat(&m_d.m_depthBias); break;
   case FID(TMON): pbr->GetBool(&m_d.m_tdr.m_TimerEnabled); break;
   case FID(TMIN): pbr->GetInt(&m_d.m_tdr.m_TimerInterval); break;
   case FID(RADE): pbr->GetInt(&m_d.m_raiseDelay); break;
   case FID(MAPH): pbr->GetString(m_d.m_szPhysicsMaterial); break;
   case FID(OVPH): pbr->GetBool(&m_d.m_overwritePhysics); break;
   default: ISelect::LoadToken(id, pbr); break;
   }
   return true;
}

HRESULT HitTarget::InitPostLoad()
{
   UpdateStatusBarInfo();

   return S_OK;
}

//////////////////////////////
// Standard methods
//////////////////////////////

STDMETHODIMP HitTarget::get_Image(BSTR *pVal)
{
   WCHAR wz[MAXTOKEN];
   MultiByteToWideChar(CP_ACP, 0, m_d.m_szImage.c_str(), -1, wz, MAXTOKEN);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP HitTarget::put_Image(BSTR newVal)
{
   char szImage[MAXTOKEN];
   WideCharToMultiByte(CP_ACP, 0, newVal, -1, szImage, MAXTOKEN, NULL, NULL);
   const Texture * const tex = m_ptable->GetImage(szImage);
   if (tex && tex->IsHDR())
   {
       ShowError("Cannot use a HDR image (.exr/.hdr) here");
       return E_FAIL;
   }
   m_d.m_szImage = szImage;

   return S_OK;
}


bool HitTarget::IsTransparent() const
{
   return m_ptable->GetMaterial(m_d.m_szMaterial)->m_bOpacityActive;
}

float HitTarget::GetDepth(const Vertex3Ds& viewDir) const
{
   return m_d.m_depthBias + m_d.m_vPosition.Dot(viewDir);
}

STDMETHODIMP HitTarget::get_Material(BSTR *pVal)
{
   WCHAR wz[MAXNAMEBUFFER];
   MultiByteToWideChar(CP_ACP, 0, m_d.m_szMaterial.c_str(), -1, wz, MAXNAMEBUFFER);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP HitTarget::put_Material(BSTR newVal)
{
   char buf[MAXNAMEBUFFER];
   WideCharToMultiByte(CP_ACP, 0, newVal, -1, buf, MAXNAMEBUFFER, NULL, NULL);
   m_d.m_szMaterial = buf;

   return S_OK;
}


STDMETHODIMP HitTarget::get_Visible(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_visible);

   return S_OK;
}

STDMETHODIMP HitTarget::put_Visible(VARIANT_BOOL newVal)
{
   m_d.m_visible = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP HitTarget::get_X(float *pVal)
{
   *pVal = m_d.m_vPosition.x;
   m_vpinball->SetStatusBarUnitInfo("", true);

   return S_OK;
}

STDMETHODIMP HitTarget::put_X(float newVal)
{
   m_d.m_vPosition.x = newVal;

   return S_OK;
}

STDMETHODIMP HitTarget::get_Y(float *pVal)
{
   *pVal = m_d.m_vPosition.y;

   return S_OK;
}

STDMETHODIMP HitTarget::put_Y(float newVal)
{
   m_d.m_vPosition.y = newVal;

   return S_OK;
}

STDMETHODIMP HitTarget::get_Z(float *pVal)
{
   *pVal = m_d.m_vPosition.z;

   return S_OK;
}

STDMETHODIMP HitTarget::put_Z(float newVal)
{
   m_d.m_vPosition.z = newVal;

   return S_OK;
}

STDMETHODIMP HitTarget::get_ScaleX(float *pVal)
{
   *pVal = m_d.m_vSize.x;

   return S_OK;
}

STDMETHODIMP HitTarget::put_ScaleX(float newVal)
{
   m_d.m_vSize.x = newVal;

   return S_OK;
}

STDMETHODIMP HitTarget::get_ScaleY(float *pVal)
{
   *pVal = m_d.m_vSize.y;

   return S_OK;
}

STDMETHODIMP HitTarget::put_ScaleY(float newVal)
{
   m_d.m_vSize.y = newVal;

   return S_OK;
}

STDMETHODIMP HitTarget::get_ScaleZ(float *pVal)
{
   *pVal = m_d.m_vSize.z;

   return S_OK;
}

STDMETHODIMP HitTarget::put_ScaleZ(float newVal)
{
   m_d.m_vSize.z = newVal;

   return S_OK;
}


STDMETHODIMP HitTarget::get_Orientation(float *pVal)
{
   *pVal = m_d.m_rotZ;
   return S_OK;
}

STDMETHODIMP HitTarget::put_Orientation(float newVal)
{
   m_d.m_rotZ = newVal;

   return S_OK;
}

STDMETHODIMP HitTarget::get_HasHitEvent(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_useHitEvent);

   return S_OK;
}

STDMETHODIMP HitTarget::put_HasHitEvent(VARIANT_BOOL newVal)
{
   m_d.m_useHitEvent = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP HitTarget::get_Threshold(float *pVal)
{
   *pVal = m_d.m_threshold;

   return S_OK;
}

STDMETHODIMP HitTarget::put_Threshold(float newVal)
{
   m_d.m_threshold = newVal;

   return S_OK;
}

STDMETHODIMP HitTarget::get_Elasticity(float *pVal)
{
   *pVal = m_d.m_elasticity;

   return S_OK;
}

STDMETHODIMP HitTarget::put_Elasticity(float newVal)
{
   m_d.m_elasticity = newVal;

   return S_OK;
}

STDMETHODIMP HitTarget::get_ElasticityFalloff(float *pVal)
{
   *pVal = m_d.m_elasticityFalloff;

   return S_OK;
}

STDMETHODIMP HitTarget::put_ElasticityFalloff(float newVal)
{
   m_d.m_elasticityFalloff = newVal;

   return S_OK;
}

STDMETHODIMP HitTarget::get_Friction(float *pVal)
{
   *pVal = m_d.m_friction;

   return S_OK;
}

STDMETHODIMP HitTarget::put_Friction(float newVal)
{
   m_d.m_friction = clamp(newVal, 0.f, 1.f);

   return S_OK;
}

STDMETHODIMP HitTarget::get_Scatter(float *pVal)
{
   *pVal = m_d.m_scatter;

   return S_OK;
}

STDMETHODIMP HitTarget::put_Scatter(float newVal)
{
   m_d.m_scatter = newVal;

   return S_OK;
}

STDMETHODIMP HitTarget::get_Collidable(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB((!g_pplayer) ? m_d.m_collidable : m_vhoCollidable[0]->m_enabled);

   return S_OK;
}

STDMETHODIMP HitTarget::put_Collidable(VARIANT_BOOL newVal)
{
   const bool val = VBTOb(newVal);
   if (!g_pplayer)
      m_d.m_collidable = val;
   else
   {
       if (m_vhoCollidable.size() > 0 && m_vhoCollidable[0]->m_enabled != val)
           for (size_t i = 0; i < m_vhoCollidable.size(); i++) //!! costly
               m_vhoCollidable[i]->m_enabled = val; //copy to hit checking on entities composing the object 
   }

   return S_OK;
}

STDMETHODIMP HitTarget::get_DisableLighting(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_disableLightingTop != 0.f);

   return S_OK;
}

STDMETHODIMP HitTarget::put_DisableLighting(VARIANT_BOOL newVal)
{
   m_d.m_disableLightingTop = VBTOb(newVal) ? 1.f : 0;

   return S_OK;
}

STDMETHODIMP HitTarget::get_BlendDisableLighting(float *pVal)
{
   *pVal = m_d.m_disableLightingTop;

   return S_OK;
}

STDMETHODIMP HitTarget::put_BlendDisableLighting(float newVal)
{
   m_d.m_disableLightingTop = newVal;

   return S_OK;
}

STDMETHODIMP HitTarget::get_BlendDisableLightingFromBelow(float *pVal)
{
   *pVal = m_d.m_disableLightingBelow;

   return S_OK;
}

STDMETHODIMP HitTarget::put_BlendDisableLightingFromBelow(float newVal)
{
   m_d.m_disableLightingBelow = newVal;

   return S_OK;
}

STDMETHODIMP HitTarget::get_ReflectionEnabled(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_reflectionEnabled);

   return S_OK;
}

STDMETHODIMP HitTarget::put_ReflectionEnabled(VARIANT_BOOL newVal)
{
   m_d.m_reflectionEnabled = VBTOb(newVal);

   return S_OK;
}

void HitTarget::SetDefaultPhysics(bool fromMouseClick)
{
   static const char strKeyName[] = "DefaultProps\\HitTarget";
   m_d.m_elasticity = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "Elasticity", 0.35f) : 0.35f;
   m_d.m_elasticityFalloff = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "ElasticityFalloff", 0.5f) : 0.5f;
   m_d.m_friction = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "Friction", 0.2f) : 0.2f;
   m_d.m_scatter = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "Scatter", 5) : 5;
}

STDMETHODIMP HitTarget::get_DepthBias(float *pVal)
{
   *pVal = m_d.m_depthBias;

   return S_OK;
}

STDMETHODIMP HitTarget::put_DepthBias(float newVal)
{
   m_d.m_depthBias = newVal;

   return S_OK;
}

STDMETHODIMP HitTarget::get_DropSpeed(float *pVal)
{
   *pVal = m_d.m_dropSpeed;

   return S_OK;
}

STDMETHODIMP HitTarget::put_DropSpeed(float newVal)
{
   m_d.m_dropSpeed = newVal;

   return S_OK;
}

STDMETHODIMP HitTarget::get_IsDropped(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)(m_d.m_isDropped); //don't use FTOVB here because it converts a TRUE to -1 instead to 1

   return S_OK;
}

STDMETHODIMP HitTarget::put_IsDropped(VARIANT_BOOL newVal)
{
   const bool val = (newVal != 0);
   if (g_pplayer && m_d.m_isDropped != val)
   {
      m_moveAnimation = true;
      if (val)
      {
         m_moveAnimationOffset = 0.0f;
         m_moveDown = true;
      }
      else
      {
         m_moveAnimationOffset = -DROP_TARGET_LIMIT*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];
         m_moveDown = false;
         m_timeStamp = g_pplayer->m_time_msec;
      }
   }
   else
      m_d.m_isDropped = val;

   return S_OK;
}

STDMETHODIMP HitTarget::get_LegacyMode(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_legacy);

   return S_OK;
}

STDMETHODIMP HitTarget::put_LegacyMode(VARIANT_BOOL newVal)
{
   m_d.m_legacy = VBTOb(newVal);

   return S_OK;
}


STDMETHODIMP HitTarget::get_DrawStyle(TargetType *pVal)
{
   *pVal = m_d.m_targetType;

   return S_OK;
}

STDMETHODIMP HitTarget::put_DrawStyle(TargetType newVal)
{
   m_d.m_targetType = newVal;

   return S_OK;
}

STDMETHODIMP HitTarget::get_PhysicsMaterial(BSTR *pVal)
{
   WCHAR wz[MAXNAMEBUFFER];
   MultiByteToWideChar(CP_ACP, 0, m_d.m_szPhysicsMaterial.c_str(), -1, wz, MAXNAMEBUFFER);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP HitTarget::put_PhysicsMaterial(BSTR newVal)
{
   char buf[MAXNAMEBUFFER];
   WideCharToMultiByte(CP_ACP, 0, newVal, -1, buf, MAXNAMEBUFFER, NULL, NULL);
   m_d.m_szPhysicsMaterial = buf;

   return S_OK;
}

STDMETHODIMP HitTarget::get_OverwritePhysics(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_overwritePhysics);

   return S_OK;
}

STDMETHODIMP HitTarget::put_OverwritePhysics(VARIANT_BOOL newVal)
{
   m_d.m_overwritePhysics = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP HitTarget::get_HitThreshold(float *pVal)
{
   *pVal = m_currentHitThreshold;

   return S_OK;
}

void HitTarget::GetTimers(vector<HitTimer*> &pvht)
{
    IEditable::BeginPlay();

    HitTimer * const pht = new HitTimer(); //!! claims to be leaking
    pht->m_interval = m_d.m_tdr.m_TimerInterval >= 0 ? max(m_d.m_tdr.m_TimerInterval, MAX_TIMER_MSEC_INTERVAL) : -1;
    pht->m_nextfire = pht->m_interval;
    pht->m_pfe = (IFireEvents *)this;

    m_phittimer = pht;

    if (m_d.m_tdr.m_TimerEnabled)
        pvht.push_back(pht);
}

STDMETHODIMP HitTarget::get_RaiseDelay(long *pVal)
{
    *pVal = m_d.m_raiseDelay;

    return S_OK;
}

STDMETHODIMP HitTarget::put_RaiseDelay(long newVal)
{
    m_d.m_raiseDelay = newVal;

    return S_OK;
}
