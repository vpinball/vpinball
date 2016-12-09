#include "stdafx.h"

DispReel::DispReel()
{
   memset(m_d.m_szImage, 0, MAXTOKEN);
   memset(m_d.m_szSound, 0, MAXTOKEN);
}

DispReel::~DispReel()
{
}

// This function is called when ever a new instance of this object is created
// (along with the constructor (above))
//
HRESULT DispReel::Init(PinTable *ptable, float x, float y, bool fromMouseClick)
{
   m_ptable = ptable;

   SetDefaults(fromMouseClick);

   m_d.m_v1.x = x;
   m_d.m_v1.y = y;
   m_d.m_v2.x = x + getBoxWidth();
   m_d.m_v2.y = y + getBoxHeight();

   //m_preelframe = NULL;

   return InitVBA(fTrue, 0, NULL);
}

// set the defaults for the objects persistent data (m_d.*) in case this
// is a new instance of this object or there is a backwards compatability
// issue (old version of object doesn't contain all the needed fields)
//
void DispReel::SetDefaults(bool fromMouseClick)
{
   // object is only available on the backglass
   m_fBackglass = true;

   // set all the Data defaults
   HRESULT hr;
   float fTmp;
   int iTmp;

   hr = GetRegString("DefaultProps\\Ramp", "Image", m_d.m_szImage, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szImage[0] = 0;

   hr = GetRegString("DefaultProps\\Ramp", "Sound", m_d.m_szSound, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szSound[0] = 0;

   hr = GetRegInt("DefaultProps\\EMReel", "UseImageGrid", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fUseImageGrid = iTmp == 0 ? false : true;
   else
      m_d.m_fUseImageGrid = false;

   hr = GetRegInt("DefaultProps\\EMReel", "Visible", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fVisible = iTmp == 0 ? false : true;
   else
      m_d.m_fVisible = true;

   hr = GetRegInt("DefaultProps\\EMReel", "ImagesPerRow", &iTmp);
   m_d.m_imagesPerGridRow = (hr == S_OK) && fromMouseClick ? iTmp : 1;

   hr = GetRegInt("DefaultProps\\EMReel", "Transparent", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fTransparent = iTmp == 0 ? false : true;
   else
      m_d.m_fTransparent = false;

   hr = GetRegInt("DefaultProps\\EMReel", "ReelCount", &iTmp);
   m_d.m_reelcount = (hr == S_OK) && fromMouseClick ? iTmp : 5;

   hr = GetRegStringAsFloat("DefaultProps\\EMReel", "Width", &fTmp);
   m_d.m_width = (hr == S_OK) && fromMouseClick ? fTmp : 30.0f;

   hr = GetRegStringAsFloat("DefaultProps\\EMReel", "Height", &fTmp);
   m_d.m_height = (hr == S_OK) && fromMouseClick ? fTmp : 40.0f;

   hr = GetRegStringAsFloat("DefaultProps\\EMReel", "ReelSpacing", &fTmp);
   m_d.m_reelspacing = (hr == S_OK) && fromMouseClick ? fTmp : 4.0f;

   hr = GetRegStringAsFloat("DefaultProps\\EMReel", "MotorSteps", &fTmp);
   m_d.m_motorsteps = (hr == S_OK) && fromMouseClick ? fTmp : 2.0f;

   hr = GetRegInt("DefaultProps\\EMReel", "DigitRange", &iTmp);
   m_d.m_digitrange = (hr == S_OK) && fromMouseClick ? iTmp : 9;

   hr = GetRegInt("DefaultProps\\EMReel", "UpdateInterval", &iTmp);
   m_d.m_updateinterval = (hr == S_OK) && fromMouseClick ? iTmp : 50;

   hr = GetRegInt("DefaultProps\\EMReel", "BackColor", &iTmp);
   m_d.m_backcolor = (hr == S_OK) && fromMouseClick ? iTmp : RGB(64, 64, 64);

   hr = GetRegInt("DefaultProps\\EMReel", "TimerEnabled", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_tdr.m_fTimerEnabled = iTmp == 0 ? false : true;
   else
      m_d.m_tdr.m_fTimerEnabled = 0;

   hr = GetRegInt("DefaultProps\\EMReel", "TimerInterval", &iTmp);
   m_d.m_tdr.m_TimerInterval = (hr == S_OK) && fromMouseClick ? iTmp : 100;
}

void DispReel::WriteRegDefaults()
{
   SetRegValue("DefaultProps\\EMReel", "Image", REG_SZ, &m_d.m_szImage, lstrlen(m_d.m_szImage));
   SetRegValue("DefaultProps\\EMReel", "Sound", REG_SZ, &m_d.m_szSound, lstrlen(m_d.m_szSound));
   SetRegValueBool("DefaultProps\\Decal", "UseImageGrid", m_d.m_fUseImageGrid);
   SetRegValueBool("DefaultProps\\Decal", "Visible", m_d.m_fVisible);
   SetRegValueInt("DefaultProps\\Decal", "ImagesPerRow", m_d.m_imagesPerGridRow);
   SetRegValueBool("DefaultProps\\Decal", "Transparent", m_d.m_fTransparent);
   SetRegValueInt("DefaultProps\\Decal", "ReelCount", m_d.m_reelcount);
   SetRegValueFloat("DefaultProps\\EMReel", "Width", m_d.m_width);
   SetRegValueFloat("DefaultProps\\EMReel", "Height", m_d.m_height);
   SetRegValueFloat("DefaultProps\\EMReel", "ReelSpacing", m_d.m_reelspacing);
   SetRegValueFloat("DefaultProps\\EMReel", "MotorSteps", m_d.m_motorsteps);
   SetRegValueInt("DefaultProps\\Decal", "DigitRange", m_d.m_digitrange);
   SetRegValueInt("DefaultProps\\Decal", "UpdateInterval", m_d.m_updateinterval);
   SetRegValue("DefaultProps\\EMReel", "BackColor", REG_DWORD, &m_d.m_backcolor, 4);
   SetRegValueBool("DefaultProps\\EMReel", "TimerEnabled", m_d.m_tdr.m_fTimerEnabled);
   SetRegValueInt("DefaultProps\\EMReel", "TimerInterval", m_d.m_tdr.m_TimerInterval);
}

STDMETHODIMP DispReel::InterfaceSupportsErrorInfo(REFIID riid)
{
   static const IID* arr[] =
   {
      &IID_IDispReel,
   };

   for (size_t i = 0; i < sizeof(arr) / sizeof(arr[0]); ++i)
   {
      if (InlineIsEqualGUID(*arr[i], riid))
         return S_OK;
   }
   return S_FALSE;
}

// this function draws the shape of the object with a solid fill
// only used in the editor and not the game
//
// this is called before the grid lines are drawn on the map
//
void DispReel::PreRender(Sur * const psur)
{
   psur->SetBorderColor(-1, false, 0);
   psur->SetFillColor(m_d.m_backcolor);
   psur->SetObject(this);

   // draw background box
   psur->Rectangle(m_d.m_v1.x, m_d.m_v1.y, m_d.m_v2.x, m_d.m_v2.y);

   // draw n reels in the box (in blue)
   psur->SetFillColor(RGB(0, 0, 255));
   for (int i = 0; i < m_d.m_reelcount; ++i)
   {
      // set up top corner point
      const float fi = (float)i;
      const float x = m_d.m_v1.x + fi*(m_d.m_width + m_d.m_reelspacing) + m_d.m_reelspacing;
      const float y = m_d.m_v1.y + m_d.m_reelspacing;
      const float x2 = x + m_d.m_width;
      const float y2 = y + m_d.m_height;

      // set up points (clockwise)
      const Vertex2D rgv[4] = { Vertex2D(x, y), Vertex2D(x2, y), Vertex2D(x2, y2), Vertex2D(x, y2) };
      psur->Polygon(rgv, 4);
   }
}

// this function draws the shape of the object with a black outline (no solid fill)
// only used in the editor and not the game
//
// this is called after the grid lines have been drawn on the map.  draws a solid
// outline over the grid lines
//
void DispReel::Render(Sur * const psur)
{
   if (!GetPTable()->GetEMReelsEnabled()) return;

   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetFillColor(-1);
   psur->SetObject(this);
   psur->SetObject(NULL);

   // draw background box
   psur->Rectangle(m_d.m_v1.x, m_d.m_v1.y, m_d.m_v2.x, m_d.m_v2.y);

   // draw n reels in the box
   for (int i = 0; i < m_d.m_reelcount; ++i)
   {
      // set up top corner point
      const float fi = (float)i;
      const float x = m_d.m_v1.x + fi*(m_d.m_width + m_d.m_reelspacing) + m_d.m_reelspacing;
      const float y = m_d.m_v1.y + m_d.m_reelspacing;
      const float x2 = x + m_d.m_width;
      const float y2 = y + m_d.m_height;

      // set up points (clockwise)
      const Vertex2D rgv[4] = { Vertex2D(x, y), Vertex2D(x2, y), Vertex2D(x2, y2), Vertex2D(x, y2) };
      psur->Polygon(rgv, 4);
   }
}


// Registers the timer with the game call which then makes a call back when the interval
// has expired.
//
// for this sort of object (reel driver) it is basically not really required but hey, somebody
// might use it..
//
void DispReel::GetTimers(Vector<HitTimer> * const pvht)
{
   IEditable::BeginPlay();

   HitTimer * const pht = new HitTimer();
   pht->m_interval = m_d.m_tdr.m_TimerInterval >= 0 ? max(m_d.m_tdr.m_TimerInterval, MAX_TIMER_MSEC_INTERVAL) : -1;
   pht->m_nextfire = pht->m_interval;
   pht->m_pfe = (IFireEvents *)this;

   m_phittimer = pht;

   if (m_d.m_tdr.m_fTimerEnabled)
   {
      pvht->AddElement(pht);
   }
}


// This function is supposed to return the hit shapes for the object but since it is
// off screen we use it to register the screen updater in the game engine.. this means
// that Check3d (and Draw3d) are called in the updater class.
//
void DispReel::GetHitShapes(Vector<HitObject> * const pvho)
{
   m_dispreelanim.m_pDispReel = this;

   // HACK - adding object directly to screen update list.  Someday make hit objects and screenupdaters seperate objects
   g_pplayer->m_vanimate.AddElement(&m_dispreelanim);
}

void DispReel::GetHitShapesDebug(Vector<HitObject> * const pvho)
{
}

// This method is called as the game exits..
// it cleans up any allocated memory used by the instance of the object
//
void DispReel::EndPlay()
{
   IEditable::EndPlay();
}

void DispReel::PostRenderStatic(RenderDevice* pd3dDevice)
{
   TRACE_FUNCTION();

   if (!m_d.m_fVisible || !GetPTable()->GetEMReelsEnabled())
      return;

   // get a pointer to the image specified in the object
   Texture * const pin = m_ptable->GetImage(m_d.m_szImage); // pointer to image information from the image manager

   if (!pin)
      return;

   if (g_pplayer->m_ptable->m_tblMirrorEnabled^g_pplayer->m_ptable->m_fReflectionEnabled)
      pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_NONE);
   else
      pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);

   pd3dDevice->SetRenderState(RenderDevice::DEPTHBIAS, 0);
   pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);

   g_pplayer->m_pin3d.EnableAlphaTestReference(0xE0); //!!
   pd3dDevice->SetRenderState(RenderDevice::ALPHAFUNC, D3DCMP_GREATER); //!! still necessary?
   g_pplayer->m_pin3d.EnableAlphaBlend(false);

   pd3dDevice->DMDShader->SetTechnique("basic_noDMD");

   const D3DXVECTOR4 c = convertColor(0xFFFFFFFF, 1.f);
   pd3dDevice->DMDShader->SetVector("vColor_Intensity", &c);

   pd3dDevice->DMDShader->SetTexture("Texture0", pin);
   
   pd3dDevice->DMDShader->Begin(0);
   for (int r = 0; r < m_d.m_reelcount; ++r) //!! optimize by doing all in one
   {
       const float posx = ReelInfo[r].position.left;
       const float posy = ReelInfo[r].position.top;
       const float width = ReelInfo[r].position.right;
       const float height = ReelInfo[r].position.bottom;
       const float u0 = m_digitTexCoords[ReelInfo[r].currentValue].u_min;
       const float v0 = m_digitTexCoords[ReelInfo[r].currentValue].v_min;
       const float u1 = m_digitTexCoords[ReelInfo[r].currentValue].u_max;
       const float v1 = m_digitTexCoords[ReelInfo[r].currentValue].v_max;

       float Verts[4 * 5] =
       {
           1.0f, 1.0f, 0.0f, u1, v1,
           0.0f, 1.0f, 0.0f, u0, v1,
           1.0f, 0.0f, 0.0f, u1, v0,
           0.0f, 0.0f, 0.0f, u0, v0
       };

       for (unsigned int i = 0; i < 4; ++i)
       {
           Verts[i * 5] = (Verts[i * 5] * width + posx)*2.0f - 1.0f;
           Verts[i * 5 + 1] = 1.0f - (Verts[i * 5 + 1] * height + posy)*2.0f;
       }

       pd3dDevice->DrawTexturedQuad((Vertex3D_TexelOnly*)Verts);
   }
   pd3dDevice->DMDShader->End();

   //g_pplayer->m_pin3d.DisableAlphaBlend(); //!! not necessary anymore
   pd3dDevice->SetRenderState(RenderDevice::ALPHATESTENABLE, FALSE);

   //if(g_pplayer->m_ptable->m_tblMirrorEnabled^g_pplayer->m_ptable->m_fReflectionEnabled)
   //	pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);
}

void DispReel::RenderSetup(RenderDevice* pd3dDevice)
{
   Pin3D * const ppin3d = &g_pplayer->m_pin3d;

   // get the render sizes of the objects (reels and frame)
   m_renderwidth = max(0.0f, m_d.m_width / (float)EDITOR_BG_WIDTH);
   m_renderheight = max(0.0f, m_d.m_height / (float)EDITOR_BG_HEIGHT);
   const float m_renderspacingx = max(0.0f, m_d.m_reelspacing / (float)EDITOR_BG_WIDTH);
   const float m_renderspacingy = max(0.0f, m_d.m_reelspacing / (float)EDITOR_BG_HEIGHT);

   // set up all the reel positions within the object frame
   const float x0 = m_d.m_v1.x / (float)EDITOR_BG_WIDTH;
   const float y0 = m_d.m_v1.y / (float)EDITOR_BG_HEIGHT;
   float x1 = x0 + m_renderspacingx;

   for (int i = 0; i < m_d.m_reelcount; ++i)
   {
      ReelInfo[i].position.left = x1;
      ReelInfo[i].position.right = m_renderwidth;
      ReelInfo[i].position.top = y0 + m_renderspacingy;
      ReelInfo[i].position.bottom = m_renderheight;

      ReelInfo[i].currentValue = 0;
      ReelInfo[i].motorPulses = 0;
      ReelInfo[i].motorStepCount = 0;
      ReelInfo[i].motorCalcStep = 0;
      ReelInfo[i].motorOffset = 0;

      // move to the next reel
      x1 += m_renderspacingx + m_renderwidth;
   }

   // get a pointer to the image specified in the object
   Texture * const pin = m_ptable->GetImage(m_d.m_szImage); // pointer to image information from the image manager

   if (!pin)
      return;

   int	GridCols, GridRows;

   // get the number of images per row of the image
   if (m_d.m_fUseImageGrid)
   {
      GridCols = m_d.m_imagesPerGridRow;
      if (GridCols != 0) // best to be safe
      {
         GridRows = (m_d.m_digitrange + 1) / GridCols;
         if ((GridRows * GridCols) < (m_d.m_digitrange + 1))
            ++GridRows;
      }
      else
         GridRows = 1;
   }
   else
   {
      GridCols = m_d.m_digitrange + 1;
      GridRows = 1;
   }

   // save the color to use in any transparent blitting
   //!! m_rgbImageTransparent = pin->m_rgbTransparent;
   if (GridCols != 0 && GridRows != 0)
   {
      // get the size of the individual reel digits (if m_digitrange is wrong we can forget the rest)
      m_reeldigitwidth = (float)pin->m_width / (float)GridCols;
      m_reeldigitheight = (float)pin->m_height / (float)GridRows;
   }
   else
      ShowError("DispReel: GridCols/GridRows are zero!");

   const float ratiox = (float)m_reeldigitwidth / (float)pin->m_width;
   const float ratioy = (float)m_reeldigitheight / (float)pin->m_height;

   int gr = 0;
   int gc = 0;

   m_digitTexCoords.resize(m_d.m_digitrange + 1);

   for (int i = 0; i <= m_d.m_digitrange; ++i)
   {
      m_digitTexCoords[i].u_min = (float)gc * ratiox;
      m_digitTexCoords[i].v_min = (float)gr * ratioy;
      m_digitTexCoords[i].u_max = m_digitTexCoords[i].u_min + ratiox;
      m_digitTexCoords[i].v_max = m_digitTexCoords[i].v_min + ratioy;

      ++gc;
      if (gc >= GridCols)
      {
         gc = 0;
         ++gr;
      }

      if (i == m_d.m_digitrange)
      {
         // Go back and draw the first picture at the end of the strip
         gc = 0;
         gr = 0;
      }
   }

   m_timenextupdate = g_pplayer->m_time_msec + m_d.m_updateinterval;
}

void DispReel::RenderStatic(RenderDevice* pd3dDevice)
{
}

// This function is called during Animate().  It basically check to see if the update
// interval has expired and if so handles the rolling of the reels according to the
// number of motor steps queued up for each reel
//
// if a screen update is required it returns true..
//
void DispReel::Animate()
{
   OLECHAR mySound[256];

   if (g_pplayer->m_time_msec >= m_timenextupdate)
   {
      m_timenextupdate = g_pplayer->m_time_msec + m_d.m_updateinterval;

      // work out the roll over values
      const int OverflowValue = m_d.m_digitrange;
      const int AdjustValue = OverflowValue + 1;

      const float step = (float)m_reeldigitheight / m_d.m_motorsteps;

      // start at the last reel and work forwards (right to left)
      for (int i = m_d.m_reelcount - 1; i >= 0; i--)
      {
         // if the motor has stoped, and there are still motor steps then start another one
         if ((ReelInfo[i].motorPulses != 0) && (ReelInfo[i].motorStepCount == 0))
         {
            // get the number of steps (or increments) needed to move the reel
            ReelInfo[i].motorStepCount = (int)m_d.m_motorsteps;
            ReelInfo[i].motorCalcStep = (ReelInfo[i].motorPulses > 0) ? step : -step;
            ReelInfo[i].motorOffset = 0;

            // play the sound (if any) for each click of the reel
            if (m_d.m_szSound[0] != 0)
            {
               MultiByteToWideChar(CP_ACP, 0, m_d.m_szSound, -1, mySound, 32);
               m_ptable->PlaySound(mySound, 0, 1.0f, 0.f, 0.f, 0, VARIANT_FALSE, VARIANT_TRUE);
            }
         }

         // is the reel in the process of moving??
         if (ReelInfo[i].motorStepCount != 0)
         {
            ReelInfo[i].motorOffset += ReelInfo[i].motorCalcStep;
            ReelInfo[i].motorStepCount--;
            // have re reached the end of the step
            if (ReelInfo[i].motorStepCount <= 0)
            {
               ReelInfo[i].motorStepCount = 0;      // best to be safe (paranoid)
               ReelInfo[i].motorOffset = 0;

               if (ReelInfo[i].motorPulses < 0)
               {
                  ReelInfo[i].motorPulses++;
                  ReelInfo[i].currentValue--;
                  if (ReelInfo[i].currentValue < 0)
                  {
                     ReelInfo[i].currentValue += AdjustValue;
                     // if not the first reel then decrement the next reel by 1
                     if (i != 0)
                        ReelInfo[i - 1].motorPulses--;
                  }
               }
               else
               {
                  ReelInfo[i].motorPulses--;
                  ReelInfo[i].currentValue++;
                  if (ReelInfo[i].currentValue > OverflowValue)
                  {
                     ReelInfo[i].currentValue -= AdjustValue;
                     // if not the first reel then increment the next reel
                     // along by 1 (just like a car odometer)
                     if (i != 0)
                        ReelInfo[i - 1].motorPulses++;
                  }
               }
            }
         }
      }
   }
}

void DispReel::SetObjectPos()
{
   g_pvp->SetObjectPosCur(m_d.m_v1.x, m_d.m_v1.y);
}


void DispReel::MoveOffset(const float dx, const float dy)
{
   m_d.m_v1.x += dx;
   m_d.m_v1.y += dy;

   m_d.m_v2.x += dx;
   m_d.m_v2.y += dy;

   m_ptable->SetDirtyDraw();
}


void DispReel::GetCenter(Vertex2D * const pv) const
{
   *pv = m_d.m_v1;
}


void DispReel::PutCenter(const Vertex2D * const pv)
{
   m_d.m_v1 = *pv;

   m_d.m_v2.x = pv->x + getBoxWidth();
   m_d.m_v2.y = pv->y + getBoxHeight();

   m_ptable->SetDirtyDraw();
}


HRESULT DispReel::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   BiffWriter bw(pstm, hcrypthash, hcryptkey);

   bw.WriteStruct(FID(VER1), &m_d.m_v1, sizeof(Vertex2D));
   bw.WriteStruct(FID(VER2), &m_d.m_v2, sizeof(Vertex2D));
   bw.WriteInt(FID(CLRB), m_d.m_backcolor);
   bw.WriteBool(FID(TMON), m_d.m_tdr.m_fTimerEnabled);
   bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
   bw.WriteBool(FID(TRNS), m_d.m_fTransparent);
   bw.WriteString(FID(IMAG), m_d.m_szImage);
   bw.WriteString(FID(SOUN), m_d.m_szSound);
   bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);
   bw.WriteFloat(FID(WDTH), m_d.m_width);
   bw.WriteFloat(FID(HIGH), m_d.m_height);
   const float reel = (float)m_d.m_reelcount;
   bw.WriteFloat(FID(RCNT), reel);
   bw.WriteFloat(FID(RSPC), m_d.m_reelspacing);
   bw.WriteFloat(FID(MSTP), m_d.m_motorsteps);
   const float dig = (float)m_d.m_digitrange;
   bw.WriteFloat(FID(RANG), dig);
   bw.WriteInt(FID(UPTM), m_d.m_updateinterval);
   bw.WriteBool(FID(UGRD), m_d.m_fUseImageGrid);
   bw.WriteBool(FID(VISI), m_d.m_fVisible);
   bw.WriteInt(FID(GIPR), m_d.m_imagesPerGridRow);

   ISelect::SaveData(pstm, hcrypthash, hcryptkey); //add BDS2

   bw.WriteTag(FID(ENDB));

   return S_OK;
}


HRESULT DispReel::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   SetDefaults(false);

   BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);

   m_ptable = ptable;

   br.Load();
   return S_OK;
}


