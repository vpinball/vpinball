#include "StdAfx.h"

LightSeq::LightSeq()
{
   m_lightseqanim.m_pLightSeq = this;
}

LightSeq::~LightSeq()
{
}

HRESULT LightSeq::Init(PinTable *ptable, float x, float y, bool fromMouseClick)
{
   m_ptable = ptable;

   m_d.m_v.x = x;
   m_d.m_v.y = y;

   SetDefaults(fromMouseClick);

   return InitVBA(fTrue, 0, NULL);
}

void LightSeq::SetDefaults(bool fromMouseClick)
{
   HRESULT hr;
   float fTmp;
   int iTmp;

   hr = GetRegInt("DefaultProps\\LightSequence", "UpdateInterval", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_updateinterval = iTmp;
   else
      m_d.m_updateinterval = 25;

   char tmp[MAXNAMEBUFFER];
   hr = GetRegString("DefaultProps\\LightSequence", "Collection", tmp, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_wzCollection[0] = 0x00;
   else
   {
      UNICODE_FROM_ANSI(m_d.m_wzCollection, tmp, lstrlen(tmp));
      m_d.m_wzCollection[lstrlen(tmp)] = '\0';
   }
   hr = GetRegStringAsFloat("DefaultProps\\LightSequence", "CenterX", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_vCenter.x = fTmp;
   else
      m_d.m_vCenter.x = EDITOR_BG_WIDTH / 2;

   hr = GetRegStringAsFloat("DefaultProps\\LightSequence", "CenterY", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_vCenter.y = fTmp;
   else
      m_d.m_vCenter.y = (2 * EDITOR_BG_WIDTH) / 2;

   hr = GetRegInt("DefaultProps\\LightSequence", "TimerEnabled", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_tdr.m_fTimerEnabled = iTmp == 0 ? false : true;
   else
      m_d.m_tdr.m_fTimerEnabled = false;

   hr = GetRegInt("DefaultProps\\LightSequence", "TimerInterval", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_tdr.m_TimerInterval = iTmp;
   else
      m_d.m_tdr.m_TimerInterval = 100;
}

void LightSeq::WriteRegDefaults()
{
   MAKE_ANSIPTR_FROMWIDE(strTmp2, (WCHAR *)m_d.m_wzCollection);
   SetRegValue("DefaultProps\\LightSequence", "UpdateInterval", REG_DWORD, &m_d.m_updateinterval, 4);
   SetRegValue("DefaultProps\\LightSequence", "Collection", REG_SZ, strTmp2, 66);
   SetRegValueFloat("DefaultProps\\LightSequence", "CenterX", m_d.m_vCenter.x);
   SetRegValueFloat("DefaultProps\\LightSequence", "CenterY", m_d.m_vCenter.y);
   SetRegValueBool("DefaultProps\\LightSequence", "TimerEnabled", m_d.m_tdr.m_fTimerEnabled);
   SetRegValue("DefaultProps\\LightSequence", "TimerInterval", REG_DWORD, &m_d.m_tdr.m_TimerInterval, 4);
}

void LightSeq::SetObjectPos()
{
   g_pvp->SetObjectPosCur(m_d.m_v.x, m_d.m_v.y);
}

void LightSeq::MoveOffset(const float dx, const float dy)
{
   m_d.m_v.x += dx;
   m_d.m_v.y += dy;

   m_ptable->SetDirtyDraw();
}

void LightSeq::GetCenter(Vertex2D * const pv) const
{
   *pv = m_d.m_v;
}

void LightSeq::PutCenter(const Vertex2D * const pv)
{
   m_d.m_v = *pv;

   m_ptable->SetDirtyDraw();
}

// this function draws the shape of the object with a solid fill
// only used in the editor and not the game
//
// this is called before the grid lines are drawn on the map
//
void LightSeq::PreRender(Sur * const psur)
{
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetObject(this);

   for (int i = 0; i < 8; ++i)
   {
      psur->SetFillColor((i % 2 == 0) ? RGB(255, 0, 0) : RGB(128, 0, 0));
      const float angle = (float)((M_PI*2.0) / 8.0)*(float)i;
      const float sn = sinf(angle);
      const float cs = cosf(angle);
      psur->Ellipse(m_d.m_v.x + sn*12.0f, m_d.m_v.y - cs*12.0f, 4.0f);
   }

   psur->SetFillColor(RGB(255, 0, 0));
   psur->Ellipse(m_d.m_v.x, m_d.m_v.y - 3.0f, 4.0f);
}

// this function draws the shape of the object with a black outline (no solid fill)
// only used in the editor and not the game
//
// this is called after the grid lines have been drawn on the map.  draws a solid
// outline over the grid lines
//
void LightSeq::Render(Sur * const psur)
{
   psur->SetFillColor(-1);
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);

   psur->SetObject(this);

   psur->Ellipse(m_d.m_v.x, m_d.m_v.y, 18.0f);

   for (int i = 0; i < 8; ++i)
   {
      const float angle = (float)((M_PI*2.0) / 8.0)*(float)i;
      const float sn = sinf(angle);
      const float cs = cosf(angle);
      psur->Ellipse(m_d.m_v.x + sn*12.0f, m_d.m_v.y - cs*12.0f, 4.0f);
   }

   psur->Ellipse(m_d.m_v.x, m_d.m_v.y - 3.0f, 4.0f);

   RenderOutline(psur);
}

// this function draw the little center marker which is a cross with the usual LS circles on it

void LightSeq::RenderOutline(Sur * const psur)
{
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetObject((ISelect *)this);

   psur->Line(m_d.m_vCenter.x - 10.0f, m_d.m_vCenter.y, m_d.m_vCenter.x + 10.0f, m_d.m_vCenter.y);
   psur->Line(m_d.m_vCenter.x, m_d.m_vCenter.y - 10.0f, m_d.m_vCenter.x, m_d.m_vCenter.y + 10.0f);

   for (int i = 0; i < 8; ++i)
   {
      psur->SetFillColor((i % 2 == 0) ? RGB(255, 0, 0) : RGB(128, 0, 0));
      const float angle = (float)((M_PI*2.0) / 8.0)*(float)i;
      const float sn = sinf(angle);
      const float cs = cosf(angle);
      psur->Ellipse(m_d.m_vCenter.x + sn*7.0f, m_d.m_vCenter.y - cs*7.0f, 2.0f);
   }

   psur->SetFillColor(RGB(255, 0, 0));
   psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y - 2.5f, 2.0f);
}

// Renders the image onto the Blueprint
//
// We don't want this on the blue print as it is non-essensial
//
void LightSeq::RenderBlueprint(Sur *psur, const bool solid)
{
}

// Registers the timer with the game call which then makes a call back when the interval
// has expired.
//
// for this sort of object it is basically not really required but hey, somebody might use it..
//
void LightSeq::GetTimers(Vector<HitTimer> * const pvht)
{
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

void LightSeq::GetHitShapes(Vector<HitObject> * const pvho)
{
}

void LightSeq::GetHitShapesDebug(Vector<HitObject> * const pvho)
{
}

// This method is called as the game exits..
// it cleans up any allocated memory used by the instace of the object
//
void LightSeq::EndPlay()
{
   if (m_pgridData != NULL)
   {
      delete[] m_pgridData;
      m_pgridData = NULL;
   }

   IEditable::EndPlay();
}

void LightSeq::PostRenderStatic(RenderDevice* pd3dDevice)
{
}

void LightSeq::RenderSetup(RenderDevice* pd3dDevice)
{
   // zero pointers as a safe guard
   m_pcollection = NULL;
   m_pgridData = NULL;
   // no animation in progress
   m_playInProgress = false;
   m_pauseInProgress = false;
   // turn off any tracers
   m_th1.type = eSeqNull;
   m_th2.type = eSeqNull;
   m_tt1.type = eSeqNull;
   m_tt2.type = eSeqNull;
   // flush the queue
   m_queue.Head = 0;
   m_queue.Tail = 0;

   // get a BSTR version of the collection we are to use
   CComBSTR bstrCollection = m_d.m_wzCollection;

   // get the number of collections available
   int size = m_ptable->m_vcollection.Size();
   for (int i = 0; i < size; ++i)
   {
      // get the name of this collection
      CComBSTR bstr;
      m_ptable->m_vcollection.ElementAt(i)->get_Name(&bstr);
      // is it the one we are to use?
      if (WideStrCmp(bstr, bstrCollection) == 0)
      {
         // yep, set a pointer to this sub-collection
         m_pcollection = m_ptable->m_vcollection.ElementAt(i);
         break;
      }
   }

   // if the collection wasn't found or there are no collections available then bomb out
   if (m_pcollection == NULL)
      return;

   // get the grid demensions (from the table size)
   const float tablewidth = m_ptable->m_right - m_ptable->m_left;
   const float tableheight = m_ptable->m_bottom - m_ptable->m_top;

   m_lightSeqGridWidth = (int)tablewidth / LIGHTSEQGRIDSCALE;
   m_lightSeqGridHeight = (int)tableheight / LIGHTSEQGRIDSCALE;

   // set the centre point of the grid for effects which start from the center
   m_GridXCenter = floorf(m_d.m_vCenter.x * (float)(1.0 / LIGHTSEQGRIDSCALE));
   m_GridYCenter = floorf(m_d.m_vCenter.y * (float)(1.0 / LIGHTSEQGRIDSCALE));
   m_GridXCenterAdjust = abs(m_lightSeqGridWidth / 2 - (int)m_GridXCenter);
   m_GridYCenterAdjust = abs(m_lightSeqGridHeight / 2 - (int)m_GridYCenter);

   // allocate the grid for this sequence
   m_pgridData = new short[m_lightSeqGridHeight*m_lightSeqGridWidth];
   if (m_pgridData == NULL)
   {
      // make the entire collection (for the sequencer) invalid and bomb out
      m_pcollection = NULL;
      return;
   }
   else
      ZeroMemory((void *)m_pgridData, (size_t)((m_lightSeqGridHeight*m_lightSeqGridWidth)*sizeof(short)));

   // get the number of elements (objects) in the collection (referenced by m_visel)
   size = m_pcollection->m_visel.Size();

   // go though the collection and get the cordinates of all the lights
   for (int i = 0; i < size; ++i)
   {
      // get the type of object
      const ItemTypeEnum type = m_pcollection->m_visel.ElementAt(i)->GetIEditable()->GetItemType();
      // must be a light
      if (type == eItemLight)
      {
         float x, y;
         // process a light
         Light * const pLight = (Light *)m_pcollection->m_visel.ElementAt(i);
         pLight->get_X(&x);
         pLight->get_Y(&y);

         if (pLight->m_fBackglass)
         {
            // if the light is on the backglass then scale up its Y position
            y *= 2.666f; // 2 little devils ;-)
         }

         // scale down to suit the size of the light sequence grid
         const unsigned int ix = (int)(x * (float)(1.0 / LIGHTSEQGRIDSCALE));
         const unsigned int iy = (int)(y * (float)(1.0 / LIGHTSEQGRIDSCALE));
         // if on the playfield
         if ( /*(ix >= 0) &&*/ (ix < (unsigned int)m_lightSeqGridWidth) && //>=0 handled by unsigned int
            /*(iy >= 0) &&*/ (iy < (unsigned int)m_lightSeqGridHeight)) //>=0 handled by unsigned int
         {
            const int gridIndex = iy * m_lightSeqGridWidth + ix;

            // then store the index offset into the grid (plus 1, 0 is no object)
            m_pgridData[gridIndex] = i + 1;
         }
      }
   }
}

void LightSeq::RenderStatic(RenderDevice* pd3dDevice)
{
}

// This function is called during Animate(). It basically check to see if the update
// interval has expired and if so handles the light effect
void LightSeq::Animate()
{
   if (m_playInProgress)
   {
      if (g_pplayer->m_time_msec >= m_timeNextUpdate)
      {
         if (!m_pauseInProgress)
         {
            m_timeNextUpdate = g_pplayer->m_time_msec + m_updateRate;
            // process the head tracers
            const bool th1finished = ProcessTracer(&m_th1, LightStateOn);
            const bool th2finished = ProcessTracer(&m_th2, LightStateOn);
            // and any tail tracers
            const bool tt1finished = ProcessTracer(&m_tt1, LightStateOff);
            const bool tt2finished = ProcessTracer(&m_tt2, LightStateOff);

            // has the animation finished
            if (th1finished && th2finished && tt1finished && tt2finished)
            {
               // the sequence has finished, paused for the specified value or a single interval
               // (bit of a breather or load balance)
               m_timeNextUpdate = g_pplayer->m_time_msec + max(m_updateRate, m_pauseValue);
               m_pauseInProgress = true;
            }
         }
         else
         {
            // test the remaining replays?
            m_replayCount--;
            if (m_replayCount != 0)
            {
               // if not zero then restart the same animation again
               SetupTracers(m_playAnimation, m_tailLength, m_replayCount, m_pauseValue);
            }
            else
            {
               // move the tail to the next position
               ++m_queue.Tail;
               if (m_queue.Tail >= LIGHTSEQQUEUESIZE)
                  m_queue.Tail = 0;

               // not playing at the moment
               m_playInProgress = false;
               // if the queue is empty then reset the lights to their real state
               if (m_queue.Head == m_queue.Tail)
               {
                  StopPlay();
                  // and signal the script ( Sub <LIGHTSEQNAME>_PlayDone() )
                  FireVoidEvent(DISPID_LightSeqEvents_PlayDone);
               }
            }
         }
      }
   }
   else
   {
      // is there something in the queue?
      if (m_queue.Head != m_queue.Tail)
      {
         // yes
         const int Tail = m_queue.Tail;
         // set the update rate for this sequence
         m_updateRate = m_queue.Data[Tail].UpdateRate;
         // set up the tracers
         SetupTracers(m_queue.Data[Tail].Animation,
            m_queue.Data[Tail].TailLength,
            m_queue.Data[Tail].Repeat,
            m_queue.Data[Tail].Pause);
         // and start the ball rolling again
         m_playInProgress = true;
      }
   }
}


STDMETHODIMP LightSeq::InterfaceSupportsErrorInfo(REFIID riid)
{
   static const IID* arr[] =
   {
      &IID_ILightSeq,
   };

   for (int i = 0; i < sizeof(arr) / sizeof(arr[0]); ++i)
   {
      if (InlineIsEqualGUID(*arr[i], riid))
         return S_OK;
   }
   return S_FALSE;
}

HRESULT LightSeq::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   BiffWriter bw(pstm, hcrypthash, hcryptkey);

   bw.WriteStruct(FID(VCEN), &m_d.m_v, sizeof(Vertex2D));
   bw.WriteWideString(FID(COLC), (WCHAR *)m_d.m_wzCollection);
   bw.WriteFloat(FID(CTRX), m_d.m_vCenter.x);
   bw.WriteFloat(FID(CTRY), m_d.m_vCenter.y);
   bw.WriteInt(FID(UPTM), m_d.m_updateinterval);
   bw.WriteBool(FID(TMON), m_d.m_tdr.m_fTimerEnabled);
   bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);

   bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);

   bw.WriteBool(FID(BGLS), m_fBackglass);

   bw.WriteTag(FID(ENDB));

   return S_OK;
}

HRESULT LightSeq::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   SetDefaults(false);

   BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);

   m_ptable = ptable;

   br.Load();
   return S_OK;
}

