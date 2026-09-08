cmake_minimum_required(VERSION 3.16)
project(SecureEncrypt CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Source files ──────────────────────────────────────────────
set(SOURCES
    main.cpp
    src/User.cpp
    src/FileHandler.cpp
    src/CryptoManager.cpp
    src/SystemController.cpp
)

add_executable(SecureEncrypt ${SOURCES})

# ── libsodium ─────────────────────────────────────────────────
# Option A: vcpkg (recommended on Windows)
#   vcpkg install libsodium:x64-windows
#   cmake -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake ..
find_package(unofficial-sodium CONFIG QUIET)
if(unofficial-sodium_FOUND)
    target_link_libraries(SecureEncrypt PRIVATE unofficial-sodium::sodium)
    message(STATUS "libsodium found via vcpkg")
else()
    # Option B: pkg-config (Linux / MSYS2 / MinGW)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SODIUM REQUIRED libsodium)
    target_include_directories(SecureEncrypt PRIVATE ${SODIUM_INCLUDE_DIRS})
    target_link_libraries(SecureEncrypt PRIVATE ${SODIUM_LIBRARIES})
    message(STATUS "libsodium found via pkg-config: ${SODIUM_LIBRARIES}")
endif()

# ── Windows: ensure data/ directory exists at build time ─────
if(WIN32)
    add_custom_command(TARGET SecureEncrypt POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
                $<TARGET_FILE_DIR:SecureEncrypt>/data
    )
endif()
