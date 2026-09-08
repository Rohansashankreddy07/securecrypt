// ============================================================
//  CryptoManager.cpp
//  ChaCha20-Poly1305 AEAD encryption via libsodium.
//
//  .enc file layout:
//   Offset  Size   Description
//   ------  ----   -----------
//      0      8    Magic bytes: "SECENC\0\0"
//      8      1    Original extension length (N)
//      9      N    Original extension (e.g. ".jpg")
//    9+N      8    Random nonce
//   17+N     var   ChaCha20 ciphertext + 16-byte Poly1305 tag
//
//  The nonce is randomly generated each time we encrypt, which
//  means encrypting the same file twice yields different output.
//  This is correct AEAD usage.
// ============================================================

#include "../include/CryptoManager.h"
#include <sodium.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <stdexcept>

// Magic identifier for our .enc format
const char CryptoManager::MAGIC[8] = {'S','E','C','E','N','C','\0','\0'};

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
CryptoManager::CryptoManager()
    : m_keyLoaded(false)
{
    // Zero out key memory for safety
    sodium_memzero(m_key, KEY_BYTES);
}

// ---------------------------------------------------------------------------
// generateKey
//  Uses libsodium's CSPRNG to fill a 32-byte key.
//  Stores internally and returns hex representation.
// ---------------------------------------------------------------------------
std::string CryptoManager::generateKey()
{
    // crypto_aead_chacha20poly1305_keygen writes exactly KEY_BYTES bytes
    crypto_aead_chacha20poly1305_keygen(m_key);
    m_keyLoaded = true;
    return bytesToHex(m_key, KEY_BYTES);
}

// ---------------------------------------------------------------------------
// loadKeyFromHex
// ---------------------------------------------------------------------------
bool CryptoManager::loadKeyFromHex(const std::string& hexKey)
{
    if (hexKey.size() != KEY_BYTES * 2) return false;
    if (!hexToBytes(hexKey, m_key, KEY_BYTES)) return false;
    m_keyLoaded = true;
    return true;
}

// ---------------------------------------------------------------------------
// saveKeyToFile
// ---------------------------------------------------------------------------
bool CryptoManager::saveKeyToFile(const std::string& path) const
{
    if (!m_keyLoaded) return false;
    std::ofstream f(path, std::ios::trunc);
    if (!f.is_open()) return false;
    f << bytesToHex(m_key, KEY_BYTES) << "\n";
    return static_cast<bool>(f);
}

// ---------------------------------------------------------------------------
// loadKeyFromFile
// ---------------------------------------------------------------------------
bool CryptoManager::loadKeyFromFile(const std::string& path)
{
    std::ifstream f(path);
    if (!f.is_open()) return false;
    std::string hex;
    f >> hex;
    return loadKeyFromHex(hex);
}

// ---------------------------------------------------------------------------
// currentKeyHex
// ---------------------------------------------------------------------------
std::string CryptoManager::currentKeyHex() const
{
    if (!m_keyLoaded) return "(no key loaded)";
    return bytesToHex(m_key, KEY_BYTES);
}

// ---------------------------------------------------------------------------
// encryptFile
//  Builds the .enc blob in memory and returns it via cipherOutput.
// ---------------------------------------------------------------------------
bool CryptoManager::encryptFile(const std::vector<unsigned char>& plaintext,
                                 const std::string& originalExt,
                                 std::vector<unsigned char>& cipherOutput,
                                 std::string& errorMsg)
{
    if (!m_keyLoaded) {
        errorMsg = "No encryption key loaded. Generate or load a key first.";
        return false;
    }
    if (originalExt.size() > 255) {
        errorMsg = "File extension too long.";
        return false;
    }

    // --- Generate a fresh random nonce ---
    unsigned char nonce[NONCE_BYTES];
    randombytes_buf(nonce, NONCE_BYTES);

    // --- Compute sizes ---
    uint8_t extLen            = static_cast<uint8_t>(originalExt.size());
    size_t  headerSize        = sizeof(MAGIC)           // 8
                              + 1                       // extLen byte
                              + extLen                  // extension string
                              + NONCE_BYTES;            // 8
    size_t  ciphertextMaxLen  = plaintext.size() + MAC_BYTES;
    cipherOutput.resize(headerSize + ciphertextMaxLen);

    // --- Write header ---
    size_t pos = 0;
    std::memcpy(cipherOutput.data() + pos, MAGIC, sizeof(MAGIC));
    pos += sizeof(MAGIC);

    cipherOutput[pos++] = extLen;

    if (extLen > 0) {
        std::memcpy(cipherOutput.data() + pos, originalExt.data(), extLen);
        pos += extLen;
    }

    std::memcpy(cipherOutput.data() + pos, nonce, NONCE_BYTES);
    pos += NONCE_BYTES;

    // --- Encrypt ---
    unsigned long long actualCipherLen = 0;
    int rc = crypto_aead_chacha20poly1305_encrypt(
        cipherOutput.data() + pos,   // ciphertext output
        &actualCipherLen,            // actual length written
        plaintext.data(),            // plaintext input
        plaintext.size(),            // plaintext length
        nullptr, 0,                  // additional data (none)
        nullptr,                     // nsec (unused in this variant)
        nonce,
        m_key
    );

    if (rc != 0) {
        errorMsg = "libsodium encryption failed (unexpected error).";
        cipherOutput.clear();
        return false;
    }

    // Trim to actual size (should equal plaintext.size() + MAC_BYTES)
    cipherOutput.resize(pos + static_cast<size_t>(actualCipherLen));
    return true;
}

