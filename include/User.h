#pragma once
// ============================================================
//  User.h
//  Manages user credentials: registration, login, storage.
//  Passwords are stored as hex-encoded SHA-256 hashes via
//  libsodium's crypto_hash_sha256 so plain-text is never kept.
// ============================================================

#ifndef USER_H
#define USER_H

#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Credential record stored in users.dat
// ---------------------------------------------------------------------------
struct Credential {
    std::string username;
    std::string passwordHash; // hex-encoded SHA-256
};

// ---------------------------------------------------------------------------
// User  –  wraps the currently-authenticated session
// ---------------------------------------------------------------------------
class User {
public:
    // Constructor / destructor
    User();
    ~User() = default;

    // ---- Authentication ----
    bool registerUser(const std::string& username,
                      const std::string& password);

    bool login(const std::string& username,
               const std::string& password);

    void logout();

    // ---- Getters ----
    bool        isLoggedIn()  const { return m_loggedIn; }
    std::string getUsername() const { return m_username; }

    // ---- Credential file helpers ----
    static std::string hashPassword(const std::string& password);

private:
    bool        m_loggedIn;
    std::string m_username;

    // Credential store (flat file)
    static const std::string CRED_FILE;

    std::vector<Credential> loadCredentials() const;
    bool saveCredential(const Credential& cred) const;
    bool credentialExists(const std::string& username) const;
};

#endif // USER_H
