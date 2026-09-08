// ============================================================
//  main.cpp
//  Entry point for the Secure File Encryption & Decryption
//  System.  Initialises libsodium and hands control to the
//  SystemController.
// ============================================================

#include "include/SystemController.h"
#include <sodium.h>
#include <iostream>
#include <cstdlib>  // for _mkdir / mkdir

// ---------------------------------------------------------------------------
// Ensure the data/ directory exists (stores users.dat)
// ---------------------------------------------------------------------------
static void ensureDataDir()
{
#ifdef _WIN32
    system("if not exist data mkdir data");
#else
    mkdir("data", 0755);
#endif
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main()
{
    // ---- 1. Initialise libsodium ----
    // This MUST be called before any other libsodium function.
    if (sodium_init() < 0) {
        std::cerr << "[FATAL] libsodium initialisation failed.\n"
                  << "        The system cannot provide secure random numbers.\n"
                  << "        Aborting.\n";
        return 1;
    }

    // ---- 2. Create data directory if it doesn't exist ----
    ensureDataDir();

    // ---- 3. Hand over to the controller ----
    SystemController controller;
    controller.run();

    return 0;
}
