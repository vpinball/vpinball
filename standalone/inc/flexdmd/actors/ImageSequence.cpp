#include "stdafx.h"

#include "ImageSequence.h"

ImageSequence::ImageSequence(FlexDMD* pFlexDMD, AssetManager* pAssetManager, const string& paths, const string& name, int fps, bool loop) : AnimatedActor(pFlexDMD, name)
{
   m_fps = fps;
   SetLoop(loop);

   std::stringstream ss(paths);
   string path;
   while (std::getline(ss, path, '|'))
      m_frames.push_back(new Image(pFlexDMD, pAssetManager, path, ""));

   m_frame = 0;
   SetFrameDuration(1.0f / fps);
   Pack();
}

ImageSequence::~ImageSequence()
{
}

void ImageSequence::OnStageStateChanged()
{
    for (Image* pFrame : m_frames)
       pFrame->SetOnStage(GetOnStage());
}

void ImageSequence::Rewind() 
{
    AnimatedActor::Rewind();
    m_frame = 0;
}

void ImageSequence::ReadNextFrame()
{
    if (m_frame == m_frames.size() - 1)
       SetEndOfAnimation(true);
    else {
       m_frame++;
       SetFrameTime(m_frame * GetFrameDuration());
    }
}

void ImageSequence::Draw(Graphics* graphics)
{
   AnimatedActor::Draw(graphics);
   if (GetVisible()) {
      m_frames[m_frame]->SetScaling(GetScaling());
      m_frames[m_frame]->SetAlignment(GetAlignment());
      m_frames[m_frame]->SetBounds(GetX(), GetY(), GetWidth(), GetHeight());
      m_frames[m_frame]->Draw(graphics);
   }
}