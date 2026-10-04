// license:GPLv3+

#include "core/stdafx.h"

#include "math/Mesh.h"

// Geometry only use of tinygltf (no image, no file IO as we handle it ourselves), sharing the
// project's nlohmann json instead of the one bundled with tinygltf
#include <nlohmann/json.hpp>
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_INCLUDE_JSON
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_FS
#include "tinygltf/tiny_gltf.h"


// glTF <-> VPX coordinate conversion. VPX uses a left handed coordinate system expressed in VP units
// (X to the right, Y toward the player, Z up). The glTF files use the standard glTF convention:
// right handed, +Y up, expressed in meters (same convention as the Wavefront OBJ "Meters" mode).
// Conversion: gltf(x, y, z) = vpx(x, z, y) expressed in meters, with a triangle winding reversal
// to compensate for the handedness change, and a flipped V texture coordinate.

namespace
{

vec3 VpxToGltf(const float x, const float y, const float z) { return vec3(VPUTOM(x), VPUTOM(z), VPUTOM(y)); }
vec3 GltfToVpx(const float x, const float y, const float z) { return vec3(MTOVPU(x), MTOVPU(z), MTOVPU(y)); }

int AddAccessor(tinygltf::Model& model, const void* data, const size_t dataSize, const int componentType, const size_t count, const int type, const float* minBounds = nullptr,
   const float* maxBounds = nullptr)
{
   tinygltf::BufferView bufferView;
   bufferView.buffer = 0;
   bufferView.byteOffset = model.buffers[0].data.size();
   bufferView.byteLength = dataSize;
   bufferView.byteStride = 0;
   model.bufferViews.push_back(bufferView);

   auto* const bytes = static_cast<const unsigned char*>(data);
   model.buffers[0].data.insert(model.buffers[0].data.end(), bytes, bytes + dataSize);

   tinygltf::Accessor accessor;
   accessor.bufferView = static_cast<int>(model.bufferViews.size()) - 1;
   accessor.byteOffset = 0;
   accessor.componentType = componentType;
   accessor.count = count;
   accessor.type = type;
   if (minBounds)
      accessor.minValues.assign(minBounds, minBounds + 3);
   if (maxBounds)
      accessor.maxValues.assign(maxBounds, maxBounds + 3);
   model.accessors.push_back(accessor);
   return static_cast<int>(model.accessors.size()) - 1;
}

// Reads a float array accessor (VEC2/VEC3) as a flat float array
bool ReadFloatAccessor(const tinygltf::Model& model, const int accessorIndex, std::vector<float>& out, const int numComponents)
{
   if (accessorIndex < 0 || accessorIndex >= static_cast<int>(model.accessors.size()))
      return false;
   const tinygltf::Accessor& accessor = model.accessors[accessorIndex];
   if (accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT || accessor.count == 0)
      return false;
   const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
   const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
   const size_t stride = bufferView.byteStride ? bufferView.byteStride : numComponents * sizeof(float);
   const size_t base = bufferView.byteOffset + accessor.byteOffset;
   out.resize(accessor.count * numComponents);
   for (size_t i = 0; i < accessor.count; i++)
      memcpy(out.data() + i * numComponents, buffer.data.data() + base + i * stride, numComponents * sizeof(float));
   return true;
}

} // anonymous namespace


