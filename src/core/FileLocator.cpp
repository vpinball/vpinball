// license:GPLv3+

// Implementation of WinMain (Windows with UI) or main (Standalone)

#include "core/stdafx.h"

#include "FileLocator.h"

FileLocator::FileLocator()
   : m_appPath(EvaluateAppPath())
{
   SetupPrefPath();
   UpdateFileLayoutMode();
}

// Evaluate path of exe (without the exe's filename)
std::filesystem::path FileLocator::EvaluateAppPath()
{
   std::filesystem::path appPath;
#ifdef __ANDROID__
   // Android may not open files in the APK resources through fopen so we copy them outside of the apk in the internal storage
   appPath = std::filesystem::path(SDL_GetAndroidInternalStoragePath()) / "";
#elif defined(__APPLE__) && defined(TARGET_OS_IOS) && TARGET_OS_IOS && !defined(__LIBVPINBALL__)
   // Pref path is hidden on iOS, so we use Documents to be able to access/drag'n drop through Finder via UIFileSharingEnabled info.plist key
   // FIXME still app path is for readonly files, so shouldn't it just be SDL_GetBasePath() ?
   appPath = SDL_GetUserFolder(SDL_FOLDER_DOCUMENTS);
#else
   appPath = SDL_GetBasePath();
#endif
   return appPath;
}

