// primitive.cpp: implementation of the Primitive class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h" 
#include "forsyth.h"
#include "objloader.h"
#include "inc\miniz.c"
#include "inc\progmesh.h"
#include "inc\ThreadPool.h"

// defined in objloader.cpp
extern bool WaveFrontObj_Load(const string& filename, const bool flipTv, const bool convertToLeftHanded);
extern void WaveFrontObj_GetVertices(std::vector<Vertex3D_NoTex2>& verts);
extern void WaveFrontObj_GetIndices(std::vector<unsigned int>& list);
extern void WaveFrontObj_Save(const string& filename, const string& description, const Mesh& mesh);
//

ThreadPool *g_pPrimitiveDecompressThreadPool = NULL;
extern int logicalNumberOfProcessors;

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
}

bool Mesh::LoadAnimation(const char *fname, const bool flipTV, const bool convertToLeftHanded)
{
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
   WIN32_FIND_DATA data;
   const HANDLE h = FindFirstFile(sname.c_str(), &data);
   std::vector<string> allFiles;
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
      if (WaveFrontObj_Load(sname, flipTV, convertToLeftHanded))
      {
         std::vector<Vertex3D_NoTex2> verts;
         WaveFrontObj_GetVertices(verts);
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
   return true;
}

bool Mesh::LoadWavefrontObj(const string& fname, const bool flipTV, const bool convertToLeftHanded)
{
   Clear();

   if (WaveFrontObj_Load(fname, flipTV, convertToLeftHanded))
   {
      WaveFrontObj_GetVertices(m_vertices);
      WaveFrontObj_GetIndices(m_indices);
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
   WaveFrontObj_Save(fname, description.empty() ? fname : description, *this);
}

void Mesh::UploadToVB(VertexBuffer * vb, const float frame) 
{
   if (frame != -1.f)
   {
      float intPart;
      const float fractpart = modf(frame, &intPart);
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

////////////////////////////////////////////////////////////////////////////////

Primitive::Primitive()
{
   m_vertexBuffer = 0;
   m_vertexBufferRegenerate = true;
   m_indexBuffer = 0;
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

   m_numIndices = 0;
   m_numVertices = 0;
   m_propPhysics = NULL;
   m_propPosition = NULL;
   m_propVisual = NULL;
   memset(m_d.m_szNormalMap, 0, sizeof(m_d.m_szNormalMap));
   m_d.m_overwritePhysics = true;
   m_d.m_useAsPlayfield = false;
}

Primitive::~Primitive()
{
   WaitForMeshDecompression(); //!! needed nowadays due to multithreaded mesh decompression
   if (m_vertexBuffer)
      m_vertexBuffer->release();
   if (m_indexBuffer)
      m_indexBuffer->release();
}

void Primitive::CreateRenderGroup(const Collection * const collection)
{
   if (!collection->m_groupElements)
      return;

   size_t overall_size = 0;
   vector<Primitive*> prims;
   vector<Primitive*> renderedPrims;
   for (int i = 0; i < collection->m_visel.Size(); i++)
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

   if (m_vertexBuffer)
      m_vertexBuffer->release();

   RenderDevice * const pd3dDevice = g_pplayer->m_pin3d.m_pd3dPrimaryDevice;
   pd3dDevice->CreateVertexBuffer(m_numGroupVertices, 0, MY_D3DFVF_NOTEX2_VERTEX, &m_vertexBuffer);

   if (m_indexBuffer)
      m_indexBuffer->release();
   m_indexBuffer = pd3dDevice->CreateAndFillIndexBuffer(indices);

   unsigned int ofs = 0;
   Vertex3D_NoTex2 *buf;
   m_vertexBuffer->lock(0, 0, (void**)&buf, VertexBuffer::WRITEONLY);
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
   m_vertexBuffer->unlock();
}

HRESULT Primitive::Init(PinTable *ptable, float x, float y, bool fromMouseClick)
{
   m_ptable = ptable;

   m_d.m_vPosition.x = x;
   m_d.m_vPosition.y = y;

   SetDefaults(fromMouseClick);

   InitVBA(fTrue, 0, NULL);

   UpdateStatusBarInfo();

   return S_OK;
}

void Primitive::SetDefaults(bool fromMouseClick)
{
   static const char strKeyName[] = "DefaultProps\\Primitive";

   m_d.m_useAsPlayfield = false;
   m_d.m_use3DMesh = false;

   m_d.m_meshFileName.clear();
   // sides
   m_d.m_Sides = fromMouseClick ? LoadValueIntWithDefault(strKeyName, "Sides", 4) : 4;
   if (m_d.m_Sides > Max_Primitive_Sides)
      m_d.m_Sides = Max_Primitive_Sides;

   // colors
   m_d.m_SideColor = fromMouseClick ? LoadValueIntWithDefault(strKeyName, "SideColor", RGB(150, 150, 150)) : RGB(150, 150, 150);

   m_d.m_visible = fromMouseClick ? LoadValueBoolWithDefault(strKeyName, "Visible", true) : true;
   m_d.m_staticRendering = fromMouseClick ? LoadValueBoolWithDefault(strKeyName, "StaticRendering", true) : true;
   m_d.m_drawTexturesInside = fromMouseClick ? LoadValueBoolWithDefault(strKeyName, "DrawTexturesInside", false) : false;

   // Position (X and Y is already set by the click of the user)
   m_d.m_vPosition.z = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "Position_Z", 0.0f) : 0.0f;

   // Size
   m_d.m_vSize.x = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "Size_X", 100.0f) : 100.0f;
   m_d.m_vSize.y = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "Size_Y", 100.0f) : 100.0f;
   m_d.m_vSize.z = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "Size_Z", 100.0f) : 100.0f;

   // Rotation and Transposition
   m_d.m_aRotAndTra[0] = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "RotAndTra0", 0.0f) : 0.0f;
   m_d.m_aRotAndTra[1] = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "RotAndTra1", 0.0f) : 0.0f;
   m_d.m_aRotAndTra[2] = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "RotAndTra2", 0.0f) : 0.0f;
   m_d.m_aRotAndTra[3] = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "RotAndTra3", 0.0f) : 0.0f;
   m_d.m_aRotAndTra[4] = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "RotAndTra4", 0.0f) : 0.0f;
   m_d.m_aRotAndTra[5] = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "RotAndTra5", 0.0f) : 0.0f;
   m_d.m_aRotAndTra[6] = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "RotAndTra6", 0.0f) : 0.0f;
   m_d.m_aRotAndTra[7] = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "RotAndTra7", 0.0f) : 0.0f;
   m_d.m_aRotAndTra[8] = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "RotAndTra8", 0.0f) : 0.0f;

   char buf[MAXTOKEN] = { 0 };
   HRESULT hr = LoadValueString(strKeyName, "Image", buf, MAXTOKEN);
   if ((hr != S_OK) && fromMouseClick)
      m_d.m_szImage.clear();
   else
      m_d.m_szImage = buf;

   hr = LoadValueString(strKeyName, "NormalMap", m_d.m_szNormalMap, MAXTOKEN);
   if ((hr != S_OK) && fromMouseClick)
       m_d.m_szNormalMap[0] = 0;

   m_d.m_threshold = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "HitThreshold", 2.0f) : 2.0f;

   SetDefaultPhysics(fromMouseClick);

   m_d.m_edgeFactorUI = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "EdgeFactorUI", 0.25f) : 0.25f;
   m_d.m_collision_reductionFactor = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "CollisionReductionFactor", 0.f) : 0.f;

   m_d.m_collidable = fromMouseClick ? LoadValueBoolWithDefault(strKeyName, "Collidable", true) : true;
   m_d.m_toy = fromMouseClick ? LoadValueBoolWithDefault(strKeyName, "IsToy", false) : false;
   m_d.m_disableLightingTop = dequantizeUnsigned<8>(fromMouseClick ? LoadValueIntWithDefault(strKeyName, "DisableLighting", 0) : 0); // stored as uchar for backward compatibility
   m_d.m_disableLightingBelow = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "DisableLightingBelow", 0.f) : 0.f;
   m_d.m_reflectionEnabled = fromMouseClick ? LoadValueBoolWithDefault(strKeyName, "ReflectionEnabled", true) : true;
   m_d.m_backfacesEnabled = fromMouseClick ? LoadValueBoolWithDefault(strKeyName, "BackfacesEnabled", false) : false;
   m_d.m_displayTexture = fromMouseClick ? LoadValueBoolWithDefault(strKeyName, "DisplayTexture", false) : false;
   m_d.m_objectSpaceNormalMap = fromMouseClick ? LoadValueBoolWithDefault(strKeyName, "ObjectSpaceNormalMap", false) : false;
}

