#include "StdAfx.h"

Plunger::Plunger()
{
	m_phitplunger = NULL;
   vertexBuffer = NULL;
   verts = 0;
}

Plunger::~Plunger()
{
   if(vertexBuffer)
   {
      vertexBuffer->release();
      vertexBuffer = NULL;
   }
   if(verts)
   {
	   delete [] verts;
      verts=0;
   }
}

HRESULT Plunger::Init(PinTable *ptable, float x, float y, bool fromMouseClick)
	{
	m_ptable = ptable;

	m_d.m_v.x = x;
	m_d.m_v.y = y;

	SetDefaults(fromMouseClick);
	return InitVBA(fTrue, 0, NULL);
	}

void Plunger::SetDefaults(bool fromMouseClick)
	{
	HRESULT hr;
	float fTmp;
	int iTmp;

	hr = GetRegStringAsFloat("DefaultProps\\Plunger", "Height", &fTmp);
	if ((hr == S_OK) && fromMouseClick)
		m_d.m_height = fTmp;
	else
		m_d.m_height = 20;

	hr = GetRegStringAsFloat("DefaultProps\\Plunger", "Width", &fTmp);
	if ((hr == S_OK) && fromMouseClick)
		m_d.m_width = fTmp;
	else
		m_d.m_width = 25;

	hr = GetRegStringAsFloat("DefaultProps\\Plunger","Stroke", &fTmp);
	if ((hr == S_OK) && fromMouseClick)
		m_d.m_stroke = fTmp;
	else
		m_d.m_stroke = m_d.m_height*4;

	hr = GetRegStringAsFloat("DefaultProps\\Plunger","PullSpeed", &fTmp);
	if ((hr == S_OK) && fromMouseClick)
		m_d.m_speedPull = fTmp;
	else
		m_d.m_speedPull = 5;

	hr = GetRegInt("DefaultProps\\Plunger","PlungerType", &iTmp);
	if ((hr == S_OK) && fromMouseClick)
		m_d.m_type = (enum PlungerType)iTmp;
	else
		m_d.m_type = PlungerTypeOrig;

	hr = GetRegInt("DefaultProps\\Plunger","Color", &iTmp);
	if ((hr == S_OK) && fromMouseClick)
		m_d.m_color = iTmp;
	else
		m_d.m_color = RGB(76,76,76);

	hr = GetRegString("DefaultProps\\Plunger","Image", m_d.m_szImage, MAXTOKEN);
	if ((hr != S_OK) || !fromMouseClick)
		m_d.m_szImage[0] = 0;

	hr = GetRegStringAsFloat("DefaultProps\\Plunger","ReleaseSpeed", &fTmp);
	if ((hr == S_OK) && fromMouseClick)
		m_d.m_speedFire = fTmp;
	else
		m_d.m_speedFire = 80;
	
	hr = GetRegInt("DefaultProps\\Plunger","TimerEnabled", &iTmp);
	if ((hr == S_OK) && fromMouseClick)
		m_d.m_tdr.m_fTimerEnabled = iTmp == 0 ? false : true;
	else
		m_d.m_tdr.m_fTimerEnabled = false;
	
	hr = GetRegInt("DefaultProps\\Plunger","TimerInterval", &iTmp);
	if ((hr == S_OK) && fromMouseClick)
		m_d.m_tdr.m_TimerInterval = iTmp;
	else
		m_d.m_tdr.m_TimerInterval = 100;

	hr = GetRegString("DefaultProps\\Plunger", "Surface", &m_d.m_szSurface, MAXTOKEN);
	if ((hr != S_OK) || !fromMouseClick)
		m_d.m_szSurface[0] = 0;

	hr = GetRegInt("DefaultProps\\Plunger","MechPlunger", &iTmp);
	if ((hr == S_OK) && fromMouseClick)
		m_d.m_mechPlunger = iTmp == 0 ? false : true;
	else
		m_d.m_mechPlunger = fFalse;		// plungers require selection for mechanical input
	
	hr = GetRegInt("DefaultProps\\Plunger","AutoPlunger", &iTmp);
	if ((hr == S_OK) && fromMouseClick)
		m_d.m_autoPlunger = iTmp == 0 ? false : true;
	else
		m_d.m_autoPlunger = fFalse;		
	
	hr = GetRegStringAsFloat("DefaultProps\\Plunger","MechStrength", &fTmp);
	if ((hr == S_OK) && fromMouseClick)
		m_d.m_mechStrength = fTmp;
	else
		m_d.m_mechStrength = 85;
	
	hr = GetRegStringAsFloat("DefaultProps\\Plunger","ParkPosition", &fTmp);
	if ((hr == S_OK) && fromMouseClick)
		m_d.m_parkPosition = fTmp;
	else
		m_d.m_parkPosition = (float)(0.5/3.0);	// typical mechanical plunger has 3 inch stroke and 0.5 inch rest position

	hr = GetRegInt("DefaultProps\\Plunger","Visible", &iTmp);
	if ((hr == S_OK) && fromMouseClick)
		m_d.m_fVisible = iTmp == 0 ? false : true;
	else
		m_d.m_fVisible = fTrue;

	hr = GetRegStringAsFloat("DefaultProps\\Plunger","ScatterVelocity", &fTmp);
	if ((hr == S_OK) && fromMouseClick)
		m_d.m_scatterVelocity = fTmp;
	else
		m_d.m_scatterVelocity = 0;
	
	hr = GetRegStringAsFloat("DefaultProps\\Plunger","BreakOverVelocity", &fTmp);
	if ((hr == S_OK) && fromMouseClick)
		m_d.m_breakOverVelocity = fTmp;
	else
		m_d.m_breakOverVelocity = 18.0f;
	}

