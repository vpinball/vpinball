#include "core/stdafx.h"

#include "EditorUIPartRegistry.h"

#include "BallUIPart.h"
#include "BumperUIPart.h"
#include "DecalUIPart.h"
#include "DispReelUIPart.h"
#include "FlasherUIPart.h"
#include "FlipperUIPart.h"
#include "GateUIPart.h"
#include "HitTargetUIPart.h"
#include "KickerUIPart.h"
#include "LightUIPart.h"
#include "LightSeqUIPart.h"
#include "PartGroupUIPart.h"
#include "PlungerUIPart.h"
#include "PrimitiveUIPart.h"
#include "RampUIPart.h"
#include "RubberUIPart.h"
#include "SpinnerUIPart.h"
#include "SurfaceUIPart.h"
#include "TextBoxUIPart.h"
#include "TimerUIPart.h"
#include "TriggerUIPart.h"

namespace VPX::EditorUI
{

ankerl::unordered_dense::map<ItemTypeEnum, std::function<std::shared_ptr<EditorUIPart>(IEditable*)>> EditorUIPartRegistry::m_map;

void EditorUIPartRegistry::InitRegistry()
{
   if (!m_map.empty())
      return;

   Register<BallUIPart, Ball>();
   Register<BumperUIPart, Bumper>();
   Register<DecalUIPart, Decal>();
   Register<DispReelUIPart, DispReel>();
   Register<FlasherUIPart, Flasher>();
   Register<FlipperUIPart, Flipper>();
   Register<GateUIPart, Gate>();
   Register<HitTargetUIPart, HitTarget>();
   Register<KickerUIPart, Kicker>();
   Register<LightUIPart, Light>();
   Register<LightSeqUIPart, LightSeq>();
   Register<PartGroupUIPart, PartGroup>();
   Register<PlungerUIPart, Plunger>();
   Register<PrimitiveUIPart, Primitive>();
   Register<RampUIPart, Ramp>();
   Register<RubberUIPart, Rubber>();
   Register<SpinnerUIPart, Spinner>();
   Register<SurfaceUIPart, Surface>();
   Register<TextBoxUIPart, Textbox>();
   Register<TimerUIPart, Timer>();
   Register<TriggerUIPart, Trigger>();
}

std::shared_ptr<EditorUIPart> EditorUIPartRegistry::Create(IEditable* editable)
{
   const auto it = m_map.find(editable->GetItemType());
   return (it != m_map.end()) ? it->second(editable) : nullptr;
}

}
