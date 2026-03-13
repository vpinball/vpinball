// license:GPLv3+

#pragma once

class Mesh final
{
public:
   Vertex3Ds middlePoint;
   struct VertData
   {
      float x, y, z;
      float nx, ny, nz;
   };
   struct FrameData
   {
      vector<VertData> m_frameVerts;
   };

   vector<FrameData> m_animationFrames;
   vector<Vertex3D_NoTex2> m_vertices;
   vector<unsigned int> m_indices;
   Vertex3Ds m_minAABound, m_maxAABound;
   bool m_validBounds = false;

   Mesh()
   {
      middlePoint.x = 0.0f;
      middlePoint.y = 0.0f;
      middlePoint.z = 0.0f;
   }
   void Clear();
   bool LoadWavefrontObj(const string& fname, const bool flipTV, const bool convertToLeftHanded);
   void SaveWavefrontObj(const string& fname, const string& description);
   bool LoadAnimation(const char* fname, const bool flipTV, const bool convertToLeftHanded);

   size_t NumVertices() const { return m_vertices.size(); }
   size_t NumIndices() const { return m_indices.size(); }
   void UploadToVB(std::shared_ptr<VertexBuffer>, const float frame);
   void UpdateBounds();
};