void Plunger::WriteRegDefaults()
	{
	char strTmp[40];

	sprintf_s(strTmp, 40, "%f", m_d.m_height);
	SetRegValue("DefaultProps\\Plunger","Height", REG_SZ, &strTmp,strlen(strTmp));
	sprintf_s(strTmp, 40, "%f", m_d.m_width);
	SetRegValue("DefaultProps\\Plunger","Width", REG_SZ, &strTmp,strlen(strTmp));
	sprintf_s(strTmp, 40, "%f", m_d.m_stroke);
	SetRegValue("DefaultProps\\Plunger","Stroke", REG_SZ, &strTmp,strlen(strTmp));
	sprintf_s(strTmp, 40, "%f", m_d.m_speedPull);
	SetRegValue("DefaultProps\\Plunger","PullSpeed", REG_SZ, &strTmp,strlen(strTmp));
	sprintf_s(strTmp, 40, "%f", m_d.m_speedFire);
	SetRegValue("DefaultProps\\Plunger","ReleaseSpeed", REG_SZ, &strTmp,strlen(strTmp));
	SetRegValue("DefaultProps\\Plunger","PlungerType",REG_DWORD,&m_d.m_type,4);
	SetRegValue("DefaultProps\\Plunger","Color",REG_DWORD,&m_d.m_color,4);
	SetRegValue("DefaultProps\\Plunger","Image", REG_SZ, &m_d.m_szImage, strlen(m_d.m_szImage));
	SetRegValue("DefaultProps\\Plunger","TimerEnabled",REG_DWORD, &m_d.m_tdr.m_fTimerEnabled,4);
	SetRegValue("DefaultProps\\Plunger","TimerInterval",REG_DWORD, &m_d.m_tdr.m_TimerInterval,4);
	SetRegValue("DefaultProps\\Plunger","Surface", REG_SZ, &m_d.m_szSurface, strlen(m_d.m_szSurface));
	SetRegValue("DefaultProps\\Plunger","MechPlunger",REG_DWORD, &m_d.m_mechPlunger,4);
	SetRegValue("DefaultProps\\Plunger","AutoPlunger",REG_DWORD, &m_d.m_autoPlunger,4);
	sprintf_s(strTmp, 40, "%f", m_d.m_mechStrength);
	SetRegValue("DefaultProps\\Plunger","MechStrength", REG_SZ, &strTmp,strlen(strTmp));
	sprintf_s(strTmp, 40, "%f", m_d.m_parkPosition);
	SetRegValue("DefaultProps\\Plunger","ParkPosition", REG_SZ, &strTmp,strlen(strTmp));
	SetRegValue("DefaultProps\\Plunger","Visible",REG_DWORD, &m_d.m_fVisible,4);
	sprintf_s(strTmp, 40, "%f", m_d.m_scatterVelocity);
	SetRegValue("DefaultProps\\Plunger","ScatterVelocity", REG_SZ, &strTmp,strlen(strTmp));
	sprintf_s(strTmp, 40, "%f", m_d.m_breakOverVelocity);
	SetRegValue("DefaultProps\\Plunger","BreakOverVelocity", REG_SZ, &strTmp,strlen(strTmp));
	}

void Plunger::PreRender(Sur * const psur)
	{
	}

void Plunger::Render(Sur * const psur)
	{
	psur->SetBorderColor(RGB(0,0,0),false,0);
	psur->SetFillColor(-1);
	psur->SetObject(this);

	psur->Rectangle(m_d.m_v.x - m_d.m_width, m_d.m_v.y - m_d.m_stroke,
			        m_d.m_v.x + m_d.m_width, m_d.m_v.y + m_d.m_height);
	}

void Plunger::GetHitShapes(Vector<HitObject> * const pvho)
	{
	const float zheight = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_v.x, m_d.m_v.y);

	HitPlunger * const php = new HitPlunger(m_d.m_v.x - m_d.m_width,
											m_d.m_v.y + m_d.m_height, m_d.m_v.x + m_d.m_width,
											m_d.m_v.y - m_d.m_stroke, zheight, this);

	php->m_pfe = NULL;

	php->m_plungeranim.m_frameStart = m_d.m_v.y;
	php->m_plungeranim.m_frameEnd = m_d.m_v.y - m_d.m_stroke;

	pvho->AddElement(php);
	php->m_pplunger = this;
	m_phitplunger = php;
	}

void Plunger::GetHitShapesDebug(Vector<HitObject> * const pvho)
	{
	}

void Plunger::GetTimers(Vector<HitTimer> * const pvht)
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

void Plunger::EndPlay()
	{
	if (m_phitplunger) // Failed Player case
		{
		for (int i=0;i<m_phitplunger->m_plungeranim.m_vddsFrame.Size();i++)
			{
			delete m_phitplunger->m_plungeranim.m_vddsFrame.ElementAt(i);
			}

		m_phitplunger = NULL;
		}

	IEditable::EndPlay();
   if(vertexBuffer)
   {
      vertexBuffer->release();
      vertexBuffer = NULL;
   }
}

void Plunger::SetObjectPos()
	{
	g_pvp->SetObjectPosCur(m_d.m_v.x, m_d.m_v.y);
	}

