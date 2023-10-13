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
   void UpdateDependency(RenderTarget* target, RenderPass* newDependency);

   void SortPasses(vector<RenderPass*>& sortedPasses, vector<RenderPass*>& allPasses);
   void SortCommands();

   void Submit(RenderCommand* command);
   bool Execute(const bool log = false);

   void RecycleCommands(std::vector<RenderCommand*>& commandPool);

   unsigned int GetCommandCount() const { return (unsigned int)m_commands.size(); }

   RenderTarget* m_rt;
   int m_singleLayerRendering = -1; // if positive, rendering will only be performed on the corresponding layer
   string m_name;
   bool m_depthReadback = false;

   vector<RenderCommand*> m_commands;
   vector<RenderPass*> m_dependencies; // List of render passes that must have been performed before executing this pass
   vector<RenderTarget*> m_referencedRT; // List of render targets used by dependencies
   int m_sortKey = 0;
   bool m_updated = false;
};
