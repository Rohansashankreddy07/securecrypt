// SystemController.cpp
// Main controller - handles all menus and user interaction

#include "../include/SystemController.h"
#include <iostream>
#include <string>
#include <limits>
#include <algorithm>
#include <cctype>

#ifdef _WIN32
  #include <windows.h>
  #define CLEAR_CMD "cls"
#else
  #define CLEAR_CMD "clear"
#endif

// ANSI colors
#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"
#define WHITE   "\033[97m"
#define MAGENTA "\033[35m"
#define DIM     "\033[2m"

// --------------------------------------
// Folder paths
// .enc files go to  -> encrypted files folder
// .key files go to  -> decrypted keys folder
// decrypted files   -> same folder as the .enc file
// --------------------------------------
static const std::string ENCRYPTED_FILES_FOLDER = "D:/ENCRYPTED";
static const std::string DECRYPTED_KEYS_FOLDER  = "D:/ENCRYPTED_KEYS";

// --------------------------------------
// Constructor
// --------------------------------------
SystemController::SystemController()
{
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD mode = 0;
        if (GetConsoleMode(hOut, &mode))
            SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
#endif
}

// --------------------------------------
// cleanPath  -  handles drag & drop from Windows Explorer
//
// drag & drop in MSYS2 gives:  '/e/photos for testing/file.pdf'
//
// This function:
//   1. strips surrounding quotes
//   2. converts backslashes to forward slashes
//   3. converts /e/folder -> E:/folder  (MSYS2 -> Windows style)
//      because C++ file functions need Windows paths on MSYS2
// --------------------------------------
static std::string cleanPath(std::string path)
{
    // trim leading whitespace
    while (!path.empty() && (path.front() == ' ' || path.front() == '\t' || path.front() == '\r'))
        path.erase(path.begin());

    // trim trailing whitespace
    while (!path.empty() && (path.back() == ' ' || path.back() == '\t' || path.back() == '\r' || path.back() == '\n'))
        path.pop_back();

    // strip surrounding single or double quotes
    if (path.size() >= 2) {
        if ((path.front() == '\'' && path.back() == '\'') ||
            (path.front() == '"'  && path.back() == '"')) {
            path = path.substr(1, path.size() - 2);
        }
    }

    // convert all backslashes to forward slashes
    for (char& c : path)
        if (c == '\\') c = '/';

    // convert MSYS2 style  /e/folder  ->  E:/folder  (Windows style)
    // C++ runtime on MSYS2 needs Windows paths, not MSYS2 /e/ paths
    if (path.size() >= 2 &&
        path[0] == '/' &&
        std::isalpha((unsigned char)path[1]) &&
        (path.size() == 2 || path[2] == '/')) {
        char drive = (char)std::toupper((unsigned char)path[1]);
        path = std::string(1, drive) + ":/" + path.substr(3);
    }

    return path;
}

// --------------------------------------
// Main loop
// --------------------------------------
void SystemController::run()
{
    showWelcomeBanner();
    pauseForInput();

    bool running = true;
    while (running)
    {
        if (!m_user.isLoggedIn())
        {
            showMainMenu();
            std::string choice = promptLine("Enter choice");

            if      (choice == "1") handleRegister();
            else if (choice == "2") handleLogin();
            else if (choice == "3") { clearScreen(); printInfo("Goodbye! Stay secure."); running = false; continue; }
            else                    printError("Invalid option. Please enter 1, 2 or 3.");
        }
        else
        {
            showLoggedInMenu();
            std::string choice = promptLine("Enter choice");

            if      (choice == "1") handleEncrypt();
            else if (choice == "2") handleDecrypt();
            else if (choice == "3") handleKeyMenu();
            else if (choice == "4") { m_user.logout(); printInfo("You have been logged out."); }
            else if (choice == "5") { clearScreen(); printInfo("Goodbye! Stay secure."); running = false; continue; }
            else                    printError("Invalid option. Please enter 1-5.");
        }

        pauseForInput();
    }
}