void Plunger::MoveOffset(const float dx, const float dy)
	{
	m_d.m_v.x += dx;
	m_d.m_v.y += dy;
	m_ptable->SetDirtyDraw();
	}

void Plunger::GetCenter(Vertex2D * const pv) const
	{
	*pv = m_d.m_v;
	}

void Plunger::PutCenter(const Vertex2D * const pv)
	{
	m_d.m_v = *pv;

	m_ptable->SetDirtyDraw();
	}

void Plunger::PostRenderStatic(const RenderDevice* pd3dDevice)
	{
	}
const float rgcrossplunger0[][2] =
{
   1.0f, 0.0f,
   1.0f, 10.0f,
   0.35f, 20.0f,
   0.35f, 24.0f,
   0.35f, 100.0f
};

const float rgcrossplungerNormal0[][2] =
{
   1.0f, 0.0f,
   0.8f, 0.6f,
   0.0f, 1.0f,
   1.0f, 0.0f,
   1.0f, 0.0f
};

const float rgcrossplunger1[][3] =
{
   //new plunger tip - Added by rascal 
   // the third value is the v texture coordinate for the tip
   // index 0-4/5 is the tip 5 is the ring between tip and the rest of the plunger
   // index 6 is the rest of the plunger
   0.20f, 0.0f,   0.0f, 
   0.30f, 3.0f,   0.11f,
   0.35f, 5.0f,   0.14f,
   0.35f, 23.0f,  0.19f,
   0.45f, 23.0f,  0.21f,
   0.25f, 24.0f,  0.25f,
   0.25f, 100.0f, 1.0f
};

const float rgcrossplungerNormal1[][2] = {
   //new plunger tip white
   1.0f, 0.0f,
   1.0f, 0.0f,
   1.0f, 0.0f,
   1.0f, 0.0f,
   0.8f, 0.0f,
   0.3f, 0.0f,
   0.3f, 0.0f
};

const float rgPlunger[][2]=
{
   0.0f, 0.0f,
   0.0f, 1.0f,
   1.0f, 1.0f,
   1.0f, 0.0f
};

#define PLUNGER_FRAME_COUNT 25   //frame per 80 units distance

void Plunger::RenderSetup(const RenderDevice* _pd3dDevice )
{
   const float zheight = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_v.x, m_d.m_v.y);
   const float stroke = m_d.m_stroke;
   const float beginy = m_d.m_v.y;
   const float endy = m_d.m_v.y - stroke;
   cframes = (int)((float)PLUNGER_FRAME_COUNT * (stroke*(float)(1.0/80.0))) + 1; // 25 frames per 80 units travel
   const float inv_cframes = (cframes > 1) ? ((endy - beginy)/(float)(cframes-1)) : 0.0f;
   const float inv_scale = (cframes > 1) ? (1.0f/(float)(cframes-1)) : 0.0f;

   material.setColor( 1.f, m_d.m_color );
   material.setPower( 8.0f );
   material.setSpecular( 1.0f, 1.0f, 1.0f, 1.0f );

   if(verts)
	   delete [] verts;
   verts = new Vertices[cframes];
   if ( vertexBuffer == NULL )
   {
      if ( m_d.m_type == PlungerTypeModern )
      {
         g_pplayer->m_pin3d.m_pd3dDevice->createVertexBuffer( cframes*16*PLUNGEPOINTS1, 0, MY_D3DFVF_NOTEX2_VERTEX, &vertexBuffer );
         NumVideoBytes += (cframes*16*PLUNGEPOINTS1)*sizeof(Vertex3D_NoTex2);
      }
      else
      {
         g_pplayer->m_pin3d.m_pd3dDevice->createVertexBuffer( cframes*16*PLUNGEPOINTS0, 0, MY_D3DFVF_NOTEX2_VERTEX, &vertexBuffer );
         NumVideoBytes += (cframes*16*PLUNGEPOINTS0)*sizeof(Vertex3D_NoTex2);
      }
   }

   int vbOffset=0;
   Vertex3D_NoTex2 *buf;
   vertexBuffer->lock(0,0,(void**)&buf, VertexBuffer::WRITEONLY | VertexBuffer::NOOVERWRITE);

   renderNewPlunger=false;
