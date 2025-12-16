// license:GPLv3+

#include "core/stdafx.h"
#include "MeshBuffer.h"
#include "RenderDevice.h"

#include "Shader.h"
#include "VertexBuffer.h"

#include <locale>
#include <codecvt>

MeshBuffer::MeshBuffer(VertexBuffer* vb, IndexBuffer* ib, const bool applyVertexBufferOffsetToIndexBuffer)
   : MeshBuffer(L""s, vb, ib, applyVertexBufferOffsetToIndexBuffer)
{
}

MeshBuffer::MeshBuffer(const wstring& name, VertexBuffer* vb, IndexBuffer* ib, const bool applyVertexBufferOffsetToIndexBuffer)
   : m_wname(name)
   , m_vb(vb)
   , m_ib(ib)
   , m_isVBOffsetApplied(applyVertexBufferOffsetToIndexBuffer)
{
   if (m_ib != nullptr && applyVertexBufferOffsetToIndexBuffer)
      m_ib->ApplyOffset(m_vb);
}

MeshBuffer::~MeshBuffer()
{
#if defined(ENABLE_OPENGL)
   if (m_sharedVAO != nullptr)
   {
      m_sharedVAO->ref_count--;
      if (m_sharedVAO->ref_count == 0)
      {
         glDeleteVertexArrays(1, &m_sharedVAO->vao);
         RemoveFromVectorSingle(m_vb->m_rd->m_sharedVAOs, m_sharedVAO);
         delete m_sharedVAO;
      }
   }
   else if (m_vao != 0)
   {
      glDeleteVertexArrays(1, &m_vao);
   }
#endif
   if (!m_sharedVB)
      delete m_vb;
   delete m_ib;
}

unsigned int MeshBuffer::GetSortKey() const
{
   #if defined(ENABLE_BGFX)
   if (m_ib)
      return ((m_vb->m_isStatic ? m_vb->GetStaticBuffer().idx : m_vb->GetDynamicBuffer().idx)
            ^ (m_ib->m_isStatic ? m_ib->GetStaticBuffer().idx : m_ib->GetDynamicBuffer().idx));
   else
      return m_vb->m_isStatic ? m_vb->GetStaticBuffer().idx : m_vb->GetDynamicBuffer().idx;
   #elif defined(ENABLE_OPENGL)
   return m_vao;
   #elif defined(ENABLE_DX9)
   return (unsigned int) ((reinterpret_cast<uintptr_t>(m_vb)) ^ (reinterpret_cast<uintptr_t>(m_ib)));
   #endif
}

std::unique_ptr<MeshBuffer> MeshBuffer::CreateSharedVertexMeshBuffer(IndexBuffer* ib) const
{
   MeshBuffer* meshBuffer = new MeshBuffer(m_wname, m_vb, ib, true);
   meshBuffer->m_sharedVB = true;
   return std::unique_ptr<MeshBuffer>(meshBuffer);
}

std::unique_ptr<MeshBuffer> MeshBuffer::CreateEdgeMeshBuffer(const vector<unsigned int>& indices) const
{
   vector<unsigned int> edgeIndices(indices.size() * 2);
   for (size_t i = 0; i < indices.size(); i += 3)
   {
      edgeIndices[i * 2 + 0] = indices[i + 0];
      edgeIndices[i * 2 + 1] = indices[i + 1];
      edgeIndices[i * 2 + 2] = indices[i + 1];
      edgeIndices[i * 2 + 3] = indices[i + 2];
      edgeIndices[i * 2 + 4] = indices[i + 2];
      edgeIndices[i * 2 + 5] = indices[i + 0];
   }
   return CreateSharedVertexMeshBuffer(new IndexBuffer(m_vb->m_rd, edgeIndices));
}

std::unique_ptr<MeshBuffer> MeshBuffer::CreateEdgeMeshBuffer(const vector<unsigned int>& indices, const vector<Vertex3D_NoTex2>& vertices) const
{
   union idStruct
   {
      uint64_t id;
      struct
      {
         uint32_t edge1;
         uint32_t edge2;
      } edges;
   };
   struct Edge
   {
      int state; // 0 = one edge, 1 = multiple colinear edges, 2 multiple non colinear edges
      vec3 normal;
   };
   ankerl::unordered_dense::map<uint64_t, Edge> edges;
   auto pushEdge = [&](unsigned int i1, unsigned int i2, const vec3& normal) {
      idStruct id;
      if (i1 < i2)
      {
         id.edges.edge1 = i1;
         id.edges.edge2 = i2;
      }
      else
      {
         id.edges.edge1 = i2;
         id.edges.edge2 = i1;
      }
      if (auto it = edges.find(id.id); it != edges.end())
      {
         if (it->second.state < 2)
         {
            const float colinear = normal.Dot(it->second.normal);
            if (colinear < 0.99f)
               it->second.state = 2; // Edge: render
            else
               it->second.state = 1; // Internal colinear edge: do not render
         }
      }
      else
      {
         edges[id.id] = { 0, normal }; // Edge: render
      }
   };
   for (size_t i = 0; i < indices.size(); i += 3)
   {
      Vertex3D_NoTex2 a = vertices[indices[i + 0]];
      Vertex3D_NoTex2 b = vertices[indices[i + 1]];
      Vertex3D_NoTex2 c = vertices[indices[i + 2]];
      vec3 pa(a.x, a.y, a.z);
      vec3 pb(b.x, b.y, b.z);
      vec3 pc(c.x, c.y, c.z);
      vec3 ab = pb - pa;
      vec3 ac = pc - pa;
      vec3 normal = CrossProduct(ab, ac);
      float lengthSquared = normal.LengthSquared();
      if (lengthSquared <= FLT_MIN)
         continue;
      const float oneoverlength = 1.0f / sqrtf(lengthSquared);
      normal.x *= oneoverlength;
      normal.y *= oneoverlength;
      normal.z *= oneoverlength;
      pushEdge(indices[i + 0], indices[i + 1], normal);
      pushEdge(indices[i + 1], indices[i + 2], normal);
      pushEdge(indices[i + 2], indices[i + 0], normal);
   }
   vector<unsigned int> edgeIndices;
   for (const auto& [edge, state] : edges)
   {
      if (state.state != 1)
      {
         idStruct id;
         id.id = edge;
         edgeIndices.push_back(id.edges.edge1);
         edgeIndices.push_back(id.edges.edge2);
      }
   }
   return CreateSharedVertexMeshBuffer(new IndexBuffer(m_vb->m_rd, edgeIndices));
}