// --------------------------------------
// Welcome banner
// --------------------------------------
void SystemController::showWelcomeBanner() const
{
    std::cout << "\n";
    std::cout << "  +------------------------------------------------------+\n";
    std::cout << "  |                                                      |\n";
    std::cout << "  |      SECURE FILE ENCRYPTION & DECRYPTION SYSTEM      |\n";
    std::cout << "  |                                                      |\n";
    std::cout << "  |        Algorithm : ChaCha20-Poly1305 (AEAD)          |\n";
    std::cout << "  |        Library   : libsodium                         |\n";
    std::cout << "  |        Key Size  : 256-bit                           |\n";
    std::cout << "  |                                                      |\n";
    std::cout << "  +------------------------------------------------------+\n";
    std::cout << "\n";
}

// --------------------------------------
// Main menu (not logged in)
// --------------------------------------
void SystemController::showMainMenu() const
{
    clearScreen();
    showWelcomeBanner();
    std::cout << WHITE << BOLD << "  MAIN MENU\n" << RESET;
    std::cout << DIM   << "  --------------------------------------\n" << RESET;
    std::cout << "  " << CYAN << "[1]" << RESET << "  Register new account\n";
    std::cout << "  " << CYAN << "[2]" << RESET << "  Login to existing account\n";
    std::cout << "  " << CYAN << "[3]" << RESET << "  Exit\n";
    std::cout << DIM   << "  --------------------------------------\n" << RESET;
    std::cout << "\n";
}

// --------------------------------------
// Logged-in menu
// --------------------------------------
void SystemController::showLoggedInMenu() const
{
    clearScreen();
    showWelcomeBanner();
    std::cout << GREEN << "  Logged in as: " << BOLD << m_user.getUsername() << RESET << "\n\n";
    std::cout << WHITE << BOLD << "  WHAT WOULD YOU LIKE TO DO?\n" << RESET;
    std::cout << DIM   << "  --------------------------------------\n" << RESET;
    std::cout << "  " << CYAN    << "[1]" << RESET << "  Encrypt a file\n";
    std::cout << "  " << CYAN    << "[2]" << RESET << "  Decrypt a file\n";
    std::cout << "  " << MAGENTA << "[3]" << RESET << "  Key Management\n";
    std::cout << "  " << YELLOW  << "[4]" << RESET << "  Logout\n";
    std::cout << "  " << RED     << "[5]" << RESET << "  Exit\n";
    std::cout << DIM   << "  --------------------------------------\n" << RESET;
    std::cout << "\n";
}

// --------------------------------------
// Register
// --------------------------------------
void SystemController::handleRegister()
{
    clearScreen();
    std::cout << "\n" << BOLD << CYAN << "  REGISTER NEW ACCOUNT\n" << RESET;
    std::cout << DIM << "  --------------------------------------\n" << RESET;
    std::cout << "\n";

    std::string username = promptLine("Choose a username");
    if (username.empty()) {
        printError("Username cannot be empty.");
        return;
    }

    if (username.find(' ') != std::string::npos) {
        printError("Username cannot contain spaces.");
        return;
    }

    std::string pw1 = promptPassword("Choose a password");
    if (pw1.size() < 4) {
        printError("Password must be at least 4 characters long.");
        return;
    }

    std::string pw2 = promptPassword("Confirm your password");
    if (pw1 != pw2) {
        printError("Passwords do not match. Please try again.");
        return;
    }

    if (m_user.registerUser(username, pw1)) {
        std::cout << "\n";
        printSuccess("Account created successfully!");
        printInfo("You can now login with your credentials.");
    } else {
        printError("Username already exists. Please choose a different one.");
    }
}

// --------------------------------------
// Login
// --------------------------------------
void SystemController::handleLogin()
{
    clearScreen();
    std::cout << "\n" << BOLD << CYAN << "  LOGIN\n" << RESET;
    std::cout << DIM << "  --------------------------------------\n" << RESET;
    std::cout << "\n";

    std::string username = promptLine("Username");
    std::string password = promptPassword("Password");

    if (m_user.login(username, password)) {
        std::cout << "\n";
        printSuccess("Login successful! Welcome back, " + username + ".");
    } else {
        printError("Incorrect username or password. Please try again.");
    }
}

