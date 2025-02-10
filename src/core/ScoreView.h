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
   void Select();
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
      string glassPath;
      float glassRoughness;
      vec3 glassAmbient;
      float glassPadL, glassPadR, glassPadT, glassPadB;
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
   };

   vector<Layout> m_layouts;
   Layout* m_bestLayout = nullptr;

   void LoadGlass(Visual& visual);
   robin_hood::unordered_map<string, Texture*> m_images;
};
