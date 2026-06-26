// license:GPLv3+

#pragma once

#include "input/PhysicsSensor.h"
#include "math/vector.h"
#include "physics/cabinet/CabinetPhysics.h"
#include "physics/cabinet/NudgeHandler.h"
#include "physics/cabinet/NudgeIntentHandler.h"

class InputManager;
class PhysicsSensor;
class Settings;

namespace VPX::Physics
{

class GamepadNudge final : public NudgeSensor
{
public:
   GamepadNudge(InputManager* inputManager);
   ~GamepadNudge() override;

   void Load(const Settings& settings, int sensorIndex) override;
   void Save(Settings& settings, int sensorIndex) const override;

   bool IsSameSensor(const std::unique_ptr<NudgeSensor>& sensor) const override;
   bool IsMappedToDevice(uint16_t deviceId) const override;

   string GetDisplayName() const override;

   void StepOneMillisecond() override;
   const Vertex2D& GetCabinetAcceleration() const override;
   const Vertex2D& GetCabinetOffset() const override;

   bool IsActive() const override;

   float GetStrengthScale() const override { return m_nudgeStrengthScale; }
   void SetStrengthScale(float scale) override { m_nudgeStrengthScale = scale; }

   PhysicsSensor& GetXSensor() { return m_xSensor; }
   PhysicsSensor& GetYSensor() { return m_ySensor; }
   const PhysicsSensor& GetXSensor() const { return m_xSensor; }
   const PhysicsSensor& GetYSensor() const { return m_ySensor; }

private:
   PhysicsSensor m_xSensor;
   PhysicsSensor m_ySensor;

   NudgeIntentHandler m_nudgeIntentHandler;
   CabinetPhysics m_cabinetModel;

   float m_nudgeStrengthScale = 1.f;

   int m_deactivationDelay = 0;
};

}
