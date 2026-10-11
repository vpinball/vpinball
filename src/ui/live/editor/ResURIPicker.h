// license:GPLv3+

#pragma once

#include <functional>
#include <string>

#include "plugins/ResURIBuilder.h"

namespace VPX::EditorUI
{

using PinballPlugin::ResURIKind;

// Modal dialog building a ctrl:// URI toward a controller-exposed source.
// The picker enumerates the loaded plugins and their sources through the shared
// ResURIBuilder backend and can drive plugin/controller lifecycle (enable/disable,
// run/stop) when the wanted source is not yet exposed.
class ResURIPicker
{
public:
   // Opens the modal for the given source kind. apply is invoked with the generated URI on validation.
   static void Open(ResURIKind kind, const std::string& currentUri, std::function<void(const std::string&)> apply);

   // Renders the modal. Must be called every frame in the same ImGui id scope as the Open call.
   static void Render();
};

}
