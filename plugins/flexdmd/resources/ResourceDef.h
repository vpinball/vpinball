#pragma once

#include "actors/Layout.h"
#include "VPXPlugin.h"

#include <string>
using std::string;

class BaseDef
{
public:
   virtual ~BaseDef() = default;

   PSC_IMPLEMENT_REFCOUNT()
};


class FontDef final : public BaseDef
{
public:
   FontDef(const string& path, uint32_t tint, uint32_t borderTint, int borderSize = 0)
      : m_szPath(path)
      , m_tint(tint)
      , m_borderTint(borderTint)
      , m_borderSize(borderSize)
   {}
   ~FontDef() override {}

   bool operator==(const FontDef& other) const
   {
      return m_szPath == other.m_szPath &&
         m_tint == other.m_tint &&
         m_borderTint == other.m_borderTint &&
         m_borderSize == other.m_borderSize ;
   }

   size_t hash() const 
   {
      size_t hashCode = -1876634251;
      hashCode = hashCode * -1521134295 + std::hash<string>{}(m_szPath);
      hashCode = hashCode * -1521134295 + m_tint;
      hashCode = hashCode * -1521134295 + m_borderTint;
      hashCode = hashCode * -1521134295 + m_borderSize;
      return hashCode;
   }

   const string& GetPath() const { return m_szPath; }
   uint32_t GetTint() const { return m_tint; }
   uint32_t GetBorderTint() const { return m_borderTint; }
   int GetBorderSize() const { return m_borderSize; }

private:
   const string m_szPath;
   const uint32_t m_tint;
   const uint32_t m_borderTint;
   const int m_borderSize;
};


class VideoDef final : public BaseDef
{
public:
   VideoDef(const string& videoFilename, bool loop)
      : m_videoFilename(videoFilename)
      , m_loop(loop)
      , m_scaling(Scaling_Stretch)
      , m_alignment(Alignment_Center)
      {}
   ~VideoDef() override {}

   bool operator==(const VideoDef& other) const
   {
      return m_videoFilename == other.m_videoFilename &&
         m_loop == other.m_loop && 
         m_scaling == other.m_scaling &&
         m_alignment == other.m_alignment;
   }

   size_t hash() const
   {
       size_t hashCode = -96768724;
       hashCode = hashCode * -1521134295 + std::hash<string>{}(m_videoFilename);
       hashCode = hashCode * -1521134295 + m_loop;
       hashCode = hashCode * -1521134295 + m_scaling;
       hashCode = hashCode * -1521134295 + m_alignment;
       return hashCode;
   }

   const string& GetVideoFilename() const { return m_videoFilename; }
   int GetLoop() const { return m_loop; }
   Scaling GetScaling() const { return m_scaling; }
   Alignment GetAlignment() const { return m_alignment; }

   // FIXME Warning, this object should be a const as it is used as a resource Id
   void SetScaling(Scaling scaling) { m_scaling = scaling; }
   void SetAlignment(Alignment alignment) { m_alignment = alignment; }

private:
   const string m_videoFilename;
   const int m_loop;
   Scaling m_scaling;
   Alignment m_alignment;
};


class ImageSequenceDef final : public BaseDef
{
public:
   ImageSequenceDef(const string& images, int fps, bool loop)
      : m_images(images)
      , m_fps(fps)
      , m_loop(loop)
      , m_scaling(Scaling_Stretch)
      , m_alignment(Alignment_Center)
      {}
   ~ImageSequenceDef() override {}

   bool operator==(const ImageSequenceDef& other) const
   {
      return m_images == other.m_images &&
         m_fps == other.m_fps &&
         m_loop == other.m_loop &&
         m_scaling == other.m_scaling &&
         m_alignment == other.m_alignment;
   }

   size_t hash() const
   {
      size_t hashCode = -2035125405;
      hashCode = hashCode * -1521134295 + std::hash<string>{}(m_images);
      hashCode = hashCode * -1521134295 + m_fps;
      hashCode = hashCode * -1521134295 + m_loop;
      hashCode = hashCode * -1521134295 + m_scaling;
      hashCode = hashCode * -1521134295 + m_alignment;
      return hashCode;
   }

   const string& GetImages() const { return m_images; }
   int GetFPS() const { return m_fps; }
   bool GetLoop() const { return m_loop; }
   Scaling GetScaling() const { return m_scaling; }
   Alignment GetAlignment() const { return m_alignment; }

private:
   const string m_images;
   const int m_fps;
   const int m_loop;
   const Scaling m_scaling;
   const Alignment m_alignment;
};
