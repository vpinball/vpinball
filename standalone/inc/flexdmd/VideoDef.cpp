#include "stdafx.h"
#include "VideoDef.h"

VideoDef::VideoDef(const string& videoFilename, bool loop)
{
   m_videoFilename = videoFilename;
   m_loop = loop;

   m_scaling = Scaling_Stretch;
   m_alignment = Alignment_Center;
}

VideoDef::~VideoDef()
{
}

bool VideoDef::operator==(const VideoDef& other) const
{
   return m_videoFilename == other.m_videoFilename &&
      m_loop == other.m_loop && 
      m_scaling == other.m_scaling &&
      m_alignment == other.m_alignment;
}

size_t VideoDef::hash() const
{
    size_t hashCode = -96768724;
    hashCode = hashCode * -1521134295 + std::hash<string>{}(m_videoFilename);
    hashCode = hashCode * -1521134295 + m_loop;
    hashCode = hashCode * -1521134295 + m_scaling;
    hashCode = hashCode * -1521134295 + m_alignment;
    return hashCode;
}