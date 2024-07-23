// primitive.cpp: implementation of the Primitive class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h" 
#include "forsyth.h"
#include "objloader.h"
#include "miniz/miniz.h"
#include "progmesh.h"
#include "ThreadPool.h"
#include "renderer/Shader.h"

ThreadPool *g_pPrimitiveDecompressThreadPool = nullptr;

void Mesh::Clear()
{
   m_vertices.clear();
   m_indices.clear();
   for (size_t i = 0; i < m_animationFrames.size(); i++)
      m_animationFrames[i].m_frameVerts.clear();
   m_animationFrames.clear();
   middlePoint.x = 0.0f;
   middlePoint.y = 0.0f;
   middlePoint.z = 0.0f;
   m_validBounds = false;
}

bool Mesh::LoadAnimation(const char *fname, const bool flipTV, const bool convertToLeftHanded)
{
   m_validBounds = false;
   string name(fname);
   size_t idx = name.find_last_of('_');
   if (idx == string::npos)
   {
      ShowError("Can't find sequence of obj files! The file name of the sequence must be <meshname>_x.obj where x is the frame number!");
      return false;
   }
   idx++;
   name = name.substr(0,idx);
   string sname = name + "*.obj";
#ifndef __STANDALONE__
   WIN32_FIND_DATA data;
   const HANDLE h = FindFirstFile(sname.c_str(), &data);
   vector<string> allFiles;
   int frameCounter = 0;
   if (h != INVALID_HANDLE_VALUE)
   {
      do
      {
         allFiles.push_back(data.cFileName);
         frameCounter++;
      } while (FindNextFile(h, &data));
   }
   m_animationFrames.resize(frameCounter);
   for (size_t i = 0; i < allFiles.size(); i++)
   {
      sname = allFiles[i];
      ObjLoader loader;
      if (loader.Load(sname, flipTV, convertToLeftHanded))
      {
         const vector<Vertex3D_NoTex2>& verts = loader.GetVertices();
         const vector<unsigned int>& indices = loader.GetIndices();
         if ((m_indices.size() != indices.size()) || (m_vertices.size() != verts.size()) || (memcmp(m_indices.data(), indices.data(), indices.size()*sizeof(unsigned int)) != 0))
         {
            ShowError("Error: frames of animation do not share the same data layout.");
            return false;
         }
         for (size_t t = 0; t < verts.size(); t++)
         {
            VertData vd;
            vd.x = verts[t].x; vd.y = verts[t].y; vd.z = verts[t].z;
            vd.nx = verts[t].nx; vd.ny = verts[t].ny; vd.nz = verts[t].nz;
            m_animationFrames[i].m_frameVerts.push_back(vd);
         }
      }
      else
      {
         name = "Unable to load file " + sname;
         ShowError(name);
         return false;
      }

   }
   sname = std::to_string(frameCounter)+" frames imported!";
   g_pvp->MessageBox(sname.c_str(), "Info", MB_OK | MB_ICONEXCLAMATION);
#endif
   return true;
}

bool Mesh::LoadWavefrontObj(const string& fname, const bool flipTV, const bool convertToLeftHanded)
{
   m_validBounds = false;
   Clear();
   ObjLoader loader;
   if (loader.Load(fname, flipTV, convertToLeftHanded))
   {
      m_vertices = loader.GetVertices();
      m_indices = loader.GetIndices();
      float maxX = -FLT_MAX, minX = FLT_MAX;
      float maxY = -FLT_MAX, minY = FLT_MAX;
      float maxZ = -FLT_MAX, minZ = FLT_MAX;

      for (size_t i = 0; i < m_vertices.size(); i++)
      {
         if (m_vertices[i].x > maxX) maxX = m_vertices[i].x;
         if (m_vertices[i].x < minX) minX = m_vertices[i].x;
         if (m_vertices[i].y > maxY) maxY = m_vertices[i].y;
         if (m_vertices[i].y < minY) minY = m_vertices[i].y;
         if (m_vertices[i].z > maxZ) maxZ = m_vertices[i].z;
         if (m_vertices[i].z < minZ) minZ = m_vertices[i].z;
      }
      middlePoint.x = (maxX + minX)*0.5f;
      middlePoint.y = (maxY + minY)*0.5f;
      middlePoint.z = (maxZ + minZ)*0.5f;

      return true;
   }
   else
      return false;
}

void Mesh::SaveWavefrontObj(const string& fname, const string& description)
{
   ObjLoader loader;
   loader.Save(fname, description.empty() ? fname : description, *this);
}

void Mesh::UploadToVB(VertexBuffer * vb, const float frame) 
{
   if(!vb)
      return;

   if (frame >= 0.f)
   {
      float intPart;
      const float fractpart = modff(frame, &intPart);
      const int iFrame = (int)intPart;

      if (iFrame+1 < (int)m_animationFrames.size())
      {
          for (size_t i = 0; i < m_vertices.size(); i++)
          {
              const VertData& v  = m_animationFrames[iFrame  ].m_frameVerts[i];
              const VertData& v2 = m_animationFrames[iFrame+1].m_frameVerts[i];
              m_vertices[i].x  = v.x  + (v2.x  - v.x) *fractpart;
              m_vertices[i].y  = v.y  + (v2.y  - v.y) *fractpart;
              m_vertices[i].z  = v.z  + (v2.z  - v.z) *fractpart;
              m_vertices[i].nx = v.nx + (v2.nx - v.nx)*fractpart;
              m_vertices[i].ny = v.ny + (v2.ny - v.ny)*fractpart;
              m_vertices[i].nz = v.nz + (v2.nz - v.nz)*fractpart;
          }
      }
      else
          for (size_t i = 0; i < m_vertices.size(); i++)
          {
              const VertData& v = m_animationFrames[iFrame].m_frameVerts[i];
              m_vertices[i].x  = v.x;
              m_vertices[i].y  = v.y;
              m_vertices[i].z  = v.z;
              m_vertices[i].nx = v.nx;
              m_vertices[i].ny = v.ny;
              m_vertices[i].nz = v.nz;
          }
   }

   Vertex3D_NoTex2 *buf;
   vb->lock(0, 0, (void**)&buf, VertexBuffer::WRITEONLY);
   memcpy(buf, m_vertices.data(), sizeof(Vertex3D_NoTex2)*m_vertices.size());
   vb->unlock();
}

void Mesh::UpdateBounds()
{
   if (!m_validBounds)
   {
      m_validBounds = true;
      m_minAABound = Vertex3Ds(FLT_MAX, FLT_MAX, FLT_MAX);
      m_maxAABound = Vertex3Ds(-FLT_MAX, -FLT_MAX, -FLT_MAX);
      for (const Vertex3D_NoTex2 &v : m_vertices)
      {
          m_minAABound.x = min(m_minAABound.x, v.x);
          m_minAABound.y = min(m_minAABound.y, v.y);
          m_minAABound.z = min(m_minAABound.z, v.z);
          m_maxAABound.x = max(m_maxAABound.x, v.x);
          m_maxAABound.y = max(m_maxAABound.y, v.y);
          m_maxAABound.z = max(m_maxAABound.z, v.z);
      }
   }
}

////////////////////////////////////////////////////////////////////////////////

Primitive::Primitive()
{
   m_meshBuffer = nullptr;
   m_vertexBufferRegenerate = true;
   m_d.m_use3DMesh = false;
   m_d.m_staticRendering = false;
   m_d.m_edgeFactorUI = 0.25f;
   m_d.m_collision_reductionFactor = 0.f;
   m_d.m_depthBias = 0.0f;
   m_d.m_skipRendering = false;
   m_d.m_groupdRendering = false;
   m_d.m_reflectionEnabled = true;
   m_numGroupIndices = 0;
   m_numGroupVertices = 0;
   m_currentFrame = -1.f;
   m_lockedByLS = false;

   m_speed = 0.0f;
   m_ptable = nullptr;
   m_inPlayState = false;
   m_endless = false;
   m_doAnimation = false;
   m_compressedVertices = false;
   m_compressedIndices = false;
   m_compressedAnimationVertices = false;

   m_numIndices = 0;
   m_numVertices = 0;
   m_propPhysics = nullptr;
   m_propPosition = nullptr;
   m_propVisual = nullptr;
   m_d.m_overwritePhysics = true;
   m_d.m_useAsPlayfield = false;
}

Primitive::~Primitive()
{
   WaitForMeshDecompression(); //!! needed nowadays due to multithreaded mesh decompression
   assert(m_rd == nullptr); // RenderRelease must be explicitely called before deleting this object
}

Primitive *Primitive::CopyForPlay(PinTable *live_table)
{
   STANDARD_EDITABLE_COPY_FOR_PLAY_IMPL(Primitive, live_table)
   dst->m_mesh = m_mesh;
   return dst;
}

void Primitive::CreateRenderGroup(const Collection * const collection)
{
   if (!collection->m_groupElements)
      return;

   size_t overall_size = 0;
   vector<Primitive*> prims;
   vector<Primitive*> renderedPrims;
   for (int i = 0; i < collection->m_visel.size(); i++)
   {
      const ISelect * const pisel = collection->m_visel.ElementAt(i);
      if (pisel->GetItemType() != eItemPrimitive)
         continue;

      Primitive * const prim = (Primitive*)pisel;
      // only support dynamic mesh primitives for now
      if (!prim->m_d.m_use3DMesh || prim->m_d.m_staticRendering)
         continue;

      prims.push_back(prim);
   }

   if (prims.size() <= 1)
      return;

   // The first primitive in the group is the base primitive
   // this element gets rendered by rendering all other group primitives
   // the rest of the group is marked as skipped rendering
   const Material * const groupMaterial = m_ptable->GetMaterial(prims[0]->m_d.m_szMaterial);
   const Texture * const groupTexel = m_ptable->GetImage(prims[0]->m_d.m_szImage);
   m_numGroupVertices = (int)prims[0]->m_mesh.NumVertices();
   m_numGroupIndices = (int)prims[0]->m_mesh.NumIndices();
   overall_size = prims[0]->m_mesh.NumIndices();

   // Now calculate the overall size of indices
   for (size_t i = 1; i < prims.size(); i++)
   {
      const Material * const mat = m_ptable->GetMaterial(prims[i]->m_d.m_szMaterial);
      const Texture * const texel = m_ptable->GetImage(prims[i]->m_d.m_szImage);
      if (mat == groupMaterial && texel == groupTexel)
      {
         overall_size += prims[i]->m_mesh.NumIndices();
      }
   }

   // all primitives in the collection don't share the same texture and material
   // don't group them and render them as usual
   if (overall_size == prims[0]->m_mesh.NumIndices())
      return;

   prims[0]->m_d.m_groupdRendering = true;
   vector<unsigned int> indices(overall_size);

   // copy with a loop because memcpy seems to do some strange things with the indices
   for (size_t k = 0; k < prims[0]->m_mesh.NumIndices(); k++)
      indices[k] = prims[0]->m_mesh.m_indices[k];

   renderedPrims.push_back(prims[0]);
   for (size_t i = 1; i < prims.size(); i++)
   {
      const Material * const mat = m_ptable->GetMaterial(prims[i]->m_d.m_szMaterial);
      const Texture * const texel = m_ptable->GetImage(prims[i]->m_d.m_szImage);
      if (mat == groupMaterial && texel == groupTexel)
      {
         const Mesh &m = prims[i]->m_mesh;
         for (size_t k = 0; k < m.NumIndices(); k++)
            indices[m_numGroupIndices + k] = m_numGroupVertices + m.m_indices[k];

         m_numGroupVertices += (int)m.NumVertices();
         m_numGroupIndices += (int)m.NumIndices();
         prims[i]->m_d.m_skipRendering = true;
         renderedPrims.push_back(prims[i]);
      }
      else
         prims[i]->m_d.m_skipRendering = false;
   }

   VertexBuffer* vertexBuffer = new VertexBuffer(g_pplayer->m_pin3d.m_pd3dPrimaryDevice, m_numGroupVertices);
   IndexBuffer *indexBuffer = new IndexBuffer(g_pplayer->m_pin3d.m_pd3dPrimaryDevice, indices);
   unsigned int ofs = 0;
   Vertex3D_NoTex2 *buf;
   vertexBuffer->lock(0, 0, (void**)&buf, VertexBuffer::WRITEONLY);
   for (size_t i = 0; i < renderedPrims.size(); i++)
   {
      renderedPrims[i]->RecalculateMatrices();
      const Mesh &m = renderedPrims[i]->m_mesh;
      for (size_t t = 0; t < m.NumVertices(); t++)
      {
         Vertex3D_NoTex2 vt = m.m_vertices[t];
         renderedPrims[i]->m_fullMatrix.MultiplyVector(vt, vt);

         Vertex3Ds n;
         renderedPrims[i]->m_fullMatrix.MultiplyVectorNoTranslateNormal(vt, n);
         vt.nx = n.x; vt.ny = n.y; vt.nz = n.z;
         buf[ofs] = vt;
         ofs++;
      }
   }
   vertexBuffer->unlock();
   delete prims[0]->m_meshBuffer;
   prims[0]->m_meshBuffer = new MeshBuffer(m_wzName + L".RenderGroup"s, vertexBuffer, indexBuffer, true);
}

HRESULT Primitive::Init(PinTable *const ptable, const float x, const float y, const bool fromMouseClick, const bool forPlay)
{
   m_ptable = ptable;
   SetDefaults(fromMouseClick);
   m_d.m_vPosition.x = x;
   m_d.m_vPosition.y = y;
   UpdateStatusBarInfo();
   return forPlay ? S_OK : InitVBA(fTrue, 0, nullptr);
}

