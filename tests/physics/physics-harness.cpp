// license:GPLv3+

#include "core/stdafx.h"

#include "physics-harness.h"
#include "../vpx-test.h"

#include "core/ieditable.h"
#include "math/dragpoint.h"
#include "parts/ball.h"
#include "parts/pintable.h"
#include "parts/surface.h"
#include "physics/PhysicsEngine.h"
#include "physics/collide.h"
#include "physics/hitball.h"
#include "physics/hitable.h"

#include "doctest.h"


// Minimal IEditable/IHitable owning the raw hit objects registered through AddHitObject,
// so they are set up/released through the regular engine path like any part's colliders.
class PhysicsTestHarness::HitObjectSet final : public IEditable, public IHitable
{
public:
   // IEditable (this shim is not refcounted, not serializable and not scriptable)
   ULONG STDMETHODCALLTYPE AddRef() override { return 1; }
   ULONG STDMETHODCALLTYPE Release() override { return 1; }
   PinTable *GetPTable() override { return m_ptable; }
   const PinTable *GetPTable() const override { return m_ptable; }
   IHitable *GetIHitable() override { return this; }
   const IHitable *GetIHitable() const override { return this; }
   IRenderable *GetIRenderable() override { return nullptr; }
   const IRenderable *GetIRenderable() const override { return nullptr; }
   IScriptable *GetIScriptable() override { return nullptr; }
   const IScriptable *GetIScriptable() const override { return nullptr; }
   IFireEvents *GetIFireEvents() override { return nullptr; }
   EventProxyBase *GetEventProxyBase() override { return nullptr; }
   ItemTypeEnum GetItemType() const override { return eItemInvalid; }
   void SetDefaults(const bool fromMouseClick) override { }
   void WriteRegDefaults() override { }
   void Save(IObjectWriter &writer, const bool saveForUndo) override { }
   void Load(IObjectReader &partReader) override { }
   IEditable *CopyForPlay() const override { return nullptr; }
   Vertex2D GetCenter() const override { return Vertex2D(0.f, 0.f); }
   void Translate(const Vertex2D &offset) override { }

   // IHitable: hands the pending objects over to the engine (physics colliders only,
   // they are not registered for UI picking). The engine owns them once added.
   void PhysicSetup(PhysicsEngine *physics, const bool isUI) override
   {
      if (isUI)
         return;
      for (; m_added < m_objects.size(); ++m_added)
      {
         HitObject *const obj = m_objects[m_added].release();
         obj->m_editable = this;
         physics->AddCollider(obj, false);
      }
   }
   void PhysicRelease(PhysicsEngine *physics, const bool isUI) override
   {
      if (isUI)
         return;
      // The engine deletes the registered objects itself; only objects that were
      // added but never set up are still owned (and therefore deleted) here.
      m_objects.clear();
      m_added = 0;
   }

   void Add(std::unique_ptr<HitObject> hitObject) { m_objects.push_back(std::move(hitObject)); }

private:
   vector<std::unique_ptr<HitObject>> m_objects;
   size_t m_added = 0; // objects already handed over to the engine (PhysicSetup is re-entrant for live adds)
};


PhysicsTestHarness::PhysicsTestHarness(const float tableWidth, const float tableHeight)
   : m_table(CreateTestTable())
   , m_hitObjectSet(std::make_unique<HitObjectSet>())
{
   m_table->m_right = tableWidth;
   m_table->m_bottom = tableHeight;
   // Same default gravity as the player: the table's slope and gravity strength
   m_gravitySlopeDeg = m_table->GetPlayfieldSlope();
   m_gravityStrength = m_table->m_Gravity;
}

PhysicsTestHarness::~PhysicsTestHarness()
{
   // Destroy the engine before the table: it releases the colliders and notifies their editables
   m_physics.reset();
   m_table->Release();
}

