#pragma once

#ifdef _MSC_VER
#define SERUM_API extern "C" __declspec(dllexport)
#else
#define SERUM_API extern "C" __attribute__((visibility("default")))
#endif

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#include "serum.h"

/** @brief Load a Serum file.
 *
 *  @param altcolorpath: path to the altcolor directory (e.g. "C:\\visual pinball\\vpinmame\\altcolor\\")
 *  @param romname: name of the rom (e.g. "afm_113b")
 *  @param flags (only needed for v2 files): this can be a combination of: FLAG_REQUEST_32P_FRAMES (if you want the 32-pixel-high frame to be returned if available) / FLAG_REQUEST_64P_FRAMES (same for 64-pixel-high frame) / FLAG_REQUEST_FILL_MODIFIED_ELEMENTS (Serum_Rotate() fills the modifiedelementsXX buffers to know which points have changed in the rotation)
 * 
 *  @return A pointer to the Serum_Frame_Struc as described in the serum.h file (to keep and read all along the use of the loaded Serum)
 */
SERUM_API Serum_Frame_Struc* Serum_Load(const char* const altcolorpath, const char* const romname, uint8_t flags);
SERUM_API void Serum_SetIgnoreUnknownFramesTimeout(uint16_t milliseconds);

SERUM_API void Serum_SetMaximumUnknownFramesToSkip(uint8_t maximum);

SERUM_API void Serum_SetStandardPalette(const uint8_t* palette, const int bitDepth);

/** @brief Release the content and memory of the loaded Serum file.
*/
SERUM_API void Serum_Dispose(void);

/** @brief Colorize a frame and set the values in the Serum_Frame_Struc (corresponding to the pointer returned at Serum_Load() time)
* 
* @param Frame: a buffer to the rom uncolorized frame containing width*height bytes with [0,3] or [0,15] values (according to the ROM)
* 
* @return Either return IDENTIFY_NO_FRAME if no frame in the Serum file matches or, if a frame was identified, 0 if it has no rotation or a value giving the time in milliseconds before the first color rotation
*/
SERUM_API uint32_t Serum_Colorize(uint8_t* frame);

/** @brief Perform the color rotations of the current frame. For v1, it modifies "palette", for v2, it modifies "frame32" and/or "frame64"
* 
* @return The time in milliseconds before the next rotation in the low word. If there was a rotation in the 32P frame, the first bit of the high word is set (0x10000) and if there was a rotation in the 64P frame, the second bit of the high word is set (0x20000)
*/
SERUM_API uint32_t Serum_Rotate(void);

SERUM_API void Serum_DisableColorization(void);

SERUM_API void Serum_EnableColorization(void);

/** @brief Get the full version of this library
* 
* @return A string formatted "major.minor.patch"
*/
SERUM_API const char* Serum_GetVersion(void);

/** @brief Get the short version of this library
*
* @return A string formatted "major.minor"
*/
SERUM_API const char* Serum_GetMinorVersion(void);

