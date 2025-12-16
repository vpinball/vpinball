#pragma once

#include "core/stdafx.h"

#include "ImageUIPart.h"

namespace VPX::EditorUI
{

ImageUIPart::ImageUIPart(Image* image)
   : m_image(image)
{
}

ImageUIPart::~ImageUIPart()
{
}

void ImageUIPart::Render(const EditorRenderContext& ctx) {
}

void ImageUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("Image", m_image);

}

}