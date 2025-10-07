// license:GPLv3+

#pragma once


class ButtonMapping final
{
public:
   // Input event manager that manages button event sources and dispatch events to mappings
   class ButtonInputEventManager
   {
   public:
      virtual ~ButtonInputEventManager() = default;
      virtual void Register(class ButtonMapping* mapping) = 0;
      virtual void Unregister(class ButtonMapping* mapping) = 0;
   };

   // Mapping handler that handle mapping state change into actions
   class ButtonMappingHandler
   {
   public:
      virtual ~ButtonMappingHandler() = default;
      virtual void OnInputChanged(class ButtonMapping* mapping) = 0;
   };

   ButtonMapping(
      ButtonInputEventManager* eventManager, ButtonMappingHandler* mappingHandler, 
      uint16_t deviceId, uint16_t axisOrButtonId, float axisThreshold = 0.f, bool axisReversed = false)
      : m_deviceId(deviceId)
      , m_axisOrButtonId(axisOrButtonId)
      , m_eventManager(eventManager)
      , m_mappingHandler(mappingHandler)
      , m_isPressed(false)
      , m_axisThreshold(axisThreshold)
      , m_isAxisReversed(axisReversed)
   {
      if (m_eventManager)
         m_eventManager->Register(this);
   }

   ButtonMapping(ButtonMapping&& other) noexcept
      : m_deviceId(other.m_deviceId)
      , m_axisOrButtonId(other.m_axisOrButtonId)
      , m_eventManager(other.m_eventManager)
      , m_mappingHandler(other.m_mappingHandler)
      , m_isPressed(other.m_isPressed)
      , m_axisThreshold(other.m_axisThreshold)
      , m_isAxisReversed(other.m_isAxisReversed)
   {
      if (m_eventManager)
         m_eventManager->Register(this);
   }

   ButtonMapping& operator=(ButtonMapping&& other) noexcept
   {
      if (this != &other)
      {
         if (m_eventManager)
            m_eventManager->Unregister(this);
         m_deviceId = other.m_deviceId;
         m_axisOrButtonId = other.m_axisOrButtonId;
         m_eventManager = other.m_eventManager;
         m_mappingHandler = other.m_mappingHandler;
         m_isPressed = other.m_isPressed;
         m_axisThreshold = other.m_axisThreshold;
         m_isAxisReversed = other.m_isAxisReversed;
         m_buttonCaptureStartMs = other.m_buttonCaptureStartMs;
         m_buttonCapturePos = other.m_buttonCapturePos;
         if (m_eventManager)
            m_eventManager->Register(this);
      }
      return *this;
   }

   ~ButtonMapping()
   {
      if (m_eventManager)
         m_eventManager->Unregister(this);
   }

   bool operator==(const ButtonMapping& other) const
   {
      return m_eventManager == other.m_eventManager
         && m_mappingHandler == other.m_mappingHandler
         && m_deviceId == other.m_deviceId 
         && m_axisOrButtonId == other.m_axisOrButtonId
         && m_axisThreshold == other.m_axisThreshold
         && m_isAxisReversed == other.m_isAxisReversed;
   }
   
   static vector<ButtonMapping> Create(uint16_t deviceId, uint16_t buttonOrAxisId, float threshold = 0.f, bool axisReversed = false)
   {
      vector<ButtonMapping> mapping;
      mapping.emplace_back(nullptr, nullptr, deviceId, buttonOrAxisId, threshold, axisReversed);
      return mapping;
   }

   ButtonMapping With(ButtonInputEventManager* eventManager, ButtonMappingHandler* mappingHandler) const
   {
      return ButtonMapping(eventManager, mappingHandler, m_deviceId, m_axisOrButtonId, m_axisThreshold, m_isAxisReversed);
   }

   void SetAxisPosition(float position)
   {
      if (m_isAxisReversed)
         SetPressed(position <= m_axisThreshold);
      else
         SetPressed(position >= m_axisThreshold);
   }

   void SetPressed(bool pressed)
   {
      if (pressed != m_isPressed)
      {
         m_isPressed = pressed;
         if (m_mappingHandler)
            m_mappingHandler->OnInputChanged(this);
      }
   }

   bool IsPressed() const
   {
      return m_isPressed;
   }

   uint16_t GetDeviceId() const { return m_deviceId; }
   uint16_t GetAxisOrButtonId() const { return m_axisOrButtonId; }
   void SetAxisThreshold(float threshold) { m_axisThreshold = threshold; }
   float GetAxisThreshold() const { return m_axisThreshold; }
   void SetAxisReversed(bool reversed) { m_isAxisReversed = reversed; }
   bool IsAxisReversed() const { return m_isAxisReversed; } // if true, the button is pressed when axis value is below threshold

   // Temporary fields used during capture of analog axis as button mapping
   uint32_t m_buttonCaptureStartMs = 0;
   int m_buttonCapturePos = 0;

private:
   uint16_t m_deviceId;
   uint16_t m_axisOrButtonId;
   ButtonInputEventManager* m_eventManager;
   ButtonMappingHandler* m_mappingHandler;

   bool m_isPressed;
   float m_axisThreshold;
   bool m_isAxisReversed;
};