/*
   //This is a special test case if the mapping of the moddern plunger isn't usable for everyone
   //if renderNewPlunger is true a simple rectangle is drawn with an alpha map, it's the same like the alpha plunger method
   if ( m_d.m_type == PlungerTypeModern )
   {
      Texture *pin = m_ptable->GetImage(m_d.m_szImage);
      if ( pin )
      {
         renderNewPlunger=true;
      }
   }
*/
   for ( int i=0;i<cframes; i++ )
   {
      const float height = beginy + inv_cframes*(float)i;
      Vertex3D_NoTex2 *ptr = verts[i].moverVertices;

      if (m_d.m_type == PlungerTypeModern)
      {
         if ( !renderNewPlunger )
         {
            // creating the modern plunger by going in a circle for each angle seven points on the Y axis are calculated
            int k=0;
            // start around the middle of the texture otherwise it looks odd
            float tu=0.51f;
            const float stepU = 1.0f/16.0f;
            for (int l=0;l<16;l++, tu+=stepU)
            {
               if ( tu>1.0f ) tu=1.0f-tu;
               const float angle = (float)(M_PI*2.0/16.0)*(float)l;
               const float sn = sinf(angle);
               const float cs = cosf(angle);
               const int offset = l*PLUNGEPOINTS1;
               for (int m=0;m<PLUNGEPOINTS1;m++)
               {
                  float y = height + rgcrossplunger1[m][1];
                  ptr[m + offset].x = rgcrossplunger1[m][0] * (sn * m_d.m_width) + m_d.m_v.x;
                  ptr[m + offset].y = y;
                  ptr[m + offset].z = rgcrossplunger1[m][0] * (cs * m_d.m_width) + m_d.m_width + zheight;
                  ptr[m + offset].nx = rgcrossplungerNormal1[m][0] * sn;
                  ptr[m + offset].ny = rgcrossplungerNormal1[m][1];
                  ptr[m + offset].nz = -rgcrossplungerNormal1[m][0] * cs;
                  ptr[m + offset].tu = tu;
                  ptr[m + offset].tv = rgcrossplunger1[m][2];
                  if( m<PLUNGEPOINTS1-1 )
                  {
                     indices[k  ] = m+offset;
                     indices[k+1] = (m + offset + PLUNGEPOINTS1) % (16*PLUNGEPOINTS1);
                     indices[k+2] = (m + offset + 1 + PLUNGEPOINTS1) % (16*PLUNGEPOINTS1);
                     indices[k+3] = m + offset +1;
                     k+=4;
                  }
               }
               ptr[PLUNGEPOINTS1-1 + offset].y = m_d.m_v.y + m_d.m_height; // cuts off at bottom (bottom of shaft disappears)
            }
            memcpy( &buf[vbOffset], ptr, 16*PLUNGEPOINTS1*sizeof(Vertex3D_NoTex2));
            vbOffset += (16*PLUNGEPOINTS1);
         }
         else
         {
            const float tv = (float)i*inv_scale;
            ptr[0].x = m_d.m_v.x;               ptr[0].nx = 0.0f;          ptr[0].tu = 0.0f;
            ptr[0].y = m_d.m_v.y;               ptr[0].ny = 0.0f;          ptr[0].tv = tv;
            ptr[0].z = m_d.m_width+zheight;     ptr[0].nz = -1.0f;
            ptr[1].x = m_d.m_v.x;               ptr[1].nx = 0.0f;          ptr[1].tu = 0.0f;
            ptr[1].y = height;                  ptr[1].ny = 0.0f;          ptr[1].tv = 0.0f;
            ptr[1].z = m_d.m_width+zheight;     ptr[1].nz = -1.0f;
            ptr[2].x = m_d.m_v.x+m_d.m_width;   ptr[2].nx = 0.0f;          ptr[2].tu = 1.0f;
            ptr[2].y = height;                  ptr[2].ny = 0.0f;          ptr[2].tv = 0.0f;
            ptr[2].z = m_d.m_width+zheight;     ptr[2].nz = -1.0f;
            ptr[3].x = m_d.m_v.x+m_d.m_width;   ptr[3].nx = 0.0f;          ptr[3].tu = 1.0f;
            ptr[3].y = m_d.m_v.y;               ptr[3].ny = 0.0f;          ptr[3].tv = tv;
            ptr[3].z = m_d.m_width+zheight;     ptr[3].nz = -1.0f;
            memcpy( &buf[vbOffset], ptr, 4*sizeof(Vertex3D_NoTex2));
            vbOffset += 4;
         }
      }
      else if (m_d.m_type == PlungerTypeOrig)
      {
         int k=0;
         for (int l=0;l<16;l++)
         {
            const float angle = (float)(M_PI*2.0/16.0)*(float)l;
            const float sn = sinf(angle);
            const float cs = cosf(angle);
            const int offset = l*PLUNGEPOINTS0;
            for (int m=0;m<PLUNGEPOINTS0;m++)
            {
               ptr[m + offset].x = rgcrossplunger0[m][0] * (sn * m_d.m_width) + m_d.m_v.x;
               ptr[m + offset].y = height + rgcrossplunger0[m][1];
               ptr[m + offset].z = rgcrossplunger0[m][0] * (cs * m_d.m_width) + m_d.m_width + zheight;
               ptr[m + offset].nx = rgcrossplungerNormal0[m][0] * sn;
               ptr[m + offset].ny = rgcrossplungerNormal0[m][1];
               ptr[m + offset].nz = -rgcrossplungerNormal0[m][0] * cs;
               if( m<PLUNGEPOINTS0-1 )
               {
                  indices[k  ] = m+offset;
                  indices[k+1] = (m + offset + PLUNGEPOINTS0) % (16*PLUNGEPOINTS0);
                  indices[k+2] = (m + offset + 1 + PLUNGEPOINTS0) % (16*PLUNGEPOINTS0);
                  indices[k+3] = m + offset +1;
                  k+=4;
               }

            }
            ptr[PLUNGEPOINTS0-1 + offset].y = m_d.m_v.y + m_d.m_height; // cuts off at bottom (bottom of shaft disappears)
         }
         memcpy( &buf[vbOffset], ptr, 16*PLUNGEPOINTS0*sizeof(Vertex3D_NoTex2));
         vbOffset += (16*PLUNGEPOINTS0);
      }
   }
   vertexBuffer->unlock();
}

void Plunger::RenderStatic(const RenderDevice* pd3dDevice)
	{
	}

