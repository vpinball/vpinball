// license:GPLv3+

#include "core/stdafx.h"
#include "SensorMapping.h"

#include <cmath>


float SensorMapping::GetMainFrequency()
{
   if (m_fftFuture.valid())
   {
      // Wait for the FFT result
      if (m_fftFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
         m_lastMainFreq = m_fftFuture.get();
   }
   if (m_fftCaptureBuffer)
   {
      // A capture is in progress, just let it continue
   }
   else
   {
      // Nothing in progress, start a new capture
      m_fftCaptureBuffer = std::make_unique<std::valarray<std::complex<double>>>(1024);
      m_fftCapturePosition = 0;
   }
   return m_lastMainFreq;
}

// Cooley-Tukey FFT algorithm (recursive)
static void fft(std::valarray<std::complex<double>>& x)
{
   const size_t N = x.size();
   if (N <= 1)
      return;

   // Divide
   std::valarray even = x[std::slice(0, N / 2, 2)];
   std::valarray odd = x[std::slice(1, N / 2, 2)];

   // Conquer
   fft(even);
   fft(odd);

   // Combine
   for (size_t k = 0; k < N / 2; ++k)
   {
      std::complex<double> t = std::polar(1.0, -2.0 * M_PI * static_cast<double>(k) / static_cast<double>(N)) * odd[k];
      x[k] = even[k] + t;
      x[k + N / 2] = even[k] - t;
   }
}

void SensorMapping::CaptureFFT()
{
   assert(m_fftCaptureBuffer);
   uint64_t sampleMs = m_rawValueTimestampNs / 1000000ULL;

   if (m_fftCapturePosition == 0)
   {
      m_fftCaptureHeadTimestampMs = sampleMs;
      (*m_fftCaptureBuffer)[m_fftCapturePosition] = m_rawValue;
      m_fftCapturePosition++;
      return;
   }

   if (sampleMs == m_fftCaptureHeadTimestampMs)
   {
      (*m_fftCaptureBuffer)[m_fftCapturePosition] = m_rawValue;
      return;
   }

   assert(sampleMs > m_fftCaptureHeadTimestampMs);
   const int nSteps = static_cast<int>(sampleMs - m_fftCaptureHeadTimestampMs);
   const float prevValue = static_cast<float>((*m_fftCaptureBuffer)[m_fftCapturePosition].real());
   for (int i = 0; i < nSteps && m_fftCapturePosition < m_fftCaptureBuffer->size(); i++)
   {
      (*m_fftCaptureBuffer)[m_fftCapturePosition] = lerp(prevValue, m_rawValue, static_cast<float>(i + 1) / static_cast<float>(nSteps));
      m_fftCapturePosition++;
   }
   m_fftCaptureHeadTimestampMs = sampleMs;

   if (m_fftCapturePosition == m_fftCaptureBuffer->size())
   {
      //for (int i = 0; i < m_fftCaptureBuffer->size(); i++)
      //   (*m_fftCaptureBuffer)[i] = sin(16.f * 2.f * M_PI * i / static_cast<float>(m_fftCaptureBuffer->size()));
      auto processFFT = [](std::unique_ptr<std::valarray<std::complex<double>>> samples)
      {
         fft(*samples.get());
         double maxVal = 0.0;
         float maxFreq = 0.f;
         for (int i = 1; i < samples->size() / 2; i++)
         {
            double val = (*samples)[i].real() * (*samples)[i].real()+(*samples)[i].imag() * (*samples)[i].imag();
            if (val > maxVal)
            {
               maxVal = val;
               maxFreq = (static_cast<float>(i) * 1000.f) / static_cast<float>(samples->size());
            }
         }
         //PLOGD << "FFT main frequency: " << maxFreq << " Hz";
         return maxFreq;
      };
      m_fftFuture = std::async(std::launch::async, processFFT, std::move(m_fftCaptureBuffer));
   }
}
