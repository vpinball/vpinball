// license:GPLv3+

// implementation of the HitTarget class.

#include "core/stdafx.h" 
#include "utils/objloader.h"
#include "meshes/dropTargetT2Mesh.h"
#include "meshes/dropTargetT3Mesh.h"
#include "meshes/dropTargetT4Mesh.h"
#include "meshes/hitTargetRoundMesh.h"
#include "meshes/hitTargetRectangleMesh.h"
#include "meshes/hitTargetFatRectangleMesh.h"
#include "meshes/hitTargetFatSquareMesh.h"
#include "meshes/hitTargetT1SlimMesh.h"
#include "meshes/hitTargetT2SlimMesh.h"
#include "renderer/Shader.h"
#include "renderer/VertexBuffer.h"
#include "renderer/IndexBuffer.h"

HitTarget::HitTarget()
{
   m_d.m_depthBias = 0.0f;
   m_d.m_reflectionEnabled = true;

   m_propPosition = nullptr;
   m_propVisual = nullptr;
   m_d.m_overwritePhysics = true;
   m_moveAnimation = false;
   m_moveDown = true;
   m_moveAnimationOffset = 0.0f;
   m_hitEvent = false;
   m_timeStamp = 0;
}

HitTarget::~HitTarget()
{
   assert(m_rd == nullptr);
}

HitTarget *HitTarget::CopyForPlay(PinTable *live_table) const
{
   STANDARD_EDITABLE_COPY_FOR_PLAY_IMPL(HitTarget, live_table)
   dst->m_hitEvent = m_hitEvent;
   return dst;
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

HRESULT HitTarget::Init(PinTable *const ptable, const float x, const float y, const bool fromMouseClick, const bool forPlay)
{
   m_ptable = ptable;
   SetDefaults(false);
   m_d.m_vPosition.x = x;
   m_d.m_vPosition.y = y;
   m_hitEvent = false;
   UpdateStatusBarInfo();
   return forPlay ? S_OK : InitVBA(fTrue, 0, nullptr);
}

void HitTarget::SetDefaults(const bool fromMouseClick)
{
#define strKeyName Settings::DefaultPropsHitTarget

   m_d.m_legacy = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "LegacyMode"s, false) : false;
   m_d.m_tdr.m_TimerEnabled = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "TimerEnabled"s, false) : false;
   m_d.m_tdr.m_TimerInterval = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "TimerInterval"s, 100) : 100;
   m_d.m_hitEvent = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "HitEvent"s, true) : true;
   m_d.m_visible = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "Visible"s, true) : true;
   m_d.m_isDropped = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "IsDropped"s, false) : false;

   // Position (X and Y is already set by the click of the user)
   m_d.m_vPosition.z = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "Position_Z"s, 0.0f) : 0.0f;

   // Size
   m_d.m_vSize.x = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "ScaleX"s, 32.0f) : 32.0f;
   m_d.m_vSize.y = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "ScaleY"s, 32.0f) : 32.0f;
   m_d.m_vSize.z = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "ScaleZ"s, 32.0f) : 32.0f;

   // Rotation and Transposition
   m_d.m_rotZ = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "Orientation"s, 0.0f) : 0.0f;

   const bool hr = g_pvp->m_settings.LoadValue(strKeyName, "Image"s, m_d.m_szImage);
   if (!hr && fromMouseClick)
      m_d.m_szImage.clear();

   m_d.m_targetType = fromMouseClick ? (TargetType)g_pvp->m_settings.LoadValueWithDefault(strKeyName, "TargetType"s, (int)DropTargetSimple) : DropTargetSimple;
   m_d.m_threshold = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "HitThreshold"s, 2.0f) : 2.0f;
   if (m_d.m_targetType == DropTargetBeveled || m_d.m_targetType == DropTargetSimple || m_d.m_targetType == DropTargetFlatSimple)
       m_d.m_dropSpeed = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "DropSpeed"s, 0.5f) : 0.5f;
   else
       m_d.m_dropSpeed = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "DropSpeed"s, 0.2f) : 0.2f;

   SetDefaultPhysics(fromMouseClick);

   m_d.m_collidable = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "Collidable"s, true) : true;
   m_d.m_disableLightingTop = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "DisableLighting"s, 0.f) : 0.f;
   m_d.m_disableLightingBelow = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "DisableLightingBelow"s, 1.f) : 1.f;
   m_d.m_reflectionEnabled = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "ReflectionEnabled"s, true) : true;
   m_d.m_raiseDelay = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "RaiseDelay"s, 100) : 100;

