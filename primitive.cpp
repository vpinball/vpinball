// primitive.cpp: implementation of the Primitive class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h" 
#include "forsyth.h"
#include "objloader.h"
#include "inc\miniz.c"
#include "inc\progmesh.h"

// defined in objloader.cpp
extern bool WaveFrontObj_Load(const char *filename, const bool flipTv, const bool convertToLeftHanded);
extern void WaveFrontObj_GetVertices(std::vector<Vertex3D_NoTex2>& verts);
extern void WaveFrontObj_GetIndices(std::vector<unsigned int>& list);
extern void WaveFrontObj_Save(const char *filename, const char *description, const Mesh& mesh);
//


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
   WIN32_FIND_DATA data;
   HANDLE h;
   string name(fname);
   int frameCounter = 0;
   size_t idx = name.find_last_of("_");
   std::vector<string> allFiles;
   if (idx == string::npos)
   {
      ShowError("Can't find sequence of obj files! The file name of the sequence must be <meshname>_x.obj where x is the frame number!");
      return false;
   }
   idx++;
   name = name.substr(0,idx);
   string sname = name + "*.obj";
   h = FindFirstFile(sname.c_str(), &data);
   if (h != INVALID_HANDLE_VALUE)
   {
      do
      {
         allFiles.push_back(string(data.cFileName));
         frameCounter++;
      } while (FindNextFile(h, &data));
   }
   m_animationFrames.resize(frameCounter);
   for (size_t i = 0; i < allFiles.size(); i++)
   {
      sname = allFiles[i];
      if (WaveFrontObj_Load(sname.c_str(), flipTV, convertToLeftHanded))
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
         ShowError((char*)name.c_str());
         return false;
      }

   }
   sname = to_string((long long)frameCounter)+" frames imported!";
   MessageBox(NULL, sname.c_str(), "Info", MB_OK | MB_ICONEXCLAMATION);
   return true;
}

bool Mesh::LoadWavefrontObj(const char *fname, const bool flipTV, const bool convertToLeftHanded)
{
   Clear();

   if (WaveFrontObj_Load(fname, flipTV, convertToLeftHanded))
   {
      WaveFrontObj_GetVertices(m_vertices);
      WaveFrontObj_GetIndices(m_indices);
      float maxX = FLT_MIN, minX = FLT_MAX;
      float maxY = FLT_MIN, minY = FLT_MAX;
      float maxZ = FLT_MIN, minZ = FLT_MAX;

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

void Mesh::SaveWavefrontObj(const char *fname, const char *description)
{
   if (description == NULL)
      description = fname;

   WaveFrontObj_Save(fname, description, *this);
}

void Mesh::UploadToVB(VertexBuffer * vb, float frame) 
{
   float intPart;
   const float fractpart = modf(frame, &intPart);
   const int iFrame = (int)intPart;

   if (frame != -1)
   {
      for (size_t i = 0; i < m_vertices.size(); i++)
      {
         m_vertices[i].x = m_animationFrames[iFrame].m_frameVerts[i].x;
         m_vertices[i].y = m_animationFrames[iFrame].m_frameVerts[i].y;
         m_vertices[i].z = m_animationFrames[iFrame].m_frameVerts[i].z;
         m_vertices[i].nx = m_animationFrames[iFrame].m_frameVerts[i].nx;
         m_vertices[i].ny = m_animationFrames[iFrame].m_frameVerts[i].ny;
         m_vertices[i].nz = m_animationFrames[iFrame].m_frameVerts[i].nz;

         if (iFrame + 1 < (int)m_animationFrames.size())
         {
            m_vertices[i].x += (m_animationFrames[iFrame + 1].m_frameVerts[i].x - m_vertices[i].x)*fractpart;
            m_vertices[i].y += (m_animationFrames[iFrame + 1].m_frameVerts[i].y - m_vertices[i].y)*fractpart;
            m_vertices[i].z += (m_animationFrames[iFrame + 1].m_frameVerts[i].z - m_vertices[i].z)*fractpart;
            m_vertices[i].nx += (m_animationFrames[iFrame + 1].m_frameVerts[i].nx - m_vertices[i].nx)*fractpart;
            m_vertices[i].ny += (m_animationFrames[iFrame + 1].m_frameVerts[i].ny - m_vertices[i].ny)*fractpart;
            m_vertices[i].nz += (m_animationFrames[iFrame + 1].m_frameVerts[i].nz - m_vertices[i].nz)*fractpart;
         }
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
   vertexBuffer = 0;
   vertexBufferRegenerate = true;
   indexBuffer = 0;
   m_d.m_use3DMesh = false;
   m_d.m_meshFileName[0] = 0;
   m_d.m_staticRendering = false;
   m_d.m_edgeFactorUI = 0.25f;
   m_d.m_collision_reductionFactor = 0.f;
   m_d.m_depthBias = 0.0f;
   m_d.m_fSkipRendering = false;
   m_d.m_fGroupdRendering = false;
   m_d.m_fReflectionEnabled = true;
   m_numGroupIndices = 0;
   m_numGroupVertices = 0;

   numIndices = 0;
   numVertices = 0;
   m_propPhysics = NULL;
   m_propPosition = NULL;
   m_propVisual = NULL;
   memset(m_d.m_szImage, 0, MAXTOKEN);
   memset(m_d.m_szNormalMap, 0, MAXTOKEN);
   memset( m_d.m_szMaterial, 0, 32 );
   memset( m_d.m_szPhysicsMaterial, 0, 32 );
   m_d.m_fOverwritePhysics=true;
}

Primitive::~Primitive()
{
   if (vertexBuffer)
      vertexBuffer->release();
   if (indexBuffer)
      indexBuffer->release();
}

void Primitive::CreateRenderGroup(Collection *collection, RenderDevice *pd3dDevice)
{
   if (!collection->m_fGroupElements)
      return;

   unsigned int overall_size = 0;
   vector<Primitive*> prims;
   vector<Primitive*> renderedPrims;
   for (int i = 0; i < collection->m_visel.size(); i++)
   {
      ISelect *pisel = collection->m_visel.ElementAt(i);
      if (pisel->GetItemType() != eItemPrimitive)
         continue;

      Primitive *prim = (Primitive*)pisel;
      // only support dynamic mesh primitives for now
      if (!prim->m_d.m_use3DMesh || prim->m_d.m_staticRendering)
         continue;

      prims.push_back(prim);

      overall_size += (unsigned int)prim->m_mesh.NumIndices();
   }

   if (prims.size() == 0)
      return;

   // The first primitive in the group is the base primitive
   // this element gets rendered by rendering all other group primitives
   // the rest of the group is marked as skipped rendering
   const Material * const groupMaterial = g_pplayer->m_ptable->GetMaterial(prims[0]->m_d.m_szMaterial);
   const Texture * const groupTexel = g_pplayer->m_ptable->GetImage(prims[0]->m_d.m_szImage);
   m_numGroupVertices = (int)prims[0]->m_mesh.NumVertices();
   m_numGroupIndices = (int)prims[0]->m_mesh.NumIndices();
   prims[0]->m_d.m_fGroupdRendering = true;
   vector<unsigned int> indices(overall_size);
   memcpy(indices.data(), prims[0]->m_mesh.m_indices.data(), prims[0]->m_mesh.NumIndices());

   for (size_t i = 1; i < prims.size(); i++)
   {
      const Material * const mat = g_pplayer->m_ptable->GetMaterial(prims[i]->m_d.m_szMaterial);
      const Texture * const texel = g_pplayer->m_ptable->GetImage(prims[i]->m_d.m_szImage);
      if (mat == groupMaterial && texel == groupTexel)
      {
         const Mesh &m = prims[i]->m_mesh;
         for (size_t k = 0; k < m.NumIndices(); k++)
            indices[m_numGroupIndices + k] = m_numGroupVertices + m.m_indices[k];

         m_numGroupVertices += (int)m.NumVertices();
         m_numGroupIndices += (int)m.NumIndices();
         prims[i]->m_d.m_fSkipRendering = true;
         renderedPrims.push_back(prims[i]);
      }
      else
         prims[i]->m_d.m_fSkipRendering = false;
   }

   if (vertexBuffer)
      vertexBuffer->release();
   pd3dDevice->CreateVertexBuffer(m_numGroupVertices, 0, MY_D3DFVF_NOTEX2_VERTEX, &vertexBuffer);

   if (indexBuffer)
      indexBuffer->release();
   indexBuffer = pd3dDevice->CreateAndFillIndexBuffer(indices);

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
         renderedPrims[i]->fullMatrix.MultiplyVector(vt, vt);

         Vertex3Ds n;
         renderedPrims[i]->fullMatrix.MultiplyVectorNoTranslateNormal(vt, n);
         vt.nx = n.x; vt.ny = n.y; vt.nz = n.z;
         buf[ofs] = vt;
         ofs++;
      }
   }
   vertexBuffer->unlock();

   prims.clear();
   renderedPrims.clear();
}

HRESULT Primitive::Init(PinTable *ptable, float x, float y, bool fromMouseClick)
{
   m_ptable = ptable;

   m_d.m_vPosition.x = x;
   m_d.m_vPosition.y = y;

   SetDefaults(false);

   InitVBA(fTrue, 0, NULL);

   if (!m_d.m_use3DMesh)
      CalculateBuiltinOriginal();

   UpdateEditorView();

   return S_OK;
}

void Primitive::SetDefaults(bool fromMouseClick)
{
   static const char strKeyName[] = "DefaultProps\\Primitive";

   HRESULT hr;

   m_d.m_use3DMesh = false;
   m_d.m_meshFileName[0] = 0;
   // sides
   m_d.m_Sides = fromMouseClick ? GetRegIntWithDefault(strKeyName, "Sides", 4) : 4;
   if (m_d.m_Sides > Max_Primitive_Sides)
      m_d.m_Sides = Max_Primitive_Sides;

   // colors
   m_d.m_SideColor = fromMouseClick ? GetRegIntWithDefault(strKeyName, "SideColor", RGB(150, 150, 150)) : RGB(150, 150, 150);

   m_d.m_fVisible = fromMouseClick ? GetRegBoolWithDefault(strKeyName, "Visible", true) : true;
   m_d.m_staticRendering = fromMouseClick ? GetRegBoolWithDefault(strKeyName, "StaticRendering", true) : true;
   m_d.m_DrawTexturesInside = fromMouseClick ? GetRegBoolWithDefault(strKeyName, "DrawTexturesInside", false) : false;

   // Position (X and Y is already set by the click of the user)
   m_d.m_vPosition.z = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "Position_Z", 0.0f) : 0.0f;

   // Size
   m_d.m_vSize.x = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "Size_X", 100.0f) : 100.0f;
   m_d.m_vSize.y = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "Size_Y", 100.0f) : 100.0f;
   m_d.m_vSize.z = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "Size_Z", 100.0f) : 100.0f;

   // Rotation and Transposition
   m_d.m_aRotAndTra[0] = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "RotAndTra0", 0.0f) : 0.0f;
   m_d.m_aRotAndTra[1] = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "RotAndTra1", 0.0f) : 0.0f;
   m_d.m_aRotAndTra[2] = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "RotAndTra2", 0.0f) : 0.0f;
   m_d.m_aRotAndTra[3] = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "RotAndTra3", 0.0f) : 0.0f;
   m_d.m_aRotAndTra[4] = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "RotAndTra4", 0.0f) : 0.0f;
   m_d.m_aRotAndTra[5] = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "RotAndTra5", 0.0f) : 0.0f;
   m_d.m_aRotAndTra[6] = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "RotAndTra6", 0.0f) : 0.0f;
   m_d.m_aRotAndTra[7] = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "RotAndTra7", 0.0f) : 0.0f;
   m_d.m_aRotAndTra[8] = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "RotAndTra8", 0.0f) : 0.0f;

   hr = GetRegString(strKeyName, "Image", m_d.m_szImage, MAXTOKEN);
   if ((hr != S_OK) && fromMouseClick)
      m_d.m_szImage[0] = 0;

   hr = GetRegString(strKeyName, "NormalMap", m_d.m_szNormalMap, MAXTOKEN);
   if ((hr != S_OK) && fromMouseClick)
       m_d.m_szNormalMap[0] = 0;

   m_d.m_threshold = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "HitThreshold", 2.0f) : 2.0f;

   SetDefaultPhysics(fromMouseClick);

   m_d.m_edgeFactorUI = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "EdgeFactorUI", 0.25f) : 0.25f;
   m_d.m_collision_reductionFactor = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "CollisionReductionFactor", 0.f) : 0.f;

   m_d.m_fCollidable = fromMouseClick ? GetRegBoolWithDefault(strKeyName, "Collidable", true) : true;
   m_d.m_fToy = fromMouseClick ? GetRegBoolWithDefault(strKeyName, "IsToy", false) : false;
   m_d.m_fDisableLighting = dequantizeUnsigned<8>(fromMouseClick ? GetRegIntWithDefault(strKeyName, "DisableLighting", 0) : 0); // stored as uchar for backward compatibility
   m_d.m_fReflectionEnabled = fromMouseClick ? GetRegBoolWithDefault(strKeyName, "ReflectionEnabled", true) : true;
   m_d.m_fBackfacesEnabled = fromMouseClick ? GetRegBoolWithDefault(strKeyName, "BackfacesEnabled", false) : false;
}

