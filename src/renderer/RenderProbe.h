// license:GPLv3+

#pragma once

extern const string PLAYFIELD_REFLECTION_RENDERPROBE_NAME;

// A render probe is a render of the scene to an offscreen render target which is later used for shading scene parts, for example for reflections
class RenderProbe final : ILoadable
{
public:
   enum ProbeType
   {
      PLANE_REFLECTION,
      SCREEN_SPACE_TRANSPARENCY,
   };

   enum ReflectionMode
   {
      REFL_NONE, // No reflections
      REFL_BALLS, // Only balls reflections
      REFL_STATIC, // Only static (prerendered) reflections
      REFL_STATIC_N_BALLS, // Static reflections and balls, without depth sync (static or dynamic reflection may be rendered while they should be occluded)
      REFL_STATIC_N_DYNAMIC, // Static and dynamic reflections, without depth sync (static or dynamic reflection may be rendered while they should be occluded)
      REFL_DYNAMIC // All reflections are dynamic allowing for correct occlusion between them at the cost of performance (static are still prerendered)
   };

   RenderProbe();
   ~RenderProbe();

   string& GetName();
   void SetName(const string& name);
   ProbeType GetType() const { return m_type; }
   void SetType(const ProbeType type) { m_type = type; }
   int GetRoughness() const { return m_roughness; }
   void SetRoughness(const int roughness) { m_roughness = roughness; }

   // Reflection plane properties
   void GetReflectionPlane(vec4& plane) const;
   void SetReflectionPlane(const vec4& plane);
   void GetReflectionPlaneNormal(vec3& normal) const;
   ReflectionMode GetReflectionMode() const { return m_reflection_mode;  }
   void SetReflectionMode(ReflectionMode mode);
   bool GetReflectionNoLightmaps() const { return m_disableLightReflection; }
   void SetReflectionNoLightmaps(const bool disableLightmaps) { m_disableLightReflection = disableLightmaps; }
   void ExtendAreaOfInterest(const float xMin, const float xMax, const float yMin, const float yMax);
   void ApplyAreaOfInterest(RenderPass* pass = nullptr);

   // Load/Save
   int GetSaveSize() const;
   HRESULT SaveData(IStream* pstm, HCRYPTHASH hcrypthash, const bool saveForUndo);
   HRESULT LoadData(IStream* pstm, PinTable* ppt, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey);
   bool LoadToken(const int id, BiffReader* const pbr);

   // Rendering
   void RenderSetup(class Renderer* renderer);
   void MarkDirty(); // Mark this probe as dirty, should be called when starting a new frame
   bool IsRendering() const;
   void PreRenderStatic(); // Allows to precompute static parts
   RenderTarget* Render(const unsigned int renderMask); // Lazily update render probe and returns it
   void RenderRelease();

private:
   int GetRoughnessDownscale(const int roughness) const;
   void ApplyRoughness(RenderTarget* probe, const int roughness);

   void RenderScreenSpaceTransparency();

   void PreRenderStaticReflectionProbe();
   void RenderReflectionProbe(const unsigned int renderMask);
   void DoRenderReflectionProbe(const bool render_static, const bool render_balls, const bool render_dynamic);

   // Base properties
   ProbeType m_type = PLANE_REFLECTION;
   string m_name;
   int m_roughness = 0;

   // Properties for reflection probe
   vec4 m_reflection_plane = vec4(0.f, 0.f, 1.f, 0.f); // Plane equation: xyz is the normal, w is the projected distance
   vec4 m_reflection_clip_bounds;
   ReflectionMode m_reflection_mode = REFL_DYNAMIC;
   bool m_disableLightReflection = false; // Disable rendering of lightmaps in reflection render probes, needed to avoid having having reflections of playfield lightmaps onto the playfield itself

   // Properties used for rendering (not saved)
   Renderer* m_renderer = nullptr;
   RenderDevice* m_rd = nullptr;
   RenderDeviceState* m_rdState = nullptr;
   bool m_dirty = true;
   bool m_rendering = false;
   RenderTarget* m_blurRT = nullptr;
   RenderTarget* m_prerenderRT = nullptr;
   RenderTarget* m_dynamicRT = nullptr;
   RenderPass* m_finalPass = nullptr; // Pass after rougness has been applied
   RenderPass* m_copyPass = nullptr; // Pass that performs the screen space copy
};
