#pragma once

#include "Shader.h"

class RenderDevice;
class RenderPass;
class RenderCommand;

class RenderFrame final
{
public:
   RenderFrame(RenderDevice* renderDevice);
   ~RenderFrame();

   RenderPass* AddPass(const string& name, RenderTarget* const rt);
   bool Execute(const bool log = false);

   RenderCommand* NewCommand();

private:
   RenderDevice* const m_rd;
   vector<RenderPass*> m_passes;
   vector<RenderPass*> m_passPool;
   vector<RenderCommand*> m_commandPool;

   // Shader state
   Shader::ShaderState* m_basicShaderState = nullptr;
   Shader::ShaderState* m_DMDShaderState = nullptr;
   Shader::ShaderState* m_FBShaderState = nullptr;
   Shader::ShaderState* m_flasherShaderState = nullptr;
   Shader::ShaderState* m_lightShaderState = nullptr;
   Shader::ShaderState* m_ballShaderState = nullptr;
};