void Primitive::WriteRegDefaults()
{
   static const char strKeyName[] = "DefaultProps\\Primitive";

   SetRegValueInt(strKeyName, "SideColor", m_d.m_SideColor);
   SetRegValueBool(strKeyName, "Visible", m_d.m_fVisible);
   SetRegValueBool(strKeyName, "StaticRendering", m_d.m_staticRendering);
   SetRegValueBool(strKeyName, "DrawTexturesInside", m_d.m_DrawTexturesInside);

   SetRegValueFloat(strKeyName, "Position_Z", m_d.m_vPosition.z);

   SetRegValueFloat(strKeyName, "Size_X", m_d.m_vSize.x);
   SetRegValueFloat(strKeyName, "Size_Y", m_d.m_vSize.y);
   SetRegValueFloat(strKeyName, "Size_Z", m_d.m_vSize.z);

   SetRegValueFloat(strKeyName, "RotAndTra0", m_d.m_aRotAndTra[0]);
   SetRegValueFloat(strKeyName, "RotAndTra1", m_d.m_aRotAndTra[1]);
   SetRegValueFloat(strKeyName, "RotAndTra2", m_d.m_aRotAndTra[2]);
   SetRegValueFloat(strKeyName, "RotAndTra3", m_d.m_aRotAndTra[3]);
   SetRegValueFloat(strKeyName, "RotAndTra4", m_d.m_aRotAndTra[4]);
   SetRegValueFloat(strKeyName, "RotAndTra5", m_d.m_aRotAndTra[5]);
   SetRegValueFloat(strKeyName, "RotAndTra6", m_d.m_aRotAndTra[6]);
   SetRegValueFloat(strKeyName, "RotAndTra7", m_d.m_aRotAndTra[7]);
   SetRegValueFloat(strKeyName, "RotAndTra8", m_d.m_aRotAndTra[8]);

   SetRegValueString(strKeyName, "Image", m_d.m_szImage);
   SetRegValueString(strKeyName, "NormalMap", m_d.m_szNormalMap);
   SetRegValueBool(strKeyName, "HitEvent", m_d.m_fHitEvent);
   SetRegValueFloat(strKeyName, "HitThreshold", m_d.m_threshold);
   SetRegValueFloat(strKeyName, "Elasticity", m_d.m_elasticity);
   SetRegValueFloat(strKeyName, "ElasticityFalloff", m_d.m_elasticityFalloff);
   SetRegValueFloat(strKeyName, "Friction", m_d.m_friction);
   SetRegValueFloat(strKeyName, "Scatter", m_d.m_scatter);

   SetRegValueFloat(strKeyName, "EdgeFactorUI", m_d.m_edgeFactorUI);
   SetRegValueFloat(strKeyName, "CollisionReductionFactor", m_d.m_collision_reductionFactor);

   SetRegValueBool(strKeyName, "Collidable", m_d.m_fCollidable);
   SetRegValueBool(strKeyName, "IsToy", m_d.m_fToy);
   const int tmp = quantizeUnsigned<8>(clamp(m_d.m_fDisableLighting, 0.f, 1.f));
   SetRegValueInt(strKeyName, "DisableLighting", (tmp == 1) ? 0 : tmp); // backwards compatible saving
   SetRegValueBool(strKeyName, "ReflectionEnabled", m_d.m_fReflectionEnabled);
   SetRegValueBool(strKeyName, "BackfacesEnabled", m_d.m_fBackfacesEnabled);
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
   TransformVertices(); //!! could also only do this for the optional reduced variant!

   //

   const unsigned int reduced_vertices = max((unsigned int)powf((float)vertices.size(), clamp(1.f - m_d.m_collision_reductionFactor, 0.f, 1.f)*0.25f + 0.75f), 420u); //!! 420 = magic

   if (reduced_vertices < vertices.size())
   {
      std::vector<ProgMesh::float3> prog_vertices(vertices.size());
      for (size_t i = 0; i < vertices.size(); ++i) //!! opt. use original data directly!
      {
         prog_vertices[i].x = vertices[i].x;
         prog_vertices[i].y = vertices[i].y;
         prog_vertices[i].z = vertices[i].z;
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
      for (unsigned int i = 0; i < prog_new_indices.size(); ++i)
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
      for (unsigned i = 0; i < prog_vertices.size(); ++i)
         SetupHitObject(pvho, new HitPoint(prog_vertices[i].x, prog_vertices[i].y, prog_vertices[i].z));
   }
   else
   {
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
}

void Primitive::GetHitShapesDebug(Vector<HitObject> * const pvho)
{
}

void Primitive::AddHitEdge(Vector<HitObject> * pvho, std::set< std::pair<unsigned, unsigned> >& addedEdges, const unsigned i, const unsigned j, const Vertex3Ds &vi, const Vertex3Ds &vj)
{
   // create pair uniquely identifying the edge (i,j)
   std::pair<unsigned, unsigned> p(std::min(i, j), std::max(i, j));

   if (addedEdges.count(p) == 0)   // edge not yet added?
   {
      addedEdges.insert(p);
      SetupHitObject(pvho, new HitLine3D(vi, vj));
   }
}

void Primitive::SetupHitObject(Vector<HitObject> * pvho, HitObject * obj)
{
   Material *mat = m_ptable->GetMaterial( m_d.m_szPhysicsMaterial );
   if ( mat != NULL && !m_d.m_fOverwritePhysics )
   {
      obj->m_elasticity = mat->m_fElasticity;
      obj->m_elasticityFalloff = mat->m_fElasticityFalloff;
      obj->SetFriction( mat->m_fFriction );
      obj->m_scatter = ANGTORAD( mat->m_fScatterAngle );
   }
   else
   {
      obj->m_elasticity = m_d.m_elasticity;
      obj->m_elasticityFalloff = m_d.m_elasticityFalloff;
      obj->SetFriction( m_d.m_friction );
      obj->m_scatter = ANGTORAD( m_d.m_scatter );
   }

   obj->m_threshold = m_d.m_threshold;
   obj->m_ObjType = ePrimitive;
   obj->m_fEnabled = m_d.m_fCollidable;
   if (m_d.m_fHitEvent)
      obj->m_pfe = (IFireEvents *)this;
   obj->m_pe = this;

   pvho->AddElement(obj);
   m_vhoCollidable.push_back(obj);	//remember hit components of primitive
}

void Primitive::EndPlay()
{
   m_vhoCollidable.clear();

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
   m_d.m_fGroupdRendering = false;
}

//////////////////////////////
// Calculation
//////////////////////////////

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

   fullMatrix = Smatrix;
   RTmatrix.Multiply(fullMatrix, fullMatrix);
   Tmatrix.Multiply(fullMatrix, fullMatrix);        // fullMatrix = Smatrix * RTmatrix * Tmatrix
   Smatrix.SetScaling(1.0f, 1.0f, m_ptable->m_BG_scalez[m_ptable->m_BG_current_set]);
   Smatrix.Multiply(fullMatrix, fullMatrix);
}

// recalculate vertices for editor display
void Primitive::TransformVertices()
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
void Primitive::PreRender(Sur * const psur)
{
}

void Primitive::Render(Sur * const psur)
{
   psur->SetLineColor(RGB(0, 0, 0), false, 1);
   psur->SetObject(this);

   if ((m_d.m_edgeFactorUI <= 0.0f) || (m_d.m_edgeFactorUI >= 1.0f) || !m_d.m_use3DMesh)
   {
      if (!m_d.m_use3DMesh || (m_d.m_edgeFactorUI >= 1.0f) || (m_mesh.NumVertices() <= 100)) // small mesh: draw all triangles
      {
         for (unsigned i = 0; i < m_mesh.NumIndices(); i += 3)
         {
            const Vertex3Ds * const A = &vertices[m_mesh.m_indices[i]];
            const Vertex3Ds * const B = &vertices[m_mesh.m_indices[i + 1]];
            const Vertex3Ds * const C = &vertices[m_mesh.m_indices[i + 2]];
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

            const Vertex3Ds& A = vertices[m_mesh.m_indices[0]];
            drawVertices[0] = Vertex2D(A.x, A.y);

            unsigned int o = 1;
            for (size_t i = 0; i < m_mesh.NumIndices(); i += 3, ++o)
            {
               const Vertex3Ds& B = vertices[m_mesh.m_indices[i + 1]];
               drawVertices[o] = Vertex2D(B.x, B.y);
            }

            psur->Polyline(&drawVertices[0], (int)drawVertices.size());
         }
      }
   }
   else
   {
      std::vector<Vertex2D> drawVertices;
      for (unsigned i = 0; i < m_mesh.NumIndices(); i += 3)
      {
         const Vertex3Ds * const A = &vertices[m_mesh.m_indices[i]];
         const Vertex3Ds * const B = &vertices[m_mesh.m_indices[i + 1]];
         const Vertex3Ds * const C = &vertices[m_mesh.m_indices[i + 2]];
         const float An = normals[m_mesh.m_indices[i]];
         const float Bn = normals[m_mesh.m_indices[i + 1]];
         const float Cn = normals[m_mesh.m_indices[i + 2]];
         if (fabsf(An + Bn) < m_d.m_edgeFactorUI)
         {
            drawVertices.push_back(Vertex2D(A->x, A->y));
            drawVertices.push_back(Vertex2D(B->x, B->y));
         }
         if (fabsf(Bn + Cn) < m_d.m_edgeFactorUI)
         {
            drawVertices.push_back(Vertex2D(B->x, B->y));
            drawVertices.push_back(Vertex2D(C->x, C->y));
         }
         if (fabsf(Cn + An) < m_d.m_edgeFactorUI)
         {
            drawVertices.push_back(Vertex2D(C->x, C->y));
            drawVertices.push_back(Vertex2D(A->x, A->y));
         }
      }

      if (drawVertices.size() > 0)
         psur->Lines(&drawVertices[0], (int)(drawVertices.size() / 2));
   }

   // draw center marker
   psur->SetLineColor(RGB(128, 128, 128), false, 1);
   psur->Line(m_d.m_vPosition.x - 10.0f, m_d.m_vPosition.y, m_d.m_vPosition.x + 10.0f, m_d.m_vPosition.y);
   psur->Line(m_d.m_vPosition.x, m_d.m_vPosition.y - 10.0f, m_d.m_vPosition.x, m_d.m_vPosition.y + 10.0f);
}

void Primitive::RenderBlueprint(Sur *psur, const bool solid)
{
   if (solid)
      psur->SetFillColor(BLUEPRINT_SOLID_COLOR);
   else
      psur->SetFillColor(-1);

   psur->SetLineColor(RGB(0, 0, 0), false, 1);
   psur->SetObject(this);

   if ((m_d.m_edgeFactorUI <= 0.0f) || (m_d.m_edgeFactorUI >= 1.0f) || !m_d.m_use3DMesh)
   {
      if (!m_d.m_use3DMesh || (m_d.m_edgeFactorUI >= 1.0f) || (m_mesh.NumVertices() <= 100)) // small mesh: draw all triangles
      {
         for (unsigned i = 0; i < m_mesh.NumIndices(); i += 3)
         {
            const Vertex3Ds * const A = &vertices[m_mesh.m_indices[i]];
            const Vertex3Ds * const B = &vertices[m_mesh.m_indices[i + 1]];
            const Vertex3Ds * const C = &vertices[m_mesh.m_indices[i + 2]];
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

            const Vertex3Ds& A = vertices[m_mesh.m_indices[0]];
            drawVertices[0] = Vertex2D(A.x, A.y);

            unsigned int o = 1;
            for (size_t i = 0; i < m_mesh.NumIndices(); i += 3, ++o)
            {
               const Vertex3Ds& B = vertices[m_mesh.m_indices[i + 1]];
               drawVertices[o] = Vertex2D(B.x, B.y);
            }

            psur->Polyline(&drawVertices[0], (int)drawVertices.size());
         }
      }
   }
   else
   {
      std::vector<Vertex2D> drawVertices;
      for (unsigned i = 0; i < m_mesh.NumIndices(); i += 3)
      {
         const Vertex3Ds * const A = &vertices[m_mesh.m_indices[i]];
         const Vertex3Ds * const B = &vertices[m_mesh.m_indices[i + 1]];
         const Vertex3Ds * const C = &vertices[m_mesh.m_indices[i + 2]];
         const float An = normals[m_mesh.m_indices[i]];
         const float Bn = normals[m_mesh.m_indices[i + 1]];
         const float Cn = normals[m_mesh.m_indices[i + 2]];
         if (fabsf(An + Bn) < m_d.m_edgeFactorUI)
         {
            drawVertices.push_back(Vertex2D(A->x, A->y));
            drawVertices.push_back(Vertex2D(B->x, B->y));
         }
         if (fabsf(Bn + Cn) < m_d.m_edgeFactorUI)
         {
            drawVertices.push_back(Vertex2D(B->x, B->y));
            drawVertices.push_back(Vertex2D(C->x, C->y));
         }
         if (fabsf(Cn + An) < m_d.m_edgeFactorUI)
         {
            drawVertices.push_back(Vertex2D(C->x, C->y));
            drawVertices.push_back(Vertex2D(A->x, A->y));
         }
      }

      if (drawVertices.size() > 0)
         psur->Lines(&drawVertices[0], (int)(drawVertices.size() / 2));
   }
}

void Primitive::CalculateBuiltinOriginal()
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

   // 2 restore indices
   //   check if anti culling is enabled:
   if (m_d.m_DrawTexturesInside)
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
   ComputeNormals(m_mesh.m_vertices, m_mesh.m_indices);
}

