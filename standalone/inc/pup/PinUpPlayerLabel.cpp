#include "stdafx.h"
#include "PinUpPlayerLabel.h"

PupLabel::~PupLabel()
{
    if (m_texture) SDL_DestroyTexture(m_texture);
}

void PupLabel::render(SDL_Renderer* renderer) 
{
    if (m_texture) SDL_DestroyTexture(m_texture);
    if (m_text.empty()) return;

    TTF_SetFontSize(m_ftFace, m_height);

    SDL_Surface* text_surface;
    SDL_Color sdlColor = { GetRValue(m_color), GetGValue(m_color), GetBValue(m_color) };
    if(!(text_surface=TTF_RenderUTF8_Blended(m_ftFace, m_text.c_str(), sdlColor))) {
        PLOGW << "Error rendering label " << m_text << ". " << TTF_GetError();
        return;
    } else {
        m_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
        SDL_FreeSurface(text_surface);
    }

    // Debug Info
    // int w, h;
    // SDL_QueryTexture(m_texture, NULL, NULL, &w, &h);
    // PLOGI << "## Rendered label " << m_text << " " << w << "x" << h << " " << " font size: " << m_height;
}