#undef strKeyName
}

void HitTarget::WriteRegDefaults()
{
#define strKeyName Settings::DefaultPropsHitTarget

   g_pvp->m_settings.SaveValue(strKeyName, "LegacyMode"s, m_d.m_legacy);
   g_pvp->m_settings.SaveValue(strKeyName, "TimerEnabled"s, m_d.m_tdr.m_TimerEnabled);
   g_pvp->m_settings.SaveValue(strKeyName, "TimerInterval"s, m_d.m_tdr.m_TimerInterval);
   g_pvp->m_settings.SaveValue(strKeyName, "Visible"s, m_d.m_visible);
   g_pvp->m_settings.SaveValue(strKeyName, "IsDropped"s, m_d.m_isDropped);

   g_pvp->m_settings.SaveValue(strKeyName, "Position_Z"s, m_d.m_vPosition.z);
   g_pvp->m_settings.SaveValue(strKeyName, "DropSpeed"s, m_d.m_dropSpeed);

   g_pvp->m_settings.SaveValue(strKeyName, "ScaleX"s, m_d.m_vSize.x);
   g_pvp->m_settings.SaveValue(strKeyName, "ScaleY"s, m_d.m_vSize.y);
   g_pvp->m_settings.SaveValue(strKeyName, "ScaleZ"s, m_d.m_vSize.z);

   g_pvp->m_settings.SaveValue(strKeyName, "Orientation"s, m_d.m_rotZ);

   g_pvp->m_settings.SaveValue(strKeyName, "Image"s, m_d.m_szImage);
   g_pvp->m_settings.SaveValue(strKeyName, "HitEvent"s, m_d.m_hitEvent);
   g_pvp->m_settings.SaveValue(strKeyName, "HitThreshold"s, m_d.m_threshold);
   g_pvp->m_settings.SaveValue(strKeyName, "Elasticity"s, m_d.m_elasticity);
   g_pvp->m_settings.SaveValue(strKeyName, "ElasticityFalloff"s, m_d.m_elasticityFalloff);
   g_pvp->m_settings.SaveValue(strKeyName, "Friction"s, m_d.m_friction);
   g_pvp->m_settings.SaveValue(strKeyName, "Scatter"s, m_d.m_scatter);

   g_pvp->m_settings.SaveValue(strKeyName, "TargetType"s, m_d.m_targetType);

   g_pvp->m_settings.SaveValue(strKeyName, "Collidable"s, m_d.m_collidable);
   g_pvp->m_settings.SaveValue(strKeyName, "DisableLighting"s, m_d.m_disableLightingTop);
   g_pvp->m_settings.SaveValue(strKeyName, "DisableLightingBelow"s, m_d.m_disableLightingBelow);
   g_pvp->m_settings.SaveValue(strKeyName, "ReflectionEnabled"s, m_d.m_reflectionEnabled);
   g_pvp->m_settings.SaveValue(strKeyName, "RaiseDelay"s, m_d.m_raiseDelay);

#undef strKeyName
}

// Ported at: VisualPinball.Engine/VPT/HitTarget/HitTargetHitGenerator.cs

constexpr unsigned int num_dropTargetHitPlaneVertices = 16;
static const Vertex3Ds dropTargetHitPlaneVertices[num_dropTargetHitPlaneVertices] =
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

constexpr unsigned int num_dropTargetHitPlaneIndices = 42;
constexpr WORD dropTargetHitPlaneIndices[num_dropTargetHitPlaneIndices] =
{
   0, 1, 2, 2, 3, 0, 1, 4, 5, 6, 7, 2, 5, 6, 1,
   2, 1, 6, 4, 8, 9, 9, 5, 4, 8, 10, 11, 11, 9, 8,
   6, 12, 7, 12, 6, 13, 12, 13, 14, 13, 15, 14
};