void Primitive::UpdateEditorView()
{
   RecalculateMatrices();
   TransformVertices();
}

void Primitive::ExportMesh(FILE *f)
{
   char name[MAX_PATH];
   if (m_d.m_fVisible)
   {
      WideCharToMultiByte(CP_ACP, 0, m_wzName, -1, name, MAX_PATH, NULL, NULL);
      Vertex3D_NoTex2 *buf = new Vertex3D_NoTex2[m_mesh.NumVertices()];
      RecalculateMatrices();
      for (unsigned int i = 0; i < m_mesh.NumVertices(); i++)
      {
         const Vertex3D_NoTex2 &v = m_mesh.m_vertices[i];
         Vertex3Ds vert(v.x, v.y, v.z);
         vert = fullMatrix.MultiplyVector(vert);
         buf[i].x = vert.x;
         buf[i].y = vert.y;
         buf[i].z = vert.z;

         vert = Vertex3Ds(v.nx, v.ny, v.nz);
         vert = fullMatrix.MultiplyVectorNoTranslate(vert);
         buf[i].nx = vert.x;
         buf[i].ny = vert.y;
         buf[i].nz = vert.z;
         buf[i].tu = v.tu;
         buf[i].tv = v.tv;
      }
      WaveFrontObj_WriteObjectName(f, name);
      WaveFrontObj_WriteVertexInfo(f, buf, (unsigned int)m_mesh.NumVertices());
      const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);
      WaveFrontObj_WriteMaterial(m_d.m_szMaterial, NULL, mat);
      WaveFrontObj_UseTexture(f, m_d.m_szMaterial);
      WaveFrontObj_WriteFaceInfoLong(f, m_mesh.m_indices);
      WaveFrontObj_UpdateFaceOffset((unsigned int)m_mesh.NumVertices());
      delete[] buf;
   }
}

