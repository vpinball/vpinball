// license:GPLv3+

#pragma once

class RenderState final
{
public:
   #define RENDER_STATE(name, bitpos, bitsize) name,
   // These definitions must be copy/pasted between RenderState.h/cpp when modified to keep the implementation in sync
   enum RenderStates
   {
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
      RENDERSTATE_COUNT,
      RENDERSTATE_INVALID
   };
   #undef RENDER_STATE

   enum RenderStateValue
   {
      //Booleans
      RS_FALSE = 0,
      RS_TRUE = 1,
      //Culling
      CULL_NONE = 0,
      CULL_CW = 1,
      CULL_CCW = 2,
      //Depth functions
      Z_ALWAYS = 0,
      Z_LESS = 1,
      Z_LESSEQUAL = 2,
      Z_GREATER = 3,
      Z_GREATEREQUAL = 4,
      //Blending ops
      BLENDOP_MAX = 0,
      BLENDOP_ADD = 1,
      BLENDOP_REVSUBTRACT = 2,
      //Blending values
      ZERO = 0,
      ONE = 1,
      SRC_ALPHA = 2,
      DST_ALPHA = 3,
      INVSRC_ALPHA = 4,
      INVSRC_COLOR = 5,
      //Color mask
      RGBMASK_NONE = 0x00000000u,
      RGBMASK_RGBA = 0x0000000Fu,

      UNDEFINED
   };

   RenderState();

   RenderStateValue GetRenderState(const RenderStates p1) const;
   void SetRenderState(const RenderStates p1, const RenderStateValue p2);
   void SetRenderStateDepthBias(float bias);

   bool IsOpaque() const;

   void Apply(RenderDevice* device);

   string GetLog() const;

   unsigned int m_state;
   float m_depthBias;

private:
   struct RenderStateMask
   {
      uint32_t shift;
      uint32_t mask;
      uint32_t clear_mask;
   };
   static const RenderStateMask render_state_masks[RENDERSTATE_COUNT];
};