BOOL LightSeq::LoadToken(int id, BiffReader *pbr)
{
   if (id == FID(PIID))
   {
      pbr->GetInt((int *)pbr->m_pdata);
   }
   else if (id == FID(VCEN))
   {
      pbr->GetStruct(&m_d.m_v, sizeof(Vertex2D));
   }
   else if (id == FID(COLC))
   {
      pbr->GetWideString((WCHAR *)m_d.m_wzCollection);
   }
   else if (id == FID(CTRX))
   {
      pbr->GetFloat(&m_d.m_vCenter.x);
   }
   else if (id == FID(CTRY))
   {
      pbr->GetFloat(&m_d.m_vCenter.y);
   }
   else if (id == FID(UPTM))
   {
      pbr->GetInt(&m_d.m_updateinterval);
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
   else if (id == FID(BGLS))
   {
      pbr->GetBool(&m_fBackglass);
   }
   return fTrue;
}

HRESULT LightSeq::InitPostLoad()
{
   return S_OK;
}

void LightSeq::GetDialogPanes(Vector<PropertyPane> *pvproppane)
{
   PropertyPane *pproppane;

   pproppane = new PropertyPane(IDD_PROP_NAME, NULL);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPLIGHTSEQ_POSITION, IDS_POSITION);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPLIGHTSEQ_STATE, IDS_STATE);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROP_TIMER, IDS_MISC);
   pvproppane->AddElement(pproppane);
}


