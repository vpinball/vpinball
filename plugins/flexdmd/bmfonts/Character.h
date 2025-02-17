#pragma once

#include <SDL3/SDL_surface.h>

class Character final {
public:
   Character() = default;
   ~Character() = default;

   const SDL_Rect& GetBounds() const { return m_bounds; }
   void SetBounds(const SDL_Rect& bounds) { m_bounds = bounds; }
   void SetChannel(int channel) { m_channel = channel; }
   void SetChar(char char_) { m_char = char_; }
   const SDL_Point& GetOffset() { return m_offset; }
   void SetOffset(const SDL_Point& offset) { m_offset = offset; }
   int GetTexturePage() const { return m_texturePage; }
   void SetTexturePage(int texturePage) { m_texturePage = texturePage; }
   int GetXAdvance() const { return m_xadvance; }
   void SetXAdvance(int xadvance) { m_xadvance = xadvance; }

private:
   int m_channel;
   char m_char;
   int m_texturePage;
   int m_xadvance;

   SDL_Rect m_bounds;
   SDL_Point m_offset;
};
