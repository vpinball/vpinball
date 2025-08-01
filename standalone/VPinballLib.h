#pragma once

#include <string>
#include <functional>
#include <queue>
#include <mutex>

class WebServer;

namespace VPinballLib {

enum class LogLevel {
   Debug,
   Info,
   Warn,
   Error
};

enum class VPinballStatus {
   Success,
   Failure
};

enum class ScriptErrorType {
   Compile,
   Runtime
};

enum class OptionUnit {
   NoUnit,
   Percent
};

enum class Event {
   ArchiveUncompressing,
   ArchiveCompressing,
   LoadingItems,
   LoadingSounds,
   LoadingImages,
   LoadingFonts,
   LoadingCollections,
   Play,
   CreatingPlayer,
   WindowCreated,
   Prerendering,
   PlayerStarted, 
   Rumble,
   ScriptError,
   LiveUIToggle,
   LiveUIUpdate,
   PlayerClosing,
   PlayerClosed, 
   Stopped,
   WebServer,
   CaptureScreenshot,
   TableList,
   TableImport,
   TableRename,
   TableDelete
};

struct ProgressData {
   int progress;
};

struct WindowCreatedData {
   void* pWindow;
   const char* pTitle;
};

struct RumbleData {
   uint16_t lowFrequencyRumble;
   uint16_t highFrequencyRumble;
   uint32_t durationMs;
};

struct ScriptErrorData {
   ScriptErrorType error;
   int line;
   int position;
   const char* description;
};

struct WebServerData {
   const char* url;
};

struct CaptureScreenshotData {
   bool success;
};

struct TableInfo {
   char* tableId;
   char* name;
};

struct TablesData {
   TableInfo* tables;
   int tableCount;
   bool success;
};

struct TableEventData {
   const char* tableId;
   const char* newName;
   const char* path;
   bool success;
};

struct TableOptions {
   float globalEmissionScale;
   float globalDifficulty;
   float exposure;
   int toneMapper;
   int musicVolume;
   int soundVolume;
};

struct CustomTableOption {
   const char* sectionName;
   const char* id;
   const char* name;
   int showMask;
   float minValue;
   float maxValue;
   float step;
   float defaultValue;
   OptionUnit unit;
   const char* literals;
   float value;
};

struct ViewSetup {
   int viewMode;
   float sceneScaleX;
   float sceneScaleY;
   float sceneScaleZ;
   float viewX;
   float viewY;
   float viewZ;
   float lookAt;
   float viewportRotation;
   float fov;
   float layback;
   float viewHOfs;
   float viewVOfs;
   float windowTopZOfs;
   float windowBottomZOfs;
};

class VPinball {
public:
   void LoadPlugins();
   void UnloadPlugins();
   static VPinball& GetInstance() { return s_instance; }
   void SetGameLoop(std::function<void()> gameLoop) { m_gameLoop = gameLoop; }
   static void* SendEvent(Event event, void* data);
   void Init(std::function<void*(Event, void*)> callback);
   string GetVersionStringFull();
   void Log(LogLevel level, const string& message);
   void ResetLog();
   int LoadValueInt(const string& sectionName, const string& key, int defaultValue);
   float LoadValueFloat(const string& sectionName, const string& key, float defaultValue);
   string LoadValueString(const string& sectionName, const string& key, const string& defaultValue);
   void SaveValueInt(const string& sectionName, const string& key, int value);
   void SaveValueFloat(const string& sectionName, const string& key, float value);
   void SaveValueString(const string& sectionName, const string& key, const string& value);
   VPinballStatus Uncompress(const string& source);
   VPinballStatus Compress(const string& source, const string& destination);
   void UpdateWebServer();
   VPinballStatus ResetIni();
   VPinballStatus Load(const string& source);
   VPinballStatus ExtractScript(const string& source);
   VPinballStatus Play();
   VPinballStatus Stop();
   void SetPlayState(int enable);
   void ToggleFPS();
   void GetTableOptions(TableOptions& tableOptions);
   void SetTableOptions(const TableOptions& tableOptions);
   void SetDefaultTableOptions();
   void ResetTableOptions();
   void SaveTableOptions();
   int GetCustomTableOptionsCount();
   void GetCustomTableOption(int index, CustomTableOption& customTableOption);
   void SetCustomTableOption(const CustomTableOption& customTableOption);
   void SetDefaultCustomTableOptions();
   void ResetCustomTableOptions();
   void SaveCustomTableOptions();
   void GetViewSetup(ViewSetup& viewSetup);
   void SetViewSetup(const ViewSetup& viewSetup);
   void SetDefaultViewSetup();
   void ResetViewSetup();
   void SaveViewSetup();
   void CaptureScreenshot(const string& filename);
   void SetWebServerUpdated();

private:
   VPinball();
   static void GameLoop(void* pUserData);
   void ProcessSetTableOptions(const TableOptions& tableOptions);
   void ProcessSetDefaultTableOptions();
   void ProcessResetTableOptions();
   void ProcessSetCustomTableOption(const CustomTableOption& customTableOption);
   void ProcessSetDefaultCustomTableOptions();
   void ProcessResetCustomTableOptions();
   void ProcessSetViewSetup(const ViewSetup& setup);
   void ProcessSetDefaultViewSetup();
   void ProcessResetViewSetup();
   void Cleanup();

   vector<std::shared_ptr<MsgPlugin>> m_plugins;
   std::queue<std::function<void()>> m_liveUIQueue;
   std::mutex m_liveUIMutex;
   std::function<void*(Event, void*)> m_eventCallback;
   std::function<void()> m_gameLoop;
   WebServer* m_pWebServer;

   static VPinball s_instance;
};

} // namespace VPinballLib
