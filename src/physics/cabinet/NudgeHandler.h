#pragma once

#include "math/vector.h"


namespace VPX::Physics
{

class NudgeSource
{
public:
   virtual ~NudgeSource() = default;

   virtual void StepOneMillisecond() = 0;

   virtual bool IsActive() const = 0; // Used to select the active nudge source

   virtual float GetStrengthScale() const = 0;
   virtual void SetStrengthScale(float scale) = 0;

   virtual const Vertex2D& GetCabinetAcceleration() const = 0; // Applied on parts moving relatively to the cabinet (change of reference frame), expressed in m/s^2
   virtual const Vertex2D& GetCabinetOffset() const = 0; // Visual feedback, expressed in m
};


// Nudge driven by analog sensors (gamepad, cabinet,...)
class NudgeSensor : public NudgeSource
{
public:
   virtual ~NudgeSensor() = default;

   virtual void Load(const Settings& settings, int sensorIndex) = 0;
   virtual void Save(Settings& settings, int sensorIndex) const = 0;

   virtual bool IsSameSensor(const std::unique_ptr<NudgeSensor>& sensor) const = 0;
   virtual bool IsMappedToDevice(uint16_t deviceId) const = 0;

   virtual string GetDisplayName() const = 0;
};


// Nudge driven by script commands, usually triggered by digital input (keyboard or joystick button)
class KeyboardNudge : public NudgeSource
{
public:
   virtual ~KeyboardNudge() = default;

   virtual void Nudge(float angle, float force) = 0;
};


// Manage a group of nudge sources and consolidate the value to be used inside the physics emulation
class NudgeHandler final
{
public:
   NudgeHandler(InputManager* inputManager);

   void ApplyKeyboardImpulse(float angle, float force);

   void StepOneMillisecond();
   const Vertex2D& GetCabinetAcceleration() const; // m/s^2
   const Vertex2D& GetCabinetOffset() const; // m

   enum KeyboardNudgeMode
   {
      VP9_PUSH_RETRACT = 0,
      VPX_BOX_MODEL = 1,
      VPX_CAB_MODEL = 2
   };
   KeyboardNudgeMode GetKeyboardNudgeMode() const;
   void SetKeyboardNudgeMode(KeyboardNudgeMode mode);
   float GetKeyboardNudgeStrength() const;
   void SetKeyboardNudgeStrength(float strength);
   int GetKeyboardNudgeIndex() const { return m_keyboardNudgeIndex; }

   int GetSensorCount() const;
   const std::unique_ptr<NudgeSensor>& GetSensor(int index) const;
   void AddSensor(std::unique_ptr<NudgeSensor>& sensor);
   void ReplaceSensor(int index, std::unique_ptr<NudgeSensor> sensor);
   void RemoveSensor(int index);
   bool HasSensor(const std::unique_ptr<NudgeSensor>& sensor) const;
   bool IsMappedToDevice(uint16_t deviceId) const;
   void UnmapDevice(uint16_t deviceId);

   void SetExternalNudge(bool enableOverride, const Vertex2D& acceleration, const Vertex2D& displacement);

private:
   std::unique_ptr<KeyboardNudge> m_keyboardNudge;
   vector<std::unique_ptr<NudgeSensor>> m_sensors;
   Vertex2D m_noNudge = Vertex2D(0.f, 0.f);
   KeyboardNudgeMode m_keyboardNudgeMode;
   int m_keyboardNudgeIndex = 0;
};

};
