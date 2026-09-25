#pragma once

#include "math/ExponentialMovingAverage.h"
#include "math/vector.h"
#include "PlungerKalmanFilter.h"

class Settings;
class InputManager;
class PhysicsSensor;
class PlungerSensor;

// VPX uses different unit convention for plungers, therefore all functions are marked with the following notations:
// - VPU  & VPU/VPT: these are the usual VPX units for absolute coordinates and time
// - m    &     m/s: SI units used by some of the physics elements
// - t.u. &  t.u./s: Plunger & HitPlunger use relative units where 1 is fully retracted, the rest/park position is defined by the plunger object in the 0..1 range, 0 is fully extended
// - p.u. &  p.u./s: PlungerHandler and PlungerSensor use relative units where 1 is fully retracted, 0 is rest, and the fully extended is somewhere below 0 but limited -1/3 as no plunger extends that much
class PlungerHandler final
{
public:
   PlungerHandler(InputManager* inputManager, Settings& appSettings);

   void StepOneMillisecond();

   bool HasPlungerSensor() const;
   float GetRawVelocity() const; // p.u/s
   float GetHitVelocity(float restPos) const; // restPos must be in t.u., returned velocity is in t.u/s.
   float GetPosition(float restPos) const; // t.u.

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
   Settings& m_appSettings; // Sensor mapping is an application wide setting (not overridable per table)
   vector<std::unique_ptr<PlungerSensor>> m_sensors;

   bool m_isPullBackAndRetract = false; // enable 1s retract phase for button/key plunger
   float m_rawVelocity = 0.f; // p.u./s
   float m_position = 0.f; // p.u.
};


class PlungerSensor final
{
public:
   PlungerSensor(InputManager* inputManager);
   ~PlungerSensor();

   void Load(const Settings& settings, int sensorIndex);
   void Save(Settings& settings, int sensorIndex) const;

   void StepOneMillisecond();

   float GetRawPosition() const { return m_position; } // p.u.
   float GetRawVelocity() const { return m_emaVelocity.Get(); } // p.u/s
   float GetHitVelocity(float restPos) const; // restPos must be in t.u., returned velocity is in t.u/s.

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

   const float m_kalmanUnitScale = VPUTOM(INCHESTOVPU(3.f)); // length of a default fully retracted plunger, used to feed the kalman filter in real world units (m, m/s) instead of per unit
   PlungerKalmanFilter m_pvKalmanFilter; // Use m & m/s but for a fixed virtual plunger length defined through m_kalmanUnitScale
   ExponentialMovingAverage m_emaPosition; // p.u.
   ExponentialMovingAverage m_emaVelocity; // p.u./s

   float m_position = 0.f; // p.u.
   std::array<float, 100> m_prevPosition {}; // p.u.
   int m_PrevPositionPos = 0;
};
