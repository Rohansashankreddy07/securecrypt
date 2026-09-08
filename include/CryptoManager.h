#pragma once
// ============================================================
//  CryptoManager.h
//  Wraps libsodium ChaCha20-Poly1305 AEAD cipher.
//
//  .enc file layout (little-endian):
//  ┌─────────────────────────────────────────────────────────┐
//  │  8 bytes  │  magic "SECENC\0\0"                         │
//  │  1 byte   │  original extension length (N)              │
//  │  N bytes  │  original extension string (e.g. ".jpg")    │
//  │  8 bytes  │  nonce  (crypto_aead_chacha20poly1305_NPUBBYTES = 8) │
//  │  rest     │  ciphertext + 16-byte Poly1305 MAC tag      │
//  └─────────────────────────────────────────────────────────┘
//
//  Key: 32 bytes (crypto_aead_chacha20poly1305_KEYBYTES)
//  Displayed / stored as 64-character lowercase hex string.
// ============================================================

#ifndef CRYPTOMANAGER_H
#define CRYPTOMANAGER_H

#include <string>
#include <vector>

class CryptoManager {
public:
    CryptoManager();
    ~CryptoManager() = default;

    // ---- Key management ----

    // Generates a cryptographically-random 32-byte key and
    // stores it internally; returns the key as a hex string.
    std::string generateKey();

    // Loads key from a hex string (64 hex chars → 32 bytes).
    // Returns false if the string is malformed.
    bool loadKeyFromHex(const std::string& hexKey);

    // Saves the current key as a hex string to a file.
    // Returns false if the write fails.
    bool saveKeyToFile(const std::string& path) const;

    // Loads key from a file (must contain exactly 64 hex chars).
    bool loadKeyFromFile(const std::string& path);

    // ---- Encryption / Decryption ----

    // Encrypts plaintext and writes the .enc format described above.
    // originalExt: the dot-prefixed extension of the source file (e.g. ".jpg")
    // Returns true on success; sets errorMsg on failure.
    bool encryptFile(const std::vector<unsigned char>& plaintext,
                     const std::string& originalExt,
                     std::vector<unsigned char>& cipherOutput,
                     std::string& errorMsg);

    // Decrypts a .enc buffer back to the original bytes.
    // Returns true on success; sets errorMsg on failure.
    // outOriginalExt receives the original extension.
    bool decryptFile(const std::vector<unsigned char>& encData,
                     std::vector<unsigned char>& plaintextOutput,
                     std::string& outOriginalExt,
                     std::string& errorMsg);

    // ---- Helpers ----
    std::string currentKeyHex() const;
    bool        hasKey()        const { return m_keyLoaded; }

private:
    static const size_t KEY_BYTES   = 32; // crypto_aead_chacha20poly1305_KEYBYTES
    static const size_t NONCE_BYTES = 8;  // crypto_aead_chacha20poly1305_NPUBBYTES
    static const size_t MAC_BYTES   = 16; // crypto_aead_chacha20poly1305_ABYTES
    static const char   MAGIC[8];

    unsigned char m_key[32];
    bool          m_keyLoaded;

    // Convert between hex and raw bytes
    static std::string  bytesToHex(const unsigned char* data, size_t len);
    static bool         hexToBytes(const std::string& hex,
                                   unsigned char* out, size_t expectedLen);
};

#endif // CRYPTOMANAGER_H
