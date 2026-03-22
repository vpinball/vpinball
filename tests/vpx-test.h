// license:GPLv3+

extern std::filesystem::path GetAssetPath();
extern bool CheckMatchingBitmaps(const string& filePath1, const string& filePath2);
extern void CaptureRender(const string& tablePath, const string& screenshotPath);

extern void ResetVPX();

#ifdef ENABLE_BGFX
bgfx::RendererType::Enum GetLastRenderer();
#endif
