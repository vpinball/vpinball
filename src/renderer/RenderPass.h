// license:GPLv3+

#pragma once

class RenderTarget;
class RenderCommand;

class RenderPass final
{
public:
   RenderPass(const string& name, RenderTarget* const rt);
   ~RenderPass();

   void Reset(const string& name, RenderTarget* const rt);

   void AddPrecursor(RenderPass* dependency);

   void SortCommands();

   void Submit(RenderCommand* command);
   bool Execute(const bool log = false);

   void RecycleCommands(std::vector<RenderCommand*>& commandPool);

   unsigned int GetCommandCount() const { return (unsigned int)m_commands.size(); }

   RenderTarget* m_rt;
   vec4 m_areaOfInterest; // if defined (not FLT_MAX), defines render clipping bounds (in clip space -1..1)
   int m_singleLayerRendering = -1; // if positive, rendering will only be performed on the corresponding layer
   string m_name;
   bool m_depthReadback = false;
   bool m_mergeable = true; // true if this pass can be merged with its precursor if they are on the same render target, leading to sorting the render commands of both passes together

   vector<RenderCommand*> m_commands;
   vector<RenderPass*> m_dependencies; // List of render passes that must have been performed before executing this pass (i.e. this passes uses the render target of its dependencies)

   int m_sortKey = 0; // Flag used during sorting and pass splitting
   vector<RenderTarget*> m_referencedRT; // When frame sort passes, it uses this list to avoid render targets overwrite between render passes
};
