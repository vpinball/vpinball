// license:GPLv3+

#pragma once

#include "InputManager.h"

// Open Pinball Device context (defined in the OPD implementation module)
class OpenPinDevContext;

class OpenPinDevHandler final : public InputManager::InputHandler
{
public:
   explicit OpenPinDevHandler(InputManager& pininput);
   ~OpenPinDevHandler() override;
   void Update() override;

private:
   InputManager& m_inputManager;

   // Open Pinball Device context.  This is an opaque object managed
   // by the OPD implementation module, so that the whole implementation
   // can be detached at the build script level.
   OpenPinDevContext* m_OpenPinDevContext = nullptr;
};
