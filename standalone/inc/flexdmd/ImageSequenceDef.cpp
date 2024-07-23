#include "stdafx.h"

#include "ImageSequenceDef.h"

ImageSequenceDef::ImageSequenceDef(const string& images, int fps, bool loop)
{
   m_images = images;
   m_fps = fps;
   m_loop = loop;

   m_scaling = Scaling_Stretch;
   m_alignment = Alignment_Center;
}

ImageSequenceDef::~ImageSequenceDef()
{
}

bool ImageSequenceDef::operator==(const ImageSequenceDef& other) const
{
   return m_images == other.m_images &&
      m_fps == other.m_fps &&
      m_loop == other.m_loop &&
      m_scaling == other.m_scaling &&
      m_alignment == other.m_alignment;
}

size_t ImageSequenceDef::hash() const
{
   size_t hashCode = -2035125405;
   hashCode = hashCode * -1521134295 + std::hash<string>{}(m_images);
   hashCode = hashCode * -1521134295 + m_fps;
   hashCode = hashCode * -1521134295 + m_loop;
   hashCode = hashCode * -1521134295 + m_scaling;
   hashCode = hashCode * -1521134295 + m_alignment;
   return hashCode;
}