void Plunger::RenderMovers(const RenderDevice* _pd3dDevice)
{
   RenderDevice* pd3dDevice = (RenderDevice*)_pd3dDevice;
   if(m_d.m_fVisible)
   {
      _ASSERTE(m_phitplunger);
      Pin3D * const ppin3d = &g_pplayer->m_pin3d;
      Texture *pin = NULL;

      material.set();
      ppin3d->ClearSpriteRectangle( &m_phitplunger->m_plungeranim, NULL );
      pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);

      for (int i=0;i<cframes;i++)
      {
         Vertex3D_NoTex2 *ptr = verts[i].moverVertices;

         ObjFrame * const pof = new ObjFrame();

         if (m_d.m_type == PlungerTypeModern)
         {
            ppin3d->ClearSpriteRectangle( NULL, pof );
            if ( renderNewPlunger )
            {
               //render a simple rectangle as an embedded alpha ramp plunger ;)
               ppin3d->ExpandExtents(&pof->rc, ptr, &m_phitplunger->m_plungeranim.m_znear, &m_phitplunger->m_plungeranim.m_zfar, 4, fFalse);
               pin = m_ptable->GetImage(m_d.m_szImage);
               pin->CreateAlphaChannel();
               pin->Set(ePictureTexture);
               pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);
               pd3dDevice->SetRenderState(RenderDevice::LIGHTING, FALSE );
               ppin3d->EnableAlphaBlend( 1, false );
               ppin3d->SetTextureFilter ( ePictureTexture, TEXTURE_MODE_TRILINEAR );
               const WORD idx[6] = {0,1,2,2,3,0};
               pd3dDevice->renderPrimitive( D3DPT_TRIANGLELIST, vertexBuffer, i*4, 4, (LPWORD)idx, 6, 0 );   
               pin->Unset(ePictureTexture);
               pd3dDevice->SetRenderState(RenderDevice::LIGHTING, TRUE );
            }
            else
            {
               pin = m_ptable->GetImage(m_d.m_szImage);
               if ( pin )
               {
                  pin->CreateAlphaChannel();
                  pin->Set(ePictureTexture);
                  pd3dDevice->SetRenderState(RenderDevice::LIGHTING, FALSE );
                  ppin3d->EnableAlphaBlend( 1, false );
                  ppin3d->SetTextureFilter ( ePictureTexture, TEXTURE_MODE_TRILINEAR );
               }

               int k=0;
               ppin3d->ExpandExtents(&pof->rc, ptr, &m_phitplunger->m_plungeranim.m_znear, &m_phitplunger->m_plungeranim.m_zfar, (16*PLUNGEPOINTS1), fFalse);
               for (int l=0;l<16;l++)
               {
                  for (int m=0;m<(PLUNGEPOINTS1-1);m++,k+=4)
                  {
                     pd3dDevice->renderPrimitive( D3DPT_TRIANGLEFAN, vertexBuffer, i*(16*PLUNGEPOINTS1), (16*PLUNGEPOINTS1), (LPWORD)&indices[k], 4, 0 );
                  }
               }
               if ( pin )
               {
                  pin->Unset(ePictureTexture);
                  pd3dDevice->SetRenderState(RenderDevice::LIGHTING, TRUE );
               }
            }
         }
         else if (m_d.m_type == PlungerTypeOrig)
         {
            ppin3d->ClearSpriteRectangle( NULL, pof );
            ppin3d->ExpandExtents(&pof->rc, ptr, &m_phitplunger->m_plungeranim.m_znear, &m_phitplunger->m_plungeranim.m_zfar, (16*PLUNGEPOINTS0), fFalse);

            int k=0;
            for (int l=0;l<16;l++)
            {
               const int offset = l*PLUNGEPOINTS0;
               for (int m=0;m<(PLUNGEPOINTS0-1);m++,k+=4)
               {
                  pd3dDevice->renderPrimitive( D3DPT_TRIANGLEFAN, vertexBuffer, i*(16*PLUNGEPOINTS0), (16*PLUNGEPOINTS0), (LPWORD)&indices[k], 4, 0 );
               }
            }
         }
         ppin3d->CreateAndCopySpriteBuffers( &m_phitplunger->m_plungeranim, pof );
         m_phitplunger->m_plungeranim.m_vddsFrame.AddElement(pof);
      }
   }
}

STDMETHODIMP Plunger::InterfaceSupportsErrorInfo(REFIID riid)
{
   static const IID* arr[] =
   {
      &IID_IPlunger,
   };

   for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
   {
      if (InlineIsEqualGUID(*arr[i],riid))
         return S_OK;
   }
   return S_FALSE;
}

HRESULT Plunger::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
	{
	BiffWriter bw(pstm, hcrypthash, hcryptkey);

#ifdef VBA
	bw.WriteInt(FID(PIID), ApcProjectItem.ID());
#endif
	bw.WriteStruct(FID(VCEN), &m_d.m_v, sizeof(Vertex2D));
	bw.WriteFloat(FID(WDTH), m_d.m_width);
	bw.WriteFloat(FID(HIGH), m_d.m_height);
	bw.WriteFloat(FID(HPSL), m_d.m_stroke);
	bw.WriteFloat(FID(SPDP), m_d.m_speedPull);
	bw.WriteFloat(FID(SPDF), m_d.m_speedFire);
	bw.WriteInt(FID(TYPE), m_d.m_type);
	bw.WriteInt(FID(COLR), m_d.m_color);
	bw.WriteString(FID(IMAG), m_d.m_szImage);

	bw.WriteFloat(FID(MESTH), m_d.m_mechStrength);
	bw.WriteBool(FID(MECH), m_d.m_mechPlunger);
	bw.WriteBool(FID(APLG), m_d.m_autoPlunger);
	
	bw.WriteFloat(FID(MPRK), m_d.m_parkPosition);
	bw.WriteFloat(FID(PSCV), m_d.m_scatterVelocity);
	bw.WriteFloat(FID(PBOV), m_d.m_breakOverVelocity);

	bw.WriteBool(FID(TMON), m_d.m_tdr.m_fTimerEnabled);
	bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
	bw.WriteBool(FID(VSBL), m_d.m_fVisible);
	bw.WriteString(FID(SURF), m_d.m_szSurface);
	bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);

	ISelect::SaveData(pstm, hcrypthash, hcryptkey);

	bw.WriteTag(FID(ENDB));

	return S_OK;
	}