BOOL DispReel::LoadToken(int id, BiffReader *pbr)
{
   if (id == FID(PIID))
   {
      pbr->GetInt((int *)pbr->m_pdata);
   }
   else if (id == FID(VER1))
   {
      pbr->GetStruct(&m_d.m_v1, sizeof(Vertex2D));
   }
   else if (id == FID(VER2))
   {
      pbr->GetStruct(&m_d.m_v2, sizeof(Vertex2D));
   }
   else if (id == FID(WDTH))
   {
      pbr->GetFloat(&m_d.m_width);
   }
   else if (id == FID(HIGH))
   {
      pbr->GetFloat(&m_d.m_height);
   }
   else if (id == FID(CLRB))
   {
      pbr->GetInt(&m_d.m_backcolor);
   }
   else if (id == FID(TMON))
   {
      pbr->GetBool(&m_d.m_tdr.m_fTimerEnabled);
   }
   else if (id == FID(TMIN))
   {
      pbr->GetInt(&m_d.m_tdr.m_TimerInterval);
   }
   else if (id == FID(NAME))
   {
      pbr->GetWideString((WCHAR *)m_wzName);
   }
   else if (id == FID(TRNS))
   {
      pbr->GetBool(&m_d.m_fTransparent);
   }
   else if (id == FID(IMAG))
   {
      pbr->GetString(m_d.m_szImage);
   }
   else if (id == FID(RCNT))
   {
      float reel;
      pbr->GetFloat(&reel);
      m_d.m_reelcount = (int)reel;
   }
   else if (id == FID(RSPC))
   {
      pbr->GetFloat(&m_d.m_reelspacing);
   }
   else if (id == FID(MSTP))
   {
      pbr->GetFloat(&m_d.m_motorsteps);
   }
   else if (id == FID(SOUN))
   {
      pbr->GetString(m_d.m_szSound);
   }
   else if (id == FID(UGRD))
   {
      pbr->GetBool(&m_d.m_fUseImageGrid);
   }
   else if (id == FID(VISI))
   {
      pbr->GetBool(&m_d.m_fVisible);
   }
   else if (id == FID(GIPR))
   {
      pbr->GetInt(&m_d.m_imagesPerGridRow);
   }
   else if (id == FID(RANG))
   {
      float dig;
      pbr->GetFloat(&dig);
      m_d.m_digitrange = (int)dig;
   }
   else if (id == FID(UPTM))
   {
      pbr->GetInt(&m_d.m_updateinterval);
   }
   else if (id == FID(FONT)) //!! deprecated, only here to support loading of old tables
   {
      IFont *pIFont;
      FONTDESC fd;
      fd.cbSizeofstruct = sizeof(FONTDESC);
      fd.lpstrName = L"Times New Roman";
      fd.cySize.int64 = 260000;
      //fd.cySize.Lo = 0;
      fd.sWeight = FW_BOLD;
      fd.sCharset = 0;
      fd.fItalic = 0;
      fd.fUnderline = 0;
      fd.fStrikethrough = 0;
      OleCreateFontIndirect(&fd, IID_IFont, (void **)&pIFont);

      IPersistStream * ips;
      pIFont->QueryInterface(IID_IPersistStream, (void **)&ips);

      ips->Load(pbr->m_pistream);

      pIFont->Release();
   }
   else
   {
      ISelect::LoadToken(id, pbr);
   }
   return fTrue;
}

