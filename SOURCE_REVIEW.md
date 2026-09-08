# Supplied source: review notes

The source project is named SecureEncrypt. The portfolio uses SecureCrypt, based on the owner’s project name. The C++ implementation has been copied without modifying its behavior.

## Source layout

`main.cpp` starts the menu-driven application. `User` handles local accounts; `FileHandler` reads and writes binary files; `CryptoManager` calls libsodium; `SystemController` orchestrates the interaction. The supplied CMake target is `SecureEncrypt` and uses C++17.

## What the code shows

- Encryption/decryption call `crypto_aead_chacha20poly1305_encrypt` and `crypto_aead_chacha20poly1305_decrypt`.
- Key and nonce buffers in this source are 32 bytes and 8 bytes respectively. The original README’s “IETF” label is not carried forward.
- `User::hashPassword` calls `crypto_hash_sha256`; review password storage before release.
- Keys can be exported as plain hexadecimal files. Key-handling and storage need a dedicated review.
- `CryptoManager` currently has a default destructor. The old README’s claim that destruction wipes the key is not carried forward.
- Additional authenticated data is passed as `nullptr, 0`; review protection of file-format metadata.
- The non-Windows path in `main.cpp` calls `mkdir`; verify its platform headers and the build before claiming Linux compatibility.

These are observations from source inspection, not a complete security audit. No supplied executable was run. A reproducible build, functional tests, tamper tests, and security review remain to be done.

The package intentionally omits `data/users.dat`, compiled executables, DLLs, build caches, and IDE settings. User credentials and encryption keys should remain outside source control.
