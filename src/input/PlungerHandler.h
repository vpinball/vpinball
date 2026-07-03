#pragma once

#include "math/ExponentialMovingAverage.h"
#include "math/vector.h"
#include "PlungerKalmanFilter.h"

class InputManager;
class PhysicsSensor;
class PlungerSensor;


class PlungerHandler final
{
public:
   PlungerHandler(InputManager* inputManager);

   void StepOneMillisecond();

   bool HasPlungerSensor() const;
   float GetRawVelocity() const;
   float GetHitVelocity(float restPos) const;
   float GetPosition(float restPos) const;

   bool IsPullBackandRetract() const;
   void SetPullBackandRetract(bool isPullBackAndRetract);

   int GetSensorCount() const;
   const std::unique_ptr<PlungerSensor>& GetSensor(int index) const;
   void AddSensor(std::unique_ptr<PlungerSensor>& sensor);
   void ReplaceSensor(int index, std::unique_ptr<PlungerSensor> sensor);
   void RemoveSensor(int index);
   bool HasSensor(const std::unique_ptr<PlungerSensor>& sensor) const;
   bool IsMappedToDevice(uint16_t deviceId) const;
   void UnmapDevice(uint16_t deviceId);

   void SetExternalPlunger(bool enableOverride, const float velocity, const float displacement);

private:
   vector<std::unique_ptr<PlungerSensor>> m_sensors;

   bool m_isPullBackAndRetract = false; // enable 1s retract phase for button/key plunger
   float m_rawVelocity = 0.f;
   float m_position = 0.f;
};


class PlungerSensor final
{
public:
   PlungerSensor(InputManager* inputManager);
   ~PlungerSensor();

   void Load(const Settings& settings, int sensorIndex);
   void Save(Settings& settings, int sensorIndex) const;

   void StepOneMillisecond();

   float GetPosition() const { return m_position; } // 0 at rest, +1 fully retracted, negative value scale depends on 'linear' flag
   float GetRawVelocity() const { return m_emaVelocity.Get(); } // Unit is 'per unit'/s where 'per unit' correspond to the plunger travel length
   float GetHitVelocity(float restPos) const; // Unit is 'per unit'/s where 'per unit' correspond to the plunger travel length

   bool IsActive() const;

   const std::unique_ptr<PhysicsSensor>& GetPositionSensor() const { return m_positionSensor; }
   const std::unique_ptr<PhysicsSensor>& GetVelocitySensor() const { return m_velocitySensor; }

private:
   std::unique_ptr<PhysicsSensor> m_positionSensor;
   std::unique_ptr<PhysicsSensor> m_velocitySensor;
   uint64_t m_lastTimestampNs = 0;
   uint64_t m_timeNs = 0;
   int64_t m_clockDeltaNs = 0;
   int m_nRestSamples = 1000;
   int m_deactivationDelay = 0;

   PlungerKalmanFilter m_pvKalmanFilter;
   ExponentialMovingAverage m_emaPosition;
   ExponentialMovingAverage m_emaVelocity;
   const float m_kalmanUnitScale = VPUTOM(INCHESTOVPU(3.f)); // length of a default fully retracted plunger to setup the kalman filter in real world units (m, m/s)

   float m_position = 0.f;
   std::array<float, 100> m_prevPosition;
   int m_PrevPositionPos = 0;
};
