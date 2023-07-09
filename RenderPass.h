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

   unsigned int GetCommandCount() const { return m_commands.size(); }

   RenderTarget* m_rt;
   string m_name;
   bool m_depthReadback = false;

   vector<RenderCommand*> m_commands;
   vector<RenderPass*> m_dependencies; // List of render passes that must have been performed before executing this pass
   int m_sortKey = 0;
   bool m_updated = false;
};
