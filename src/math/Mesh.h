// license:GPLv3+

#pragma once

class VertexBuffer;

// Units and axes conventions supported when importing from or exporting to a Wavefront OBJ file.
// VPX uses a left-handed coordinate system (X to the right, Y toward the player, Z up) expressed in VP units.
enum class MeshUnits
{
   VPUnits, // File expressed in VP units, -Z up (VPX convention)
   Meters // File expressed in meters, +Y up, -Z forward (Wavefront convention, Blender's defaults)
};

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
   bool LoadWavefrontObj(const string& fname, const MeshUnits units);
   void SaveWavefrontObj(const string& fname, const string& description, const MeshUnits units);
   bool LoadAnimation(const char* fname, const MeshUnits units);

   size_t NumVertices() const { return m_vertices.size(); }
   size_t NumIndices() const { return m_indices.size(); }
   void UploadToVB(std::shared_ptr<VertexBuffer>, const float frame);
   void UpdateBounds();
};