// --------------------------------------
// Encrypt
//
// Flow:
//   source file  ->  drag & drop any file
//   .enc output  ->  ENCRYPTED_FILES_FOLDER
//   .key output  ->  DECRYPTED_KEYS_FOLDER
// --------------------------------------
void SystemController::handleEncrypt()
{
    clearScreen();
    std::cout << "\n" << BOLD << CYAN << "  ENCRYPT A FILE\n" << RESET;
    std::cout << DIM << "  --------------------------------------\n" << RESET;
    std::cout << "\n";

    // make sure we have a key ready
    if (!m_crypto.hasKey()) {
        printInfo("No encryption key found. Generating a new 256-bit key...");
        std::string hex = m_crypto.generateKey();
        std::cout << "\n";
        printSuccess("Key generated!");
        std::cout << "  " << YELLOW << BOLD << hex << RESET << "\n\n";
        std::cout << RED << BOLD << "  *** IMPORTANT: Save this key - you need it to decrypt your file! ***\n" << RESET;
        std::cout << "\n";
    } else {
        std::cout << DIM << "  Active key: " << RESET << YELLOW << m_crypto.currentKeyHex() << RESET << "\n\n";
    }

    // file path input
    std::cout << DIM << "  TIP: You can drag & drop a file directly into this window!\n" << RESET;
    std::cout << "\n";

    std::string inputPath = cleanPath(promptLine("File path to encrypt"));

    if (inputPath.empty()) {
        printError("No file path entered.");
        return;
    }

    if (!FileHandler::fileExists(inputPath)) {
        printError("File not found: " + inputPath);
        std::cout << DIM << "  Make sure the path is correct. Tip: drag the file into this window.\n" << RESET;
        return;
    }

    std::string base = FileHandler::baseName(inputPath);
    std::string ext  = FileHandler::extension(inputPath);

    // .enc file goes to ENCRYPTED_FILES_FOLDER
    std::string outputPath = ENCRYPTED_FILES_FOLDER + "/" + base + ".enc";

    // .key file goes to DECRYPTED_KEYS_FOLDER
    std::string keyPath = DECRYPTED_KEYS_FOLDER + "/" + base + ".key";

    std::cout << "\n";
    std::cout << DIM << "  Source    : " << RESET << inputPath  << "\n";
    std::cout << DIM << "  Encrypted : " << RESET << outputPath << "\n";
    std::cout << DIM << "  Key file  : " << RESET << keyPath    << "\n\n";

    // read the source file
    std::vector<unsigned char> plaintext;
    std::string errMsg;

    printInfo("Reading file...");
    if (!m_fileHandler.readFile(inputPath, plaintext, errMsg)) {
        printError(errMsg);
        return;
    }
    std::cout << DIM << "  File size : " << RESET << plaintext.size() << " bytes\n\n";

    // encrypt
    printInfo("Encrypting...");
    std::vector<unsigned char> cipherOutput;
    if (!m_crypto.encryptFile(plaintext, ext, cipherOutput, errMsg)) {
        printError(errMsg);
        return;
    }

    // write .enc file to encrypted files folder
    if (!m_fileHandler.writeFile(outputPath, cipherOutput, errMsg)) {
        printError(errMsg);
        printInfo("Make sure this folder exists: " + ENCRYPTED_FILES_FOLDER);
        return;
    }

    // auto-save .key file to decrypted keys folder
    if (m_crypto.saveKeyToFile(keyPath)) {
        printSuccess("Key automatically saved to keys folder.");
    } else {
        printError("Could not save key to keys folder - showing key here instead:");
        std::cout << "  " << YELLOW << BOLD << m_crypto.currentKeyHex() << RESET << "\n";
        printInfo("Copy the key above and save it manually!");
    }

    // success summary
    std::cout << "\n";
    printSuccess("File encrypted successfully!");
    std::cout << "\n";
    std::cout << "  " << DIM << "Original  : " << RESET << inputPath  << " (" << plaintext.size()    << " bytes)\n";
    std::cout << "  " << DIM << "Encrypted : " << RESET << outputPath << " (" << cipherOutput.size() << " bytes)\n";
    std::cout << "  " << DIM << "Key saved : " << RESET << keyPath    << "\n";
    std::cout << "  " << DIM << "Key hex   : " << RESET << YELLOW << m_crypto.currentKeyHex() << RESET << "\n";
}