void FileLocator::SetupPrefPath()
{
   std::filesystem::path basePrefPath;
#if defined(__ANDROID__)
   char* szPrefPath = SDL_GetPrefPath(NULL, "");
   basePrefPath = szPrefPath;
   SDL_free(szPrefPath);
#elif defined(__APPLE__) && defined(TARGET_OS_IOS) && TARGET_OS_IOS
   // Pref path is hidden on iOS, so we use Documents to be able to access/drag'n drop through Finder via UIFileSharingEnabled info.plist key
   basePrefPath = SDL_GetUserFolder(SDL_FOLDER_DOCUMENTS);
#else
   char* szPrefPath = SDL_GetPrefPath(nullptr, "VPinballX");
   basePrefPath = szPrefPath;
   SDL_free(szPrefPath);
#endif

   // Preference are stored per minor version (grouping all minor revision together, as minor revisions are not allowed to need user setup changes)
   const string versionPath(STR(VP_VERSION_MAJOR) "." STR(VP_VERSION_MINOR));
   m_prefPath = basePrefPath / versionPath;
   if (DirExists(m_prefPath))
      return;

   // We need to migrate, fix or setup the install for proper operation
   if (std::error_code ec; !std::filesystem::create_directories(m_prefPath, ec))
   {
      PLOGE << "Unable to create pref path: " << m_prefPath;
      return;
   }
   PLOGI << "New preference folder created: " << m_prefPath;

   // The user may have enabled the legacy file layout mode (ini along exe) and deleted the preference folder after upgrading to this version
   // In this situation, we do not want to do anything else than recreating the pref folder (which we just did)
   if (FileExists(GetAppPath(FileLocator::AppSubFolder::Root) / "VPinballX.ini"))
   {
      mINI::INIStructure m_ini;
      if (mINI::INIFile(GetAppPath(FileLocator::AppSubFolder::Root) / "VPinballX.ini").read(m_ini) && m_ini.has("Version"s) && m_ini["Version"s].has("VPinball"s))
      {
         const string existingVersionString = m_ini["Version"s]["VPinball"s];
         std::istringstream iss(existingVersionString);
         std::string token;
         int minor;
         int major;
         if ((iss >> token) && try_parse_int(token, major) && (major == VP_VERSION_MAJOR) && (iss >> token) && try_parse_int(token, minor) && (minor == VP_VERSION_MINOR))
            return;
      }
   }

   // Try to migrate settings from a previous install folder, we test:
   // - Folders corresponding to previous location, so %AppData%/VPinballX/major.minor
   // - Base %AppData%/VPinballX which was used before migrating to the 'major.minor' subfolder layout
   std::filesystem::path prevPref;
   for (int major = VP_VERSION_MAJOR; prevPref.empty() && major >= 10; major--)
   {
      for (int minor = (major == VP_VERSION_MAJOR ? VP_VERSION_MINOR : 9); prevPref.empty() && minor >= 0; minor--)
      {
         const string testVersion = std::format("{}.{}", major, minor);
         if (auto testPath = basePrefPath / testVersion; testPath != m_prefPath && DirExists(testPath))
            prevPref = testPath;
      }
   }
   if (prevPref.empty() && DirExists(basePrefPath))
      prevPref = basePrefPath;
   if (!prevPref.empty())
   {
      PLOGI << "Initializing new preference folder from previous install in: " << prevPref;
      for (const auto& entry : std::filesystem::recursive_directory_iterator(prevPref))
      {
         // Handle upgrading from settings stored in %AppData%/VPinballX and not in %AppData%/VPinballX/version, so the new folder is a child of the old one
         if (entry.path() == m_prefPath)
            continue;
         try
         {
            std::filesystem::path destPath = m_prefPath / std::filesystem::relative(entry.path(), prevPref);
            if (std::filesystem::is_directory(entry.status()))
               std::filesystem::create_directories(destPath);
            else
               std::filesystem::copy_file(entry.path(), destPath, std::filesystem::copy_options::overwrite_existing);
         }
         catch (const std::filesystem::filesystem_error& e)
         {
            PLOGE << "Error processing " << entry.path() << ": " << e.what();
         }
      }
   }

   // If we did not find a setting file to migrate but we have an old setting file along the exe, use it for the migration
   if (!FileExists(m_prefPath / "VPinballX.ini") && FileExists(GetAppPath(FileLocator::AppSubFolder::Root) / "VPinballX.ini"))
   {
      PLOGI << "Initializing preferences from old settings file: " << (GetAppPath(FileLocator::AppSubFolder::Root) / "VPinballX.ini");
      std::filesystem::copy_file(GetAppPath(FileLocator::AppSubFolder::Root) / "VPinballX.ini", m_prefPath / "VPinballX.ini");
   }

   // If we did not find any settings file, migrate settings from Windows registry (used before 10.8)
#ifdef _WIN32
   if (!FileExists(m_prefPath / "VPinballX.ini"))
   {
      PLOGI << "Initializing preferences from Window registry";
      mINI::INIStructure ini;
      static const vector<string> regKeys
         = { "Controller"s, "Editor"s, "Player"s, "PlayerVR"s, "RecentDir"s, "Version"s, "CVEdit"s, "TableOverride"s, "TableOption"s, "DefaultProps\\Bumper"s, "DefaultProps\\Decal"s,
              "DefaultProps\\EMReel"s, "DefaultProps\\Flasher"s, "DefaultProps\\Flipper"s, "DefaultProps\\Gate"s, "DefaultProps\\HitTarget"s, "DefaultProps\\Kicker"s, "DefaultProps\\Light"s,
              "DefaultProps\\LightSequence"s, "DefaultProps\\Plunger"s, "DefaultProps\\Primitive"s, "DefaultProps\\Ramp"s, "DefaultProps\\Rubber"s, "DefaultProps\\Spinner"s,
              "DefaultProps\\Wall"s, "DefaultProps\\Target"s, "DefaultProps\\TextBox"s, "DefaultProps\\Timer"s, "DefaultProps\\Trigger"s, "Defaults\\Camera"s };
      for (const string& regKey : regKeys)
      {
         string regpath = (regKey == "Controller"s ? "Software\\Visual Pinball\\"s : "Software\\Visual Pinball\\VP10\\"s) + regKey;
         HKEY hk;
         LSTATUS res = RegOpenKeyEx(HKEY_CURRENT_USER, regpath.c_str(), 0, KEY_READ, &hk);
         if (res != ERROR_SUCCESS)
            continue;
         for (DWORD Index = 0;; ++Index)
         {
            DWORD dwSize = MAX_PATH;
            TCHAR szName[MAX_PATH];
            res = RegEnumValue(hk, Index, szName, &dwSize, nullptr, nullptr, nullptr, nullptr);
            if (res == ERROR_NO_MORE_ITEMS)
               break;
            if (res != ERROR_SUCCESS || dwSize == 0 || szName[0] == '\0')
               continue;
            dwSize = MAXSTRING;
            BYTE pvalue[MAXSTRING];
            DWORD type = REG_NONE;
            res = RegQueryValueEx(hk, szName, nullptr, &type, pvalue, &dwSize);
            if (res != ERROR_SUCCESS)
               continue;
            if (type == REG_SZ)
            {
               string value((char*)pvalue);
               // old Win32xx and Win32xx 9+ docker keys
               if (value == "Dock Windows"s) // should not happen, as a folder, not value.. BUT also should save these somehow and restore for Win32++, or not ?
                  continue;
               if (value == "Dock Settings"s) // should not happen, as a folder, not value.. BUT also should save these somehow and restore for Win32++, or not ?
                  continue;
               ini[regKey][szName] = value;
            }
            else if (type == REG_DWORD)
               ini[regKey][szName] = std::to_string(*(DWORD*)pvalue);
         }
         RegCloseKey(hk);
      }
      mINI::INIFile file(m_prefPath / "VPinballX.ini");
      if (!file.write(ini, true))
      {
         PLOGE << "Failed to save imported settings.";
      }
   }
#endif
}