HRESULT Plunger::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
	m_d.m_color = RGB(76,76,76); //initialize color for new plunger
	SetDefaults(false);

	BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);

	m_ptable = ptable;

	br.Load();
	return S_OK;
}

BOOL Plunger::LoadToken(int id, BiffReader *pbr)
	{
	if (id == FID(PIID))
		{
		pbr->GetInt((int *)pbr->m_pdata);
		}
	else if (id == FID(VCEN))
		{
		pbr->GetStruct(&m_d.m_v, sizeof(Vertex2D));
		}
	else if (id == FID(WDTH))
		{
		pbr->GetFloat(&m_d.m_width);
		m_d.m_width = 25.0f;
		}
	else if (id == FID(HIGH))
		{
		pbr->GetFloat(&m_d.m_height);
		}
	else if (id == FID(HPSL))
		{
		pbr->GetFloat(&m_d.m_stroke);
		}
	else if (id == FID(SPDP))
		{
		pbr->GetFloat(&m_d.m_speedPull);
		}
	else if (id == FID(SPDF))
		{
		pbr->GetFloat(&m_d.m_speedFire);
		}
	else if (id == FID(MESTH))
		{
		pbr->GetFloat(&m_d.m_mechStrength);
		}
	else if (id == FID(MPRK))
		{
		pbr->GetFloat(&m_d.m_parkPosition);
		}
	else if (id == FID(PSCV))
		{
		pbr->GetFloat(&m_d.m_scatterVelocity);
		}
	else if (id == FID(PBOV))
		{
		pbr->GetFloat(&m_d.m_breakOverVelocity);
		}
	else if (id == FID(TMON))
		{
		pbr->GetBool(&m_d.m_tdr.m_fTimerEnabled);
		}
	else if (id == FID(MECH))
		{
		pbr->GetBool(&m_d.m_mechPlunger);
		}
	else if (id == FID(APLG))
		{
		pbr->GetBool(&m_d.m_autoPlunger);
		}
	else if (id == FID(TMIN))
		{
		pbr->GetInt(&m_d.m_tdr.m_TimerInterval);
		}
	else if (id == FID(NAME))
		{
		pbr->GetWideString((WCHAR *)m_wzName);
		}
	else if (id == FID(TYPE))
		{
		pbr->GetInt(&m_d.m_type);
		}
	else if (id == FID(COLR))
		{
		pbr->GetInt(&m_d.m_color);
	//	if (!(m_d.m_color & MINBLACKMASK)) {m_d.m_color |= MINBLACK;}	// set minimum black
		}
	else if (id == FID(IMAG))
		{
		pbr->GetString(m_d.m_szImage);
		}
	else if (id == FID(VSBL))
		{
		pbr->GetBool(&m_d.m_fVisible);
		}
	else if (id == FID(SURF))
		{
		pbr->GetString(m_d.m_szSurface);
		}
	else
		{
		ISelect::LoadToken(id, pbr);
		}
	return fTrue;
	}

HRESULT Plunger::InitPostLoad()
	{
	return S_OK;
	}

STDMETHODIMP Plunger::PullBack()
{
	if (m_phitplunger)
		{
		m_phitplunger->m_plungeranim.m_posdesired = m_d.m_v.y;
		m_phitplunger->m_plungeranim.m_speed = 0;  // m_d.m_speedPull
		m_phitplunger->m_plungeranim.m_force = m_d.m_speedPull;

		if (m_phitplunger->m_plungeranim.m_mechTimeOut <= 0)
			{			
			m_phitplunger->m_plungeranim.m_fAcc = true;
			}
		}

	return S_OK;
}

STDMETHODIMP Plunger::MotionDevice(int *pVal)
{
	*pVal=g_pplayer->m_pininput.uShockType;

	return S_OK;
}

