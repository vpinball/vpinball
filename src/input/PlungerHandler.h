#pragma once

#include "math/vector.h"

class InputManager;
class PhysicsSensor;
class PlungerSensor;


class PlungerHandler final
{
public:
   PlungerHandler(InputManager* inputManager);

   void StepOneMillisecond();
   float GetVelocity() const;
   float GetPosition() const;
   bool IsLinear() const;
   bool HasVelocity() const;

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
   float m_position = 0.f;
   float m_velocity = 0.f;
   bool m_isLinear = true;
   bool m_hasVelocity = false;
};


class PlungerSensor final
{
public:
   PlungerSensor(InputManager* inputManager);
   ~PlungerSensor();

   void Load(const Settings& settings, int sensorIndex);
   void Save(Settings& settings, int sensorIndex) const;

   bool IsLinear() const;
   void SetLinear(bool isLinear);

   bool IsPositionFilterEnabled() const;
   void EnablePositionFilter(bool enable);

   bool HasVelocity() const;

   void StepOneMillisecond();
   float GetVelocity() const { return m_velocity; }
   float GetPosition() const { return m_position; } // 0 at rest, +1 fully retracted, negative value scale depends on 'linear' flag

   bool IsActive() const;

   const std::unique_ptr<PhysicsSensor>& GetPositionSensor() const { return m_positionSensor; }
   const std::unique_ptr<PhysicsSensor>& GetVelocitySensor() const { return m_velocitySensor; }

private:
   std::unique_ptr<PhysicsSensor> m_positionSensor;
   std::unique_ptr<PhysicsSensor> m_velocitySensor;
   std::unique_ptr<class PlungerPositionFilter> m_positionFilter;
   bool m_linearPlunger = true;

   float m_position = 0.f;
   float m_velocity = 0.f;
   int m_deactivationDelay = 0;
};