void Primitive::RenderObject(RenderDevice *pd3dDevice)
{
   if (!m_d.m_fGroupdRendering)
   {
      RecalculateMatrices();

      if (vertexBufferRegenerate)
      {
         m_mesh.UploadToVB(vertexBuffer, m_currentFrame);
         if (m_currentFrame != -1.0f && m_DoAnimation)
         {
            m_currentFrame+=m_speed;
            if (m_currentFrame >= (float)m_mesh.m_animationFrames.size())
            {
               if (m_Endless)
                  m_currentFrame = 0.0f;
               else
               {
                  m_currentFrame = (float)(m_mesh.m_animationFrames.size() - 1);
                  m_DoAnimation = false;
                  vertexBufferRegenerate = false;
               }
            }
         }
         else
            vertexBufferRegenerate = false;
      }
   }

   const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);
   pd3dDevice->basicShader->SetMaterial(mat);

   pd3dDevice->SetRenderState(RenderDevice::DEPTHBIAS, 0);
   pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);
   pd3dDevice->SetRenderState(RenderDevice::CULLMODE, m_d.m_fBackfacesEnabled && mat->m_bOpacityActive ? D3DCULL_CW : D3DCULL_CCW);

   if (m_d.m_fDisableLighting != 0.f)
      pd3dDevice->basicShader->SetDisableLighting(m_d.m_fDisableLighting);

   Texture * const pin = m_ptable->GetImage(m_d.m_szImage);
   Texture * const nMap = m_ptable->GetImage(m_d.m_szNormalMap);

   if (pin && nMap)
   {
       pd3dDevice->basicShader->SetTechnique(mat->m_bIsMetal ? "basic_with_texture_normal_isMetal" : "basic_with_texture_normal_isNotMetal");
       pd3dDevice->basicShader->SetTexture("Texture0", pin);
       pd3dDevice->basicShader->SetTexture("Texture4", nMap);
       pd3dDevice->basicShader->SetAlphaTestValue(pin->m_alphaTestValue * (float)(1.0 / 255.0));

       //g_pplayer->m_pin3d.SetTextureFilter(0, TEXTURE_MODE_TRILINEAR);
       // accomodate models with UV coords outside of [0,1]
       pd3dDevice->SetTextureAddressMode(0, RenderDevice::TEX_WRAP);
   }
   else if (pin)
   {
      pd3dDevice->basicShader->SetTechnique(mat->m_bIsMetal ? "basic_with_texture_isMetal" : "basic_with_texture_isNotMetal");
      pd3dDevice->basicShader->SetTexture("Texture0", pin);
      pd3dDevice->basicShader->SetAlphaTestValue(pin->m_alphaTestValue * (float)(1.0 / 255.0));

      //g_pplayer->m_pin3d.SetTextureFilter(0, TEXTURE_MODE_TRILINEAR);
      // accomodate models with UV coords outside of [0,1]
      pd3dDevice->SetTextureAddressMode(0, RenderDevice::TEX_WRAP);
   }
   else
      pd3dDevice->basicShader->SetTechnique(mat->m_bIsMetal ? "basic_without_texture_isMetal" : "basic_without_texture_isNotMetal");

   // set transform
   if (!m_d.m_fGroupdRendering)
      g_pplayer->UpdateBasicShaderMatrix(fullMatrix);

   // draw the mesh
   pd3dDevice->basicShader->Begin(0);
   if (m_d.m_fGroupdRendering)
      pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, vertexBuffer, 0, m_numGroupVertices, indexBuffer, 0, m_numGroupIndices);
   else
      pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, vertexBuffer, 0, (DWORD)m_mesh.NumVertices(), indexBuffer, 0, (DWORD)m_mesh.NumIndices());
   pd3dDevice->basicShader->End();

   if(m_d.m_fBackfacesEnabled && mat->m_bOpacityActive)
   {
       pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);
       pd3dDevice->basicShader->Begin(0);
       if (m_d.m_fGroupdRendering)
          pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, vertexBuffer, 0, m_numGroupVertices, indexBuffer, 0, m_numGroupIndices);
       else
          pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, vertexBuffer, 0, (DWORD)m_mesh.NumVertices(), indexBuffer, 0, (DWORD)m_mesh.NumIndices());
       pd3dDevice->basicShader->End();
   }

   // reset transform
   if (!m_d.m_fGroupdRendering)
      g_pplayer->UpdateBasicShaderMatrix();

   pd3dDevice->SetTextureAddressMode(0, RenderDevice::TEX_CLAMP);
   //g_pplayer->m_pin3d.DisableAlphaBlend(); //!! not necessary anymore
   if (m_d.m_fDisableLighting != 0.f)
      pd3dDevice->basicShader->SetDisableLighting(0.f);

}

