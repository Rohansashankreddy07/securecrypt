# SecureCrypt — C++ File Encryption Application

> **Status: Source implementation · Build verification and security review pending**

SecureCrypt is a menu-driven C++17 application for local account handling, file encryption and decryption, and encryption-key management. The supplied source project is named **SecureEncrypt**; that name remains in the build target and executable.

This repository contains actual C++ source and build definitions. Its presence does not establish that the application is production-ready or security-audited.

## Contents

- [The problem](#the-problem)
- [Project approach](#project-approach)
- [Source architecture](#source-architecture)
- [Implemented code paths](#implemented-code-paths)
- [Repository contents](#repository-contents)
- [Build preparation](#build-preparation)
- [Current development status](#current-development-status)
- [Review priorities](#review-priorities)
- [Development roadmap](#development-roadmap)

## The problem

A local file-encryption tool needs to coordinate several responsibilities: selecting files, managing keys, performing cryptographic operations, and explaining success or failure to the user. Mixing those responsibilities makes the program harder to inspect and maintain.

SecureCrypt explores a small, class-based implementation that separates the application menu from account handling, binary file operations, and cryptographic calls.

## Project approach

The application provides a command-line workflow around libsodium. Local accounts organize access to the application, while encryption keys are generated or imported for file operations. Account authentication and encryption-key possession are separate concerns; a successful login should not be described as a substitute for protecting the encryption key.

## Source architecture

| Component | Responsibility visible in the supplied code |
|---|---|
| `main.cpp` | Application entry point and startup |
| `SystemController` | Menu flow and coordination between components |
| `User` | Local account registration, login, and password-hash handling |
| `FileHandler` | Binary file reading and writing |
| `CryptoManager` | libsodium operations, key generation, and key import/export |

The separation provides a starting point for focused changes and tests without rewriting the whole interface.

## Implemented code paths

Source is present for:

- Local account registration and login.
- File encryption and decryption.
- Key generation and hexadecimal key-file import/export.
- Calls to `crypto_aead_chacha20poly1305_encrypt` and `crypto_aead_chacha20poly1305_decrypt`.
- A menu-driven application using dedicated C++ classes.

These are source observations. Functional correctness, tamper handling, and cross-platform operation have not been demonstrated by this repository-packaging work.

## Repository contents

| Path | Contents |
|---|---|
| `main.cpp` | Entry point |
| `include/` | Four class headers |
| `src/` | Four class implementations |
| `CMakeLists.txt` | C++17 `SecureEncrypt` target; libsodium discovery |
| `Makefile` | Supplied Windows/MinGW-oriented build definition |
| `SOURCE_REVIEW.md` | Specific observations and unresolved review items |
| `project.json` | Editable project information and portfolio link |

The uploaded source retains its original behavior. Account databases, keys, executables, DLLs, and build caches are excluded.

## Build preparation

The supplied CMake file requires CMake 3.16 or newer, a C++17 toolchain, and libsodium. It attempts a vcpkg package first, then pkg-config. With those dependencies configured, its intended workflow is:

```sh
cmake -S . -B build
cmake --build build
```

The target name is `SecureEncrypt`. These commands describe the supplied build configuration; a successful build has not yet been verified. The Makefile uses Windows shell commands and a configurable `SODIUM_DIR`; it should not be presented as a portable Unix Makefile.

## Current development status

- [x] C++ source, headers, and build files available.
- [x] Account, file, crypto, and menu responsibilities separated.
- [x] Initial source-review notes documented.
- [ ] Reproducible build recorded on a named platform.
- [ ] Encryption/decryption round-trip tests recorded.
- [ ] Wrong-key, truncated-file, and modified-file behavior tested.
- [ ] Account storage and key lifecycle reviewed.
- [ ] Independent security review completed.

## Review priorities

The implementation uses the original ChaCha20-Poly1305 API with 32-byte keys and 8-byte nonces. It must not be labeled as the IETF variant. Further inspection is needed before making security claims:

1. `User::hashPassword` currently calls plain SHA-256; password storage needs a dedicated review.
2. Exported keys are plain hexadecimal files, making storage and access control important.
3. `CryptoManager` has a default destructor; automatic key wiping is not established.
4. Additional authenticated data is passed as `nullptr, 0`; review file-format metadata protection.
5. Verify the non-Windows `mkdir` path and platform headers during build work.

See [SOURCE_REVIEW.md](SOURCE_REVIEW.md) for the source-inspection scope. Review findings are not a complete audit.

## Development roadmap

| Phase | Work | Completion evidence |
|---|---|---|
| 1 — Reproducible build | Resolve environment and platform issues | Toolchain, dependency versions, and build log |
| 2 — Functional validation | Round-trip, empty-file, binary-file, and failure cases | Repeatable tests with expected results |
| 3 — Security improvements | Account storage, key lifecycle, nonce and format review | Documented design decisions and reviewed changes |
| 4 — Usability | Clearer errors and recovery guidance | Demonstrated workflows |
| 5 — Release preparation | Versioned format and packaging | Reviewed release with known limitations |

The immediate objective is a verifiable development baseline. No production-security certification or benchmark result is claimed.

## Author

**Rohan Sashank Reddy**  
[GitHub](https://github.com/Rohansashankreddy07) · [LinkedIn](https://www.linkedin.com/in/rohan-sashank-reddy-chilukuri-aa169336a/) · [Instagram](https://www.instagram.com/rohansashankreddy/)

## Maintaining this repository

Keep this README and `project.json` aligned as the project develops. Record evidence when a planned feature becomes implemented or a target becomes a measured result. Preserve the project ID so future portfolio updates can link to the same project.
