#include "stdafx.h"
#include "renderer/Shader.h"

DispReel::DispReel()
{
}

DispReel::~DispReel()
{
   assert(m_rd == nullptr);
}

DispReel *DispReel::CopyForPlay(PinTable *live_table)
{
   STANDARD_EDITABLE_COPY_FOR_PLAY_IMPL(DispReel, live_table)
   return dst;
}

// This function is called when ever a new instance of this object is created
// (along with the constructor (above))
//
HRESULT DispReel::Init(PinTable *const ptable, const float x, const float y, const bool fromMouseClick, const bool forPlay)
{
   m_ptable = ptable;
   SetDefaults(fromMouseClick);
   m_d.m_v1.x = x;
   m_d.m_v1.y = y;
   m_d.m_v2.x = x + getBoxWidth();
   m_d.m_v2.y = y + getBoxHeight();
   return forPlay ? S_OK : InitVBA(fTrue, 0, nullptr);
}

// set the defaults for the objects persistent data (m_d.*) in case this
// is a new instance of this object or there is a backwards compatability
// issue (old version of object doesn't contain all the needed fields)
//
void DispReel::SetDefaults(const bool fromMouseClick)
{
#define regKey Settings::DefaultPropsEMReel

   // object is only available on the backglass
   m_backglass = true;

   // set all the Data defaults
   bool hr;
   hr = g_pvp->m_settings.LoadValue(regKey, "Image"s, m_d.m_szImage);
   if (!hr || !fromMouseClick)
      m_d.m_szImage.clear();

   hr = g_pvp->m_settings.LoadValue(regKey, "Sound"s, m_d.m_szSound);
   if (!hr || !fromMouseClick)
      m_d.m_szSound.clear();

   m_d.m_useImageGrid = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "UseImageGrid"s, false) : false;
   m_d.m_visible = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "Visible"s, true) : true;
   m_d.m_imagesPerGridRow = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "ImagesPerRow"s, 1) : 1;
   m_d.m_transparent = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "Transparent"s, false) : false;
   m_d.m_reelcount = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "ReelCount"s, 5) : 5;
   m_d.m_width = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "Width"s, 30.0f) : 30.0f;
   m_d.m_height = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "Height"s, 40.0f) : 40.0f;
   m_d.m_reelspacing = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "ReelSpacing"s, 4.0f) : 4.0f;
   m_d.m_motorsteps = fromMouseClick ? (int)g_pvp->m_settings.LoadValueWithDefault(regKey, "MotorSteps"s, 2.f) : 2;
   m_d.m_digitrange = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "DigitRange"s, 9) : 9;
   m_d.m_updateinterval = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "UpdateInterval"s, 50) : 50;
   m_d.m_backcolor = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "BackColor"s, (int)RGB(64, 64, 64)) : RGB(64, 64, 64);
   m_d.m_tdr.m_TimerEnabled = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "TimerEnabled"s, false) : false;
   m_d.m_tdr.m_TimerInterval = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "TimerInterval"s, 100) : 100;

#undef regKey
}