void FileLocator::SetPrefPath(const std::filesystem::path& path)
{
   m_prefPath = path;
   if (!DirExists(m_prefPath))
   {
      PLOGE << "Custom pref path is missing, aborting: " << m_prefPath;
      exit(1);
   }
   UpdateFileLayoutMode();
}

void FileLocator::UpdateFileLayoutMode()
{
   std::filesystem::path defaultPath = m_prefPath / "VPinballX.ini";
   std::filesystem::path appPath = m_appPath / "VPinballX.ini";
   if (FileExists(defaultPath))
      m_fileLayoutMode = FileLayoutMode::AppPrefData;
   else if (FileExists(appPath))
      m_fileLayoutMode = FileLayoutMode::AppOnly;
   else
      m_fileLayoutMode = FileLayoutMode::AppPrefData;
   PLOGI << "File layout mode set to " << (m_fileLayoutMode == FileLayoutMode::AppOnly ? "AppOnly" : "AppPrefData");
}

std::filesystem::path FileLocator::GetAppPath(AppSubFolder sub, const std::filesystem::path& file) const
{
   std::filesystem::path path;
   switch (sub)
   {
   // Readonly deployment files, always located along executable file
   case FileLocator::AppSubFolder::Root: path = m_appPath; break;
   case FileLocator::AppSubFolder::Assets: path = m_appPath / "assets"; break;
   case FileLocator::AppSubFolder::Plugins: path = m_appPath / "plugins"; break;
   case FileLocator::AppSubFolder::GLShaders: path = m_appPath / ("shaders-" + std::to_string(VP_VERSION_MAJOR) + '.' + std::to_string(VP_VERSION_MINOR) + '.' + std::to_string(VP_VERSION_REV)); break;
   case FileLocator::AppSubFolder::Docs:
      // A bit hacky as doc files have moved over time, so we check the various locations they may be in
      if (file.empty())
         path = m_appPath / "docs";
      else if (FileExists(m_appPath / file))
         path = m_appPath;
      else if (FileExists(m_appPath / "Doc" / file))
         path = m_appPath / "Doc";
      else if (FileExists(m_appPath / "docs" / file))
         path = m_appPath / "docs";
      break;

   // Scripts are special as the file is searched through a few different paths
   // Maybe we should change this to be a table relative path instead (searching through the usual path but also inside table folder)
   case FileLocator::AppSubFolder::Scripts:
      if (file.empty())
         // No file: just return the default read only core scripts folder
         return m_appPath / "scripts";
      else
         // Search for the script file, without a table specified
         return SearchScript(nullptr, file);
      break;

   // Application settings (read/write, not usually exposed to user)
   case FileLocator::AppSubFolder::Preferences: path = m_prefPath; break;

   // Read/write user documents
   case FileLocator::AppSubFolder::Tables:
      // This used to be a subfolder of the main application installation folder, but as this is not ok for most system, we simply go
      // to the system's defined user documents folder on first run. Later on, UI will use the last location visited.
      // Note: SDL_GetUserFolder returns NULL on Android (see SDL_sysfilesystem.c) and some Linux distros, so use app path instead (usually /data/data/org.vpinball.vpinball/files)
      {
         const char* docPath = SDL_GetUserFolder(SDL_FOLDER_DOCUMENTS);
         path = docPath == nullptr ? m_appPath : std::filesystem::path(docPath);
         break;
      }

   default: assert(false); break;
   }

   return file.empty() ? path : (path / file);
}