void MeshBuffer::bind()
{
#if defined(ENABLE_BGFX)
   m_vb->Upload();
   /* if (m_vb->m_isStatic)
      bgfx::setVertexBuffer(0, m_vb->GetStaticBuffer(), m_vb->GetVertexOffset(), m_vb->m_vertexCount);
   else
      bgfx::setVertexBuffer(0, m_vb->GetDynamicBuffer(), m_vb->GetVertexOffset(), m_vb->m_vertexCount); */
   if (m_ib != nullptr)
   {
      m_ib->Upload();
      /*if (m_ib->m_isStatic)
         bgfx::setIndexBuffer(m_ib->GetStaticBuffer(), m_ib->GetIndexOffset(), m_ib->m_indexCount);
      else
         bgfx::setIndexBuffer(m_ib->GetDynamicBuffer(), m_ib->GetIndexOffset(), m_ib->m_indexCount);*/
   }

#elif defined(ENABLE_OPENGL)
   RenderDevice* const rd = m_vb->m_rd;
   if (m_vao == 0)
   {
      // If index & vertex buffer are using shared buffers (for static objects), then we can also use a shared VAO
      bool isShared = m_vb->IsSharedBuffer() && (m_ib == nullptr || m_ib->IsSharedBuffer());
      if (isShared)
      {
         GLuint vb = m_vb->GetBuffer(), ib = m_ib == nullptr ? 0 : m_ib->GetBuffer();
         std::vector<SharedVAO*>::iterator existing
            = std::find_if(m_vb->m_rd->m_sharedVAOs.begin(), m_vb->m_rd->m_sharedVAOs.end(), [vb, ib](SharedVAO* v) { return v->vb == vb && v->ib == ib; });
         if (existing != m_vb->m_rd->m_sharedVAOs.end())
         {
            m_sharedVAO = *existing;
            m_sharedVAO->ref_count++;
            m_vao = m_sharedVAO->vao;
         }
      }
      if (m_vao == 0)
      {
         glGenVertexArrays(1, &m_vao);
         glBindVertexArray(m_vao);
         rd->m_curVAO = m_vao;
         m_vb->Upload();
         m_vb->Bind();
         if (m_ib != nullptr)
         {
            m_ib->Upload();
            m_ib->Bind();
         }
         // this needs the attribute layout to be enforced in the shaders using layout(location=...)
         switch (m_vb->m_vertexFormat)
         {
         case VertexFormat::VF_POS_NORMAL_TEX:
            glEnableVertexAttribArray(0); // Position
            glEnableVertexAttribArray(1); // Normal
            glEnableVertexAttribArray(2); // Texture Coordinate
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 32, (void*)0);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 32, (void*)12);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 32, (void*)24);
            break;
         case VertexFormat::VF_POS_TEX:
            glEnableVertexAttribArray(0); // Position
            glEnableVertexAttribArray(1); // Texture Coordinate
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 20, (void*)0);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 20, (void*)12);
            break;
         default: assert(false); // Unsupported FVF
         }
         if (isShared)
         {
            m_sharedVAO = new SharedVAO { m_vb->GetBuffer(), m_ib == nullptr ? 0 : m_ib->GetBuffer(), m_vao, 1 };
            m_vb->m_rd->m_sharedVAOs.push_back(m_sharedVAO);
         }
      }
   }
   if (rd->m_curVAO != m_vao)
   {
      glBindVertexArray(m_vao);
      rd->m_curVAO = m_vao;
   }
   m_vb->Upload();
   if (m_ib != nullptr)
   {
      m_ib->Upload();
      // For some reason, it seems the index buffer binding is not stored in the VAO (likely the binding to the VAO is lost somewhere, but where ?)
      // This can be easily reproduced by commenting the IB binding and running Flupper's Totan 1.5 in camera mode. It should crash directly
      m_ib->Bind(); 
   }

   #if defined(DEBUG) && 0
   GLint current_vao;
   glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &current_vao);
   assert(current_vao == m_vao);
   GLint enabled;
   glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);
   assert(enabled);
   glGetVertexAttribiv(1, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);
   assert(enabled);
   glGetVertexAttribiv(2, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);
   assert(m_vb->m_vertexFormat == VertexFormat::VF_POS_NORMAL_TEX ? enabled : !enabled);
   GLint buffer;
   glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &buffer);
   assert(buffer == m_vb->GetBuffer());
   glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &buffer);
   assert(m_ib == nullptr ? buffer == 0 : buffer == m_ib->GetBuffer());
   #endif

#elif defined(ENABLE_DX9)
   m_vb->Upload();
   m_vb->Bind();
   if (m_ib != nullptr)
   {
      m_ib->Upload();
      m_ib->Bind();
   }
#endif
}