void DispReel::WriteRegDefaults()
{
#define regKey Settings::DefaultPropsEMReel

   g_pvp->m_settings.SaveValue(regKey, "Image"s, m_d.m_szImage);
   g_pvp->m_settings.SaveValue(regKey, "Sound"s, m_d.m_szSound);
   g_pvp->m_settings.SaveValue(regKey, "UseImageGrid"s, m_d.m_useImageGrid);
   g_pvp->m_settings.SaveValue(regKey, "Visible"s, m_d.m_visible);
   g_pvp->m_settings.SaveValue(regKey, "ImagesPerRow"s, m_d.m_imagesPerGridRow);
   g_pvp->m_settings.SaveValue(regKey, "Transparent"s, m_d.m_transparent);
   g_pvp->m_settings.SaveValue(regKey, "ReelCount"s, m_d.m_reelcount);
   g_pvp->m_settings.SaveValue(regKey, "Width"s, m_d.m_width);
   g_pvp->m_settings.SaveValue(regKey, "Height"s, m_d.m_height);
   g_pvp->m_settings.SaveValue(regKey, "ReelSpacing"s, m_d.m_reelspacing);
   g_pvp->m_settings.SaveValue(regKey, "MotorSteps"s, (float)m_d.m_motorsteps);
   g_pvp->m_settings.SaveValue(regKey, "DigitRange"s, m_d.m_digitrange);
   g_pvp->m_settings.SaveValue(regKey, "UpdateInterval"s, m_d.m_updateinterval);
   g_pvp->m_settings.SaveValue(regKey, "BackColor"s, (int)m_d.m_backcolor);
   g_pvp->m_settings.SaveValue(regKey, "TimerEnabled"s, m_d.m_tdr.m_TimerEnabled);
   g_pvp->m_settings.SaveValue(regKey, "TimerInterval"s, m_d.m_tdr.m_TimerInterval);

#undef regKey
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
void DispReel::UIRenderPass1(Sur * const psur)
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
void DispReel::UIRenderPass2(Sur * const psur)
{
   if (!GetPTable()->GetEMReelsEnabled()) return;

   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetFillColor(-1);
   psur->SetObject(this);
   psur->SetObject(nullptr);

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
void DispReel::GetTimers(vector<HitTimer*> &pvht)
{
   IEditable::BeginPlay();
   m_phittimer = new HitTimer(GetName(), m_d.m_tdr.m_TimerInterval, this);
   if (m_d.m_tdr.m_TimerEnabled)
      pvht.push_back(m_phittimer);
}

#pragma region Physics

void DispReel::GetHitShapes(vector<HitObject*> &pvho)
{
}

void DispReel::GetHitShapesDebug(vector<HitObject*> &pvho)
{
}

void DispReel::EndPlay()
{
   IEditable::EndPlay();
}

#pragma endregion


#pragma region Rendering

void DispReel::RenderSetup(RenderDevice *device)
{
   assert(m_rd == nullptr);
   m_rd = device;

   // get the render sizes of the objects (reels and frame)
   m_renderwidth = max(0.0f, m_d.m_width / (float)EDITOR_BG_WIDTH);
   m_renderheight = max(0.0f, m_d.m_height / (float)EDITOR_BG_HEIGHT);

   for (int i = 0; i < m_d.m_reelcount; ++i)
   {
      m_reelInfo[i].currentValue = 0;
      m_reelInfo[i].motorPulses = 0;
      m_reelInfo[i].motorStepCount = 0;
      m_reelInfo[i].motorCalcStep = 0;
      m_reelInfo[i].motorOffset = 0;
   }

   // get a pointer to the image specified in the object
   Texture * const pin = m_ptable->GetImage(m_d.m_szImage); // pointer to image information from the image manager

   if (!pin)
      return;

   int GridCols, GridRows;

   // get the number of images per row of the image
   if (m_d.m_useImageGrid)
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
      m_reeldigitwidth  = (float)pin->m_width  / (float)GridCols;
      m_reeldigitheight = (float)pin->m_height / (float)GridRows;
   }
   else
      ShowError("DispReel: GridCols/GridRows are zero!");

   const float ratiox = m_reeldigitwidth  / (float)pin->m_width;
   const float ratioy = m_reeldigitheight / (float)pin->m_height;

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

   m_timeNextUpdate = g_pplayer->m_time_msec + m_d.m_updateinterval;
}

void DispReel::RenderRelease()
{
   assert(m_rd != nullptr);
   m_rd = nullptr;
}

// This function is called each frame.  It basically check to see if the update
// interval has expired and if so handles the rolling of the reels according to the
// number of motor steps queued up for each reel
void DispReel::UpdateAnimation(const float diff_time_msec)
{
   assert(m_rd != nullptr);
   bool animated = false;
   while (g_pplayer->m_time_msec >= m_timeNextUpdate)
   {
      m_timeNextUpdate += m_d.m_updateinterval;

      // work out the roll over values
      const int OverflowValue = m_d.m_digitrange;
      const int AdjustValue = OverflowValue + 1;

      const float step = m_reeldigitheight / (float)m_d.m_motorsteps;

      // start at the last reel and work forwards (right to left)
      for (int i = m_d.m_reelcount - 1; i >= 0; i--)
      {
         // if the motor has stopped, and there are still motor steps then start another one
         if ((m_reelInfo[i].motorPulses != 0) && (m_reelInfo[i].motorStepCount == 0))
         {
            // get the number of steps (or increments) needed to move the reel
            m_reelInfo[i].motorStepCount = m_d.m_motorsteps;
            m_reelInfo[i].motorCalcStep = (m_reelInfo[i].motorPulses > 0) ? step : -step;
            m_reelInfo[i].motorOffset = 0;

            // play the sound (if any) for each click of the reel
            if (!m_d.m_szSound.empty() && (m_d.m_szSound != "<None>"))
            {
               WCHAR mySound[MAXTOKEN];
               MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szSound.c_str(), -1, mySound, MAXTOKEN);
               const BSTR mySoundBSTR = SysAllocString(mySound);
               m_ptable->PlaySound(mySoundBSTR, 0, 1.0f, 0.f, 0.f, 0, VARIANT_FALSE, VARIANT_TRUE, 0.f);
               SysFreeString(mySoundBSTR);
            }

            animated = true;
         }

         // is the reel in the process of moving??
         if (m_reelInfo[i].motorStepCount != 0)
         {
            m_reelInfo[i].motorOffset += m_reelInfo[i].motorCalcStep;
            m_reelInfo[i].motorStepCount--;
            // have we reached the end of the step
            if (m_reelInfo[i].motorStepCount <= 0)
            {
               m_reelInfo[i].motorStepCount = 0;      // best to be safe (paranoid)
               m_reelInfo[i].motorOffset = 0;

               if (m_reelInfo[i].motorPulses < 0)
               {
                  m_reelInfo[i].motorPulses++;
                  m_reelInfo[i].currentValue--;
                  if (m_reelInfo[i].currentValue < 0)
                  {
                     m_reelInfo[i].currentValue += AdjustValue;
                     // if not the first reel then decrement the next reel by 1
                     if (i != 0)
                        m_reelInfo[i - 1].motorPulses--;
                  }
               }
               else
               {
                  m_reelInfo[i].motorPulses--;
                  m_reelInfo[i].currentValue++;
                  if (m_reelInfo[i].currentValue > OverflowValue)
                  {
                     m_reelInfo[i].currentValue -= AdjustValue;
                     // if not the first reel then increment the next reel
                     // along by 1 (just like a car odometer)
                     if (i != 0)
                        m_reelInfo[i - 1].motorPulses++;
                  }
               }
            }

            animated = true;
         }
      }
   }
   if (animated)
      FireGroupEvent(DISPID_AnimateEvents_Animate);
}