void Primitive::WriteRegDefaults()
{
   static const char strKeyName[] = "DefaultProps\\Primitive";

   SaveValueInt(strKeyName, "SideColor", m_d.m_SideColor);
   SaveValueBool(strKeyName, "Visible", m_d.m_visible);
   SaveValueBool(strKeyName, "StaticRendering", m_d.m_staticRendering);
   SaveValueBool(strKeyName, "DrawTexturesInside", m_d.m_drawTexturesInside);

   SaveValueFloat(strKeyName, "Position_Z", m_d.m_vPosition.z);

   SaveValueFloat(strKeyName, "Size_X", m_d.m_vSize.x);
   SaveValueFloat(strKeyName, "Size_Y", m_d.m_vSize.y);
   SaveValueFloat(strKeyName, "Size_Z", m_d.m_vSize.z);

   SaveValueFloat(strKeyName, "RotAndTra0", m_d.m_aRotAndTra[0]);
   SaveValueFloat(strKeyName, "RotAndTra1", m_d.m_aRotAndTra[1]);
   SaveValueFloat(strKeyName, "RotAndTra2", m_d.m_aRotAndTra[2]);
   SaveValueFloat(strKeyName, "RotAndTra3", m_d.m_aRotAndTra[3]);
   SaveValueFloat(strKeyName, "RotAndTra4", m_d.m_aRotAndTra[4]);
   SaveValueFloat(strKeyName, "RotAndTra5", m_d.m_aRotAndTra[5]);
   SaveValueFloat(strKeyName, "RotAndTra6", m_d.m_aRotAndTra[6]);
   SaveValueFloat(strKeyName, "RotAndTra7", m_d.m_aRotAndTra[7]);
   SaveValueFloat(strKeyName, "RotAndTra8", m_d.m_aRotAndTra[8]);

   SaveValueString(strKeyName, "Image", m_d.m_szImage);
   SaveValueString(strKeyName, "NormalMap", m_d.m_szNormalMap);
   SaveValueBool(strKeyName, "HitEvent", m_d.m_hitEvent);
   SaveValueFloat(strKeyName, "HitThreshold", m_d.m_threshold);
   SaveValueFloat(strKeyName, "Elasticity", m_d.m_elasticity);
   SaveValueFloat(strKeyName, "ElasticityFalloff", m_d.m_elasticityFalloff);
   SaveValueFloat(strKeyName, "Friction", m_d.m_friction);
   SaveValueFloat(strKeyName, "Scatter", m_d.m_scatter);

   SaveValueFloat(strKeyName, "EdgeFactorUI", m_d.m_edgeFactorUI);
   SaveValueFloat(strKeyName, "CollisionReductionFactor", m_d.m_collision_reductionFactor);

   SaveValueBool(strKeyName, "Collidable", m_d.m_collidable);
   SaveValueBool(strKeyName, "IsToy", m_d.m_toy);
   const int tmp = quantizeUnsigned<8>(clamp(m_d.m_disableLightingTop, 0.f, 1.f));
   SaveValueInt(strKeyName, "DisableLighting", (tmp == 1) ? 0 : tmp); // backwards compatible saving
   SaveValueFloat(strKeyName, "DisableLightingBelow", m_d.m_disableLightingBelow);
   SaveValueBool(strKeyName, "ReflectionEnabled", m_d.m_reflectionEnabled);
   SaveValueBool(strKeyName, "BackfacesEnabled", m_d.m_backfacesEnabled);
   SaveValueBool(strKeyName, "DisplayTexture", m_d.m_displayTexture);
   SaveValueBool(strKeyName, "ObjectSpaceNormalMap", m_d.m_objectSpaceNormalMap);
}

