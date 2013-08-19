#include "StdAfx.h"

/////////////////////////////////////////////////////////////////////////////

Ramp::Ramp()
{
   m_menuid = IDR_SURFACEMENU;
   m_d.m_fCollidable = fTrue;
   m_d.m_IsVisible = fTrue;
   //invalidationRectCalculated = false;
   staticVertexBuffer = 0;
   dynamicVertexBuffer = 0;
   dynamicVertexBufferRegenerate = true;
}

Ramp::~Ramp()
{
	if(staticVertexBuffer) {
		staticVertexBuffer->release();
		staticVertexBuffer = 0;
	}

	if(dynamicVertexBuffer) {
		dynamicVertexBuffer->release();
		dynamicVertexBuffer = 0;
		//dynamicVertexBufferRegenerate = true;

		delete [] rgvbuf;
        delete [] rgibuf;
        delete [] invrgibuf;
	}
}

HRESULT Ramp::Init(PinTable *ptable, float x, float y, bool fromMouseClick)
{
   m_ptable = ptable;
   m_d.m_IsVisible = fTrue;

   HRESULT hr;
   float fTmp;

   hr = GetRegStringAsFloat("DefaultProps\\Ramp", "Length", &fTmp);
   float length = 200.0f;
   if (hr == S_OK)
      length = fTmp*0.5f;

   CComObject<DragPoint> *pdp;
   CComObject<DragPoint>::CreateInstance(&pdp);
   if (pdp)
   {
      pdp->AddRef();
      pdp->Init(this, x, y+length);
      pdp->m_fSmooth = fTrue;
      m_vdpoint.AddElement(pdp);
   }

   CComObject<DragPoint>::CreateInstance(&pdp);
   if (pdp)
   {
      pdp->AddRef();
      pdp->Init(this, x, y-length);
      pdp->m_fSmooth = fTrue;
      m_vdpoint.AddElement(pdp);
   }

   SetDefaults(fromMouseClick);

   InitVBA(fTrue, 0, NULL);

   return S_OK;
}