void DispReel::Render(const unsigned int renderMask)
{
   assert(m_rd != nullptr);
   const bool isStaticOnly = renderMask & Player::STATIC_ONLY;
   const bool isDynamicOnly = renderMask & Player::DYNAMIC_ONLY;
   const bool isReflectionPass = renderMask & Player::REFLECTION_PASS;
   TRACE_FUNCTION();

   if (isStaticOnly
   || !m_d.m_visible
   || !GetPTable()->GetEMReelsEnabled()
   || (m_backglass && isReflectionPass))
      return;

   Texture * const pin = m_ptable->GetImage(m_d.m_szImage);
   if (!pin)
      return;

   m_rd->ResetRenderState();

   if (m_backglass)
   {
      m_rd->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);
      m_rd->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
      m_rd->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
   }

   m_rd->DMDShader->SetFloat(SHADER_alphaTestValue, (float)(128.0 / 255.0));
   m_rd->EnableAlphaBlend(false);

   m_rd->DMDShader->SetTechnique(SHADER_TECHNIQUE_basic_noDMD);

   const vec4 c = convertColor(0xFFFFFFFF, 1.f);
   m_rd->DMDShader->SetVector(SHADER_vColor_Intensity, &c);

   m_rd->DMDShader->SetTexture(SHADER_tex_sprite, pin, SF_TRILINEAR, SA_REPEAT, SA_REPEAT);

   // set up all the reel positions within the object frame
   const float renderspacingx = max(0.0f, m_d.m_reelspacing / (float)EDITOR_BG_WIDTH);
   const float renderspacingy = max(0.0f, m_d.m_reelspacing / (float)EDITOR_BG_HEIGHT);
         float x1 = m_d.m_v1.x / (float)EDITOR_BG_WIDTH  + renderspacingx;
   const float y1 = m_d.m_v1.y / (float)EDITOR_BG_HEIGHT + renderspacingy;

   for (int r = 0; r < m_d.m_reelcount; ++r) //!! optimize by doing all draws in a single one
   {
      const float u0 = m_digitTexCoords[m_reelInfo[r].currentValue].u_min;
      const float v0 = m_digitTexCoords[m_reelInfo[r].currentValue].v_min;
      const float u1 = m_digitTexCoords[m_reelInfo[r].currentValue].u_max;
      const float v1 = m_digitTexCoords[m_reelInfo[r].currentValue].v_max;

      Vertex3D_NoTex2 vertices[4] =
      {
         { 1.f, 1.f, 0.f, 0.f, 0.f, 1.f, u1, v1 },
         { 0.f, 1.f, 0.f, 0.f, 0.f, 1.f, u0, v1 },
         { 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, u1, v0 },
         { 0.f, 0.f, 0.f, 0.f, 0.f, 1.f, u0, v0 }
      };

      for (unsigned int i = 0; i < 4; ++i)
      {
         vertices[i].x =        (vertices[i].x * m_renderwidth  + x1)*2.0f - 1.0f;
         vertices[i].y = 1.0f - (vertices[i].y * m_renderheight + y1)*2.0f;
      }

      m_rd->DrawTexturedQuad(m_rd->DMDShader, vertices);

      // move to the next reel
      x1 += renderspacingx + m_renderwidth;
   }

   m_rd->DMDShader->SetFloat(SHADER_alphaTestValue, 1.0f);
}

