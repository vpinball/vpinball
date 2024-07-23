#pragma once

#include "BaseDef.h"

class ImageSequenceDef : public BaseDef
{
public:
   ImageSequenceDef(const string& images, int fps, bool loop);
   ~ImageSequenceDef();

   bool operator==(const ImageSequenceDef& other) const;
   size_t hash() const;

   const string& GetImages() const { return m_images; }
   int GetFPS() const { return m_fps; }
   bool GetLoop() const { return m_loop; }
   Scaling GetScaling() const { return m_scaling; }
   Alignment GetAlignment() const { return m_alignment; }

private:
   string m_images;
   int m_fps;
   int m_loop;
   Scaling m_scaling;
   Alignment m_alignment;
};
