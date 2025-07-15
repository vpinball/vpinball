// license:GPLv3+

extern string GetAssetPath();
extern bool CheckMatchingBitmaps(const string& filePath1, const string& filePath2);
extern void CaptureRender(const string& tablePath, const string& screenshotPath);

extern void ResetVPX();
extern void AddOnPrepareFrameHandler(msgpi_msg_callback onPrepareFrame, void* context);
extern void RemoveOnPrepareFrameHandler(msgpi_msg_callback onPrepareFrame);

#ifdef ENABLE_BGFX
bgfx::RendererType::Enum GetLastRenderer();
#endif
