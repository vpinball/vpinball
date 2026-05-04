// license:GPLv3+

#pragma once

class ModelViewProj final
{
public:
   enum FlipMode { NONE, FLIPX, FLIPY };
   ModelViewProj(const unsigned int nEyes = 1) : m_nEyes(nEyes) {}

   void SetModel(const Matrix3D& model) { SetWithDirty(model, m_matModel); }
   void SetReflection(const Matrix3D& reflection) { SetWithDirty(reflection, m_matReflection); }
   void SetView(const unsigned int index, const Matrix3D& view) { SetWithDirty(view, m_matView[index]); }
   void SetProj(const unsigned int index, const Matrix3D& proj) { SetWithDirty(proj, m_matProj[index]); }
   void SetFlip(const FlipMode flip) { m_dirty = true; m_flip = flip; }

   void Set(const ModelViewProj& mvp) {
      assert(m_nEyes == mvp.m_nEyes);
      m_flip = mvp.m_flip;
      m_matModel = mvp.m_matModel;
      m_matReflection = mvp.m_matReflection;
      for (unsigned int i = 0; i < m_nEyes; i++)
      {
         m_matView[i] = mvp.m_matView[i];
         m_matProj[i] = mvp.m_matProj[i];
      }
      m_dirty = mvp.m_dirty;
      if (!m_dirty)
      {
         for (unsigned int i = 0; i < m_nEyes; i++)
         {
            m_matReflectedView[i] = mvp.m_matReflectedView[i];
            m_matModelView[i] = mvp.m_matModelView[i];
            m_matModelViewInverse[i] = mvp.m_matModelViewInverse[i];
            m_matModelViewInverseTranspose[i] = mvp.m_matModelViewInverseTranspose[i];
            m_matModelViewProj[i] = mvp.m_matModelViewProj[i];
            m_viewVec[i] = mvp.m_viewVec[i];
         }
      }
   }

   const Matrix3D& GetModel() const { return m_matModel; }
   const Matrix3D& GetView(const unsigned int eye) const { Update(); return m_matReflectedView[eye]; }
   const Matrix3D& GetProj(const unsigned int eye) const { return m_matProj[eye]; }
   const Matrix3D& GetModelView(const unsigned int eye) const { Update(); return m_matModelView[eye]; }
   const Matrix3D& GetModelViewInverse(const unsigned int eye) const { Update(); return m_matModelViewInverse[eye]; }
   const Matrix3D& GetModelViewInverseTranspose(const unsigned int eye) const { Update(); return m_matModelViewInverseTranspose[eye]; }
   const Matrix3D& GetModelViewProj(const unsigned int eye) const { Update(); return m_matModelViewProj[eye]; }
   const Vertex3Ds& GetViewVec(const unsigned int eye) const { Update(); return m_viewVec[eye]; }
   const Matrix3D& GetRotViewProj(const unsigned int eye) const { Update(); return m_matRotViewProj[eye]; }
   const vec4& GetCameraPos(const unsigned int eye) const { Update(); return m_cameraPos[eye]; }

   const unsigned int m_nEyes;

private:
   void SetWithDirty(const Matrix3D& newMat, Matrix3D& localMat)
   {
      if (m_dirty)
         localMat = newMat;
      else if (memcmp(localMat.m, newMat.m, 4 * 4 * sizeof(float)) != 0)
      {
         m_dirty = true;
         localMat = newMat;
      }
   }

   void Update() const
   {
      if (m_dirty)
      {
         m_dirty = false;
         for (unsigned int eye = 0; eye < m_nEyes; eye++)
         {
            m_matReflectedView[eye] = m_matReflection * m_matView[eye];
            m_matModelView[eye] = m_matModel * m_matReflectedView[eye];
            m_matModelViewProj[eye] = m_matModelView[eye] * m_matProj[eye];

            m_matModelViewInverse[eye] = Matrix3D::MatrixInverse(m_matModelView[eye]);
            m_matModelViewInverseTranspose[eye] = m_matModelViewInverse[eye];
            m_matModelViewInverseTranspose[eye].Transpose();

            const Matrix3D viewRot = Matrix3D::MatrixInverse(m_matReflectedView[eye]).GetRotationPart();
            m_viewVec[eye] = viewRot * Vertex3Ds { 0.f, 0.f, 1.f };
            m_viewVec[eye].NormalizeSafe();

            const Vertex3Ds camPos = Matrix3D::MatrixInverse(m_matReflectedView[eye]).GetOrthoNormalPos();
            m_cameraPos[eye] = vec4(camPos.x, camPos.y, camPos.z, 0.f);
            m_matRotViewProj[eye] = m_matReflectedView[eye].GetRotationPart() * GetProj(eye);
         }

         // Flip is a clipspace flip, applied after projection
         switch (m_flip)
         {
         case NONE: break;
         case FLIPX:
         {
            static constexpr Matrix3D flipx{-1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
            for (unsigned int eye = 0; eye < m_nEyes; eye++)
               m_matModelViewProj[eye] = m_matModelViewProj[eye] * flipx;
            break;
         }
         case FLIPY:
         {
            static constexpr Matrix3D flipy{1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
            for (unsigned int eye = 0; eye < m_nEyes; eye++)
               m_matModelViewProj[eye] = m_matModelViewProj[eye] * flipy;
            break;
         }
         default: assert(false); break;
         }
      }
   }

   Matrix3D m_matModel { Matrix3D::MatrixIdentity() };
   Matrix3D m_matReflection { Matrix3D::MatrixIdentity() };
   Matrix3D m_matView[2];
   Matrix3D m_matProj[2];

   mutable bool m_dirty = true;
   FlipMode m_flip = NONE;
   mutable Matrix3D m_matReflectedView[2];
   mutable Matrix3D m_matModelView[2];
   mutable Matrix3D m_matModelViewInverse[2];
   mutable Matrix3D m_matModelViewInverseTranspose[2];
   mutable Matrix3D m_matModelViewProj[2];
   mutable Matrix3D m_matRotViewProj[2];
   mutable vec4 m_cameraPos[2];
   mutable Vertex3Ds m_viewVec[2];
};
