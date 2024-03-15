#include "finddup.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

#include "util.h"

// TODO: filter by extension
bool endsWith(const std::string &str, const std::string &suffix)
{
    if (str.length() < suffix.length())
        return false;
    return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

DupsTable finddup(const fs::path &dirPath, uintmax_t *dupSizeTotal, bool includeHidden, bool includeZeroSize, bool dupOnlyIfInSameDir)
{
	std::vector<std::pair<uintmax_t, fs::path>> fileEntries;

	for (const auto &entry: fs::recursive_directory_iterator(dirPath, fs::directory_options::skip_permission_denied)) {
		if (!includeHidden && entry.path().filename().string()[0] == '.')
			continue;

		bool isregfile;
		try {
			isregfile = fs::is_regular_file(entry.status());
		} catch (fs::filesystem_error &fse) {
			if (errno != ELOOP) { // if ELOOP ("too many symlinks") we can ignore
				std::cerr << "filesystem_error exception caught at " << __FILE__ << ", " << __LINE__ << ": " << fse.what() << "\n";
				perror("");
				exit(EXIT_FAILURE);
			}
		}
		if (isregfile) {
			uintmax_t fileSize = fs::file_size(entry);
			if (!includeZeroSize && fileSize == 0)
				continue;
			fileEntries.emplace_back(fileSize, entry.path());
		}
	}

	// fs::path not hashable so convert to string
	std::unordered_map<std::string, uintmax_t> sizeMap;
	for (auto &[size, path]: fileEntries)
		sizeMap[path.string()] = size;

	std::unordered_map<std::string, std::pair<uintmax_t, std::vector<fs::path>>> hashMap;
	for (const auto &[fileSize, filePath]: fileEntries) {
		std::string hash = calculateSHA256(filePath.string());
		if (!hash.empty()) {
			if (dupOnlyIfInSameDir)
				hash += filePath.parent_path().string();

			hashMap[hash].first = fileSize;
			hashMap[hash].second.push_back(filePath);
		}
	}

	DupsTable dupFiles;
	*dupSizeTotal = 0;
	for (const auto &[hash, sizePathList]: hashMap)
		if (sizePathList.second.size() > 1) {
			dupFiles.push_back({ hashMap[hash].first, hashMap[hash].second });
			*dupSizeTotal += hashMap[hash].first * (hashMap[hash].second.size() - 1);
		}

	std::sort(dupFiles.begin(), dupFiles.end(), 
			[](const std::pair<uintmax_t, std::vector<fs::path>> &a, const std::pair<uintmax_t, std::vector<fs::path>> &b) {
				return a.first > b.first;
			});

	return dupFiles;
}

void rmDups(const DupsTable &dupFiles)
{
	for (auto &[size, pathlist]: dupFiles) {
		for (size_t i = 1; i < pathlist.size(); ++i)
			std::cerr << pathlist[i] << "\n";
		std::cerr << "Delete " << (pathlist.size() > 2 ? "these files" : "this file") << " (y/n)? Size: " << fmtsize(size * (pathlist.size()-1)) << "\n";
		std::string resp;
		std::getline(std::cin, resp);
		std::cerr << "\n";
		if (resp == "Y" || resp == "y") {
			for (size_t i = 1; i < pathlist.size(); ++i)
				fs::remove(pathlist[i]);
		}
	}
}