STDMETHODIMP LightSeq::get_Collection(BSTR *pVal)
{
   WCHAR wz[sizeof(m_d.m_wzCollection)];

   memcpy(wz, m_d.m_wzCollection, sizeof(m_d.m_wzCollection));
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP LightSeq::put_Collection(BSTR newVal)
{
   STARTUNDO
      memcpy(m_d.m_wzCollection, (void *)newVal, sizeof(m_d.m_wzCollection));
   STOPUNDO

      return S_OK;
}

STDMETHODIMP LightSeq::get_CenterX(float *pVal)
{
   *pVal = m_d.m_vCenter.x;

   return S_OK;
}

STDMETHODIMP LightSeq::put_CenterX(float newVal)
{
   if ((newVal < 0) || (newVal >= (float)EDITOR_BG_WIDTH))
      return E_FAIL;
   STARTUNDO
      m_d.m_vCenter.x = newVal;
   // set the centre point of the grid for effects which start from the center
   m_GridXCenter = floorf(m_d.m_vCenter.x * (float)(1.0 / LIGHTSEQGRIDSCALE));
   m_GridXCenterAdjust = abs(m_lightSeqGridWidth / 2 - (int)m_GridXCenter);
   STOPUNDO

      return S_OK;
}

STDMETHODIMP LightSeq::get_CenterY(float *pVal)
{
   *pVal = m_d.m_vCenter.y;

   return S_OK;
}

STDMETHODIMP LightSeq::put_CenterY(float newVal)
{
   if ((newVal < 0) || (newVal >= (float)(2 * EDITOR_BG_WIDTH)))
      return E_FAIL;

   STARTUNDO
      m_d.m_vCenter.y = newVal;
   // set the centre point of the grid for effects which start from the center
   m_GridYCenter = floorf(m_d.m_vCenter.y * (float)(1.0 / LIGHTSEQGRIDSCALE));
   m_GridYCenterAdjust = abs(m_lightSeqGridHeight / 2 - (int)m_GridYCenter);
   STOPUNDO

      return S_OK;
}

STDMETHODIMP LightSeq::get_UpdateInterval(long *pVal)
{
   *pVal = m_d.m_updateinterval;

   return S_OK;
}

STDMETHODIMP LightSeq::put_UpdateInterval(long newVal)
{
   STARTUNDO
      m_d.m_updateinterval = max(1, newVal);
   STOPUNDO
      return S_OK;
}

STDMETHODIMP LightSeq::Play(SequencerState Animation, long TailLength, long Repeat, long Pause)
{
   HRESULT	rc = S_OK;

   // sanity check the parameters
   if (TailLength < 0)
   {
      TailLength = 0;
   }
   if (Repeat <= 0)
   {
      Repeat = 1;
   }
   if (Pause < 0)
   {
      Pause = 0;
   }

   // 'all lights on' and 'all lights off' are directly processed and not put into the queue
   if (Animation == SeqAllOn)
   {
      // turn on all lights
      SetAllLightsToState(LightStateOn);
   }
   else
   {
      if (Animation == SeqAllOff)
      {
         // turn off all lights
         SetAllLightsToState(LightStateOff);
      }
      else
      {
         // move the head of the queue to the next position
         int newHead = m_queue.Head + 1;
         // handle the wrap around (circlular queue)
         if (newHead >= LIGHTSEQQUEUESIZE)
         {
            newHead = 0;
         }
         // if the queue is full, then bomb out
         if (newHead == m_queue.Tail)
         {
            rc = E_FAIL;
         }
         else
         {
            // else load up the queue
            m_queue.Data[m_queue.Head].Animation = Animation;
            m_queue.Data[m_queue.Head].TailLength = TailLength;
            m_queue.Data[m_queue.Head].Repeat = Repeat;
            m_queue.Data[m_queue.Head].Pause = Pause;
            m_queue.Data[m_queue.Head].UpdateRate = m_d.m_updateinterval;
            m_queue.Head = newHead;
         }
      }
   }
   return rc;
}

STDMETHODIMP LightSeq::StopPlay()
{
   // no animation in progress
   m_playInProgress = false;
   m_pauseInProgress = false;
   // turn off any tracers
   m_th1.type = eSeqNull;
   m_th2.type = eSeqNull;
   m_tt1.type = eSeqNull;
   m_tt2.type = eSeqNull;
   // flush the queue
   m_queue.Head = 0;
   m_queue.Tail = 0;

   // Reset lights back to original state
   if (m_pcollection != NULL)
   {
      const int size = m_pcollection->m_visel.Size();
      for (int i = 0; i < size; ++i)
      {
         const ItemTypeEnum type = m_pcollection->m_visel.ElementAt(i)->GetIEditable()->GetItemType();
         if (type == eItemLight)
         {
            Light * const pLight = (Light *)m_pcollection->m_visel.ElementAt(i);
            LightState state;
            pLight->get_State(&state);
            pLight->unLockLight();
            pLight->put_State(state);
         }
      }
   }
   return S_OK;
}

void LightSeq::SetupTracers(const SequencerState Animation, long TailLength, long Repeat, long Pause)
{
   bool inverse = false;

   // no animation in progress
   m_playInProgress = false;
   m_pauseInProgress = false;

   // turn off any tracers
   m_th1.type = eSeqNull;
   m_th2.type = eSeqNull;
   m_tt1.type = eSeqNull;
   m_tt2.type = eSeqNull;

   // remember the current sequence (before it gets altered)
   m_playAnimation = Animation;
   m_tailLength = TailLength;
   m_replayCount = Repeat;
   m_pauseValue = Pause;

   switch (Animation)
   {
      // blink all lights on and off
   case SeqBlinking:
      m_th1.type = eSeqBlink;
      m_th1.length = 0;
      m_th1.frameCount = 2;
      break;

      // Randomly turn lights on and off for the pause period
   case SeqRandom:
      m_th1.type = eSeqRandom;
      m_th1.length = TailLength;
      m_th1.frameCount = Pause / m_updateRate;
      // no repeat or pause for this effect and certainly no tail
      TailLength = 0;
      Repeat = 1;
      Pause = 0;
      m_tailLength = TailLength;
      m_replayCount = Repeat;
      m_pauseValue = Pause;
      break;

      // Turn on all lights starting at the bottom of the playfield and moving up
   case SeqUpOff:
      inverse = true;
   case SeqUpOn:
      m_th1.type = eSeqLine;
      m_th1.x = 0;
      m_th1.stepX = 0;
      m_th1.processStepX = 1.0f;
      m_th1.y = (float)m_lightSeqGridHeight - 1.0f;
      m_th1.stepY = -1.0f;
      m_th1.processStepY = 0;
      m_th1.length = m_lightSeqGridWidth;
      m_th1.frameCount = m_lightSeqGridHeight;
      break;

      // Turn on all lights starting at the top of the playfield and moving down
   case SeqDownOff:
      inverse = true;
   case SeqDownOn:
      m_th1.type = eSeqLine;
      m_th1.x = 0;
      m_th1.stepX = 0;
      m_th1.processStepX = 1.0f;
      m_th1.y = 0;
      m_th1.stepY = 1.0f;
      m_th1.processStepY = 0;
      m_th1.length = m_lightSeqGridWidth;
      m_th1.frameCount = m_lightSeqGridHeight;
      break;

      // Turn on all lights starting at the left of the playfield and moving right
   case SeqRightOff:
      inverse = true;
   case SeqRightOn:
      m_th1.type = eSeqLine;
      m_th1.x = 0;
      m_th1.stepX = 1.0f;
      m_th1.processStepX = 0;
      m_th1.y = 0;
      m_th1.stepY = 0;
      m_th1.processStepY = 1.0f;
      m_th1.length = m_lightSeqGridHeight;
      m_th1.frameCount = m_lightSeqGridWidth;
      TailLength /= 2;
      break;

      // Turn on all lights starting at the right of the playfield and moving left
   case SeqLeftOff:
      inverse = true;
   case SeqLeftOn:
      m_th1.type = eSeqLine;
      m_th1.x = (float)m_lightSeqGridWidth - 1.0f;
      m_th1.stepX = -1.0f;
      m_th1.processStepX = 0;
      m_th1.y = 0;
      m_th1.stepY = 0;
      m_th1.processStepY = 1.0f;
      m_th1.length = m_lightSeqGridHeight;
      m_th1.frameCount = m_lightSeqGridWidth;
      TailLength /= 2;
      break;

      // Turn on all lights starting at the bottom/left of the playfield and diagonally up
   case SeqDiagUpRightOff:
      inverse = true;
   case SeqDiagUpRightOn:
      m_th1.type = eSeqLine;
      m_th1.x = 0;
      m_th1.stepX = 0;
      m_th1.processStepX = 0.5f;
      m_th1.y = (float)m_lightSeqGridHeight - 1;
      m_th1.stepY = -1.0f;
      m_th1.processStepY = 1.0f;
      m_th1.length = m_lightSeqGridWidth * 2;
      m_th1.frameCount = m_lightSeqGridHeight + (m_lightSeqGridWidth * 2);
      break;

      // Turn on all lights starting at the bottom/right of the playfield and diagonally up
   case SeqDiagUpLeftOff:
      inverse = true;
   case SeqDiagUpLeftOn:
      m_th1.type = eSeqLine;
      m_th1.x = (float)m_lightSeqGridWidth - 1.0f;
      m_th1.stepX = 0;
      m_th1.processStepX = -0.5f;
      m_th1.y = (float)m_lightSeqGridHeight - 1.0f;
      m_th1.stepY = -1.0f;
      m_th1.processStepY = 1.0f;
      m_th1.length = m_lightSeqGridWidth * 2;
      m_th1.frameCount = m_lightSeqGridHeight + (m_lightSeqGridWidth * 2);
      break;

      // Turn on all lights starting at the top/left of the playfield and diagonally down
   case SeqDiagDownRightOff:
      inverse = true;
   case SeqDiagDownRightOn:
      m_th1.type = eSeqLine;
      m_th1.x = 0;
      m_th1.stepX = 0;
      m_th1.processStepX = 0.5f;
      m_th1.y = 0;
      m_th1.stepY = 1.0f;
      m_th1.processStepY = -1.0f;
      m_th1.length = m_lightSeqGridWidth * 2;
      m_th1.frameCount = m_lightSeqGridHeight + (m_lightSeqGridWidth * 2);
      break;

      // Turn on all lights starting at the top/right of the playfield and diagonally down
   case SeqDiagDownLeftOff:
      inverse = true;
   case SeqDiagDownLeftOn:
      m_th1.type = eSeqLine;
      m_th1.x = (float)m_lightSeqGridWidth - 1.0f;
      m_th1.stepX = 0;
      m_th1.processStepX = -0.5f;
      m_th1.y = 0;
      m_th1.stepY = 1.0f;
      m_th1.processStepY = -1.0f;
      m_th1.length = m_lightSeqGridWidth * 2;
      m_th1.frameCount = m_lightSeqGridHeight + (m_lightSeqGridWidth * 2);
      break;

      // Turn on all lights starting in the middle and moving outwards to the side edges
   case SeqMiddleOutHorizOff:
      inverse = true;
   case SeqMiddleOutHorizOn:
      m_th1.type = eSeqLine;
      m_th1.x = m_GridXCenter - 1.0f;
      m_th1.stepX = -1.0f;
      m_th1.processStepX = 0;
      m_th1.y = 0;
      m_th1.stepY = 0;
      m_th1.processStepY = 1.0f;
      m_th1.length = m_lightSeqGridHeight;
      m_th1.frameCount = m_lightSeqGridWidth / 2 + m_GridXCenterAdjust;
      m_th2.type = eSeqLine;
      m_th2.x = m_GridXCenter;
      m_th2.stepX = 1.0f;
      m_th2.processStepX = 0;
      m_th2.y = 0;
      m_th2.stepY = 0;
      m_th2.processStepY = 1.0f;
      m_th2.length = m_lightSeqGridHeight;
      m_th2.frameCount = m_lightSeqGridWidth / 2 + m_GridXCenterAdjust;
      TailLength /= 4;
      break;

      // Turn on all lights starting on the side edges and moving into the middle
   case SeqMiddleInHorizOff:
      inverse = true;
   case SeqMiddleInHorizOn:
   {
      const float effectlength = (float)(m_lightSeqGridWidth / 2 + m_GridXCenterAdjust);
      m_th1.type = eSeqLine;
      m_th1.x = m_GridXCenter - effectlength;
      m_th1.stepX = 1.0f;
      m_th1.processStepX = 0;
      m_th1.y = 0;
      m_th1.stepY = 0;
      m_th1.processStepY = 1.0f;
      m_th1.length = m_lightSeqGridHeight;
      m_th1.frameCount = (int)effectlength + 1;
      m_th2.type = eSeqLine;
      m_th2.x = m_GridXCenter + effectlength;
      m_th2.stepX = -1.0f;
      m_th2.processStepX = 0;
      m_th2.y = 0;
      m_th2.stepY = 0;
      m_th2.processStepY = 1.0f;
      m_th2.length = m_lightSeqGridHeight;
      m_th2.frameCount = (int)effectlength;
      TailLength /= 4;
   }
   break;

   // Turn on all lights starting in the middle and moving outwards to the top and bottom
   case SeqMiddleOutVertOff:
      inverse = true;
   case SeqMiddleOutVertOn:
      m_th1.type = eSeqLine;
      m_th1.x = 0;
      m_th1.stepX = 0;
      m_th1.processStepX = 1.0f;
      m_th1.y = m_GridYCenter - 1.0f;
      m_th1.stepY = -1;
      m_th1.processStepY = 0;
      m_th1.length = m_lightSeqGridWidth;
      m_th1.frameCount = m_lightSeqGridHeight / 2 + m_GridYCenterAdjust;
      m_th2.type = eSeqLine;
      m_th2.x = 0;
      m_th2.stepX = 0;
      m_th2.processStepX = 1.0f;
      m_th2.y = m_GridYCenter;
      m_th2.stepY = 1.0f;
      m_th2.processStepY = 0;
      m_th2.length = m_lightSeqGridWidth;
      m_th2.frameCount = m_lightSeqGridHeight / 2 + m_GridYCenterAdjust;
      TailLength /= 2;
      break;

      // Turn on all lights starting on the top and bottom edges and moving inwards to the middle
   case SeqMiddleInVertOff:
      inverse = true;
   case SeqMiddleInVertOn:
   {
      const float effectlength = (float)(m_lightSeqGridHeight / 2 + m_GridYCenterAdjust);
      m_th1.type = eSeqLine;
      m_th1.x = 0;
      m_th1.stepX = 0;
      m_th1.processStepX = 1.0f;
      m_th1.y = m_GridYCenter - effectlength;
      m_th1.stepY = 1.0f;
      m_th1.processStepY = 0;
      m_th1.length = m_lightSeqGridWidth;
      m_th1.frameCount = (int)effectlength + 1;
      m_th2.type = eSeqLine;
      m_th2.x = 0;
      m_th2.stepX = 0;
      m_th2.processStepX = 1.0f;
      m_th2.y = m_GridYCenter + effectlength;
      m_th2.stepY = -1.0f;
      m_th2.processStepY = 0;
      m_th2.length = m_lightSeqGridWidth;
      m_th2.frameCount = (int)effectlength;
      TailLength /= 2;
   }
   break;

   // top half of the playfield wipes on to the right while the bottom half wipes on to the left
   case SeqStripe1HorizOff:
      inverse = true;
   case SeqStripe1HorizOn:
      m_th1.type = eSeqLine;
      m_th1.x = 0;
      m_th1.stepX = 1.0f;
      m_th1.processStepX = 0;
      m_th1.y = m_GridYCenter - 1.0f;
      m_th1.stepY = 0;
      m_th1.processStepY = -1.0f;
      m_th1.length = m_lightSeqGridHeight / 2 + m_GridYCenterAdjust;
      m_th1.frameCount = m_lightSeqGridWidth;
      m_th2.type = eSeqLine;
      m_th2.x = (float)m_lightSeqGridWidth - 1.0f;
      m_th2.stepX = -1.0f;
      m_th2.processStepX = 0;
      m_th2.y = m_GridYCenter;
      m_th2.stepY = 0;
      m_th2.processStepY = 1.0f;
      m_th2.length = m_lightSeqGridHeight / 2 + m_GridXCenterAdjust;
      m_th2.frameCount = m_lightSeqGridWidth;
      TailLength /= 2;
      break;

      // top half of the playfield wipes on to the left while the bottom half wipes on to the right
   case SeqStripe2HorizOff:
      inverse = true;
   case SeqStripe2HorizOn:
      m_th1.type = eSeqLine;
      m_th1.x = (float)m_lightSeqGridWidth - 1.0f;
      m_th1.stepX = -1.0f;
      m_th1.processStepX = 0;
      m_th1.y = m_GridYCenter - 1.0f;
      m_th1.stepY = 0;
      m_th1.processStepY = -1.0f;
      m_th1.length = m_lightSeqGridHeight / 2 + m_GridYCenterAdjust;
      m_th1.frameCount = m_lightSeqGridWidth;
      m_th2.type = eSeqLine;
      m_th2.x = 0;
      m_th2.stepX = 1.0f;
      m_th2.processStepX = 0;
      m_th2.y = m_GridYCenter;
      m_th2.stepY = 0;
      m_th2.processStepY = 1.0f;
      m_th2.length = m_lightSeqGridHeight / 2 + m_GridYCenterAdjust;
      m_th2.frameCount = m_lightSeqGridWidth;
      TailLength /= 2;
      break;

      // left side of the playfield wipes on going up while the right side wipes on doing down
   case SeqStripe1VertOff:
      inverse = true;
   case SeqStripe1VertOn:
      m_th1.type = eSeqLine;
      m_th1.x = m_GridXCenter - 1.0f;
      m_th1.stepX = 0;
      m_th1.processStepX = -1.0f;
      m_th1.y = (float)m_lightSeqGridHeight - 1.0f;
      m_th1.stepY = -1.0f;
      m_th1.processStepY = 0;
      m_th1.length = m_lightSeqGridWidth / 2 + m_GridXCenterAdjust;
      m_th1.frameCount = m_lightSeqGridHeight;
      m_th2.type = eSeqLine;
      m_th2.x = m_GridXCenter;
      m_th2.stepX = 0;
      m_th2.processStepX = 1.0f;
      m_th2.y = 0;
      m_th2.stepY = 1.0f;
      m_th2.processStepY = 0;
      m_th2.length = m_lightSeqGridWidth / 2 + m_GridXCenterAdjust;
      m_th2.frameCount = m_lightSeqGridHeight;
      break;

      // left side of the playfield wipes on going down while the right side wipes on doing up
   case SeqStripe2VertOff:
      inverse = true;
   case SeqStripe2VertOn:
      m_th1.type = eSeqLine;
      m_th1.x = m_GridXCenter - 1.0f;
      m_th1.stepX = 0;
      m_th1.processStepX = -1.0f;
      m_th1.y = 0;
      m_th1.stepY = 1.0f;
      m_th1.processStepY = 0;
      m_th1.length = m_lightSeqGridWidth / 2 + m_GridXCenterAdjust;
      m_th1.frameCount = m_lightSeqGridHeight;
      m_th2.type = eSeqLine;
      m_th2.x = m_GridXCenter;
      m_th2.stepX = 0;
      m_th2.processStepX = 1.0f;
      m_th2.y = (float)m_lightSeqGridHeight - 1.0f;
      m_th2.stepY = -1.0f;
      m_th2.processStepY = 0;
      m_th2.length = m_lightSeqGridWidth / 2 + m_GridXCenterAdjust;
      m_th2.frameCount = m_lightSeqGridHeight;
      break;

      // turn lights on, cross-hatch with even lines going right and odd lines going left
   case SeqHatch1HorizOff:
      inverse = true;
   case SeqHatch1HorizOn:
      m_th1.type = eSeqLine;
      m_th1.x = 0;
      m_th1.stepX = 1.0f;
      m_th1.processStepX = 0;
      m_th1.y = 0;
      m_th1.stepY = 0;
      m_th1.processStepY = 2.0f;
      m_th1.length = m_lightSeqGridHeight / 2;
      m_th1.frameCount = m_lightSeqGridWidth;
      m_th2.type = eSeqLine;
      m_th2.x = (float)m_lightSeqGridWidth - 1.0f;
      m_th2.stepX = -1.0f;
      m_th2.processStepX = 0;
      m_th2.y = 1.0f;
      m_th2.stepY = 0;
      m_th2.processStepY = 2.0f;
      m_th2.length = m_lightSeqGridHeight / 2;
      m_th2.frameCount = m_lightSeqGridWidth;
      TailLength /= 2;
      break;

      // turn lights on, cross-hatch with even lines going left and odd lines going right
   case SeqHatch2HorizOff:
      inverse = true;
   case SeqHatch2HorizOn:
      m_th1.type = eSeqLine;
      m_th1.x = (float)m_lightSeqGridWidth - 1.0f;
      m_th1.stepX = -1.0f;
      m_th1.processStepX = 0;
      m_th1.y = 0;
      m_th1.stepY = 0;
      m_th1.processStepY = 2.0f;
      m_th1.length = m_lightSeqGridHeight / 2;
      m_th1.frameCount = m_lightSeqGridWidth;
      m_th2.type = eSeqLine;
      m_th2.x = 0;
      m_th2.stepX = 1.0f;
      m_th2.processStepX = 0;
      m_th2.y = 1.0f;
      m_th2.stepY = 0;
      m_th2.processStepY = 2.0f;
      m_th2.length = m_lightSeqGridHeight / 2;
      m_th2.frameCount = m_lightSeqGridWidth;
      TailLength /= 2;
      break;

      // turn lights on, cross-hatch with even lines going up and odd lines going down
   case SeqHatch1VertOff:
      inverse = true;
   case SeqHatch1VertOn:
      m_th1.type = eSeqLine;
      m_th1.x = 0;
      m_th1.stepX = 0;
      m_th1.processStepX = 2.0f;
      m_th1.y = (float)m_lightSeqGridHeight - 1.0f;
      m_th1.stepY = -1.0f;
      m_th1.processStepY = 0;
      m_th1.length = m_lightSeqGridWidth / 2;
      m_th1.frameCount = m_lightSeqGridHeight;
      m_th2.type = eSeqLine;
      m_th2.x = 1.0f;
      m_th2.stepX = 0;
      m_th2.processStepX = 2.0f;
      m_th2.y = 0;
      m_th2.stepY = 1.0f;
      m_th2.processStepY = 0;
      m_th2.length = m_lightSeqGridWidth / 2;
      m_th2.frameCount = m_lightSeqGridHeight;
      break;

      // turn lights on, cross-hatch with even lines going down and odd lines going up
   case SeqHatch2VertOff:
      inverse = true;
   case SeqHatch2VertOn:
      m_th1.type = eSeqLine;
      m_th1.x = 0;
      m_th1.stepX = 0;
      m_th1.processStepX = 2.0f;
      m_th1.y = 0;
      m_th1.stepY = 1.0f;
      m_th1.processStepY = 0;
      m_th1.length = m_lightSeqGridWidth / 2;
      m_th1.frameCount = m_lightSeqGridHeight;
      m_th2.type = eSeqLine;
      m_th2.x = 1.0f;
      m_th2.stepX = 0;
      m_th2.processStepX = 2.0f;
      m_th2.y = (float)m_lightSeqGridHeight - 1.0f;
      m_th2.stepY = -1.0f;
      m_th2.processStepY = 0;
      m_th2.length = m_lightSeqGridWidth / 2;
      m_th2.frameCount = m_lightSeqGridHeight;
      break;

      // turn on all the lights, starting in the table center and circle out
   case SeqCircleOutOff:
      inverse = true;
   case SeqCircleOutOn:
      m_th1.type = eSeqCircle;
      m_th1.radius = 0;
      m_th1.stepRadius = 1.0f;
      m_th1.x = m_GridXCenter;
      m_th1.y = m_GridYCenter;
      m_th1.frameCount = m_lightSeqGridWidth / 2 + m_GridXCenterAdjust + m_lightSeqGridHeight / 2 + m_GridYCenterAdjust;
      break;

      // turn on all the lights, starting at the table edges and circle in
   case SeqCircleInOff:
      inverse = true;
   case SeqCircleInOn:
      m_th1.type = eSeqCircle;
      m_th1.radius = (float)(m_lightSeqGridWidth / 2 + m_GridXCenterAdjust + m_lightSeqGridHeight / 2 + m_GridYCenterAdjust);
      m_th1.stepRadius = -1.0f;
      m_th1.x = m_GridXCenter;
      m_th1.y = m_GridYCenter;
      m_th1.frameCount = m_lightSeqGridWidth / 2 + m_GridXCenterAdjust + m_lightSeqGridHeight / 2 + m_GridYCenterAdjust;
      break;

      // turn all the lights on starting in the middle and sweeping around to the right
   case SeqClockRightOff:
      inverse = true;
   case SeqClockRightOn:
      m_th1.type = eSeqRadar;
      m_th1.angle = 0;
      m_th1.stepAngle = 1.0f;
      m_th1.processRadius = 1.0f;
      m_th1.x = m_GridXCenter;
      m_th1.y = m_GridYCenter;
      m_th1.length = m_lightSeqGridWidth / 2 + m_GridXCenterAdjust + m_lightSeqGridHeight / 2 + m_GridYCenterAdjust;
      m_th1.frameCount = 360;
      break;

      // turn all the lights on starting in the middle and sweeping around to the left
   case SeqClockLeftOff:
      inverse = true;
   case SeqClockLeftOn:
      m_th1.type = eSeqRadar;
      m_th1.angle = 0;
      m_th1.stepAngle = -1.0f;
      m_th1.processRadius = 1.0f;
      m_th1.x = m_GridXCenter;
      m_th1.y = m_GridYCenter;
      m_th1.length = m_lightSeqGridWidth / 2 + m_GridXCenterAdjust + m_lightSeqGridHeight / 2 + m_GridYCenterAdjust;
      m_th1.frameCount = 360;
      break;

      // turn all the lights on starting in the middle/bottom and sweeping around to the right
   case SeqRadarRightOff:
      inverse = true;
   case SeqRadarRightOn:
      m_th1.type = eSeqRadar;
      m_th1.angle = 270.0f;
      m_th1.stepAngle = 1.0f;
      m_th1.processRadius = 1.0f;
      m_th1.x = m_GridXCenter;
      m_th1.y = (float)m_lightSeqGridHeight - 1.0f;
      m_th1.length = m_lightSeqGridHeight + m_lightSeqGridWidth + m_GridYCenterAdjust + m_GridXCenterAdjust;
      m_th1.frameCount = 180;
      break;

      // turn all the lights on starting in the middle/bottom and sweeping around to the right
   case SeqRadarLeftOff:
      inverse = true;
   case SeqRadarLeftOn:
      m_th1.type = eSeqRadar;
      m_th1.angle = 90.0f;
      m_th1.stepAngle = -1.0f;
      m_th1.processRadius = 1.0f;
      m_th1.x = m_GridXCenter;
      m_th1.y = (float)m_lightSeqGridHeight - 1.0f;
      m_th1.length = m_lightSeqGridHeight + m_lightSeqGridWidth + m_GridYCenterAdjust + m_GridXCenterAdjust;
      m_th1.frameCount = 180;
      break;

      // turn all the lights on starting in the middle/yop and sweeping around to the right
   case SeqWiperRightOff:
      inverse = true;
   case SeqWiperRightOn:
      m_th1.type = eSeqRadar;
      m_th1.angle = 270.0f;
      m_th1.stepAngle = -1.0f;
      m_th1.processRadius = 1.0f;
      m_th1.x = m_GridXCenter;
      m_th1.y = 0;
      m_th1.length = m_lightSeqGridHeight + m_lightSeqGridWidth + m_GridYCenterAdjust + m_GridXCenterAdjust;
      m_th1.frameCount = 180;
      break;

      // turn all the lights on starting in the middle/top and sweeping around to the right
   case SeqWiperLeftOff:
      inverse = true;
   case SeqWiperLeftOn:
      m_th1.type = eSeqRadar;
      m_th1.angle = 90.0f;
      m_th1.stepAngle = 1.0f;
      m_th1.processRadius = 1.0f;
      m_th1.x = m_GridXCenter;
      m_th1.y = 0;
      m_th1.length = m_lightSeqGridHeight + m_lightSeqGridWidth + m_GridYCenterAdjust + m_GridXCenterAdjust;
      m_th1.frameCount = 180;
      break;

      // turn all the lights on starting in the middle/left edge and sweeping up
   case SeqFanLeftUpOff:
      inverse = true;
   case SeqFanLeftUpOn:
      m_th1.type = eSeqRadar;
      m_th1.angle = 180.0f;
      m_th1.stepAngle = -1.0f;
      m_th1.processRadius = 1.0f;
      m_th1.x = 0;
      m_th1.y = m_GridYCenter;
      m_th1.length = m_lightSeqGridHeight + m_lightSeqGridWidth + m_GridYCenterAdjust + m_GridXCenterAdjust;
      m_th1.frameCount = 180;
      break;

      // turn all the lights on starting in the middle/left edge and sweeping down
   case SeqFanLeftDownOff:
      inverse = true;
   case SeqFanLeftDownOn:
      m_th1.type = eSeqRadar;
      m_th1.angle = 0;
      m_th1.stepAngle = 1.0f;
      m_th1.processRadius = 1.0f;
      m_th1.x = 0;
      m_th1.y = m_GridYCenter;
      m_th1.length = m_lightSeqGridHeight + m_lightSeqGridWidth + m_GridYCenterAdjust + m_GridXCenterAdjust;
      m_th1.frameCount = 180;
      break;

      // turn all the lights on starting in the middle/right edge and sweeping up
   case SeqFanRightUpOff:
      inverse = true;
   case SeqFanRightUpOn:
      m_th1.type = eSeqRadar;
      m_th1.angle = 180.0f;
      m_th1.stepAngle = -1.0f;
      m_th1.processRadius = -1.0f;
      m_th1.x = (float)m_lightSeqGridWidth - 1.0f;
      m_th1.y = m_GridYCenter;
      m_th1.length = m_lightSeqGridHeight + m_lightSeqGridWidth + m_GridYCenterAdjust + m_GridXCenterAdjust;
      m_th1.frameCount = 180;
      break;

      // turn all the lights on starting in the middle/right edge and sweeping down
   case SeqFanRightDownOff:
      inverse = true;
   case SeqFanRightDownOn:
      m_th1.type = eSeqRadar;
      m_th1.angle = 0;
      m_th1.stepAngle = 1.0f;
      m_th1.processRadius = -1.0f;
      m_th1.x = (float)m_lightSeqGridWidth - 1.0f;
      m_th1.y = m_GridYCenter;
      m_th1.length = m_lightSeqGridHeight + m_lightSeqGridWidth + m_GridYCenterAdjust + m_GridXCenterAdjust;
      m_th1.frameCount = 180;
      break;

      // turn all the lights on starting in the bottom/left corner and arcing up
   case SeqArcBottomLeftUpOff:
      inverse = true;
   case SeqArcBottomLeftUpOn:
      m_th1.type = eSeqRadar;
      m_th1.angle = 90.0f;
      m_th1.stepAngle = -1.0f;
      m_th1.processRadius = 1.0f;
      m_th1.x = 0;
      m_th1.y = (float)m_lightSeqGridHeight - 1.0f;
      m_th1.length = m_lightSeqGridHeight + m_lightSeqGridWidth;
      m_th1.frameCount = 90;
      break;

      // turn all the lights on starting in the bottom/left corner and arcing down
   case SeqArcBottomLeftDownOff:
      inverse = true;
   case SeqArcBottomLeftDownOn:
      m_th1.type = eSeqRadar;
      m_th1.angle = 0;
      m_th1.stepAngle = 1.0f;
      m_th1.processRadius = 1.0f;
      m_th1.x = 0;
      m_th1.y = (float)m_lightSeqGridHeight - 1.0f;
      m_th1.length = m_lightSeqGridHeight + m_lightSeqGridWidth;
      m_th1.frameCount = 90;
      break;

      // turn all the lights on starting in the bottom/right corner and arcing up
   case SeqArcBottomRightUpOff:
      inverse = true;
   case SeqArcBottomRightUpOn:
      m_th1.type = eSeqRadar;
      m_th1.angle = 90.0f;
      m_th1.stepAngle = 1.0f;
      m_th1.processRadius = -1.0f;
      m_th1.x = (float)m_lightSeqGridWidth - 1.0f;
      m_th1.y = (float)m_lightSeqGridHeight - 1.0f;
      m_th1.length = m_lightSeqGridHeight + m_lightSeqGridWidth;
      m_th1.frameCount = 90;
      break;

      // turn all the lights on starting in the bottom/right corner and arcing down
   case SeqArcBottomRightDownOff:
      inverse = true;
   case SeqArcBottomRightDownOn:
      m_th1.type = eSeqRadar;
      m_th1.angle = 180.0f;
      m_th1.stepAngle = -1.0f;
      m_th1.processRadius = -1.0f;
      m_th1.x = (float)m_lightSeqGridWidth - 1.0f;
      m_th1.y = (float)m_lightSeqGridHeight - 1.0f;
      m_th1.length = m_lightSeqGridHeight + m_lightSeqGridWidth;
      m_th1.frameCount = 90;
      break;

      // turn all the lights on starting in the top/left corner and arcing up
   case SeqArcTopLeftUpOff:
      inverse = true;
   case SeqArcTopLeftUpOn:
      m_th1.type = eSeqRadar;
      m_th1.angle = 180.0f;
      m_th1.stepAngle = -1.0f;
      m_th1.processRadius = 1.0f;
      m_th1.x = 0;
      m_th1.y = 0;
      m_th1.length = m_lightSeqGridHeight + m_lightSeqGridWidth;
      m_th1.frameCount = 90;
      break;

      // turn all the lights on starting in the top/left corner and arcing down
   case SeqArcTopLeftDownOff:
      inverse = true;
   case SeqArcTopLeftDownOn:
      m_th1.type = eSeqRadar;
      m_th1.angle = 90.0f;
      m_th1.stepAngle = 1.0f;
      m_th1.processRadius = 1.0f;
      m_th1.x = 0;
      m_th1.y = 0;
      m_th1.length = m_lightSeqGridHeight + m_lightSeqGridWidth;
      m_th1.frameCount = 90;
      break;

      // turn all the lights on starting in the top/right corner and arcing up
   case SeqArcTopRightUpOff:
      inverse = true;
   case SeqArcTopRightUpOn:
      m_th1.type = eSeqRadar;
      m_th1.angle = 0;
      m_th1.stepAngle = 1.0f;
      m_th1.processRadius = -1.0f;
      m_th1.x = (float)m_lightSeqGridWidth - 1.0f;
      m_th1.y = 0;
      m_th1.length = m_lightSeqGridHeight + m_lightSeqGridWidth;
      m_th1.frameCount = 90;
      break;

      // turn all the lights on starting in the top/right corner and arcing down
   case SeqArcTopRightDownOff:
      inverse = true;
   case SeqArcTopRightDownOn:
      m_th1.type = eSeqRadar;
      m_th1.angle = 90.0f;
      m_th1.stepAngle = -1.0f;
      m_th1.processRadius = -1.0f;
      m_th1.x = (float)m_lightSeqGridWidth - 1.0f;
      m_th1.y = 0;
      m_th1.length = m_lightSeqGridHeight + m_lightSeqGridWidth;
      m_th1.frameCount = 90;
      break;

      // turn all the lights on starting in the centre and screwing (2 tracers) clockwise
   case SeqScrewRightOff:
      inverse = true;
   case SeqScrewRightOn:
      m_th1.type = eSeqRadar;
      m_th1.angle = 0;
      m_th1.stepAngle = 1.0f;
      m_th1.processRadius = 1.0f;
      m_th1.x = m_GridXCenter;
      m_th1.y = m_GridYCenter - 1.0f;
      m_th1.length = m_lightSeqGridHeight / 2 + m_GridYCenterAdjust + m_lightSeqGridWidth / 2 + m_GridXCenterAdjust;
      m_th1.frameCount = 180;
      m_th2.type = eSeqRadar;
      m_th2.angle = (float)(180 - 1);
      m_th2.stepAngle = 1.0f;
      m_th2.processRadius = 1.0f;
      m_th2.x = m_GridXCenter;
      m_th2.y = m_GridYCenter;
      m_th2.length = m_lightSeqGridHeight / 2 + m_GridYCenterAdjust + m_lightSeqGridWidth / 2 + m_GridXCenterAdjust;
      m_th2.frameCount = 180;
      break;

      // turn all the lights on starting in the centre and screwing (2 tracers) anti-clockwise
   case SeqScrewLeftOff:
      inverse = true;
   case SeqScrewLeftOn:
      m_th1.type = eSeqRadar;
      m_th1.angle = 0;
      m_th1.stepAngle = -1.0f;
      m_th1.processRadius = 1.0f;
      m_th1.x = m_GridXCenter;
      m_th1.y = m_GridYCenter - 1.0f;
      m_th1.length = m_lightSeqGridHeight / 2 + m_GridYCenterAdjust + m_lightSeqGridWidth / 2 + m_GridXCenterAdjust;
      m_th1.frameCount = 180;
      m_th2.type = eSeqRadar;
      m_th2.angle = (float)(180 - 1);
      m_th2.stepAngle = -1.0f;
      m_th2.processRadius = 1.0f;
      m_th2.x = m_GridXCenter;
      m_th2.y = m_GridYCenter;
      m_th2.length = m_lightSeqGridHeight / 2 + m_GridYCenterAdjust + m_lightSeqGridWidth / 2 + m_GridXCenterAdjust;
      m_th2.frameCount = 180;
      break;

      // unknown/supported animation (should happen as the animation is a enum, but hey!)
   default:
      // cause it to expire in 1 frame and move onto the next animation
      m_th1.length = 0;
      m_th1.frameCount = 1;
      TailLength = 0;
      Repeat = 1;
      Pause = 0;
      break;
   }

   // no delays on the effects
   m_th1.delay = 0;
   m_th2.delay = 0;
   m_tt1.delay = 0;
   m_tt2.delay = 0;

   // is there a tail for this effect?
   if (TailLength != 0)
   {
      // then the head effect becomes the tail with the tail length used as the delay
      memcpy(&m_tt1, &m_th1, sizeof(_tracer));
      m_tt1.delay = TailLength;
      memcpy(&m_tt2, &m_th2, sizeof(_tracer));
      m_tt2.delay = TailLength;
   }

   // are we are in inverse mode (tail is the lead) or not?
   if (inverse)
   {
      _tracer	temp;
      // swap the head and tail over
      memcpy(&temp, &m_tt1, sizeof(_tracer));
      memcpy(&m_tt1, &m_th1, sizeof(_tracer));
      memcpy(&m_th1, &temp, sizeof(_tracer));

      memcpy(&temp, &m_tt2, sizeof(_tracer));
      memcpy(&m_tt2, &m_th2, sizeof(_tracer));
      memcpy(&m_th2, &temp, sizeof(_tracer));
   }

   m_timeNextUpdate = g_pplayer->m_time_msec + m_updateRate;
   m_playInProgress = true;
}

bool LightSeq::ProcessTracer(_tracer * const pTracer, const LightState State)
{
   bool rc = false;

   // if this tracer isn't valid, then exit with a finished return code
   if (pTracer->type == eSeqNull)
      return true;

   if (pTracer->delay == 0)
   {
      switch (pTracer->type)
      {
         // process the blink type of effect
      case eSeqBlink:
         switch (pTracer->frameCount--)
         {
         case 2:
            SetAllLightsToState(LightStateOn);
            m_timeNextUpdate = g_pplayer->m_time_msec + m_pauseValue;
            break;

         case 1:
            SetAllLightsToState(LightStateOff);
            m_timeNextUpdate = g_pplayer->m_time_msec + m_pauseValue;
            break;

         case 0:
            rc = true;
            break;
         }
         break;

         // process the random type of effect
      case eSeqRandom: {
         // get the number of elements in this
         const float size = (float)m_pcollection->m_visel.Size();
         // randomly pick n elements and invert their state
         for (int i = 0; i < pTracer->length; ++i)
         {
            // Generates integer random number 0..(size-1)
            const int randomLight = (int)(size * rand_mt_01());
            // get the state of this light
            LightState state = GetElementState(randomLight);
            // invert the state
            if (state == LightStateOn)
            {
               state = LightStateOff;
            }
            else
            {
               state = LightStateOn;
            }
            SetElementToState(randomLight, state);
         }

         pTracer->frameCount -= 1;
         if (pTracer->frameCount == 0)
         {
            // nullify this tracer
            pTracer->type = eSeqNull;
            rc = true;
         }
         break;
      }

                       // process the line tracers
      case eSeqLine: {
         // get the start of the trace line
         float x = pTracer->x;
         float y = pTracer->y;

         for (int i = 0; i < pTracer->length; ++i)
         {
            VerifyAndSetGridElement((int)x, (int)y, State);
            // move to the next position in the line
            x += pTracer->processStepX;
            y += pTracer->processStepY;
         }

         pTracer->frameCount -= 1;
         if (pTracer->frameCount == 0)
         {
            // nullify this tracer
            pTracer->type = eSeqNull;
            rc = true;
         }
         else
         {
            // move to the next grid position
            pTracer->x += pTracer->stepX;
            pTracer->y += pTracer->stepY;
         }
         break;
      }

                     // process the circle type of effect
      case eSeqCircle:
         for (float fi = 0; fi < 360.0f; fi += 0.5f)
         {
            const float angle = (float)(M_PI * 2.0 / 360.0)*fi;
            const float sn = sinf(angle);
            const float cs = cosf(angle);
            const float x = pTracer->x + sn*pTracer->radius;
            const float y = pTracer->y - cs*pTracer->radius;
            VerifyAndSetGridElement((int)x, (int)y, State);
         }
         pTracer->frameCount -= 1;
         if (pTracer->frameCount == 0)
         {
            // nullify this tracer
            pTracer->type = eSeqNull;
            rc = true;
         }
         else
         {
            pTracer->radius += pTracer->stepRadius;
         }
         break;

         // process the radar type of effect
      case eSeqRadar: {
         const float angle = pTracer->angle * (float)(M_PI * 2.0 / 360.0);
         const float sn = sinf(angle);
         const float cs = cosf(angle);
         // we need to process an extra 3 quarter steps for radar effects or it tends to miss lights
         // when the radius get big and setting the step to .250 makes the effect too slow
         const float angle2 = (pTracer->angle + pTracer->stepAngle*0.25f) * (float)(M_PI * 2.0 / 360.0);
         const float sn2 = sinf(angle2);
         const float cs2 = cosf(angle2);
         const float angle3 = (pTracer->angle + pTracer->stepAngle*0.5f) * (float)(M_PI * 2.0 / 360.0);
         const float sn3 = sinf(angle3);
         const float cs3 = cosf(angle3);
         const float angle4 = (pTracer->angle + pTracer->stepAngle*0.75f) * (float)(M_PI * 2.0 / 360.0);
         const float sn4 = sinf(angle4);
         const float cs4 = cosf(angle4);

         if (m_th1.processRadius == 1.0f)
         {
            for (int i = 0; i < pTracer->length; ++i)
            {
               const float fi = (float)i;
               float x = pTracer->x + sn*fi;
               float y = pTracer->y - cs*fi;
               VerifyAndSetGridElement((int)x, (int)y, State);
               x = pTracer->x + sn2*fi;
               y = pTracer->y - cs2*fi;
               VerifyAndSetGridElement((int)x, (int)y, State);
               x = pTracer->x + sn3*fi;
               y = pTracer->y - cs3*fi;
               VerifyAndSetGridElement((int)x, (int)y, State);
               x = pTracer->x + sn4*fi;
               y = pTracer->y - cs4*fi;
               VerifyAndSetGridElement((int)x, (int)y, State);
            }
         }
         else
         {
            for (int i = 0; i < pTracer->length; ++i)
            {
               const float fi = (float)i;
               float x = pTracer->x - sn*fi;
               float y = pTracer->y + cs*fi;
               VerifyAndSetGridElement((int)x, (int)y, State);
               x = pTracer->x - sn2*fi;
               y = pTracer->y + cs2*fi;
               VerifyAndSetGridElement((int)x, (int)y, State);
               x = pTracer->x - sn3*fi;
               y = pTracer->y + cs3*fi;
               VerifyAndSetGridElement((int)x, (int)y, State);
               x = pTracer->x - sn4*fi;
               y = pTracer->y + cs4*fi;
               VerifyAndSetGridElement((int)x, (int)y, State);
            }
         }
         pTracer->frameCount -= 1;
         if (pTracer->frameCount == 0)
         {
            // nullify this tracer
            pTracer->type = eSeqNull;
            rc = true;
         }
         else
         {
            // move to the next angle
            pTracer->angle += pTracer->stepAngle;
            // process any wrap around
            if (pTracer->angle >= 360.0f)
            {
               pTracer->angle -= 360.0f;
            }
            if (pTracer->angle < 0)
            {
               pTracer->angle += 360.0f;
            }
         }
         break;
      }

                      // invalid tracer type (shouldn't happen but hey!)
      default:
         rc = true;
      }
   }
   else
   {
      pTracer->delay -= 1;
   }
   return (rc);
}

void LightSeq::SetAllLightsToState(const LightState State)
{
   if (m_pcollection != NULL)
   {
      const int size = m_pcollection->m_visel.Size();
      for (int i = 0; i < size; ++i)
      {
         SetElementToState(i, State);
      }
   }
}

void LightSeq::SetElementToState(const int index, const LightState State)
{
   if (m_pcollection->m_visel.size() == 0)
      return;

   const ItemTypeEnum type = m_pcollection->m_visel.ElementAt(index)->GetIEditable()->GetItemType();
   if (type == eItemLight)
   {
      Light * const pLight = (Light *)m_pcollection->m_visel.ElementAt(index);
      pLight->setLightStateBypass(State);
   }
}

bool LightSeq::VerifyAndSetGridElement(const int x, const int y, const LightState State)
{
   if (((x >= 0) && (x < m_lightSeqGridWidth)) &&
       ((y >= 0) && (y < m_lightSeqGridHeight)))
   {
      const int gridIndex = (y * m_lightSeqGridWidth) + x;

      int	index = m_pgridData[gridIndex];
      if (index != 0)
      {
         index--;
         SetElementToState(index, State);
      }
      return true;
   }
   else
   {
      return false;
   }
}

LightState LightSeq::GetElementState(const int index)
{
   // just incase the element isn't a light
   LightState rc = LightStateOff;

   if (m_pcollection->m_visel.size() == 0)
      return rc;

   const ItemTypeEnum type = m_pcollection->m_visel.ElementAt(index)->GetIEditable()->GetItemType();
   if (type == eItemLight)
   {
      Light * const pLight = (Light *)m_pcollection->m_visel.ElementAt(index);
      rc = pLight->m_realState;
   }
   return rc;
}


// un-used code but kept

#if 0
// turn on all lights starting in the centre and scrolling (screwing) clockwise out
		case SeqTwirlOutRightOff:
         inverse				= true;
      case SeqTwirlOutRightOn:
         m_th1.type			= eSeqTwirl;
         m_th1.radius		= 0;
         m_th1.stepRadius	= 1;
         m_th1.angle			= 0;
         m_th1.stepAngle		= 1;
         m_th1.x				= m_GridXCenter;
         m_th1.y				= m_GridYCenter;
         m_th1.length		= 30;
         m_th1.frameCount	= (360/m_th1.length)*(m_lightSeqGridHeight/2+m_GridYCenterAdjust);
         break;

         // turn on all lights starting in the centre and scrolling (screwing) anti-clockwise out
      case SeqTwirlOutLeftOff:
         inverse				= true;
      case SeqTwirlOutLeftOn:
         break;

         // turn on all lights starting in the outside and scrolling (screwing) clockwise in
      case SeqTwirlInRightOff:
         inverse				= true;
      case SeqTwirlInRightOn:
         break;

         // turn on all lights starting in the outside and scrolling (screwing) anti-clockwise in
      case SeqTwirlInLeftOff:
         inverse				= true;
      case SeqTwirlInLeftOn:
         break;


         // process the twirl type of effect
      case eSeqTwirl:
         for (int i=0; i<pTracer->length; ++i)
         {
            const float angle = (float)((M_PI*2.0)/360.0)*pTracer->angle;
            const float sn = sinf(angle);
            const float cs = cosf(angle);
            const float x = pTracer->x + sn*pTracer->radius;
            const float y = pTracer->y - cs*pTracer->radius;
            VerifyAndSetGridElement((int)x, (int)y, State);

            // move to the next angle
            pTracer->angle += pTracer->stepAngle;
            // process any wrap around
            if (pTracer->angle >= 360.0f)
            {
               pTracer->angle -= 360.0f;
               pTracer->radius += pTracer->stepRadius;
            }
            if (pTracer->angle < 0)
            {
               pTracer->angle += 360.0f;
               pTracer->radius += pTracer->stepRadius;
            }
         }

         pTracer->frameCount -= 1;
         if (pTracer->frameCount == 0)
         {
            // nullify this tracer
            pTracer->type = eSeqNull;
            rc = true;
         }
         break;
#endif
