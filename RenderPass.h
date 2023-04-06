#pragma once

class RenderTarget;
class RenderCommand;

class RenderPass final
{
public:
   RenderPass(const string& name, RenderTarget* const rt, const bool ignoreStereo = false);
   ~RenderPass();

   void AddPrecursor(RenderPass* dependency);
   void Sort(vector<RenderPass*>& sortedPasses);
   
   void Submit(RenderCommand* command);
   void Execute();

   void RecycleCommands(std::vector<RenderCommand*>& commandPool);

   RenderTarget* const m_rt;
   const string m_name;

private:
   int m_sortKey = 0;
   const bool m_ignoreStereo;
   std::vector<RenderCommand*> m_commands;
   std::vector<RenderPass*> m_dependencies; // List of render passes that must have been performed before executing this pass
};
