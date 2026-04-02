// license:GPLv3+

// implementation of the Primitive class.

#include "core/stdafx.h" 

#include "utils/objloader.h"

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
#ifndef __STANDALONE__
   idx++;
   name.erase(idx);
   string sname = name + "*.obj";
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

void Mesh::UploadToVB(std::shared_ptr<VertexBuffer> vb, const float frame) 
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
   vb->Lock(buf);
   memcpy(buf, m_vertices.data(), sizeof(Vertex3D_NoTex2)*m_vertices.size());
   vb->Unlock();
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
