#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#include <string>
#include <filesystem>

std::string fmtsize(uintmax_t sizeBytes);
void handleEVPError();
std::string calculateSHA256(const std::string &filePath);
template <typename TP> std::time_t to_time_t(TP tp);
std::string getLastWriteTimeStr(const std::filesystem::path &p);

#endif

