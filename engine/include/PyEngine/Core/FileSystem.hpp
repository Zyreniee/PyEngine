#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace PyEngine {
namespace FileSystem {

std::vector<char> ReadBinaryFile(const std::string &filepath);
std::string ReadTextFile(const std::string &filepath);
bool FileExists(const std::string &filepath);
size_t GetFileSize(const std::string &filepath);
std::filesystem::path GetAbsolutePath(const std::string &filepath);
std::filesystem::path GetExecutableDirectory();

} // namespace FileSystem
} // namespace PyEngine
