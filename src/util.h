#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#include <string>

std::string fmtsize(uintmax_t sizeBytes);
void handleEVPError();
std::string calculateSHA256(const std::string &filePath);

#endif

