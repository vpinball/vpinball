#include "stdafx.h"
#include "RenderState.h"

#define RENDER_STATE(name, bitpos, bitsize)                                                                                                                                                  \
   constexpr uint32_t RENDER_STATE_SHIFT_##name = bitpos;                                                                                                                                        \
   constexpr uint32_t RENDER_STATE_MASK_##name = ((0x00000001u << (bitsize)) - 1) << (bitpos);                                                                                                   \
   constexpr uint32_t RENDER_STATE_CLEAR_MASK_##name = ~(((0x00000001u << (bitsize)) - 1) << (bitpos));
// These definition must be copy/pasted to RenderState.h/cpp when modified to keep the implementation in sync
RENDER_STATE(ALPHABLENDENABLE, 0, 1) // RS_FALSE or RS_TRUE
RENDER_STATE(ZENABLE, 1, 1) // RS_FALSE or RS_TRUE
RENDER_STATE(BLENDOP, 2, 2) // Operation from BLENDOP_MAX, BLENDOP_ADD, BLENDOP_SUB, BLENDOP_REVSUBTRACT
RENDER_STATE(CLIPPLANEENABLE, 4, 1) // RS_TRUE or RS_FALSE
RENDER_STATE(CULLMODE, 5, 2) // CULL_NONE, CULL_CW, CULL_CCW
RENDER_STATE(DESTBLEND, 7, 3) // ZERO, ONE, SRC_ALPHA, DST_ALPHA, INVSRC_ALPHA, INVSRC_COLOR
RENDER_STATE(SRCBLEND, 10, 3) // ZERO, ONE, SRC_ALPHA, DST_ALPHA, INVSRC_ALPHA, INVSRC_COLOR
RENDER_STATE(ZFUNC, 13, 3) // Operation from Z_ALWAYS, Z_LESS, Z_LESSEQUAL, Z_GREATER, Z_GREATEREQUAL
RENDER_STATE(ZWRITEENABLE, 16, 1) // RS_FALSE or RS_TRUE
RENDER_STATE(COLORWRITEENABLE, 17, 4) // RGBA mask (4 bits)
#undef RENDER_STATE

#define RENDER_STATE(name, bitpos, bitsize) { RENDER_STATE_SHIFT_##name, RENDER_STATE_MASK_##name, RENDER_STATE_CLEAR_MASK_##name },
// These definitions must be copy/pasted to RenderState.h/cpp when modified to keep the implementation in sync
constexpr RenderState::RenderStateMask RenderState::render_state_masks[RENDERSTATE_COUNT] {
   RENDER_STATE(ALPHABLENDENABLE, 0, 1) // RS_FALSE or RS_TRUE
   RENDER_STATE(ZENABLE, 1, 1) // RS_FALSE or RS_TRUE
   RENDER_STATE(BLENDOP, 2, 2) // Operation from BLENDOP_MAX, BLENDOP_ADD, BLENDOP_SUB, BLENDOP_REVSUBTRACT
   RENDER_STATE(CLIPPLANEENABLE, 4, 1) // RS_TRUE or RS_FALSE
   RENDER_STATE(CULLMODE, 5, 2) // CULL_NONE, CULL_CW, CULL_CCW
   RENDER_STATE(DESTBLEND, 7, 3) // ZERO, ONE, SRC_ALPHA, DST_ALPHA, INVSRC_ALPHA, INVSRC_COLOR
   RENDER_STATE(SRCBLEND, 10, 3) // ZERO, ONE, SRC_ALPHA, DST_ALPHA, INVSRC_ALPHA, INVSRC_COLOR
   RENDER_STATE(ZFUNC, 13, 3) // Operation from Z_ALWAYS, Z_LESS, Z_LESSEQUAL, Z_GREATER, Z_GREATEREQUAL
   RENDER_STATE(ZWRITEENABLE, 16, 1) // RS_FALSE or RS_TRUE
   RENDER_STATE(COLORWRITEENABLE, 17, 4) // RGBA mask (4 bits)
};
#undef RENDER_STATE