void Primitive::GetTimers(vector<HitTimer*> &pvht)
{
   IEditable::BeginPlay();
}

void Primitive::GetHitShapes(vector<HitObject*> &pvho)
{
   char name[sizeof(m_wzName)/sizeof(m_wzName[0])];
   WideCharToMultiByteNull(CP_ACP, 0, m_wzName, -1, name, sizeof(name), NULL, NULL);
   if (strcmp(name, "playfield_mesh") == 0)
   {
      m_d.m_visible = false;
      m_d.m_useAsPlayfield = true;
   }

   //

   // playfield can't be a toy
   if (m_d.m_toy && !m_d.m_useAsPlayfield)
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
      std::vector<ProgMesh::float3> prog_vertices(m_vertices.size());
      for (size_t i = 0; i < m_vertices.size(); ++i) //!! opt. use original data directly!
      {
         prog_vertices[i].x = m_vertices[i].x;
         prog_vertices[i].y = m_vertices[i].y;
         prog_vertices[i].z = m_vertices[i].z;
      }
      std::vector<ProgMesh::tridata> prog_indices(m_mesh.NumIndices() / 3);
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
      std::vector<unsigned int> prog_map;
      std::vector<unsigned int> prog_perm;
      ProgMesh::ProgressiveMesh(prog_vertices, prog_indices, prog_map, prog_perm);
      ProgMesh::PermuteVertices(prog_perm, prog_vertices, prog_indices);
      prog_perm.clear();

      std::vector<ProgMesh::tridata> prog_new_indices;
      ProgMesh::ReMapIndices(reduced_vertices, prog_indices, prog_new_indices, prog_map);
      prog_indices.clear();
      prog_map.clear();

      //

      std::set< std::pair<unsigned, unsigned> > addedEdges;

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
      std::set< std::pair<unsigned, unsigned> > addedEdges;

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

void Primitive::AddHitEdge(vector<HitObject*> &pvho, std::set< std::pair<unsigned, unsigned> >& addedEdges, const unsigned i, const unsigned j, const Vertex3Ds &vi, const Vertex3Ds &vj)
{
   // create pair uniquely identifying the edge (i,j)
   std::pair<unsigned, unsigned> p(std::min(i, j), std::max(i, j));

   if (addedEdges.count(p) == 0)   // edge not yet added?
   {
      addedEdges.insert(p);
      SetupHitObject(pvho, new HitLine3D(vi, vj));
   }
}

//
// end of license:GPLv3+, back to 'old MAME'-like
//

void Primitive::SetupHitObject(vector<HitObject*> &pvho, HitObject * obj)
{
   const Material * const mat = m_ptable->GetMaterial(m_d.m_szPhysicsMaterial);
   if (!m_d.m_useAsPlayfield)
   {
       if (mat != NULL && !m_d.m_overwritePhysics)
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
   }
   else
   {
       obj->m_elasticity = m_ptable->m_elasticity;
       obj->m_elasticityFalloff = m_ptable->m_elasticityFalloff;
       obj->SetFriction(m_ptable->m_friction);
       obj->m_scatter = ANGTORAD(m_ptable->m_scatter);
       obj->m_enabled = true;
   }
   obj->m_threshold = m_d.m_threshold;
   obj->m_ObjType = ePrimitive;
   obj->m_obj = (IFireEvents *)this;
   obj->m_e = true;
   obj->m_fe = m_d.m_hitEvent;

   pvho.push_back(obj);
   m_vhoCollidable.push_back(obj); // remember hit components of primitive
}

void Primitive::EndPlay()
{
   m_vhoCollidable.clear();

   if (m_vertexBuffer)
   {
      m_vertexBuffer->release();
      m_vertexBuffer = 0;
      m_vertexBufferRegenerate = true;
   }
   if (m_indexBuffer)
   {
      m_indexBuffer->release();
      m_indexBuffer = 0;
   }
   m_d.m_skipRendering = false;
   m_d.m_groupdRendering = false;

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
   Tmatrix.SetTranslation(m_d.m_vPosition.x, m_d.m_vPosition.y, m_d.m_vPosition.z + m_ptable->m_tableheight);

   // translation + rotation matrix
   Matrix3D RTmatrix;
   RTmatrix.SetTranslation(m_d.m_aRotAndTra[3], m_d.m_aRotAndTra[4], m_d.m_aRotAndTra[5]);

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

   m_fullMatrix = Smatrix;
   RTmatrix.Multiply(m_fullMatrix, m_fullMatrix);
   Tmatrix.Multiply(m_fullMatrix, m_fullMatrix);        // m_fullMatrix = Smatrix * RTmatrix * Tmatrix
   Smatrix.SetScaling(1.0f, 1.0f, m_ptable->m_BG_scalez[m_ptable->m_BG_current_set]);
   Smatrix.Multiply(m_fullMatrix, m_fullMatrix);
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
               std::vector<Vertex2D> drawVertices(numPts);

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
         std::vector<Vertex2D> drawVertices;
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

         if (drawVertices.size() > 0)
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
            std::vector<RenderVertex> vvertex;
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
            std::vector<Vertex2D> drawVertices(numPts);

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
      std::vector<Vertex2D> drawVertices;
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

      if (drawVertices.size() > 0)
         psur->Lines(&drawVertices[0], (int)(drawVertices.size() / 2));
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
       m_vpinball->SetStatusBarUnitInfo("", false);

}

void Primitive::ExportMesh(FILE *f)
{
   if (m_d.m_visible)
   {
      char name[sizeof(m_wzName)/sizeof(m_wzName[0])];
      WideCharToMultiByteNull(CP_ACP, 0, m_wzName, -1, name, sizeof(name), NULL, NULL);
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
      WaveFrontObj_WriteObjectName(f, name);
      WaveFrontObj_WriteVertexInfo(f, buf, (unsigned int)m_mesh.NumVertices());
      const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);
      WaveFrontObj_WriteMaterial(m_d.m_szMaterial, string(), mat);
      WaveFrontObj_UseTexture(f, m_d.m_szMaterial);
      WaveFrontObj_WriteFaceInfoLong(f, m_mesh.m_indices);
      WaveFrontObj_UpdateFaceOffset((unsigned int)m_mesh.NumVertices());
      delete[] buf;
   }
}

