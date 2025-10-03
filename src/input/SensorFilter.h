// license:GPLv3+

#pragma once

// Utility classes to perform sensor data storage and filtering.
// 
// For data storage and preprocessing:
// - NoOpStoreSensorFilter: simply store and return the last provided sensor state
// - VelocityToAccelerationFilter: compute the first derivative of the input signal (for velocity based sensor to get acceleration, for example Pinscape devices)
// - PositionToAccelerationFilter: compute the second derivative of the input signal (for position based sensor to get acceleration, for example analog gamepads)
// 
// For signal filtering:
// - PlungerPositionFilter: a 4th order Butterworth IIR filter with a 10Hz passband for plunger position (used to be part of the Player class)
// - NudgeAccelerationFilter: a custom filter to suppress low frequency noise in nudge acceleration data (used to be implemented inside NudgeFilter class)
//
// All filters expect the Get method to be called with increasing timestamps in nanoseconds.
// Filters are designed to be used with the physics engine update rate (1ms).


// Base class for input filters: allow to get the value of the filter at the given timestamp
class SensorFilter
{
public:
   virtual ~SensorFilter() = default;
   virtual float Get(uint64_t timestampNs) = 0;

   // Allow to give a name for debugging purpose
   void SetName(const string& name) { m_name = name; }

protected:
   string m_name;
};


class ZeroSensorFilter final : public SensorFilter
{
public:
   ~ZeroSensorFilter() = default;
   float Get(uint64_t timestampNs) override { return 0.f; }
};


// Base class for input filters that store the data they process
class StoreSensorFilter : public SensorFilter
{
public:
   virtual ~StoreSensorFilter() = default;
   virtual void Push(uint64_t timestampNs, float value) = 0;
};


class NoOpStoreSensorFilter final : public StoreSensorFilter
{
public:
   ~NoOpStoreSensorFilter() = default;
   void Push(uint64_t timestampNs, float value) override { m_value = value; }
   float Get(uint64_t timestampNs) override { return m_value; }

private:
   float m_value = 0.f;
};


class VelocityToAccelerationFilter final : public StoreSensorFilter
{
public:
   ~VelocityToAccelerationFilter() = default;
   void Push(uint64_t timestampNs, float value) override;
   float Get(uint64_t timestampNs) override;

private:
   uint64_t m_filterTimestampMs = 0;
   float m_target = 0.f;
   float m_value = 0.f;
   float m_p[2] {};
};


class PositionToAccelerationFilter final : public StoreSensorFilter
{
public:
   ~PositionToAccelerationFilter() = default;
   void Push(uint64_t timestampNs, float value) override;
   float Get(uint64_t timestampNs) override;

private:
   uint64_t m_filterTimestampMs = 0;
   float m_target = 0.f;
   float m_subTarget = 0.f;
   float m_value = 0.f;
   float m_p[3] {};
};


// Base class for input filters that request the data they process from another filter
class SourcedSensorFilter : public SensorFilter
{
public:
   SourcedSensorFilter(std::shared_ptr<SensorFilter> source)
      : m_source(source)
   {
   }
   SourcedSensorFilter()
      : m_source(std::make_shared < ZeroSensorFilter>())
   {
   }
   virtual ~SourcedSensorFilter() = default;
   void SetSource(std::shared_ptr<SensorFilter> source) { m_source = source; }

protected:
   std::shared_ptr<SensorFilter> m_source;
};


class NoOpSensorFilter final : public SourcedSensorFilter
{
public:
   NoOpSensorFilter()
      : SourcedSensorFilter()
   {
   }
   NoOpSensorFilter(std::shared_ptr<SensorFilter> source)
      : SourcedSensorFilter(source)
   {
   }
   ~NoOpSensorFilter() = default;
   float Get(uint64_t timestampNs) override { return m_source->Get(timestampNs); }
};


// A simple Butterworth IIR filter with a 10Hz passband for filtered plunger position
class PlungerPositionFilter final : public SourcedSensorFilter
{
public:
   ~PlungerPositionFilter() = default;
   float Get(uint64_t timestampNs) override;

private:
   static constexpr const int IIR_Order = 4;

   // coefficients for IIR_Order Butterworth filter set to 10 Hz passband
   static constexpr float IIR_a[IIR_Order + 1] = {
      0.0048243445f,
      0.019297378f,
      0.028946068f,
      0.019297378f,
      0.0048243445f };

   static constexpr float IIR_b[IIR_Order + 1] = {
      1.00000000f, //if not 1 add division below
      -2.369513f,
      2.3139884f,
      -1.0546654f,
      0.1873795f };

   int m_init = IIR_Order;
   float m_x[IIR_Order + 1] = {};
   float m_y[IIR_Order + 1] = {};

   uint32_t m_filterTimestampMs = 0;
};


// A nudge acceleration filter to suppress low frequency noise
class NudgeAccelerationFilter final : public SourcedSensorFilter
{
public:
   ~NudgeAccelerationFilter() = default;
   float Get(uint64_t timestampNs) override;

private:
   // running total of samples
   float m_sum = 0.f;

   // previous sample
   float m_prv = 0.f;

   // timestamp of last zero crossing in the raw acceleration data
   uint64_t m_tzc = 0;

   // timestamp of last correction inserted into the data
   uint64_t m_tCorr = 0;

   // timestamp of last motion == start of rest
   uint64_t m_tMotion = 0;

   // last evaluated value and its timestamp
   uint32_t m_filterTimestampMs = 0;
   float m_value = 0.f;
};