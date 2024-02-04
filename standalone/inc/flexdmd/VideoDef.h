#pragma once

#include "flexdmd_i.h"
#include "BaseDef.h"

class VideoDef : public BaseDef
{
public:
   VideoDef(const string& videoFilename, bool loop);
   ~VideoDef();

   bool operator==(const VideoDef& other) const;
   size_t hash() const;

   const string& GetVideoFilename() const { return m_videoFilename; }
   int GetLoop() const { return m_loop; }
   Scaling GetScaling() const { return m_scaling; }
   void SetScaling(Scaling scaling) { m_scaling = scaling; }
   Alignment GetAlignment() const { return m_alignment; }
   void SetAlignment(Alignment alignment) { m_alignment = alignment; }

private:
   string m_videoFilename;
   int m_loop;
   Scaling m_scaling;
   Alignment m_alignment;
};