void Primitive::RenderObject()
{
   if (!m_d.m_groupdRendering)
   {
      RecalculateMatrices();

      if (m_vertexBufferRegenerate)
      {
         m_mesh.UploadToVB(m_vertexBuffer, m_currentFrame);
         if (m_currentFrame != -1.0f && m_doAnimation)
         {
            m_currentFrame += m_speed;
            if (m_currentFrame >= (float)m_mesh.m_animationFrames.size())
            {
               if (m_endless)
                  m_currentFrame = 0.0f;
               else
               {
                  m_currentFrame = (float)(m_mesh.m_animationFrames.size() - 1);
                  m_doAnimation = false;
                  m_vertexBufferRegenerate = false;
               }
            }
         }
         else
            m_vertexBufferRegenerate = false;
      }
   }
   else
      m_fullMatrix.SetIdentity();

   RenderDevice * const pd3dDevice = g_pplayer->m_pin3d.m_pd3dPrimaryDevice;

   if (!m_d.m_useAsPlayfield)
   {
      const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);
      pd3dDevice->basicShader->SetMaterial(mat);

      pd3dDevice->SetRenderState(RenderDevice::DEPTHBIAS, 0);
      pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, RenderDevice::RS_TRUE);
      pd3dDevice->SetRenderState(RenderDevice::CULLMODE, m_d.m_backfacesEnabled && mat->m_bOpacityActive ? RenderDevice::CULL_CW : RenderDevice::CULL_CCW);

      if (m_d.m_disableLightingTop != 0.f || m_d.m_disableLightingBelow != 0.f)
      {
         const vec4 tmp(m_d.m_disableLightingTop, m_d.m_disableLightingBelow, 0.f, 0.f);
         pd3dDevice->basicShader->SetDisableLighting(tmp);
      }

      Texture * const pin = m_ptable->GetImage(m_d.m_szImage);
      Texture * const nMap = m_ptable->GetImage(m_d.m_szNormalMap);

      if (pin && nMap)
      {
         pd3dDevice->basicShader->SetTechnique(mat->m_bIsMetal ? "basic_with_texture_normal_isMetal" : "basic_with_texture_normal_isNotMetal");
         pd3dDevice->basicShader->SetTexture("Texture0", pin, false);
         pd3dDevice->basicShader->SetTexture("Texture4", nMap, true);
         pd3dDevice->basicShader->SetAlphaTestValue(pin->m_alphaTestValue * (float)(1.0 / 255.0));
         pd3dDevice->basicShader->SetBool("objectSpaceNormalMap", m_d.m_objectSpaceNormalMap);

         //g_pplayer->m_pin3d.SetPrimaryTextureFilter(0, TEXTURE_MODE_TRILINEAR);
         // accommodate models with UV coords outside of [0,1]
         pd3dDevice->SetTextureAddressMode(0, RenderDevice::TEX_WRAP);
      }
      else if (pin)
      {
         pd3dDevice->basicShader->SetTechnique(mat->m_bIsMetal ? "basic_with_texture_isMetal" : "basic_with_texture_isNotMetal");
         pd3dDevice->basicShader->SetTexture("Texture0", pin, false);
         pd3dDevice->basicShader->SetAlphaTestValue(pin->m_alphaTestValue * (float)(1.0 / 255.0));

         //g_pplayer->m_pin3d.SetPrimaryTextureFilter(0, TEXTURE_MODE_TRILINEAR);
         // accommodate models with UV coords outside of [0,1]
         pd3dDevice->SetTextureAddressMode(0, RenderDevice::TEX_WRAP);
      }
      else
         pd3dDevice->basicShader->SetTechnique(mat->m_bIsMetal ? "basic_without_texture_isMetal" : "basic_without_texture_isNotMetal");

      // set transform
      g_pplayer->UpdateBasicShaderMatrix(m_fullMatrix);

      // draw the mesh
      pd3dDevice->basicShader->Begin(0);
      if (m_d.m_groupdRendering)
         pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, m_vertexBuffer, 0, m_numGroupVertices, m_indexBuffer, 0, m_numGroupIndices);
      else
         pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, m_vertexBuffer, 0, (DWORD)m_mesh.NumVertices(), m_indexBuffer, 0, (DWORD)m_mesh.NumIndices());
      pd3dDevice->basicShader->End();

      if (m_d.m_backfacesEnabled && mat->m_bOpacityActive)
      {
         pd3dDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_CCW);
         pd3dDevice->basicShader->Begin(0);
         if (m_d.m_groupdRendering)
            pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, m_vertexBuffer, 0, m_numGroupVertices, m_indexBuffer, 0, m_numGroupIndices);
         else
            pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, m_vertexBuffer, 0, (DWORD)m_mesh.NumVertices(), m_indexBuffer, 0, (DWORD)m_mesh.NumIndices());
         pd3dDevice->basicShader->End();
      }

      // reset transform
      g_pplayer->UpdateBasicShaderMatrix();

      pd3dDevice->SetTextureAddressMode(0, RenderDevice::TEX_CLAMP);
      //g_pplayer->m_pin3d.DisableAlphaBlend(); //!! not necessary anymore
      if (m_d.m_disableLightingTop != 0.f || m_d.m_disableLightingBelow != 0.f)
      {
         const vec4 tmp(0.f, 0.f, 0.f, 0.f);
         pd3dDevice->basicShader->SetDisableLighting(tmp);
      }
   }
   else // m_d.m_useAsPlayfield == true:
   {
      // shader is already fully configured in the playfield rendering case when we arrive here, so we only setup some special primitive params

      if (m_d.m_disableLightingTop != 0.f || m_d.m_disableLightingBelow != 0.f)
      {
         const vec4 tmp(m_d.m_disableLightingTop, m_d.m_disableLightingBelow, 0.f, 0.f);
         pd3dDevice->basicShader->SetDisableLighting(tmp);
      }

      //pd3dDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_CCW); // don't mess with the render states when doing playfield rendering
      // set transform
      g_pplayer->UpdateBasicShaderMatrix(m_fullMatrix);
      pd3dDevice->basicShader->Begin(0);
      pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, m_vertexBuffer, 0, (DWORD)m_mesh.NumVertices(), m_indexBuffer, 0, (DWORD)m_mesh.NumIndices());
      pd3dDevice->basicShader->End();
      // reset transform
      g_pplayer->UpdateBasicShaderMatrix();

      if (m_d.m_disableLightingTop != 0.f || m_d.m_disableLightingBelow != 0.f)
      {
         const vec4 tmp(0.f, 0.f, 0.f, 0.f);
         pd3dDevice->basicShader->SetDisableLighting(tmp);
      }
   }
}