RenderState::RenderState()
   : m_depthBias(0.f),
     m_state(0x001f5146)
{
   // Default render state is:
   // Blend: { _  A  SA   RSA } Depth: { Z  <=  ZW } Clip: _ Cull: CCW Mask: F
   #if 0
   // Kept to easily check default state constant
   SetRenderState(ALPHABLENDENABLE, RS_FALSE);
   SetRenderState(ZENABLE, RS_TRUE);
   SetRenderState(BLENDOP, BLENDOP_ADD);
   SetRenderState(CLIPPLANEENABLE, RS_FALSE);
   SetRenderState(CULLMODE, CULL_CCW);
   SetRenderState(DESTBLEND, SRC_ALPHA);
   SetRenderState(SRCBLEND, INVSRC_ALPHA);
   SetRenderState(ZFUNC, Z_LESSEQUAL);
   SetRenderState(ZWRITEENABLE, RS_TRUE);
   SetRenderState(COLORWRITEENABLE, RGBMASK_RGBA);
   OutputDebugString(GetLog().c_str());
   OutputDebugString("\n");
   #endif
}

RenderState::RenderStateValue RenderState::GetRenderState(const RenderStates p1) const
{
   return (RenderState::RenderStateValue) ((m_state & render_state_masks[p1].mask) >> render_state_masks[p1].shift);
}

void RenderState::SetRenderState(const RenderStates p1, const RenderStateValue p2)
{
   // Value must fit inside the render state bit mask
   assert(((p2 << render_state_masks[p1].shift) & render_state_masks[p1].clear_mask) == 0);
   m_state &= render_state_masks[p1].clear_mask;
   m_state |= p2 << render_state_masks[p1].shift;
}

void RenderState::SetRenderStateDepthBias(float bias)
{
   m_depthBias = bias;
}

bool RenderState::IsOpaque() const
{
   return (m_state & RENDER_STATE_MASK_ALPHABLENDENABLE) == 0 // Mesh does not use framebuffer color blending
      && (m_state & RENDER_STATE_MASK_ZENABLE) != 0 // Depth testing is enabled
      && ((m_state & RENDER_STATE_MASK_ZFUNC) >> RENDER_STATE_SHIFT_ZFUNC) == Z_LESSEQUAL ; // Depth testing use default depth camparison
}

const string RenderState::GetLog() const
{
   const auto blend = (m_state & RENDER_STATE_MASK_ALPHABLENDENABLE) != 0;
   const auto z_test = (m_state & RENDER_STATE_MASK_ZENABLE) != 0;
   const auto blend_op = (m_state & RENDER_STATE_MASK_BLENDOP) >> RENDER_STATE_SHIFT_BLENDOP;
   const auto clip_plane = (m_state & RENDER_STATE_MASK_CLIPPLANEENABLE) != 0;
   const auto cull_mode = (m_state & RENDER_STATE_MASK_CULLMODE) >> RENDER_STATE_SHIFT_CULLMODE;
   const auto blend_dest = (m_state & RENDER_STATE_MASK_DESTBLEND) >> RENDER_STATE_SHIFT_DESTBLEND;
   const auto blend_src = (m_state & RENDER_STATE_MASK_SRCBLEND) >> RENDER_STATE_SHIFT_SRCBLEND;
   const auto z_func = (m_state & RENDER_STATE_MASK_ZFUNC) >> RENDER_STATE_SHIFT_ZFUNC;
   const auto z_write = (m_state & RENDER_STATE_MASK_ZWRITEENABLE) != 0;
   const auto color_write = (m_state & RENDER_STATE_MASK_COLORWRITEENABLE) >> RENDER_STATE_SHIFT_COLORWRITEENABLE;
   static const string cull_modes[] = { " ___ "s, " CW  "s, " CCW "s };
   static const string functions[] = { " __ "s, " <  "s, " <= "s, " >  "s, " >= "s };
   static const string blend_modes[] = { " M "s, " A "s, " R "s };
   static const string blend_functions[] = { "  0  "s, "  1  "s, " SA  "s, " DA  "s, " RSA "s, " RSC "s };
   string s { "Blend: {"s };
   s.append(blend ? " B " : " _ ");
   s.append(blend ? blend_modes[blend_op] : " _ ");
   s.append(blend ? blend_functions[blend_dest] : " ___ ");
   s.append(blend ? blend_functions[blend_src] : " ___ ");
   s.append("} Depth: {");
   s.append(z_test ? " Z " : " _ ");
   s.append(z_test ? functions[z_func] : " __ ");
   s.append(z_write ? " ZW " : " __ ");
   s.append("} Clip:");
   s.append(clip_plane ? " C " : " _ ");
   s.append("Cull:");
   s.append(cull_modes[cull_mode]);
   s.append("Mask: ");
   s += (char)((color_write < 10 ? 48 : 55) + color_write);
   return s;
}


