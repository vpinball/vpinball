// license:GPLv3+

#include "core/stdafx.h"

#include "ui/win/WinUIPartRegistry.h"

#ifndef __STANDALONE__
#include "ui/win/parts/BallWinUIPart.h"
#include "parts/ball.h"
#include "ui/win/parts/BumperWinUIPart.h"
#include "parts/bumper.h"
#include "ui/win/parts/DecalWinUIPart.h"
#include "parts/decal.h"
#include "ui/win/parts/DispReelWinUIPart.h"
#include "parts/dispreel.h"
#include "ui/win/parts/DragPointWinUIPart.h"
#include "parts/dragpoint.h"
#include "ui/win/parts/FlasherWinUIPart.h"
#include "parts/flasher.h"
#include "ui/win/parts/FlipperWinUIPart.h"
#include "parts/flipper.h"
#include "ui/win/parts/GateWinUIPart.h"
#include "parts/gate.h"
#include "ui/win/parts/HitTargetWinUIPart.h"
#include "parts/hittarget.h"
#include "ui/win/parts/KickerWinUIPart.h"
#include "parts/kicker.h"
#include "ui/win/parts/LightWinUIPart.h"
#include "parts/light.h"
#include "ui/win/parts/LightSeqWinUIPart.h"
#include "parts/lightseq.h"
#include "ui/win/parts/PartGroupWinUIPart.h"
#include "parts/PartGroup.h"
#include "ui/win/parts/PlungerWinUIPart.h"
#include "parts/plunger.h"
#include "ui/win/parts/PrimitiveWinUIPart.h"
#include "parts/primitive.h"
#include "ui/win/parts/RampWinUIPart.h"
#include "parts/ramp.h"
#include "ui/win/parts/RubberWinUIPart.h"
#include "parts/rubber.h"
#include "ui/win/parts/SpinnerWinUIPart.h"
#include "parts/spinner.h"
#include "ui/win/parts/SurfaceWinUIPart.h"
#include "parts/surface.h"
#include "ui/win/parts/TextboxWinUIPart.h"
#include "parts/textbox.h"
#include "ui/win/parts/TimerWinUIPart.h"
#include "parts/timer.h"
#include "ui/win/parts/TriggerWinUIPart.h"
#include "parts/trigger.h"
#endif

ankerl::unordered_dense::map<ItemTypeEnum, WinUIPartRegistry::CreateFunc> WinUIPartRegistry::m_map;

#ifndef __STANDALONE__
namespace
{
// Fallback UI part for ISelect types that do not have a dedicated WinUI part (e.g. light centers).
// It only provides the shared context menu commands of the base implementation.
class GenericWinUIPart final : public IWinUIPart
{
public:
   GenericWinUIPart(PinTableWnd* editor, ISelect* select)
      : IWinUIPart(editor, select)
   {
   }
   void UIRenderPass1(Sur* psur) override { }
   void UIRenderPass2(Sur* psur) override { }
};
}
#endif

std::unique_ptr<IWinUIPart> WinUIPartRegistry::Create(PinTableWnd* editor, ISelect* select)
{
   if (!select)
      return nullptr;

   auto it = m_map.find(select->GetItemType());
   if (it != m_map.end())
      return it->second(editor, select);

   return nullptr;
}

void WinUIPartRegistry::InitRegistry()
{
#ifndef __STANDALONE__
   Register<BallWinUIPart, Ball>();
   Register<BumperWinUIPart, Bumper>();
   Register<DecalWinUIPart, Decal>();
   Register<DispReelWinUIPart, DispReel>();
   Register<DragPointWinUIPart, DragPoint>();
   Register<FlasherWinUIPart, Flasher>();
   Register<FlipperWinUIPart, Flipper>();
   Register<GateWinUIPart, Gate>();
   Register<HitTargetWinUIPart, HitTarget>();
   Register<KickerWinUIPart, Kicker>();
   Register<LightWinUIPart, Light>();
   Register<LightSeqWinUIPart, LightSeq>();
   Register<PartGroupWinUIPart, PartGroup>();
   Register<PlungerWinUIPart, Plunger>();
   Register<PrimitiveWinUIPart, Primitive>();
   Register<RampWinUIPart, Ramp>();
   Register<RubberWinUIPart, Rubber>();
   Register<SpinnerWinUIPart, Spinner>();
   Register<SurfaceWinUIPart, Surface>();
   Register<TextboxWinUIPart, Textbox>();
   Register<TimerWinUIPart, Timer>();
   Register<TriggerWinUIPart, Trigger>();

   m_map[eItemLightCenter] = [](PinTableWnd* editor, ISelect* part) -> std::unique_ptr<IWinUIPart> { return std::make_unique<GenericWinUIPart>(editor, part); };
#endif
}