void Primitive::SetDefaults(const bool fromMouseClick)
{
#define strKeyName Settings::DefaultPropsPrimitive

   m_d.m_useAsPlayfield = false;
   m_d.m_use3DMesh = false;

   m_d.m_meshFileName.clear();
   // sides
   m_d.m_Sides = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "Sides"s, 4) : 4;
   if (m_d.m_Sides > Max_Primitive_Sides)
      m_d.m_Sides = Max_Primitive_Sides;

   // colors
   m_d.m_SideColor = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "SideColor"s, (int)RGB(150, 150, 150)) : RGB(150, 150, 150);

   m_d.m_visible = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "Visible"s, true) : true;
   m_inPlayState = m_d.m_visible;
   m_d.m_staticRendering = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "StaticRendering"s, true) : true;
   m_d.m_drawTexturesInside = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "DrawTexturesInside"s, false) : false;

   // Position (X and Y is already set by the click of the user)
   m_d.m_vPosition.z = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "Position_Z"s, 0.0f) : 0.0f;

   // Size
   m_d.m_vSize.x = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "Size_X"s, 100.0f) : 100.0f;
   m_d.m_vSize.y = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "Size_Y"s, 100.0f) : 100.0f;
   m_d.m_vSize.z = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "Size_Z"s, 100.0f) : 100.0f;

   // Rotation and Transposition
   m_d.m_aRotAndTra[0] = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "RotAndTra0"s, 0.0f) : 0.0f;
   m_d.m_aRotAndTra[1] = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "RotAndTra1"s, 0.0f) : 0.0f;
   m_d.m_aRotAndTra[2] = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "RotAndTra2"s, 0.0f) : 0.0f;
   m_d.m_aRotAndTra[3] = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "RotAndTra3"s, 0.0f) : 0.0f;
   m_d.m_aRotAndTra[4] = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "RotAndTra4"s, 0.0f) : 0.0f;
   m_d.m_aRotAndTra[5] = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "RotAndTra5"s, 0.0f) : 0.0f;
   m_d.m_aRotAndTra[6] = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "RotAndTra6"s, 0.0f) : 0.0f;
   m_d.m_aRotAndTra[7] = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "RotAndTra7"s, 0.0f) : 0.0f;
   m_d.m_aRotAndTra[8] = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "RotAndTra8"s, 0.0f) : 0.0f;

   bool hr = g_pvp->m_settings.LoadValue(strKeyName, "Image"s, m_d.m_szImage);
   if (!hr && fromMouseClick)
      m_d.m_szImage.clear();

   hr = g_pvp->m_settings.LoadValue(strKeyName, "NormalMap"s, m_d.m_szNormalMap);
   if (!hr  && fromMouseClick)
       m_d.m_szNormalMap.clear();

   m_d.m_threshold = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "HitThreshold"s, 2.0f) : 2.0f;

   SetDefaultPhysics(fromMouseClick);

   m_d.m_alpha = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "Opacity"s, 100.0f) : 100.0f;
   m_d.m_addBlend = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "AddBlend"s, false) : false;
   m_d.m_useDepthMask = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "DepthMask"s, true) : true;
   m_d.m_color = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "Color"s, (int)RGB(255, 255, 255)) : RGB(255, 255, 255);

   m_d.m_edgeFactorUI = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "EdgeFactorUI"s, 0.25f) : 0.25f;
   m_d.m_collision_reductionFactor = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "CollisionReductionFactor"s, 0.f) : 0.f;

   m_d.m_collidable = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "Collidable"s, true) : true;
   m_d.m_toy = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "IsToy"s, false) : false;
   m_d.m_disableLightingTop = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "DisableLighting"s, 0.f) : 0.f;
   m_d.m_disableLightingBelow = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "DisableLightingBelow"s, 1.f) : 1.f;
   m_d.m_reflectionEnabled = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "ReflectionEnabled"s, true) : true;
   m_d.m_backfacesEnabled = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "BackfacesEnabled"s, false) : false;
   m_d.m_displayTexture = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "DisplayTexture"s, false) : false;
   m_d.m_objectSpaceNormalMap = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "ObjectSpaceNormalMap"s, false) : false;

#undef strKeyName
}

void Primitive::WriteRegDefaults()
{
#define strKeyName Settings::DefaultPropsPrimitive

   g_pvp->m_settings.SaveValue(strKeyName, "SideColor"s, (int)m_d.m_SideColor);
   g_pvp->m_settings.SaveValue(strKeyName, "Visible"s, m_d.m_visible);
   g_pvp->m_settings.SaveValue(strKeyName, "StaticRendering"s, m_d.m_staticRendering);
   g_pvp->m_settings.SaveValue(strKeyName, "DrawTexturesInside"s, m_d.m_drawTexturesInside);

   g_pvp->m_settings.SaveValue(strKeyName, "Position_Z"s, m_d.m_vPosition.z);

   g_pvp->m_settings.SaveValue(strKeyName, "Size_X"s, m_d.m_vSize.x);
   g_pvp->m_settings.SaveValue(strKeyName, "Size_Y"s, m_d.m_vSize.y);
   g_pvp->m_settings.SaveValue(strKeyName, "Size_Z"s, m_d.m_vSize.z);

   g_pvp->m_settings.SaveValue(strKeyName, "RotAndTra0"s, m_d.m_aRotAndTra[0]);
   g_pvp->m_settings.SaveValue(strKeyName, "RotAndTra1"s, m_d.m_aRotAndTra[1]);
   g_pvp->m_settings.SaveValue(strKeyName, "RotAndTra2"s, m_d.m_aRotAndTra[2]);
   g_pvp->m_settings.SaveValue(strKeyName, "RotAndTra3"s, m_d.m_aRotAndTra[3]);
   g_pvp->m_settings.SaveValue(strKeyName, "RotAndTra4"s, m_d.m_aRotAndTra[4]);
   g_pvp->m_settings.SaveValue(strKeyName, "RotAndTra5"s, m_d.m_aRotAndTra[5]);
   g_pvp->m_settings.SaveValue(strKeyName, "RotAndTra6"s, m_d.m_aRotAndTra[6]);
   g_pvp->m_settings.SaveValue(strKeyName, "RotAndTra7"s, m_d.m_aRotAndTra[7]);
   g_pvp->m_settings.SaveValue(strKeyName, "RotAndTra8"s, m_d.m_aRotAndTra[8]);

   g_pvp->m_settings.SaveValue(strKeyName, "Image"s, m_d.m_szImage);
   g_pvp->m_settings.SaveValue(strKeyName, "NormalMap"s, m_d.m_szNormalMap);
   g_pvp->m_settings.SaveValue(strKeyName, "HitEvent"s, m_d.m_hitEvent);
   g_pvp->m_settings.SaveValue(strKeyName, "HitThreshold"s, m_d.m_threshold);
   g_pvp->m_settings.SaveValue(strKeyName, "Elasticity"s, m_d.m_elasticity);
   g_pvp->m_settings.SaveValue(strKeyName, "ElasticityFalloff"s, m_d.m_elasticityFalloff);
   g_pvp->m_settings.SaveValue(strKeyName, "Friction"s, m_d.m_friction);
   g_pvp->m_settings.SaveValue(strKeyName, "Scatter"s, m_d.m_scatter);

   g_pvp->m_settings.SaveValue(strKeyName, "AddBlend"s, m_d.m_addBlend);
   g_pvp->m_settings.SaveValue(strKeyName, "DepthMask"s, m_d.m_useDepthMask);
   g_pvp->m_settings.SaveValue(strKeyName, "Opacity"s, m_d.m_alpha);
   g_pvp->m_settings.SaveValue(strKeyName, "Color"s, (int)m_d.m_color);

   g_pvp->m_settings.SaveValue(strKeyName, "EdgeFactorUI"s, m_d.m_edgeFactorUI);
   g_pvp->m_settings.SaveValue(strKeyName, "CollisionReductionFactor"s, m_d.m_collision_reductionFactor);

   g_pvp->m_settings.SaveValue(strKeyName, "Collidable"s, m_d.m_collidable);
   g_pvp->m_settings.SaveValue(strKeyName, "IsToy"s, m_d.m_toy);
   g_pvp->m_settings.SaveValue(strKeyName, "DisableLighting"s, m_d.m_disableLightingTop);
   g_pvp->m_settings.SaveValue(strKeyName, "DisableLightingBelow"s, m_d.m_disableLightingBelow);
   g_pvp->m_settings.SaveValue(strKeyName, "ReflectionEnabled"s, m_d.m_reflectionEnabled);
   g_pvp->m_settings.SaveValue(strKeyName, "BackfacesEnabled"s, m_d.m_backfacesEnabled);
   g_pvp->m_settings.SaveValue(strKeyName, "DisplayTexture"s, m_d.m_displayTexture);
   g_pvp->m_settings.SaveValue(strKeyName, "ObjectSpaceNormalMap"s, m_d.m_objectSpaceNormalMap);

#undef strKeyName
}

void Primitive::GetTimers(vector<HitTimer*> &pvht)
{
   IEditable::BeginPlay();
}

void Primitive::GetHitShapes(vector<HitObject*> &pvho)
{
   char name[sizeof(m_wzName)/sizeof(m_wzName[0])];
   WideCharToMultiByteNull(CP_ACP, 0, m_wzName, -1, name, sizeof(name), nullptr, nullptr);
   m_d.m_useAsPlayfield = IsPlayfield();

   //

   if (m_d.m_toy)
      return;

   RecalculateMatrices();
   TransformVertices(); //!! could also only do this for the optional reduced variant!

   //

   const unsigned int reduced_vertices = max((unsigned int)pow((double)m_vertices.size(), clamp(1.f - m_d.m_collision_reductionFactor, 0.f, 1.f)*0.25f + 0.75f), 420u); //!! 420 = magic

//
// license:GPLv3+
// Ported at: VisualPinball.Engine/VPT/Primitive/PrimitiveHitGenerator.cs
//

   if (reduced_vertices < m_vertices.size())
   {
      vector<ProgMesh::float3> prog_vertices(m_vertices.size());
      for (size_t i = 0; i < m_vertices.size(); ++i) //!! opt. use original data directly!
      {
         prog_vertices[i].x = m_vertices[i].x;
         prog_vertices[i].y = m_vertices[i].y;
         prog_vertices[i].z = m_vertices[i].z;
      }
      vector<ProgMesh::tridata> prog_indices(m_mesh.NumIndices() / 3);
      {
      size_t i2 = 0;
      for (size_t i = 0; i < m_mesh.NumIndices(); i += 3)
      {
         ProgMesh::tridata t;
         t.v[0] = m_mesh.m_indices[i];
         t.v[1] = m_mesh.m_indices[i + 1];
         t.v[2] = m_mesh.m_indices[i + 2];
         if (t.v[0] != t.v[1] && t.v[1] != t.v[2] && t.v[2] != t.v[0])
            prog_indices[i2++] = t;
      }
      if (i2 < prog_indices.size())
         prog_indices.resize(i2);
      }
      vector<unsigned int> prog_map;
      vector<unsigned int> prog_perm;
      ProgMesh::ProgressiveMesh(prog_vertices, prog_indices, prog_map, prog_perm);
      ProgMesh::PermuteVertices(prog_perm, prog_vertices, prog_indices);
      prog_perm.clear();

      vector<ProgMesh::tridata> prog_new_indices;
      ProgMesh::ReMapIndices(reduced_vertices, prog_indices, prog_new_indices, prog_map);
      prog_indices.clear();
      prog_map.clear();

      //

      robin_hood::unordered_set<robin_hood::pair<unsigned, unsigned>> addedEdges;

      // add collision triangles and edges
      for (size_t i = 0; i < prog_new_indices.size(); ++i)
      {
         const unsigned int i0 = prog_new_indices[i].v[0];
         const unsigned int i1 = prog_new_indices[i].v[1];
         const unsigned int i2 = prog_new_indices[i].v[2];

         Vertex3Ds rgv3D[3];
         // NB: HitTriangle wants CCW vertices, but for rendering we have them in CW order
         rgv3D[0].x = prog_vertices[i0].x; rgv3D[0].y = prog_vertices[i0].y; rgv3D[0].z = prog_vertices[i0].z;
         rgv3D[1].x = prog_vertices[i2].x; rgv3D[1].y = prog_vertices[i2].y; rgv3D[1].z = prog_vertices[i2].z;
         rgv3D[2].x = prog_vertices[i1].x; rgv3D[2].y = prog_vertices[i1].y; rgv3D[2].z = prog_vertices[i1].z;
         SetupHitObject(pvho, new HitTriangle(rgv3D));

         AddHitEdge(pvho, addedEdges, i0, i1, rgv3D[0], rgv3D[2]);
         AddHitEdge(pvho, addedEdges, i1, i2, rgv3D[2], rgv3D[1]);
         AddHitEdge(pvho, addedEdges, i2, i0, rgv3D[1], rgv3D[0]);
      }

      prog_new_indices.clear();

      // add collision vertices
      for (size_t i = 0; i < prog_vertices.size(); ++i)
         SetupHitObject(pvho, new HitPoint(prog_vertices[i].x, prog_vertices[i].y, prog_vertices[i].z));
   }

//
// end of license:GPLv3+, back to 'old MAME'-like
//

   else
   {
      robin_hood::unordered_set<robin_hood::pair<unsigned, unsigned>> addedEdges;

      // add collision triangles and edges
      for (size_t i = 0; i < m_mesh.NumIndices(); i += 3)
      {
         const unsigned int i0 = m_mesh.m_indices[i];
         const unsigned int i1 = m_mesh.m_indices[i + 1];
         const unsigned int i2 = m_mesh.m_indices[i + 2];

         Vertex3Ds rgv3D[3];
         // NB: HitTriangle wants CCW vertices, but for rendering we have them in CW order
         rgv3D[0] = m_vertices[i0];
         rgv3D[1] = m_vertices[i2];
         rgv3D[2] = m_vertices[i1];
         SetupHitObject(pvho, new HitTriangle(rgv3D));

         AddHitEdge(pvho, addedEdges, i0, i1, rgv3D[0], rgv3D[2]);
         AddHitEdge(pvho, addedEdges, i1, i2, rgv3D[2], rgv3D[1]);
         AddHitEdge(pvho, addedEdges, i2, i0, rgv3D[1], rgv3D[0]);
      }

      // add collision vertices
      for (size_t i = 0; i < m_mesh.NumVertices(); ++i)
         SetupHitObject(pvho, new HitPoint(m_vertices[i]));
   }
}

void Primitive::GetHitShapesDebug(vector<HitObject*> &pvho)
{
}

//
// license:GPLv3+
// Ported at: VisualPinball.Engine/Math/EdgeSet.cs
//

void Primitive::AddHitEdge(vector<HitObject*> &pvho, robin_hood::unordered_set< robin_hood::pair<unsigned, unsigned> >& addedEdges, const unsigned i, const unsigned j, const Vertex3Ds &vi, const Vertex3Ds &vj)
{
   // create pair uniquely identifying the edge (i,j)
   const robin_hood::pair<unsigned, unsigned> p(std::min(i, j), std::max(i, j));

   if (addedEdges.insert(p).second) // edge not yet added?
      SetupHitObject(pvho, new HitLine3D(vi, vj));
}

//
// end of license:GPLv3+, back to 'old MAME'-like
//

void Primitive::SetupHitObject(vector<HitObject*> &pvho, HitObject * obj)
{
   const Material * const mat = m_ptable->GetMaterial(m_d.m_szPhysicsMaterial);
   if (m_d.m_useAsPlayfield)
   {
      obj->SetFriction(m_ptable->m_overridePhysics ? m_ptable->m_fOverrideContactFriction : m_ptable->m_friction);
      obj->m_elasticity = m_ptable->m_overridePhysics ? m_ptable->m_fOverrideElasticity : m_ptable->m_elasticity;
      obj->m_elasticityFalloff = m_ptable->m_overridePhysics ? m_ptable->m_fOverrideElasticityFalloff : m_ptable->m_elasticityFalloff;
      obj->m_scatter = ANGTORAD(m_ptable->m_overridePhysics ? m_ptable->m_fOverrideScatterAngle : m_ptable->m_scatter);
   }
   else if (mat != nullptr && !m_d.m_overwritePhysics)
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

   obj->m_enabled = m_d.m_collidable;
   obj->m_threshold = m_d.m_threshold;
   obj->m_ObjType = ePrimitive;
   obj->m_obj = (IFireEvents *)this;
   obj->m_e = 1;
   obj->m_fe = m_d.m_hitEvent;

   pvho.push_back(obj);
   m_vhoCollidable.push_back(obj); // remember hit components of primitive
}

