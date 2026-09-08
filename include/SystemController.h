#pragma once
// SystemController.h

#ifndef SYSTEMCONTROLLER_H
#define SYSTEMCONTROLLER_H

#include "User.h"
#include "FileHandler.h"
#include "CryptoManager.h"

class SystemController {
public:
    SystemController();
    ~SystemController() = default;

    void run();

private:
    User          m_user;
    FileHandler   m_fileHandler;
    CryptoManager m_crypto;

    // menus
    void showWelcomeBanner()  const;
    void showMainMenu()       const;
    void showLoggedInMenu()   const;

    // handlers
    void handleRegister();
    void handleLogin();
    void handleEncrypt();
    void handleDecrypt();
    void handleKeyMenu();
    bool askUserForKey();   // helper for decrypt - returns false if user cancels

    // utilities
    static void        clearScreen();
    static void        pauseForInput();
    static std::string promptLine(const std::string& prompt);
    static std::string promptPassword(const std::string& prompt);
    static void        printSeparator(char c = '-', int width = 40);
    static void        printSuccess(const std::string& msg);
    static void        printError(const std::string& msg);
    static void        printInfo(const std::string& msg);
};

#endif // SYSTEMCONTROLLER_H