#pragma endregion


void DispReel::SetObjectPos()
{
   m_vpinball->SetObjectPosCur(m_d.m_v1.x, m_d.m_v1.y);
}

void DispReel::MoveOffset(const float dx, const float dy)
{
   m_d.m_v1.x += dx;
   m_d.m_v1.y += dy;

   m_d.m_v2.x += dx;
   m_d.m_v2.y += dy;
}

Vertex2D DispReel::GetCenter() const
{
   return m_d.m_v1;
}

void DispReel::PutCenter(const Vertex2D& pv)
{
   m_d.m_v1 = pv;

   m_d.m_v2.x = pv.x + getBoxWidth();
   m_d.m_v2.y = pv.y + getBoxHeight();
}

HRESULT DispReel::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, const bool saveForUndo)
{
   BiffWriter bw(pstm, hcrypthash);

   bw.WriteVector2(FID(VER1), m_d.m_v1);
   bw.WriteVector2(FID(VER2), m_d.m_v2);
   bw.WriteInt(FID(CLRB), m_d.m_backcolor);
   bw.WriteBool(FID(TMON), m_d.m_tdr.m_TimerEnabled);
   bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
   bw.WriteBool(FID(TRNS), m_d.m_transparent);
   bw.WriteString(FID(IMAG), m_d.m_szImage);
   bw.WriteString(FID(SOUN), m_d.m_szSound);
   bw.WriteWideString(FID(NAME), m_wzName);
   bw.WriteFloat(FID(WDTH), m_d.m_width);
   bw.WriteFloat(FID(HIGH), m_d.m_height);
   bw.WriteFloat(FID(RCNT), (float)m_d.m_reelcount);
   bw.WriteFloat(FID(RSPC), m_d.m_reelspacing);
   bw.WriteFloat(FID(MSTP), (float)m_d.m_motorsteps);
   bw.WriteFloat(FID(RANG), (float)m_d.m_digitrange);
   bw.WriteInt(FID(UPTM), m_d.m_updateinterval);
   bw.WriteBool(FID(UGRD), m_d.m_useImageGrid);
   bw.WriteBool(FID(VISI), m_d.m_visible);
   bw.WriteInt(FID(GIPR), m_d.m_imagesPerGridRow);

   ISelect::SaveData(pstm, hcrypthash);

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

bool DispReel::LoadToken(const int id, BiffReader * const pbr)
{
   switch (id)
   {
   case FID(PIID): pbr->GetInt((int *)pbr->m_pdata); break;
   case FID(VER1): pbr->GetVector2(m_d.m_v1); break;
   case FID(VER2): pbr->GetVector2(m_d.m_v2); break;
   case FID(WDTH): pbr->GetFloat(m_d.m_width); break;
   case FID(HIGH): pbr->GetFloat(m_d.m_height); break;
   case FID(CLRB): pbr->GetInt(m_d.m_backcolor); break;
   case FID(TMON): pbr->GetBool(m_d.m_tdr.m_TimerEnabled); break;
   case FID(TMIN): pbr->GetInt(m_d.m_tdr.m_TimerInterval); break;
   case FID(NAME): pbr->GetWideString(m_wzName,sizeof(m_wzName)/sizeof(m_wzName[0])); break;
   case FID(TRNS): pbr->GetBool(m_d.m_transparent); break;
   case FID(IMAG): pbr->GetString(m_d.m_szImage); break;
   case FID(RCNT):
   {
      float reel;
      pbr->GetFloat(reel);
      m_d.m_reelcount = (int)reel;
      break;
   }
   case FID(RSPC): pbr->GetFloat(m_d.m_reelspacing); break;
   case FID(MSTP):
   {
      float motorsteps;
      pbr->GetFloat(motorsteps);
      m_d.m_motorsteps = (int)motorsteps;
      break;
   }
   case FID(SOUN): pbr->GetString(m_d.m_szSound); break;
   case FID(UGRD): pbr->GetBool(m_d.m_useImageGrid); break;
   case FID(VISI): pbr->GetBool(m_d.m_visible); break;
   case FID(GIPR): pbr->GetInt(m_d.m_imagesPerGridRow); break;
   case FID(RANG):
   {
      float dig;
      pbr->GetFloat(dig);
      m_d.m_digitrange = (int)dig;
      break;
   }
   case FID(UPTM): pbr->GetInt(m_d.m_updateinterval); break;
   case FID(FONT): //!! deprecated, only here to support loading of old tables
   {
#ifndef __STANDALONE__
      IFont *pIFont;
      FONTDESC fd;
      fd.cbSizeofstruct = sizeof(FONTDESC);
      fd.lpstrName = (LPOLESTR)(L"Times New Roman");
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
#else
      // https://github.com/freezy/VisualPinball.Engine/blob/master/VisualPinball.Engine/VPT/Font.cs#L25
      char data[255];

      ULONG read;
      pbr->ReadBytes(data, 3, &read);
      pbr->ReadBytes(data, 1, &read); // Italic
      pbr->ReadBytes(data, 2, &read); // Weight
      pbr->ReadBytes(data, 4, &read); // Size
      pbr->ReadBytes(data, 1, &read); // nameLen
      pbr->ReadBytes(data, (int)data[0], &read); // name
#endif

      break;
   }
   default: ISelect::LoadToken(id, pbr); break;
   }
   return true;
}

HRESULT DispReel::InitPostLoad()
{
   return S_OK;
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
   m_d.m_backcolor = newVal;

   return S_OK;
}

STDMETHODIMP DispReel::get_Reels(float *pVal)
{
   *pVal = (float)GetReels();

   return S_OK;
}

STDMETHODIMP DispReel::put_Reels(float newVal)
{
   SetReels((int)newVal);

   return S_OK;
}

STDMETHODIMP DispReel::get_Width(float *pVal)
{
   *pVal = GetWidth();

   return S_OK;
}

STDMETHODIMP DispReel::put_Width(float newVal)
{
   SetWidth(newVal);

   return S_OK;
}

STDMETHODIMP DispReel::get_Height(float *pVal)
{
   *pVal = GetHeight();

   return S_OK;
}

STDMETHODIMP DispReel::put_Height(float newVal)
{
   SetHeight(newVal);

   return S_OK;
}

STDMETHODIMP DispReel::get_X(float *pVal)
{
   *pVal = GetX();
   m_vpinball->SetStatusBarUnitInfo(string(), true);

   return S_OK;
}

STDMETHODIMP DispReel::put_X(float newVal)
{
   SetX(newVal);

   return S_OK;
}

STDMETHODIMP DispReel::get_Y(float *pVal)
{
   *pVal = GetY();

   return S_OK;
}

STDMETHODIMP DispReel::put_Y(float newVal)
{
   SetY(newVal);

   return S_OK;
}

STDMETHODIMP DispReel::get_IsTransparent(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_transparent);

   return S_OK;
}