void Primitive::EndPlay()
{
   m_d.m_skipRendering = false;
   m_d.m_groupdRendering = false;
   m_vhoCollidable.clear();
   IEditable::EndPlay();
}

//////////////////////////////
// Calculation
//////////////////////////////

//
// license:GPLv3+
// Ported at: VisualPinball.Engine/VPT/Primitive/PrimitiveMeshGenerator.cs
//

void Primitive::RecalculateMatrices()
{
   // scale matrix
   Matrix3D Smatrix;
   Smatrix.SetScaling(m_d.m_vSize.x, m_d.m_vSize.y, m_d.m_vSize.z);

   // translation matrix
   Matrix3D Tmatrix;
   Tmatrix.SetTranslation(m_d.m_vPosition.x, m_d.m_vPosition.y, m_d.m_vPosition.z);

   // translation + rotation matrix
   Matrix3D RTmatrix;
   RTmatrix.SetTranslation(m_d.m_aRotAndTra[3], m_d.m_aRotAndTra[4], m_d.m_aRotAndTra[5]);

   Matrix3D tempMatrix;
   tempMatrix.SetRotateZ(ANGTORAD(m_d.m_aRotAndTra[2]));
   tempMatrix.Multiply(RTmatrix, RTmatrix);
   tempMatrix.SetRotateY(ANGTORAD(m_d.m_aRotAndTra[1]));
   tempMatrix.Multiply(RTmatrix, RTmatrix);
   tempMatrix.SetRotateX(ANGTORAD(m_d.m_aRotAndTra[0]));
   tempMatrix.Multiply(RTmatrix, RTmatrix);

   tempMatrix.SetRotateZ(ANGTORAD(m_d.m_aRotAndTra[8]));
   tempMatrix.Multiply(RTmatrix, RTmatrix);
   tempMatrix.SetRotateY(ANGTORAD(m_d.m_aRotAndTra[7]));
   tempMatrix.Multiply(RTmatrix, RTmatrix);
   tempMatrix.SetRotateX(ANGTORAD(m_d.m_aRotAndTra[6]));
   tempMatrix.Multiply(RTmatrix, RTmatrix);

   m_fullMatrix = Smatrix;
   RTmatrix.Multiply(m_fullMatrix, m_fullMatrix);
   Tmatrix.Multiply(m_fullMatrix, m_fullMatrix);        // m_fullMatrix = RTmatrix * Tmatrix
}

//
// end of license:GPLv3+, back to 'old MAME'-like
//

// recalculate vertices for editor display
void Primitive::TransformVertices()
{
   m_vertices.resize(m_mesh.NumVertices());
   m_normals.resize(m_mesh.NumVertices());

   for (size_t i = 0; i < m_mesh.NumVertices(); i++)
   {
      m_fullMatrix.MultiplyVector(m_mesh.m_vertices[i], m_vertices[i]);
      Vertex3Ds n;
      m_fullMatrix.MultiplyVectorNoTranslateNormal(m_mesh.m_vertices[i], n);
      n.Normalize();
      m_normals[i] = n.z;
   }
}

//////////////////////////////
// Rendering
//////////////////////////////

// 2D

void Primitive::UIRenderPass1(Sur * const psur)
{
}

void Primitive::UIRenderPass2(Sur * const psur)
{
   psur->SetLineColor(RGB(0, 0, 0), false, 1);
   psur->SetObject(this);
   if (!m_d.m_displayTexture)
   {
      if ((m_d.m_edgeFactorUI <= 0.0f) || (m_d.m_edgeFactorUI >= 1.0f) || !m_d.m_use3DMesh)
      {
         if (!m_d.m_use3DMesh || (m_d.m_edgeFactorUI >= 1.0f) || (m_mesh.NumVertices() <= 100)) // small mesh: draw all triangles
         {
            for (size_t i = 0; i < m_mesh.NumIndices(); i += 3)
            {
               const Vertex3Ds * const A = &m_vertices[m_mesh.m_indices[i]];
               const Vertex3Ds * const B = &m_vertices[m_mesh.m_indices[i + 1]];
               const Vertex3Ds * const C = &m_vertices[m_mesh.m_indices[i + 2]];
               psur->Line(A->x, A->y, B->x, B->y);
               psur->Line(B->x, B->y, C->x, C->y);
               psur->Line(C->x, C->y, A->x, A->y);
            }
         }
         else // large mesh: draw a simplified mesh for performance reasons, does not approximate the shape well
         {
            if (m_mesh.NumIndices() > 0)
            {
               const size_t numPts = m_mesh.NumIndices() / 3 + 1;
               vector<Vertex2D> drawVertices(numPts);

               const Vertex3Ds& A = m_vertices[m_mesh.m_indices[0]];
               drawVertices[0] = Vertex2D(A.x, A.y);

               unsigned int o = 1;
               for (size_t i = 0; i < m_mesh.NumIndices(); i += 3, ++o)
               {
                  const Vertex3Ds& B = m_vertices[m_mesh.m_indices[i + 1]];
                  drawVertices[o] = Vertex2D(B.x, B.y);
               }

               psur->Polyline(&drawVertices[0], (int)drawVertices.size());
            }
         }
      }
      else
      {
         vector<Vertex2D> drawVertices;
         for (size_t i = 0; i < m_mesh.NumIndices(); i += 3)
         {
            const Vertex3Ds * const A = &m_vertices[m_mesh.m_indices[i]];
            const Vertex3Ds * const B = &m_vertices[m_mesh.m_indices[i + 1]];
            const Vertex3Ds * const C = &m_vertices[m_mesh.m_indices[i + 2]];
            const float An = m_normals[m_mesh.m_indices[i]];
            const float Bn = m_normals[m_mesh.m_indices[i + 1]];
            const float Cn = m_normals[m_mesh.m_indices[i + 2]];
            if (fabsf(An + Bn) < m_d.m_edgeFactorUI)
            {
               drawVertices.emplace_back(Vertex2D(A->x, A->y));
               drawVertices.emplace_back(Vertex2D(B->x, B->y));
            }
            if (fabsf(Bn + Cn) < m_d.m_edgeFactorUI)
            {
               drawVertices.emplace_back(Vertex2D(B->x, B->y));
               drawVertices.emplace_back(Vertex2D(C->x, C->y));
            }
            if (fabsf(Cn + An) < m_d.m_edgeFactorUI)
            {
               drawVertices.emplace_back(Vertex2D(C->x, C->y));
               drawVertices.emplace_back(Vertex2D(A->x, A->y));
            }
         }

         if (!drawVertices.empty())
            psur->Lines(&drawVertices[0], (int)(drawVertices.size() / 2));
      }
   }

   // draw center marker
   psur->SetLineColor(RGB(128, 128, 128), false, 1);
   psur->Line(m_d.m_vPosition.x - 10.0f, m_d.m_vPosition.y, m_d.m_vPosition.x + 10.0f, m_d.m_vPosition.y);
   psur->Line(m_d.m_vPosition.x, m_d.m_vPosition.y - 10.0f, m_d.m_vPosition.x, m_d.m_vPosition.y + 10.0f);
   
   if (m_d.m_displayTexture)
   {
      Texture * const ppi = m_ptable->GetImage(m_d.m_szImage);
      if (ppi)
      {
         ppi->CreateGDIVersion();
         if (ppi->m_hbmGDIVersion)
         {
            vector<RenderVertex> vvertex;
            for (size_t i = 0; i < m_mesh.NumIndices(); i += 3)
            {
               const Vertex3Ds * const A = &m_vertices[m_mesh.m_indices[i]];
               const Vertex3Ds * const B = &m_vertices[m_mesh.m_indices[i + 1]];
               const Vertex3Ds * const C = &m_vertices[m_mesh.m_indices[i + 2]];
               RenderVertex rvA;
               RenderVertex rvB;
               RenderVertex rvC;
               rvA.x = A->x;
               rvA.y = A->y;
               rvB.x = B->x;
               rvB.y = B->y;
               rvC.x = C->x;
               rvC.y = C->y;
               vvertex.push_back(rvC);
               vvertex.push_back(rvB);
               vvertex.push_back(rvA);
            }
            psur->PolygonImage(vvertex, ppi->m_hbmGDIVersion, m_ptable->m_left, m_ptable->m_top, m_ptable->m_right, m_ptable->m_bottom, ppi->m_width, ppi->m_height);

            vvertex.clear();
         }
      }
   }

}

void Primitive::RenderBlueprint(Sur *psur, const bool solid)
{
   psur->SetFillColor(solid ? BLUEPRINT_SOLID_COLOR : -1);
   psur->SetLineColor(RGB(0, 0, 0), false, 1);
   psur->SetObject(this);

   if (solid && m_d.m_use3DMesh)
   {
       for (size_t i = 0; i < m_mesh.NumIndices(); i += 3)
       {
           const Vertex3Ds * const A = &m_vertices[m_mesh.m_indices[i]];
           const Vertex3Ds * const B = &m_vertices[m_mesh.m_indices[i + 1]];
           const Vertex3Ds * const C = &m_vertices[m_mesh.m_indices[i + 2]];

           Vertex2D rv[3];
           rv[0].x = C->x; rv[0].y = C->y;
           rv[1].x = B->x; rv[1].y = B->y;
           rv[2].x = A->x; rv[2].y = A->y;
           psur->Polygon(rv, 3);
       }
       return;
   }
   if ((m_d.m_edgeFactorUI <= 0.0f) || (m_d.m_edgeFactorUI >= 1.0f) || !m_d.m_use3DMesh)
   {
      if (!m_d.m_use3DMesh || (m_d.m_edgeFactorUI >= 1.0f) || (m_mesh.NumVertices() <= 100)) // small mesh: draw all triangles
      {
         for (size_t i = 0; i < m_mesh.NumIndices(); i += 3)
         {
            const Vertex3Ds * const A = &m_vertices[m_mesh.m_indices[i]];
            const Vertex3Ds * const B = &m_vertices[m_mesh.m_indices[i + 1]];
            const Vertex3Ds * const C = &m_vertices[m_mesh.m_indices[i + 2]];
            psur->Line(A->x, A->y, B->x, B->y);
            psur->Line(B->x, B->y, C->x, C->y);
            psur->Line(C->x, C->y, A->x, A->y);
         }
      }
      else // large mesh: draw a simplified mesh for performance reasons, does not approximate the shape well
      {
         if (m_mesh.NumIndices() > 0)
         {
            const size_t numPts = m_mesh.NumIndices() / 3 + 1;
            vector<Vertex2D> drawVertices(numPts);

            const Vertex3Ds& A = m_vertices[m_mesh.m_indices[0]];
            drawVertices[0] = Vertex2D(A.x, A.y);

            unsigned int o = 1;
            for (size_t i = 0; i < m_mesh.NumIndices(); i += 3, ++o)
            {
               const Vertex3Ds& B = m_vertices[m_mesh.m_indices[i + 1]];
               drawVertices[o] = Vertex2D(B.x, B.y);
            }

            psur->Polyline(&drawVertices[0], (int)drawVertices.size());
         }
      }
   }
   else
   {
      vector<Vertex2D> drawVertices;
      for (size_t i = 0; i < m_mesh.NumIndices(); i += 3)
      {
         const Vertex3Ds * const A = &m_vertices[m_mesh.m_indices[i]];
         const Vertex3Ds * const B = &m_vertices[m_mesh.m_indices[i + 1]];
         const Vertex3Ds * const C = &m_vertices[m_mesh.m_indices[i + 2]];
         const float An = m_normals[m_mesh.m_indices[i]];
         const float Bn = m_normals[m_mesh.m_indices[i + 1]];
         const float Cn = m_normals[m_mesh.m_indices[i + 2]];
         if (fabsf(An + Bn) < m_d.m_edgeFactorUI)
         {
            drawVertices.emplace_back(Vertex2D(A->x, A->y));
            drawVertices.emplace_back(Vertex2D(B->x, B->y));
         }
         if (fabsf(Bn + Cn) < m_d.m_edgeFactorUI)
         {
            drawVertices.emplace_back(Vertex2D(B->x, B->y));
            drawVertices.emplace_back(Vertex2D(C->x, C->y));
         }
         if (fabsf(Cn + An) < m_d.m_edgeFactorUI)
         {
            drawVertices.emplace_back(Vertex2D(C->x, C->y));
            drawVertices.emplace_back(Vertex2D(A->x, A->y));
         }
      }

      if (!drawVertices.empty())
         psur->Lines(&drawVertices[0], (int)(drawVertices.size() / 2));
   }
}

void Primitive::GetBoundingVertices(vector<Vertex3Ds> &pvvertex3D, const bool isLegacy)
{
   // VPX before 10.8 computed the viewer position based on a partial bounding volume that would not include primitives
   if (isLegacy || !m_d.m_visible)
      return;
   RecalculateMatrices();
   if (m_d.m_use3DMesh)
   {
      m_mesh.UpdateBounds();
      Vertex3Ds minBound(m_mesh.m_minAABound);
      Vertex3Ds maxBound(m_mesh.m_maxAABound);
      if (minBound.x != FLT_MAX)
      {
         m_fullMatrix.TransformVec3(minBound);
         m_fullMatrix.TransformVec3(maxBound);
         pvvertex3D.push_back(Vertex3Ds(minBound.x, minBound.y, minBound.z));
         pvvertex3D.push_back(Vertex3Ds(minBound.x, minBound.y, maxBound.z));
         pvvertex3D.push_back(Vertex3Ds(minBound.x, maxBound.y, minBound.z));
         pvvertex3D.push_back(Vertex3Ds(minBound.x, maxBound.y, maxBound.z));
         pvvertex3D.push_back(Vertex3Ds(maxBound.x, minBound.y, minBound.z));
         pvvertex3D.push_back(Vertex3Ds(maxBound.x, minBound.y, maxBound.z));
         pvvertex3D.push_back(Vertex3Ds(maxBound.x, maxBound.y, minBound.z));
         pvvertex3D.push_back(Vertex3Ds(maxBound.x, maxBound.y, maxBound.z));
      }
   }
}

//
// license:GPLv3+
// Ported at: VisualPinball.Engine/VPT/Primitive/PrimitiveMeshGenerator.cs
//

