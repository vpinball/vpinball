// license:GPLv3+

#pragma once

class ModelViewProj final
{
public:
   enum FlipMode { NONE, FLIPX, FLIPY };
   ModelViewProj(const unsigned int nEyes = 1) : m_nEyes(nEyes) {}

   void SetFlip(const FlipMode flip) { m_dirty = true; m_flip = flip; }
   void SetModel(const Matrix3D& Model) { MarkDirty(Model, m_matModel); m_matModel = Model; }
   void SetView(const Matrix3D& view) { MarkDirty(view, m_matView); m_matView = view; }
   void SetProj(const unsigned int index, const Matrix3D& proj) { MarkDirty(proj, m_matProj[index]); m_matProj[index] = proj; }

   const Matrix3D& GetModel() const { return m_matModel; }
   const Matrix3D& GetView() const { return m_matView; }
   const Matrix3D& GetProj(const unsigned int eye) const { return m_matProj[eye]; }
   const Matrix3D& GetModelView() const { Update(); return m_matModelView; }
   const Matrix3D& GetModelViewInverse() const { Update(); return m_matModelViewInverse; }
   const Matrix3D& GetModelViewInverseTranspose() const { Update(); return m_matModelViewInverseTranspose; }
   const Matrix3D& GetModelViewProj(const unsigned int eye) const { Update(); return m_matModelViewProj[eye]; }
   const Vertex3Ds& GetViewVec() const { Update(); return m_viewVec; }

   const unsigned int m_nEyes;

private:
   void MarkDirty(const Matrix3D& newMat, const Matrix3D& oldMat)
   {
      if (!m_dirty)
         m_dirty = memcmp(oldMat.m, newMat.m, 4 * 4 * sizeof(float)) != 0;
   }

   void Update() const
   {
      if (m_dirty)
      {
         m_dirty = false;
         m_matModelViewInverse = m_matModelView = m_matModel * m_matView;
         m_matModelViewInverse.Invert();
         m_matModelViewInverseTranspose = m_matModelViewInverse;
         m_matModelViewInverseTranspose.Transpose();
         switch (m_flip)
         {
         case NONE:
         {
            for (unsigned int eye = 0; eye < m_nEyes; eye++)
               m_matModelViewProj[eye] = m_matModelView * m_matProj[eye];
            break;
         }
         case FLIPX:
         {
            static const Matrix3D flipx{-1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
            for (unsigned int eye = 0; eye < m_nEyes; eye++)
               m_matModelViewProj[eye] = m_matModelView * m_matProj[eye] * flipx;
            break;
         }
         case FLIPY:
         {
            static const Matrix3D flipy{1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
            for (unsigned int eye = 0; eye < m_nEyes; eye++)
               m_matModelViewProj[eye] = m_matModelView * m_matProj[eye] * flipy;
            break;
         }
         }
         Matrix3D temp = m_matView;
         temp.Invert();
         const Matrix3D viewRot = temp.GetRotationPart();
         m_viewVec = viewRot * Vertex3Ds{0, 0, 1};
         m_viewVec.NormalizeSafe();
      }
   }

   Matrix3D m_matModel;
   Matrix3D m_matView;
   Matrix3D m_matProj[6];

   mutable bool m_dirty = true;
   FlipMode m_flip = NONE;
   mutable Matrix3D m_matModelView;
   mutable Matrix3D m_matModelViewInverse;
   mutable Matrix3D m_matModelViewInverseTranspose;
   mutable Matrix3D m_matModelViewProj[6];
   mutable Vertex3Ds m_viewVec;
};