STDMETHODIMP DispReel::put_IsTransparent(VARIANT_BOOL newVal)
{
   m_d.m_transparent = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP DispReel::get_Image(BSTR *pVal)
{
   WCHAR wz[MAXTOKEN];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szImage.c_str(), -1, wz, MAXTOKEN);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP DispReel::put_Image(BSTR newVal)
{
   char szImage[MAXTOKEN];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, szImage, MAXTOKEN, nullptr, nullptr);
   const Texture * const tex = m_ptable->GetImage(szImage);
   if (tex && tex->IsHDR())
   {
      ShowError("Cannot use a HDR image (.exr/.hdr) here");
      return E_FAIL;
   }
   m_d.m_szImage = szImage;

   return S_OK;
}

STDMETHODIMP DispReel::get_Spacing(float *pVal)
{
   *pVal = GetSpacing();
   return S_OK;
}

STDMETHODIMP DispReel::put_Spacing(float newVal)
{
   SetSpacing(newVal);

   return S_OK;
}

STDMETHODIMP DispReel::get_Sound(BSTR *pVal)
{
   WCHAR wz[MAXTOKEN];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szSound.c_str(), -1, wz, MAXTOKEN);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP DispReel::put_Sound(BSTR newVal)
{
   char buf[MAXTOKEN];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXTOKEN, nullptr, nullptr);
   m_d.m_szSound = buf;

   return S_OK;
}