void HitTarget::BeginPlay(vector<HitTimer*> &pvht) { IEditable::BeginPlay(pvht, &m_d.m_tdr, this); }

void HitTarget::EndPlay() { IEditable::EndPlay(); }

// Ported at: VisualPinball.Engine/VPT/HitTarget/HitTargetHitGenerator.cs

void HitTarget::PhysicSetup(PhysicsEngine* physics, const bool isUI)
{
   TransformVertices();

   if (m_d.m_targetType == DropTargetBeveled || m_d.m_targetType == DropTargetFlatSimple || m_d.m_targetType == DropTargetSimple)
   {
      robin_hood::unordered_set<robin_hood::pair<unsigned, unsigned>> addedEdges;

      const Matrix3D fullMatrix = Matrix3D::MatrixRotateZ(ANGTORAD(m_d.m_rotZ));

      // add the normal drop target as collidable but without hit event
      for (unsigned i = 0; i < m_numIndices; i += 3)
      {
         const unsigned int i0 = m_indices[i];
         const unsigned int i1 = m_indices[i + 1];
         const unsigned int i2 = m_indices[i + 2];

         const Vertex3Ds rgv3D[3] = { // NB: HitTriangle wants CCW vertices, but for rendering we have them in CW order
            Vertex3Ds(m_hitUIVertices[i0].x, m_hitUIVertices[i0].y, m_hitUIVertices[i0].z), Vertex3Ds(m_hitUIVertices[i2].x, m_hitUIVertices[i2].y, m_hitUIVertices[i2].z),
            Vertex3Ds(m_hitUIVertices[i1].x, m_hitUIVertices[i1].y, m_hitUIVertices[i1].z)
         };
         SetupHitObject(physics, new HitTriangle(rgv3D), m_d.m_legacy, isUI);

         AddHitEdge(physics, addedEdges, i0, i1, rgv3D[0], rgv3D[2], m_d.m_legacy, isUI);
         AddHitEdge(physics, addedEdges, i1, i2, rgv3D[2], rgv3D[1], m_d.m_legacy, isUI);
         AddHitEdge(physics, addedEdges, i2, i0, rgv3D[1], rgv3D[0], m_d.m_legacy, isUI);
      }

      // add collision vertices
      if (!isUI)
         for (unsigned i = 0; i < m_numVertices; ++i)
            SetupHitObject(physics, new HitPoint(m_hitUIVertices[i]), m_d.m_legacy, isUI);

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
            Vertex3Ds vert(dropTargetHitPlaneVertices[i].x, dropTargetHitPlaneVertices[i].y + hitShapeOffset, dropTargetHitPlaneVertices[i].z);
            vert.x *= m_d.m_vSize.x;
            vert.y *= m_d.m_vSize.y;
            vert.z *= m_d.m_vSize.z;
            vert = fullMatrix * vert;

            rgv3D[i].x = vert.x + m_d.m_vPosition.x;
            rgv3D[i].y = vert.y + m_d.m_vPosition.y;
            rgv3D[i].z = vert.z + m_d.m_vPosition.z;
         }

         for (unsigned int i = 0; i < num_dropTargetHitPlaneIndices; i += 3)
         {
            const unsigned int i0 = dropTargetHitPlaneIndices[i];
            const unsigned int i1 = dropTargetHitPlaneIndices[i + 1];
            const unsigned int i2 = dropTargetHitPlaneIndices[i + 2];

            const Vertex3Ds rgv3D2[3] = { // NB: HitTriangle wants CCW vertices, but for rendering we have them in CW order
               rgv3D[i0], rgv3D[i2], rgv3D[i1]
            };
            SetupHitObject(physics, new HitTriangle(rgv3D2), true, isUI);

            AddHitEdge(physics, addedEdges, i0, i1, rgv3D2[0], rgv3D2[2], true, isUI);
            AddHitEdge(physics, addedEdges, i1, i2, rgv3D2[2], rgv3D2[1], true, isUI);
            AddHitEdge(physics, addedEdges, i2, i0, rgv3D2[1], rgv3D2[0], true, isUI);
         }

         // add collision vertices
         if (!isUI)
            for (unsigned i = 0; i < num_dropTargetHitPlaneVertices; ++i)
               SetupHitObject(physics, new HitPoint(rgv3D[i]), true, isUI);
      }
   }
   else
   {
      robin_hood::unordered_set<robin_hood::pair<unsigned, unsigned>> addedEdges;
      // add collision triangles and edges
      for (unsigned i = 0; i < m_numIndices; i += 3)
      {
         const unsigned int i0 = m_indices[i];
         const unsigned int i1 = m_indices[i + 1];
         const unsigned int i2 = m_indices[i + 2];

         const Vertex3Ds rgv3D[3] = { // NB: HitTriangle wants CCW vertices, but for rendering we have them in CW order
            Vertex3Ds(m_hitUIVertices[i0].x, m_hitUIVertices[i0].y, m_hitUIVertices[i0].z), Vertex3Ds(m_hitUIVertices[i2].x, m_hitUIVertices[i2].y, m_hitUIVertices[i2].z),
            Vertex3Ds(m_hitUIVertices[i1].x, m_hitUIVertices[i1].y, m_hitUIVertices[i1].z)
         };
         SetupHitObject(physics, new HitTriangle(rgv3D), true, isUI);

         AddHitEdge(physics, addedEdges, i0, i1, rgv3D[0], rgv3D[2], true, isUI);
         AddHitEdge(physics, addedEdges, i1, i2, rgv3D[2], rgv3D[1], true, isUI);
         AddHitEdge(physics, addedEdges, i2, i0, rgv3D[1], rgv3D[0], true, isUI);
      }

      // add collision vertices
      if (!isUI)
         for (unsigned i = 0; i < m_numVertices; ++i)
            SetupHitObject(physics, new HitPoint(m_hitUIVertices[i]), true, isUI);
   }
}

