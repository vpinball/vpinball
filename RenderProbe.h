#pragma once

// A render probe is a render of the scene to an offscreen render target which is later used for shading scene parts, for example for reflections
class RenderProbe : ILoadable
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
      REFL_STATIC, // Only static reflections
      REFL_STATIC_N_BALLS, // Static reflections and balls, without depth sync (static or dynamic reflection may be rendered while they should be occluded)
      REFL_UNSYNCED_DYNAMIC, // Static and dynamic reflections, without depth sync (static or dynamic reflection may be rendered while they should be occluded)
      REFL_SYNCED_DYNAMIC, // Static and dynamic reflections, with depth sync (static reflection may be rendered while they should be occluded)
      REFL_DYNAMIC // All reflections are dynamic allowing for correct occlusion between them at the cost of performance
   };

   RenderProbe();
   ~RenderProbe();

   string GetName() const;
   void SetName(const string& name);
   ProbeType GetType() const { return m_type; }

   // Reflection plane properties
   void SetReflectionPlane(const vec4& plane);
   void GetReflectionPlaneNormal(vec3& normal) const;
   void SetReflectionMode(ReflectionMode mode);

   // Load/Save
   HRESULT SaveData(IStream* pstm, HCRYPTHASH hcrypthash);
   HRESULT LoadData(IStream* pstm, PinTable* ppt, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey);
   bool LoadToken(const int id, BiffReader* const pbr);

   // Rendering
   void RenderSetup();
   void MarkDirty(); // Mark this probe as dirty, should be called when starting a new frame
   bool IsRendering() const; // Rendering is not reentrant so GetProbe should not be called when this returns true
   Sampler* GetProbe(const bool is_static); // Request render probe, eventually rendering it if it is dirty
   void EndPlay();

private:
   void RenderReflectionProbe(const bool is_static);
   void RenderScreenSpaceTransparency(const bool is_static);

   ProbeType m_type = PLANE_REFLECTION;
   bool m_dirty = true;
   string m_name;
   int m_roughness = 1; // 0 is perfect mirror (full size, matching sampling), 1 is half texel offseted sampling, 2 is 50% rendering, 3 if 50% rendering with half texel offseted sampling, and so on
   RenderTarget* m_staticRT = nullptr;
   RenderTarget* m_dynamicRT = nullptr;
   bool m_rendering = false;

   // Properties for reflection probe
   vec4 m_reflection_plane = vec4(0.f, 0.f, 1.f, 0.f); // Plane equation: xyz is the normal, w is the projected distance
   ReflectionMode m_reflection_mode = REFL_DYNAMIC;
};