STDMETHODIMP DispReel::get_Steps(float *pVal)
{
   *pVal = (float)GetMotorSteps();

   return S_OK;
}

STDMETHODIMP DispReel::put_Steps(float newVal)
{
   SetMotorSteps((int)newVal);

   return S_OK;
}

STDMETHODIMP DispReel::get_Range(float *pVal)
{
   *pVal = (float)GetRange();

   return S_OK;
}

STDMETHODIMP DispReel::put_Range(float newVal)
{
   SetRange((int)newVal);

   return S_OK;
}

STDMETHODIMP DispReel::get_UpdateInterval(long *pVal)
{
   *pVal = GetUpdateInterval();

   return S_OK;
}

STDMETHODIMP DispReel::put_UpdateInterval(long newVal)
{
   SetUpdateInterval((int)newVal);
   if (g_pplayer)
      m_timeNextUpdate = g_pplayer->m_time_msec + m_d.m_updateinterval;

   return S_OK;
}

STDMETHODIMP DispReel::get_UseImageGrid(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_useImageGrid);

   return S_OK;
}

STDMETHODIMP DispReel::put_UseImageGrid(VARIANT_BOOL newVal)
{
   m_d.m_useImageGrid = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP DispReel::get_Visible(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_visible);

   return S_OK;
}

STDMETHODIMP DispReel::put_Visible(VARIANT_BOOL newVal)
{
   m_d.m_visible = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP DispReel::get_ImagesPerGridRow(long *pVal)
{
   *pVal = GetImagesPerGridRow();

   return S_OK;
}

STDMETHODIMP DispReel::put_ImagesPerGridRow(long newVal)
{
   SetImagesPerGridRow((int)newVal);

   return S_OK;
}

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
         m_reelInfo[i].motorPulses -= digitValue;
      else
         m_reelInfo[i].motorPulses += digitValue;

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
      m_reelInfo[l].currentValue = 0;
      m_reelInfo[l].motorPulses = 0;
      m_reelInfo[l].motorStepCount = 0;
      m_reelInfo[l].motorCalcStep = 0;
      m_reelInfo[l].motorOffset = 0;
   }

   // set the reel values (startint at the right most reel and move left)
   int i = m_d.m_reelcount - 1;
   while ((val != 0) && (i >= 0))
   {
      const int digitValue = val % valbase;
      // remove the value for this reel from the overall number
      val /= valbase;
      m_reelInfo[i].currentValue = digitValue;
      // move to next reel
      i--;
   }

   // force a immediate screen update
   m_timeNextUpdate = g_pplayer->m_time_msec;

   return S_OK;
}

STDMETHODIMP DispReel::ResetToZero()
{
   int carry = 0;
   const int overflowValue = m_d.m_digitrange + 1;

   // work for the last reel to the first one
   for (int i = m_d.m_reelcount - 1; i >= 0; i--)
   {
      const int adjust = overflowValue - carry - m_reelInfo[i].currentValue;
      carry = 0;

      if (adjust != overflowValue)
      {
         // overwrite the pulse count with the adjust value
         m_reelInfo[i].motorPulses = adjust;
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
      m_reelInfo[reel].motorPulses += PulseCount;
      return S_OK;
   }
   else
      return E_FAIL;
}


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
