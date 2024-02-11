#pragma once

#include <unordered_set>

class ObjLoader final
{
public:

   ObjLoader() = default;
   ~ObjLoader()
   {
      if (m_fHandle != nullptr) fclose(m_fHandle);
      if (m_matFile != nullptr) fclose(m_matFile);
      m_verts.clear();
      m_indices.clear();
   }

   bool Load(const string& filename, const bool flipTv, const bool convertToLeftHanded);
   void Save(const string& filename, const string& description, const Mesh& mesh);
   bool ExportStart(const string& filename);
   void ExportEnd()
   {
      fclose(m_fHandle);
      m_fHandle = nullptr;
      fclose(m_matFile);
      m_matFile = nullptr;
   }
   void UpdateFaceOffset(unsigned int numVertices)
   {
      m_faceIndexOffset += numVertices;
   }
   void WriteObjectName(const string& objname)
   {
      fprintf_s(m_fHandle, "o %s\n", objname.c_str());
   }
   void WriteVertexInfo(const Vertex3D_NoTex2* verts, unsigned int numVerts);
   void WriteFaceInfo(const vector<WORD>& faces);
   void WriteFaceInfoLong(const vector<unsigned int>& faces);
   void WriteFaceInfoList(const WORD* faces, const unsigned int numIndices);
   void UseTexture(const string& texelName) const
   {
      fprintf_s(m_fHandle, "usemtl %s\n", texelName.c_str());
   }
   bool LoadMaterial(const string& filename, Material* const mat);
   void WriteMaterial(const string& texelName, const string& texelFilename, const Material* const mat);

   const vector<Vertex3D_NoTex2>& GetVertices()
   {
      return m_verts;
   }
   const vector<unsigned int>& GetIndices()
   {
      return m_indices;
   }
private:

   struct MyPoly
   {
      int vi0, ti0, ni0;
      int vi1, ti1, ni1;
      int vi2, ti2, ni2;
   };

   vector<Vertex3Ds> m_tmpVerts;
   vector<Vertex3Ds> m_tmpNorms;
   vector<Vertex2D> m_tmpTexel;
   std::unordered_set<std::pair<const Vertex3D_NoTex2*, const unsigned int>, Vertex3D_NoTex2IdxHashFunctor, Vertex3D_NoTex2IdxComparator> m_tmpCombined; // only used to find duplicate vertices quickly
   vector<MyPoly> m_tmpFaces;
   vector<Vertex3D_NoTex2> m_verts;
   vector<unsigned int> m_indices;
   unsigned int m_faceIndexOffset = 0;
   FILE* m_matFile = nullptr;
   FILE* m_fHandle = nullptr;
};
