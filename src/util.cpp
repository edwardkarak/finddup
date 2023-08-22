#include "util.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

#include <openssl/evp.h>
#include <openssl/err.h>

std::string fmtsize(uintmax_t sizeBytes)
{
	if (1024 > sizeBytes)
		return std::to_string(sizeBytes) + " B";
	if (1024*1024 > sizeBytes)
		return std::to_string((double) sizeBytes / 1024.) + " KiB";
	if (1024*1024*1024 > sizeBytes)
		return std::to_string((double) sizeBytes / (1024.*1024.)) + " MiB";

	return std::to_string((double) sizeBytes / (1024.*1024.*1024.)) + " GiB";
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
			std::cerr << "Read error: " << filePath << ": " << strerror(errno) << "\n";
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