Surface *PhysicsTestHarness::AddWall(const vector<Vertex2D> &outline, const float heightBottom, const float heightTop, const float elasticity, const float friction)
{
   Surface *const wall = Surface::COMCreate();
   wall->Init(0.f, 0.f, false);
   wall->SetName("TestWall" + std::to_string(++m_partCounter));
   wall->m_curve.ClearPoints();
   for (const Vertex2D &v : outline)
      wall->m_curve.PushPoint(std::make_unique<DragPoint>(&wall->m_curve, v.x, v.y, 0.f, false));
   wall->m_d.m_heightbottom = heightBottom;
   wall->m_d.m_heighttop = heightTop;
   wall->m_d.m_collidable = true;
   wall->m_d.m_hitEvent = false;
   wall->m_d.m_droppable = false;
   wall->m_d.m_overwritePhysics = true;
   wall->m_d.m_elasticity = elasticity;
   wall->m_d.m_friction = friction;
   wall->m_d.m_scatter = 0.f;
   m_table->AddPart(wall);
   wall->Release(); // owned by the table
   return wall;
}

Surface *PhysicsTestHarness::AddWallSegment(
   const float x1, const float y1, const float x2, const float y2, const float heightBottom, const float heightTop, const float halfThickness, const float elasticity, const float friction)
{
   const float dx = x2 - x1, dy = y2 - y1;
   const float len = sqrtf(dx * dx + dy * dy);
   REQUIRE(len > 0.f);
   const float nx = -dy / len * halfThickness, ny = dx / len * halfThickness;
   return AddWall({ Vertex2D(x1 + nx, y1 + ny), Vertex2D(x2 + nx, y2 + ny), Vertex2D(x2 - nx, y2 - ny), Vertex2D(x1 - nx, y1 - ny) }, heightBottom, heightTop, elasticity, friction);
}

void PhysicsTestHarness::AddHitObject(std::unique_ptr<HitObject> hitObject)
{
   REQUIRE(hitObject != nullptr);
   m_hitObjectSet->Add(std::move(hitObject));
   if (m_physics) // live add through the editable update path
      m_physics->Add(m_hitObjectSet.get());
}

Ball *PhysicsTestHarness::AddBall(const float x, const float y, const float z, const float vx, const float vy, const float vz, const float radius, const float mass)
{
   Ball *const ball = Ball::COMCreate();
   ball->Init(x, y, false, true);
   m_table->AddPart(ball);
   ball->m_hitBall.m_d.m_pos.z = z + radius;
   ball->m_hitBall.m_d.m_mass = mass;
   ball->m_hitBall.m_d.m_radius = radius;
   ball->m_hitBall.m_d.m_vel = Vertex3Ds(vx, vy, vz);
   if (m_physics) // if the engine exists, the ball is not collected by its constructor anymore
      ball->PhysicSetup(m_physics.get(), false);
   ball->Release(); // owned by the table
   return ball;
}

void PhysicsTestHarness::RemoveBall(Ball *ball)
{
   if (m_physics)
      ball->PhysicRelease(m_physics.get(), false);
   m_table->RemovePart(ball);
}

void PhysicsTestHarness::SetGravity(const float slopeDeg, const float strength)
{
   m_gravitySlopeDeg = slopeDeg;
   m_gravityStrength = strength;
   if (m_physics)
      m_physics->SetGravity(slopeDeg, strength);
}

void PhysicsTestHarness::Start()
{
   REQUIRE(m_physics == nullptr);
   m_physics = std::make_unique<PhysicsEngine>(m_table);
   m_physics->SetGravity(m_gravitySlopeDeg, m_gravityStrength);
   m_physics->SetImplicitPlayfieldMesh(m_playfieldFloor);
   m_physics->Add(m_hitObjectSet.get()); // registers the raw hit objects, if any
   m_physics->StartPhysics();
}

void PhysicsTestHarness::AdvanceMs(const uint32_t ms)
{
   REQUIRE(m_physics != nullptr); // Start() must be called first
   m_stepCount += ms;
   // UpdatePhysics runs one step for each full PHYSICS_STEPTIME strictly before the target
   // time, so the +1 usec ensures exactly the requested number of steps is executed.
   m_physics->UpdatePhysics(m_physics->GetStartTime() + m_stepCount * PHYSICS_STEPTIME + 1);
}

bool PhysicsTestHarness::AdvanceUntil(const std::function<bool()> &predicate, const uint32_t maxMs)
{
   for (uint32_t i = 0; i < maxMs; ++i)
   {
      if (predicate())
         return true;
      AdvanceMs(1);
   }
   return predicate();
}

uint32_t PhysicsTestHarness::GetTimeMs() const { return m_physics->GetTimeMsec(); }

const vector<HitBall *> &PhysicsTestHarness::GetBalls() const { return m_physics->GetBalls(); }
