#include "stdafx.h"

#include "Video.h"

Video::Video(FlexDMD* pFlexDMD, const string& name) : AnimatedActor(pFlexDMD, name)
{
}

Video* Video::Create(FlexDMD* pFlexDMD, const string& path, const string& name, bool loop)
{
   Video* pVideo = new Video(pFlexDMD, name);
   pVideo->SetVisible(true);
   pVideo->m_seek = -1;
   pVideo->SetLoop(loop);

   PLOGW.printf("Video not supported %s", path.c_str());

   return pVideo;
}

Video::~Video()
{
}

void Video::OnStageStateChanged()
{
}

STDMETHODIMP Video::Seek(single posInSeconds)
{
   m_seek = posInSeconds;
   SetTime(posInSeconds);
   //if (_audioReader != null)
   //{
   //   _audioReader.CurrentTime = TimeSpan.FromSeconds(position);
   //}
   //if (_videoReader != null)
   //{
      ReadNextFrame();
      SetTime(GetFrameTime());
   //}

   return S_OK;
}

void Video::Rewind()
{
   AnimatedActor::Rewind();
   Seek(0);
}

void Video::ReadNextFrame()
{
}

void Video::Draw(VP::SurfaceGraphics* pGraphics)
{
   if (GetVisible()) {
      float w = 0;
      float h = 0;
      Layout::Scale(GetScaling(), GetPrefWidth(), GetPrefHeight(), GetWidth(), GetHeight(), w, h);
      float x = 0;
      float y = 0;
      Layout::Align(GetAlignment(), w, h, GetWidth(), GetHeight(), x, y);
      SDL_Rect rect = { (int)(GetX() + x), (int)(GetY() + y), (int)w, (int)h };
      //pGraphics->DrawImage(m_pFrame, NULL, &rect);
   }
}