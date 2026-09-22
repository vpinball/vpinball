// license:GPLv3+

#include "ui/VPXFileFeedback.h"

extern std::filesystem::path GetAssetPath();
extern bool CheckMatchingBitmaps(const string& filePath1, const string& filePath2);
extern void CaptureRender(const string& tablePath, const string& screenshotPath);

extern void ResetVPX();

// Records the progress reported while loading or saving a table, so tests can verify
// that it grows monotonically and reaches the announced total length.
class TestFileFeedback final : public VPXFileFeedback
{
public:
   void SetProgress(unsigned int progress) override
   {
      m_isMonotonic &= (m_progressCalls == 0) || (progress >= m_lastProgress);
      m_lastProgress = progress;
      ++m_progressCalls;
   }
   void SetLength(unsigned int length) override
   {
      // Each reported operation starts with SetLength: reset the recorded progress
      m_length = length;
      m_lastProgress = 0;
      m_progressCalls = 0;
      m_isMonotonic = true;
   }

   unsigned int m_length = 0;
   unsigned int m_lastProgress = 0;
   unsigned int m_progressCalls = 0;
   bool m_isMonotonic = true;
};

#ifdef ENABLE_BGFX
bgfx::RendererType::Enum GetLastRenderer();
#endif
