// license:GPLv3+

#pragma once

#include <filesystem>
#include <functional>

class ZipUtils {
public:
   typedef std::function<void(int current, int total, const char* filename)> ProgressCallback;

   static bool Zip(const std::filesystem::path& sourcePath, const std::filesystem::path& destPath, ProgressCallback callback = nullptr);
   static bool Unzip(const std::filesystem::path& sourcePath, const std::filesystem::path& destPath, ProgressCallback callback = nullptr);
};
