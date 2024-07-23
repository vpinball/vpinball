#include "stdafx.h"

#include "ImageSequence.h"

ImageSequence::ImageSequence(FlexDMD* pFlexDMD, const string& name) : AnimatedActor(pFlexDMD, name)
{
}

ImageSequence* ImageSequence::Create(FlexDMD* pFlexDMD, AssetManager* pAssetManager, const string& paths, const string& name, int fps, bool loop)
{
   vector<Image*> frames;
   std::stringstream ss(paths);
   string path;
   while (std::getline(ss, path, '|')) {
      Image* pImage = Image::Create(pFlexDMD, pAssetManager, path, "");
      if (pImage)
         frames.push_back(pImage);
      else {
         for (Image* pFrame : frames)
            delete pFrame;
         return NULL;
      }
   }

   ImageSequence* pImageSequence = new ImageSequence(pFlexDMD, name);
   pImageSequence->m_fps = fps;
   pImageSequence->SetLoop(loop);
   pImageSequence->m_frames = frames;
   pImageSequence->m_frame = 0;
   pImageSequence->SetFrameDuration(1.0f / fps);
   pImageSequence->Pack();

   return pImageSequence;
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

void ImageSequence::Draw(VP::SurfaceGraphics* pGraphics)
{
   AnimatedActor::Draw(pGraphics);
   if (GetVisible()) {
      m_frames[m_frame]->SetScaling(GetScaling());
      m_frames[m_frame]->SetAlignment(GetAlignment());
      m_frames[m_frame]->SetBounds(GetX(), GetY(), GetWidth(), GetHeight());
      m_frames[m_frame]->Draw(pGraphics);
   }
}