void HitTarget::PhysicRelease(PhysicsEngine* physics, const bool isUI)
{
   if (!isUI)
      m_vhoCollidable.clear();
}

void HitTarget::AddHitEdge(PhysicsEngine* physics, robin_hood::unordered_set< robin_hood::pair<unsigned, unsigned> >& addedEdges, const unsigned i, const unsigned j, const Vertex3Ds &vi, const Vertex3Ds &vj, const bool setHitObject, const bool isUI)
{
   // create pair uniquely identifying the edge (i,j)
   const robin_hood::pair<unsigned, unsigned> p(std::min(i, j), std::max(i, j));
   if (!isUI && addedEdges.insert(p).second) // edge not yet added?
      SetupHitObject(physics, new HitLine3D(vi, vj), setHitObject, isUI);
}

// Ported at: VisualPinball.Engine/Physics/HitObject.cs

void HitTarget::SetupHitObject(PhysicsEngine* physics, HitObject *obj, const bool setHitObject, const bool isUI)
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
   obj->m_enabled = isUI ? true : m_d.m_collidable;
   obj->m_ObjType = eHitTarget;
   obj->m_obj = (IFireEvents*)this;
   obj->m_e = 2;
   obj->m_fe = setHitObject && m_d.m_hitEvent;

   physics->AddCollider(obj, this, isUI);

   if (!isUI)
      m_vhoCollidable.push_back(obj);	//remember hit components of primitive
}

//////////////////////////////
// Calculation
//////////////////////////////