void Primitive::CalculateBuiltinOriginal()
{
    if (m_d.m_use3DMesh)
        return;

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
   middle->nx = 0.0f;
   middle->ny = 0.0f;
   middle->nz = 1.0f;
   middle = &m_mesh.m_vertices[m_d.m_Sides + 1]; // middle point bottom
   middle->x = 0.0f;
   middle->y = 0.0f;
   middle->z = -0.5f;
   middle->nx = 0.0f;
   middle->ny = 0.0f;
   middle->nz = -1.0f;
   for (int i = 0; i < m_d.m_Sides; ++i)
   {
      // calculate top
      Vertex3D_NoTex2 * const topVert = &m_mesh.m_vertices[i + 1]; // top point at side
      const float currentAngle = addAngle*(float)i + offsAngle;
      topVert->x = sinf(currentAngle)*outerRadius;
      topVert->y = cosf(currentAngle)*outerRadius;
      topVert->z = 0.5f;
      topVert->nx = 0.0f;
      topVert->ny = 0.0f;
      topVert->nz = 1.0f;

      // calculate bottom
      Vertex3D_NoTex2 * const bottomVert = &m_mesh.m_vertices[i + 1 + m_d.m_Sides + 1]; // bottompoint at side
      bottomVert->x = topVert->x;
      bottomVert->y = topVert->y;
      bottomVert->z = -0.5f;
      bottomVert->nx = 0.0f;
      bottomVert->ny = 0.0f;
      bottomVert->nz = -1.0f;

      // calculate sides
      Vertex3D_NoTex2 &sideTopVert = m_mesh.m_vertices[m_d.m_Sides * 2 + 2 + i];
      sideTopVert = *topVert;
      sideTopVert.nx = sinf(currentAngle);
      sideTopVert.ny = cosf(currentAngle);
      sideTopVert.nz = 0.0f;
      Vertex3D_NoTex2 &sideBottomVert = m_mesh.m_vertices[m_d.m_Sides * 3 + 2 + i];
      sideBottomVert = *bottomVert;
      sideBottomVert.nx = sideTopVert.nx;
      sideBottomVert.ny = sideTopVert.ny;
      sideBottomVert.nz = 0.0f;

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

      Vertex3D_NoTex2 * const bottomVert = &m_mesh.m_vertices[i + 1 + m_d.m_Sides + 1]; // bottom point at side
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

   // 2 restore indices
   //   check if anti culling is enabled:
   if (m_d.m_drawTexturesInside)
   {
      m_mesh.m_indices.resize(m_d.m_Sides * 24);
      // yes: draw everything twice
      // restore indices
      for (int i = 0; i < m_d.m_Sides; i++)
      {
         const int tmp = (i == m_d.m_Sides - 1) ? 1 : (i + 2); // wrapping around
         // top
         m_mesh.m_indices[i * 6] = 0;
         m_mesh.m_indices[i * 6 + 1] = i + 1;
         m_mesh.m_indices[i * 6 + 2] = tmp;
         m_mesh.m_indices[i * 6 + 3] = 0;
         m_mesh.m_indices[i * 6 + 4] = tmp;
         m_mesh.m_indices[i * 6 + 5] = i + 1;

         const int tmp2 = tmp + 1;
         // bottom
         m_mesh.m_indices[6 * (i + m_d.m_Sides)] = m_d.m_Sides + 1;
         m_mesh.m_indices[6 * (i + m_d.m_Sides) + 1] = m_d.m_Sides + tmp2;
         m_mesh.m_indices[6 * (i + m_d.m_Sides) + 2] = m_d.m_Sides + 2 + i;
         m_mesh.m_indices[6 * (i + m_d.m_Sides) + 3] = m_d.m_Sides + 1;
         m_mesh.m_indices[6 * (i + m_d.m_Sides) + 4] = m_d.m_Sides + 2 + i;
         m_mesh.m_indices[6 * (i + m_d.m_Sides) + 5] = m_d.m_Sides + tmp2;

         // sides
         m_mesh.m_indices[12 * (i + m_d.m_Sides)] = m_d.m_Sides * 2 + tmp2;
         m_mesh.m_indices[12 * (i + m_d.m_Sides) + 1] = m_d.m_Sides * 2 + 2 + i;
         m_mesh.m_indices[12 * (i + m_d.m_Sides) + 2] = m_d.m_Sides * 3 + 2 + i;
         m_mesh.m_indices[12 * (i + m_d.m_Sides) + 3] = m_d.m_Sides * 2 + tmp2;
         m_mesh.m_indices[12 * (i + m_d.m_Sides) + 4] = m_d.m_Sides * 3 + 2 + i;
         m_mesh.m_indices[12 * (i + m_d.m_Sides) + 5] = m_d.m_Sides * 3 + tmp2;
         m_mesh.m_indices[12 * (i + m_d.m_Sides) + 6] = m_d.m_Sides * 2 + tmp2;
         m_mesh.m_indices[12 * (i + m_d.m_Sides) + 7] = m_d.m_Sides * 3 + 2 + i;
         m_mesh.m_indices[12 * (i + m_d.m_Sides) + 8] = m_d.m_Sides * 2 + 2 + i;
         m_mesh.m_indices[12 * (i + m_d.m_Sides) + 9] = m_d.m_Sides * 2 + tmp2;
         m_mesh.m_indices[12 * (i + m_d.m_Sides) + 10] = m_d.m_Sides * 3 + tmp2;
         m_mesh.m_indices[12 * (i + m_d.m_Sides) + 11] = m_d.m_Sides * 3 + 2 + i;
      }
   }
   else {
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
   }

   //SetNormal(&m_mesh.m_vertices[0], &m_mesh.m_indices[0], m_mesh.NumIndices()); // SetNormal only works for plane polygons
   //ComputeNormals(m_mesh.m_vertices, m_mesh.m_indices);
}

//
// end of license:GPLv3+, back to 'old MAME'-like
//

void Primitive::UpdateStatusBarInfo()
{
   CalculateBuiltinOriginal();
   RecalculateMatrices();
   TransformVertices();
   if (m_d.m_use3DMesh)
   {
       const string tbuf = "Vertices: " + std::to_string(m_mesh.NumVertices()) + " | Polygons: " + std::to_string(m_mesh.NumIndices());
       m_vpinball->SetStatusBarUnitInfo(tbuf, false);
   }
   else
       m_vpinball->SetStatusBarUnitInfo(string(), false);

}

void Primitive::ExportMesh(ObjLoader& loader)
{
   if (m_d.m_visible)
   {
      char name[sizeof(m_wzName)/sizeof(m_wzName[0])];
      WideCharToMultiByteNull(CP_ACP, 0, m_wzName, -1, name, sizeof(name), nullptr, nullptr);
      Vertex3D_NoTex2 *const buf = new Vertex3D_NoTex2[m_mesh.NumVertices()];
      RecalculateMatrices();
      for (size_t i = 0; i < m_mesh.NumVertices(); i++)
      {
         const Vertex3D_NoTex2 &v = m_mesh.m_vertices[i];
         Vertex3Ds vert(v.x, v.y, v.z);
         vert = m_fullMatrix.MultiplyVector(vert);
         buf[i].x = vert.x;
         buf[i].y = vert.y;
         buf[i].z = vert.z;

         vert = Vertex3Ds(v.nx, v.ny, v.nz);
         vert = m_fullMatrix.MultiplyVectorNoTranslate(vert);
         buf[i].nx = vert.x;
         buf[i].ny = vert.y;
         buf[i].nz = vert.z;
         buf[i].tu = v.tu;
         buf[i].tv = v.tv;
      }
      loader.WriteObjectName(name);
      loader.WriteVertexInfo(buf, (unsigned int)m_mesh.NumVertices());
      const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);
      loader.WriteMaterial(m_d.m_szMaterial, string(), mat);
      loader.UseTexture(m_d.m_szMaterial);
      loader.WriteFaceInfoLong(m_mesh.m_indices);
      loader.UpdateFaceOffset((unsigned int)m_mesh.NumVertices());
      delete[] buf;
   }
}

void Primitive::RenderSetup(RenderDevice *device)
{
   assert(m_rd == nullptr);
   m_rd = device;

   if (m_d.m_groupdRendering || m_d.m_skipRendering)
      return;

   //const char* const szT = MakeChar(m_wzName);
   //PLOGD_IF(m_d.m_staticRendering && m_d.m_disableLightingBelow != 1.0f && m_d.m_visible) << "Primitive '" << szT << "' is set as static rendering with lighting from below not disabled. The back lighting will not be performed.";
   //delete[] szT;

   m_lightmap = m_ptable->GetLight(m_d.m_szLightmap);

   m_currentFrame = -1.f;
   m_d.m_isBackGlassImage = IsBackglass();

   delete m_meshBuffer;
   VertexBuffer* vertexBuffer = new VertexBuffer(m_rd, (unsigned int)m_mesh.NumVertices(), nullptr, !(m_d.m_staticRendering || m_mesh.m_animationFrames.size() == 0));
   IndexBuffer* indexBuffer = new IndexBuffer(m_rd, m_mesh.m_indices);
   m_meshBuffer = new MeshBuffer(m_wzName, vertexBuffer, indexBuffer, true);

   // Compute and upload mesh to let a chance for renderdevice to share the buffers with other static objects
   RecalculateMatrices();
   m_mesh.UploadToVB(vertexBuffer, m_currentFrame);
   m_vertexBufferRegenerate = false;
}

void Primitive::RenderRelease()
{
   assert(m_rd != nullptr);
   delete m_meshBuffer;
   m_meshBuffer = nullptr;
   m_lightmap = nullptr;
   m_rd = nullptr;
}