// ---------------------------------------------------------------------------
// decryptFile
//  Parses the .enc header, verifies the magic, and decrypts.
//  Returns false (and sets errorMsg) if the key is wrong or data is corrupt.
// ---------------------------------------------------------------------------
bool CryptoManager::decryptFile(const std::vector<unsigned char>& encData,
                                 std::vector<unsigned char>& plaintextOutput,
                                 std::string& outOriginalExt,
                                 std::string& errorMsg)
{
    if (!m_keyLoaded) {
        errorMsg = "No decryption key loaded. Load the key used during encryption.";
        return false;
    }

    // Minimum size check: magic + extLen + nonce + MAC
    const size_t MIN_SIZE = sizeof(MAGIC) + 1 + NONCE_BYTES + MAC_BYTES;
    if (encData.size() < MIN_SIZE) {
        errorMsg = "File is too small to be a valid .enc file.";
        return false;
    }

    size_t pos = 0;

    // --- Verify magic ---
    if (std::memcmp(encData.data() + pos, MAGIC, sizeof(MAGIC)) != 0) {
        errorMsg = "Invalid .enc file format (bad magic). Is this a SecureEncrypt file?";
        return false;
    }
    pos += sizeof(MAGIC);

    // --- Read extension ---
    uint8_t extLen = encData[pos++];
    if (encData.size() < pos + extLen + NONCE_BYTES + MAC_BYTES) {
        errorMsg = "Corrupted .enc file (truncated header).";
        return false;
    }

    outOriginalExt = std::string(reinterpret_cast<const char*>(encData.data() + pos), extLen);
    pos += extLen;

    // --- Read nonce ---
    const unsigned char* nonce = encData.data() + pos;
    pos += NONCE_BYTES;

    // --- Decrypt ---
    const unsigned char* ciphertext    = encData.data() + pos;
    size_t               ciphertextLen = encData.size() - pos;

    if (ciphertextLen < MAC_BYTES) {
        errorMsg = "Ciphertext too short (missing authentication tag).";
        return false;
    }

    plaintextOutput.resize(ciphertextLen - MAC_BYTES);

    unsigned long long decryptedLen = 0;
    int rc = crypto_aead_chacha20poly1305_decrypt(
        plaintextOutput.data(),    // plaintext output
        &decryptedLen,             // actual length
        nullptr,                   // nsec (unused)
        ciphertext,                // ciphertext + tag
        ciphertextLen,
        nullptr, 0,                // additional data (none)
        nonce,
        m_key
    );

    if (rc != 0) {
        // libsodium returns -1 when authentication fails (wrong key / corrupted data)
        plaintextOutput.clear();
        errorMsg = "Decryption FAILED – authentication tag mismatch.\n"
                   "  Possible causes:\n"
                   "  1. Wrong key supplied.\n"
                   "  2. File has been tampered with or corrupted.\n"
                   "  3. The .enc file is not from this application.";
        return false;
    }

    plaintextOutput.resize(static_cast<size_t>(decryptedLen));
    return true;
}

// ---------------------------------------------------------------------------
// bytesToHex  (private static)
// ---------------------------------------------------------------------------
std::string CryptoManager::bytesToHex(const unsigned char* data, size_t len)
{
    std::ostringstream oss;
    for (size_t i = 0; i < len; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(data[i]);
    }
    return oss.str();
}

// ---------------------------------------------------------------------------
// hexToBytes  (private static)
// ---------------------------------------------------------------------------
bool CryptoManager::hexToBytes(const std::string& hex,
                                unsigned char* out, size_t expectedLen)
{
    if (hex.size() != expectedLen * 2) return false;
    for (size_t i = 0; i < expectedLen; ++i) {
        std::string byte = hex.substr(i * 2, 2);
        // Validate hex characters
        for (char c : byte) {
            if (!std::isxdigit(static_cast<unsigned char>(c))) return false;
        }
        out[i] = static_cast<unsigned char>(std::stoul(byte, nullptr, 16));
    }
    return true;
}
