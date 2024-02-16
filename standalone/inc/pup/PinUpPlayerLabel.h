#pragma once

extern "C" {
   #include "SDL_ttf.h"
}

class PupLabel {
public:
   PupLabel(TTF_Font* face, float h, LONG c, bool vis) {
      m_ftFace = face;
      m_height = h;
      m_color = c;
      m_visible = vis;

      m_text = "";
      m_x = 0;
      m_y = 0;
      m_xalign = 0;
      m_yalign = 0;
      m_texture = NULL;
   }
   ~PupLabel();

    TTF_Font* m_ftFace;
    float m_height;
    LONG m_color;
    bool m_visible;
    string m_text;
    float m_x;
    float m_y;
    int m_xalign;
    int m_yalign;
    SDL_Texture* m_texture;

    void render(SDL_Renderer* renderer);    
};