// license:GPLv3+

#pragma once



class ScoreView final
{
public:
   ScoreView();
   ~ScoreView();

   bool HasLayouts() const { return !m_layouts.empty(); }
   void Load(const string& path);
   void Reset() { m_layouts.clear(); }
   void Select(const VPX::RenderOutput& output);
   void Render(const VPX::RenderOutput& output);

private:
   void Parse(const string& path, std::istream& content);

   enum Fit
   {
      Fill,       // Stretch to fill container
      Contain,    // Stretch to fit inside container, keeping aspect ratio, eventually causing black bars
      Cover,      // Stretch to cover container, keeping aspect ratio, eventually cropping
      None,       // No stretching, content is placed at center
      ScaleDown   // Same as 'None' unless the content overflow the container, in which situation 'Contain' is applied
   };
   enum VisualType
   {
      DMD, SegDisplay, Image
   };
   struct Visual
   {
      VisualType type;
      float x, y, w, h;
      // Data source, depending on visual type
      string srcUri;
      // For DMD, CRT and segment displays
      string style;
      vec3 tint;
      vec4 glassPad;
      vec3 glassTint;
      float glassRoughness;
      string glassPath;
      vec4 glassArea;
      vec3 glassAmbient;
      // For segment displays
      Renderer::SegmentFamily segFamilyHint;
      int nElements;
      std::vector<float> xOffsets;
      // For DMD displays
      int2 dmdSize;
      // Live data (not serialized)
      Texture* glass;
      int liveStyle;
   };
   struct Layout
   {
      string path;
      float width, height;
      Fit fit;
      vector<Visual> visuals;
      // Live data
      unsigned int unmatchedVisuals;
      unsigned int matchedVisuals;
      float unfittedPixels;
   };

   vector<Layout> m_layouts;
   Layout* m_bestLayout = nullptr;

   void LoadGlass(Visual& visual);
   robin_hood::unordered_map<string, Texture*> m_images;
};