void HitTarget::GenerateMesh(vector<Vertex3D_NoTex2> &buf)
{
   SetMeshType(m_d.m_targetType);

   const Matrix3D fullMatrix = Matrix3D::MatrixRotateZ(ANGTORAD(m_d.m_rotZ));

   for (unsigned int i = 0; i < m_numVertices; i++)
   {
      Vertex3Ds vert(m_vertices[i].x, m_vertices[i].y, m_vertices[i].z);
      vert.x *= m_d.m_vSize.x;
      vert.y *= m_d.m_vSize.y;
      vert.z *= m_d.m_vSize.z;
      vert = fullMatrix * vert;

      buf[i].x = vert.x + m_d.m_vPosition.x;
      buf[i].y = vert.y + m_d.m_vPosition.y;
      buf[i].z = vert.z + m_d.m_vPosition.z;

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

   m_hitUIVertices.resize(m_numVertices);
   const Matrix3D fullMatrix = Matrix3D::MatrixRotateZ(ANGTORAD(m_d.m_rotZ));

   for (unsigned i = 0; i < m_numVertices; i++)
   {
      Vertex3Ds vert(m_vertices[i].x, m_vertices[i].y, m_vertices[i].z);
      vert.x *= m_d.m_vSize.x;
      vert.y *= m_d.m_vSize.y;
      vert.z *= m_d.m_vSize.z;
      vert = fullMatrix * vert;

      m_hitUIVertices[i].x = vert.x + m_d.m_vPosition.x;
      m_hitUIVertices[i].y = vert.y + m_d.m_vPosition.y;
      m_hitUIVertices[i].z = vert.z + m_d.m_vPosition.z;
   }
}

void HitTarget::ExportMesh(ObjLoader& loader)
{
   char name[sizeof(m_wzName)/sizeof(m_wzName[0])];
   WideCharToMultiByteNull(CP_ACP, 0, m_wzName, -1, name, sizeof(name), nullptr, nullptr);

   SetMeshType(m_d.m_targetType);

   m_transformedVertices.resize(m_numVertices);

   loader.WriteObjectName(name);

   GenerateMesh(m_transformedVertices);

   loader.WriteVertexInfo(m_transformedVertices.data(), m_numVertices);
   const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);
   loader.WriteMaterial(m_d.m_szMaterial, string(), mat);
   loader.UseTexture(m_d.m_szMaterial);
   loader.WriteFaceInfoList(m_indices, m_numIndices);
   loader.UpdateFaceOffset(m_numVertices);
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
    constexpr float halflength = 50.0f;
    constexpr float len1 = halflength * 0.5f;
    constexpr float len2 = len1 * 0.5f;
    {
       Vertex2D tmp;

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

#pragma region Rendering

void HitTarget::RenderSetup(RenderDevice *device)
{
   assert(m_rd == nullptr);
   m_rd = device;

   SetMeshType(m_d.m_targetType);
   m_transformedVertices.resize(m_numVertices);

   GenerateMesh(m_transformedVertices);
   delete m_meshBuffer;
   VertexBuffer *vertexBuffer = new VertexBuffer(m_rd, (unsigned int)m_numVertices, (float *)m_transformedVertices.data(), true);
   IndexBuffer *indexBuffer = new IndexBuffer(m_rd, m_numIndices, m_indices);
   m_meshBuffer = new MeshBuffer(m_wzName, vertexBuffer, indexBuffer, true);

   m_moveAnimationOffset = 0.0f;
   if (m_d.m_targetType == DropTargetBeveled || m_d.m_targetType == DropTargetSimple || m_d.m_targetType == DropTargetFlatSimple)
   {
       if (m_d.m_isDropped)
       {
           m_moveDown = false;
           m_moveAnimationOffset = -DROP_TARGET_LIMIT;
           UpdateTarget();
           return;
       }
   }
}

void HitTarget::RenderRelease()
{
   assert(m_rd != nullptr);
   delete m_meshBuffer;
   m_meshBuffer = nullptr;
   m_rd = nullptr;
}

// Ported at: VisualPinball.Unity/VisualPinball.Unity/VPT/HitTarget/HitTargetAnimationSystem.cs

void HitTarget::UpdateAnimation(const float diff_time_msec)
{
    // Do not perform animation of invisible targets
    // This is needed for backward compatibility since animation used to be part of rendering and would not be performed, therefore
    // hidden drop targets would never actually drop, and old tables rely on this behavior.
    if (!m_d.m_visible)
       return;

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
            float step = m_d.m_dropSpeed;
            constexpr float limit = DROP_TARGET_LIMIT;
            if (m_moveDown)
                step = -step;
            else if ((g_pplayer->m_time_msec - m_timeStamp) < (unsigned int)m_d.m_raiseDelay)
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
                    if (m_d.m_hitEvent)
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
                    if (m_d.m_hitEvent)
                        FireGroupEvent(DISPID_TargetEvents_Raised);
                }
            }
            UpdateTarget();
            FireGroupEvent(DISPID_AnimateEvents_Animate);
        }
    }
    else
    {
        if (m_moveAnimation)
        {
            float step = m_d.m_dropSpeed;
            constexpr float limit = 13.0f;
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
            FireGroupEvent(DISPID_AnimateEvents_Animate);
        }
    }
}