// --------------------------------------
// Decrypt
//
// Flow:
//   .enc input      ->  drag & drop from ENCRYPTED_FILES_FOLDER
//   decrypted file  ->  same folder as the .enc file
// --------------------------------------
void SystemController::handleDecrypt()
{
    clearScreen();
    std::cout << "\n" << BOLD << CYAN << "  DECRYPT A FILE\n" << RESET;
    std::cout << DIM << "  --------------------------------------\n" << RESET;
    std::cout << "\n";

    // file path input
    std::cout << DIM << "  TIP: You can drag & drop the .enc file directly into this window!\n" << RESET;
    std::cout << "\n";

    std::string inputPath = cleanPath(promptLine("Path to .enc file"));

    if (inputPath.empty()) {
        printError("No file path entered.");
        return;
    }

    if (!FileHandler::fileExists(inputPath)) {
        printError("File not found: " + inputPath);
        std::cout << DIM << "  Make sure the path ends with .enc and the file exists.\n" << RESET;
        return;
    }

    std::cout << "\n";

    // key selection
    if (m_crypto.hasKey()) {
        std::cout << DIM << "  Current key: " << RESET << YELLOW << m_crypto.currentKeyHex() << RESET << "\n\n";
        std::string useIt = promptLine("Use this key? [Y/n]");
        if (!useIt.empty() && (useIt[0] == 'n' || useIt[0] == 'N')) {
            if (!askUserForKey()) return;
        }
    } else {
        printInfo("No key loaded. Please provide the decryption key.");
        std::cout << "\n";
        if (!askUserForKey()) return;
    }

    std::cout << "\n";

    // read the encrypted file
    std::vector<unsigned char> encData;
    std::string errMsg;

    printInfo("Reading encrypted file...");
    if (!m_fileHandler.readFile(inputPath, encData, errMsg)) {
        printError(errMsg);
        return;
    }

    // decrypt and verify integrity
    printInfo("Decrypting and verifying integrity...");
    std::vector<unsigned char> plaintext;
    std::string originalExt;

    if (!m_crypto.decryptFile(encData, plaintext, originalExt, errMsg)) {
        std::cout << "\n";
        printError(errMsg);
        return;
    }

    // save decrypted file to same folder as the .enc file
    std::string base = FileHandler::baseName(inputPath);
    std::string outputPath;
    size_t lastSlash = inputPath.find_last_of("/\\");
    if (lastSlash != std::string::npos)
        outputPath = inputPath.substr(0, lastSlash + 1) + base + originalExt;
    else
        outputPath = base + originalExt;

    // avoid overwriting if file already exists there
    if (FileHandler::fileExists(outputPath)) {
        if (lastSlash != std::string::npos)
            outputPath = inputPath.substr(0, lastSlash + 1) + base + "_decrypted" + originalExt;
        else
            outputPath = base + "_decrypted" + originalExt;
    }

    if (!m_fileHandler.writeFile(outputPath, plaintext, errMsg)) {
        printError(errMsg);
        return;
    }

    std::cout << "\n";
    printSuccess("File decrypted successfully!");
    std::cout << "\n";
    std::cout << "  " << DIM << "Encrypted : " << RESET << inputPath  << " (" << encData.size()   << " bytes)\n";
    std::cout << "  " << DIM << "Restored  : " << RESET << outputPath << " (" << plaintext.size() << " bytes)\n";
    std::cout << "  " << DIM << "Saved to  : " << RESET << outputPath << "\n";
}

// --------------------------------------
// askUserForKey  -  helper used in decrypt
// --------------------------------------
bool SystemController::askUserForKey()
{
    std::cout << "  " << CYAN << "[1]" << RESET << "  Type / paste the 64-character hex key\n";
    std::cout << "  " << CYAN << "[2]" << RESET << "  Load key from a .key file\n\n";

    std::string choice = promptLine("Choice");

    if (choice == "1") {
        std::string hex = promptLine("Paste your 64-character key");
        if (!m_crypto.loadKeyFromHex(hex)) {
            printError("Invalid key. It must be exactly 64 hexadecimal characters.");
            return false;
        }
        printSuccess("Key accepted.");
        return true;
    }
    else if (choice == "2") {
        std::cout << DIM << "  TIP: You can drag & drop the .key file into this window!\n" << RESET << "\n";
        std::string keyPath = cleanPath(promptLine("Path to .key file"));
        if (!m_crypto.loadKeyFromFile(keyPath)) {
            printError("Could not load key from: " + keyPath);
            return false;
        }
        printSuccess("Key loaded from file.");
        return true;
    }

    printError("Invalid choice.");
    return false;
}

