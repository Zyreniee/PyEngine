#include "PyEngine/Core/FileSystem.hpp"
#include "PyEngine/Core/Log.hpp"
#include <fstream>
#include <sstream>

#ifdef __linux__
#include <limits.h>
#include <unistd.h>
#endif

namespace PyEngine {
namespace FileSystem {

std::vector<char> ReadBinaryFile(const std::string &filepath) {
  std::ifstream file(filepath, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    PYENGINE_CORE_ERROR("Failed to open file: {}", filepath);
    return {};
  }

  size_t fileSize = static_cast<size_t>(file.tellg());
  std::vector<char> buffer(fileSize);

  file.seekg(0);
  file.read(buffer.data(), fileSize);
  file.close();

  return buffer;
}

std::string ReadTextFile(const std::string &filepath) {
  std::ifstream file(filepath);

  if (!file.is_open()) {
    PYENGINE_CORE_ERROR("Failed to open file: {}", filepath);
    return "";
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

bool FileExists(const std::string &filepath) {
  return std::filesystem::exists(filepath);
}

size_t GetFileSize(const std::string &filepath) {
  if (!FileExists(filepath)) {
    return 0;
  }
  return std::filesystem::file_size(filepath);
}

std::filesystem::path GetAbsolutePath(const std::string &filepath) {
  return std::filesystem::absolute(filepath);
}

std::filesystem::path GetExecutableDirectory() {
#ifdef __linux__
  char result[PATH_MAX];
  ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
  if (count != -1) {
    std::filesystem::path exePath(std::string(result, count));
    return exePath.parent_path();
  }
#endif
  return std::filesystem::current_path();
}

} // namespace FileSystem
} // namespace PyEngine