void HitTarget::Render(const unsigned int renderMask)
{
   assert(m_rd != nullptr);
   const bool isStaticOnly = renderMask & Renderer::STATIC_ONLY;
   const bool isDynamicOnly = renderMask & Renderer::DYNAMIC_ONLY;
   const bool isReflectionPass = renderMask & Renderer::REFLECTION_PASS;
   TRACE_FUNCTION();
   
   if (isStaticOnly 
   || !m_d.m_visible 
   || (isReflectionPass && !m_d.m_reflectionEnabled))
      return;

   m_rd->ResetRenderState();
   m_rd->m_basicShader->SetVector(SHADER_fDisableLighting_top_below, m_d.m_disableLightingTop, m_d.m_disableLightingBelow, 0.f, 0.f);
   const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);
   m_rd->m_basicShader->SetBasic(mat, m_ptable->GetImage(m_d.m_szImage));
   #ifdef TWOSIDED_TRANSPARENCY
   if (mat->m_bOpacityActive)
   {
      RenderState::RenderStateValue cullMode = m_rd->GetRenderState().GetRenderState(RenderState::CULLMODE);
      m_rd->SetRenderState(RenderState::CULLMODE, cullMode == RenderState::CULL_CCW ? RenderState::CULL_CW : RenderState::CULL_CCW);
      m_rd->DrawMesh(m_rd->m_basicShader, mat->m_bOpacityActive, m_d.m_vPosition, m_d.m_depthBias, m_meshBuffer, RenderDevice::TRIANGLELIST, 0, m_numIndices);
      m_rd->SetRenderState(RenderState::CULLMODE, cullMode);
      m_rd->DrawMesh(m_rd->m_basicShader, mat->m_bOpacityActive, m_d.m_vPosition, m_d.m_depthBias, m_meshBuffer, RenderDevice::TRIANGLELIST, 0, m_numIndices);
   }
   else
   {
      m_rd->DrawMesh(m_rd->m_basicShader, mat->m_bOpacityActive, m_d.m_vPosition, m_d.m_depthBias, m_meshBuffer, RenderDevice::TRIANGLELIST, 0, m_numIndices);
   }
   #else
   m_rd->DrawMesh(m_rd->m_basicShader, mat->m_bOpacityActive, m_d.m_vPosition, m_d.m_depthBias, m_meshBuffer, RenderDevice::TRIANGLELIST, 0, m_numIndices);
   #endif
   m_rd->m_basicShader->SetVector(SHADER_fDisableLighting_top_below, 0.f, 0.f, 0.f, 0.f);
}

void HitTarget::UpdateTarget()
{
   Vertex3D_NoTex2 *buf;
   m_meshBuffer->m_vb->Lock(buf);
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
       const Matrix3D fullMatrix = Matrix3D::MatrixRotateX(ANGTORAD(m_moveAnimationOffset))
                                 * Matrix3D::MatrixRotateZ(ANGTORAD(m_d.m_rotZ));
       const Matrix3D vertMatrix = (Matrix3D::MatrixScale(m_d.m_vSize.x, m_d.m_vSize.y, m_d.m_vSize.z)
                                  * fullMatrix)
                                  * Matrix3D::MatrixTranslate(m_d.m_vPosition.x, m_d.m_vPosition.y, m_d.m_vPosition.z);

       //TODO Update object Matrix instead
       for (unsigned int i = 0; i < m_numVertices; i++)
       {
           Vertex3Ds vert = vertMatrix * Vertex3Ds{m_vertices[i].x, m_vertices[i].y, m_vertices[i].z};
           buf[i].x = vert.x;
           buf[i].y = vert.y;
           buf[i].z = vert.z;

           vert = fullMatrix.MultiplyVectorNoTranslate(Vertex3Ds{m_vertices[i].nx, m_vertices[i].ny, m_vertices[i].nz});
           buf[i].nx = vert.x;
           buf[i].ny = vert.y;
           buf[i].nz = vert.z;
           buf[i].tu = m_vertices[i].tu;
           buf[i].tv = m_vertices[i].tv;
       }
   }
   m_meshBuffer->m_vb->Unlock();
}

