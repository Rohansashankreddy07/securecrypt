#pragma once
// ============================================================
//  FileHandler.h
//  Binary file I/O for ALL file types.
//  Data is always handled as vector<unsigned char> so that
//  binary content (images, audio, etc.) is never corrupted.
// ============================================================

#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <string>
#include <vector>

class FileHandler {
public:
    FileHandler()  = default;
    ~FileHandler() = default;

    // ---- Core I/O ----

    // Reads entire file into a byte vector.
    // Returns true on success; sets errorMsg on failure.
    bool readFile(const std::string& path,
                  std::vector<unsigned char>& outData,
                  std::string& errorMsg) const;

    // Writes a byte vector to disk (creates / overwrites).
    // Returns true on success; sets errorMsg on failure.
    bool writeFile(const std::string& path,
                   const std::vector<unsigned char>& data,
                   std::string& errorMsg) const;

    // ---- Utility ----

    // Returns true if the file exists and is readable.
    static bool fileExists(const std::string& path);
    static std::string stripQuotes(const std::string& s);

    // Extracts the base name without extension, e.g. "photo" from "photo.jpg"
    static std::string baseName(const std::string& path);

    // Extracts the extension including the dot, e.g. ".jpg"
    static std::string extension(const std::string& path);

    // Derives the original filename stored inside the .enc file header.
    // The header format is:  [1 byte ext-len][ext bytes][...payload...]
    static bool parseOriginalExtension(const std::vector<unsigned char>& encData,
                                       std::string& outExt,
                                       size_t& outPayloadOffset);
};

#endif // FILEHANDLER_H