// Always called each frame to render over everything else (along with alpha ramps)
void Primitive::RenderDynamic()
{
   TRACE_FUNCTION();

   if (m_d.m_staticRendering || !m_d.m_visible || m_d.m_skipRendering)
      return;
   if (m_ptable->m_reflectionEnabled && !m_d.m_reflectionEnabled)
      return;

   RenderObject();
}

void Primitive::RenderSetup()
{
   if (m_d.m_groupdRendering || m_d.m_skipRendering)
      return;

   m_currentFrame = -1.f;

   if (m_vertexBuffer)
      m_vertexBuffer->release();

   RenderDevice * const pd3dDevice = g_pplayer->m_pin3d.m_pd3dPrimaryDevice;

   pd3dDevice->CreateVertexBuffer((unsigned int)m_mesh.NumVertices(), 0, MY_D3DFVF_NOTEX2_VERTEX, &m_vertexBuffer);

   if (m_indexBuffer)
      m_indexBuffer->release();
   m_indexBuffer = pd3dDevice->CreateAndFillIndexBuffer(m_mesh.m_indices);
}

void Primitive::RenderStatic()
{
   if (m_d.m_staticRendering && m_d.m_visible)
   {
      if (m_ptable->m_reflectionEnabled && !m_d.m_reflectionEnabled)
         return;

      RenderObject();
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


HRESULT Primitive::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, const bool backupForPlay)
{
   BiffWriter bw(pstm, hcrypthash);

   /*
    * Someone decided that it was a good idea to write these vectors including
    * the fourth padding float that they used to have, so now we have to write
    * them padded to 4 floats to maintain compatibility.
    */
   bw.WriteVector3Padded(FID(VPOS), &m_d.m_vPosition);
   bw.WriteVector3Padded(FID(VSIZ), &m_d.m_vSize);
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
   const int tmp = quantizeUnsigned<8>(clamp(m_d.m_disableLightingTop, 0.f, 1.f));
   bw.WriteInt(FID(DILI), (tmp == 1) ? 0 : tmp); // backwards compatible saving
   bw.WriteFloat(FID(DILB), m_d.m_disableLightingBelow);
   bw.WriteBool(FID(REEN), m_d.m_reflectionEnabled);
   bw.WriteBool(FID(EBFC), m_d.m_backfacesEnabled);
   bw.WriteString(FID(MAPH), m_d.m_szPhysicsMaterial);
   bw.WriteBool(FID(OVPH), m_d.m_overwritePhysics);
   bw.WriteBool(FID(DIPT), m_d.m_displayTexture);
   bw.WriteBool(FID(OSNM), m_d.m_objectSpaceNormalMap);

   // No need to backup the meshes for play as the script cannot change them 
   if (m_d.m_use3DMesh && !backupForPlay)
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
         std::vector<WORD> tmp(m_mesh.NumIndices());
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
      
      if (m_mesh.m_animationFrames.size() > 0)
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

   ISelect::SaveData(pstm, hcrypthash);

   bw.WriteTag(FID(ENDB));

   return S_OK;
}

HRESULT Primitive::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   SetDefaults(false);

   BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);

   m_ptable = ptable;

   br.Load();

   if(version < 1011) // so that old tables do the reorderForsyth on each load, new tables only on mesh import, so a simple resave of a old table will also skip this step
   {
      WaitForMeshDecompression(); //!! needed nowadays due to multithreaded mesh decompression

      unsigned int* const tmp = reorderForsyth(m_mesh.m_indices, (int)m_mesh.NumVertices());
      if (tmp != NULL)
      {
         memcpy(m_mesh.m_indices.data(), tmp, m_mesh.NumIndices() * sizeof(unsigned int));
         delete[] tmp;
      }
   }

   return S_OK;
}

