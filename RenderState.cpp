#include "stdafx.h"
#include "RenderState.h"

#define RENDER_STATE(name, bitpos, bitsize) { RENDER_STATE_SHIFT_##name, RENDER_STATE_MASK_##name, RENDER_STATE_CLEAR_MASK_##name },
// These definition must be copy/pasted to RenderState.h/cpp when modified to keep the implementation in sync
const RenderState::RenderStateMask RenderState::render_state_masks[RENDERSTATE_COUNT] {
   RENDER_STATE(ALPHABLENDENABLE, 0, 1) // RS_FALSE or RS_TRUE
   RENDER_STATE(ZENABLE, 1, 1) // RS_FALSE or RS_TRUE
   RENDER_STATE(BLENDOP, 2, 2) // Operation from BLENDOP_MAX, BLENDOP_ADD, BLENDOP_SUB, BLENDOP_REVSUBTRACT
   RENDER_STATE(CLIPPLANEENABLE, 4, 1) // PLANE0 or 0 (for disable)
   RENDER_STATE(CULLMODE, 5, 2) // CULL_NONE, CULL_CW, CULL_CCW
   RENDER_STATE(DESTBLEND, 7, 3) // ZERO, ONE, SRC_ALPHA, DST_ALPHA, INVSRC_ALPHA, INVSRC_COLOR
   RENDER_STATE(SRCBLEND, 10, 3) // ZERO, ONE, SRC_ALPHA, DST_ALPHA, INVSRC_ALPHA, INVSRC_COLOR
   RENDER_STATE(ZFUNC, 13, 3) // Operation from Z_ALWAYS, Z_LESS, Z_LESSEQUAL, Z_GREATER, Z_GREATEREQUAL
   RENDER_STATE(ZWRITEENABLE, 16, 1) // RS_FALSE or RS_TRUE
   RENDER_STATE(COLORWRITEENABLE, 17, 4) // RGBA mask (4 bits)
};
#undef RENDER_STATE


void RenderState::SetRenderState(const RenderStates p1, const RenderStateValue p2)
{
   assert(((p2 << render_state_masks[p1].shift) & ~render_state_masks[p1].mask) == 0);
   m_state &= render_state_masks[p1].clear_mask;
   m_state |= p2 << render_state_masks[p1].shift;
}

void RenderState::SetRenderStateClipPlane(const bool enabled)
{
   SetRenderState(CLIPPLANEENABLE, (RenderStateValue)(enabled ? 1 : 0));
}

void RenderState::SetRenderStateCulling(RenderStateValue cull)
{
   if (g_pplayer && (g_pplayer->m_ptable->m_tblMirrorEnabled ^ g_pplayer->IsRenderPass(Player::REFLECTION_PASS)))
   {
      if (cull == CULL_CCW)
         cull = CULL_CW;
      else if (cull == CULL_CW)
         cull = CULL_CCW;
   }
   SetRenderState(CULLMODE, cull);
}

void RenderState::SetRenderStateDepthBias(float bias)
{
   m_depthBias = bias;
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
   s.append(blend_modes[blend_op]);
   s.append(blend_functions[blend_dest]);
   s.append(blend_functions[blend_src]);
   s.append("} Depth: {");
   s.append(z_test ? " Z " : " _ ");
   s.append(functions[z_func]);
   s.append(z_write ? " ZW " : " __ ");
   s.append("} Clip:");
   s.append(clip_plane ? " C " : " _ ");
   s.append("Cull:");
   s.append(cull_modes[cull_mode]);
   s.append("Mask: ");
   s += (char)((color_write < 10 ? 48 : 55) + color_write);
   return s;
}