std::filesystem::path FileLocator::GetTablePath(const PinTable* table, TableSubFolder sub, bool searchForWriting) const
{
   std::filesystem::path path;
   auto withSubFolder = [&sub](const std::filesystem::path& basePath)
   {
      std::filesystem::path folder;
      switch (sub)
      {
      case TableSubFolder::Music: return basePath / "music"; break;
      case TableSubFolder::Cache: return basePath / "cache"; break;
      case TableSubFolder::User: return basePath / "user"; break;
      case TableSubFolder::AutoSave: return basePath / "autosave"; break;
      default: return basePath; break;
      }
   };

   switch (m_fileLayoutMode)
   {
   // App/Pref/Data mode: table files are stored along table (to avoid mixing everything together and store files in a place with write access)
   case FileLayoutMode::AppPrefData:
      if (table != nullptr && !table->m_filename.empty() && FileExists(table->m_filename))
      {
         path = withSubFolder(table->m_filename.parent_path());
         if (sub == TableSubFolder::Cache)
            path = path / table->m_title; // table's title is its file name without extension
         if (!DirExists(path))
         {
            if (searchForWriting)
            {
               std::filesystem::create_directories(path);
               string type;
               switch (sub)
               {
               case TableSubFolder::Music: type = "Music"s; break;
               case TableSubFolder::Cache: type = "Cache"s; break;
               case TableSubFolder::User: type = "User"s; break;
               }
               PLOGI << type << " folder was created for table '" << table->m_filename << "': " << path;
            }
            else
            {
               // Not found and path searched for read only, retry in app mode only which was the only file layout used before 10.8.1
               // This ease the transition, especially for music (if the user did not migrate it) and user folders (which will read for previous plays, but saved in new location)
               const_cast<FileLocator*>(this)->m_fileLayoutMode = FileLayoutMode::AppOnly;
               path = GetTablePath(table, sub, searchForWriting);
               const_cast<FileLocator*>(this)->m_fileLayoutMode = FileLayoutMode::AppPrefData;
            }
         }
      }
      break;

   // App mode: store everything inside the application path, needing write access to the application folder, defaulting to table then preference if folder has been removed after install
   case FileLayoutMode::AppOnly:
      switch (sub)
      {
      case TableSubFolder::Cache:
         // Cache is stored inside preference directory with a sub folder per table title
         if (table != nullptr)
         {
            path = GetAppPath(FileLocator::AppSubFolder::Preferences) / "Cache"s / table->m_title; // table's title is its file name without extension
            if (searchForWriting && !DirExists(path))
               std::filesystem::create_directories(path);
         }
         break;

      case TableSubFolder::User:
         // Shared user folder along the application path. This requires write access to the application path
         path = GetAppPath(FileLocator::AppSubFolder::Root) / "user"s;
         if (searchForWriting && !DirExists(path))
         {
            std::filesystem::create_directories(path);
            PLOGI << "User folder was created for table '" << (table ? table->m_filename : "null table") << "': " << path;
         }
         break;

      case TableSubFolder::AutoSave:
         // AutoSave along main application
         path = m_appPath;
         break;

      default:
         path = withSubFolder(m_appPath);
         if (!DirExists(path))
         {
            if (table == nullptr || table->m_filename.empty() || !FileExists(table->m_filename))
            {
               // No table: defaults to preference folder (even if it does not exist)
               path = withSubFolder(m_prefPath);
            }
            else
            {
               path = withSubFolder(table->m_filename.parent_path());
               if (!DirExists(path))
               {
                  path = withSubFolder(m_prefPath);
                  if (!DirExists(path))
                  {
                     // Not found: defaults to folder along table, not creating the dirs (backward compatibility behavior)
                     path = withSubFolder(table->m_filename.parent_path());
                  }
               }
            }
         }
         break;
      }
      break;
   }
   return path;
}

std::filesystem::path FileLocator::SearchScript(const PinTable* table, const std::filesystem::path& script) const
{
   // Search along the table path first
   if (table)
   {
      const auto tablePath = table->m_filename.parent_path();
      if (auto path = find_case_insensitive_file_path(tablePath / script); !path.empty())
         return path;
      if (auto path = find_case_insensitive_file_path(tablePath / "user"s / script); !path.empty())
         return path;
      if (auto path = find_case_insensitive_file_path(tablePath / "scripts"s / script); !path.empty())
         return path;
   }

   // Search in the application paths
   if (auto path = find_case_insensitive_file_path(m_appPath / script); !path.empty())
      return path;
   if (auto path = find_case_insensitive_file_path(m_appPath / "user"s / script); !path.empty())
      return path;
   if (auto path = find_case_insensitive_file_path(m_appPath / "scripts"s / script); !path.empty())
      return path;

   // Search in the preference paths
   if (auto path = find_case_insensitive_file_path(m_prefPath / script); !path.empty())
      return path;
   if (auto path = find_case_insensitive_file_path(m_prefPath / "user"s / script); !path.empty())
      return path;
   if (auto path = find_case_insensitive_file_path(m_prefPath / "scripts"s / script); !path.empty())
      return path;

   return std::filesystem::path();
}
