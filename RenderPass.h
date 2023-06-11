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
   void Sort(vector<RenderPass*>& sortedPasses);
   
   void Submit(RenderCommand* command);
   void Execute(const bool log = false);

   void RecycleCommands(std::vector<RenderCommand*>& commandPool);

   RenderTarget* m_rt;
   string m_name;
   bool m_depthReadback = false;

private:
   int m_sortKey = 0;
   vector<RenderCommand*> m_commands;
   vector<RenderPass*> m_dependencies; // List of render passes that must have been performed before executing this pass
};
