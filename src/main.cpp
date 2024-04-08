#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <chrono>
#include <openssl/evp.h>
#include <openssl/err.h>

#include "util.h"
#include "finddup.h"

/* TODO: histogram of filesize frequencies */

namespace fs = std::filesystem;

constexpr int MAX_WIDTH = 168;

void displayDups(const fs::path &dirPath, const DupsTable &dupFiles, uintmax_t dupSizeTotal)
{
	bool anyDupsFound = false;
	int longestPathWidth = fmin(longestPath(dupFiles) + 2, MAX_WIDTH);
	for (auto &[size, pathlist]: dupFiles) {
		std::cout << "Identical files, size " << fmtsize(size) << ":\n";
		for (auto &path: pathlist) {
			std::cout << "  " << std::left << std::setw(longestPathWidth) << path << std::setw(0) << "\t" << getLastWriteTimeStr(path) << "\n";
			anyDupsFound = true;
		}
	}
	if (anyDupsFound)
		std::cout << "\n=====================================================\nDuplicate files found wasting " << fmtsize(dupSizeTotal) << "\n";
	else
		std::cout << "No duplicate files found in " << dirPath << "\n";
}
// TODO: ask which file want to keep
void help(const std::string &progname)
{
	std::cerr << "Usage: " << progname << " [-0adhqs] <directory_path>\n";
	std::cerr << "Options:\n";
	std::cerr << "0: Don't skip files or directories of size 0\n";
	std::cerr << "a: Don't skip hidden files\n";
	std::cerr << "d: Delete duplicate files. For each group, deletes all but one. Prompts before every deletion. Must be used with -s option\n";
	std::cerr << "h: Shows this help text\n";
	std::cerr << "q: Quiet mode, i.e. don't output a list of duplicate files. Still prompts for each file deletion if -d option is used\n";
	std::cerr << "s: Require that 2 files be considered duplicates if they are also siblings (i.e., they are in the same directory)\n";
	std::cerr << "None of these options is mutually exclusive\n";
}

int main(int argc, char **argv)
{
	int idxDirPath = 1;
	bool includeHidden = false, 
				deleteDups = false,
				includeZeroSize = false,
				dupOnlyIfInSameDir = false,
				quiet = false;

	if (argc < 2) {
		help(argv[0]);
		return 1;
	}

	if (argv[1][0] == '-') {
		idxDirPath = 2;
		if (strchr(argv[1], '0'))
			includeZeroSize = true;
		if (strchr(argv[1], 'a'))
			includeHidden = true;
		if (strchr(argv[1], 'd'))
			deleteDups = true;
		if (strchr(argv[1], 's'))
			dupOnlyIfInSameDir = true;
		if (strchr(argv[1], 'q'))
			quiet = true;
		if (strchr(argv[1], 'h')) {
			help(argv[0]);
			return 0;
		}

		if (deleteDups && !dupOnlyIfInSameDir) {
			std::cerr << "-s option must be specified if -d option is used\n";
			return 1;
		}
	}

	fs::path dirPath(argv[idxDirPath]);
	if (!fs::exists(dirPath) || !fs::is_directory(dirPath)) {
		std::cerr << "Invalid directory path: " << dirPath << "\n";
		return 1;
	}

	uintmax_t wasted;

	std::cerr << "Search started...\n";
	auto start = std::chrono::high_resolution_clock::now();
	DupsTable dt;
	try {
		dt = finddup(dirPath, &wasted, includeHidden, includeZeroSize, dupOnlyIfInSameDir);
	} catch (fs::filesystem_error &fse) {
		std::cerr << "filesystem_error exception caught at " << __FILE__ << ", " << __LINE__ << ": " << fse.what() << "\n";
		return 1;
	}
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start); // microseconds

	if (!quiet)
		displayDups(dirPath, dt, wasted);
	std::cerr << "Search completed in " << round(duration.count() * 1e-6) << " s.\n";

	if (deleteDups && wasted > 0) {
		std::string resp;
		std::cerr << "Do you want to delete " << fmtsize(wasted) << " (y/n)? You will be prompted for each group. ";
		std::getline(std::cin, resp);
		std::cerr << "\n";
		if (resp == "Y" || resp == "y") {
			uintmax_t nbyDeleted = rmDups(dt);
			std::cerr << "Freed " << fmtsize(nbyDeleted) << "\n";
		} else
			std::cerr << "Deletion aborted\n";
	}

	return 0;
}