// --------------------------------------
// Key Management menu
// --------------------------------------
void SystemController::handleKeyMenu()
{
    clearScreen();
    std::cout << "\n" << BOLD << MAGENTA << "  KEY MANAGEMENT\n" << RESET;
    std::cout << DIM << "  --------------------------------------\n" << RESET;
    std::cout << "\n";

    if (m_crypto.hasKey()) {
        std::cout << "  Active key:\n";
        std::cout << "  " << YELLOW << BOLD << m_crypto.currentKeyHex() << RESET << "\n\n";
    } else {
        std::cout << DIM << "  No key currently loaded.\n\n" << RESET;
    }

    std::cout << "  " << CYAN   << "[1]" << RESET << "  Generate a new random key\n";
    std::cout << "  " << CYAN   << "[2]" << RESET << "  Enter a key manually\n";
    std::cout << "  " << CYAN   << "[3]" << RESET << "  Load key from .key file\n";
    std::cout << "  " << CYAN   << "[4]" << RESET << "  Save current key to file\n";
    std::cout << "  " << YELLOW << "[5]" << RESET << "  Back to main menu\n";
    std::cout << DIM << "  --------------------------------------\n" << RESET;
    std::cout << "\n";

    std::string choice = promptLine("Choice");

    if (choice == "1")
    {
        std::string hex = m_crypto.generateKey();
        std::cout << "\n";
        printSuccess("New 256-bit key generated:");
        std::cout << "  " << YELLOW << BOLD << hex << RESET << "\n\n";
        std::cout << RED << BOLD << "  *** STORE THIS KEY SAFELY - losing it means losing your files! ***\n" << RESET;
    }
    else if (choice == "2")
    {
        std::string hex = promptLine("Enter the 64-character hex key");
        if (m_crypto.loadKeyFromHex(hex)) {
            printSuccess("Key loaded and ready to use.");
        } else {
            printError("Invalid key. Must be exactly 64 hex characters (0-9, a-f).");
        }
    }
    else if (choice == "3")
    {
        std::cout << DIM << "  TIP: Drag & drop the .key file into this window!\n" << RESET << "\n";
        std::string path = cleanPath(promptLine("Path to .key file"));
        if (m_crypto.loadKeyFromFile(path)) {
            printSuccess("Key loaded from: " + path);
            std::cout << "  " << YELLOW << m_crypto.currentKeyHex() << RESET << "\n";
        } else {
            printError("Could not read key from: " + path);
        }
    }
    else if (choice == "4")
    {
        if (!m_crypto.hasKey()) {
            printError("No key to save. Generate or enter a key first.");
            return;
        }
        std::string filename = promptLine("Enter filename for key (e.g. mykey.key)");
        if (filename.empty()) { printError("No filename given."); return; }

        std::string fullPath = DECRYPTED_KEYS_FOLDER + "/" + filename;
        if (m_crypto.saveKeyToFile(fullPath)) {
            printSuccess("Key saved to: " + fullPath);
        } else {
            printError("Failed to save. Make sure this folder exists: " + DECRYPTED_KEYS_FOLDER);
        }
    }
    // choice 5 = back, do nothing
}

// --------------------------------------
// Utility helpers
// --------------------------------------
void SystemController::clearScreen()
{
    std::system(CLEAR_CMD);
}

void SystemController::pauseForInput()
{
    std::cout << "\n" << DIM << "  Press ENTER to continue..." << RESET;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

std::string SystemController::promptLine(const std::string& prompt)
{
    std::cout << BOLD << "  > " << prompt << ": " << RESET;
    std::string line;
    std::getline(std::cin, line);

    // remove trailing carriage return (Windows line endings)
    if (!line.empty() && line.back() == '\r')
        line.pop_back();

    return line;
}

std::string SystemController::promptPassword(const std::string& prompt)
{
    std::cout << BOLD << "  > " << prompt << ": " << RESET;
    std::string line;
    std::getline(std::cin, line);

    if (!line.empty() && line.back() == '\r')
        line.pop_back();

    return line;
}

void SystemController::printSeparator(char c, int width)
{
    std::cout << "  ";
    for (int i = 0; i < width; ++i) std::cout << c;
    std::cout << "\n";
}

void SystemController::printSuccess(const std::string& msg)
{
    std::cout << GREEN << "  [OK] " << msg << RESET << "\n";
}

void SystemController::printError(const std::string& msg)
{
    std::cout << RED << "  [ERROR] " << msg << RESET << "\n";
}

void SystemController::printInfo(const std::string& msg)
{
    std::cout << CYAN << "  [INFO] " << msg << RESET << "\n";
}