void Primitive::Render(const unsigned int renderMask)
{
   assert(m_rd != nullptr);
   const bool isStaticOnly = renderMask & Player::STATIC_ONLY;
   const bool isDynamicOnly = renderMask & Player::DYNAMIC_ONLY;
   const bool isReflectionPass = renderMask & Player::REFLECTION_PASS;
   TRACE_FUNCTION();

   // Update playfield primitive settings from table settings
   SamplerFilter pinf = SF_UNDEFINED; // Use the default filtering of the sampler (trilinear or anisotropic, depending on user choice)
   if (m_d.m_useAsPlayfield)
   {
      m_d.m_szMaterial = g_pplayer->m_ptable->m_playfieldMaterial;
      m_d.m_szImage = g_pplayer->m_ptable->m_image;
      m_d.m_szReflectionProbe = PLAYFIELD_REFLECTION_RENDERPROBE_NAME;
      m_d.m_reflectionStrength = m_ptable->m_playfieldReflectionStrength;
      pinf = SF_ANISOTROPIC;
   }

   // Do not render ourself inside our reflection probe (no self reflection)
   RenderProbe * const reflection_probe = m_d.m_reflectionStrength <= 0 ? nullptr : m_ptable->GetRenderProbe(m_d.m_szReflectionProbe);
   if (reflection_probe != nullptr && reflection_probe->IsRendering())
      return;

   if (!m_d.m_visible || m_d.m_skipRendering)
      return;
   
   if (isReflectionPass && !m_d.m_reflectionEnabled)
      return;
   
   if (isStaticOnly && !m_d.m_staticRendering)
      return;
   
   // Don't render in LS when state off (visibility driven by light sequencer)
   if (m_lockedByLS && !m_inPlayState)
      return;
   
   // only render if we have dynamic reflections to render above the staticly prerendered primitive
   RenderTarget *const reflections = reflection_probe ? reflection_probe->Render(renderMask) : nullptr;
   if (isDynamicOnly && m_d.m_staticRendering && (reflections == nullptr))
      return; 

   // Update lightmap before checking anything that uses alpha
   float alpha = m_d.m_alpha;
   if (m_lightmap)
      alpha *= m_lightmap->m_currentIntensity / (m_lightmap->m_d.m_intensity * m_lightmap->m_d.m_intensity_scale);

   // don't render additive primitive if there is nothing to add
   if (m_d.m_addBlend && (m_d.m_color == 0 || alpha == 0.f))
      return;

   // Request probes before setting up state since this can trigger a renderprobe update which modifies the render state
   RenderProbe * const refraction_probe = m_ptable->GetRenderProbe(m_d.m_szRefractionProbe);
   RenderTarget *const refractions = refraction_probe ? refraction_probe->Render(renderMask) : nullptr;

   m_rd->ResetRenderState();
   
   if (m_d.m_groupdRendering)
      m_fullMatrix.SetIdentity();
   else
   {
      RecalculateMatrices();
      if (m_vertexBufferRegenerate)
      {
         m_mesh.UploadToVB(m_meshBuffer->m_vb, m_currentFrame);
         m_vertexBufferRegenerate = false;
      }
   }

   const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);

   const bool depthMask = m_d.m_useDepthMask && !m_d.m_addBlend;

   RenderState::RenderStateValue cullMode = m_rd->GetRenderState().GetRenderState(RenderState::CULLMODE);
   RenderState::RenderStateValue reversedCullMode = cullMode == RenderState::CULL_CCW ? RenderState::CULL_CW : RenderState::CULL_CCW;
   m_rd->SetRenderState(RenderState::CULLMODE, depthMask ? ((m_d.m_backfacesEnabled && mat->m_bOpacityActive) ? reversedCullMode : cullMode) : RenderState::CULL_NONE);

   // Force disable light from below for objects marked as static since there is no light from below during pre-render pass (to get the same result in dynamic mode & static mode)
   m_rd->basicShader->SetVector(SHADER_fDisableLighting_top_below, m_d.m_disableLightingTop, m_d.m_staticRendering ? 1.0f : m_d.m_disableLightingBelow, 0.f, 0.f);

   // Select textures, replacing backglass image by capture if it is available
   Texture * const nMap = m_ptable->GetImage(m_d.m_szNormalMap);
   BaseTexture *pin = nullptr;
   float pinAlphaTest = -1.f;
   if (g_pplayer->m_texPUP && m_d.m_isBackGlassImage)
   {
      pin = g_pplayer->m_texPUP;
      m_rd->basicShader->SetAlphaTestValue(0.f);
   }
   else
   {
      Texture * const img = m_ptable->GetImage(m_d.m_szImage);
      if (img != nullptr)
      {
         pin = img->m_pdsBuffer;
         pinAlphaTest = img->m_alphaTestValue;
         m_rd->basicShader->SetAlphaTestValue(img->m_alphaTestValue);
      }
      else
      {
         pin = nullptr;
      }
   }

   // accommodate models with UV coords outside of [0,1] by using Repeat address mode
   if (pin && nMap)
   {
      m_rd->basicShader->SetTexture(SHADER_tex_base_color, pin, pinf, SA_REPEAT, SA_REPEAT);
      m_rd->basicShader->SetTexture(SHADER_tex_base_normalmap, nMap, SF_UNDEFINED, SA_REPEAT, SA_REPEAT, true);
      m_rd->basicShader->SetBool(SHADER_objectSpaceNormalMap, m_d.m_objectSpaceNormalMap);
      m_rd->basicShader->SetMaterial(mat, !pin->IsOpaque() || alpha != 100.f);
   }
   else if (pin)
   {
      m_rd->basicShader->SetTexture(SHADER_tex_base_color, pin, pinf, SA_REPEAT, SA_REPEAT);
      m_rd->basicShader->SetMaterial(mat, !pin->IsOpaque() || alpha != 100.f);
   }
   else
   {
      m_rd->basicShader->SetMaterial(mat, alpha != 100.f);
   }

   // set transform
   g_pplayer->UpdateBasicShaderMatrix(m_fullMatrix);

   // Check if this primitive is used as a lightmap and should be convoluted with the light shadows
   bool lightmap = m_lightmap != nullptr && m_lightmap->m_d.m_shadows == ShadowMode::RAYTRACED_BALL_SHADOWS;
   if (lightmap)
      m_rd->basicShader->SetVector(SHADER_lightCenter_doShadow, m_lightmap->m_d.m_vCenter.x, m_lightmap->m_d.m_vCenter.y, m_lightmap->GetCurrentHeight(), 1.0f);

   if (m_d.m_addBlend)
   {
      // Additive blending is a special unlit mode with depth mask disabled
      m_rd->EnableAlphaBlend(true);
      m_rd->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
      const vec4 color = convertColor(m_d.m_color, alpha * (float)(1.0 / 100.0));
      m_rd->basicShader->SetVector(SHADER_staticColor_Alpha, color.x * color.w, color.y * color.w, color.z * color.w, color.w);
      m_rd->basicShader->SetTechnique(lightmap ? (pin ? SHADER_TECHNIQUE_unshaded_with_texture_shadow : SHADER_TECHNIQUE_unshaded_without_texture_shadow)
                                               : (pin ? SHADER_TECHNIQUE_unshaded_with_texture : SHADER_TECHNIQUE_unshaded_without_texture));
      m_rd->DrawMesh(m_rd->basicShader, true, m_d.m_vPosition, m_d.m_depthBias, m_meshBuffer, RenderDevice::TRIANGLELIST, 0, m_d.m_groupdRendering ? m_numGroupIndices : (DWORD)m_mesh.NumIndices());
   }
   else
   {
      // Default lit primitive rendering
      m_rd->SetRenderState(RenderState::ZWRITEENABLE, depthMask ? RenderState::RS_TRUE : RenderState::RS_FALSE);
      const vec4 color = convertColor(m_d.m_color, alpha * (float)(1.0 / 100.0));
      m_rd->basicShader->SetVector(SHADER_staticColor_Alpha, &color);
      m_rd->basicShader->SetTechniqueMaterial(pin ? SHADER_TECHNIQUE_basic_with_texture : SHADER_TECHNIQUE_basic_without_texture, 
         mat, pin ? pinAlphaTest >= 0.f && !pin->IsOpaque() : false, nMap, reflections, refractions);
      bool is_reflection_only_pass = false;

      // Handle render probes
      if (reflections || refractions)
      {
         m_mesh.UpdateBounds();
         float xMin = 1.f, yMin = 1.f, xMax = -1.f, yMax = -1.f;
         const int nEyes = m_rd->m_stereo3D != STEREO_OFF ? 2 : 1;
         for (int eye = 0; eye < nEyes; eye++)
         {
            const Matrix3D & mvp = g_pplayer->m_pin3d.GetMVP().GetModelViewProj(eye);
            for (int i = 0; i < 8; i++)
            {
               Vertex3Ds p;
               p.x = (i & 1) ? m_mesh.m_minAABound.x : m_mesh.m_maxAABound.x;
               p.y = (i & 2) ? m_mesh.m_minAABound.y : m_mesh.m_maxAABound.y;
               p.z = (i & 4) ? m_mesh.m_minAABound.z : m_mesh.m_maxAABound.z;
                     float xp = mvp._11 * p.x + mvp._21 * p.y + mvp._31 * p.z + mvp._41;
                     float yp = mvp._12 * p.x + mvp._22 * p.y + mvp._32 * p.z + mvp._42;
               const float wp = mvp._14 * p.x + mvp._24 * p.y + mvp._34 * p.z + mvp._44;
               if (wp > 0.f)
               {
                  xp /= wp;
                  yp /= wp;
                  xMin = min(xMin, xp);
                  xMax = max(xMax, xp);
                  yMin = min(yMin, yp);
                  yMax = max(yMax, yp);
               }
               else
               {
                  xMin = yMin = -1.f;
                  xMax = yMax = 1.f;
                  break;
               }
            }
         }
         if (reflections)
         {
            reflection_probe->ExtendAreaOfInterest(xMin, xMax, yMin, yMax);
            m_rd->AddRenderTargetDependency(reflections);
            vec3 plane_normal;
            reflection_probe->GetReflectionPlaneNormal(plane_normal);
            Matrix3D matWorldViewInverseTranspose = g_pplayer->m_pin3d.GetMVP().GetModelViewInverseTranspose();
            matWorldViewInverseTranspose.MultiplyVectorNoTranslate(plane_normal, plane_normal);
            Vertex3Ds n(plane_normal.x, plane_normal.y, plane_normal.z);
            n.Normalize();
            m_rd->basicShader->SetVector(SHADER_mirrorNormal_factor, n.x, n.y, n.z, m_d.m_reflectionStrength);
            m_rd->basicShader->SetTexture(SHADER_tex_reflection, reflections->GetColorSampler());
            is_reflection_only_pass = m_d.m_staticRendering && isDynamicOnly;
            if (!is_reflection_only_pass && !m_rd->GetRenderState().IsOpaque())
            { // Primitive uses alpha transparency => render in 2 passes, one for the texture with alpha blending, one for the reflections which can happen above a transparent part (like for a glass or insert plastic)
               m_rd->basicShader->SetTechniqueMaterial(pin ? SHADER_TECHNIQUE_basic_with_texture : SHADER_TECHNIQUE_basic_without_texture, 
                  mat, pin ? pinAlphaTest >= 0.f && !pin->IsOpaque() : false, nMap, false, false);
               m_rd->DrawMesh(m_rd->basicShader, mat->m_bOpacityActive && !m_d.m_staticRendering, m_d.m_vPosition, m_d.m_depthBias, 
                  m_meshBuffer, RenderDevice::TRIANGLELIST, 0, m_d.m_groupdRendering ? m_numGroupIndices : (DWORD)m_mesh.NumIndices());
               is_reflection_only_pass = true;
            }
            if (is_reflection_only_pass)
            { // If the primitive is already rendered (dynamic pass after a static prepass, or multipass rendering due to alpha blending) => only render additive reflections
               m_rd->EnableAlphaBlend(true);
               m_rd->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
               m_rd->basicShader->SetTechnique(SHADER_TECHNIQUE_basic_reflection_only);
            }
         }
         if (refractions)
         {
            refraction_probe->ExtendAreaOfInterest(xMin, xMax, yMin, yMax);
            const vec4 color = convertColor(mat->m_cRefractionTint, m_d.m_refractionThickness);
            m_rd->basicShader->SetVector(SHADER_refractionTint_thickness, &color);
            m_rd->basicShader->SetTexture(SHADER_tex_refraction, refractions->GetColorSampler());
            m_rd->basicShader->SetTexture(SHADER_tex_probe_depth, refractions->GetDepthSampler());
            m_rd->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
            if (!is_reflection_only_pass)
               m_rd->AddRenderTargetDependencyOnNextRenderCommand(refractions); // Add a renderpass dependency on the render command (instead of in the renderframe) for the pass to be sorted with the command
         }
      }

      // draw the mesh (back of it if backface enabled)
      m_rd->DrawMesh(m_rd->basicShader, 
            is_reflection_only_pass // The reflection pass is an additive (so transparent) pass to be drawn after the opaque one
         || refractions // Refractions must be rendered back to front since they rely on what is behind
         || (mat->m_bOpacityActive && !m_d.m_staticRendering /* && !m_rd->GetRenderState().IsOpaque() */), // We can not use the real render state opaque state since Blood Machine and other tables use depth masks
            m_d.m_vPosition, m_d.m_depthBias, m_meshBuffer, RenderDevice::TRIANGLELIST, 0, m_d.m_groupdRendering ? m_numGroupIndices : (DWORD)m_mesh.NumIndices());
   }

   // Draw the front of the primitive if backface enabled
   if (depthMask && m_d.m_backfacesEnabled && mat->m_bOpacityActive)
   {
      m_rd->SetRenderState(RenderState::CULLMODE, cullMode);
      m_rd->DrawMesh(m_rd->basicShader, mat->m_bOpacityActive, m_d.m_vPosition, m_d.m_depthBias, 
         m_meshBuffer, RenderDevice::TRIANGLELIST, 0, m_d.m_groupdRendering ? m_numGroupIndices : (DWORD)m_mesh.NumIndices());
   }

   // Restore state
   g_pplayer->UpdateBasicShaderMatrix();
   m_rd->basicShader->SetVector(SHADER_mirrorNormal_factor, 0.f, 0.f, 0.f, 0.f);
   m_rd->basicShader->SetVector(SHADER_lightCenter_doShadow, 0.0f, 0.0f, 0.0f, 0.0f);
   m_rd->basicShader->SetVector(SHADER_staticColor_Alpha, 1.0f, 1.0f, 1.0f, 1.0f);
   m_rd->basicShader->SetVector(SHADER_fDisableLighting_top_below, 0.f, 0.f, 0.f, 0.f);
}

void Primitive::UpdateAnimation(const float diff_time_msec)
{
   if (m_currentFrame != -1.0f && m_doAnimation)
   {
      const float previousFrame = m_currentFrame;
      m_currentFrame += m_speed * (diff_time_msec * (float)(60. / 1000.));
      if (m_currentFrame >= (float)m_mesh.m_animationFrames.size())
      {
          if (m_endless)
          {
             m_currentFrame = 0.0f;
          }
          else
          {
             m_currentFrame = (float)(m_mesh.m_animationFrames.size() - 1);
             m_doAnimation = false;
          }
      }
      m_vertexBufferRegenerate |= m_currentFrame != previousFrame;
   }
}

//////////////////////////////
// Positioning
//////////////////////////////

void Primitive::SetObjectPos()
{
    m_vpinball->SetObjectPosCur(m_d.m_vPosition.x, m_d.m_vPosition.y);
}

void Primitive::MoveOffset(const float dx, const float dy)
{
   m_d.m_vPosition.x += dx;
   m_d.m_vPosition.y += dy;
   UpdateStatusBarInfo();
}

Vertex2D Primitive::GetCenter() const
{
   return Vertex2D(m_d.m_vPosition.x, m_d.m_vPosition.y);
}

void Primitive::PutCenter(const Vertex2D& pv)
{
   m_d.m_vPosition.x = pv.x;
   m_d.m_vPosition.y = pv.y;
   UpdateStatusBarInfo();
}

//////////////////////////////
// Save and Load
//////////////////////////////


HRESULT Primitive::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, const bool saveForUndo)
{
   BiffWriter bw(pstm, hcrypthash);

   /*
    * Someone decided that it was a good idea to write these vectors including
    * the fourth padding float that they used to have, so now we have to write
    * them padded to 4 floats to maintain compatibility.
    */
   bw.WriteVector3Padded(FID(VPOS), m_d.m_vPosition);
   bw.WriteVector3Padded(FID(VSIZ), m_d.m_vSize);
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
   bw.WriteString(FID(NRMA), m_d.m_szNormalMap);
   bw.WriteInt(FID(SIDS), m_d.m_Sides);
   bw.WriteWideString(FID(NAME), m_wzName);
   bw.WriteString(FID(MATR), m_d.m_szMaterial);
   bw.WriteInt(FID(SCOL), m_d.m_SideColor);
   bw.WriteBool(FID(TVIS), m_d.m_visible);
   bw.WriteBool(FID(DTXI), m_d.m_drawTexturesInside);
   bw.WriteBool(FID(HTEV), m_d.m_hitEvent);
   bw.WriteFloat(FID(THRS), m_d.m_threshold);
   bw.WriteFloat(FID(ELAS), m_d.m_elasticity);
   bw.WriteFloat(FID(ELFO), m_d.m_elasticityFalloff);
   bw.WriteFloat(FID(RFCT), m_d.m_friction);
   bw.WriteFloat(FID(RSCT), m_d.m_scatter);
   bw.WriteFloat(FID(EFUI), m_d.m_edgeFactorUI);
   bw.WriteFloat(FID(CORF), m_d.m_collision_reductionFactor);
   bw.WriteBool(FID(CLDR), m_d.m_collidable);
   bw.WriteBool(FID(ISTO), m_d.m_toy);
   bw.WriteBool(FID(U3DM), m_d.m_use3DMesh);
   bw.WriteBool(FID(STRE), m_d.m_staticRendering);
   bw.WriteFloat(FID(DILT), m_d.m_disableLightingTop);
   bw.WriteFloat(FID(DILB), m_d.m_disableLightingBelow);
   bw.WriteBool(FID(REEN), m_d.m_reflectionEnabled);
   bw.WriteBool(FID(EBFC), m_d.m_backfacesEnabled);
   bw.WriteString(FID(MAPH), m_d.m_szPhysicsMaterial);
   bw.WriteBool(FID(OVPH), m_d.m_overwritePhysics);
   bw.WriteBool(FID(DIPT), m_d.m_displayTexture);
   bw.WriteBool(FID(OSNM), m_d.m_objectSpaceNormalMap);

   // Don't save the meshes for undo/redo
   if (m_d.m_use3DMesh && !saveForUndo)
   {
      bw.WriteString(FID(M3DN), m_d.m_meshFileName);
      bw.WriteInt(FID(M3VN), (int)m_mesh.NumVertices());

#ifndef COMPRESS_MESHES
      bw.WriteStruct(FID(M3DX), m_mesh.m_vertices.data(), (int)(sizeof(Vertex3D_NoTex2)*m_mesh.NumVertices()));
#else
      /*bw.WriteTag(FID(M3CX));
      {
      LZWWriter lzwwriter(pstm, (int *)m_mesh.m_vertices.data(), sizeof(Vertex3D_NoTex2)*m_mesh.NumVertices(), 1, sizeof(Vertex3D_NoTex2)*m_mesh.NumVertices());
      lzwwriter.CompressBits(8 + 1);
      }*/
      {
      const mz_ulong slen = (mz_ulong)(sizeof(Vertex3D_NoTex2)*m_mesh.NumVertices());
      mz_ulong clen = compressBound(slen);
      mz_uint8 * c = (mz_uint8 *)malloc(clen);
      if (compress2(c, &clen, (const unsigned char *)m_mesh.m_vertices.data(), slen, MZ_BEST_COMPRESSION) != Z_OK)
         ShowError("Could not compress primitive vertex data");
      bw.WriteInt(FID(M3CY), (int)clen);
      bw.WriteStruct(FID(M3CX), c, clen);
      free(c);
      }
#endif

      bw.WriteInt(FID(M3FN), (int)m_mesh.NumIndices());
      if (m_mesh.NumVertices() > 65535)
      {
#ifndef COMPRESS_MESHES
         bw.WriteStruct(FID(M3DI), m_mesh.m_indices.data(), (int)(sizeof(unsigned int)*m_mesh.NumIndices()));
#else
         /*bw.WriteTag(FID(M3CI));
          LZWWriter lzwwriter(pstm, (int *)m_mesh.m_indices.data(), sizeof(unsigned int)*m_mesh.NumIndices(), 1, sizeof(unsigned int)*m_mesh.NumIndices());
          lzwwriter.CompressBits(8 + 1);*/
         const mz_ulong slen = (mz_ulong)(sizeof(unsigned int)*m_mesh.NumIndices());
         mz_ulong clen = compressBound(slen);
         mz_uint8 * c = (mz_uint8 *)malloc(clen);
         if (compress2(c, &clen, (const unsigned char *)m_mesh.m_indices.data(), slen, MZ_BEST_COMPRESSION) != Z_OK)
            ShowError("Could not compress primitive index data");
         bw.WriteInt(FID(M3CJ), (int)clen);
         bw.WriteStruct(FID(M3CI), c, clen);
         free(c);
#endif
      }
      else
      {
         vector<WORD> tmp(m_mesh.NumIndices());
         for (size_t i = 0; i < m_mesh.NumIndices(); ++i)
            tmp[i] = m_mesh.m_indices[i];
#ifndef COMPRESS_MESHES
         bw.WriteStruct(FID(M3DI), tmp.data(), (int)(sizeof(WORD)*m_mesh.NumIndices()));
#else
         /*bw.WriteTag(FID(M3CI));
          LZWWriter lzwwriter(pstm, (int *)tmp.data(), sizeof(WORD)*m_mesh.NumIndices(), 1, sizeof(WORD)*m_mesh.NumIndices());
          lzwwriter.CompressBits(8 + 1);*/
         const mz_ulong slen = (mz_ulong)(sizeof(WORD)*m_mesh.NumIndices());
         mz_ulong clen = compressBound(slen);
         mz_uint8 * c = (mz_uint8 *)malloc(clen);
         if (compress2(c, &clen, (const unsigned char *)tmp.data(), slen, MZ_BEST_COMPRESSION) != Z_OK)
            ShowError("Could not compress primitive index data");
         bw.WriteInt(FID(M3CJ), (int)clen);
         bw.WriteStruct(FID(M3CI), c, clen);
         free(c);
#endif
      }
      
      if (!m_mesh.m_animationFrames.empty())
      {
         const mz_ulong slen = (mz_ulong)(sizeof(Mesh::VertData)*m_mesh.NumVertices());
         for (size_t i = 0; i < m_mesh.m_animationFrames.size(); i++)
         {
            mz_ulong clen = compressBound(slen);
            mz_uint8 * c = (mz_uint8 *)malloc(clen);
            if (compress2(c, &clen, (const unsigned char *)m_mesh.m_animationFrames[i].m_frameVerts.data(), slen, MZ_BEST_COMPRESSION) != Z_OK)
               ShowError("Could not compress primitive animation vertex data");
            bw.WriteInt(FID(M3AY), (int)clen);
            bw.WriteStruct(FID(M3AX), c, clen);
            free(c);
         }

      }
   }
   bw.WriteFloat(FID(PIDB), m_d.m_depthBias);
   bw.WriteBool(FID(ADDB), m_d.m_addBlend);
   bw.WriteBool(FID(ZMSK), m_d.m_useDepthMask);
   bw.WriteFloat(FID(FALP), m_d.m_alpha);
   bw.WriteInt(FID(COLR), m_d.m_color);

   bw.WriteString(FID(LMAP), m_d.m_szLightmap);

   bw.WriteString(FID(REFL), m_d.m_szReflectionProbe);
   bw.WriteFloat(FID(RSTR), m_d.m_reflectionStrength);
   bw.WriteString(FID(REFR), m_d.m_szRefractionProbe);
   bw.WriteFloat(FID(RTHI), m_d.m_refractionThickness);

   ISelect::SaveData(pstm, hcrypthash);

   bw.WriteTag(FID(ENDB));

   return S_OK;
}

