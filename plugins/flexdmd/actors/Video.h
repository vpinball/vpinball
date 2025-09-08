#pragma once

#include "AnimatedActor.h"
#include "resources/AssetManager.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

namespace Flex {

class Video final : public AnimatedActor
{
public:
   ~Video() override;

   static Video* Create(FlexDMD* pFlexDMD, AssetManager* pAssetManager, const string& path, const string& name, bool loop);

   void OnStageStateChanged() override;
   void Rewind() override;
   void ReadNextFrame() override;
   void Draw(Flex::SurfaceGraphics* pGraphics) override;
   float GetLength() const override { return m_length; }

private:
   Video(FlexDMD* pFlexDMD, const string& name);

   std::vector<SDL_Surface*> m_frames;
   SDL_Surface* m_pActiveFrameSurface = nullptr;
   int m_pos = 0;
   float m_frameDuration = 0.0f;
   SwsContext* m_pVideoConversionContext = nullptr;
   AVFormatContext* m_pFormatContext = nullptr;
   AVCodecContext* m_pCodecContext = nullptr;
   int m_videoStreamIndex = -1;
   float m_length;
};

}