// Always called each frame to render over everything else (along with alpha ramps)
void Primitive::PostRenderStatic(RenderDevice* pd3dDevice)
{
   TRACE_FUNCTION();

   if (m_d.m_staticRendering || !m_d.m_fVisible || m_d.m_fSkipRendering)
      return;
   if (m_ptable->m_fReflectionEnabled && !m_d.m_fReflectionEnabled)
      return;

   RenderObject(pd3dDevice);
}

void Primitive::RenderSetup(RenderDevice* pd3dDevice)
{
   if (m_d.m_fGroupdRendering || m_d.m_fSkipRendering)
      return;

   m_currentFrame = -1;

   if (vertexBuffer)
      vertexBuffer->release();

   pd3dDevice->CreateVertexBuffer((unsigned int)m_mesh.NumVertices(), 0, MY_D3DFVF_NOTEX2_VERTEX, &vertexBuffer);

   if (indexBuffer)
      indexBuffer->release();
   indexBuffer = pd3dDevice->CreateAndFillIndexBuffer(m_mesh.m_indices);
}

void Primitive::RenderStatic(RenderDevice* pd3dDevice)
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
   bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);
   bw.WriteString(FID(MATR), m_d.m_szMaterial);
   bw.WriteInt(FID(SCOL), m_d.m_SideColor);
   bw.WriteBool(FID(TVIS), m_d.m_fVisible);
   bw.WriteBool(FID(DTXI), m_d.m_DrawTexturesInside);
   bw.WriteBool(FID(HTEV), m_d.m_fHitEvent);
   bw.WriteFloat(FID(THRS), m_d.m_threshold);
   bw.WriteFloat(FID(ELAS), m_d.m_elasticity);
   bw.WriteFloat(FID(ELFO), m_d.m_elasticityFalloff);
   bw.WriteFloat(FID(RFCT), m_d.m_friction);
   bw.WriteFloat(FID(RSCT), m_d.m_scatter);
   bw.WriteFloat(FID(EFUI), m_d.m_edgeFactorUI);
   bw.WriteFloat(FID(CORF), m_d.m_collision_reductionFactor);
   bw.WriteBool(FID(CLDRP), m_d.m_fCollidable);
   bw.WriteBool(FID(ISTO), m_d.m_fToy);
   bw.WriteBool(FID(U3DM), m_d.m_use3DMesh);
   bw.WriteBool(FID(STRE), m_d.m_staticRendering);
   const int tmp = quantizeUnsigned<8>(clamp(m_d.m_fDisableLighting, 0.f, 1.f));
   bw.WriteInt(FID(DILI), (tmp == 1) ? 0 : tmp); // backwards compatible saving
   bw.WriteBool(FID(REEN), m_d.m_fReflectionEnabled);
   bw.WriteBool(FID(EBFC), m_d.m_fBackfacesEnabled);
   bw.WriteString( FID( MAPH ), m_d.m_szPhysicsMaterial );
   bw.WriteBool( FID( OVPH ), m_d.m_fOverwritePhysics );

   if (m_d.m_use3DMesh)
   {
      bw.WriteString(FID(M3DN), m_d.m_meshFileName);
      bw.WriteInt(FID(M3VN), (int)m_mesh.NumVertices());

#ifndef COMPRESS_MESHES
      bw.WriteStruct( FID(M3DX), &m_mesh.m_vertices[0], (int)(sizeof(Vertex3D_NoTex2)*m_mesh.NumVertices()) );
#else
      /*bw.WriteTag(FID(M3CX));
      {
      LZWWriter lzwwriter(pstm, (int *)&m_mesh.m_vertices[0], sizeof(Vertex3D_NoTex2)*m_mesh.NumVertices(), 1, sizeof(Vertex3D_NoTex2)*m_mesh.NumVertices());
      lzwwriter.CompressBits(8 + 1);
      }*/
      {
      const mz_ulong slen = (mz_ulong)(sizeof(Vertex3D_NoTex2)*m_mesh.NumVertices());
      mz_ulong clen = compressBound(slen);
      mz_uint8 * c = (mz_uint8 *)malloc(clen);
      if (compress2(c, &clen, (const unsigned char *)&m_mesh.m_vertices[0], slen, MZ_BEST_COMPRESSION) != Z_OK)
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
         bw.WriteStruct( FID(M3DI), &m_mesh.m_indices[0], (int)(sizeof(unsigned int)*m_mesh.NumIndices()) );
#else
         /*bw.WriteTag(FID(M3CI));
          LZWWriter lzwwriter(pstm, (int *)&m_mesh.m_indices[0], sizeof(unsigned int)*m_mesh.NumIndices(), 1, sizeof(unsigned int)*m_mesh.NumIndices());
          lzwwriter.CompressBits(8 + 1);*/
         const mz_ulong slen = (mz_ulong)(sizeof(unsigned int)*m_mesh.NumIndices());
         mz_ulong clen = compressBound(slen);
         mz_uint8 * c = (mz_uint8 *)malloc(clen);
         if (compress2(c, &clen, (const unsigned char *)&m_mesh.m_indices[0], slen, MZ_BEST_COMPRESSION) != Z_OK)
            ShowError("Could not compress primitive index data");
         bw.WriteInt(FID(M3CJ), (int)clen);
         bw.WriteStruct(FID(M3CI), c, clen);
         free(c);
#endif
      }
      else
      {
         std::vector<WORD> tmp(m_mesh.NumIndices());
         for (unsigned int i = 0; i < m_mesh.NumIndices(); ++i)
            tmp[i] = m_mesh.m_indices[i];
#ifndef COMPRESS_MESHES
         bw.WriteStruct( FID(M3DI), &tmp[0], (int)(sizeof(WORD)*m_mesh.NumIndices()) );
#else
         /*bw.WriteTag(FID(M3CI));
          LZWWriter lzwwriter(pstm, (int *)&tmp[0], sizeof(WORD)*m_mesh.NumIndices(), 1, sizeof(WORD)*m_mesh.NumIndices());
          lzwwriter.CompressBits(8 + 1);*/
         const mz_ulong slen = (mz_ulong)(sizeof(WORD)*m_mesh.NumIndices());
         mz_ulong clen = compressBound(slen);
         mz_uint8 * c = (mz_uint8 *)malloc(clen);
         if (compress2(c, &clen, (const unsigned char *)&tmp[0], slen, MZ_BEST_COMPRESSION) != Z_OK)
            ShowError("Could not compress primitive index data");
         bw.WriteInt(FID(M3CJ), (int)clen);
         bw.WriteStruct(FID(M3CI), c, clen);
         free(c);
#endif
      }
      
      if (m_mesh.m_animationFrames.size() > 0)
      {
         const mz_ulong slen = (mz_ulong)(sizeof(Mesh::VertData)*m_mesh.NumVertices());
         for (unsigned int i = 0; i < m_mesh.m_animationFrames.size(); i++)
         {
            mz_ulong clen = compressBound(slen);
            mz_uint8 * c = (mz_uint8 *)malloc(clen);
            if (compress2(c, &clen, (const unsigned char *)&m_mesh.m_animationFrames[i].m_frameVerts[0], slen, MZ_BEST_COMPRESSION) != Z_OK)
               ShowError("Could not compress primitive animation vertex data");
            bw.WriteInt(FID(M3AY), (int)clen);
            bw.WriteStruct(FID(M3AX), c, clen);
            free(c);
         }

      }
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
   if (!m_d.m_use3DMesh)
      CalculateBuiltinOriginal();

   unsigned int* tmp = reorderForsyth(m_mesh.m_indices.data(), (int)(m_mesh.NumIndices() / 3), (int)m_mesh.NumVertices());
   if (tmp != NULL)
   {
      memcpy(m_mesh.m_indices.data(), tmp, m_mesh.NumIndices()*sizeof(unsigned int));
      delete[] tmp;
   }

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
   else if (id == FID(NRMA))
   {
       pbr->GetString(m_d.m_szNormalMap);
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
   else if (id == FID(EFUI))
   {
      pbr->GetFloat(&m_d.m_edgeFactorUI);
   }
   else if (id == FID(CORF))
   {
      pbr->GetFloat(&m_d.m_collision_reductionFactor);
   }
   else if (id == FID(CLDRP))
   {
      pbr->GetBool(&m_d.m_fCollidable);
   }
   else if (id == FID(ISTO))
   {
      pbr->GetBool(&m_d.m_fToy);
   }
   else if ( id == FID( MAPH ) )
   {
       pbr->GetString( m_d.m_szPhysicsMaterial );
   }
   else if ( id == FID( OVPH ) )
   {
       pbr->GetBool( &m_d.m_fOverwritePhysics );
   }
   else if (id == FID(STRE))
   {
      pbr->GetBool(&m_d.m_staticRendering);
   }
   else if (id == FID(DILI))
   {
      int tmp;
      pbr->GetInt(&tmp);
      m_d.m_fDisableLighting = (tmp == 1) ? 1.f : dequantizeUnsigned<8>(tmp); // backwards compatible hacky loading!
   }
   else if (id == FID(U3DM))
   {
      pbr->GetBool(&m_d.m_use3DMesh);
   }
   else if (id == FID(EBFC))
   {
      pbr->GetBool(&m_d.m_fBackfacesEnabled);
   }
   else if (id == FID(M3DN))
   {
      pbr->GetWideString((WCHAR *)m_d.m_meshFileName);
   }
   else if (id == FID(M3VN))
   {
      pbr->GetInt(&numVertices);
      if (m_mesh.m_animationFrames.size() > 0)
      {
         for (unsigned int i = 0; i < m_mesh.m_animationFrames.size(); i++)
            m_mesh.m_animationFrames[i].m_frameVerts.clear();
         m_mesh.m_animationFrames.clear();
      }
   }
   else if (id == FID(M3DX))
   {
      m_mesh.m_vertices.clear();
      m_mesh.m_vertices.resize(numVertices);
      pbr->GetStruct(m_mesh.m_vertices.data(), (int)sizeof(Vertex3D_NoTex2)*numVertices);
   }
#ifdef COMPRESS_MESHES
   else if (id == FID(M3AY))
   {
      pbr->GetInt(&compressedAnimationVertices);
   }
   else if (id == FID(M3AX))
   {
      Mesh::FrameData frameData;
      frameData.m_frameVerts.clear();
      frameData.m_frameVerts.resize(numVertices);

      /*LZWReader lzwreader(pbr->m_pistream, (int *)m_mesh.m_vertices.data(), sizeof(Vertex3D_NoTex2)*numVertices, 1, sizeof(Vertex3D_NoTex2)*numVertices);
      lzwreader.Decoder();*/
      mz_ulong uclen = (mz_ulong)(sizeof(Mesh::VertData)*m_mesh.NumVertices());
      mz_uint8 * c = (mz_uint8 *)malloc(compressedAnimationVertices);
      pbr->GetStruct(c, compressedAnimationVertices);
      const int error = uncompress((unsigned char *)frameData.m_frameVerts.data(), &uclen, c, compressedAnimationVertices);
      if (error != Z_OK)
      {
         char err[128];
         sprintf_s(err, "Could not uncompress primitive animation vertex data, error %d", error);
         ShowError(err);
      }
      free(c);
      m_mesh.m_animationFrames.push_back(frameData);
   }
   else if (id == FID(M3CY))
   {
      pbr->GetInt(&compressedVertices);
   }
   else if (id == FID(M3CX))
   {
      m_mesh.m_vertices.clear();
      m_mesh.m_vertices.resize(numVertices);
      /*LZWReader lzwreader(pbr->m_pistream, (int *)m_mesh.m_vertices.data(), sizeof(Vertex3D_NoTex2)*numVertices, 1, sizeof(Vertex3D_NoTex2)*numVertices);
       lzwreader.Decoder();*/
      mz_ulong uclen = (mz_ulong)(sizeof(Vertex3D_NoTex2)*m_mesh.NumVertices());
      mz_uint8 * c = (mz_uint8 *)malloc(compressedVertices);
      pbr->GetStruct(c, compressedVertices);
      const int error = uncompress((unsigned char *)m_mesh.m_vertices.data(), &uclen, c, compressedVertices);
      if (error != Z_OK)
      {
         char err[128];
         sprintf_s(err, "Could not uncompress primitive vertex data, error %d", error);
         ShowError(err);
      }
      free(c);
   }
#endif
   else if (id == FID(M3FN))
   {
      pbr->GetInt(&numIndices);
   }
   else if (id == FID(M3DI))
   {
      m_mesh.m_indices.resize(numIndices);
      if (numVertices > 65535)
         pbr->GetStruct(m_mesh.m_indices.data(), (int)sizeof(unsigned int)*numIndices);
      else
      {
         std::vector<WORD> tmp(numIndices);
         pbr->GetStruct(tmp.data(), (int)sizeof(WORD)*numIndices);
         for (int i = 0; i < numIndices; ++i)
            m_mesh.m_indices[i] = tmp[i];
      }
   }
#ifdef COMPRESS_MESHES
   else if (id == FID(M3CJ))
   {
      pbr->GetInt(&compressedIndices);
   }
   else if (id == FID(M3CI))
   {
      m_mesh.m_indices.resize(numIndices);
      if (numVertices > 65535)
      {
         //LZWReader lzwreader(pbr->m_pistream, (int *)m_mesh.m_indices.data(), sizeof(unsigned int)*numIndices, 1, sizeof(unsigned int)*numIndices);
         //lzwreader.Decoder();
         mz_ulong uclen = (mz_ulong)(sizeof(unsigned int)*m_mesh.NumIndices());
         mz_uint8 * c = (mz_uint8 *)malloc(compressedIndices);
         pbr->GetStruct(c, compressedIndices);
         const int error = uncompress((unsigned char *)m_mesh.m_indices.data(), &uclen, c, compressedIndices);
         if (error != Z_OK)
         {
            char err[128];
            sprintf_s(err, "Could not uncompress (large) primitive index data, error %d", error);
            ShowError(err);
         }
         free(c);
      }
      else
      {
         std::vector<WORD> tmp(numIndices);

         //LZWReader lzwreader(pbr->m_pistream, (int *)tmp.data(), sizeof(WORD)*numIndices, 1, sizeof(WORD)*numIndices);
         //lzwreader.Decoder();
         mz_ulong uclen = (mz_ulong)(sizeof(WORD)*m_mesh.NumIndices());
         mz_uint8 * c = (mz_uint8 *)malloc(compressedIndices);
         pbr->GetStruct(c, compressedIndices);
         const int error = uncompress((unsigned char *)tmp.data(), &uclen, c, compressedIndices);
         if (error != Z_OK)
         {
            char err[128];
            sprintf_s(err, "Could not uncompress (small) primitive index data, error %d", error);
            ShowError(err);
         }
         free(c);

         for (int i = 0; i < numIndices; ++i)
            m_mesh.m_indices[i] = tmp[i];
      }
   }
#endif
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
   if (!m_d.m_use3DMesh)
      CalculateBuiltinOriginal();

   UpdateEditorView();

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
            char szFileName[1024] = { 0 };

            GetDlgItemText(hwndDlg, IDC_FILENAME_EDIT, szFileName, 1023);
            if (szFileName[0] == 0)
            {
               ShowError("No .obj file selected!");
               break;
            }
            prim->m_mesh.Clear();
            prim->m_d.m_use3DMesh = false;
            if (prim->vertexBuffer)
            {
               prim->vertexBuffer->release();
               prim->vertexBuffer = 0;
            }
            bool flipTV = false;
            bool convertToLeftHanded = IsDlgButtonChecked(hwndDlg, IDC_CONVERT_COORD_CHECK) == BST_CHECKED;
            bool importAbsolutePosition = IsDlgButtonChecked(hwndDlg, IDC_ABS_POSITION_RADIO) == BST_CHECKED;
            bool centerMesh = IsDlgButtonChecked(hwndDlg, IDC_CENTER_MESH) == BST_CHECKED;
            bool importMaterial = IsDlgButtonChecked(hwndDlg, IDC_IMPORT_MATERIAL) == BST_CHECKED;
            bool importAnimation = IsDlgButtonChecked(hwndDlg, IDC_IMPORT_ANIM_SEQUENCE) == BST_CHECKED;
            if (importMaterial)
            {
               string filename(szFileName);
               size_t index = filename.find_last_of(".");
               if (index != -1)
               {
                  char szMatName[1024] = { 0 };
                  memcpy(szMatName, szFileName, index);
                  strcat_s(szMatName, ".mtl");
                  Material *mat = new Material();
                  if (WaveFrontObjLoadMaterial(szMatName, mat))
                  {
                     PinTable *pActiveTable = g_pvp->GetActiveTable();
                     if(pActiveTable)
                     {
                         pActiveTable->AddMaterial( mat );
                     }
                     strcpy_s(prim->m_d.m_szMaterial, mat->m_szName);
                     g_pvp->m_sb.PopulateDropdowns(); // May need to update list of images
                     g_pvp->m_sb.RefreshProperties();
                  }
               }
            }
            if (prim->m_mesh.LoadWavefrontObj(szFileName, flipTV, convertToLeftHanded))
            {
               if (importAbsolutePosition || centerMesh)
               {
                  for (unsigned int i = 0; i < prim->m_mesh.m_vertices.size(); i++)
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
                        for (unsigned int t = 0; t < prim->m_mesh.m_animationFrames.size(); t++)
                        {
                           for (unsigned int i = 0; i < prim->m_mesh.m_vertices.size(); i++)
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
               prim->UpdateEditorView();
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

            const HRESULT hr = GetRegString("RecentDir", "ImportDir", szInitialDir, 1024);
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
            SetForegroundWindow(hwndDlg);
            if (ret)
            {
               SetDlgItemText(hwndDlg, IDC_FILENAME_EDIT, szFileName);
               SetRegValue("RecentDir", "ImportDir", REG_SZ, szInitialDir, lstrlen(szInitialDir));
               string filename(szFileName);
               size_t index = filename.find_last_of("\\");
               if (index != -1)
               {
                  index++;
                  string name = filename.substr(index, filename.length() - index);
                  strcpy_s(prim->m_d.m_meshFileName, name.c_str());
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
   DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_MESH_IMPORT_DIALOG), g_pvp->m_hwnd, ObjImportProc, (size_t)this);
   return false;

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

   const HRESULT hr = GetRegString("RecentDir", "ImportDir", szInitialDir, 1024);
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
   if (index != -1)
   {
      index++;
      string name = filename.substr(index, filename.length() - index);
      strcpy_s(m_d.m_meshFileName, name.c_str());
   }
   if (ret == 0)
   {
      return false;
   }
   SetRegValue("RecentDir", "ImportDir", REG_SZ, szInitialDir, lstrlen(szInitialDir));
   m_mesh.Clear();
   m_d.m_use3DMesh = false;
   if (vertexBuffer)
   {
      vertexBuffer->release();
      vertexBuffer = 0;
   }
   bool flipTV = false;
   bool convertToLeftHanded = false;
   int ans = MessageBox(g_pvp->m_hwnd, "Do you want to mirror the object?", "Convert coordinate system?", MB_YESNO | MB_DEFBUTTON2);
   if (ans == IDYES)
   {
      convertToLeftHanded = true;
   }
   else
   {
      ans = MessageBox(g_pvp->m_hwnd, "Do you want to convert texture coordinates?", "Confirm", MB_YESNO | MB_DEFBUTTON2);
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

STDMETHODIMP Primitive::get_NormalMap(BSTR *pVal)
{
    WCHAR wz[512];

    MultiByteToWideChar(CP_ACP, 0, m_d.m_szNormalMap, -1, wz, 32);
    *pVal = SysAllocString(wz);

    return S_OK;
}

STDMETHODIMP Primitive::put_NormalMap(BSTR newVal)
{
    char szImage[MAXTOKEN];
    WideCharToMultiByte(CP_ACP, 0, newVal, -1, szImage, 32, NULL, NULL);
    const Texture * const tex = m_ptable->GetImage(szImage);
    if (tex && tex->IsHDR())
    {
        ShowError("Cannot use a HDR image (.exr/.hdr) here");
        return E_FAIL;
    }

    STARTUNDO

        strcpy_s(m_d.m_szNormalMap, szImage);

    STOPUNDO

        return S_OK;
}

STDMETHODIMP Primitive::get_MeshFileName(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_meshFileName, -1, wz, 256);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Primitive::put_MeshFileName(BSTR newVal)
{
   STARTUNDO

      WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_meshFileName, 256, NULL, NULL);

   STOPUNDO
      return S_OK;
}

bool Primitive::LoadMesh()
{
   bool result = false;
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

   const HRESULT hr = GetRegString("RecentDir", "LoadDir", szInitialDir, 1024);
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
   if (ret == 0)
   {
      return;
   }
   char name[MAX_PATH];
   WideCharToMultiByte(CP_ACP, 0, m_wzName, -1, name, MAX_PATH, NULL, NULL);
   m_mesh.SaveWavefrontObj(ofn.lpstrFile, m_d.m_use3DMesh ? name : "Primitive");
}

bool Primitive::IsTransparent()
{
   if (m_d.m_fSkipRendering)
      return false;

   return m_ptable->GetMaterial(m_d.m_szMaterial)->m_bOpacityActive;
}

float Primitive::GetDepth(const Vertex3Ds& viewDir)
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
      STARTUNDO

         m_d.m_Sides = newVal;
      if (!m_d.m_use3DMesh)
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
   if (m_d.m_SideColor != newVal)
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
   if (m_d.m_DrawTexturesInside != VBTOF(newVal))
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

STDMETHODIMP Primitive::get_Y(float *pVal)
{
   *pVal = m_d.m_vPosition.y;

   return S_OK;
}

STDMETHODIMP Primitive::put_Y(float newVal)
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

STDMETHODIMP Primitive::get_Z(float *pVal)
{
   *pVal = m_d.m_vPosition.z;

   return S_OK;
}

STDMETHODIMP Primitive::put_Z(float newVal)
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

STDMETHODIMP Primitive::get_Size_X(float *pVal)
{
   *pVal = m_d.m_vSize.x;

   return S_OK;
}

STDMETHODIMP Primitive::put_Size_X(float newVal)
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

STDMETHODIMP Primitive::get_Size_Y(float *pVal)
{
   *pVal = m_d.m_vSize.y;

   return S_OK;
}

STDMETHODIMP Primitive::put_Size_Y(float newVal)
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

STDMETHODIMP Primitive::get_Size_Z(float *pVal)
{
   *pVal = m_d.m_vSize.z;

   return S_OK;
}

STDMETHODIMP Primitive::put_Size_Z(float newVal)
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
   if (m_d.m_aRotAndTra[1] != newVal)
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
   if (m_d.m_aRotAndTra[2] != newVal)
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
   if (m_d.m_aRotAndTra[3] != newVal)
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
   if (m_d.m_aRotAndTra[4] != newVal)
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
   if (m_d.m_aRotAndTra[5] != newVal)
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
   if (m_d.m_aRotAndTra[6] != newVal)
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
   if (m_d.m_aRotAndTra[7] != newVal)
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
   if (m_d.m_aRotAndTra[8] != newVal)
   {
      STARTUNDO
         m_d.m_aRotAndTra[8] = newVal;
      STOPUNDO

         if (!g_pplayer)
            UpdateEditorView();
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
   STARTUNDO
      m_d.m_edgeFactorUI = newVal;
   STOPUNDO

      return S_OK;
}

STDMETHODIMP Primitive::get_CollisionReductionFactor(float *pVal)
{
   *pVal = m_d.m_collision_reductionFactor;
   return S_OK;
}

STDMETHODIMP Primitive::put_CollisionReductionFactor(float newVal)
{
   STARTUNDO
      m_d.m_collision_reductionFactor = newVal;
   STOPUNDO

      return S_OK;
}

STDMETHODIMP Primitive::get_EnableStaticRendering(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_staticRendering);

   return S_OK;
}

STDMETHODIMP Primitive::put_EnableStaticRendering(VARIANT_BOOL newVal)
{
   STARTUNDO

      m_d.m_staticRendering = VBTOF(newVal);

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

STDMETHODIMP Primitive::get_ElasticityFalloff(float *pVal)
{
   *pVal = m_d.m_elasticityFalloff;

   return S_OK;
}

STDMETHODIMP Primitive::put_ElasticityFalloff(float newVal)
{
   STARTUNDO
      m_d.m_elasticityFalloff = newVal;
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

   m_d.m_friction = clamp(newVal, 0.f, 1.f);

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
   *pVal = (VARIANT_BOOL)FTOVB((!g_pplayer) ? m_d.m_fCollidable : m_vhoCollidable[0]->m_fEnabled);

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
   {
       const bool b = !!fNewVal;
       if (m_vhoCollidable.size() > 0 && m_vhoCollidable[0]->m_fEnabled != b)
           for (size_t i = 0; i < m_vhoCollidable.size(); i++) //!! costly
               m_vhoCollidable[i]->m_fEnabled = b; //copy to hit-testing on entities composing the object
   }

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

STDMETHODIMP Primitive::get_BackfacesEnabled(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fBackfacesEnabled);

   return S_OK;
}

STDMETHODIMP Primitive::put_BackfacesEnabled(VARIANT_BOOL newVal)
{
   STARTUNDO

   m_d.m_fBackfacesEnabled = VBTOF(newVal);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Primitive::get_DisableLighting(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fDisableLighting != 0.f);

   return S_OK;
}

STDMETHODIMP Primitive::put_DisableLighting(VARIANT_BOOL newVal)
{
   STARTUNDO

   m_d.m_fDisableLighting = VBTOF(newVal) ? 1.f : 0;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Primitive::get_BlendDisableLighting(float *pVal)
{
   *pVal = m_d.m_fDisableLighting;

   return S_OK;
}

STDMETHODIMP Primitive::put_BlendDisableLighting(float newVal)
{
   STARTUNDO

   m_d.m_fDisableLighting = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Primitive::get_ReflectionEnabled(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fReflectionEnabled);

   return S_OK;
}

STDMETHODIMP Primitive::put_ReflectionEnabled(VARIANT_BOOL newVal)
{
   STARTUNDO

      m_d.m_fReflectionEnabled = VBTOF(newVal);

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Primitive::get_PhysicsMaterial( BSTR *pVal )
{
    WCHAR wz[512];

    MultiByteToWideChar( CP_ACP, 0, m_d.m_szPhysicsMaterial, -1, wz, 32 );
    *pVal = SysAllocString( wz );

    return S_OK;
}

STDMETHODIMP Primitive::put_PhysicsMaterial( BSTR newVal )
{
    STARTUNDO

        WideCharToMultiByte( CP_ACP, 0, newVal, -1, m_d.m_szPhysicsMaterial, 32, NULL, NULL );

    STOPUNDO

        return S_OK;
}

STDMETHODIMP Primitive::get_OverwritePhysics( VARIANT_BOOL *pVal )
{
    *pVal = (VARIANT_BOOL)FTOVB( m_d.m_fOverwritePhysics );

    return S_OK;
}

STDMETHODIMP Primitive::put_OverwritePhysics( VARIANT_BOOL newVal )
{
    STARTUNDO

        m_d.m_fOverwritePhysics = VBTOF( newVal );

    STOPUNDO

        return S_OK;
}

STDMETHODIMP Primitive::PlayAnim(float startFrame, float speed)
{
   int iFrame = (int)startFrame;
   if (m_mesh.m_animationFrames.size() > 0 )
   {
      if (startFrame >= m_mesh.m_animationFrames.size())
         startFrame = 0.0f;
      if (startFrame < 0.0f)
         startFrame *= 1.0f;

      m_currentFrame = startFrame;
      if (speed < 0.0f) speed *= -1.0f;
      m_speed = speed;
      m_DoAnimation = true;
      m_Endless = false;
      vertexBufferRegenerate = true;
   }
   return S_OK;
}

STDMETHODIMP Primitive::PlayAnimEndless(float speed)
{
   if (m_mesh.m_animationFrames.size() > 0)
   {
      m_currentFrame = 0.0f;
      if (speed < 0.0f) speed *= -1.0f;
      m_speed = speed;
      m_DoAnimation = true;
      m_Endless = true;
      vertexBufferRegenerate = true;
   }
   return S_OK;
}
STDMETHODIMP Primitive::StopAnim()
{
   m_DoAnimation = false;
   vertexBufferRegenerate = false;
   return S_OK;
}

STDMETHODIMP Primitive::ContinueAnim(float speed)
{
   if (m_currentFrame > 0.0f)
   {
      if (speed < 0.0f) speed *= -1.0f;
      m_speed = speed;
      m_DoAnimation = true;
      vertexBufferRegenerate = true;
   }
   return S_OK;
}

STDMETHODIMP Primitive::ShowFrame(float frame)
{
   int iFrame = (int)frame;
   m_DoAnimation = false;
   if (iFrame >= (int)m_mesh.m_animationFrames.size())
      frame = (float)(m_mesh.m_animationFrames.size() - 1);
   m_currentFrame = frame;
   vertexBufferRegenerate = true;
   return S_OK;
}

void Primitive::GetDialogPanes(Vector<PropertyPane> *pvproppane)
{
   PropertyPane *pproppane;

   pproppane = new PropertyPane(IDD_PROP_NAME, NULL);
   pvproppane->AddElement(pproppane);

   m_propVisual = new PropertyPane(IDD_PROPPRIMITIVE_VISUALS, IDS_VISUALS);
   pvproppane->AddElement(m_propVisual);

   m_propPosition = new PropertyPane(IDD_PROPPRIMITIVE_POSITION, IDS_POSITION_TRANSLATION);
   pvproppane->AddElement(m_propPosition);

   m_propPhysics = new PropertyPane(IDD_PROPPRIMITIVE_PHYSICS, IDS_PHYSICS);
   pvproppane->AddElement(m_propPhysics);
}

void Primitive::UpdatePropertyPanes()
{
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
      EnableWindow( GetDlgItem( m_propPhysics->dialogHwnd, IDC_MATERIAL_COMBO4 ), FALSE );
      EnableWindow( GetDlgItem( m_propPhysics->dialogHwnd, IDC_OVERWRITE_MATERIAL_SETTINGS ), FALSE );
   }
   else if (!m_d.m_fToy && m_d.m_fCollidable)
   {
      EnableWindow( GetDlgItem( m_propPhysics->dialogHwnd, IDC_OVERWRITE_MATERIAL_SETTINGS ), TRUE );
      EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 34), TRUE);
      EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 111), TRUE);
      EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 481), TRUE);
      if (m_d.m_fHitEvent)
         EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 33), TRUE);
      else
         EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 33), FALSE);

      if ( m_d.m_fOverwritePhysics )
      {
          EnableWindow( GetDlgItem( m_propPhysics->dialogHwnd, 110 ), TRUE );
          EnableWindow( GetDlgItem( m_propPhysics->dialogHwnd, 112 ), TRUE );
          EnableWindow( GetDlgItem( m_propPhysics->dialogHwnd, 114 ), TRUE );
          EnableWindow( GetDlgItem( m_propPhysics->dialogHwnd, 115 ), TRUE );
          EnableWindow( GetDlgItem( m_propPhysics->dialogHwnd, IDC_MATERIAL_COMBO4 ), FALSE );
      }
      else
      {
          EnableWindow( GetDlgItem( m_propPhysics->dialogHwnd, 110 ), FALSE );
          EnableWindow( GetDlgItem( m_propPhysics->dialogHwnd, 112 ), FALSE );
          EnableWindow( GetDlgItem( m_propPhysics->dialogHwnd, 114 ), FALSE );
          EnableWindow( GetDlgItem( m_propPhysics->dialogHwnd, 115 ), FALSE );
          EnableWindow( GetDlgItem( m_propPhysics->dialogHwnd, IDC_MATERIAL_COMBO4 ), TRUE );
      }
   }

}

void Primitive::SetDefaultPhysics(bool fromMouseClick)
{
   static const char strKeyName[] = "DefaultProps\\Primitive";
   m_d.m_elasticity = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "Elasticity", 0.3f) : 0.3f;
   m_d.m_elasticityFalloff = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "ElasticityFalloff", 0.5f) : 0.5f;
   m_d.m_friction = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "Friction", 0.3f) : 0.3f;
   m_d.m_scatter = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "Scatter", 0) : 0;
}

STDMETHODIMP Primitive::get_DepthBias(float *pVal)
{
   *pVal = m_d.m_depthBias;

   return S_OK;
}

STDMETHODIMP Primitive::put_DepthBias(float newVal)
{
   if (m_d.m_depthBias != newVal)
   {
      STARTUNDO

         m_d.m_depthBias = newVal;

      STOPUNDO
   }

   return S_OK;
}
