// license:GPLv3+

#pragma once

struct FontDesc;
class PinTable;
typedef struct TTF_Font TTF_Font;

// Cross platform font resolution shared by parts that rasterize table text (Decal, Textbox).
namespace TextRenderer
{
// Initializes SDL_ttf and resolves the closest matching font for a VPX font descriptor, in order:
// table-adjacent '<FontName>-<Style>.ttf' files, table-embedded fonts, installed Windows fonts
// (when built with the Win32 SDK), then the bundled fallback font.
// The returned font must be released with TTF_CloseFont, followed by TTF_Quit to balance the TTF_Init done here.
TTF_Font *LoadFont(const FontDesc &font, const PinTable *table);
}