void Ramp::SetDefaults(bool fromMouseClick)
{
   HRESULT hr;
   float fTmp;
   int iTmp;

   hr = GetRegStringAsFloat("DefaultProps\\Ramp","HeightBottom", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_heightbottom = fTmp;
   else
      m_d.m_heightbottom = 0;

   hr = GetRegStringAsFloat("DefaultProps\\Ramp","HeightTop", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_heighttop = fTmp;
   else
      m_d.m_heighttop = 100.0f;

   hr = GetRegStringAsFloat("DefaultProps\\Ramp","WidthBottom", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_widthbottom = fTmp;
   else
      m_d.m_widthbottom = 75.0f;

   hr = GetRegStringAsFloat("DefaultProps\\Ramp","WidthTop", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_widthtop = fTmp;
   else
      m_d.m_widthtop = 60.0f;

   hr = GetRegInt("DefaultProps\\Ramp","Color", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_color = iTmp;
   else
      m_d.m_color = RGB(50,200,50);

   hr = GetRegInt("DefaultProps\\Ramp","RampType", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_type = (enum RampType)iTmp;
   else
      m_d.m_type = RampTypeFlat;

   hr = GetRegInt("DefaultProps\\Ramp","TimerEnabled", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_tdr.m_fTimerEnabled = iTmp == 0 ? false : true;
   else
      m_d.m_tdr.m_fTimerEnabled = false;

   hr = GetRegInt("DefaultProps\\Ramp","TimerInterval", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_tdr.m_TimerInterval = iTmp;
   else
      m_d.m_tdr.m_TimerInterval = 100;

   hr = GetRegString("DefaultProps\\Ramp","Image", m_d.m_szImage, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szImage[0] = 0;

   hr = GetRegInt("DefaultProps\\Ramp","ImageMode", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_imagealignment = (enum RampImageAlignment)iTmp;
   else
      m_d.m_imagealignment = ImageModeWorld;

   hr = GetRegInt("DefaultProps\\Ramp","ImageWalls", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fImageWalls = iTmp == 0 ? false : true;
   else
      m_d.m_fImageWalls = fTrue;

   hr = GetRegInt("DefaultProps\\Ramp","CastsShadow", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fCastsShadow = iTmp == 0 ? false : true;
   else
      m_d.m_fCastsShadow = fTrue;

   hr = GetRegInt("DefaultProps\\Ramp","Acrylic", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fAcrylic = iTmp == 0 ? false : true;
   else
      m_d.m_fAcrylic = fFalse;

   hr = GetRegInt("DefaultProps\\Ramp","Alpha", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fAlpha = iTmp == 0 ? false : true;
   else
      m_d.m_fAlpha = fFalse;
   if (m_d.m_fAlpha) 
      m_d.m_fAcrylic = true;   // A alpha Ramp is automatically acrylic.

   hr = GetRegStringAsFloat("DefaultProps\\Ramp","LeftWallHeight", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_leftwallheight = fTmp;
   else
      m_d.m_leftwallheight = 62.0f;

   hr = GetRegStringAsFloat("DefaultProps\\Ramp","RightWallHeight", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_rightwallheight = fTmp;
   else
      m_d.m_rightwallheight = 62.0f;

   hr = GetRegStringAsFloat("DefaultProps\\Ramp","LeftWallHeightVisible", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_leftwallheightvisible = fTmp;
   else
      m_d.m_leftwallheightvisible = 30.0f;

   hr = GetRegStringAsFloat("DefaultProps\\Ramp","RightWallHeightVisible", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_rightwallheightvisible = fTmp;
   else
      m_d.m_rightwallheightvisible = 30.0f;

   hr = GetRegStringAsFloat("DefaultProps\\Ramp","Elasticity", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_elasticity = fTmp;
   else
      m_d.m_elasticity = 0.3f;

   hr = GetRegStringAsFloat("DefaultProps\\Ramp","Friction", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_friction = fTmp;
   else
      m_d.m_friction = 0;	//zero uses global value

   hr = GetRegStringAsFloat("DefaultProps\\Ramp","Scatter", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_scatter = fTmp;
   else
      m_d.m_scatter = 0;	//zero uses global value

   hr = GetRegInt("DefaultProps\\Ramp","Collidable", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fCollidable = iTmp == 0 ? false : true;
   else
      m_d.m_fCollidable = fTrue;

   hr = GetRegInt("DefaultProps\\Ramp","Visible", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_IsVisible = iTmp == 0 ? false : true;
   else
      m_d.m_IsVisible = fTrue;

   hr = GetRegInt("DefaultProps\\Ramp","Modify3DStereo", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fModify3DStereo = iTmp == 0 ? false : true;
   else
      m_d.m_fModify3DStereo = fTrue;

   hr = GetRegInt("DefaultProps\\Ramp","AddBlend", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fAddBlend = iTmp == 0 ? false : true;
   else
      m_d.m_fAddBlend = fFalse;
}

void Ramp::WriteRegDefaults()
{
   char strTmp[40];

   sprintf_s(strTmp, 40, "%f", m_d.m_heightbottom);
   SetRegValue("DefaultProps\\Ramp","HeightBottom", REG_SZ, &strTmp,strlen(strTmp));
   sprintf_s(strTmp, 40, "%f", m_d.m_heighttop);
   SetRegValue("DefaultProps\\Ramp","HeightTop", REG_SZ, &strTmp,strlen(strTmp));
   sprintf_s(strTmp, 40, "%f", m_d.m_widthbottom);
   SetRegValue("DefaultProps\\Ramp","WidthBottom", REG_SZ, &strTmp,strlen(strTmp));
   sprintf_s(strTmp, 40, "%f", m_d.m_widthtop);
   SetRegValue("DefaultProps\\Ramp","WidthTop", REG_SZ, &strTmp,strlen(strTmp));
   SetRegValue("DefaultProps\\Ramp","Color",REG_DWORD,&m_d.m_color,4);
   SetRegValue("DefaultProps\\Ramp","RampType",REG_DWORD,&m_d.m_type,4);
   SetRegValue("DefaultProps\\Ramp","TimerEnabled",REG_DWORD,&m_d.m_tdr.m_fTimerEnabled,4);
   SetRegValue("DefaultProps\\Ramp","TimerInterval",REG_DWORD,&m_d.m_tdr.m_TimerInterval,4);
   SetRegValue("DefaultProps\\Ramp","Image", REG_SZ, &m_d.m_szImage, strlen(m_d.m_szImage));
   SetRegValue("DefaultProps\\Ramp","ImageMode",REG_DWORD,&m_d.m_imagealignment,4);
   SetRegValue("DefaultProps\\Ramp","ImageWalls",REG_DWORD,&m_d.m_fImageWalls,4);
   SetRegValue("DefaultProps\\Ramp","CastsShadow",REG_DWORD,&m_d.m_fCastsShadow,4);
   SetRegValue("DefaultProps\\Ramp","Acrylic",REG_DWORD,&m_d.m_fAcrylic,4);
   SetRegValue("DefaultProps\\Ramp","Alpha",REG_DWORD,&m_d.m_fAlpha,4);
   sprintf_s(strTmp, 40, "%f", m_d.m_leftwallheight);
   SetRegValue("DefaultProps\\Ramp","LeftWallHeight", REG_SZ, &strTmp,strlen(strTmp));
   sprintf_s(strTmp, 40, "%f", m_d.m_rightwallheight);
   SetRegValue("DefaultProps\\Ramp","RightWallHeight", REG_SZ, &strTmp,strlen(strTmp));
   SetRegValue("DefaultProps\\Ramp","LeftWallHeightVisible",REG_DWORD,&m_d.m_leftwallheightvisible,4);
   SetRegValue("DefaultProps\\Ramp","RightWallHeightVisible",REG_DWORD,&m_d.m_rightwallheightvisible,4);
   sprintf_s(strTmp, 40, "%f", m_d.m_elasticity);
   SetRegValue("DefaultProps\\Ramp","Elasticity", REG_SZ, &strTmp,strlen(strTmp));	
   sprintf_s(strTmp, 40, "%f", m_d.m_friction);
   SetRegValue("DefaultProps\\Ramp","Friction", REG_SZ, &strTmp,strlen(strTmp));	
   sprintf_s(strTmp, 40, "%f", m_d.m_scatter);
   SetRegValue("DefaultProps\\Ramp","Scatter", REG_SZ, &strTmp,strlen(strTmp));	
   SetRegValue("DefaultProps\\Ramp","Collidable",REG_DWORD,&m_d.m_fCollidable,4);
   SetRegValue("DefaultProps\\Ramp","Visible",REG_DWORD,&m_d.m_IsVisible,4);
   SetRegValue("DefaultProps\\Ramp","Modify3DStereo",REG_DWORD,&m_d.m_fModify3DStereo,4);
   SetRegValue("DefaultProps\\Ramp","AddBlend",REG_DWORD,&m_d.m_fAddBlend,4);
}

void Ramp::PreRender(Sur * const psur)
{
   //make 1 wire ramps look unique in editor - uses ramp color
   psur->SetFillColor((m_d.m_type == RampType1Wire) ? m_d.m_color : RGB(192,192,192));
   psur->SetBorderColor(-1,false,0);
   psur->SetObject(this);

   int cvertex;
   Vertex2D * const rgvLocal = GetRampVertex(cvertex, NULL, NULL, NULL);

   psur->Polygon(rgvLocal, cvertex*2);

   delete [] rgvLocal;
}

void Ramp::Render(Sur * const psur)
{
   psur->SetFillColor(-1);
   psur->SetBorderColor(RGB(0,0,0),false,0);
   psur->SetLineColor(RGB(0,0,0),false,0);
   psur->SetObject(this);
   psur->SetObject(NULL); // NULL so this won't be hit-tested

   int cvertex;
   bool *pfCross;
   Vertex2D * const rgvLocal = GetRampVertex(cvertex, NULL, &pfCross, NULL);

   psur->Polygon(rgvLocal, cvertex*2);
   for (int i=0;i<cvertex;i++)
      if (pfCross[i])
         psur->Line(rgvLocal[i].x, rgvLocal[i].y, rgvLocal[cvertex*2 - i - 1].x, rgvLocal[cvertex*2 - i - 1].y);

   if (m_d.m_type == RampType4Wire || m_d.m_type == RampType3WireRight)
   {
      psur->SetLineColor(RGB(0,0,0),false,3);
      psur->Polyline(rgvLocal, cvertex);
   }
   if (m_d.m_type == RampType4Wire || m_d.m_type == RampType3WireLeft)
   {
      psur->SetLineColor(RGB(0,0,0),false,3);
      psur->Polyline(&rgvLocal[cvertex], cvertex);
   }

   delete [] rgvLocal;
   delete [] pfCross;

   bool fDrawDragpoints = ( (m_selectstate != eNotSelected) || (g_pvp->m_fAlwaysDrawDragPoints) );
   // if the item is selected then draw the dragpoints (or if we are always to draw dragpoints)
   if (!fDrawDragpoints)
   {
      // if any of the dragpoints of this object are selected then draw all the dragpoints
      for (int i=0;i<m_vdpoint.Size();i++)
      {
         const CComObject<DragPoint> * const pdp = m_vdpoint.ElementAt(i);
         if (pdp->m_selectstate != eNotSelected)
         {
            fDrawDragpoints = true;
            break;
         }
      }
   }

   if (fDrawDragpoints)
   {
      for (int i=0;i<m_vdpoint.Size();i++)
      {
         CComObject<DragPoint> * const pdp = m_vdpoint.ElementAt(i);
         psur->SetFillColor(-1);
         psur->SetBorderColor(RGB(255,0,0),false,0);
         psur->SetObject(pdp);

         if (pdp->m_fDragging)
         {
            psur->SetBorderColor(RGB(0,255,0),false,0);
         }

         psur->Ellipse2(pdp->m_v.x, pdp->m_v.y, 8);
      }
   }
}

void Ramp::RenderOutline(Sur * const psur)
{
   psur->SetFillColor(-1);
   psur->SetBorderColor(RGB(0,0,0),false,0);
   psur->SetLineColor(RGB(0,0,0),false,0);
   psur->SetObject(this);
   psur->SetObject(NULL); // NULL so this won't be hit-tested

   int cvertex;
   bool *pfCross;
   Vertex2D * const rgvLocal = GetRampVertex(cvertex, NULL, &pfCross, NULL);

   psur->Polygon(rgvLocal, cvertex*2);

   for (int i=0;i<cvertex;i++)
   {
      if (pfCross[i])
      {
         psur->Line(rgvLocal[i].x, rgvLocal[i].y, rgvLocal[cvertex*2 - i - 1].x, rgvLocal[cvertex*2 - i - 1].y);
      }
   }

   delete [] rgvLocal;
   delete [] pfCross;
}

void Ramp::RenderBlueprint(Sur *psur)
{
   RenderOutline(psur);
}

void Ramp::RenderShadow(ShadowSur * const psur, const float height)
{
   if (!m_d.m_fCastsShadow || !m_ptable->m_fRenderShadows || !m_d.m_IsVisible) 
      return; //skip render if not visible

   psur->SetFillColor(RGB(0,0,0));
   psur->SetBorderColor(-1,false,0);
   psur->SetLineColor(RGB(0,0,0),false,2);
   psur->SetObject(this);

   float *rgheight1;
   int cvertex;
   Vertex2D * const rgvLocal = GetRampVertex(cvertex, &rgheight1, NULL, NULL);

   // Find the range of vertices to draw a shadow for
   int startvertex = cvertex;
   int stopvertex = 0;
   for (int i=0;i<cvertex;i++)
   {
      if (rgheight1[i] >= height)
      {
         if(i < startvertex)
            startvertex = i;
         stopvertex = i;
      }
   }

   const int range = (stopvertex - startvertex);

   if (range > 0)
   {
      if (m_d.m_type == RampType4Wire 
         || m_d.m_type == RampType1Wire 
         || m_d.m_type == RampType2Wire 
         || m_d.m_type == RampType3WireLeft 
         || m_d.m_type == RampType3WireRight)
      {
         float * const rgheight2 = new float[cvertex];

         for (int i=0;i<cvertex;i++)
            rgheight2[i] = rgheight1[cvertex - i - 1];

         if (m_d.m_type != RampType1Wire)
            psur->PolylineSkew(rgvLocal, cvertex, rgheight1, 0, 0);

         psur->PolylineSkew(&rgvLocal[cvertex], cvertex, rgheight2, 0, 0);

         for (int i=0;i<cvertex;i++)
         {
            rgheight1[i] += 44.0f;
            rgheight2[i] += 44.0f;
         }

         if (m_d.m_type == RampType4Wire || m_d.m_type == RampType3WireRight)
            psur->PolylineSkew(rgvLocal, cvertex, rgheight1, 0, 0);

         if (m_d.m_type == RampType4Wire || m_d.m_type == RampType3WireLeft)
            psur->PolylineSkew(&rgvLocal[cvertex], cvertex, rgheight2, 0, 0);

         delete [] rgheight2;
      }
      else
      {
         Vertex2D * const rgv2 = new Vertex2D[range*2];
         float * const rgheight2 = new float[range*2];

         for (int i=0;i<range;i++)
         {
            rgv2[i] = rgvLocal[i + startvertex];
            rgv2[range*2 - i - 1] = rgvLocal[cvertex*2 - i - 1 - startvertex];
            rgheight2[i] = rgheight1[i + startvertex];
            rgheight2[range*2 - i - 1] = rgheight1[i + startvertex];
         }

         psur->PolygonSkew(rgv2, range*2, rgheight2);

         delete [] rgv2;
         delete [] rgheight2;
      }
   }

   delete [] rgvLocal;
   delete [] rgheight1;
}

void Ramp::GetBoundingVertices(Vector<Vertex3Ds> * const pvvertex3D)
{
   float *rgheight1;
   int cvertex;
   const Vertex2D * const rgvLocal = GetRampVertex(cvertex, &rgheight1, NULL, NULL);

   for (int i=0;i<cvertex;i++)
   {
      {
         Vertex3Ds * const pv = new Vertex3Ds();
         pv->x = rgvLocal[i].x;
         pv->y = rgvLocal[i].y;
         pv->z = rgheight1[i]+50.0f; // leave room for ball
         pvvertex3D->AddElement(pv);
      }

      Vertex3Ds * const pv = new Vertex3Ds();
      pv->x = rgvLocal[cvertex*2-i-1].x;
      pv->y = rgvLocal[cvertex*2-i-1].y;
      pv->z = rgheight1[i]+50.0f; // leave room for ball
      pvvertex3D->AddElement(pv);
   }

   delete [] rgvLocal;
   delete [] rgheight1;
}

Vertex2D *Ramp::GetRampVertex(int &pcvertex, float ** const ppheight, bool ** const ppfCross, float ** const ppratio)
{
   Vector<RenderVertex> vvertex;
   GetRgVertex(&vvertex);

   const int cvertex = vvertex.Size();
   Vertex2D * const rgvLocal = new Vertex2D[cvertex * 2];
   if (ppheight)
   {
      *ppheight = new float[cvertex];
   }
   if (ppfCross)
   {
      *ppfCross = new bool[cvertex];
   }
   if (ppratio)
   {
      *ppratio = new float[cvertex];
   }

   float totallength = 0;
   for (int i=0; i<(cvertex-1); i++)
   {
      const RenderVertex * const pv1 = vvertex.ElementAt(i);
      const RenderVertex * const pv2 = vvertex.ElementAt(i+1);

      const float dx = pv1->x - pv2->x;
      const float dy = pv1->y - pv2->y;
      const float length = sqrtf(dx*dx + dy*dy);

      totallength += length;
   }

   float currentlength = 0;
   for (int i=0; i<cvertex; i++)
   {
      const RenderVertex * const pv1 = vvertex.ElementAt((i>0) ? i-1 : i);
      const RenderVertex * const pv2 = vvertex.ElementAt((i < (cvertex-1)) ? i+1 : i);
      const RenderVertex * const pvmiddle = vvertex.ElementAt(i);

      Vertex2D vnormal;
      {
         // Get normal at this point
         // Notice that these values equal the ones in the line
         // equation and could probably be substituted by them.
         Vertex2D v1normal(pv1->y - pvmiddle->y, pvmiddle->x - pv1->x);
         Vertex2D v2normal(pvmiddle->y - pv2->y, pv2->x - pvmiddle->x);		

         if (i == (cvertex-1))
         {
            v1normal.Normalize();
            vnormal = v1normal;
         }
         else if (i == 0)
         {
            v2normal.Normalize();
            vnormal = v2normal;
         }
         else
         {
            v1normal.Normalize();
            v2normal.Normalize();
            if (fabsf(v1normal.x-v2normal.x) < 0.0001f && fabsf(v1normal.y-v2normal.y) < 0.0001f)
            {
               // Two parallel segments
               v1normal.Normalize();
               vnormal = v1normal;
            }
            else
            {
               v1normal.Normalize();
               v2normal.Normalize();

               // Find intersection of the two edges meeting this points, but
               // shift those lines outwards along their normals

               // First line
               const float A = pv1->y - pvmiddle->y;
               const float B = pvmiddle->x - pv1->x;

               // Shift line along the normal
               const float C = -(A*(pv1->x-v1normal.x) + B*(pv1->y-v1normal.y));

               // Second line
               const float D = pv2->y - pvmiddle->y;
               const float E = pvmiddle->x - pv2->x;

               // Shift line along the normal
               const float F = -(D*(pv2->x-v2normal.x) + E*(pv2->y-v2normal.y));

               const float det = A*E - B*D;
               const float inv_det = (det != 0.0f) ? 1.0f/det : 0.0f;

               const float intersectx = (B*F-E*C)*inv_det;
               const float intersecty = (C*D-A*F)*inv_det;

               //rgvLocal[i].x = intersectx;
               //rgvLocal[i].y = intersecty;

               //vnormal = Calc2DNormal(pv1, pv2);

               vnormal.x = pvmiddle->x - intersectx;
               vnormal.y = pvmiddle->y - intersecty;
            }
         }
      }

      {
         const float dx = pv1->x - pvmiddle->x;
         const float dy = pv1->y - pvmiddle->y;
         const float length = sqrtf(dx*dx + dy*dy);

         currentlength += length;
      }

      const float widthcur = (currentlength/totallength) * (m_d.m_widthtop - m_d.m_widthbottom) + m_d.m_widthbottom;

      if (ppheight)
      {
         const float percentage = 1.0f-(currentlength/totallength); // ramps have no ends ... a line joint is needed
         const float heightcur = (1.0f - percentage) * (m_d.m_heighttop - m_d.m_heightbottom) + m_d.m_heightbottom;
         (*ppheight)[i] = heightcur;
      }

      if (ppratio)
      {
         const float percentage = 1.0f-(currentlength/totallength);
         (*ppratio)[i] = percentage;
      }

      rgvLocal[i].x = pvmiddle->x + vnormal.x * (widthcur*0.5f);
      rgvLocal[i].y = pvmiddle->y + vnormal.y * (widthcur*0.5f);
      rgvLocal[cvertex*2 - i - 1].x = pvmiddle->x - vnormal.x * (widthcur*0.5f);
      rgvLocal[cvertex*2 - i - 1].y = pvmiddle->y - vnormal.y * (widthcur*0.5f);
   }

   if (ppfCross)
   {
      for (int i=0;i<cvertex;i++)
      {
         (*ppfCross)[i] = vvertex.ElementAt(i)->fControlPoint;
      }
   }

   for (int i=0;i<cvertex;i++)
   {
      delete vvertex.ElementAt(i);
   }

   pcvertex = cvertex;
   return rgvLocal;
}

void Ramp::GetRgVertex(Vector<RenderVertex> * const pvv)
{
   const int cpoint = m_vdpoint.Size();
   RenderVertex rendv2;
   if(cpoint < 2) {
      const CComObject<DragPoint> * const pdp = m_vdpoint.ElementAt(0);
      rendv2.x = pdp->m_v.x;
      rendv2.y = pdp->m_v.y;
   }

   // calculate AlphaRampsAccuracyValue
   const float alphaRampsAccuracyValue = 4.0f*powf(10.0f, (10.0f-m_ptable->m_alphaRampsAccuracy)*(float)(1.0/1.5)); // min = 4, max = 4 * 10^(10/1.5) = 18.000.000

   for (int i=0;i<(cpoint-1);i++)
   {
      const CComObject<DragPoint> * const pdp1 = m_vdpoint.ElementAt(i);
      const CComObject<DragPoint> * const pdp2 = m_vdpoint.ElementAt(i+1);
      const CComObject<DragPoint> * const pdp0 = m_vdpoint.ElementAt((i>0 && pdp1->m_fSmooth) ? i-1 : i);
      const CComObject<DragPoint> * const pdp3 = m_vdpoint.ElementAt((i<cpoint-2 && pdp2->m_fSmooth) ? i+2 : (i+1));
      CatmullCurve cc;
      cc.SetCurve(&pdp0->m_v, &pdp1->m_v, &pdp2->m_v, &pdp3->m_v);

      RenderVertex rendv1;
      rendv1.x = pdp1->m_v.x;
      rendv1.y = pdp1->m_v.y;
      rendv1.fSlingshot = (pdp1->m_fSlingshot != 0);
      rendv1.fSmooth = (pdp1->m_fSmooth != 0);
      rendv1.fControlPoint = true;

      // Properties of last point don't matter, because it won't be added to the list on this pass (it'll get added as the first point of the next curve)
      rendv2.x = pdp2->m_v.x;
      rendv2.y = pdp2->m_v.y;

      if (m_d.m_fAlpha)
         RecurseSmoothLineWithAccuracy(&cc, 0, 1, &rendv1, &rendv2, pvv, alphaRampsAccuracyValue);
      else
         RecurseSmoothLine(&cc, 0, 1, &rendv1, &rendv2, pvv);
   }

   // Add the very last point to the list because nobody else added it
   rendv2.fSmooth = true;
   rendv2.fSlingshot = false;
   rendv2.fControlPoint = false;
   RenderVertex * const pvT = new RenderVertex;
   *pvT = rendv2;
   pvv->AddElement(pvT);
}

void Ramp::GetTimers(Vector<HitTimer> * const pvht)
{
   IEditable::BeginPlay();

   HitTimer * const pht = new HitTimer();
   pht->m_interval = m_d.m_tdr.m_TimerInterval;
   pht->m_nextfire = pht->m_interval;
   pht->m_pfe = (IFireEvents *)this;

   m_phittimer = pht;

   if (m_d.m_tdr.m_fTimerEnabled)
   {
      pvht->AddElement(pht);
   }
}

void Ramp::GetHitShapes(Vector<HitObject> * const pvho)
{
   //!! Somehow reduce number of update elements that originate from this?

   int cvertex;
   float *rgheight1;
   Vertex2D * const rgvLocal = GetRampVertex(cvertex, &rgheight1, NULL, NULL);

   float wallheightright, wallheightleft;

   if (m_d.m_type == RampTypeFlat)
   {
      wallheightright = m_d.m_rightwallheight;
      wallheightleft = m_d.m_leftwallheight;
   }
   else if (m_d.m_type == RampType1Wire) //add check for 1 wire
   {
      // backwards compatible physics
      wallheightright = 31.0f;
      wallheightleft = 31.0f;
   }
   else if (m_d.m_type == RampType2Wire)
   {
      // backwards compatible physics
      wallheightright = 31.0f;
      wallheightleft = 31.0f;
   }
   else if (m_d.m_type == RampType4Wire)
   {
      wallheightleft = 62.0f;
      wallheightright = 62.0f;
   }
   else if (m_d.m_type == RampType3WireRight)
   {
      wallheightright = 62.0f;
      wallheightleft = (float)(6+12.5);
   }
   else if (m_d.m_type == RampType3WireLeft)
   {
      wallheightleft = 62.0f;
      wallheightright = (float)(6+12.5);
   }

   if (wallheightright > 0)
   {
      for (int i=0;i<(cvertex-1);i++)
      {
         const Vertex2D * const pv1 = (i>0) ? &rgvLocal[i-1] : NULL;
         const Vertex2D * const pv2 = &rgvLocal[i];
         const Vertex2D * const pv3 = &rgvLocal[i+1];
         const Vertex2D * const pv4 = (i<(cvertex-2)) ? &rgvLocal[i+2] : NULL;

#ifndef RAMPTEST
         AddLine(pvho, pv2, pv3, pv1, rgheight1[i], rgheight1[i+1]+wallheightright);
         AddLine(pvho, pv3, pv2, pv4, rgheight1[i], rgheight1[i+1]+wallheightright);
#else
         AddSideWall(pvho, pv2, pv3,rgheight1[i], rgheight1[i+1], wallheightright);
         AddSideWall(pvho, pv3, pv2,rgheight1[i+1], rgheight1[i], wallheightright);
#endif
      }
   }

   if (wallheightleft > 0)
   {
      for (int i=0;i<(cvertex-1);i++)
      {
         const Vertex2D * const pv1 = (i>0) ? &rgvLocal[cvertex + i - 1] : NULL;
         const Vertex2D * const pv2 = &rgvLocal[cvertex + i];
         const Vertex2D * const pv3 = &rgvLocal[cvertex + i + 1];
         const Vertex2D * const pv4 = (i<(cvertex-2)) ? &rgvLocal[cvertex + i + 2] : NULL;

#ifndef RAMPTEST
         AddLine(pvho, pv2, pv3, pv1, rgheight1[cvertex - i - 2], rgheight1[cvertex - i - 1] + wallheightleft);
         AddLine(pvho, pv3, pv2, pv4, rgheight1[cvertex - i - 2], rgheight1[cvertex - i - 1] + wallheightleft);
#else
         AddSideWall(pvho, pv2, pv3, rgheight1[cvertex - i - 1], rgheight1[cvertex - i - 2], wallheightleft);
         AddSideWall(pvho, pv3, pv2, rgheight1[cvertex - i - 2], rgheight1[cvertex - i - 1], wallheightleft);
#endif
      }
   }

#ifndef RAMPTEST
   {
      Hit3DPoly *ph3dpolyOld = NULL;

      const Vertex2D *pv1;
      const Vertex2D *pv2;
      const Vertex2D *pv3;
      const Vertex2D *pv4;

      for (int i=0;i<(cvertex-1);i++)
      {
         pv1 = &rgvLocal[i];
         pv2 = &rgvLocal[cvertex*2 - i - 1];
         pv3 = &rgvLocal[cvertex*2 - i - 2];
         pv4 = &rgvLocal[i+1];

         {
            Vertex3Ds * const rgv3D = new Vertex3Ds[3];
            rgv3D[0] = Vertex3Ds(pv2->x,pv2->y,rgheight1[i]);
            rgv3D[1] = Vertex3Ds(pv1->x,pv1->y,rgheight1[i]);
            rgv3D[2] = Vertex3Ds(pv3->x,pv3->y,rgheight1[i+1]);

            Hit3DPoly * const ph3dpoly = new Hit3DPoly(rgv3D,3);
            ph3dpoly->m_elasticity = m_d.m_elasticity;
            ph3dpoly->m_antifriction = 1.0f - m_d.m_friction;	//antifriction
            ph3dpoly->m_scatter = ANGTORAD(m_d.m_scatter);

            if (m_d.m_type == RampTypeFlat)
               ph3dpoly->m_fVisible = fTrue;

            pvho->AddElement(ph3dpoly);

            m_vhoCollidable.AddElement(ph3dpoly);	//remember hit components of ramp
            ph3dpoly->m_fEnabled = m_d.m_fCollidable;

            if (ph3dpolyOld)
               CheckJoint(pvho, ph3dpolyOld, ph3dpoly);

            ph3dpolyOld = ph3dpoly;
         }

         Vertex3Ds * const rgv3D = new Vertex3Ds[3];
         rgv3D[0] = Vertex3Ds(pv3->x,pv3->y,rgheight1[i+1]);
         rgv3D[1] = Vertex3Ds(pv1->x,pv1->y,rgheight1[i]);
         rgv3D[2] = Vertex3Ds(pv4->x,pv4->y,rgheight1[i+1]);

         Hit3DPoly * const ph3dpoly = new Hit3DPoly(rgv3D,3);
         ph3dpoly->m_elasticity = m_d.m_elasticity;
         ph3dpoly->m_antifriction = 1.0f - m_d.m_friction;
         ph3dpoly->m_scatter = ANGTORAD(m_d.m_scatter);

         if (m_d.m_type == RampTypeFlat)
            ph3dpoly->m_fVisible = fTrue;

         pvho->AddElement(ph3dpoly);

         m_vhoCollidable.AddElement(ph3dpoly);	//remember hit components of ramp
         ph3dpoly->m_fEnabled = m_d.m_fCollidable;

         CheckJoint(pvho, ph3dpolyOld, ph3dpoly);
         ph3dpolyOld = ph3dpoly;
      }

      Vertex3Ds * const rgv3D = new Vertex3Ds[3];
      rgv3D[0] = Vertex3Ds(pv4->x,pv4->y,rgheight1[cvertex-1]);
      rgv3D[1] = Vertex3Ds(pv3->x,pv3->y,rgheight1[cvertex-1]);
      rgv3D[2] = Vertex3Ds(pv1->x,pv1->y,rgheight1[cvertex-1]);
      ph3dpolyOld = new Hit3DPoly(rgv3D,3);

      CheckJoint(pvho, ph3dpolyOld, ph3dpolyOld);
      delete ph3dpolyOld;
      ph3dpolyOld = NULL;
   }

   // add outside bottom, 
   // joints at the intersections are not needed since the inner surface has them
   // this surface is identical... except for the direction of the normal face.
   // hence the joints protect both surface edges from haveing a fall through

   for (int i=0; i<(cvertex-1); i++)
   {
      const Vertex2D * const pv1 = &rgvLocal[i];
      const Vertex2D * const pv2 = &rgvLocal[cvertex*2 - i - 1];
      const Vertex2D * const pv3 = &rgvLocal[cvertex*2 - i - 2];
      const Vertex2D * const pv4 = &rgvLocal[i+1];

      {
         Vertex3Ds * const rgv3D = new Vertex3Ds[3];
         rgv3D[0] = Vertex3Ds(pv1->x,pv1->y,rgheight1[i]);
         rgv3D[1] = Vertex3Ds(pv2->x,pv2->y,rgheight1[i]);
         rgv3D[2] = Vertex3Ds(pv3->x,pv3->y,rgheight1[i+1]);

         Hit3DPoly * const ph3dpoly = new Hit3DPoly(rgv3D,3);
         ph3dpoly->m_elasticity = m_d.m_elasticity;
         ph3dpoly->m_antifriction = 1.0f - m_d.m_friction;	//antifriction
         ph3dpoly->m_scatter = ANGTORAD(m_d.m_scatter);

         pvho->AddElement(ph3dpoly);

         m_vhoCollidable.AddElement(ph3dpoly);	//remember hit components of ramp
         ph3dpoly->m_fEnabled = m_d.m_fCollidable;
      }

      Vertex3Ds * const rgv3D = new Vertex3Ds[3];
      rgv3D[0] = Vertex3Ds(pv3->x,pv3->y,rgheight1[i+1]);
      rgv3D[1] = Vertex3Ds(pv4->x,pv4->y,rgheight1[i+1]);
      rgv3D[2] = Vertex3Ds(pv1->x,pv1->y,rgheight1[i]);

      Hit3DPoly * const ph3dpoly = new Hit3DPoly(rgv3D,3);
      ph3dpoly->m_elasticity = m_d.m_elasticity;
      ph3dpoly->m_antifriction = 1.0f - m_d.m_friction;
      ph3dpoly->m_scatter = ANGTORAD(m_d.m_scatter);

      pvho->AddElement(ph3dpoly);

      m_vhoCollidable.AddElement(ph3dpoly);	//remember hit components of ramp
      ph3dpoly->m_fEnabled = m_d.m_fCollidable;
   }
#endif

   delete [] rgheight1;
   delete [] rgvLocal;
}

void Ramp::GetHitShapesDebug(Vector<HitObject> * const pvho)
{
}

void Ramp::AddSideWall(Vector<HitObject> * const pvho, const Vertex2D * const pv1, const Vertex2D * const pv2, const float height1, const float height2, const float wallheight)
{
   Vertex3Ds * const rgv3D = new Vertex3Ds[4];
   rgv3D[0] = Vertex3Ds(pv1->x,pv1->y,height1 - (float)PHYS_SKIN);
   rgv3D[1] = Vertex3Ds(pv2->x,pv2->y,height2 - (float)PHYS_SKIN);
   rgv3D[2] = Vertex3Ds(pv2->x + WALLTILT,pv2->y + WALLTILT,height2 + wallheight);
   rgv3D[3] = Vertex3Ds(pv1->x + WALLTILT,pv1->y + WALLTILT,height1 + wallheight);

   Hit3DPoly * const ph3dpoly = new Hit3DPoly(rgv3D,4);
   ph3dpoly->m_elasticity = m_d.m_elasticity;
   ph3dpoly->m_antifriction = 1.0f - m_d.m_friction;
   ph3dpoly->m_scatter = ANGTORAD(m_d.m_scatter);

   pvho->AddElement(ph3dpoly);

   m_vhoCollidable.AddElement(ph3dpoly);	//remember hit components of ramp
   ph3dpoly->m_fEnabled = m_d.m_fCollidable;
}

void Ramp::CheckJoint(Vector<HitObject> * const pvho, const Hit3DPoly * const ph3d1, const Hit3DPoly * const ph3d2)
{
   Vertex3Ds vjointnormal = CrossProduct(ph3d1->normal, ph3d2->normal);
   //vjointnormal.x = ph3d1->normal.x + ph3d2->normal.x;
   //vjointnormal.y = ph3d1->normal.y + ph3d2->normal.y;
   //vjointnormal.z = ph3d1->normal.z + ph3d2->normal.z;

   const float sqrlength = vjointnormal.x * vjointnormal.x + vjointnormal.y * vjointnormal.y + vjointnormal.z * vjointnormal.z;
   if (sqrlength < 1.0e-8f) return;

   const float inv_length = 1.0f/sqrtf(sqrlength);
   vjointnormal.x *= inv_length;
   vjointnormal.y *= inv_length;
   vjointnormal.z *= inv_length;

   // By convention of the calling function, points 1 [0] and 2 [1] of the second polygon will
   // be the common-edge points

   Hit3DCylinder * const ph3dc = new Hit3DCylinder(&ph3d2->m_rgv[0], &ph3d2->m_rgv[1], &vjointnormal);
   ph3dc->m_elasticity = m_d.m_elasticity;
   ph3dc->m_antifriction = 1.0f - m_d.m_friction;	//antifriction
   ph3dc->m_scatter = ANGTORAD(m_d.m_scatter);
   pvho->AddElement(ph3dc);

   m_vhoCollidable.AddElement(ph3dc);	//remember hit components of ramp
   ph3dc->m_fEnabled = m_d.m_fCollidable;
}


void Ramp::AddLine(Vector<HitObject> * const pvho, const Vertex2D * const pv1, const Vertex2D * const pv2, const Vertex2D * const pv3, const float height1, const float height2)
{
   LineSeg * const plineseg = new LineSeg();
   plineseg->m_elasticity = m_d.m_elasticity;
   plineseg->m_antifriction = 1.0f - m_d.m_friction;
   plineseg->m_scatter = ANGTORAD(m_d.m_scatter);

   plineseg->m_pfe = NULL;

   plineseg->m_rcHitRect.zlow = height1;//m_d.m_heightbottom;
   plineseg->m_rcHitRect.zhigh = height2;//m_d.m_heighttop;

   plineseg->v1 = *pv1;
   plineseg->v2 = *pv2;

   pvho->AddElement(plineseg);

   m_vhoCollidable.AddElement(plineseg);	//remember hit components of ramp
   plineseg->m_fEnabled = m_d.m_fCollidable;

   plineseg->CalcNormal();

   const Vertex2D vt1(pv1->x - pv2->x, pv1->y - pv2->y);

   if (pv3)
   {
      const Vertex2D vt2(pv1->x - pv3->x, pv1->y - pv3->y);

      const float dot = vt1.x*vt2.y - vt1.y*vt2.x;

      if (dot < 0) // Inside edges don't need joint hit-testing (dot == 0 continuous segments should mathematically never hit)
      {
         Joint * const pjoint = new Joint();
         pjoint->m_elasticity = m_d.m_elasticity;
         pjoint->m_antifriction = 1.0f - m_d.m_friction;
         pjoint->m_scatter = ANGTORAD(m_d.m_scatter);

         pjoint->m_pfe = NULL;

         pjoint->m_rcHitRect.zlow = height1;//m_d.m_heightbottom;
         pjoint->m_rcHitRect.zhigh = height2;//m_d.m_heighttop;

         pjoint->center = *pv1;
         pvho->AddElement(pjoint);

         m_vhoCollidable.AddElement(pjoint);	//remember hit components of ramp
         pjoint->m_fEnabled = m_d.m_fCollidable;

         // Set up line normal
         const float inv_length = 1.0f/sqrtf(vt2.x * vt2.x + vt2.y * vt2.y);
         pjoint->normal.x = plineseg->normal.x - vt2.y *inv_length;
         pjoint->normal.y = vt2.x *inv_length + plineseg->normal.y;

         // Set up line normal
         const float inv_length2 = 1.0f/sqrtf(pjoint->normal.x * pjoint->normal.x + pjoint->normal.y * pjoint->normal.y);
         pjoint->normal.x *= inv_length2;
         pjoint->normal.y *= inv_length2;
      }
   }
}

void Ramp::EndPlay()
{
    IEditable::EndPlay();
    m_vhoCollidable.RemoveAllElements();

   	if(staticVertexBuffer) {
		staticVertexBuffer->release();
		staticVertexBuffer = 0;
	}

	if(dynamicVertexBuffer) {
		dynamicVertexBuffer->release();
		dynamicVertexBuffer = 0;
		dynamicVertexBufferRegenerate = true;

		delete [] rgvbuf;
        delete [] rgibuf;
        delete [] invrgibuf;
	}
}

static const WORD rgicrosssection[] = {
   0,1,16,
   1,17,16,
   1,2,17,
   2,18,17,
   2,3,18,
   3,19,18,
   3,0,19,
   0,16,19,

   8,9,24,
   9,25,24,
   9,10,25,
   10,26,25,
   10,11,26,
   11,27,26,
   11,8,27,
   8,24,27,

   4,5,20,
   5,21,20,
   5,6,21,
   6,22,21,
   6,7,22,
   7,23,22,
   7,4,23,
   4,20,23,

   12,13,28,
   13,29,28,
   13,14,29,
   14,30,29,
   14,15,30,
   15,31,30,
   15,12,31,
   12,28,31
};

void Ramp::RenderStaticHabitrail(const RenderDevice* _pd3dDevice)
{
   RenderDevice* pd3dDevice=(RenderDevice*)_pd3dDevice;

   pd3dDevice->SetRenderState(RenderDevice::SPECULARENABLE, TRUE);

   habitrailMaterial.set();
   int offset=0;
   for (int i=0;i<rampVertex;i++,offset+=32)
   {
      RenderPolygons(pd3dDevice, offset, (WORD*)rgicrosssection, 0, 16);
      if (m_d.m_type == RampType4Wire || m_d.m_type == RampType3WireRight)
      {
         RenderPolygons(pd3dDevice, offset, (WORD*)rgicrosssection, 16, 24);
      }

      if (m_d.m_type == RampType4Wire || m_d.m_type == RampType3WireLeft)
      {
         RenderPolygons(pd3dDevice, offset, (WORD*)rgicrosssection, 24, 32);
      }
   }

   pd3dDevice->SetRenderState(RenderDevice::SPECULARENABLE, FALSE);
}

void Ramp::RenderPolygons(const RenderDevice* _pd3dDevice, int offset, WORD * const rgi, const int start, const int stop)
{
   RenderDevice* pd3dDevice=(RenderDevice*)_pd3dDevice;  
   if (m_d.m_type == RampType1Wire)
   {
      pd3dDevice->renderPrimitive( D3DPT_TRIANGLELIST, staticVertexBuffer, offset, 32, rgi+stop/2*3, 3*(stop-stop/2), 0);
   }
   else
   {
      pd3dDevice->renderPrimitive( D3DPT_TRIANGLELIST, staticVertexBuffer, offset, 32, rgi+start*3, 3*(stop-start), 0);
   }
}

static const WORD rgiRampStatic1[4] = {0,3,2,1};
void Ramp::prepareHabitrail(RenderDevice* pd3dDevice )
{
   const int numVertices = rampVertex*32;
   pd3dDevice->createVertexBuffer(numVertices, 0, MY_D3DFVF_NOTEX_VERTEX, &staticVertexBuffer);
   NumVideoBytes += numVertices*sizeof(Vertex3D_NoTex);

   Vertex3D_NoTex *buf;
   staticVertexBuffer->lock(0,0,(void**)&buf, VertexBuffer::WRITEONLY | VertexBuffer::DISCARDCONTENTS);

   int offset=0;
   Vertex3D_NoTex rgv3D[32];
   for (int i=0;i<rampVertex;i++)
   {
      rgv3D[0].x = -3.0f;
      rgv3D[0].y = -3.0f;
      rgv3D[0].z = 0;
      rgv3D[0].nx = -1.0f;
      rgv3D[0].ny = -1.0f;
      rgv3D[0].nz = 0;
      rgv3D[0].NormalizeNormal();

      rgv3D[1].x = 3.0f;
      rgv3D[1].y = -3.0f;
      rgv3D[1].z = 0;
      rgv3D[1].nx = 1.0f;
      rgv3D[1].ny = -1.0f;
      rgv3D[1].nz = 0;
      rgv3D[1].NormalizeNormal();

      rgv3D[2].x = 3.0f;
      rgv3D[2].y = 3.0f;
      rgv3D[2].z = 0;
      rgv3D[2].nx = 1.0f;
      rgv3D[2].ny = 1.0f;
      rgv3D[2].nz = 0;
      rgv3D[2].NormalizeNormal();

      rgv3D[3].x = -3.0f;
      rgv3D[3].y = 3.0f;
      rgv3D[3].z = 0;
      rgv3D[3].nx = -1.0f;
      rgv3D[3].ny = 1.0f;
      rgv3D[3].nz = 0;
      rgv3D[3].NormalizeNormal();

      if (m_d.m_type != RampType1Wire)
      {
         for (int l=0;l<4;l++)
         {
            rgv3D[l+ 4].x = rgv3D[l].x + 44.0f; //44.0f
            rgv3D[l+ 4].y = rgv3D[l].y - 19.0f; //22.0f
            rgv3D[l+ 4].z = rgv3D[l].z;
            rgv3D[l+ 8].x = rgv3D[l].x + 9.5f;
            rgv3D[l+ 8].y = rgv3D[l].y + 19.0f;
            rgv3D[l+ 8].z = rgv3D[l].z;
            rgv3D[l+12].x = rgv3D[l].x + 44.0f;
            rgv3D[l+12].y = rgv3D[l].y + 19.0f;
            rgv3D[l+12].z = rgv3D[l].z;

            rgv3D[l+ 4].nx = rgv3D[l].nx;
            rgv3D[l+ 4].ny = rgv3D[l].ny;
            rgv3D[l+ 4].nz = rgv3D[l].nz;
            rgv3D[l+ 8].nx = rgv3D[l].nx;
            rgv3D[l+ 8].ny = rgv3D[l].ny;
            rgv3D[l+ 8].nz = rgv3D[l].nz;
            rgv3D[l+12].nx = rgv3D[l].nx;
            rgv3D[l+12].ny = rgv3D[l].ny;
            rgv3D[l+12].nz = rgv3D[l].nz;
         }
         for (int l=0;l<4;l++)
         {
            rgv3D[l].x += 9.5f;
            rgv3D[l].y += -19.0f;
         }
      }
      else
      {
         for (int l=0;l<4;l++)
         {
            rgv3D[l+ 4].x = rgv3D[l].x+44.0f; //44.0f
            rgv3D[l+ 4].y = rgv3D[l].y;
            rgv3D[l+ 4].z = rgv3D[l].z;
            rgv3D[l+ 8].x = rgv3D[l].x + 9.5f;
            rgv3D[l+ 8].y = rgv3D[l].y;
            rgv3D[l+ 8].z = rgv3D[l].z;
            rgv3D[l+12].x = rgv3D[l].x+44.0f;
            rgv3D[l+12].y = rgv3D[l].y;
            rgv3D[l+12].z = rgv3D[l].z;

            rgv3D[l+ 4].nx = rgv3D[l].nx;
            rgv3D[l+ 4].ny = rgv3D[l].ny;
            rgv3D[l+ 4].nz = rgv3D[l].nz;
            rgv3D[l+ 8].nx = rgv3D[l].nx;
            rgv3D[l+ 8].ny = rgv3D[l].ny;
            rgv3D[l+ 8].nz = rgv3D[l].nz;
            rgv3D[l+12].nx = rgv3D[l].nx;
            rgv3D[l+12].ny = rgv3D[l].ny;
            rgv3D[l+12].nz = rgv3D[l].nz;
         }
      }

      const int p1 = (i==0) ? 0 : (i-1);
      const int p2 = i;
      const int p3 = (i==(rampVertex-1)) ? i : (i+1);
      const int p4 = rampVertex*2 - i - 1; //!! ?? *2 valid?

      Vertex3Ds vacross(rgvInit[p4].x - rgvInit[p2].x, rgvInit[p4].y - rgvInit[p2].y, 0.0f);

      // The vacross vector is our local up vector.  Rotate the cross-section
      // later to match this up
      vacross.Normalize();

      Vertex3Ds tangent(rgvInit[p3].x - rgvInit[p1].x, rgvInit[p3].y - rgvInit[p1].y, rgheightInit[p3] - rgheightInit[p1]);

      // This is the vector describing the tangent to the ramp at this point
      tangent.Normalize();

      const Vertex3Ds rotationaxis(tangent.y, -tangent.x, 0.0f);
      /*
      Vertex3Ds up(0,0,1.0f);
      // Get axis of rotation to rotate our cross-section into place
      CrossProduct(tangent, up, &rotationaxis);*/

      const float dot = tangent.z; //tangent.Dot(&up);
      const float angle = acosf(dot);

      RotateAround(rotationaxis, rgv3D, 16, angle);

      // vnewup is the beginning up vector of the cross-section
      const Vertex2D vnewupdef(0.0f,1.0f);
      const Vertex3Ds vnewup = RotateAround(rotationaxis, vnewupdef, angle);

      // vacross is not out real up vector, but the up vector for the cross-section isn't real either
      //Vertex3D vrampup;
      //CrossProduct(&tangent, &vacross, &vrampup);
      const float dotupcorrection = vnewup.Dot(vacross);
      float angleupcorrection = acosf(dotupcorrection);

      if (vacross.x >= 0)
      {
         angleupcorrection = -angleupcorrection;
      }

      RotateAround(tangent, rgv3D, 16, -angleupcorrection);

      for (int l=0;l<16;l++)
      {
         rgv3D[l].x += (rgvInit[p2].x + rgvInit[p4].x)*0.5f;
         rgv3D[l].y += (rgvInit[p2].y + rgvInit[p4].y)*0.5f;
         rgv3D[l].z += rgheightInit[p2];
      }

      if (i != 0)
      {
         memcpy( &buf[offset], rgv3D, sizeof(Vertex3D_NoTex)*32);
         offset+=32;
      }
      memcpy(&rgv3D[16], rgv3D, sizeof(Vertex3D_NoTex)*16);
   }

   staticVertexBuffer->unlock();  
}

void Ramp::prepareStatic(RenderDevice* pd3dDevice)
{
//   float *rgheight,*rgratio;
//   Vertex2D *rgv = GetRampVertex(rampVertex, &rgheight, NULL, &rgratio);

   int numVertices=rampVertex*4*5;
   pd3dDevice->createVertexBuffer( numVertices, 0, MY_D3DFVF_NOTEX2_VERTEX, &staticVertexBuffer);
   NumVideoBytes += numVertices*sizeof(Vertex3D_NoTex2);

   Pin3D *const ppin3d = &g_pplayer->m_pin3d;

   PinImage * const pin = m_ptable->GetImage(m_d.m_szImage);
   float maxtu = 0, maxtv = 0;
   if (pin)
   {
      m_ptable->GetTVTU(pin, &maxtu, &maxtv);
   }

   const float tablewidth = m_ptable->m_right - m_ptable->m_left;
   const float tableheight = m_ptable->m_bottom - m_ptable->m_top;

   const float scalewidth  = (float) g_pplayer->m_pin3d.m_dwRenderWidth  * (float)(1.0/64.055);		// 64.0f is texture width.			
   const float scaleheight = (float) g_pplayer->m_pin3d.m_dwRenderHeight * (float)(1.0/64.055);		// 64.0f is texture height.

   const float inv_width = scalewidth / (float)g_pplayer->m_pin3d.m_dwRenderWidth;
   const float inv_height = scaleheight / (float)g_pplayer->m_pin3d.m_dwRenderHeight;

   const float inv_width2 = maxtu / tablewidth;
   const float inv_height2 = maxtv / tableheight;

   Vertex3D_NoTex2 *buf;
   staticVertexBuffer->lock(0,0,(void**)&buf, VertexBuffer::WRITEONLY | VertexBuffer::NOOVERWRITE);

   int offset=0;
   for (int i=0;i<(rampVertex-1);i++)
   {
      Vertex3D_NoTex2 rgv3D[4];
      rgv3D[0].x = rgvInit[i].x;
      rgv3D[0].y = rgvInit[i].y;
      rgv3D[0].z = rgheightInit[i];

      rgv3D[3].x = rgvInit[i+1].x;
      rgv3D[3].y = rgvInit[i+1].y;
      rgv3D[3].z = rgheightInit[i+1];

      rgv3D[2].x = rgvInit[rampVertex*2-i-2].x;
      rgv3D[2].y = rgvInit[rampVertex*2-i-2].y;
      rgv3D[2].z = rgheightInit[i+1];

      rgv3D[1].x = rgvInit[rampVertex*2-i-1].x;
      rgv3D[1].y = rgvInit[rampVertex*2-i-1].y;
      rgv3D[1].z = rgheightInit[i];

      if (pin)
      {
         if (m_d.m_imagealignment == ImageModeWorld)
         {
            // Check if this is an acrylic.
            if (m_d.m_fAcrylic)
            {
               Vertex2D rgvOut[4];
               // Transform vertecies into screen coordinates.
               g_pplayer->m_pin3d.TransformVertices(rgv3D, NULL, 4, rgvOut);

               // Calculate texture coordinate for each vertex.
               for (int r=0; r<4; r++)
               {
                  // Set texture coordinates so that there is a 1 to 1 correspondence
                  // between texels and pixels.  This is the best case for screen door transparency.
                  rgv3D[r].tu = rgvOut[r].x * inv_width; 
                  rgv3D[r].tv = rgvOut[r].y * inv_height; 
               }
            }
            else
            {
               rgv3D[0].tu = rgv3D[0].x * inv_width2;
               rgv3D[0].tv = rgv3D[0].y * inv_height2;
               rgv3D[1].tu = rgv3D[1].x * inv_width2;
               rgv3D[1].tv = rgv3D[1].y * inv_height2;
               rgv3D[2].tu = rgv3D[2].x * inv_width2;
               rgv3D[2].tv = rgv3D[2].y * inv_height2;
               rgv3D[3].tu = rgv3D[3].x * inv_width2;
               rgv3D[3].tv = rgv3D[3].y * inv_height2;
            }
         }
         else
         {
            rgv3D[0].tu = maxtu;
            rgv3D[0].tv = rgratioInit[i] * maxtv;
            rgv3D[1].tu = 0;
            rgv3D[1].tv = rgratioInit[i] * maxtv;
            rgv3D[2].tu = 0;
            rgv3D[2].tv = rgratioInit[i+1] * maxtv;
            rgv3D[3].tu = maxtu;
            rgv3D[3].tv = rgratioInit[i+1] * maxtv;
         }
      }

      SetNormal(rgv3D, rgi0123, 4, NULL, NULL, NULL);
      // Draw the floor of the ramp.
      memcpy( &buf[offset], rgv3D, sizeof(Vertex3D_NoTex2)*4 );
      offset+=4;
   }

   for (int i=0;i<(rampVertex-1);i++)
   {
      Vertex3D_NoTex2 rgv3D[4];
      rgv3D[0].x = rgvInit[i].x;
      rgv3D[0].y = rgvInit[i].y;
      rgv3D[0].z = rgheightInit[i];

      rgv3D[3].x = rgvInit[i+1].x;
      rgv3D[3].y = rgvInit[i+1].y;
      rgv3D[3].z = rgheightInit[i+1];

      rgv3D[2].x = rgvInit[i+1].x;
      rgv3D[2].y = rgvInit[i+1].y;
      rgv3D[2].z = rgheightInit[i+1] + m_d.m_rightwallheightvisible;

      rgv3D[1].x = rgvInit[i].x;
      rgv3D[1].y = rgvInit[i].y;
      rgv3D[1].z = rgheightInit[i] + m_d.m_rightwallheightvisible;

      if (pin && m_d.m_fImageWalls)
      {
         if (m_d.m_imagealignment == ImageModeWorld)
         {
            // Check if this is an acrylic.
            if (m_d.m_fAcrylic)
            {
               Vertex2D rgvOut[4];
               // Transform vertices into screen coordinates.
               g_pplayer->m_pin3d.TransformVertices(rgv3D, NULL, 4, rgvOut);

               // Calculate texture coordinate for each vertex.
               for (int r=0; r<4; r++)
               {
                  // Set texture coordinates so that there is a 1 to 1 correspondence
                  // between texels and pixels.  This is the best case for screen door transparency.
                  rgv3D[r].tu = rgvOut[r].x * inv_width; 
                  rgv3D[r].tv = rgvOut[r].y * inv_height; 
               }
            }
            else
            {
               rgv3D[0].tu = rgv3D[0].x * inv_width2;
               rgv3D[0].tv = rgv3D[0].y * inv_height2;
               rgv3D[2].tu = rgv3D[2].x * inv_width2;
               rgv3D[2].tv = rgv3D[2].y * inv_height2;

               rgv3D[1].tu = rgv3D[0].tu;
               rgv3D[1].tv = rgv3D[0].tv;
               rgv3D[3].tu = rgv3D[2].tu;
               rgv3D[3].tv = rgv3D[2].tv;
            }
         }
         else
         {
            rgv3D[0].tu = maxtu;
            rgv3D[0].tv = rgratioInit[i] * maxtv;
            rgv3D[2].tu = maxtu;
            rgv3D[2].tv = rgratioInit[i+1] * maxtv;

            rgv3D[1].tu = rgv3D[0].tu;
            rgv3D[1].tv = rgv3D[0].tv;
            rgv3D[3].tu = rgv3D[2].tu;
            rgv3D[3].tv = rgv3D[2].tv;
         }
      }

      // 2-Sided polygon
      SetNormal(rgv3D, rgi0123, 4, NULL, NULL, NULL);
      // Draw the wall of the ramp.
      memcpy( &buf[offset], rgv3D, sizeof(Vertex3D_NoTex2)*4 );
      offset+=4;

      SetNormal(rgv3D, rgiRampStatic1, 4, NULL, NULL, NULL);
      // Draw the wall of the ramp.
      memcpy( &buf[offset], rgv3D, sizeof(Vertex3D_NoTex2)*4 );
      offset+=4;
   }

   for (int i=0;i<(rampVertex-1);i++)
   {
      Vertex3D_NoTex2 rgv3D[4];
      rgv3D[0].x = rgvInit[rampVertex*2-i-2].x;
      rgv3D[0].y = rgvInit[rampVertex*2-i-2].y;
      rgv3D[0].z = rgheightInit[i+1];

      rgv3D[3].x = rgvInit[rampVertex*2-i-1].x;
      rgv3D[3].y = rgvInit[rampVertex*2-i-1].y;
      rgv3D[3].z = rgheightInit[i];

      rgv3D[2].x = rgvInit[rampVertex*2-i-1].x;
      rgv3D[2].y = rgvInit[rampVertex*2-i-1].y;
      rgv3D[2].z = rgheightInit[i] + m_d.m_leftwallheightvisible;

      rgv3D[1].x = rgvInit[rampVertex*2-i-2].x;
      rgv3D[1].y = rgvInit[rampVertex*2-i-2].y;
      rgv3D[1].z = rgheightInit[i+1] + m_d.m_leftwallheightvisible;

      if (pin && m_d.m_fImageWalls)
      {
         if (m_d.m_imagealignment == ImageModeWorld)
         {
            // Check if this is an acrylic.
            if (m_d.m_fAcrylic)
            {
               Vertex2D rgvOut[4];
               // Transform vertices into screen coordinates.
               g_pplayer->m_pin3d.TransformVertices(rgv3D, NULL, 4, rgvOut);

               // Calculate texture coordinate for each vertex.
               for (int r=0; r<4; r++)
               {
                  // Set texture coordinates so that there is a 1 to 1 correspondence
                  // between texels and pixels.  This is the best case for screen door transparency.
                  rgv3D[r].tu = rgvOut[r].x * inv_width; 
                  rgv3D[r].tv = rgvOut[r].y * inv_height; 
               }
            }
            else
            {
               rgv3D[0].tu = rgv3D[0].x * inv_width2;
               rgv3D[0].tv = rgv3D[0].y * inv_height2;
               rgv3D[2].tu = rgv3D[2].x * inv_width2;
               rgv3D[2].tv = rgv3D[2].y * inv_height2;

               rgv3D[1].tu = rgv3D[0].tu;
               rgv3D[1].tv = rgv3D[0].tv;
               rgv3D[3].tu = rgv3D[2].tu;
               rgv3D[3].tv = rgv3D[2].tv;
            }
         }
         else
         {
            rgv3D[0].tu = 0;
            rgv3D[0].tv = rgratioInit[i] * maxtv;
            rgv3D[2].tu = 0;
            rgv3D[2].tv = rgratioInit[i+1] * maxtv;

            rgv3D[1].tu = rgv3D[0].tu;
            rgv3D[1].tv = rgv3D[0].tv;
            rgv3D[3].tu = rgv3D[2].tu;
            rgv3D[3].tv = rgv3D[2].tv;
         }
      }

      // 2-Sided polygon
      SetNormal(rgv3D, rgi0123, 4, NULL, NULL, NULL);
      // Draw the wall of the ramp.
      memcpy( &buf[offset], rgv3D, sizeof(Vertex3D_NoTex2)*4 );
      offset+=4;

      SetNormal(rgv3D, rgiRampStatic1, 4, NULL, NULL, NULL);
      // Draw the wall of the ramp.
      memcpy( &buf[offset], rgv3D, sizeof(Vertex3D_NoTex2)*4 );
      offset+=4;
   }
   
   staticVertexBuffer->unlock();  
}


void Ramp::RenderSetup(const RenderDevice* _pd3dDevice)
{
   RenderDevice* pd3dDevice = (RenderDevice*)_pd3dDevice;

   rgvInit = GetRampVertex(rampVertex, &rgheightInit, NULL, &rgratioInit);

   solidMaterial.setColor( 1.0f, m_d.m_color );
   habitrailMaterial.setColor( 1.0f, m_d.m_color );
   habitrailMaterial.setPower( 8.0f );
   habitrailMaterial.setSpecular( 1.0f, 1.0f, 1.0f, 1.0f );

   if( !staticVertexBuffer && m_d.m_IsVisible && !m_d.m_fAlpha )
   {
      if (m_d.m_type == RampType4Wire 
         || m_d.m_type == RampType1Wire //add check for 1 wire
         || m_d.m_type == RampType2Wire 
         || m_d.m_type == RampType3WireLeft 
         || m_d.m_type == RampType3WireRight)
         prepareHabitrail( pd3dDevice );
      else
         prepareStatic( pd3dDevice );
   }
   else if( !dynamicVertexBuffer && m_d.m_IsVisible && m_d.m_fAlpha )
   {
      if (m_d.m_type == RampType4Wire 
         || m_d.m_type == RampType1Wire //add check for 1 wire
         || m_d.m_type == RampType2Wire 
         || m_d.m_type == RampType3WireLeft 
         || m_d.m_type == RampType3WireRight)
      {
         prepareHabitrail( pd3dDevice );
      }
      else
      {
         const int numVertices = (rampVertex-1)*4;
         pd3dDevice->createVertexBuffer(numVertices*5, 0, MY_D3DFVF_NOTEX2_VERTEX, &dynamicVertexBuffer);
         NumVideoBytes += numVertices*5*sizeof(Vertex3D_NoTex);     

         rgvbuf = new Vertex3D_NoTex2[numVertices];
         rgibuf = new WORD[(rampVertex-1)*6];
         invrgibuf = new WORD[(rampVertex-1)*6];

      }
   }

   delete[] rgvInit;
   delete[] rgheightInit;
   delete[] rgratioInit;
}

void Ramp::RenderStatic(const RenderDevice* _pd3dDevice)
{	
   RenderDevice* pd3dDevice = (RenderDevice*)_pd3dDevice;
   if (!m_d.m_IsVisible) return;		// return if no Visible

   // dont render alpha shaded ramps into static buffer, these are done per frame later-on
   if (m_d.m_fAlpha && g_pvp->m_pdd.m_fHardwareAccel) return;

   if (m_d.m_type == RampType4Wire 
      || m_d.m_type == RampType1Wire  //add check for 1 wire
      || m_d.m_type == RampType2Wire 
      || m_d.m_type == RampType3WireLeft 
      || m_d.m_type == RampType3WireRight)
   {
      RenderStaticHabitrail(pd3dDevice);
   }
   else
   {
      Pin3D *const ppin3d = &g_pplayer->m_pin3d;

      PinImage * const pin = m_ptable->GetImage(m_d.m_szImage);
      float maxtu = 0, maxtv = 0;

      if (pin)
      {
         m_ptable->GetTVTU(pin, &maxtu, &maxtv);

         pin->EnsureColorKey();
         if (pin->m_fTransparent)
         {				
            pd3dDevice->SetTexture(ePictureTexture, pin->m_pdsBufferColorKey);
            pd3dDevice->SetRenderState(RenderDevice::ALPHABLENDENABLE, FALSE);
            pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_NONE);
         }
         else // ppin3d->SetTexture(pin->m_pdsBuffer);
         {	
            pd3dDevice->SetTexture(ePictureTexture, pin->m_pdsBufferColorKey);
            pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);
            pd3dDevice->SetRenderState(RenderDevice::DITHERENABLE, TRUE); 	
            ppin3d->EnableAlphaTestReference(0x00000001);
            pd3dDevice->SetRenderState(RenderDevice::SRCBLEND,  D3DBLEND_SRCALPHA);
            pd3dDevice->SetRenderState(RenderDevice::DESTBLEND, m_d.m_fAddBlend ? D3DBLEND_ONE : D3DBLEND_INVSRCALPHA);
		    if(m_d.m_fAddBlend)
		        pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_COLORARG2, D3DTA_TFACTOR); // factor is 1,1,1,1 by default -> do not modify tex by diffuse lighting
         }

         ppin3d->SetColorKeyEnabled(TRUE);
         pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, m_d.m_fModify3DStereo); // do not update z if just a fake ramp (f.e. flasher fakes, etc)

         // Check if this is an acrylic.
         if (m_d.m_fAcrylic)
         {
            // Set a high threshold for writing transparent pixels to the z buffer.  
            // This allows some of the ball's pixels to write when under the ramp... 
            // giving the illusion of transparency (screen door). 
            ppin3d->EnableAlphaTestReference(127);
            // Make sure our textures tile.
            pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESS, D3DTADDRESS_WRAP);

            // Turn off texture filtering.
            ppin3d->SetTextureFilter ( ePictureTexture, TEXTURE_MODE_POINT );
         }
         else
         {
            ppin3d->SetTextureFilter ( ePictureTexture, TEXTURE_MODE_TRILINEAR );
         }

         textureMaterial.set();
      }
      else
      {
         solidMaterial.set();
      }

      int offset=0;
      for (int i=0; i<(rampVertex-1); i++,offset+=4)
         pd3dDevice->renderPrimitive(D3DPT_TRIANGLEFAN, staticVertexBuffer, offset, 4, (LPWORD)rgi0123, 4, 0 );

      if (pin && !m_d.m_fImageWalls)
      {
         ppin3d->SetTexture(NULL);
         solidMaterial.set();
      }

      for (int i=0;i<(rampVertex-1);i++)
      {
         pd3dDevice->renderPrimitive(D3DPT_TRIANGLEFAN, staticVertexBuffer, offset, 4, (LPWORD)rgi0123, 4, 0 );
         offset+=4;
         pd3dDevice->renderPrimitive(D3DPT_TRIANGLEFAN, staticVertexBuffer, offset, 4, (LPWORD)rgiRampStatic1, 4, 0 );
         offset+=4;
      }
      //render second wall
      for (int i=0;i<(rampVertex-1);i++)
      {
         pd3dDevice->renderPrimitive(D3DPT_TRIANGLEFAN, staticVertexBuffer, offset, 4, (LPWORD)rgi0123, 4, 0 );
         offset+=4;
         pd3dDevice->renderPrimitive(D3DPT_TRIANGLEFAN, staticVertexBuffer, offset, 4, (LPWORD)rgiRampStatic1, 4, 0 );
         offset+=4;
      }

      ppin3d->SetTexture(NULL);

      pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);
      pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
   }
}

void Ramp::RenderMovers(const RenderDevice* pd3dDevice)
{
}

void Ramp::SetObjectPos()
{
   g_pvp->SetObjectPosCur(0, 0);
}

void Ramp::MoveOffset(const float dx, const float dy)
{
   for (int i=0;i<m_vdpoint.Size();i++)
   {
      CComObject<DragPoint> * const pdp = m_vdpoint.ElementAt(i);

      pdp->m_v.x += dx;
      pdp->m_v.y += dy;
   }

   m_ptable->SetDirtyDraw();
}

void Ramp::ClearForOverwrite()
{
   ClearPointsForOverwrite();
}

HRESULT Ramp::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   BiffWriter bw(pstm, hcrypthash, hcryptkey);

#ifdef VBA
   bw.WriteInt(FID(PIID), ApcProjectItem.ID());
#endif
   bw.WriteFloat(FID(HTBT), m_d.m_heightbottom);
   bw.WriteFloat(FID(HTTP), m_d.m_heighttop);
   bw.WriteFloat(FID(WDBT), m_d.m_widthbottom);
   bw.WriteFloat(FID(WDTP), m_d.m_widthtop);
   bw.WriteInt(FID(COLR), m_d.m_color);
   bw.WriteBool(FID(TMON), m_d.m_tdr.m_fTimerEnabled);
   bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
   bw.WriteInt(FID(TYPE), m_d.m_type);
   bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);
   bw.WriteString(FID(IMAG), m_d.m_szImage);
   bw.WriteInt(FID(ALGN), m_d.m_imagealignment);
   bw.WriteBool(FID(IMGW), m_d.m_fImageWalls);
   bw.WriteBool(FID(CSHD), m_d.m_fCastsShadow);
   bw.WriteBool(FID(ACRY), m_d.m_fAcrylic);
   bw.WriteBool(FID(ALPH), m_d.m_fAlpha);
   bw.WriteFloat(FID(WLHL), m_d.m_leftwallheight);
   bw.WriteFloat(FID(WLHR), m_d.m_rightwallheight);
   bw.WriteFloat(FID(WVHL), m_d.m_leftwallheightvisible);
   bw.WriteFloat(FID(WVHR), m_d.m_rightwallheightvisible);
   bw.WriteFloat(FID(ELAS), m_d.m_elasticity);
   bw.WriteFloat(FID(RFCT), m_d.m_friction);
   bw.WriteFloat(FID(RSCT), m_d.m_scatter);
   bw.WriteBool(FID(CLDRP), m_d.m_fCollidable);
   bw.WriteBool(FID(RVIS), m_d.m_IsVisible);	
   bw.WriteBool(FID(MSTE), m_d.m_fModify3DStereo);
   bw.WriteBool(FID(ADDB), m_d.m_fAddBlend);

   ISelect::SaveData(pstm, hcrypthash, hcryptkey);

   bw.WriteTag(FID(PNTS));
   HRESULT hr;
   if(FAILED(hr = SavePointData(pstm, hcrypthash, hcryptkey)))
      return hr;

   bw.WriteTag(FID(ENDB));

   return S_OK;
}

HRESULT Ramp::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   SetDefaults(false);

   BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);

   m_ptable = ptable;

   br.Load();
   return S_OK;
}

BOOL Ramp::LoadToken(int id, BiffReader *pbr)
{
   if (id == FID(PIID))
   {
      pbr->GetInt((int *)pbr->m_pdata);
   }
   else if (id == FID(HTBT))
   {
      pbr->GetFloat(&m_d.m_heightbottom);
   }
   else if (id == FID(HTTP))
   {
      pbr->GetFloat(&m_d.m_heighttop);
   }
   else if (id == FID(WDBT))
   {
      pbr->GetFloat(&m_d.m_widthbottom);
   }
   else if (id == FID(WDTP))
   {
      pbr->GetFloat(&m_d.m_widthtop);
   }
   else if (id == FID(COLR))
   {
      pbr->GetInt(&m_d.m_color);
      //	if (!(m_d.m_color & MINBLACKMASK)) {m_d.m_color |= MINBLACK;}	// set minimum black
   }
   else if (id == FID(TMON))
   {
      pbr->GetBool(&m_d.m_tdr.m_fTimerEnabled);
   }
   else if (id == FID(TMIN))
   {
      pbr->GetInt(&m_d.m_tdr.m_TimerInterval);
   }
   else if (id == FID(TYPE))
   {
      pbr->GetInt(&m_d.m_type);
   }
   else if (id == FID(IMAG))
   {
      pbr->GetString(m_d.m_szImage);
   }
   else if (id == FID(ALGN))
   {
      pbr->GetInt(&m_d.m_imagealignment);
   }
   else if (id == FID(IMGW))
   {
      pbr->GetBool(&m_d.m_fImageWalls);
   }
   else if (id == FID(CSHD))
   {
      pbr->GetBool(&m_d.m_fCastsShadow);
   }
   else if (id == FID(ACRY))
   {
      pbr->GetBool(&m_d.m_fAcrylic);
      m_d.m_fAlpha = false; // Alpha is read after acrylic
   }
   else if (id == FID(ALPH))
   {
      pbr->GetBool(&m_d.m_fAlpha);
   }
   else if (id == FID(NAME))
   {
      pbr->GetWideString((WCHAR *)m_wzName);
   }
   else if (id == FID(WLHL))
   {
      pbr->GetFloat(&m_d.m_leftwallheight);
   }
   else if (id == FID(WLHR))
   {
      pbr->GetFloat(&m_d.m_rightwallheight);
   }
   else if (id == FID(WVHL))
   {
      pbr->GetFloat(&m_d.m_leftwallheightvisible);
   }
   else if (id == FID(WVHR))
   {
      pbr->GetFloat(&m_d.m_rightwallheightvisible);
   }
   else if (id == FID(ELAS))
   {
      pbr->GetFloat(&m_d.m_elasticity);
   }
   else if (id == FID(RFCT))
   {
      pbr->GetFloat(&m_d.m_friction);
   }
   else if (id == FID(RSCT))
   {
      pbr->GetFloat(&m_d.m_scatter);
   }
   else if (id == FID(CLDRP))
   {
      pbr->GetBool(&m_d.m_fCollidable);
   }
   else if (id == FID(RVIS))
   {
      pbr->GetBool(&m_d.m_IsVisible);
   }
   else if (id == FID(MSTE))
   {
      pbr->GetBool(&m_d.m_fModify3DStereo);
   }
   else if (id == FID(ADDB))
   {
      pbr->GetBool(&m_d.m_fAddBlend);
   }
   else
   {
      LoadPointToken(id, pbr, pbr->m_version);
      ISelect::LoadToken(id, pbr);
   }
   return fTrue;
}

HRESULT Ramp::InitPostLoad()
{
   return S_OK;
}

void Ramp::DoCommand(int icmd, int x, int y)
{
   ISelect::DoCommand(icmd, x, y);

   switch (icmd)
   {
   case ID_WALLMENU_FLIP:
      {
         Vertex2D vCenter;
         GetPointCenter(&vCenter);
         FlipPointY(&vCenter);
      }
      break;

   case ID_WALLMENU_MIRROR:
      {
         Vertex2D vCenter;
         GetPointCenter(&vCenter);
         FlipPointX(&vCenter);
      }
      break;

   case ID_WALLMENU_ROTATE:
      RotateDialog();
      break;

   case ID_WALLMENU_SCALE:
      ScaleDialog();
      break;

   case ID_WALLMENU_TRANSLATE:
      TranslateDialog();
      break;

   case ID_WALLMENU_ADDPOINT:
      {
         STARTUNDO

         RECT rc;
         GetClientRect(m_ptable->m_hwnd, &rc);

         HitSur * const phs = new HitSur(NULL, m_ptable->m_zoom, m_ptable->m_offsetx, m_ptable->m_offsety, rc.right - rc.left, rc.bottom - rc.top, 0, 0, NULL);

         const Vertex2D v = phs->ScreenToSurface(x, y);
         delete phs;

         Vector<RenderVertex> vvertex;
         GetRgVertex(&vvertex);

         const int cvertex = vvertex.Size();
         Vertex2D vOut;
         int iSeg;
         ClosestPointOnPolygon(vvertex, v, &vOut, &iSeg, false);

         // Go through vertices (including iSeg itself) counting control points until iSeg
         int icp = 0;
         for (int i=0;i<(iSeg+1);i++)
            if (vvertex.ElementAt(i)->fControlPoint)
               icp++;

         for (int i=0;i<cvertex;i++)
            delete vvertex.ElementAt(i);

         //if (icp == 0) // need to add point after the last point
         //icp = m_vdpoint.Size();

         CComObject<DragPoint> *pdp;
         CComObject<DragPoint>::CreateInstance(&pdp);
         if (pdp)
         {
            pdp->AddRef();
            pdp->Init(this, vOut.x, vOut.y);
            pdp->m_fSmooth = fTrue; // Ramps are usually always smooth
            m_vdpoint.InsertElementAt(pdp, icp); // push the second point forward, and replace it with this one.  Should work when index2 wraps.
         }

         SetDirtyDraw();

         STOPUNDO
      }
      break;
   }
}

void Ramp::FlipY(Vertex2D * const pvCenter)
{
   IHaveDragPoints::FlipPointY(pvCenter);
}

void Ramp::FlipX(Vertex2D * const pvCenter)
{
   IHaveDragPoints::FlipPointX(pvCenter);
}

void Ramp::Rotate(float ang, Vertex2D *pvCenter)
{
   IHaveDragPoints::RotatePoints(ang, pvCenter);
}

void Ramp::Scale(float scalex, float scaley, Vertex2D *pvCenter)
{
   IHaveDragPoints::ScalePoints(scalex, scaley, pvCenter);
}

void Ramp::Translate(Vertex2D *pvOffset)
{
   IHaveDragPoints::TranslatePoints(pvOffset);
}

STDMETHODIMP Ramp::InterfaceSupportsErrorInfo(REFIID riid)
{
   static const IID* arr[] =
   {
      &IID_IRamp,
   };

   for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
   {
      if (InlineIsEqualGUID(*arr[i],riid))
         return S_OK;
   }
   return S_FALSE;
}

STDMETHODIMP Ramp::get_HeightBottom(float *pVal)
{
   *pVal = m_d.m_heightbottom;

   return S_OK;
}

STDMETHODIMP Ramp::put_HeightBottom(float newVal)
{
   STARTUNDO

   m_d.m_heightbottom = newVal;
   dynamicVertexBufferRegenerate = true;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Ramp::get_HeightTop(float *pVal)
{
   *pVal = m_d.m_heighttop;

   return S_OK;
}

STDMETHODIMP Ramp::put_HeightTop(float newVal)
{
   STARTUNDO

   m_d.m_heighttop = newVal;
   dynamicVertexBufferRegenerate = true;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Ramp::get_WidthBottom(float *pVal)
{
   *pVal = m_d.m_widthbottom;

   return S_OK;
}

STDMETHODIMP Ramp::put_WidthBottom(float newVal)
{
   STARTUNDO

   m_d.m_widthbottom = newVal;
   dynamicVertexBufferRegenerate = true;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Ramp::get_WidthTop(float *pVal)
{
   *pVal = m_d.m_widthtop;

   return S_OK;
}

STDMETHODIMP Ramp::put_WidthTop(float newVal)
{
   STARTUNDO

   m_d.m_widthtop = newVal;
   dynamicVertexBufferRegenerate = true;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Ramp::get_Color(OLE_COLOR *pVal)
{
   *pVal = m_d.m_color;

   return S_OK;
}

STDMETHODIMP Ramp::put_Color(OLE_COLOR newVal)
{
   STARTUNDO

   m_d.m_color = newVal;
   dynamicVertexBufferRegenerate = true;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Ramp::get_Type(RampType *pVal)
{
   *pVal = m_d.m_type;

   return S_OK;
}

STDMETHODIMP Ramp::put_Type(RampType newVal)
{
   STARTUNDO

   m_d.m_type = newVal;
   dynamicVertexBufferRegenerate = true;

   STOPUNDO

   return S_OK;
}

void Ramp::GetDialogPanes(Vector<PropertyPane> *pvproppane)
{
   PropertyPane *pproppane;

   pproppane = new PropertyPane(IDD_PROP_NAME, NULL);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPRAMP_VISUALS, IDS_VISUALS);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPRAMP_POSITION, IDS_POSITION);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPRAMP_PHYSICS, IDS_PHYSICS);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROP_TIMER, IDS_MISC);
   pvproppane->AddElement(pproppane);
}


STDMETHODIMP Ramp::get_Image(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_szImage, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Ramp::put_Image(BSTR newVal)
{
   STARTUNDO

   WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szImage, 32, NULL, NULL);
   dynamicVertexBufferRegenerate = true;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Ramp::get_ImageAlignment(RampImageAlignment *pVal)
{
   *pVal = m_d.m_imagealignment;

   return S_OK;
}

STDMETHODIMP Ramp::put_ImageAlignment(RampImageAlignment newVal)
{
   STARTUNDO

   m_d.m_imagealignment = newVal;
   dynamicVertexBufferRegenerate = true;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Ramp::get_HasWallImage(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fImageWalls);

   return S_OK;
}

STDMETHODIMP Ramp::put_HasWallImage(VARIANT_BOOL newVal)
{
   STARTUNDO

   m_d.m_fImageWalls = VBTOF(newVal);
   dynamicVertexBufferRegenerate = true;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Ramp::get_CastsShadow(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fCastsShadow);

   return S_OK;
}

STDMETHODIMP Ramp::put_CastsShadow(VARIANT_BOOL newVal)
{
   STARTUNDO
   
   m_d.m_fCastsShadow = VBTOF(newVal);
   
   STOPUNDO

   return S_OK;
}

STDMETHODIMP Ramp::get_Acrylic(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fAcrylic) && !(VARIANT_BOOL)FTOVB(m_d.m_fAlpha);

   return S_OK;
}

STDMETHODIMP Ramp::put_Acrylic(VARIANT_BOOL newVal)
{
   STARTUNDO
   
   m_d.m_fAcrylic = VBTOF(newVal);
   m_d.m_fAlpha = false;
   dynamicVertexBufferRegenerate = true;
   
   STOPUNDO

   return S_OK;
}

STDMETHODIMP Ramp::get_Alpha(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fAlpha);
   return S_OK;
}

STDMETHODIMP Ramp::put_Alpha(VARIANT_BOOL newVal)
{
   STARTUNDO
   
   m_d.m_fAlpha = VBTOF(newVal);
   dynamicVertexBufferRegenerate = true;
   
   STOPUNDO

   return S_OK;
}

STDMETHODIMP Ramp::get_Solid(VARIANT_BOOL *pVal)
{
   *pVal = !(VARIANT_BOOL)FTOVB(m_d.m_fAlpha) && !(VARIANT_BOOL)FTOVB(m_d.m_fAcrylic);
   return S_OK;
}

STDMETHODIMP Ramp::put_Solid(VARIANT_BOOL newVal)
{
   STARTUNDO
  
   if (VBTOF(newVal)) {
      m_d.m_fAlpha = false;
      m_d.m_fAcrylic = false;
      dynamicVertexBufferRegenerate = true;
   }
   
   STOPUNDO

   return S_OK;
}

STDMETHODIMP Ramp::get_LeftWallHeight(float *pVal)
{
   *pVal = m_d.m_leftwallheight;

   return S_OK;
}

STDMETHODIMP Ramp::put_LeftWallHeight(float newVal)
{
   STARTUNDO

   m_d.m_leftwallheight = newVal < 0 ? 0 : newVal;
   dynamicVertexBufferRegenerate = true;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Ramp::get_RightWallHeight(float *pVal)
{
   *pVal = m_d.m_rightwallheight;

   return S_OK;
}

STDMETHODIMP Ramp::put_RightWallHeight(float newVal)
{
   STARTUNDO

   m_d.m_rightwallheight = newVal < 0 ? 0 : newVal;
   dynamicVertexBufferRegenerate = true;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Ramp::get_VisibleLeftWallHeight(float *pVal)
{
   *pVal = m_d.m_leftwallheightvisible;

   return S_OK;
}

STDMETHODIMP Ramp::put_VisibleLeftWallHeight(float newVal)
{
   STARTUNDO

   m_d.m_leftwallheightvisible = newVal < 0 ? 0 : newVal;
   dynamicVertexBufferRegenerate = true;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Ramp::get_VisibleRightWallHeight(float *pVal)
{
   *pVal = m_d.m_rightwallheightvisible;

   return S_OK;
}

STDMETHODIMP Ramp::put_VisibleRightWallHeight(float newVal)
{
   STARTUNDO

   m_d.m_rightwallheightvisible = newVal < 0 ? 0 : newVal;
   dynamicVertexBufferRegenerate = true;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Ramp::get_Elasticity(float *pVal)
{
   *pVal = m_d.m_elasticity;

   return S_OK;
}

STDMETHODIMP Ramp::put_Elasticity(float newVal)
{
   STARTUNDO

   m_d.m_elasticity = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Ramp::get_Friction(float *pVal)
{
   *pVal = m_d.m_friction;

   return S_OK;
}

STDMETHODIMP Ramp::put_Friction(float newVal)
{
   STARTUNDO

   if (newVal > 1.0f) newVal = 1.0f;
   else if (newVal < 0) newVal = 0;
 
   m_d.m_friction = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Ramp::get_Scatter(float *pVal)
{
   *pVal = m_d.m_scatter;

   return S_OK;
}

STDMETHODIMP Ramp::put_Scatter(float newVal)
{
   STARTUNDO

   m_d.m_scatter = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Ramp::get_Collidable(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB((!g_pplayer) ? m_d.m_fCollidable : m_vhoCollidable.ElementAt(0)->m_fEnabled);

   return S_OK;
}

STDMETHODIMP Ramp::put_Collidable(VARIANT_BOOL newVal)
{
   BOOL fNewVal = VBTOF(newVal);	
   if (!g_pplayer)
   {	
      STARTUNDO

      m_d.m_fCollidable = fNewVal;		

      STOPUNDO
   }
   else for (int i=0;i < m_vhoCollidable.Size();i++)
   {
      m_vhoCollidable.ElementAt(i)->m_fEnabled = fNewVal;	//copy to hit checking on enities composing the object 
   }		

   return S_OK;
}

STDMETHODIMP Ramp::get_IsVisible(VARIANT_BOOL *pVal) //temporary value of object
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_IsVisible);

   return S_OK;
}

STDMETHODIMP Ramp::put_IsVisible(VARIANT_BOOL newVal)
{	
   if (!g_pplayer )
   {
      STARTUNDO

      m_d.m_IsVisible = VBTOF(newVal);			// set visibility
      
	  STOPUNDO
   }
   /*
   // OK this (ramp.isvisible in player) does only work in HS-Device Rendering Mode.
   // The problem is (if i'm right), that the ramp is drawed to the fixed backbuffer.
   // This did work only with HD-Device rendering and alpha ramps, since alpha ramps are drawn
   // in realtime. I think that, to get this working even in SWDR, the ramp had to be written 
   // to an own texture and should have to be blit on screen.   Cupid
   else 
   {
   if (invalidationRectCalculated && m_d.m_fAcrylic && m_d.m_fAlpha)
   {
   g_pplayer->InvalidateRect(&invalidationRect);
   m_d.m_IsVisible = VBTOF(newVal);
   }
   }
   */
   return S_OK;
}

STDMETHODIMP Ramp::get_Modify3DStereo(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fModify3DStereo);

   return S_OK;
}

STDMETHODIMP Ramp::put_Modify3DStereo(VARIANT_BOOL newVal)
{
   STARTUNDO

   m_d.m_fModify3DStereo = VBTOF(newVal);
   
   STOPUNDO

   return S_OK;
}

STDMETHODIMP Ramp::get_AddBlend(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fAddBlend);

   return S_OK;
}

STDMETHODIMP Ramp::put_AddBlend(VARIANT_BOOL newVal)
{
   STARTUNDO
   
   m_d.m_fAddBlend = VBTOF(newVal);
   
   STOPUNDO

   return S_OK;
}

// Always called each frame to render over everything else (along with primitives)
// Same code as RenderStatic (with the exception of the alpha tests).
// Also has less drawing calls by bundling seperate calls.
void Ramp::PostRenderStatic(const RenderDevice* _pd3dDevice)
{
   RenderDevice* pd3dDevice=(RenderDevice*)_pd3dDevice;
   // Don't render if invisible.
   if((!m_d.m_IsVisible) ||		
      // Don't render non-Alphas. 
      (!m_d.m_fAlpha)) return;

   if(dynamicVertexBufferRegenerate)
   {
      solidMaterial.setColor(1.0f, m_d.m_color );
   }

   if (m_d.m_type == RampType4Wire 
      || m_d.m_type == RampType1Wire //add check for 1 wire
      || m_d.m_type == RampType2Wire 
      || m_d.m_type == RampType3WireLeft 
      || m_d.m_type == RampType3WireRight)
   {
      if(dynamicVertexBufferRegenerate)
      {
         habitrailMaterial.setColor( 1.0f, m_d.m_color );
         habitrailMaterial.setPower( 8.0f );
         habitrailMaterial.setSpecular( 1.0f, 1.0f, 1.0f, 1.0f );
      }
      RenderStaticHabitrail(pd3dDevice);
   }
   else
   {	
      Pin3D * const ppin3d = &g_pplayer->m_pin3d;
      PinImage * const pin = m_ptable->GetImage(m_d.m_szImage);
      float maxtu = 0;
      float maxtv = 0;

      if (pin)
      {
         m_ptable->GetTVTU(pin, &maxtu, &maxtv);

         pin->EnsureColorKey();
         pd3dDevice->SetTexture(ePictureTexture, pin->m_pdsBufferColorKey);

         pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);
         pd3dDevice->SetRenderState(RenderDevice::DITHERENABLE, TRUE); 	

         ppin3d->EnableAlphaTestReference(0x00000001);
         pd3dDevice->SetRenderState(RenderDevice::SRCBLEND,  D3DBLEND_SRCALPHA);
         pd3dDevice->SetRenderState(RenderDevice::DESTBLEND, m_d.m_fAddBlend ? D3DBLEND_ONE : D3DBLEND_INVSRCALPHA);
         if(m_d.m_fAddBlend)
            pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_COLORARG2, D3DTA_TFACTOR); // factor is 1,1,1,1 by default -> do not modify tex by diffuse lighting

         ppin3d->SetColorKeyEnabled(TRUE);
         pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, m_d.m_fModify3DStereo); // do not update z if just a fake ramp (f.e. flasher fakes, etc)

         ppin3d->SetTextureFilter ( ePictureTexture, TEXTURE_MODE_TRILINEAR );

         textureMaterial.set();
      }
      else
      {
         solidMaterial.set();
      }

      unsigned int numVertices;

      if(dynamicVertexBufferRegenerate)
      {
         dynamicVertexBufferRegenerate = false;

         Vertex3D_NoTex2 *buf;
         dynamicVertexBuffer->lock(0,0,(void**)&buf, VertexBuffer::WRITEONLY | VertexBuffer::NOOVERWRITE);

         float *rgheight, *rgratio;
         const Vertex2D * const rgvLocal = GetRampVertex(rampVertex, &rgheight, NULL, &rgratio);

         const float inv_tablewidth = maxtu/(m_ptable->m_right - m_ptable->m_left);
         const float inv_tableheight = maxtv/(m_ptable->m_bottom - m_ptable->m_top);

         numVertices=(rampVertex-1)*4;
         unsigned int offset=0;
         for (int i=0;i<(rampVertex-1);i++)
         {
            Vertex3D_NoTex2 * const rgv3D = rgvbuf+i*4;
            rgv3D[0].x = rgvLocal[i].x;
            rgv3D[0].y = rgvLocal[i].y;
            rgv3D[0].z = rgheight[i];

            rgv3D[3].x = rgvLocal[i+1].x;
            rgv3D[3].y = rgvLocal[i+1].y;
            rgv3D[3].z = rgheight[i+1];

            rgv3D[2].x = rgvLocal[rampVertex*2-i-2].x;
            rgv3D[2].y = rgvLocal[rampVertex*2-i-2].y;
            rgv3D[2].z = rgheight[i+1];

            rgv3D[1].x = rgvLocal[rampVertex*2-i-1].x;
            rgv3D[1].y = rgvLocal[rampVertex*2-i-1].y;
            rgv3D[1].z = rgheight[i];

            if (pin)
            {
               if (m_d.m_imagealignment == ImageModeWorld)
               {
                  rgv3D[0].tu = rgv3D[0].x * inv_tablewidth;
                  rgv3D[0].tv = rgv3D[0].y * inv_tableheight;
                  rgv3D[1].tu = rgv3D[1].x * inv_tablewidth;
                  rgv3D[1].tv = rgv3D[1].y * inv_tableheight;
                  rgv3D[2].tu = rgv3D[2].x * inv_tablewidth;
                  rgv3D[2].tv = rgv3D[2].y * inv_tableheight;
                  rgv3D[3].tu = rgv3D[3].x * inv_tablewidth;
                  rgv3D[3].tv = rgv3D[3].y * inv_tableheight;
               }
               else
               {
                  rgv3D[0].tu = maxtu;
                  rgv3D[0].tv = rgratio[i] * maxtv;
                  rgv3D[1].tu = 0;
                  rgv3D[1].tv = rgratio[i] * maxtv;
                  rgv3D[2].tu = 0;
                  rgv3D[2].tv = rgratio[i+1] * maxtv;
                  rgv3D[3].tu = maxtu;
                  rgv3D[3].tv = rgratio[i+1] * maxtv;
               }
            }

            SetNormal(rgv3D, rgi0123, 4, NULL, NULL, NULL);
            // Draw the floor of the ramp.
            rgibuf[i*6]   = i*4;
            rgibuf[i*6+1] = i*4+1;
            rgibuf[i*6+2] = i*4+2;
            rgibuf[i*6+3] = i*4;
            rgibuf[i*6+4] = i*4+2;
            rgibuf[i*6+5] = i*4+3;

            invrgibuf[i*6]   = i*4;
            invrgibuf[i*6+1] = i*4+3;
            invrgibuf[i*6+2] = i*4+2;
            invrgibuf[i*6+3] = i*4;
            invrgibuf[i*6+4] = i*4+2;
            invrgibuf[i*6+5] = i*4+1;
         }
         memcpy( &buf[offset], rgvbuf, sizeof(Vertex3D_NoTex2)*numVertices );
         offset+=numVertices;

         for (int i=0;i<(rampVertex-1);i++)
         {
            Vertex3D_NoTex2 * const rgv3D = rgvbuf+i*4;
            rgv3D[2].x = rgvLocal[i+1].x;
            rgv3D[2].y = rgvLocal[i+1].y;
            rgv3D[2].z = rgheight[i+1] + m_d.m_rightwallheightvisible;

            rgv3D[1].x = rgvLocal[i].x;
            rgv3D[1].y = rgvLocal[i].y;
            rgv3D[1].z = rgheight[i] + m_d.m_rightwallheightvisible;

            if (pin && m_d.m_fImageWalls)
            {
               if (m_d.m_imagealignment == ImageModeWorld)
               {
                  rgv3D[0].tu = rgv3D[0].x * inv_tablewidth;
                  rgv3D[0].tv = rgv3D[0].y * inv_tableheight;
                  rgv3D[2].tu = rgv3D[2].x * inv_tablewidth;
                  rgv3D[2].tv = rgv3D[2].y * inv_tableheight;
               }
               else
               {
                  rgv3D[0].tu = maxtu;
                  rgv3D[0].tv = rgratio[i] * maxtv;
                  rgv3D[2].tu = maxtu;
                  rgv3D[2].tv = rgratio[i+1] * maxtv;
               }

               rgv3D[1].tu = rgv3D[0].tu;
               rgv3D[1].tv = rgv3D[0].tv;
               rgv3D[3].tu = rgv3D[2].tu;
               rgv3D[3].tv = rgv3D[2].tv;
            }

            // 2-Sided polygon
            SetNormal(rgv3D, rgi0123, 4, NULL, NULL, NULL);
         }
         memcpy( &buf[offset], rgvbuf, sizeof(Vertex3D_NoTex2)*numVertices );
         offset+=numVertices;

         // Flip Normals and redraw
         for (int i=0;i<(rampVertex-1);i++)
         {
            for(int j = 0; j < 4; ++j) {
               rgvbuf[i*4+j].nx = -rgvbuf[i*4+j].nx;
               rgvbuf[i*4+j].ny = -rgvbuf[i*4+j].ny;
               rgvbuf[i*4+j].nz = -rgvbuf[i*4+j].nz;
            }
         }
         memcpy( &buf[offset], rgvbuf, sizeof(Vertex3D_NoTex2)*numVertices );
         offset+=numVertices;

         // only calculate vertices if one or both sides are visible (!=0)
         if( m_d.m_leftwallheightvisible!=0.f || m_d.m_rightwallheightvisible!=0.f )
         {
            for (int i=0;i<(rampVertex-1);i++)
            {
               Vertex3D_NoTex2 * const rgv3D = rgvbuf+i*4;
               rgv3D[0].x = rgvLocal[rampVertex*2-i-2].x;
               rgv3D[0].y = rgvLocal[rampVertex*2-i-2].y;
               rgv3D[0].z = rgheight[i+1];

               rgv3D[3].x = rgvLocal[rampVertex*2-i-1].x;
               rgv3D[3].y = rgvLocal[rampVertex*2-i-1].y;
               rgv3D[3].z = rgheight[i];

               rgv3D[2].x = rgv3D[3].x;
               rgv3D[2].y = rgv3D[3].y;
               rgv3D[2].z = rgheight[i] + m_d.m_leftwallheightvisible;

               rgv3D[1].x = rgv3D[0].x;
               rgv3D[1].y = rgv3D[0].y;
               rgv3D[1].z = rgheight[i+1] + m_d.m_leftwallheightvisible;

               if (pin && m_d.m_fImageWalls)
               {
                  if (m_d.m_imagealignment == ImageModeWorld)
                  {
                     rgv3D[0].tu = rgv3D[0].x * inv_tablewidth;
                     rgv3D[0].tv = rgv3D[0].y * inv_tableheight;
                     rgv3D[2].tu = rgv3D[2].x * inv_tablewidth;
                     rgv3D[2].tv = rgv3D[2].y * inv_tableheight;
                  }
                  else
                  {
                     rgv3D[0].tu = 0;
                     rgv3D[0].tv = rgratio[i] * maxtv;
                     rgv3D[2].tu = 0;
                     rgv3D[2].tv = rgratio[i+1] * maxtv;
                  }

                  rgv3D[1].tu = rgv3D[0].tu;
                  rgv3D[1].tv = rgv3D[0].tv;
                  rgv3D[3].tu = rgv3D[2].tu;
                  rgv3D[3].tv = rgv3D[2].tv;
               }

               // 2-Sided polygon
               SetNormal(rgv3D, rgi0123, 4, NULL, NULL, NULL);
            }
            memcpy( &buf[offset], rgvbuf, sizeof(Vertex3D_NoTex2)*numVertices );
            offset+=numVertices;

            // Flip Normals and redraw
            for (int i=0;i<(rampVertex-1);i++)
            {
               for(int j = 0; j < 4; ++j) {
                  rgvbuf[i*4+j].nx = -rgvbuf[i*4+j].nx;
                  rgvbuf[i*4+j].ny = -rgvbuf[i*4+j].ny;
                  rgvbuf[i*4+j].nz = -rgvbuf[i*4+j].nz;
               }
            }
            memcpy( &buf[offset], rgvbuf, sizeof(Vertex3D_NoTex2)*numVertices );
         }

         dynamicVertexBuffer->unlock();

         delete [] rgvLocal;
         delete [] rgheight;
         delete [] rgratio;
      }
      else
         numVertices=(rampVertex-1)*4;

      unsigned int offset=0;
      pd3dDevice->renderPrimitive(D3DPT_TRIANGLELIST, dynamicVertexBuffer, offset, numVertices, (LPWORD)rgibuf, (rampVertex-1)*6, 0 );
      offset+=numVertices;

      if (pin && !m_d.m_fImageWalls)
      {
         ppin3d->SetTexture(NULL);
         solidMaterial.set();
      }

      if ( m_d.m_rightwallheightvisible!=0.f )
      {
         //only render right side if the height is >0
         pd3dDevice->renderPrimitive(D3DPT_TRIANGLELIST, dynamicVertexBuffer, offset, numVertices, (LPWORD)rgibuf, (rampVertex-1)*6, 0 );
         offset+=numVertices;
         pd3dDevice->renderPrimitive(D3DPT_TRIANGLELIST, dynamicVertexBuffer, offset, numVertices, (LPWORD)invrgibuf, (rampVertex-1)*6, 0 );
         offset+=numVertices;
      }
      else
      {
         offset+=2*numVertices;
      }

      if ( m_d.m_leftwallheightvisible!=0.f )
      {
         //only render left side if the height is >0
         pd3dDevice->renderPrimitive(D3DPT_TRIANGLELIST, dynamicVertexBuffer, offset, numVertices, (LPWORD)rgibuf, (rampVertex-1)*6, 0 );
         offset+=numVertices;
         pd3dDevice->renderPrimitive(D3DPT_TRIANGLELIST, dynamicVertexBuffer, offset, numVertices, (LPWORD)invrgibuf, (rampVertex-1)*6, 0 );
         offset+=numVertices;
      }

      pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);
      pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

      ppin3d->SetTexture(NULL);
   }
}
