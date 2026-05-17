// license:GPLv3+

#pragma once

#include "input/PhysicsSensor.h"
#include "math/ExponentialMovingAverage.h"
#include "math/vector.h"
#include "physics/cabinet/CabinetPhysics.h"
#include "physics/cabinet/NudgeHandler.h"
#include "physics/cabinet/NudgeIntentHandler.h"
#include "physics/cabinet/MotionGainCalibratorAxis.h"
#include "physics/cabinet/MotionKalmanAxis.h"

class InputManager;
class Settings;

namespace VPX::Physics
{

class CabinetNudgeSensor : public NudgeSensor
{
public:
   CabinetNudgeSensor(InputManager* inputManager);
   ~CabinetNudgeSensor() override;

   void Load(const Settings& settings, int sensorIndex) override;
   void Save(Settings& settings, int sensorIndex) const override;

   bool IsSameSensor(const std::unique_ptr<NudgeSensor>& sensor) const override;
   bool IsMappedToDevice(uint16_t deviceId) const override;

   string GetDisplayName() const override;

   bool IsIntentSensor() const;
   void SetIntentSensor(bool isIntent);

   void StepOneMillisecond() override;
   const Vertex2D& GetCabinetAcceleration() const override;
   const Vertex2D& GetCabinetOffset() const override;

   bool IsActive() const override;

   // Custom nudge force scaling (no unit)
   float GetStrengthScale() const override { return m_nudgeStrengthScale; }
   void SetStrengthScale(float scale) override { m_nudgeStrengthScale = scale; }

   // Real world cabinet mass (as the sensors are returning velocity/acceleration, we need the cabinet mass to evaluate the impulse force of user nudges)
   float GetCabinetMass() const { return m_cabinetMass; }
   void SetCabinetMass(float mass) { m_cabinetMass = mass; }

   // Velocity sensor, unit after scale must be m/s
   PhysicsSensor& GetXVelSensor() { return m_xVelSensor.m_sensor; }
   PhysicsSensor& GetYVelSensor() { return m_yVelSensor.m_sensor; }
   const PhysicsSensor& GetXVelSensor() const { return m_xVelSensor.m_sensor; }
   const PhysicsSensor& GetYVelSensor() const { return m_yVelSensor.m_sensor; }

   // Acceleration sensor, unit after scale must be m/s^2
   PhysicsSensor& GetXAccSensor() { return m_xAccSensor.m_sensor; }
   PhysicsSensor& GetYAccSensor() { return m_yAccSensor.m_sensor; }
   const PhysicsSensor& GetXAccSensor() const { return m_xAccSensor.m_sensor; }
   const PhysicsSensor& GetYAccSensor() const { return m_yAccSensor.m_sensor; }

private:
   struct SyncedSensor
   {
      SyncedSensor(class InputManager* eventManager, const string& label, SensorMapping::Type sensorType)
         : m_sensor(eventManager, label, sensorType)
      {
      }
      PhysicsSensor m_sensor;
      uint64_t m_lastTimestampNs = 0;
      int64_t m_clockDeltaUs = 0;
      int m_restCount = 0;
      bool m_forceRest = false;
      float m_lastValue = 0.f;
   };
   void UpdateAxisSensor(SyncedSensor& sensor, MotionKalmanAxis& axis, float axisGain);
   void UpdateAxis(SyncedSensor& velSensor, SyncedSensor& accSensor, MotionKalmanAxis& kalmanFilter, MotionGainCalibratorAxis& gainCalibrator);
   MotionGainCalibratorAxis m_xGainCalibrator;
   MotionGainCalibratorAxis m_yGainCalibrator;
   SyncedSensor m_xVelSensor;
   SyncedSensor m_yVelSensor;
   SyncedSensor m_xAccSensor;
   SyncedSensor m_yAccSensor;
   bool m_accVelGainLog = false;

   MotionKalmanAxis m_kalmanX;
   MotionKalmanAxis m_kalmanY;

   ExponentialMovingAverage m_emaX;
   ExponentialMovingAverage m_emaY;

   uint64_t m_timeUs = 0;

   std::unique_ptr<NudgeIntentHandler> m_nudgeIntentHandler;
   CabinetPhysics m_cabinetModel;

   Vertex2D m_cabinetAcceleration;

   float m_cabinetMass = 1.f;

   float m_nudgeStrengthScale = 1.f;

   int m_deactivationDelay = 0;
};

}