HRESULT DispReel::InitPostLoad()
{
   return S_OK;
}

void DispReel::GetDialogPanes(Vector<PropertyPane> *pvproppane)
{
   PropertyPane *pproppane;

   pproppane = new PropertyPane(IDD_PROP_NAME, NULL);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPDISPREEL_VISUALS, IDS_VISUALS);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPDISPREEL_POSITION, IDS_POSITION);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPDISPREEL_STATE, IDS_STATE);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROP_TIMER, IDS_MISC);
   pvproppane->AddElement(pproppane);
}

// These methods provide the interface to the object through both the editor
// and the script for a of the object properties
//
STDMETHODIMP DispReel::get_BackColor(OLE_COLOR *pVal)
{
   *pVal = m_d.m_backcolor;

   return S_OK;
}

STDMETHODIMP DispReel::put_BackColor(OLE_COLOR newVal)
{
   STARTUNDO
      m_d.m_backcolor = newVal;
   STOPUNDO

      return S_OK;
}

STDMETHODIMP DispReel::get_Reels(float *pVal)
{
   *pVal = (float)m_d.m_reelcount;

   return S_OK;
}

STDMETHODIMP DispReel::put_Reels(float newVal)
{
   STARTUNDO

      m_d.m_reelcount = min(max(1, (int)newVal), MAX_REELS); // must have at least 1 reel and a max of MAX_REELS
   m_d.m_v2.x = m_d.m_v1.x + getBoxWidth();
   m_d.m_v2.y = m_d.m_v1.y + getBoxHeight();

   STOPUNDO

      return S_OK;
}