bool Primitive::LoadToken(const int id, BiffReader * const pbr)
{
   switch(id)
   {
   case FID(PIID): pbr->GetInt((int *)pbr->m_pdata); break;
   case FID(VPOS): pbr->GetVector3Padded(&m_d.m_vPosition); break;
   case FID(VSIZ): pbr->GetVector3Padded(&m_d.m_vSize); break;
   case FID(RTV0): pbr->GetFloat(&m_d.m_aRotAndTra[0]); break;
   case FID(RTV1): pbr->GetFloat(&m_d.m_aRotAndTra[1]); break;
   case FID(RTV2): pbr->GetFloat(&m_d.m_aRotAndTra[2]); break;
   case FID(RTV3): pbr->GetFloat(&m_d.m_aRotAndTra[3]); break;
   case FID(RTV4): pbr->GetFloat(&m_d.m_aRotAndTra[4]); break;
   case FID(RTV5): pbr->GetFloat(&m_d.m_aRotAndTra[5]); break;
   case FID(RTV6): pbr->GetFloat(&m_d.m_aRotAndTra[6]); break;
   case FID(RTV7): pbr->GetFloat(&m_d.m_aRotAndTra[7]); break;
   case FID(RTV8): pbr->GetFloat(&m_d.m_aRotAndTra[8]); break;
   case FID(IMAG): pbr->GetString(m_d.m_szImage); break;
   case FID(NRMA): pbr->GetString(m_d.m_szNormalMap); break;
   case FID(SIDS): pbr->GetInt(&m_d.m_Sides); break;
   case FID(NAME): pbr->GetWideString(m_wzName); break;
   case FID(MATR): pbr->GetString(m_d.m_szMaterial); break;
   case FID(SCOL): pbr->GetInt(&m_d.m_SideColor); break;
   case FID(TVIS): pbr->GetBool(&m_d.m_visible); break;
   case FID(REEN): pbr->GetBool(&m_d.m_reflectionEnabled); break;
   case FID(DTXI): pbr->GetBool(&m_d.m_drawTexturesInside); break;
   case FID(HTEV): pbr->GetBool(&m_d.m_hitEvent); break;
   case FID(THRS): pbr->GetFloat(&m_d.m_threshold); break;
   case FID(ELAS): pbr->GetFloat(&m_d.m_elasticity); break;
   case FID(ELFO): pbr->GetFloat(&m_d.m_elasticityFalloff); break;
   case FID(RFCT): pbr->GetFloat(&m_d.m_friction); break;
   case FID(RSCT): pbr->GetFloat(&m_d.m_scatter); break;
   case FID(EFUI): pbr->GetFloat(&m_d.m_edgeFactorUI); break;
   case FID(CORF): pbr->GetFloat(&m_d.m_collision_reductionFactor); break;
   case FID(CLDR): pbr->GetBool(&m_d.m_collidable); break;
   case FID(ISTO): pbr->GetBool(&m_d.m_toy); break;
   case FID(MAPH): pbr->GetString(m_d.m_szPhysicsMaterial); break;
   case FID(OVPH): pbr->GetBool(&m_d.m_overwritePhysics); break;
   case FID(STRE): pbr->GetBool(&m_d.m_staticRendering); break;
   case FID(DILI):
   {
      int tmp;
      pbr->GetInt(&tmp);
      m_d.m_disableLightingTop = (tmp == 1) ? 1.f : dequantizeUnsigned<8>(tmp); // backwards compatible hacky loading!
      break;
   }
   case FID(DILB): pbr->GetFloat(&m_d.m_disableLightingBelow); break;
   case FID(U3DM): pbr->GetBool(&m_d.m_use3DMesh); break;
   case FID(EBFC): pbr->GetBool(&m_d.m_backfacesEnabled); break;
   case FID(DIPT): pbr->GetBool(&m_d.m_displayTexture); break;
   case FID(M3DN): pbr->GetString(m_d.m_meshFileName); break;
   case FID(M3VN):
   {
      pbr->GetInt(&m_numVertices);
      if (m_mesh.m_animationFrames.size() > 0)
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
   case FID(M3AY): pbr->GetInt(&m_compressedAnimationVertices); break;
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
      {
         char err[128];
         sprintf_s(err, "Could not uncompress primitive animation vertex data, error %d", error);
         ShowError(err);
      }
      free(c);
      m_mesh.m_animationFrames.push_back(frameData);
      break;
   }
   case FID(M3CY): pbr->GetInt(&m_compressedVertices); break;
   case FID(M3CX):
   {
      m_mesh.m_vertices.clear();
      m_mesh.m_vertices.resize(m_numVertices);
      /*LZWReader lzwreader(pbr->m_pistream, (int *)m_mesh.m_vertices.data(), sizeof(Vertex3D_NoTex2)*numVertices, 1, sizeof(Vertex3D_NoTex2)*numVertices);
       lzwreader.Decoder();*/
      mz_ulong uclen = (mz_ulong)(sizeof(Vertex3D_NoTex2)*m_mesh.NumVertices());
      mz_uint8 * c = (mz_uint8 *)malloc(m_compressedVertices);
      pbr->GetStruct(c, m_compressedVertices);
	  if (g_pPrimitiveDecompressThreadPool == NULL)
		  g_pPrimitiveDecompressThreadPool = new ThreadPool(logicalNumberOfProcessors);

	  g_pPrimitiveDecompressThreadPool->enqueue([uclen, c, this] {
		  mz_ulong uclen2 = uclen;
		  const int error = uncompress((unsigned char *)m_mesh.m_vertices.data(), &uclen2, c, m_compressedVertices);
		  if (error != Z_OK)
		  {
			  char err[128];
			  sprintf_s(err, "Could not uncompress primitive vertex data, error %d", error);
			  ShowError(err);
		  }
		  free(c);
	  });
      break;
   }
#endif
   case FID(M3FN): pbr->GetInt(&m_numIndices); break;
   case FID(M3DI):
   {
      m_mesh.m_indices.resize(m_numIndices);
      if (m_numVertices > 65535)
         pbr->GetStruct(m_mesh.m_indices.data(), (int)sizeof(unsigned int)*m_numIndices);
      else
      {
         std::vector<WORD> tmp(m_numIndices);
         pbr->GetStruct(tmp.data(), (int)sizeof(WORD)*m_numIndices);
         for (int i = 0; i < m_numIndices; ++i)
            m_mesh.m_indices[i] = tmp[i];
      }
      break;
   }
#ifdef COMPRESS_MESHES
   case FID(M3CJ): pbr->GetInt(&m_compressedIndices); break;
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
		 if (g_pPrimitiveDecompressThreadPool == NULL)
			 g_pPrimitiveDecompressThreadPool = new ThreadPool(logicalNumberOfProcessors);

		 g_pPrimitiveDecompressThreadPool->enqueue([uclen, c, this] {
			 mz_ulong uclen2 = uclen;
			 const int error = uncompress((unsigned char *)m_mesh.m_indices.data(), &uclen2, c, m_compressedIndices);
			 if (error != Z_OK)
			 {
				 char err[128];
				 sprintf_s(err, "Could not uncompress (large) primitive index data, error %d", error);
				 ShowError(err);
			 }
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
         if (g_pPrimitiveDecompressThreadPool == NULL)
            g_pPrimitiveDecompressThreadPool = new ThreadPool(logicalNumberOfProcessors);

         g_pPrimitiveDecompressThreadPool->enqueue([uclen, c, this] {
            std::vector<WORD> tmp(m_numIndices);

            mz_ulong uclen2 = uclen;
            const int error = uncompress((unsigned char *)tmp.data(), &uclen2, c, m_compressedIndices);
            if (error != Z_OK)
            {
               char err[128];
               sprintf_s(err, "Could not uncompress (small) primitive index data, error %d", error);
               ShowError(err);
            }
            free(c);
            for (int i = 0; i < m_numIndices; ++i)
               m_mesh.m_indices[i] = tmp[i];
         });
      }
      break;
   }
#endif
   case FID(PIDB): pbr->GetFloat(&m_d.m_depthBias); break;
   case FID(OSNM): pbr->GetBool(&m_d.m_objectSpaceNormalMap); break;
   default: ISelect::LoadToken(id, pbr); break;
   }
   return true;
}

