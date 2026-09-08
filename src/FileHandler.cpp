// ============================================================
//  FileHandler.cpp
//  Binary file I/O implementation.
//  All reads/writes use std::ios::binary to prevent any
//  line-ending or encoding transformation on any OS.
// ============================================================

#include "../include/FileHandler.h"
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <cstdint>   // for stat()

// ---------------------------------------------------------------------------
// readFile
//  Opens a file in binary mode and loads every byte into outData.
// ---------------------------------------------------------------------------
bool FileHandler::readFile(const std::string& path,
                           std::vector<unsigned char>& outData,
                           std::string& errorMsg) const
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        errorMsg = "Cannot open file for reading: " + path;
        return false;
    }

    auto size = file.tellg();
    if (size < 0) {
        errorMsg = "Failed to determine file size: " + path;
        return false;
    }

    outData.resize(static_cast<size_t>(size));
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(outData.data()), size);

    if (!file) {
        errorMsg = "Read error on file: " + path;
        return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// writeFile
//  Writes a byte vector to disk in binary mode.
// ---------------------------------------------------------------------------
bool FileHandler::writeFile(const std::string& path,
                            const std::vector<unsigned char>& data,
                            std::string& errorMsg) const
{
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
        errorMsg = "Cannot open file for writing: " + path;
        return false;
    }

    file.write(reinterpret_cast<const char*>(data.data()),
               static_cast<std::streamsize>(data.size()));

    if (!file) {
        errorMsg = "Write error on file: " + path;
        return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// fileExists
// ---------------------------------------------------------------------------
bool FileHandler::fileExists(const std::string& path)
{
    // Try stat() first
    struct stat st;
    if (stat(path.c_str(), &st) == 0) return true;

    // stat() fails on MSYS2 with spaces in path - use ifstream as fallback
    std::ifstream f(path.c_str(), std::ios::binary);
    return f.good();
}
// ---------------------------------------------------------------------------
// baseName
//  "path/to/photo.jpg"  →  "photo"
// ---------------------------------------------------------------------------
std::string FileHandler::baseName(const std::string& path)
{
    // Strip directory component
    size_t sep = path.find_last_of("/\\");
    std::string name = (sep == std::string::npos) ? path : path.substr(sep + 1);

    // Strip extension
    size_t dot = name.rfind('.');
    if (dot == std::string::npos) return name;
    return name.substr(0, dot);
}

// ---------------------------------------------------------------------------
// extension
//  "photo.jpg"  →  ".jpg"   |   "archive.tar.gz"  →  ".gz"
// ---------------------------------------------------------------------------
std::string FileHandler::extension(const std::string& path)
{
    size_t sep = path.find_last_of("/\\");
    std::string name = (sep == std::string::npos) ? path : path.substr(sep + 1);

    size_t dot = name.rfind('.');
    if (dot == std::string::npos) return "";
    return name.substr(dot); // includes the dot
}

// ---------------------------------------------------------------------------
// parseOriginalExtension
//  Reads the extension stored in the .enc header (without decrypting).
//  Header:  [8-byte magic][1-byte ext-len][ext-bytes][8-byte nonce][cipher...]
// ---------------------------------------------------------------------------
bool FileHandler::parseOriginalExtension(const std::vector<unsigned char>& encData,
                                         std::string& outExt,
                                         size_t& outPayloadOffset)
{
    const size_t MAGIC_LEN = 8;
    if (encData.size() < MAGIC_LEN + 1) return false;

    // Skip magic – CryptoManager validates it; here we just skip it.
    size_t pos = MAGIC_LEN;

    uint8_t extLen = encData[pos++];
    if (encData.size() < pos + extLen) return false;

    outExt = std::string(reinterpret_cast<const char*>(encData.data() + pos), extLen);
    outPayloadOffset = pos + extLen;
    return true;
}

std::string FileHandler::stripQuotes(const std::string& s)
{
    if (s.size() >= 2 && s.front() == '\'' && s.back() == '\'')
        return s.substr(1, s.size() - 2);
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"')
        return s.substr(1, s.size() - 2);
    return s;
}