STDMETHODIMP Plunger::Position(int *pVal)
{
//	*pVal=m_curMechPlungerPos;
	if (g_pplayer->m_pininput.uShockType == USHOCKTYPE_PBWIZARD)
	{
		const float range = (float)JOYRANGEMX * (1.0f - m_d.m_parkPosition) - (float)JOYRANGEMN *m_d.m_parkPosition; // final range limit
		float tmp = (g_pplayer->m_curMechPlungerPos < 0.f) ? g_pplayer->m_curMechPlungerPos*m_d.m_parkPosition : (g_pplayer->m_curMechPlungerPos*(1.0f - m_d.m_parkPosition));
		tmp = tmp/range + m_d.m_parkPosition;		//scale and offset
		*pVal = (int)(tmp*(float)(1.0/0.04));
	}

	if (g_pplayer->m_pininput.uShockType == USHOCKTYPE_ULTRACADE)
	{
		const float range = (float)JOYRANGEMX * (1.0f - m_d.m_parkPosition) - (float)JOYRANGEMN *m_d.m_parkPosition; // final range limit
		float tmp = (g_pplayer->m_curMechPlungerPos < 0.f) ? g_pplayer->m_curMechPlungerPos*m_d.m_parkPosition : (g_pplayer->m_curMechPlungerPos*(1.0f - m_d.m_parkPosition));
		tmp = tmp/range + m_d.m_parkPosition;		//scale and offset
		*pVal = (int)(tmp*(float)(1.0/0.04));
	}

	if (g_pplayer->m_pininput.uShockType == USHOCKTYPE_SIDEWINDER)
	{
		const float range = (float)JOYRANGEMX * (1.0f - m_d.m_parkPosition) - (float)JOYRANGEMN *m_d.m_parkPosition; // final range limit
		float tmp = (g_pplayer->m_curMechPlungerPos < 0.f) ? g_pplayer->m_curMechPlungerPos*m_d.m_parkPosition : (g_pplayer->m_curMechPlungerPos*(1.0f - m_d.m_parkPosition));
		tmp = tmp/range + m_d.m_parkPosition;		//scale and offset
		*pVal = (int)(tmp*(float)(1.0/0.04));
	}

	if (g_pplayer->m_pininput.uShockType == USHOCKTYPE_VIRTUAPIN)
	{
		const float range = (float)JOYRANGEMX * (1.0f - m_d.m_parkPosition) - (float)JOYRANGEMN *m_d.m_parkPosition; // final range limit
		float tmp = (g_pplayer->m_curMechPlungerPos < 0.f) ? g_pplayer->m_curMechPlungerPos*m_d.m_parkPosition : (g_pplayer->m_curMechPlungerPos*(1.0f - m_d.m_parkPosition));
		tmp = tmp/range + m_d.m_parkPosition;		//scale and offset
		*pVal = (int)(tmp*(float)(1.0/0.04));
	}

	if (g_pplayer->m_pininput.uShockType == USHOCKTYPE_GENERIC)
	{
		const float range = (float)JOYRANGEMX * (1.0f - m_d.m_parkPosition) - (float)JOYRANGEMN *m_d.m_parkPosition; // final range limit
		float tmp = (g_pplayer->m_curMechPlungerPos < 0.f) ? g_pplayer->m_curMechPlungerPos*m_d.m_parkPosition : (g_pplayer->m_curMechPlungerPos*(1.0f - m_d.m_parkPosition));
		tmp = tmp/range + m_d.m_parkPosition;		//scale and offset
		*pVal = (int)(tmp*(float)(1.0/0.04));
	}

//	return tmp;

//	float range = (float)JOYRANGEMX * (1.0f - m_d.m_parkPosition) - (float)JOYRANGEMN *m_d.m_parkPosition; // final range limit
//	float tmp = ((float)JOYRANGEMN-1 < 0) ? (float)JOYRANGEMN-1*m_d.m_parkPosition : (float)JOYRANGEMN-1*(1.0f - m_d.m_parkPosition);
//	tmp = tmp/range + m_d.m_parkPosition;		//scale and offset
//	*pVal = tmp;
	return S_OK;
}

STDMETHODIMP Plunger::Fire()
{
	if (m_phitplunger)
	{
		// Check if this is an auto-plunger.
		// They always use max strength (as in a button).
		if ( m_d.m_autoPlunger ) 
		{
			// Use max strength.
			// I don't understand where the strength for the button plunger is coming from.
			// The strength is always larger than the mechanical one.  So I scaled by a constant.
			m_phitplunger->m_plungeranim.m_posdesired = m_d.m_v.y; 
			m_phitplunger->m_plungeranim.m_speed = 0;
			m_phitplunger->m_plungeranim.m_force = m_d.m_mechStrength * -1.0613f;

			m_phitplunger->m_plungeranim.m_fAcc = true;			
			m_phitplunger->m_plungeranim.m_mechTimeOut = 100;	
		}
		else
		{
			m_phitplunger->m_plungeranim.m_posdesired = m_d.m_v.y - m_d.m_stroke;
			m_phitplunger->m_plungeranim.m_speed = 0;//m_d.m_speedFire;
			m_phitplunger->m_plungeranim.m_force = -m_d.m_speedFire;

			if (m_phitplunger->m_plungeranim.m_mechTimeOut <= 0)
			{			
				m_phitplunger->m_plungeranim.m_fAcc = true;			
				m_phitplunger->m_plungeranim.m_mechTimeOut = 20;	// disable for 200 millisconds
			}
		}
	}

#ifdef LOG
	const int i = g_pplayer->m_vmover.IndexOf(m_phitplunger);
	fprintf(g_pplayer->m_flog, "Plunger Release %d\n", i);
#endif

	return S_OK;
}

STDMETHODIMP Plunger::get_PullSpeed(float *pVal)
{
	*pVal = m_d.m_speedPull;

	return S_OK;
}