void Primitive::WaitForMeshDecompression()
{
   if (g_pPrimitiveDecompressThreadPool)
   {
      // This will wait for the threads to finish decompressing meshes.
      delete g_pPrimitiveDecompressThreadPool;
      g_pPrimitiveDecompressThreadPool = NULL;
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
   static Primitive *prim = NULL;
   switch (uMsg)
   {
   case WM_INITDIALOG:
   {
      char nullstring[8] = { 0 };

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
      prim = NULL;
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
            if (szFileName[0] == 0)
            {
               ShowError("No .obj file selected!");
               break;
            }
            prim->m_mesh.Clear();
            prim->m_d.m_use3DMesh = false;
            if (prim->m_vertexBuffer)
            {
               prim->m_vertexBuffer->release();
               prim->m_vertexBuffer = 0;
            }
            bool flipTV = false;
            const bool convertToLeftHanded = IsDlgButtonChecked(hwndDlg, IDC_CONVERT_COORD_CHECK) == BST_CHECKED;
            const bool importAbsolutePosition = IsDlgButtonChecked(hwndDlg, IDC_ABS_POSITION_RADIO) == BST_CHECKED;
            const bool centerMesh = IsDlgButtonChecked(hwndDlg, IDC_CENTER_MESH) == BST_CHECKED;
            const bool importMaterial = IsDlgButtonChecked(hwndDlg, IDC_IMPORT_MATERIAL) == BST_CHECKED;
            const bool importAnimation = IsDlgButtonChecked(hwndDlg, IDC_IMPORT_ANIM_SEQUENCE) == BST_CHECKED;
            const bool doForsyth = IsDlgButtonChecked(hwndDlg, IDC_IMPORT_NO_FORSYTH) == BST_UNCHECKED;
            if (importMaterial)
            {
               string szMatName = szFileName;
               if (ReplaceExtensionFromFilename(szMatName, "mtl"))
               {
                  Material * const mat = new Material();
                  if (WaveFrontObj_LoadMaterial(szMatName, mat))
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
                   if (tmp != NULL)
                   {
                       memcpy(prim->m_mesh.m_indices.data(), tmp, prim->m_mesh.NumIndices() * sizeof(unsigned int));
                       delete[] tmp;
                   }
               }
               prim->UpdateStatusBarInfo();
               prim = NULL;
               EndDialog(hwndDlg, TRUE);
            }
            else
               ShowError("Unable to open file!");
            break;
         }
         case IDC_BROWSE_BUTTON:
         {
            if (prim == NULL)
               break;

            SetForegroundWindow(hwndDlg);

            std::vector<std::string> szFileName;
            char szInitialDir[MAXSTRING];

            HRESULT hr = LoadValueString("RecentDir", "ImportDir", szInitialDir, MAXSTRING);
            if (hr != S_OK)
               lstrcpy(szInitialDir, "c:\\Visual Pinball\\Tables\\");

            if (g_pvp->OpenFileDialog(szInitialDir, szFileName, "Wavefront obj file (*.obj)\0*.obj\0", "obj", 0))
            {
               SetDlgItemText(hwndDlg, IDC_FILENAME_EDIT, szFileName[0].c_str());

               size_t index = szFileName[0].find_last_of('\\');
               if (index != std::string::npos)
               {
                  const std::string newInitDir(szFileName[0].substr(0, index));
                  hr = SaveValueString("RecentDir", "ImportDir", newInitDir);
                  index++;
                  prim->m_d.m_meshFileName = szFileName[0].substr(index, szFileName[0].length() - index);
               }

               EnableWindow(GetDlgItem(hwndDlg, IDOK), TRUE);
            }
            break;
         }
         case IDCANCEL:
         {
            prim = NULL;
            EndDialog(hwndDlg, FALSE);
            break;
         }
         }
      }
   }
   return FALSE;
}

bool Primitive::BrowseFor3DMeshFile()
{
   DialogBoxParam(m_vpinball->theInstance, MAKEINTRESOURCE(IDD_MESH_IMPORT_DIALOG), m_vpinball->GetHwnd(), ObjImportProc, (size_t)this);
#if 1
   return false;
#else
   char szFileName[MAXSTRING];
   char szInitialDir[MAXSTRING];
   szFileName[0] = '\0';

   OPENFILENAME ofn;
   ZeroMemory(&ofn, sizeof(OPENFILENAME));
   ofn.lStructSize = sizeof(OPENFILENAME);
   ofn.hInstance = m_vpinball->theInstance;
   ofn.hwndOwner = m_vpinball->m_hwnd;
   // TEXT
   ofn.lpstrFilter = "Wavefront obj file (*.obj)\0*.obj\0";
   ofn.lpstrFile = szFileName;
   ofn.nMaxFile = sizeof(szFileName);
   ofn.lpstrDefExt = "obj";
   ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

   const HRESULT hr = LoadValueString("RecentDir", "ImportDir", szInitialDir, MAXSTRING);
   if (hr != S_OK)
       lstrcpy(szInitialDir, "c:\\Visual Pinball\\Tables\\");

   ofn.lpstrInitialDir = szInitialDir;

   const int ret = GetOpenFileName(&ofn);
   if (ret == 0)
       return false;

   string filename(ofn.lpstrFile);
   size_t index = filename.find_last_of('\\');
   if (index != std::string::npos)
   {
      const std::string newInitDir(szFilename.substr(0, index));
      SaveValueString("RecentDir", "ImportDir", newInitDir);
      index++;
      m_d.m_meshFileName = filename.substr(index, filename.length() - index);
   }

   m_mesh.Clear();
   m_d.m_use3DMesh = false;
   if (vertexBuffer)
   {
      vertexBuffer->release();
      vertexBuffer = 0;
   }
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
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, szImage, MAXTOKEN, NULL, NULL);
   const Texture * const tex = m_ptable->GetImage(szImage);
   if (tex && tex->IsHDR())
   {
       ShowError("Cannot use a HDR image (.exr/.hdr) here");
       return E_FAIL;
   }
   m_d.m_szImage = szImage;

   return S_OK;
}