#pragma endregion

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
   return {m_d.m_vPosition.x, m_d.m_vPosition.y};
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

HRESULT HitTarget::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, const bool saveForUndo)
{
   BiffWriter bw(pstm, hcrypthash);

   /*
    * Someone decided that it was a good idea to write these vectors including
    * the fourth padding float that they used to have, so now we have to write
    * them padded to 4 floats to maintain compatibility.
    */
   bw.WriteVector3Padded(FID(VPOS), m_d.m_vPosition);
   bw.WriteVector3Padded(FID(VSIZ), m_d.m_vSize);
   bw.WriteFloat(FID(ROTZ), m_d.m_rotZ);
   bw.WriteString(FID(IMAG), m_d.m_szImage);
   bw.WriteInt(FID(TRTY), m_d.m_targetType);
   bw.WriteWideString(FID(NAME), m_wzName);
   bw.WriteString(FID(MATR), m_d.m_szMaterial);
   bw.WriteBool(FID(TVIS), m_d.m_visible);
   bw.WriteBool(FID(LEMO), m_d.m_legacy);
   bw.WriteBool(FID(HTEV), m_d.m_hitEvent);
   bw.WriteFloat(FID(THRS), m_d.m_threshold);
   bw.WriteFloat(FID(ELAS), m_d.m_elasticity);
   bw.WriteFloat(FID(ELFO), m_d.m_elasticityFalloff);
   bw.WriteFloat(FID(RFCT), m_d.m_friction);
   bw.WriteFloat(FID(RSCT), m_d.m_scatter);
   bw.WriteBool(FID(CLDR), m_d.m_collidable);
   bw.WriteFloat(FID(DILT), m_d.m_disableLightingTop);
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
   case FID(VPOS): pbr->GetVector3Padded(m_d.m_vPosition); break;
   case FID(VSIZ): pbr->GetVector3Padded(m_d.m_vSize); break;
   case FID(ROTZ): pbr->GetFloat(m_d.m_rotZ); break;
   case FID(IMAG): pbr->GetString(m_d.m_szImage); break;
   case FID(TRTY): pbr->GetInt(&m_d.m_targetType); break;
   case FID(NAME): pbr->GetWideString(m_wzName, std::size(m_wzName)); break;
   case FID(MATR): pbr->GetString(m_d.m_szMaterial); break;
   case FID(TVIS): pbr->GetBool(m_d.m_visible); break;
   case FID(LEMO): pbr->GetBool(m_d.m_legacy); break;
   case FID(ISDR): pbr->GetBool(m_d.m_isDropped); break;
   case FID(DRSP): pbr->GetFloat(m_d.m_dropSpeed); break;
   case FID(REEN): pbr->GetBool(m_d.m_reflectionEnabled); break;
   case FID(HTEV): pbr->GetBool(m_d.m_hitEvent); break;
   case FID(THRS): pbr->GetFloat(m_d.m_threshold); break;
   case FID(ELAS): pbr->GetFloat(m_d.m_elasticity); break;
   case FID(ELFO): pbr->GetFloat(m_d.m_elasticityFalloff); break;
   case FID(RFCT): pbr->GetFloat(m_d.m_friction); break;
   case FID(RSCT): pbr->GetFloat(m_d.m_scatter); break;
   case FID(CLDR): pbr->GetBool(m_d.m_collidable); break;
   case FID(DILI): { int tmp; pbr->GetInt(tmp); m_d.m_disableLightingTop = (tmp == 1) ? 1.f : dequantizeUnsigned<8>(tmp); break; } // Pre 10.8 compatible hacky loading!
   case FID(DILT): pbr->GetFloat(m_d.m_disableLightingTop); break;
   case FID(DILB): pbr->GetFloat(m_d.m_disableLightingBelow); break;
   case FID(PIDB): pbr->GetFloat(m_d.m_depthBias); break;
   case FID(TMON): pbr->GetBool(m_d.m_tdr.m_TimerEnabled); break;
   case FID(TMIN): pbr->GetInt(m_d.m_tdr.m_TimerInterval); break;
   case FID(RADE): pbr->GetInt(m_d.m_raiseDelay); break;
   case FID(MAPH): pbr->GetString(m_d.m_szPhysicsMaterial); break;
   case FID(OVPH): pbr->GetBool(m_d.m_overwritePhysics); break;
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
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szImage.c_str(), -1, wz, MAXTOKEN);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP HitTarget::put_Image(BSTR newVal)
{
   char szImage[MAXTOKEN];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, szImage, MAXTOKEN, nullptr, nullptr);
   const Texture * const tex = m_ptable->GetImage(szImage);
   if (tex && tex->IsHDR())
   {
       ShowError("Cannot use a HDR image (.exr/.hdr) here");
       return E_FAIL;
   }
   m_d.m_szImage = szImage;

   return S_OK;
}


