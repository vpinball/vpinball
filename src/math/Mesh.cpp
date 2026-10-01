// license:GPLv3+

// implementation of the Primitive class.

#include "core/stdafx.h" 

#include "renderer/VertexBuffer.h"
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

bool Mesh::LoadAnimation(const std::filesystem::path& fname, const MeshUnits units)
{
   m_validBounds = false;
   const string stem = PathToUTF8(fname.stem());
   const size_t idx = stem.find_last_of('_');
   if (idx == string::npos)
   {
      ShowError("Can't find sequence of obj files! The file name of the sequence must be <meshname>_x.obj where x is the frame number!");
      return false;
   }
   // Frames are the '<meshname>_*.obj' files of the folder, in name order (matched ignoring ASCII case, like the Windows file search did)
   const string prefix = lowerCase(stem.substr(0, idx + 1));
   const std::filesystem::path folder = fname.parent_path();
   vector<std::filesystem::path> allFiles;
   std::error_code ec;
   for (auto it = std::filesystem::directory_iterator(folder.empty() ? std::filesystem::path(".") : folder, ec); !ec && it != std::filesystem::directory_iterator(); it.increment(ec))
   {
      const string name = lowerCase(PathToUTF8(it->path().filename()));
      std::error_code typeError;
      if (name.starts_with(prefix) && name.ends_with(".obj"sv) && it->is_regular_file(typeError))
         allFiles.push_back(it->path());
   }
   std::ranges::sort(allFiles, [](const std::filesystem::path& a, const std::filesystem::path& b) { return lowerCase(PathToUTF8(a.filename())) < lowerCase(PathToUTF8(b.filename())); });
   const int frameCounter = static_cast<int>(allFiles.size());
   m_animationFrames.resize(frameCounter);
   for (size_t i = 0; i < allFiles.size(); i++)
   {
      ObjLoader loader;
      if (loader.Load(allFiles[i], units))
      {
         const vector<Vertex3D_NoTex2>& verts = loader.GetVertices();
         const vector<unsigned int>& indices = loader.GetIndices();
         if ((m_indices.size() != indices.size()) || (m_vertices.size() != verts.size()) || (memcmp(m_indices.data(), indices.data(), indices.size()*sizeof(unsigned int)) != 0))
         {
            ShowError("Error: frames of animation do not share the same data layout.");
            m_animationFrames.clear(); // No partial animation (frames without vertices)
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
         ShowError("Unable to load file " + PathToUTF8(allFiles[i]));
         m_animationFrames.clear(); // No partial animation (frames without vertices)
         return false;
      }

   }
   ShowMessage(MsgSeverity::Info, std::to_string(frameCounter) + " frames imported!");
   return true;
}

bool Mesh::LoadWavefrontObj(const std::filesystem::path& fname, const MeshUnits units)
{
   m_validBounds = false;
   Clear();
   ObjLoader loader;
   if (loader.Load(fname, units))
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

bool Mesh::SaveWavefrontObj(const std::filesystem::path& fname, const string& description, const MeshUnits units)
{
   ObjLoader loader;
   return loader.Save(fname, description.empty() ? PathToUTF8(fname) : description, *this, units);
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