STDMETHODIMP Primitive::get_NormalMap(BSTR *pVal)
{
    WCHAR wz[MAXTOKEN];
    MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szNormalMap, -1, wz, MAXTOKEN);
    *pVal = SysAllocString(wz);

    return S_OK;
}

STDMETHODIMP Primitive::put_NormalMap(BSTR newVal)
{
    char szImage[sizeof(m_d.m_szNormalMap)];
    WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, szImage, sizeof(m_d.m_szNormalMap), NULL, NULL);
    const Texture * const tex = m_ptable->GetImage(szImage);
    if (tex && tex->IsHDR())
    {
        ShowError("Cannot use a HDR image (.exr/.hdr) here");
        return E_FAIL;
    }

    strncpy_s(m_d.m_szNormalMap, szImage, sizeof(m_d.m_szNormalMap)-1);

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
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXSTRING, NULL, NULL);
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
   char szInitialDir[MAXSTRING];
   HRESULT hr = LoadValueString("RecentDir", "ImportDir", szInitialDir, MAXSTRING);
   if (hr != S_OK)
       lstrcpy(szInitialDir, "c:\\Visual Pinball\\Tables\\");

   std::vector<std::string> szFileName;
   
   if (m_vpinball->SaveFileDialog(szInitialDir, szFileName, "Wavefront obj file (*.obj)\0*.obj\0", "obj", OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY))
   {
      const size_t index = szFileName[0].find_last_of('\\');
      if (index != std::string::npos)
      {
         const std::string newInitDir(szFileName[0].substr(0, index));
         hr = SaveValueString("RecentDir", "ImportDir", newInitDir);
      }

      char name[sizeof(m_wzName) / sizeof(m_wzName[0])];
      WideCharToMultiByteNull(CP_ACP, 0, m_wzName, -1, name, sizeof(name), NULL, NULL);
      m_mesh.SaveWavefrontObj(szFileName[0], m_d.m_use3DMesh ? name : "Primitive");
   }

}

bool Primitive::IsTransparent() const
{
   if (m_d.m_skipRendering)
      return false;

   return m_ptable->GetMaterial(m_d.m_szMaterial)->m_bOpacityActive;
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
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXNAMEBUFFER, NULL, NULL);
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
   if (m_d.m_vPosition.x != newVal)
   {
      m_d.m_vPosition.x = newVal;
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
   if (m_d.m_vPosition.y != newVal)
   {
      m_d.m_vPosition.y = newVal;
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
   if (m_d.m_vPosition.z != newVal)
   {
      m_d.m_vPosition.z = newVal;
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
   if (m_d.m_vSize.x != newVal)
   {
      m_d.m_vSize.x = newVal;
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
   if (m_d.m_vSize.y != newVal)
   {
      m_d.m_vSize.y = newVal;
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
   if (m_d.m_vSize.z != newVal)
   {
      m_d.m_vSize.z = newVal;
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
   if (m_d.m_aRotAndTra[0] != newVal)
   {
      m_d.m_aRotAndTra[0] = newVal;
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
   if (m_d.m_aRotAndTra[1] != newVal)
   {
      m_d.m_aRotAndTra[1] = newVal;
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
   if (m_d.m_aRotAndTra[2] != newVal)
   {
      m_d.m_aRotAndTra[2] = newVal;
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
   if (m_d.m_aRotAndTra[3] != newVal)
   {
      m_d.m_aRotAndTra[3] = newVal;
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
   if (m_d.m_aRotAndTra[4] != newVal)
   {
      m_d.m_aRotAndTra[4] = newVal;
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
   if (m_d.m_aRotAndTra[5] != newVal)
   {
      m_d.m_aRotAndTra[5] = newVal;
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
   if (m_d.m_aRotAndTra[6] != newVal)
   {
      m_d.m_aRotAndTra[6] = newVal;
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
   if (m_d.m_aRotAndTra[7] != newVal)
   {
      m_d.m_aRotAndTra[7] = newVal;
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
   if (m_d.m_aRotAndTra[8] != newVal)
   {
      m_d.m_aRotAndTra[8] = newVal;
   }

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
       if (m_vhoCollidable.size() > 0 && m_vhoCollidable[0]->m_enabled != val)
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
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXNAMEBUFFER, NULL, NULL);
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
   if (m_mesh.m_animationFrames.size() > 0)
   {
      if ((size_t)startFrame >= m_mesh.m_animationFrames.size())
         startFrame = 0.0f;
      //if (startFrame < 0.0f)
      //   startFrame = -startFrame;
      if (speed < 0.0f) speed = -speed;

      m_vertexBufferRegenerate = (m_currentFrame != startFrame) || (m_speed != speed) || !m_doAnimation || m_endless;

      m_currentFrame = startFrame;
      m_speed = speed;
      m_doAnimation = true;
      m_endless = false;
   }

   return S_OK;
}

STDMETHODIMP Primitive::PlayAnimEndless(float speed)
{
   if (m_mesh.m_animationFrames.size() > 0)
   {
      if (speed < 0.0f) speed = -speed;

      m_vertexBufferRegenerate = (m_currentFrame != 0.f) || (m_speed != speed) || !m_doAnimation || !m_endless;

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
   if (m_currentFrame > 0.0f)
   {
      if (speed < 0.0f) speed = -speed;

      m_vertexBufferRegenerate = (m_speed != speed) || !m_doAnimation;

      m_speed = speed;
      m_doAnimation = true;
   }

   return S_OK;
}

STDMETHODIMP Primitive::ShowFrame(float frame)
{
   if ((size_t)frame >= m_mesh.m_animationFrames.size())
      frame = (float)(m_mesh.m_animationFrames.size() - 1);

   m_vertexBufferRegenerate = (m_currentFrame != frame) || m_doAnimation;

   m_currentFrame = frame;
   m_doAnimation = false;

   return S_OK;
}

void Primitive::SetDefaultPhysics(bool fromMouseClick)
{
   static const char strKeyName[] = "DefaultProps\\Primitive";
   m_d.m_elasticity = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "Elasticity", 0.3f) : 0.3f;
   m_d.m_elasticityFalloff = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "ElasticityFalloff", 0.5f) : 0.5f;
   m_d.m_friction = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "Friction", 0.3f) : 0.3f;
   m_d.m_scatter = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "Scatter", 0) : 0;
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