float HitTarget::GetDepth(const Vertex3Ds& viewDir) const
{
   return m_d.m_depthBias + m_d.m_vPosition.Dot(viewDir);
}

STDMETHODIMP HitTarget::get_Material(BSTR *pVal)
{
   WCHAR wz[MAXNAMEBUFFER];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szMaterial.c_str(), -1, wz, MAXNAMEBUFFER);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP HitTarget::put_Material(BSTR newVal)
{
   char buf[MAXNAMEBUFFER];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXNAMEBUFFER, nullptr, nullptr);
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
   m_vpinball->SetStatusBarUnitInfo(string(), true);

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
   *pVal = FTOVB(m_d.m_hitEvent);
   return S_OK;
}

STDMETHODIMP HitTarget::put_HasHitEvent(VARIANT_BOOL newVal)
{
   m_d.m_hitEvent = VBTOb(newVal);
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
   *pVal = FTOVB(m_d.m_collidable);
   return S_OK;
}

STDMETHODIMP HitTarget::put_Collidable(VARIANT_BOOL newVal)
{
   const bool val = VBTOb(newVal);
   if (m_d.m_collidable != val)
   {
      m_d.m_collidable = val;
      for (auto collidable : m_vhoCollidable)
         collidable->m_enabled = val;
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
   m_d.m_disableLightingTop = VBTOb(newVal) ? 1.f : 0.f;
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

void HitTarget::SetDefaultPhysics(const bool fromMouseClick)
{
#define strKeyName Settings::DefaultPropsHitTarget

   m_d.m_elasticity = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "Elasticity"s, 0.35f) : 0.35f;
   m_d.m_elasticityFalloff = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "ElasticityFalloff"s, 0.5f) : 0.5f;
   m_d.m_friction = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "Friction"s, 0.2f) : 0.2f;
   m_d.m_scatter = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "Scatter"s, 5.f) : 5.f;

#undef strKeyName
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

STDMETHODIMP HitTarget::get_CurrentAnimOffset(float *pVal)
{
   *pVal = m_moveAnimationOffset;
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
   if (m_d.m_isDropped != val)
   {
      PLOGD << GetName() << " set IsDropped to " << val;
      m_d.m_isDropped = val;
      if (g_pplayer)
      {
         m_moveAnimation = true;
         if (val)
         {
            m_moveAnimationOffset = 0.0f;
            m_moveDown = true;
         }
         else
         {
            m_moveAnimationOffset = -DROP_TARGET_LIMIT;
            m_moveDown = false;
            m_timeStamp = g_pplayer->m_time_msec;
         }
      }
   }

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
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szPhysicsMaterial.c_str(), -1, wz, MAXNAMEBUFFER);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP HitTarget::put_PhysicsMaterial(BSTR newVal)
{
   char buf[MAXNAMEBUFFER];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXNAMEBUFFER, nullptr, nullptr);
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

STDMETHODIMP HitTarget::get_RaiseDelay(LONG *pVal)
{
    *pVal = m_d.m_raiseDelay;
    return S_OK;
}

STDMETHODIMP HitTarget::put_RaiseDelay(LONG newVal)
{
    m_d.m_raiseDelay = newVal;
    return S_OK;
}
