#pragma once

extern const string PLAYFIELD_REFLECTION_RENDERPROBE_NAME;

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

   string& GetName();
   void SetName(const string& name);
   ProbeType GetType() const { return m_type; }
   void SetType(const ProbeType type) { m_type = type; }
   int GetBaseRoughness() const { return m_roughness_base; }
   void SetBaseRoughness(const int roughness) { m_roughness_base = roughness; }
   int GetClearRoughness() const { return m_roughness_clear; }
   void SetClearRoughness(const int roughness) { m_roughness_clear = roughness; }

   // Reflection plane properties
   void GetReflectionPlane(vec4& plane) const;
   void SetReflectionPlane(const vec4& plane);
   void GetReflectionPlaneNormal(vec3& normal) const;
   ReflectionMode GetReflectionMode() const { return m_reflection_mode;  }
   void SetReflectionMode(ReflectionMode mode);

   // Load/Save
   int GetSaveSize() const;
   HRESULT SaveData(IStream* pstm, HCRYPTHASH hcrypthash, const bool saveForUndo);
   HRESULT LoadData(IStream* pstm, PinTable* ppt, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey);
   bool LoadToken(const int id, BiffReader* const pbr);

   // Rendering
   void RenderSetup();
   void MarkDirty(); // Mark this probe as dirty, should be called when starting a new frame
   bool IsRendering() const; // Rendering is not reentrant so GetProbe should not be called when this returns true
   RenderTarget* GetProbe(const bool is_static); // Request render probe, eventually rendering it if it is dirty
   void EndPlay();

private:
   int GetRoughnessDownscale(const int roughness);
   void ApplyRoughness(RenderTarget* probe, const int roughness);
   void RenderReflectionProbe(const bool is_static);
   void RenderScreenSpaceTransparency(const bool is_static);

   ProbeType m_type = PLANE_REFLECTION;
   bool m_dirty = true;
   string m_name;
   int m_roughness_base = 0; // Default roughness level
   int m_roughness_clear = 0; // Roughness level to be used when there is a roughness map (lerp between base and clear according to roughness map)
   bool m_rendering = false;
   RenderTarget* m_blurRT = nullptr;
   RenderTarget* m_staticRT = nullptr;
   RenderTarget* m_dynamicRT = nullptr;

   // Properties for reflection probe
   vec4 m_reflection_plane = vec4(0.f, 0.f, 1.f, 0.f); // Plane equation: xyz is the normal, w is the projected distance
   ReflectionMode m_reflection_mode = REFL_DYNAMIC;
};
