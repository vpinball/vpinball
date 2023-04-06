#include "stdafx.h"
#include "RenderFrame.h"
#include "RenderPass.h"
#include "RenderCommand.h"
#include "RenderDevice.h"

RenderFrame::RenderFrame(RenderDevice* renderDevice)
   : m_rd(renderDevice)
{
}

RenderFrame::~RenderFrame()
{
   for (auto item : m_commandPool)
      delete item;
   for (auto item : m_passes)
      delete item;
}

void RenderFrame::AddPass(RenderPass* pass)
{
   m_passes.push_back(pass);
}

RenderCommand* RenderFrame::NewCommand()
{
   RenderCommand* item;
   if (m_commandPool.size() == 0)
   {
      item = new RenderCommand(m_rd);
   }
   else
   {
      item = m_commandPool.back();
      m_commandPool.pop_back();
   }
   return item;
}

void RenderFrame::Execute()
{
   if (m_passes.size() == 0)
      return;

   // Sort passes to avoid useless render target switching, and allow merging passes for better draw call sorting/batching
   vector<RenderPass*> sortedPasses;
   m_passes.back()->Sort(sortedPasses);
   
   #ifndef ENABLE_SDL
   CHECKD3D(m_rd->GetCoreDevice()->BeginScene());
   #endif
   for (RenderPass* pass : sortedPasses)
   {
      pass->Execute();
      if (m_commandPool.size() < 1024)
         pass->RecycleCommands(m_commandPool);
      delete pass;
   }
   #ifndef ENABLE_SDL
   CHECKD3D(m_rd->GetCoreDevice()->EndScene());
   #endif
   m_passes.clear();
}