HRESULT Primitive::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   SetDefaults(false);

   BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);

   m_ptable = ptable;
   m_mesh.m_validBounds = false;

   br.Load();

   if(version < 1011) // so that old tables do the reorderForsyth on each load, new tables only on mesh import, so a simple resave of a old table will also skip this step
   {
      WaitForMeshDecompression(); //!! needed nowadays due to multithreaded mesh decompression

      unsigned int* const tmp = reorderForsyth(m_mesh.m_indices, (int)m_mesh.NumVertices());
      if (tmp != nullptr)
      {
         memcpy(m_mesh.m_indices.data(), tmp, m_mesh.NumIndices() * sizeof(unsigned int));
         delete[] tmp;
      }
   }

   m_inPlayState = m_d.m_visible;

   return S_OK;
}

bool Primitive::LoadToken(const int id, BiffReader * const pbr)
{
   switch(id)
   {
   case FID(PIID): pbr->GetInt((int *)pbr->m_pdata); break;
   case FID(VPOS): pbr->GetVector3Padded(m_d.m_vPosition); break;
   case FID(VSIZ): pbr->GetVector3Padded(m_d.m_vSize); break;
   case FID(RTV0): pbr->GetFloat(m_d.m_aRotAndTra[0]); break;
   case FID(RTV1): pbr->GetFloat(m_d.m_aRotAndTra[1]); break;
   case FID(RTV2): pbr->GetFloat(m_d.m_aRotAndTra[2]); break;
   case FID(RTV3): pbr->GetFloat(m_d.m_aRotAndTra[3]); break;
   case FID(RTV4): pbr->GetFloat(m_d.m_aRotAndTra[4]); break;
   case FID(RTV5): pbr->GetFloat(m_d.m_aRotAndTra[5]); break;
   case FID(RTV6): pbr->GetFloat(m_d.m_aRotAndTra[6]); break;
   case FID(RTV7): pbr->GetFloat(m_d.m_aRotAndTra[7]); break;
   case FID(RTV8): pbr->GetFloat(m_d.m_aRotAndTra[8]); break;
   case FID(IMAG): pbr->GetString(m_d.m_szImage); break;
   case FID(NRMA): pbr->GetString(m_d.m_szNormalMap); break;
   case FID(SIDS): pbr->GetInt(m_d.m_Sides); break;
   case FID(NAME): pbr->GetWideString(m_wzName,sizeof(m_wzName)/sizeof(m_wzName[0])); break;
   case FID(MATR): pbr->GetString(m_d.m_szMaterial); break;
   case FID(SCOL): pbr->GetInt(m_d.m_SideColor); break;
   case FID(TVIS): pbr->GetBool(m_d.m_visible); break;
   case FID(REEN): pbr->GetBool(m_d.m_reflectionEnabled); break;
   case FID(DTXI): pbr->GetBool(m_d.m_drawTexturesInside); break;
   case FID(HTEV): pbr->GetBool(m_d.m_hitEvent); break;
   case FID(THRS): pbr->GetFloat(m_d.m_threshold); break;
   case FID(ELAS): pbr->GetFloat(m_d.m_elasticity); break;
   case FID(ELFO): pbr->GetFloat(m_d.m_elasticityFalloff); break;
   case FID(RFCT): pbr->GetFloat(m_d.m_friction); break;
   case FID(RSCT): pbr->GetFloat(m_d.m_scatter); break;
   case FID(EFUI): pbr->GetFloat(m_d.m_edgeFactorUI); break;
   case FID(CORF): pbr->GetFloat(m_d.m_collision_reductionFactor); break;
   case FID(CLDR): pbr->GetBool(m_d.m_collidable); break;
   case FID(ISTO): pbr->GetBool(m_d.m_toy); break;
   case FID(MAPH): pbr->GetString(m_d.m_szPhysicsMaterial); break;
   case FID(OVPH): pbr->GetBool(m_d.m_overwritePhysics); break;
   case FID(STRE): pbr->GetBool(m_d.m_staticRendering); break;
   case FID(DILI): { int tmp; pbr->GetInt(tmp); m_d.m_disableLightingTop = (tmp == 1) ? 1.f : dequantizeUnsigned<8>(tmp); break; } // Pre 10.8 compatible hacky loading!
   case FID(DILT): pbr->GetFloat(m_d.m_disableLightingTop); break;
   case FID(DILB): pbr->GetFloat(m_d.m_disableLightingBelow); break;
   case FID(U3DM): pbr->GetBool(m_d.m_use3DMesh); break;
   case FID(EBFC): pbr->GetBool(m_d.m_backfacesEnabled); break;
   case FID(DIPT): pbr->GetBool(m_d.m_displayTexture); break;
   case FID(M3DN): pbr->GetString(m_d.m_meshFileName); break;
   case FID(M3VN):
   {
      pbr->GetInt(m_numVertices);
      if (!m_mesh.m_animationFrames.empty())
      {
         for (size_t i = 0; i < m_mesh.m_animationFrames.size(); i++)
            m_mesh.m_animationFrames[i].m_frameVerts.clear();
         m_mesh.m_animationFrames.clear();
      }
      break;
   }
   case FID(M3DX):
   {
      m_mesh.m_vertices.clear();
      m_mesh.m_vertices.resize(m_numVertices);
      pbr->GetStruct(m_mesh.m_vertices.data(), (int)sizeof(Vertex3D_NoTex2)*m_numVertices);
      break;
   }
#ifdef COMPRESS_MESHES
   case FID(M3AY): pbr->GetInt(m_compressedAnimationVertices); break;
   case FID(M3AX):
   {
      Mesh::FrameData frameData;
      frameData.m_frameVerts.clear();
      frameData.m_frameVerts.resize(m_numVertices);

      /*LZWReader lzwreader(pbr->m_pistream, (int *)m_mesh.m_vertices.data(), sizeof(Vertex3D_NoTex2)*numVertices, 1, sizeof(Vertex3D_NoTex2)*numVertices);
      lzwreader.Decoder();*/
      mz_ulong uclen = (mz_ulong)(sizeof(Mesh::VertData)*m_mesh.NumVertices());
      mz_uint8 * c = (mz_uint8 *)malloc(m_compressedAnimationVertices);
      pbr->GetStruct(c, m_compressedAnimationVertices);
      const int error = uncompress((unsigned char *)frameData.m_frameVerts.data(), &uclen, c, m_compressedAnimationVertices);
      if (error != Z_OK)
         ShowError("Could not uncompress primitive animation vertex data, error "+std::to_string(error));
      free(c);
      m_mesh.m_animationFrames.push_back(frameData);
      break;
   }
   case FID(M3CY): pbr->GetInt(m_compressedVertices); break;
   case FID(M3CX):
   {
      m_mesh.m_vertices.clear();
      m_mesh.m_vertices.resize(m_numVertices);
      /*LZWReader lzwreader(pbr->m_pistream, (int *)m_mesh.m_vertices.data(), sizeof(Vertex3D_NoTex2)*numVertices, 1, sizeof(Vertex3D_NoTex2)*numVertices);
       lzwreader.Decoder();*/
      mz_ulong uclen = (mz_ulong)(sizeof(Vertex3D_NoTex2)*m_mesh.NumVertices());
      mz_uint8 * c = (mz_uint8 *)malloc(m_compressedVertices);
      pbr->GetStruct(c, m_compressedVertices);
	  if (g_pPrimitiveDecompressThreadPool == nullptr)
		  g_pPrimitiveDecompressThreadPool = new ThreadPool(g_pvp->m_logicalNumberOfProcessors);

	  g_pPrimitiveDecompressThreadPool->enqueue([uclen, c, this] {
		  mz_ulong uclen2 = uclen;
		  const int error = uncompress((unsigned char *)m_mesh.m_vertices.data(), &uclen2, c, m_compressedVertices);
		  if (error != Z_OK)
			  ShowError("Could not uncompress primitive vertex data, error "+std::to_string(error));
		  free(c);
	  });
      break;
   }
#endif
   case FID(M3FN): pbr->GetInt(m_numIndices); break;
   case FID(M3DI):
   {
      m_mesh.m_indices.resize(m_numIndices);
      if (m_numVertices > 65535)
         pbr->GetStruct(m_mesh.m_indices.data(), (int)sizeof(unsigned int)*m_numIndices);
      else
      {
         vector<WORD> tmp(m_numIndices);
         pbr->GetStruct(tmp.data(), (int)sizeof(WORD)*m_numIndices);
         for (int i = 0; i < m_numIndices; ++i)
            m_mesh.m_indices[i] = tmp[i];
      }
      break;
   }
#ifdef COMPRESS_MESHES
   case FID(M3CJ): pbr->GetInt(m_compressedIndices); break;
   case FID(M3CI):
   {
      m_mesh.m_indices.resize(m_numIndices);
      if (m_numVertices > 65535)
      {
         //LZWReader lzwreader(pbr->m_pistream, (int *)m_mesh.m_indices.data(), sizeof(unsigned int)*numIndices, 1, sizeof(unsigned int)*numIndices);
         //lzwreader.Decoder();
         mz_ulong uclen = (mz_ulong)(sizeof(unsigned int)*m_mesh.NumIndices());
         mz_uint8 * c = (mz_uint8 *)malloc(m_compressedIndices);
         pbr->GetStruct(c, m_compressedIndices);
		 if (g_pPrimitiveDecompressThreadPool == nullptr)
			 g_pPrimitiveDecompressThreadPool = new ThreadPool(g_pvp->m_logicalNumberOfProcessors);

		 g_pPrimitiveDecompressThreadPool->enqueue([uclen, c, this] {
			 mz_ulong uclen2 = uclen;
			 const int error = uncompress((unsigned char *)m_mesh.m_indices.data(), &uclen2, c, m_compressedIndices);
			 if (error != Z_OK)
				 ShowError("Could not uncompress (large) primitive index data, error "+std::to_string(error));
			 free(c);
		 });
      }
      else
      {
         //LZWReader lzwreader(pbr->m_pistream, (int *)tmp.data(), sizeof(WORD)*numIndices, 1, sizeof(WORD)*numIndices);
         //lzwreader.Decoder();
         mz_ulong uclen = (mz_ulong)(sizeof(WORD)*m_mesh.NumIndices());
         mz_uint8 * c = (mz_uint8 *)malloc(m_compressedIndices);
         pbr->GetStruct(c, m_compressedIndices);
         if (g_pPrimitiveDecompressThreadPool == nullptr)
            g_pPrimitiveDecompressThreadPool = new ThreadPool(g_pvp->m_logicalNumberOfProcessors);

         g_pPrimitiveDecompressThreadPool->enqueue([uclen, c, this] {
            vector<WORD> tmp(m_numIndices);

            mz_ulong uclen2 = uclen;
            const int error = uncompress((unsigned char *)tmp.data(), &uclen2, c, m_compressedIndices);
            if (error != Z_OK)
               ShowError("Could not uncompress (small) primitive index data, error "+std::to_string(error));
            free(c);
            for (int i = 0; i < m_numIndices; ++i)
               m_mesh.m_indices[i] = tmp[i];
         });
      }
      break;
   }
#endif
   case FID(PIDB): pbr->GetFloat(m_d.m_depthBias); break;
   case FID(OSNM): pbr->GetBool(m_d.m_objectSpaceNormalMap); break;
   case FID(ADDB): pbr->GetBool(m_d.m_addBlend); break;
   case FID(ZMSK): pbr->GetBool(m_d.m_useDepthMask); break;
   case FID(FALP): pbr->GetFloat(m_d.m_alpha); break;
   case FID(COLR): pbr->GetInt(m_d.m_color); break;

   case FID(LMAP): pbr->GetString(m_d.m_szLightmap); break;

   case FID(REFL): pbr->GetString(m_d.m_szReflectionProbe); break;
   case FID(RSTR): pbr->GetFloat(m_d.m_reflectionStrength); break;
   case FID(REFR): pbr->GetString(m_d.m_szRefractionProbe); break;
   case FID(RTHI): pbr->GetFloat(m_d.m_refractionThickness); break;

   default: ISelect::LoadToken(id, pbr); break;
   }
   return true;
}

