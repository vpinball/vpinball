// license:GPLv3+

#pragma once

class SensorMapping final
{
public:
   // Input event manager that manages analog event sources (usually reported as joystick axis) and dispatch events
   class AxisInputEventManager
   {
   public:
      virtual ~AxisInputEventManager() = default;
      virtual void Register(class SensorMapping* mapping) = 0;
      virtual void Unregister(class SensorMapping* mapping) = 0;
   };

   // Mapping handler that maps an input to a sensor state, that is to say the measure of position/velocity/acceleration on a single axis
   class SensorMappingHandler
   {
   public:
      virtual ~SensorMappingHandler() = default;
      virtual void OnInputChanged(class SensorMapping* mapping) = 0;
   };

   enum class Type
   {
      Position, Velocity, Acceleration
   };

   SensorMapping(AxisInputEventManager* eventManager, SensorMappingHandler* mappingHandler,
      uint16_t deviceId, uint16_t axisId, Type type, float deadZone = 0.f, float scale = 1.f, float limit = 1.f)
      : m_deviceId(deviceId)
      , m_axisId(axisId)
      , m_type(type)
      , m_eventManager(eventManager)
      , m_mappingHandler(mappingHandler)
      , m_deadZone(deadZone)
      , m_scale(scale)
      , m_limit(limit)
   {
      if (m_eventManager)
         m_eventManager->Register(this);
   }

   SensorMapping(const SensorMapping& other) noexcept
      : m_deviceId(other.m_deviceId)
      , m_axisId(other.m_axisId)
      , m_type(other.m_type)
      , m_rawValue(other.m_rawValue)
      , m_value(other.m_value)
      , m_deadZone(other.m_deadZone)
      , m_scale(other.m_scale)
      , m_limit(other.m_limit)
      , m_eventManager(other.m_eventManager)
      , m_mappingHandler(other.m_mappingHandler)
   {
      if (m_eventManager)
         m_eventManager->Register(this);
   }

   SensorMapping& operator=(SensorMapping&& other) noexcept
   {
      if (this != &other)
      {
         if (m_eventManager)
            m_eventManager->Unregister(this);
         m_deviceId = other.m_deviceId;
         m_axisId = other.m_axisId;
         m_type = other.m_type;
         m_eventManager = other.m_eventManager;
         m_mappingHandler = other.m_mappingHandler;
         m_rawValue = other.m_rawValue;
         m_value = other.m_value;
         m_deadZone = other.m_deadZone;
         m_scale = other.m_scale;
         m_limit = other.m_limit;
         if (m_eventManager)
            m_eventManager->Register(this);
      }
      return *this;
   }

   ~SensorMapping()
   {
      if (m_eventManager)
         m_eventManager->Unregister(this);
   }

   bool operator==(const SensorMapping& other) const
   {
      return m_eventManager == other.m_eventManager
         && m_mappingHandler == other.m_mappingHandler
         && m_deviceId == other.m_deviceId 
         && m_axisId == other.m_axisId
         && m_type == other.m_type
         && m_deadZone == other.m_deadZone
         && m_scale == other.m_scale
         && m_limit == other.m_limit;
   }
   
   bool IsSame(const SensorMapping& other) const
   {
      return m_deviceId == other.m_deviceId 
         && m_axisId == other.m_axisId
         && m_type == other.m_type
         && m_deadZone == other.m_deadZone
         && m_scale == other.m_scale
         && m_limit == other.m_limit;
   }
   
   static SensorMapping Create(uint16_t deviceId, uint16_t axisId, Type type)
   {
      return SensorMapping(nullptr, nullptr, deviceId, axisId, type);
   }

   SensorMapping With(AxisInputEventManager* eventManager, SensorMappingHandler* mappingHandler) const
   {
      return SensorMapping(eventManager, mappingHandler, m_deviceId, m_axisId, m_type, m_deadZone, m_scale, m_limit);
   }

   void SetAxisPosition(uint64_t timestampNs, float value)
   {
      int updateDelayMs = static_cast<int>(timestampNs - m_rawValueTimestampNs) / 1000000;
      if (updateDelayMs > 0 && (m_shortestUpdateMs == 0 || updateDelayMs < m_shortestUpdateMs))
         m_shortestUpdateMs = updateDelayMs;
      m_rawValueTimestampNs = timestampNs;
      if (m_rawValue != value)
      {
         m_rawValue = value;
         UpdateValue();
      }
      if (m_mappingHandler)
         m_mappingHandler->OnInputChanged(this);
   }

   void SetDeadZone(float deadZone)
   {
      m_deadZone = deadZone;
      UpdateValue();
   }

   void SetScale(float scale)
   {
      m_scale = scale;
      UpdateValue();
   }

   void SetLimit(float limit)
   {
      m_limit = limit;
      UpdateValue();
   }

   uint16_t GetDeviceId() const { return m_deviceId; }
   uint16_t GetAxisId() const { return m_axisId; }
   Type GetType() const { return m_type; }
   float GetDeadZone() const { return m_deadZone; }
   float GetScale() const { return m_scale; }
   float GetLimit() const { return m_limit; }

   float GetRawValue() const { return m_rawValue; }
   uint64_t GetRawValueTimestampNs() const { return m_rawValueTimestampNs; }
   float GetValue() const { return m_value; }

   int GetShortestUpdateMs() const { return m_shortestUpdateMs; }
   void ResetShortestUpdateMs() { m_shortestUpdateMs = 0; }

private:
   void UpdateValue()
   {
      m_value = m_rawValue;
      if (m_deadZone > 0.f)
      {
         if (m_value < -m_deadZone)
            m_value = (m_value + m_deadZone) / (1.f - m_deadZone);
         else if (m_value > m_deadZone)
            m_value = (m_value - m_deadZone) / (1.f - m_deadZone);
         else
            m_value = 0.f;
      }
      m_value = clamp(m_value * m_scale, -m_limit, m_limit);
   }

   uint16_t m_deviceId;
   uint16_t m_axisId;
   Type m_type;
   AxisInputEventManager* m_eventManager;
   SensorMappingHandler* m_mappingHandler;

   uint64_t m_rawValueTimestampNs = 0;
   int m_shortestUpdateMs = 0;
   float m_rawValue = 0.f;
   float m_value = 0.f;
   float m_deadZone;
   float m_scale;
   float m_limit;
};
