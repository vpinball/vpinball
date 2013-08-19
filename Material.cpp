#include "stdafx.h"
#include "Material.h"
#include "RenderDevice.h"

RenderDevice *Material::renderDevice=0;

void Material::setRenderDevice( RenderDevice *_device )
{
   renderDevice = _device;
}

void Material::set()
{
   renderDevice->SetMaterial(&mat);
}