void Primitive::WaitForMeshDecompression()
{
   if (g_pPrimitiveDecompressThreadPool)
   {
      // This will wait for the threads to finish decompressing meshes.
      g_pPrimitiveDecompressThreadPool->wait_until_nothing_in_flight();
      delete g_pPrimitiveDecompressThreadPool;
      g_pPrimitiveDecompressThreadPool = nullptr;
   }
}

HRESULT Primitive::InitPostLoad()
{
   WaitForMeshDecompression(); //!! needed nowadays due to multithreaded mesh decompression
   UpdateStatusBarInfo();
   return S_OK;
}

INT_PTR CALLBACK Primitive::ObjImportProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#ifndef __STANDALONE__
   static Primitive *prim = nullptr;
   switch (uMsg)
   {
   case WM_INITDIALOG:
   {
      constexpr char nullstring[8] = { 0 };

      prim = (Primitive*)lParam;
      SetDlgItemText(hwndDlg, IDC_FILENAME_EDIT, nullstring);
      CheckDlgButton(hwndDlg, IDC_CONVERT_COORD_CHECK, BST_CHECKED);
      CheckDlgButton(hwndDlg, IDC_REL_POSITION_RADIO, BST_CHECKED);
      CheckDlgButton(hwndDlg, IDC_ABS_POSITION_RADIO, BST_UNCHECKED);
      CheckDlgButton(hwndDlg, IDC_CENTER_MESH, BST_UNCHECKED);
      CheckDlgButton(hwndDlg, IDC_IMPORT_NO_FORSYTH, BST_UNCHECKED);
      EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);
      return TRUE;
   }
   case WM_CLOSE:
   {
      prim = nullptr;
      EndDialog(hwndDlg, FALSE);
      break;
   }
   case WM_COMMAND:
      switch (HIWORD(wParam))
      {
      case BN_CLICKED:
         switch (LOWORD(wParam))
         {
         case IDOK:
         {
            char szFileName[MAXSTRING] = { 0 };

            GetDlgItemText(hwndDlg, IDC_FILENAME_EDIT, szFileName, MAXSTRING);
            if (szFileName[0] == '\0')
            {
               ShowError("No .obj file selected!");
               break;
            }
            prim->m_mesh.Clear();
            prim->m_d.m_use3DMesh = false;
            delete prim->m_meshBuffer;
            prim->m_meshBuffer = nullptr;

            constexpr bool flipTV = false;
            const bool convertToLeftHanded = IsDlgButtonChecked(hwndDlg, IDC_CONVERT_COORD_CHECK) == BST_CHECKED;
            const bool importAbsolutePosition = IsDlgButtonChecked(hwndDlg, IDC_ABS_POSITION_RADIO) == BST_CHECKED;
            const bool centerMesh = IsDlgButtonChecked(hwndDlg, IDC_CENTER_MESH) == BST_CHECKED;
            const bool importMaterial = IsDlgButtonChecked(hwndDlg, IDC_IMPORT_MATERIAL) == BST_CHECKED;
            const bool importAnimation = IsDlgButtonChecked(hwndDlg, IDC_IMPORT_ANIM_SEQUENCE) == BST_CHECKED;
            const bool doForsyth = IsDlgButtonChecked(hwndDlg, IDC_IMPORT_NO_FORSYTH) == BST_UNCHECKED;
            if (importMaterial)
            {
               string szMatName = szFileName;
               if (ReplaceExtensionFromFilename(szMatName, "mtl"s))
               {
                  Material * const mat = new Material();
                  ObjLoader loader;
                  if (loader.LoadMaterial(szMatName, mat))
                  {
                     CComObject<PinTable> * const pActiveTable = g_pvp->GetActiveTable();
                     if (pActiveTable)
                         pActiveTable->AddMaterial(mat);

                     prim->m_d.m_szMaterial = mat->m_szName;
                  }
               }
               else
                  ShowError("Could not load material file.");
            }
            if (prim->m_mesh.LoadWavefrontObj(szFileName, flipTV, convertToLeftHanded))
            {
               if (importAbsolutePosition || centerMesh)
               {
                  for (size_t i = 0; i < prim->m_mesh.m_vertices.size(); i++)
                  {
                     prim->m_mesh.m_vertices[i].x -= prim->m_mesh.middlePoint.x;
                     prim->m_mesh.m_vertices[i].y -= prim->m_mesh.middlePoint.y;
                     prim->m_mesh.m_vertices[i].z -= prim->m_mesh.middlePoint.z;
                  }
                  if (importAbsolutePosition)
                  {
                     prim->m_d.m_vPosition.x = prim->m_mesh.middlePoint.x;
                     prim->m_d.m_vPosition.y = prim->m_mesh.middlePoint.y;
                     prim->m_d.m_vPosition.z = prim->m_mesh.middlePoint.z;
                     prim->m_d.m_vSize.x = 1.0f;
                     prim->m_d.m_vSize.y = 1.0f;
                     prim->m_d.m_vSize.z = 1.0f;
                  }
               }
               if (importAnimation)
               {
                  if (prim->m_mesh.LoadAnimation(szFileName, flipTV, convertToLeftHanded))
                  {
                     if (centerMesh)
                     {
                        for (size_t t = 0; t < prim->m_mesh.m_animationFrames.size(); t++)
                        {
                           for (size_t i = 0; i < prim->m_mesh.m_vertices.size(); i++)
                           {
                              prim->m_mesh.m_animationFrames[t].m_frameVerts[i].x -= prim->m_mesh.middlePoint.x;
                              prim->m_mesh.m_animationFrames[t].m_frameVerts[i].y -= prim->m_mesh.middlePoint.y;
                              prim->m_mesh.m_animationFrames[t].m_frameVerts[i].z -= prim->m_mesh.middlePoint.z;
                           }
                        }
                     }
                  }
               }
               prim->m_d.m_use3DMesh = true;
               if (doForsyth)
               {
                   unsigned int* const tmp = reorderForsyth(prim->m_mesh.m_indices, (int)prim->m_mesh.NumVertices());
                   if (tmp != nullptr)
                   {
                       memcpy(prim->m_mesh.m_indices.data(), tmp, prim->m_mesh.NumIndices() * sizeof(unsigned int));
                       delete[] tmp;
                   }
               }
               prim->UpdateStatusBarInfo();
               prim = nullptr;
               EndDialog(hwndDlg, TRUE);
            }
            else
               ShowError("Unable to open file!");
            break;
         }
         case IDC_BROWSE_BUTTON:
         {
            if (prim == nullptr)
               break;

            SetForegroundWindow(hwndDlg);

            string szInitialDir = g_pvp->m_settings.LoadValueWithDefault(Settings::RecentDir, "ImportDir"s, PATH_TABLES);

            vector<string> szFileName;
            if (g_pvp->OpenFileDialog(szInitialDir, szFileName, "Wavefront obj file (*.obj)\0*.obj\0", "obj", 0))
            {
               SetDlgItemText(hwndDlg, IDC_FILENAME_EDIT, szFileName[0].c_str());

               size_t index = szFileName[0].find_last_of(PATH_SEPARATOR_CHAR);
               if (index != string::npos)
               {
                  g_pvp->m_settings.SaveValue(Settings::RecentDir, "ImportDir"s, szFileName[0].substr(0, index));
                  index++;
                  prim->m_d.m_meshFileName = szFileName[0].substr(index, szFileName[0].length() - index);
               }

               EnableWindow(GetDlgItem(hwndDlg, IDOK), TRUE);
            }
            break;
         }
         case IDCANCEL:
         {
            prim = nullptr;
            EndDialog(hwndDlg, FALSE);
            break;
         }
         }
      }
   }
#endif
   return FALSE;
}

bool Primitive::BrowseFor3DMeshFile()
{
#ifndef __STANDALONE__
   DialogBoxParam(m_vpinball->theInstance, MAKEINTRESOURCE(IDD_MESH_IMPORT_DIALOG), m_vpinball->GetHwnd(), ObjImportProc, (size_t)this);
#endif
#if 1
   return false;
#else
   char szFileName[MAXSTRING];
   szFileName[0] = '\0';
   string szInitialDir;

   OPENFILENAME ofn = {};
   ofn.lStructSize = sizeof(OPENFILENAME);
   ofn.hInstance = m_vpinball->theInstance;
   ofn.hwndOwner = m_vpinball->m_hwnd;
   // TEXT
   ofn.lpstrFilter = "Wavefront obj file (*.obj)\0*.obj\0";
   ofn.lpstrFile = szFileName;
   ofn.nMaxFile = sizeof(szFileName);
   ofn.lpstrDefExt = "obj";
   ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

   szInitialDir = g_pvp->m_settings.LoadValueWithDefault(Settings::RecentDir, "ImportDir"s, PATH_TABLES);

   ofn.lpstrInitialDir = szInitialDir.c_str();

   const int ret = GetOpenFileName(&ofn);
   if (ret == 0)
      return false;

   string filename(ofn.lpstrFile);
   size_t index = filename.find_last_of(PATH_SEPARATOR_CHAR);
   if (index != string::npos)
   {
      const string newInitDir(szFilename.substr(0, index));
      g_pvp->m_settings.SaveValue(Settings::RecentDir, "ImportDir"s, newInitDir);
      index++;
      m_d.m_meshFileName = filename.substr(index, filename.length() - index);
   }

   m_mesh.Clear();
   m_d.m_use3DMesh = false;
   SAFE_BUFFER_RELEASE(vertexBuffer);

   bool flipTV = false;
   bool convertToLeftHanded = false;
   int ans = m_vpinball->MessageBox("Do you want to mirror the object?", "Convert coordinate system?", MB_YESNO | MB_DEFBUTTON2);
   if (ans == IDYES)
   {
      convertToLeftHanded = true;
   }
   else
   {
      ans = m_vpinball->MessageBox("Do you want to convert texture coordinates?", "Confirm", MB_YESNO | MB_DEFBUTTON2);
      if (ans == IDYES)
      {
         flipTV = true;
      }
   }
   if (m_mesh.LoadWavefrontObj(ofn.lpstrFile, flipTV, convertToLeftHanded))
   {
      m_d.m_vPosition.x = m_mesh.middlePoint.x;
      m_d.m_vPosition.y = m_mesh.middlePoint.y;
      m_d.m_vPosition.z = m_mesh.middlePoint.z;
      m_d.m_vSize.x = 1.0f;
      m_d.m_vSize.y = 1.0f;
      m_d.m_vSize.z = 1.0f;
      m_d.m_use3DMesh = true;
      UpdateStatusBarInfo();
      return true;
   }
   return false;
#endif
}

//////////////////////////////
// Standard methods
//////////////////////////////

STDMETHODIMP Primitive::get_Image(BSTR *pVal)
{
   WCHAR wz[MAXTOKEN];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szImage.c_str(), -1, wz, MAXTOKEN);
   *pVal = SysAllocString(wz);
   return S_OK;
}

STDMETHODIMP Primitive::put_Image(BSTR newVal)
{
   char szImage[MAXTOKEN];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, szImage, MAXTOKEN, nullptr, nullptr);
   m_d.m_szImage = szImage;

   return S_OK;
}

STDMETHODIMP Primitive::get_NormalMap(BSTR *pVal)
{
    WCHAR wz[MAXTOKEN];
    MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szNormalMap.c_str(), -1, wz, MAXTOKEN);
    *pVal = SysAllocString(wz);

    return S_OK;
}

STDMETHODIMP Primitive::put_NormalMap(BSTR newVal)
{
    char szImage[MAXTOKEN];
    WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, szImage, MAXTOKEN, nullptr, nullptr);
    const Texture * const tex = m_ptable->GetImage(szImage);
    if (tex && tex->IsHDR())
    {
        ShowError("Cannot use a HDR image (.exr/.hdr) here");
        return E_FAIL;
    }

    m_d.m_szNormalMap = szImage;

    return S_OK;
}

STDMETHODIMP Primitive::get_MeshFileName(BSTR *pVal)
{
   WCHAR wz[MAXSTRING];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_meshFileName.c_str(), -1, wz, MAXSTRING);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Primitive::put_MeshFileName(BSTR newVal)
{
   char buf[MAXSTRING];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXSTRING, nullptr, nullptr);
   m_d.m_meshFileName = buf;

   return S_OK;
}

bool Primitive::LoadMeshDialog()
{
   STARTUNDO
   const bool result = BrowseFor3DMeshFile();
   m_vertexBufferRegenerate = true;
   STOPUNDO

   return result;
}

void Primitive::ExportMeshDialog()
{
#ifndef __STANDALONE__
   string szInitialDir = g_pvp->m_settings.LoadValueWithDefault(Settings::RecentDir, "ImportDir"s, PATH_TABLES);

   vector<string> szFileName;
   if (m_vpinball->SaveFileDialog(szInitialDir, szFileName, "Wavefront obj file (*.obj)\0*.obj\0", "obj", OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY))
   {
      const size_t index = szFileName[0].find_last_of(PATH_SEPARATOR_CHAR);
      if (index != string::npos)
      {
         const string newInitDir(szFileName[0].substr(0, index));
         g_pvp->m_settings.SaveValue(Settings::RecentDir, "ImportDir"s, newInitDir);
      }

      char name[sizeof(m_wzName) / sizeof(m_wzName[0])];
      WideCharToMultiByteNull(CP_ACP, 0, m_wzName, -1, name, sizeof(name), nullptr, nullptr);
      m_mesh.SaveWavefrontObj(szFileName[0], m_d.m_use3DMesh ? string(name) : "Primitive"s);
   }
#endif
}

float Primitive::GetDepth(const Vertex3Ds& viewDir) const
{
   return m_d.m_depthBias + m_d.m_vPosition.Dot(viewDir);
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
      m_d.m_Sides = newVal;
      if (!m_d.m_use3DMesh)
      {
         m_vertexBufferRegenerate = true;
      }
   }

   return S_OK;
}

STDMETHODIMP Primitive::get_Material(BSTR *pVal)
{
   WCHAR wz[MAXNAMEBUFFER];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szMaterial.c_str(), -1, wz, MAXNAMEBUFFER);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Primitive::put_Material(BSTR newVal)
{
   char buf[MAXNAMEBUFFER];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXNAMEBUFFER, nullptr, nullptr);
   m_d.m_szMaterial = buf;

   return S_OK;
}

STDMETHODIMP Primitive::get_SideColor(OLE_COLOR *pVal)
{
   *pVal = m_d.m_SideColor;

   return S_OK;
}