bool Mesh::SaveGLB(vector<uint8_t>& out) const
{
   if (m_vertices.empty())
      return false;

   try
   {
      tinygltf::Model model;
      model.asset.version = "2.0"s;
      model.asset.generator = "Visual Pinball"s;
      model.buffers.resize(1);

      // Attributes (positions, normals and texture coordinates, in glTF space)
      const size_t numVertices = m_vertices.size();
      std::vector<float> positions(numVertices * 3), normals(numVertices * 3), texcoords(numVertices * 2);
      vec3 minBound(FLT_MAX, FLT_MAX, FLT_MAX), maxBound(-FLT_MAX, -FLT_MAX, -FLT_MAX);
      for (size_t i = 0; i < numVertices; i++)
      {
         const Vertex3D_NoTex2& v = m_vertices[i];
         const vec3 pos = VpxToGltf(v.x, v.y, v.z);
         const vec3 nrm = VpxToGltf(v.nx, v.ny, v.nz);
         positions[i * 3 + 0] = pos.x;
         positions[i * 3 + 1] = pos.y;
         positions[i * 3 + 2] = pos.z;
         normals[i * 3 + 0] = nrm.x;
         normals[i * 3 + 1] = nrm.y;
         normals[i * 3 + 2] = nrm.z;
         texcoords[i * 2 + 0] = v.tu;
         texcoords[i * 2 + 1] = 1.f - v.tv;
         minBound.x = std::min(minBound.x, pos.x);
         minBound.y = std::min(minBound.y, pos.y);
         minBound.z = std::min(minBound.z, pos.z);
         maxBound.x = std::max(maxBound.x, pos.x);
         maxBound.y = std::max(maxBound.y, pos.y);
         maxBound.z = std::max(maxBound.z, pos.z);
      }
      const int posAccessor
         = AddAccessor(model, positions.data(), positions.size() * sizeof(float), TINYGLTF_COMPONENT_TYPE_FLOAT, numVertices, TINYGLTF_TYPE_VEC3, &minBound.x, &maxBound.x);
      const int nrmAccessor = AddAccessor(model, normals.data(), normals.size() * sizeof(float), TINYGLTF_COMPONENT_TYPE_FLOAT, numVertices, TINYGLTF_TYPE_VEC3);
      const int texAccessor = AddAccessor(model, texcoords.data(), texcoords.size() * sizeof(float), TINYGLTF_COMPONENT_TYPE_FLOAT, numVertices, TINYGLTF_TYPE_VEC2);

      // Indices, with reversed winding to compensate for the handedness change
      const size_t numIndices = m_indices.size();
      int idxAccessor;
      if (numVertices <= 65535)
      {
         std::vector<uint16_t> indices(numIndices);
         for (size_t i = 0; i < numIndices; i += 3)
         {
            indices[i + 0] = static_cast<uint16_t>(m_indices[i + 0]);
            indices[i + 1] = static_cast<uint16_t>(m_indices[i + 2]);
            indices[i + 2] = static_cast<uint16_t>(m_indices[i + 1]);
         }
         idxAccessor = AddAccessor(model, indices.data(), indices.size() * sizeof(uint16_t), TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT, numIndices, TINYGLTF_TYPE_SCALAR);
      }
      else
      {
         std::vector<uint32_t> indices(numIndices);
         for (size_t i = 0; i < numIndices; i += 3)
         {
            indices[i + 0] = m_indices[i + 0];
            indices[i + 1] = m_indices[i + 2];
            indices[i + 2] = m_indices[i + 1];
         }
         idxAccessor = AddAccessor(model, indices.data(), indices.size() * sizeof(uint32_t), TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT, numIndices, TINYGLTF_TYPE_SCALAR);
      }

      // Animation frames as morph targets (position/normal deltas, in glTF space)
      tinygltf::Primitive primitive;
      primitive.mode = TINYGLTF_MODE_TRIANGLES;
      primitive.indices = idxAccessor;
      primitive.attributes["POSITION"] = posAccessor;
      primitive.attributes["NORMAL"] = nrmAccessor;
      primitive.attributes["TEXCOORD_0"] = texAccessor;
      for (const FrameData& frame : m_animationFrames)
      {
         if (frame.m_frameVerts.size() != numVertices)
            continue;
         std::vector<float> deltaPos(numVertices * 3), deltaNrm(numVertices * 3);
         for (size_t i = 0; i < numVertices; i++)
         {
            const vec3 dp = VpxToGltf(frame.m_frameVerts[i].x - m_vertices[i].x, frame.m_frameVerts[i].y - m_vertices[i].y, frame.m_frameVerts[i].z - m_vertices[i].z);
            const vec3 dn = VpxToGltf(frame.m_frameVerts[i].nx - m_vertices[i].nx, frame.m_frameVerts[i].ny - m_vertices[i].ny, frame.m_frameVerts[i].nz - m_vertices[i].nz);
            deltaPos[i * 3 + 0] = dp.x;
            deltaPos[i * 3 + 1] = dp.y;
            deltaPos[i * 3 + 2] = dp.z;
            deltaNrm[i * 3 + 0] = dn.x;
            deltaNrm[i * 3 + 1] = dn.y;
            deltaNrm[i * 3 + 2] = dn.z;
         }
         std::map<std::string, int> target;
         target["POSITION"] = AddAccessor(model, deltaPos.data(), deltaPos.size() * sizeof(float), TINYGLTF_COMPONENT_TYPE_FLOAT, numVertices, TINYGLTF_TYPE_VEC3);
         target["NORMAL"] = AddAccessor(model, deltaNrm.data(), deltaNrm.size() * sizeof(float), TINYGLTF_COMPONENT_TYPE_FLOAT, numVertices, TINYGLTF_TYPE_VEC3);
         primitive.targets.push_back(target);
      }

      model.meshes.resize(1);
      model.meshes[0].name = "mesh"s;
      model.meshes[0].primitives.push_back(primitive);
      model.nodes.resize(1);
      model.nodes[0].mesh = 0;
      model.scenes.resize(1);
      model.scenes[0].nodes.push_back(0);
      model.defaultScene = 0;

      tinygltf::TinyGLTF gltf;
      std::ostringstream stream(std::ios::binary);
      if (!gltf.WriteGltfSceneToStream(&model, stream, false, true))
      {
         PLOGE << "Failed to serialize mesh to GLB";
         return false;
      }
      const string content = stream.str();
      out.assign(content.begin(), content.end());
      return true;
   }
   catch (const std::exception& e)
   {
      PLOGE << "Failed to serialize mesh to GLB: " << e.what();
      return false;
   }
}

