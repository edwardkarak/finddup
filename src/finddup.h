#ifndef FINDDUP_H_INCLUDED
#define FINDDUP_H_INCLUDED

#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

using DupsTable = std::vector<std::pair<uintmax_t, std::vector<fs::path>>>;

DupsTable finddup(const fs::path &dirPath, uintmax_t *dupSizeTotal, bool includeHidden, bool includeZeroSize, bool dupOnlyIfInSameDir);
uintmax_t rmDups(const DupsTable &dupFiles);
int longestPath(const DupsTable &dt);

#endif