STDMETHODIMP Primitive::put_SideColor(OLE_COLOR newVal)
{
   m_d.m_SideColor = newVal;

   return S_OK;
}

STDMETHODIMP Primitive::get_Visible(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_visible);

   return S_OK;
}

STDMETHODIMP Primitive::put_Visible(VARIANT_BOOL newVal)
{
   m_d.m_visible = VBTOF(newVal);

   return S_OK;
}

STDMETHODIMP Primitive::get_DrawTexturesInside(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_drawTexturesInside);

   return S_OK;
}

STDMETHODIMP Primitive::put_DrawTexturesInside(VARIANT_BOOL newVal)
{
   if (m_d.m_drawTexturesInside != VBTOb(newVal))
   {
      m_d.m_drawTexturesInside = VBTOb(newVal);
      m_vertexBufferRegenerate = true;
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
   m_d.m_vPosition.x = newVal;

   return S_OK;
}

STDMETHODIMP Primitive::get_Y(float *pVal)
{
   *pVal = m_d.m_vPosition.y;

   return S_OK;
}

STDMETHODIMP Primitive::put_Y(float newVal)
{
   m_d.m_vPosition.y = newVal;

   return S_OK;
}

STDMETHODIMP Primitive::get_Z(float *pVal)
{
   *pVal = m_d.m_vPosition.z;

   return S_OK;
}

STDMETHODIMP Primitive::put_Z(float newVal)
{
   m_d.m_vPosition.z = newVal;

   return S_OK;
}

STDMETHODIMP Primitive::get_Size_X(float *pVal)
{
   *pVal = m_d.m_vSize.x;

   return S_OK;
}

STDMETHODIMP Primitive::put_Size_X(float newVal)
{
   m_d.m_vSize.x = newVal;

   return S_OK;
}

STDMETHODIMP Primitive::get_Size_Y(float *pVal)
{
   *pVal = m_d.m_vSize.y;

   return S_OK;
}

STDMETHODIMP Primitive::put_Size_Y(float newVal)
{
   m_d.m_vSize.y = newVal;

   return S_OK;
}

STDMETHODIMP Primitive::get_Size_Z(float *pVal)
{
   *pVal = m_d.m_vSize.z;

   return S_OK;
}

STDMETHODIMP Primitive::put_Size_Z(float newVal)
{
   m_d.m_vSize.z = newVal;

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
   m_d.m_aRotAndTra[0] = newVal;

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
   m_d.m_aRotAndTra[1] = newVal;

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
   m_d.m_aRotAndTra[2] = newVal;

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
   m_d.m_aRotAndTra[3] = newVal;

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
   m_d.m_aRotAndTra[4] = newVal;

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
   m_d.m_aRotAndTra[5] = newVal;

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
   m_d.m_aRotAndTra[6] = newVal;

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
   m_d.m_aRotAndTra[7] = newVal;

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
   m_d.m_aRotAndTra[8] = newVal;

   return S_OK;
}


STDMETHODIMP Primitive::get_Opacity(float *pVal)
{
   *pVal = m_d.m_alpha;
   return S_OK;
}

STDMETHODIMP Primitive::put_Opacity(float newVal)
{
   SetAlpha(newVal);
   return S_OK;
}

STDMETHODIMP Primitive::get_Color(OLE_COLOR *pVal)
{
   *pVal = m_d.m_color;

   return S_OK;
}

STDMETHODIMP Primitive::put_Color(OLE_COLOR newVal)
{
   m_d.m_color = newVal;

   return S_OK;
}

STDMETHODIMP Primitive::get_AddBlend(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_addBlend);

   return S_OK;
}

STDMETHODIMP Primitive::put_AddBlend(VARIANT_BOOL newVal)
{
   m_d.m_addBlend = VBTOb(newVal);
   return S_OK;
}

STDMETHODIMP Primitive::get_EnableDepthMask(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_useDepthMask);

   return S_OK;
}

STDMETHODIMP Primitive::put_EnableDepthMask(VARIANT_BOOL newVal)
{
   m_d.m_useDepthMask = VBTOb(newVal);
   return S_OK;
}

STDMETHODIMP Primitive::get_EdgeFactorUI(float *pVal)
{
   *pVal = m_d.m_edgeFactorUI;

   return S_OK;
}

STDMETHODIMP Primitive::put_EdgeFactorUI(float newVal)
{
   m_d.m_edgeFactorUI = newVal;

   return S_OK;
}

STDMETHODIMP Primitive::get_CollisionReductionFactor(float *pVal)
{
   *pVal = m_d.m_collision_reductionFactor;

   return S_OK;
}

STDMETHODIMP Primitive::put_CollisionReductionFactor(float newVal)
{
   m_d.m_collision_reductionFactor = newVal;

   return S_OK;
}

STDMETHODIMP Primitive::get_EnableStaticRendering(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_staticRendering);

   return S_OK;
}

STDMETHODIMP Primitive::put_EnableStaticRendering(VARIANT_BOOL newVal)
{
   m_d.m_staticRendering = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP Primitive::get_HasHitEvent(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_hitEvent);

   return S_OK;
}

STDMETHODIMP Primitive::put_HasHitEvent(VARIANT_BOOL newVal)
{
   m_d.m_hitEvent = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP Primitive::get_Threshold(float *pVal)
{
   *pVal = m_d.m_threshold;

   return S_OK;
}

STDMETHODIMP Primitive::put_Threshold(float newVal)
{
   m_d.m_threshold = newVal;

   return S_OK;
}

STDMETHODIMP Primitive::get_Elasticity(float *pVal)
{
   *pVal = m_d.m_elasticity;

   return S_OK;
}

STDMETHODIMP Primitive::put_Elasticity(float newVal)
{
   m_d.m_elasticity = newVal;

   return S_OK;
}

STDMETHODIMP Primitive::get_ElasticityFalloff(float *pVal)
{
   *pVal = m_d.m_elasticityFalloff;

   return S_OK;
}

STDMETHODIMP Primitive::put_ElasticityFalloff(float newVal)
{
   m_d.m_elasticityFalloff = newVal;

   return S_OK;
}

STDMETHODIMP Primitive::get_Friction(float *pVal)
{
   *pVal = m_d.m_friction;

   return S_OK;
}

STDMETHODIMP Primitive::put_Friction(float newVal)
{
   m_d.m_friction = clamp(newVal, 0.f, 1.f);

   return S_OK;
}

STDMETHODIMP Primitive::get_Scatter(float *pVal)
{
   *pVal = m_d.m_scatter;

   return S_OK;
}

STDMETHODIMP Primitive::put_Scatter(float newVal)
{
   m_d.m_scatter = newVal;

   return S_OK;
}

STDMETHODIMP Primitive::get_Collidable(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB((!g_pplayer || m_vhoCollidable.empty()) ? m_d.m_collidable : m_vhoCollidable[0]->m_enabled);

   return S_OK;
}

STDMETHODIMP Primitive::put_Collidable(VARIANT_BOOL newVal)
{
   const bool val = VBTOb(newVal);
   if (!g_pplayer)
      m_d.m_collidable = val;
   else
   {
       if (!m_vhoCollidable.empty() && m_vhoCollidable[0]->m_enabled != val)
           for (size_t i = 0; i < m_vhoCollidable.size(); i++) //!! costly
               m_vhoCollidable[i]->m_enabled = val; //copy to hit-testing on entities composing the object
   }

   return S_OK;
}

STDMETHODIMP Primitive::get_IsToy(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_toy);

   return S_OK;
}

STDMETHODIMP Primitive::put_IsToy(VARIANT_BOOL newVal)
{
   m_d.m_toy = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP Primitive::get_BackfacesEnabled(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_backfacesEnabled);
   return S_OK;
}

STDMETHODIMP Primitive::put_BackfacesEnabled(VARIANT_BOOL newVal)
{
   m_d.m_backfacesEnabled = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP Primitive::get_ObjectSpaceNormalMap(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_objectSpaceNormalMap);
   return S_OK;
}

STDMETHODIMP Primitive::put_ObjectSpaceNormalMap(VARIANT_BOOL newVal)
{
   m_d.m_objectSpaceNormalMap = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP Primitive::get_DisableLighting(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_disableLightingTop != 0.f);

   return S_OK;
}

STDMETHODIMP Primitive::put_DisableLighting(VARIANT_BOOL newVal)
{
   m_d.m_disableLightingTop = VBTOb(newVal) ? 1.f : 0;

   return S_OK;
}

STDMETHODIMP Primitive::get_BlendDisableLighting(float *pVal)
{
   *pVal = m_d.m_disableLightingTop;

   return S_OK;
}

STDMETHODIMP Primitive::put_BlendDisableLighting(float newVal)
{
   m_d.m_disableLightingTop = newVal;

   return S_OK;
}

STDMETHODIMP Primitive::get_BlendDisableLightingFromBelow(float *pVal)
{
   *pVal = m_d.m_disableLightingBelow;

   return S_OK;
}

STDMETHODIMP Primitive::put_BlendDisableLightingFromBelow(float newVal)
{
   m_d.m_disableLightingBelow = newVal;

   return S_OK;
}

STDMETHODIMP Primitive::get_ReflectionEnabled(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_reflectionEnabled);

   return S_OK;
}

STDMETHODIMP Primitive::put_ReflectionEnabled(VARIANT_BOOL newVal)
{
   m_d.m_reflectionEnabled = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP Primitive::get_PhysicsMaterial(BSTR *pVal)
{
   WCHAR wz[MAXNAMEBUFFER];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szPhysicsMaterial.c_str(), -1, wz, MAXNAMEBUFFER);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Primitive::put_PhysicsMaterial(BSTR newVal)
{
   char buf[MAXNAMEBUFFER];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXNAMEBUFFER, nullptr, nullptr);
   m_d.m_szPhysicsMaterial = buf;

   return S_OK;
}

STDMETHODIMP Primitive::get_OverwritePhysics(VARIANT_BOOL *pVal)
{
    *pVal = FTOVB(m_d.m_overwritePhysics);

    return S_OK;
}

STDMETHODIMP Primitive::put_OverwritePhysics(VARIANT_BOOL newVal)
{
    m_d.m_overwritePhysics = VBTOb(newVal);

    return S_OK;
}

STDMETHODIMP Primitive::get_HitThreshold(float *pVal)
{
   *pVal = m_currentHitThreshold;

   return S_OK;
}

STDMETHODIMP Primitive::get_DisplayTexture(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_displayTexture);

   return S_OK;
}

STDMETHODIMP Primitive::put_DisplayTexture(VARIANT_BOOL newVal)
{
   m_d.m_displayTexture = VBTOb(newVal);

   return S_OK;
}


STDMETHODIMP Primitive::PlayAnim(float startFrame, float speed)
{
   if (!m_mesh.m_animationFrames.empty() && !m_d.m_staticRendering)
   {
      if ((size_t)startFrame >= m_mesh.m_animationFrames.size())
         startFrame = 0.0f;
      //if (startFrame < 0.0f)
      //   startFrame = -startFrame;
      if (speed < 0.0f) speed = -speed;

      m_vertexBufferRegenerate = m_vertexBufferRegenerate || (m_currentFrame != startFrame) || (m_speed != speed) || !m_doAnimation || m_endless;

      m_currentFrame = startFrame;
      m_speed = speed;
      m_doAnimation = true;
      m_endless = false;
   }

   return S_OK;
}

STDMETHODIMP Primitive::PlayAnimEndless(float speed)
{
   if (!m_mesh.m_animationFrames.empty() && !m_d.m_staticRendering)
   {
      if (speed < 0.0f) speed = -speed;

      m_vertexBufferRegenerate = m_vertexBufferRegenerate || (m_currentFrame != 0.f) || (m_speed != speed) || !m_doAnimation || !m_endless;

      m_currentFrame = 0.0f;
      m_speed = speed;
      m_doAnimation = true;
      m_endless = true;
   }

   return S_OK;
}

STDMETHODIMP Primitive::StopAnim()
{
   m_doAnimation = false;
   m_vertexBufferRegenerate = false;

   return S_OK;
}

STDMETHODIMP Primitive::ContinueAnim(float speed)
{
   if (m_currentFrame > 0.0f && !m_d.m_staticRendering)
   {
      if (speed < 0.0f) speed = -speed;

      m_vertexBufferRegenerate = m_vertexBufferRegenerate || (m_speed != speed) || !m_doAnimation;

      m_speed = speed;
      m_doAnimation = true;
   }

   return S_OK;
}

STDMETHODIMP Primitive::ShowFrame(float frame)
{
   if (!m_mesh.m_animationFrames.empty() && frame >= 0.f && !m_d.m_staticRendering)
   {
      if ((size_t)frame >= m_mesh.m_animationFrames.size())
         frame = (float)(m_mesh.m_animationFrames.size() - 1);

      m_vertexBufferRegenerate = m_vertexBufferRegenerate || (m_currentFrame != frame) || m_doAnimation;

      m_currentFrame = frame;
      m_doAnimation = false;
   }

   return S_OK;
}

void Primitive::SetDefaultPhysics(const bool fromMouseClick)
{
#define strKeyName Settings::DefaultPropsPrimitive

   m_d.m_elasticity = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "Elasticity"s, 0.3f) : 0.3f;
   m_d.m_elasticityFalloff = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "ElasticityFalloff"s, 0.5f) : 0.5f;
   m_d.m_friction = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "Friction"s, 0.3f) : 0.3f;
   m_d.m_scatter = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(strKeyName, "Scatter"s, 0.f) : 0.f;

#undef strKeyName
}

STDMETHODIMP Primitive::get_DepthBias(float *pVal)
{
   *pVal = m_d.m_depthBias;

   return S_OK;
}

STDMETHODIMP Primitive::put_DepthBias(float newVal)
{
   m_d.m_depthBias = newVal;

   return S_OK;
}

//Sets the in play state for light sequencing rendering
void Primitive::setInPlayState(const bool newVal)
{
    m_inPlayState = newVal;
}

STDMETHODIMP Primitive::get_ReflectionProbe(BSTR *pVal)
{
   WCHAR wz[MAXTOKEN];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szReflectionProbe.c_str(), -1, wz, MAXTOKEN);
   *pVal = SysAllocString(wz);
   return S_OK;
}

STDMETHODIMP Primitive::put_ReflectionProbe(BSTR newVal)
{
   char szProbe[MAXTOKEN];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, szProbe, MAXTOKEN, nullptr, nullptr);
   m_d.m_szReflectionProbe = szProbe;

   return S_OK;
}

STDMETHODIMP Primitive::get_RefractionProbe(BSTR *pVal)
{
   WCHAR wz[MAXTOKEN];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szRefractionProbe.c_str(), -1, wz, MAXTOKEN);
   *pVal = SysAllocString(wz);
   return S_OK;
}

STDMETHODIMP Primitive::put_RefractionProbe(BSTR newVal)
{
   char szProbe[MAXTOKEN];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, szProbe, MAXTOKEN, nullptr, nullptr);
   m_d.m_szRefractionProbe = szProbe;

   return S_OK;
}