void RenderState::Apply(RenderDevice* device)
{
#if defined(ENABLE_SDL) // OpenGL
   constexpr int cull_modes[] = { 0, GL_CW, GL_CCW };
   constexpr int functions[] = { GL_ALWAYS, GL_LESS, GL_LEQUAL, GL_GREATER, GL_GEQUAL };
   constexpr int blend_modes[] = { GL_MAX, GL_FUNC_ADD, GL_FUNC_REVERSE_SUBTRACT };
   constexpr int blend_functions[] = { GL_ZERO, GL_ONE, GL_SRC_ALPHA, GL_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_SRC_COLOR };
#else // DirectX 9
   constexpr int cull_modes[] = { D3DCULL_NONE, D3DCULL_CW, D3DCULL_CCW };
   constexpr int functions[] = { D3DCMP_ALWAYS, D3DCMP_LESS, D3DCMP_LESSEQUAL, D3DCMP_GREATER, D3DCMP_GREATEREQUAL };
   constexpr int blend_modes[] = { D3DBLENDOP_MAX, D3DBLENDOP_ADD, D3DBLENDOP_REVSUBTRACT };
   constexpr int blend_functions[] = { D3DBLEND_ZERO, D3DBLEND_ONE, D3DBLEND_SRCALPHA, D3DBLEND_DESTALPHA, D3DBLEND_INVSRCALPHA, D3DBLEND_INVSRCCOLOR };
   IDirect3DDevice9* const d3dDevice = device->GetCoreDevice();
#endif

   int val;
   RenderState& active_state = device->GetActiveRenderState();
   const unsigned previous_state = active_state.m_state;
   unsigned new_state = m_state;
   unsigned renderstate_mask = previous_state ^ m_state; // Identify differences
   while (renderstate_mask)
   {
      // Iterate over set bits, starting from the least significant ones
      unsigned next_difference = renderstate_mask & (unsigned)(-(int)renderstate_mask);
      switch (next_difference)
      {
      case RENDER_STATE_MASK_ALPHABLENDENABLE:
         renderstate_mask &= RENDER_STATE_CLEAR_MASK_ALPHABLENDENABLE;
         val = m_state & RENDER_STATE_MASK_ALPHABLENDENABLE;
         #ifdef ENABLE_SDL
         if (val) glEnable(GL_BLEND); else glDisable(GL_BLEND);
         #else
         CHECKD3D(d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, val ? TRUE : FALSE));
         #endif
         break;

      case RENDER_STATE_MASK_ZENABLE:
         renderstate_mask &= RENDER_STATE_CLEAR_MASK_ZENABLE;
         val = m_state & RENDER_STATE_MASK_ZENABLE;
         #ifdef ENABLE_SDL
         if (val) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
         #else
         CHECKD3D(d3dDevice->SetRenderState(D3DRS_ZENABLE, val ? TRUE : FALSE));
         #endif
         break;

      // case RENDER_STATE_MASK_BLENDOP:
      case 0x00000004u:
      case 0x00000008u:
         renderstate_mask &= RENDER_STATE_CLEAR_MASK_BLENDOP;
         if (m_state & RENDER_STATE_MASK_ALPHABLENDENABLE)
         { // Only apply if blending is enabled
            val = (m_state & RENDER_STATE_MASK_BLENDOP) >> RENDER_STATE_SHIFT_BLENDOP;
            #ifdef ENABLE_SDL
            glBlendEquation(blend_modes[val]);
            #else
            CHECKD3D(d3dDevice->SetRenderState(D3DRS_BLENDOP, blend_modes[val]));
            #endif
         }
         else
         {
            new_state &= RENDER_STATE_CLEAR_MASK_BLENDOP;
            new_state |= previous_state & RENDER_STATE_MASK_BLENDOP;
         }
         break;

      case RENDER_STATE_MASK_CLIPPLANEENABLE:
         renderstate_mask &= RENDER_STATE_CLEAR_MASK_CLIPPLANEENABLE;
         val = m_state & RENDER_STATE_MASK_CLIPPLANEENABLE;
         #ifdef ENABLE_SDL
#ifndef __OPENGLES__
         if (val) glEnable(GL_CLIP_DISTANCE0); else glDisable(GL_CLIP_DISTANCE0);
#endif
         #else
         CHECKD3D(d3dDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, val ? 1 : 0));
         #endif
         break;

      // case RENDER_STATE_MASK_CULLMODE:
      case 0x00000020u:
      case 0x00000040u:
         renderstate_mask &= RENDER_STATE_CLEAR_MASK_CULLMODE;
         val = (m_state & RENDER_STATE_MASK_CULLMODE) >> RENDER_STATE_SHIFT_CULLMODE;
         #ifdef ENABLE_SDL
         if (val == 0)
            glDisable(GL_CULL_FACE);
         else
         {
            glEnable(GL_CULL_FACE);
            glFrontFace(cull_modes[val]);
            glCullFace(GL_FRONT);
         }
         #else
         CHECKD3D(d3dDevice->SetRenderState(D3DRS_CULLMODE, cull_modes[val]));
         #endif
         break;

      // case DESTBLEND:
      case 0x00000080u:
      case 0x00000100u:
      case 0x00000200u:
         renderstate_mask &= RENDER_STATE_CLEAR_MASK_DESTBLEND;
         if (m_state & RENDER_STATE_MASK_ALPHABLENDENABLE)
         { // Only apply if blending is enabled
            #ifdef ENABLE_SDL
            {
               renderstate_mask &= RENDER_STATE_CLEAR_MASK_SRCBLEND; // Both are performed together for OpenGL
               int src = (m_state & RENDER_STATE_MASK_SRCBLEND) >> RENDER_STATE_SHIFT_SRCBLEND;
               int dst = (m_state & RENDER_STATE_MASK_DESTBLEND) >> RENDER_STATE_SHIFT_DESTBLEND;
               glBlendFunc(blend_functions[src], blend_functions[dst]);
            }
            #else
            val = (m_state & RENDER_STATE_MASK_DESTBLEND) >> RENDER_STATE_SHIFT_DESTBLEND;
            CHECKD3D(d3dDevice->SetRenderState(D3DRS_DESTBLEND, blend_functions[val]));
            #endif
         }
         else
         {
            new_state &= RENDER_STATE_CLEAR_MASK_DESTBLEND;
            new_state |= previous_state & RENDER_STATE_MASK_DESTBLEND;
         }
         break;

      // case SRCBLEND:
      case 0x00000400u:
      case 0x00000800u:
      case 0x00001000u:
         renderstate_mask &= RENDER_STATE_CLEAR_MASK_SRCBLEND;
         if (m_state & RENDER_STATE_MASK_ALPHABLENDENABLE)
         { // Only apply if blending is enabled
            #ifdef ENABLE_SDL
            {
               const int src = (m_state & RENDER_STATE_MASK_SRCBLEND) >> RENDER_STATE_SHIFT_SRCBLEND;
               const int dst = (m_state & RENDER_STATE_MASK_DESTBLEND) >> RENDER_STATE_SHIFT_DESTBLEND;
               glBlendFunc(blend_functions[src], blend_functions[dst]);
            }
            #else
            val = (m_state & RENDER_STATE_MASK_SRCBLEND) >> RENDER_STATE_SHIFT_SRCBLEND;
            CHECKD3D(d3dDevice->SetRenderState(D3DRS_SRCBLEND, blend_functions[val]));
            #endif
         }
         else
         {
            new_state &= RENDER_STATE_CLEAR_MASK_SRCBLEND;
            new_state |= previous_state & RENDER_STATE_MASK_SRCBLEND;
         }
         break;

      // ZFUNC
      case 0x00002000u:
      case 0x00004000u:
      case 0x00008000u:
         renderstate_mask &= RENDER_STATE_CLEAR_MASK_ZFUNC;
         if (m_state & RENDER_STATE_MASK_ZENABLE)
         { // Only apply if depth testing is enabled
            val = (m_state & RENDER_STATE_MASK_ZFUNC) >> RENDER_STATE_SHIFT_ZFUNC;
            #ifdef ENABLE_SDL
            glDepthFunc(functions[val]);
            #else
            CHECKD3D(d3dDevice->SetRenderState(D3DRS_ZFUNC, functions[val]));
            #endif
         }
         else
         {
            new_state &= RENDER_STATE_CLEAR_MASK_ZFUNC;
            new_state |= previous_state & RENDER_STATE_MASK_ZFUNC;
         }
         break;

      case RENDER_STATE_MASK_ZWRITEENABLE:
         renderstate_mask &= RENDER_STATE_CLEAR_MASK_ZWRITEENABLE;
         val = m_state & RENDER_STATE_MASK_ZWRITEENABLE;
         #ifdef ENABLE_SDL
         glDepthMask(val ? GL_TRUE : GL_FALSE);
         #else
         CHECKD3D(d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, val ? TRUE : FALSE));
         #endif
         break;

      // COLORWRITEENABLE
      case 0x00020000u:
      case 0x00040000u:
      case 0x00080000u:
      case 0x00100000u:
         renderstate_mask &= RENDER_STATE_CLEAR_MASK_COLORWRITEENABLE;
         val = (m_state & RENDER_STATE_MASK_COLORWRITEENABLE) >> RENDER_STATE_SHIFT_COLORWRITEENABLE;
         #ifdef ENABLE_SDL
         glColorMask((val & 1) ? GL_TRUE : GL_FALSE, (val & 2) ? GL_TRUE : GL_FALSE, (val & 4) ? GL_TRUE : GL_FALSE, (val & 8) ? GL_TRUE : GL_FALSE);
         #else
         CHECKD3D(d3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, val));
         #endif
         break;

      default: // Invalid state mask
         assert(false);
      }
      device->m_curStateChanges++;
   }
   active_state.m_state = new_state;

   if (active_state.m_depthBias != m_depthBias)
   {
      active_state.m_depthBias = m_depthBias;
      device->m_curStateChanges++;
      #ifdef ENABLE_SDL
      if (m_depthBias == 0.0f)
         glDisable(GL_POLYGON_OFFSET_FILL);
      else
      {
         glEnable(GL_POLYGON_OFFSET_FILL);
         glPolygonOffset(0.0f, m_depthBias);
      }
      #else
      CHECKD3D(d3dDevice->SetRenderState(D3DRS_DEPTHBIAS, float_as_uint(m_depthBias * BASEDEPTHBIAS)));
      #endif
   }
}