STDMETHODIMP DispReel::get_Width(float *pVal)
{
   *pVal = m_d.m_width;

   return S_OK;
}

STDMETHODIMP DispReel::put_Width(float newVal)
{
   STARTUNDO

      m_d.m_width = max(0.0f, newVal);
   m_d.m_v2.x = m_d.m_v1.x + getBoxWidth();

   STOPUNDO

      return S_OK;
}

STDMETHODIMP DispReel::get_Height(float *pVal)
{
   *pVal = m_d.m_height;

   return S_OK;
}

STDMETHODIMP DispReel::put_Height(float newVal)
{
   STARTUNDO

      m_d.m_height = max(0.0f, newVal);
   m_d.m_v2.y = m_d.m_v1.y + getBoxHeight();

   STOPUNDO

      return S_OK;
}

STDMETHODIMP DispReel::get_X(float *pVal)
{
   *pVal = m_d.m_v1.x;

   return S_OK;
}

STDMETHODIMP DispReel::put_X(float newVal)
{
   STARTUNDO

      const float delta = newVal - m_d.m_v1.x;
   m_d.m_v1.x += delta;
   m_d.m_v2.x = m_d.m_v1.x + getBoxWidth();

   STOPUNDO

      return S_OK;
}

STDMETHODIMP DispReel::get_Y(float *pVal)
{
   *pVal = m_d.m_v1.y;

   return S_OK;
}

