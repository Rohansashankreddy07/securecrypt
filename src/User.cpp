// ============================================================
//  User.cpp
//  Implementation of the User class.
//  Password hashing uses libsodium crypto_hash_sha256.
//  Credentials are persisted to a plain-text file where each
//  line is:  username:passwordHash
// ============================================================

#include "../include/User.h"
#include <sodium.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <iomanip>

// Path to the credential store (relative to the working directory)
const std::string User::CRED_FILE = "data/users.dat";

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
User::User()
    : m_loggedIn(false), m_username("")
{}

// ---------------------------------------------------------------------------
// hashPassword
//  Returns a lowercase hex-encoded SHA-256 digest of the password.
// ---------------------------------------------------------------------------
std::string User::hashPassword(const std::string& password)
{
    unsigned char hash[crypto_hash_sha256_BYTES];
    crypto_hash_sha256(hash,
                       reinterpret_cast<const unsigned char*>(password.c_str()),
                       password.size());

    std::ostringstream oss;
    for (size_t i = 0; i < crypto_hash_sha256_BYTES; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(hash[i]);
    }
    return oss.str();
}

// ---------------------------------------------------------------------------
// loadCredentials
//  Reads the credential file and returns all records.
// ---------------------------------------------------------------------------
std::vector<Credential> User::loadCredentials() const
{
    std::vector<Credential> creds;
    std::ifstream file(CRED_FILE);
    if (!file.is_open()) return creds; // empty – file may not exist yet

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        auto pos = line.find(':');
        if (pos == std::string::npos) continue;

        Credential c;
        c.username     = line.substr(0, pos);
        c.passwordHash = line.substr(pos + 1);
        creds.push_back(c);
    }
    return creds;
}

// ---------------------------------------------------------------------------
// saveCredential
//  Appends one credential record to the file (creates it if needed).
// ---------------------------------------------------------------------------
bool User::saveCredential(const Credential& cred) const
{
    std::ofstream file(CRED_FILE, std::ios::app);
    if (!file.is_open()) return false;
    file << cred.username << ":" << cred.passwordHash << "\n";
    return true;
}

// ---------------------------------------------------------------------------
// credentialExists
// ---------------------------------------------------------------------------
bool User::credentialExists(const std::string& username) const
{
    auto creds = loadCredentials();
    for (auto& c : creds) {
        if (c.username == username) return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
// registerUser
//  Creates a new account; returns false if the username already exists.
// ---------------------------------------------------------------------------
bool User::registerUser(const std::string& username,
                        const std::string& password)
{
    if (username.empty() || password.empty()) return false;
    if (credentialExists(username))           return false;

    Credential cred;
    cred.username     = username;
    cred.passwordHash = hashPassword(password);
    return saveCredential(cred);
}

// ---------------------------------------------------------------------------
// login
//  Validates credentials; sets internal state on success.
// ---------------------------------------------------------------------------
bool User::login(const std::string& username,
                 const std::string& password)
{
    auto creds    = loadCredentials();
    auto hashInput = hashPassword(password);

    for (auto& c : creds) {
        if (c.username == username && c.passwordHash == hashInput) {
            m_loggedIn = true;
            m_username = username;
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// logout
// ---------------------------------------------------------------------------
void User::logout()
{
    m_loggedIn = false;
    m_username.clear();
}
