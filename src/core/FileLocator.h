// license:GPLv3+

#pragma once


class FileLocator
{
public:
   FileLocator();

   enum class FileLayoutMode
   {
      // Layout files between 3 locations:
      // - Static app data in application installation folder/bundle (read only)
      // - Application settings in system provided preference folder with write access,
      // - Table files & folders along table with write access (works best when each table has its own folder)
      AppPrefData,
      // Store everything in application folder, mixing everything together, defaulting to table then preference folder if user has
      // deleted the base folder after installation. A missing folder will cause an error. This requires write access to the app
      // folder. This is the legacy behavior based on Windows 9x versions where applications had full access more or less everywhere,
      // but this causes quite a lot of issues & hacks with newer operating systems and platforms which are more restrictive.
      AppOnly,
   };
   void UpdateFileLayoutMode(); // File layout mode is defined depending on where the application ini file is located, defaulting to AppPrefData mode if it is missing
   FileLayoutMode GetFileLayoutMode() const { return m_fileLayoutMode; }

   // Application subfolders for core files like 'assets', 'scripts', 'plugins',...
   enum class AppSubFolder
   {
      // Read only ressources located in the installation folder/bundle/...
      Root,
      Assets,
      Scripts, // core script folder
      Plugins,
      Docs,
      GLShaders,
      // Read/write user documents
      Tables,
      Preferences,
   };
   std::filesystem::path GetAppPath(AppSubFolder sub, const std::filesystem::path& file = std::filesystem::path()) const;

   // Table folders for things like 'music', 'cache', 'user' (to save highscore),...
   enum class TableSubFolder
   {
      Root,
      Music, // 'music' folder along table file, used by PlayMusic() script function
      Cache, // 'cache/table_name/' folder along table file, used to cache player data (used textures to be loaded in GPU mem, compressed textures,...)
      User, // 'user' folder along table file, used to save user data (highscores, settings,...)
      AutoSave,
   };
   std::filesystem::path GetTablePath(const PinTable* table, TableSubFolder sub, bool searchForWriting) const;

   std::filesystem::path SearchScript(const PinTable* table, const std::filesystem::path& script) const;

   // Allow to change the preference r/w folder used for user settings
   void SetPrefPath(const std::filesystem::path& path);
   void SetTablesPath(const std::filesystem::path& path);

private:
   FileLayoutMode m_fileLayoutMode = FileLayoutMode::AppOnly;

   static std::filesystem::path EvaluateAppPath();
   const std::filesystem::path m_appPath; // The read only application path where assets, core scripts, GL shaders may be found

   void SetupPrefPath();
   std::filesystem::path m_prefPath; // The preferences path where user settings and all sort of dynamic datas are stored
   std::filesystem::path m_tablesPath;  // Used to override the default table path
};