STDMETHODIMP DispReel::put_Y(float newVal)
{
   STARTUNDO

      const float delta = newVal - m_d.m_v1.y;
   m_d.m_v1.y += delta;
   m_d.m_v2.y = m_d.m_v1.y + getBoxHeight();

   STOPUNDO

      return S_OK;
}

STDMETHODIMP DispReel::get_IsTransparent(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fTransparent);

   return S_OK;
}

STDMETHODIMP DispReel::put_IsTransparent(VARIANT_BOOL newVal)
{
   STARTUNDO
      m_d.m_fTransparent = VBTOF(newVal);
   STOPUNDO

      return S_OK;
}

STDMETHODIMP DispReel::get_Image(BSTR *pVal)
{
   OLECHAR wz[512];
   MultiByteToWideChar(CP_ACP, 0, m_d.m_szImage, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP DispReel::put_Image(BSTR newVal)
{
   char szImage[MAXTOKEN];
   WideCharToMultiByte(CP_ACP, 0, newVal, -1, szImage, 32, NULL, NULL);
   const Texture * const tex = m_ptable->GetImage(szImage);
   if(tex && tex->IsHDR())
   {
       ShowError("Cannot use a HDR image (.exr/.hdr) here");
       return E_FAIL;
   }

   STARTUNDO

   strcpy_s(m_d.m_szImage,szImage);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP DispReel::get_Spacing(float *pVal)
{
   *pVal = m_d.m_reelspacing;
   return S_OK;
}

STDMETHODIMP DispReel::put_Spacing(float newVal)
{
   STARTUNDO

      m_d.m_reelspacing = max(0.0f, newVal);
   m_d.m_v2.x = m_d.m_v1.x + getBoxWidth();
   m_d.m_v2.y = m_d.m_v1.y + getBoxHeight();

   STOPUNDO

      return S_OK;
}

STDMETHODIMP DispReel::get_Sound(BSTR *pVal)
{
   OLECHAR wz[512];
   MultiByteToWideChar(CP_ACP, 0, m_d.m_szSound, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP DispReel::put_Sound(BSTR newVal)
{
   STARTUNDO
      WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szSound, 32, NULL, NULL);
   STOPUNDO

      return S_OK;
}

STDMETHODIMP DispReel::get_Steps(float *pVal)
{
   *pVal = floorf(m_d.m_motorsteps);

   return S_OK;
}

STDMETHODIMP DispReel::put_Steps(float newVal)
{
   STARTUNDO
      m_d.m_motorsteps = max(1.0f, floorf(newVal));	// must have at least 1 step
   STOPUNDO

      return S_OK;
}

STDMETHODIMP DispReel::get_Range(float *pVal)
{
   *pVal = (float)m_d.m_digitrange;

   return S_OK;
}

STDMETHODIMP DispReel::put_Range(float newVal)
{
   STARTUNDO

      m_d.m_digitrange = (int)max(0.0f, floorf(newVal));        // must have at least 1 digit (0 is a digit)
   if (m_d.m_digitrange > 512 - 1) m_d.m_digitrange = 512 - 1;  // and a max of 512 (0->511) //!! 512 requested by highrise

   STOPUNDO

      return S_OK;
}

STDMETHODIMP DispReel::get_UpdateInterval(long *pVal)
{
   *pVal = m_d.m_updateinterval;

   return S_OK;
}

STDMETHODIMP DispReel::put_UpdateInterval(long newVal)
{
   STARTUNDO

      m_d.m_updateinterval = max(5, newVal);
   if (g_pplayer)
      m_timenextupdate = g_pplayer->m_time_msec + m_d.m_updateinterval;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP DispReel::get_UseImageGrid(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fUseImageGrid);

   return S_OK;
}

STDMETHODIMP DispReel::put_UseImageGrid(VARIANT_BOOL newVal)
{
   STARTUNDO
      m_d.m_fUseImageGrid = VBTOF(newVal);
   STOPUNDO

      return S_OK;
}

STDMETHODIMP DispReel::get_Visible(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fVisible);

   return S_OK;
}

STDMETHODIMP DispReel::put_Visible(VARIANT_BOOL newVal)
{
   STARTUNDO
      m_d.m_fVisible = VBTOF(newVal);
   STOPUNDO

      return S_OK;
}

STDMETHODIMP DispReel::get_ImagesPerGridRow(long *pVal)
{
   *pVal = m_d.m_imagesPerGridRow;

   return S_OK;
}

STDMETHODIMP DispReel::put_ImagesPerGridRow(long newVal)
{
   STARTUNDO
   m_d.m_imagesPerGridRow = max(1, newVal);
   STOPUNDO

   return S_OK;
}


// function Methods available for the scripters.
//
STDMETHODIMP DispReel::AddValue(long Value)
{
   const bool bNegative = (Value < 0);

   // ensure a positive number
   long val = labs(Value);

   // get the base of this reel
   const long valbase = m_d.m_digitrange + 1;

   // start at the right most reel and move left
   int i = m_d.m_reelcount - 1;
   while ((val != 0) && (i >= 0))
   {
      const int digitValue = val % valbase;
      // remove the value for this reel from the overall number
      val /= valbase;

      if (bNegative)
         ReelInfo[i].motorPulses -= digitValue;
      else
         ReelInfo[i].motorPulses += digitValue;

      // move to next reel
      i--;
   }

   return S_OK;
}


STDMETHODIMP DispReel::SetValue(long Value)
{
   // ensure a positive number
   long val = labs(Value);

   // get the base of this reel
   const long valbase = m_d.m_digitrange + 1;

   // reset the motor
   for (int l = 0; l < m_d.m_reelcount; ++l)
   {
      ReelInfo[l].currentValue = 0;
      ReelInfo[l].motorPulses = 0;
      ReelInfo[l].motorStepCount = 0;
      ReelInfo[l].motorCalcStep = 0;
      ReelInfo[l].motorOffset = 0;
   }

   // set the reel values (startint at the right most reel and move left)
   int i = m_d.m_reelcount - 1;
   while ((val != 0) && (i >= 0))
   {
      const int digitValue = val % valbase;
      // remove the value for this reel from the overall number
      val /= valbase;
      ReelInfo[i].currentValue = digitValue;
      // move to next reel
      i--;
   }

   // force a immediate screen update
   m_timenextupdate = g_pplayer->m_time_msec;

   return S_OK;
}


STDMETHODIMP DispReel::ResetToZero()
{
   int carry = 0;
   const int overflowValue = m_d.m_digitrange + 1;

   // work for the last reel to the first one
   for (int i = m_d.m_reelcount - 1; i >= 0; i--)
   {
      const int adjust = overflowValue - carry - ReelInfo[i].currentValue;
      carry = 0;

      if (adjust != overflowValue)
      {
         // overwrite the pulse count with the adjust value
         ReelInfo[i].motorPulses = adjust;
         // as this reel returns to zero it will roll over the next reel along
         carry = 1;
      }
   }

   return S_OK;
}


STDMETHODIMP DispReel::SpinReel(long ReelNumber, long PulseCount)
{
   if ((ReelNumber >= 1) && (ReelNumber <= m_d.m_reelcount))
   {
      const int reel = ReelNumber - 1;
      ReelInfo[reel].motorPulses += PulseCount;
      return S_OK;
   }
   else
      return E_FAIL;
}


// Private functions
//
float DispReel::getBoxWidth() const
{
   const float width = (float)m_d.m_reelcount * m_d.m_width
      + (float)m_d.m_reelcount * m_d.m_reelspacing
      + m_d.m_reelspacing;	// spacing also includes edges
   return width;
}


float DispReel::getBoxHeight() const
{
   const float height = m_d.m_height
      + m_d.m_reelspacing + m_d.m_reelspacing; // spacing also includes edges

   return height;
}
