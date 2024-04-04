#include "util.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>

#include <openssl/evp.h>
#include <openssl/err.h>

std::string fmtstr(double x, double prec=2)
{
	if (floor(x) == x)
		prec = 0;
	std::string s = std::to_string(x);
	return s.substr(0, s.find(".") + prec);
}

std::string fmtsize(uintmax_t sizeBytes)
{
	if (1024 > sizeBytes)
		return fmtstr(sizeBytes) + " B";
	if (1024*1024 > sizeBytes)
		return fmtstr((double) sizeBytes / 1024.) + " KiB";
	if (1024*1024*1024 > sizeBytes)
		return fmtstr((double) sizeBytes / (1024.*1024.)) + " MiB";

	return fmtstr((double) sizeBytes / (1024.*1024.*1024.)) + " GiB";
}

void handleEVPError()
{
	unsigned long errCode = ERR_get_error();
	char errString[256];
	ERR_error_string_n(errCode, errString, sizeof(errString));
	std::cerr << "OpenSSL error: " << errString << "\n";
}

// calculate SHA-256 hash of a file using OpenSSL EVP interface
std::string calculateSHA256(const std::string &filePath)
{
	std::ifstream file(filePath, std::ifstream::binary);
	if (!file.is_open()) {
		if (errno != ENOENT) /* OK to ignore "file not found" error: probably recursive_directory_iterator 
							finding temporary file that is deleted by the time we get here */
			std::cerr << "Read error: " << filePath << ": " << strerror(errno) << "--continuing as normal...\n";
		return "";
	}

	EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
	if (mdctx == nullptr) {
		handleEVPError();
		return "";
	}

	const EVP_MD *md = EVP_sha256();

	if (EVP_DigestInit_ex(mdctx, md, nullptr) != 1) {
		handleEVPError();
		EVP_MD_CTX_free(mdctx);
		return "";
	}

	char buffer[4096];
	while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
		if (EVP_DigestUpdate(mdctx, buffer, file.gcount()) != 1) {
			handleEVPError();
			EVP_MD_CTX_free(mdctx);
			return "";
		}
	}
	unsigned char hash[EVP_MAX_MD_SIZE];
	unsigned hashLen = 0;
	if (EVP_DigestFinal_ex(mdctx, hash, &hashLen) != 1) {
		handleEVPError();
		EVP_MD_CTX_free(mdctx);
		return "";
	}

	EVP_MD_CTX_free(mdctx);

	std::string hashStr;
	for (unsigned i = 0; i < hashLen; ++i) {
		char buf[4];
		snprintf(buf, sizeof(buf), "%02x", hash[i]);
		hashStr += buf;
	}

	return hashStr;
}

/* from https://stackoverflow.com/a/58237530 */
template <typename TP> std::time_t to_time_t(TP tp)
{
	using namespace std::chrono;
	auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now() + system_clock::now());
	return system_clock::to_time_t(sctp);
}

std::string getLastWriteTimeStr(const std::filesystem::path &p)
{
	time_t tt = to_time_t(std::filesystem::last_write_time(p));
	tm *tm = localtime(&tt);
	std::stringstream buffer;
	buffer << std::put_time(tm, "%F %X");
	return buffer.str();
}