bool Mesh::LoadGLB(const uint8_t* data, const size_t size)
{
   try
   {
      tinygltf::Model model;
      tinygltf::TinyGLTF gltf;
      std::string err, warn;
      if (!gltf.LoadBinaryFromMemory(&model, &err, &warn, data, static_cast<unsigned int>(size)))
      {
         PLOGE << "Failed to load GLB mesh: " << err;
         return false;
      }
      if (!warn.empty())
      {
         PLOGW << "GLB mesh loading warnings: " << warn;
      }

      // Find the first triangle mesh primitive
      const tinygltf::Primitive* prim = nullptr;
      for (const tinygltf::Mesh& mesh : model.meshes)
         for (const tinygltf::Primitive& p : mesh.primitives)
            if (prim == nullptr && (p.mode == -1 || p.mode == TINYGLTF_MODE_TRIANGLES))
               prim = &p;
      if (prim == nullptr)
      {
         PLOGE << "GLB mesh contains no triangle primitive";
         return false;
      }

      const auto getAttrib = [&prim](const char* name) -> int
      {
         const auto it = prim->attributes.find(name);
         return (it == prim->attributes.end()) ? -1 : it->second;
      };
      std::vector<float> positions, normals, texcoords;
      const int posIndex = getAttrib("POSITION");
      const int nrmIndex = getAttrib("NORMAL");
      const int texIndex = getAttrib("TEXCOORD_0");
      if (!ReadFloatAccessor(model, posIndex, positions, 3))
      {
         PLOGE << "GLB mesh has no position attribute";
         return false;
      }
      const bool hasNormals = ReadFloatAccessor(model, nrmIndex, normals, 3);
      const bool hasTexcoords = ReadFloatAccessor(model, texIndex, texcoords, 2);
      const size_t numVertices = positions.size() / 3;
      if (hasNormals && normals.size() != positions.size())
      {
         PLOGE << "GLB mesh has inconsistent attribute vertex counts";
         return false;
      }

      // Indices (winding is reversed to get back to the left handed VPX convention)
      if (prim->indices < 0 || prim->indices >= static_cast<int>(model.accessors.size()))
      {
         PLOGE << "GLB mesh primitive has no index buffer";
         return false;
      }
      const tinygltf::Accessor& idxAccessor = model.accessors[prim->indices];
      const tinygltf::BufferView& idxView = model.bufferViews[idxAccessor.bufferView];
      const tinygltf::Buffer& idxBuffer = model.buffers[idxView.buffer];
      const size_t idxBase = idxView.byteOffset + idxAccessor.byteOffset;
      const size_t idxCompSize = (idxAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) ? 1 : (idxAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) ? 2 : 4;
      const size_t idxStride = idxView.byteStride ? idxView.byteStride : idxCompSize;
      m_indices.resize(idxAccessor.count);
      for (size_t i = 0; i < idxAccessor.count; i += 3)
      {
         uint32_t tri[3] = { 0, 0, 0 };
         for (size_t j = 0; j < 3; j++)
         {
            const size_t ofs = idxBase + (i + j) * idxStride;
            if (idxCompSize == 1)
               tri[j] = idxBuffer.data[ofs];
            else if (idxCompSize == 2)
               tri[j] = *reinterpret_cast<const uint16_t*>(&idxBuffer.data[ofs]);
            else
               tri[j] = *reinterpret_cast<const uint32_t*>(&idxBuffer.data[ofs]);
         }
         m_indices[i + 0] = tri[0];
         m_indices[i + 1] = tri[2];
         m_indices[i + 2] = tri[1];
      }

      m_vertices.resize(numVertices);
      for (size_t i = 0; i < numVertices; i++)
      {
         Vertex3D_NoTex2& v = m_vertices[i];
         const vec3 pos = GltfToVpx(positions[i * 3 + 0], positions[i * 3 + 1], positions[i * 3 + 2]);
         v.x = pos.x;
         v.y = pos.y;
         v.z = pos.z;
         const vec3 nrm = hasNormals ? GltfToVpx(normals[i * 3 + 0], normals[i * 3 + 1], normals[i * 3 + 2]) : vec3(0.f, 0.f, 1.f);
         v.nx = nrm.x;
         v.ny = nrm.y;
         v.nz = nrm.z;
         v.tu = hasTexcoords ? texcoords[i * 2 + 0] : 0.f;
         v.tv = hasTexcoords ? 1.f - texcoords[i * 2 + 1] : 0.f;
      }

      // Animation frames from morph targets
      m_animationFrames.clear();
      for (const std::map<std::string, int>& target : prim->targets)
      {
         const auto posIt = target.find("POSITION");
         if (posIt == target.end())
            continue;
         std::vector<float> deltaPos, deltaNrm;
         if (!ReadFloatAccessor(model, posIt->second, deltaPos, 3) || deltaPos.size() != positions.size())
            continue;
         const auto nrmIt = target.find("NORMAL");
         if (nrmIt != target.end() && (!ReadFloatAccessor(model, nrmIt->second, deltaNrm, 3) || deltaNrm.size() != positions.size()))
            deltaNrm.clear();
         FrameData frame;
         frame.m_frameVerts.resize(numVertices);
         for (size_t i = 0; i < numVertices; i++)
         {
            VertData& fv = frame.m_frameVerts[i];
            const vec3 pos = GltfToVpx(positions[i * 3 + 0] + deltaPos[i * 3 + 0], positions[i * 3 + 1] + deltaPos[i * 3 + 1], positions[i * 3 + 2] + deltaPos[i * 3 + 2]);
            fv.x = pos.x;
            fv.y = pos.y;
            fv.z = pos.z;
            const vec3 nrm = deltaNrm.empty() ? vec3(m_vertices[i].nx, m_vertices[i].ny, m_vertices[i].nz)
                                              : GltfToVpx(normals[i * 3 + 0] + deltaNrm[i * 3 + 0], normals[i * 3 + 1] + deltaNrm[i * 3 + 1], normals[i * 3 + 2] + deltaNrm[i * 3 + 2]);
            fv.nx = nrm.x;
            fv.ny = nrm.y;
            fv.nz = nrm.z;
         }
         m_animationFrames.push_back(std::move(frame));
      }

      UpdateBounds();
      return true;
   }
   catch (const std::exception& e)
   {
      PLOGE << "Failed to load GLB mesh: " << e.what();
      return false;
   }
}