STDMETHODIMP Plunger::put_PullSpeed(float newVal)
{
	STARTUNDO

	m_d.m_speedPull = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Plunger::get_FireSpeed(float *pVal)
{
	*pVal = m_d.m_speedFire;

	return S_OK;
}

STDMETHODIMP Plunger::put_FireSpeed(float newVal)
{
	STARTUNDO

	m_d.m_speedFire = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Plunger::get_Type(PlungerType *pVal)
{
	*pVal = m_d.m_type;

	return S_OK;
}

STDMETHODIMP Plunger::put_Type(PlungerType newVal)
{
	STARTUNDO

	m_d.m_type = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Plunger::get_Color(OLE_COLOR *pVal)
{
	*pVal = m_d.m_color;

	return S_OK;
}

STDMETHODIMP Plunger::put_Color(OLE_COLOR newVal)
{
	STARTUNDO

	m_d.m_color = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Plunger::get_Image(BSTR *pVal)
{
	WCHAR wz[512];

	MultiByteToWideChar(CP_ACP, 0, m_d.m_szImage, -1, wz, 32);
	*pVal = SysAllocString(wz);

	return S_OK;
}

STDMETHODIMP Plunger::put_Image(BSTR newVal)
{
	STARTUNDO

	WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szImage, 32, NULL, NULL);

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Plunger::CreateBall(IBall **pBallEx)
{
	if (m_phitplunger)
		{
		const float x = (m_phitplunger->m_plungeranim.m_x + m_phitplunger->m_plungeranim.m_x2) * 0.5f;
		const float y = m_phitplunger->m_plungeranim.m_pos - (25.0f + 0.01f); // radius 25

		const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, x, y);

		Ball * const pball = g_pplayer->CreateBall(x, y, height, 0.1f, 0, 0);
		
		*pBallEx = pball->m_pballex;
		pball->m_pballex->AddRef();
		}

	return S_OK;
}

STDMETHODIMP Plunger::get_X(float *pVal)
{
	*pVal = m_d.m_v.x;

	return S_OK;
}

STDMETHODIMP Plunger::put_X(float newVal)
{
	STARTUNDO

	m_d.m_v.x = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Plunger::get_Y(float *pVal)
{
	*pVal = m_d.m_v.y;

	return S_OK;
}

STDMETHODIMP Plunger::put_Y(float newVal)
{
	STARTUNDO

	m_d.m_v.y = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Plunger::get_Surface(BSTR *pVal)
{
	WCHAR wz[512];

	MultiByteToWideChar(CP_ACP, 0, m_d.m_szSurface, -1, wz, 32);
	*pVal = SysAllocString(wz);

	return S_OK;
}

STDMETHODIMP Plunger::put_Surface(BSTR newVal)
{
	STARTUNDO

	WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szSurface, 32, NULL, NULL);

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Plunger::get_MechStrength(float *pVal)
{
	*pVal = m_d.m_mechStrength;

	return S_OK;
}

STDMETHODIMP Plunger::put_MechStrength(float newVal)
{
	STARTUNDO
	m_d.m_mechStrength = newVal;
	STOPUNDO

	return S_OK;
}

STDMETHODIMP Plunger::get_MechPlunger(VARIANT_BOOL *pVal)
{
	*pVal = (VARIANT_BOOL)FTOVB(m_d.m_mechPlunger);

	return S_OK;
}

STDMETHODIMP Plunger::put_MechPlunger(VARIANT_BOOL newVal)
{
	STARTUNDO
	m_d.m_mechPlunger = VBTOF(newVal);
	STOPUNDO

	return S_OK;
}

STDMETHODIMP Plunger::get_AutoPlunger(VARIANT_BOOL *pVal)
{
	*pVal = (VARIANT_BOOL)FTOVB(m_d.m_autoPlunger);

	return S_OK;
}

STDMETHODIMP Plunger::put_AutoPlunger(VARIANT_BOOL newVal)
{
	STARTUNDO
	m_d.m_autoPlunger = VBTOF(newVal);
	STOPUNDO

	return S_OK;
}

STDMETHODIMP Plunger::get_Visible(VARIANT_BOOL *pVal)
{
	*pVal = (VARIANT_BOOL)FTOVB(m_d.m_fVisible);

	return S_OK;
}

STDMETHODIMP Plunger::put_Visible(VARIANT_BOOL newVal)
{
	STARTUNDO

	m_d.m_fVisible = VBTOF(newVal);

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Plunger::get_ParkPosition(float *pVal)
{
	*pVal = m_d.m_parkPosition;
	return S_OK;
}

STDMETHODIMP Plunger::put_ParkPosition(float newVal)
{
	STARTUNDO
	m_d.m_parkPosition = newVal;
	STOPUNDO

	return S_OK;
}

STDMETHODIMP Plunger::get_Stroke(float *pVal)
{
	*pVal = m_d.m_stroke;

	return S_OK;
}

STDMETHODIMP Plunger::put_Stroke(float newVal)
{
	STARTUNDO

	if(newVal < 16.5f) newVal = 16.5f;
	m_d.m_stroke = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Plunger::get_ScatterVelocity(float *pVal)
{
	*pVal = m_d.m_scatterVelocity;

	return S_OK;
}

STDMETHODIMP Plunger::put_ScatterVelocity(float newVal)
{
	STARTUNDO
	m_d.m_scatterVelocity = newVal;
	STOPUNDO

	return S_OK;
}

STDMETHODIMP Plunger::get_BreakOverVelocity(float *pVal)
{
	*pVal = m_d.m_breakOverVelocity;

	return S_OK;
}

STDMETHODIMP Plunger::put_BreakOverVelocity(float newVal)
{
	STARTUNDO
	m_d.m_breakOverVelocity = newVal;
	STOPUNDO

	return S_OK;
}

void Plunger::GetDialogPanes(Vector<PropertyPane> *pvproppane)
	{
	PropertyPane *pproppane;

	pproppane = new PropertyPane(IDD_PROP_NAME, NULL);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROPPLUNGER_VISUALS, IDS_VISUALS);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROPLIGHT_POSITION, IDS_POSITION);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROPPLUNGER_PHYSICS, IDS_STATE);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROP_TIMER, IDS_MISC);
	pvproppane->AddElement(pproppane);
	}
