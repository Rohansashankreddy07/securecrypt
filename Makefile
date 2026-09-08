# ============================================================
#  Makefile  –  for MinGW-w64 / GCC on Windows
#
#  Usage (from the SecureEncrypt directory):
#    mingw32-make           → build SecureEncrypt.exe
#    mingw32-make clean     → remove build artifacts
#
#  Prerequisites:
#    1. libsodium installed to C:\libsodium
#       Download pre-built binaries from:
#       https://download.libsodium.org/libsodium/releases/
#       (choose libsodium-X.Y.Z-mingw.tar.gz for MinGW)
#    2. Adjust SODIUM_DIR below to match your installation.
# ============================================================

CXX       := g++
CXXFLAGS  := -std=c++17 -Wall -Wextra -O2

# ── libsodium paths (edit if your installation differs) ───────
SODIUM_DIR     := C:/libsodium
SODIUM_INC     := $(SODIUM_DIR)/include
SODIUM_LIB     := $(SODIUM_DIR)/lib
SODIUM_DLL_DIR := $(SODIUM_DIR)/bin

INCLUDES  := -I$(SODIUM_INC) -Iinclude
LIBS      := -L$(SODIUM_LIB) -lsodium

TARGET    := SecureEncrypt.exe

SRCS := main.cpp \
        src/User.cpp \
        src/FileHandler.cpp \
        src/CryptoManager.cpp \
        src/SystemController.cpp

OBJS := $(SRCS:.cpp=.o)

# ── Rules ─────────────────────────────────────────────────────
.PHONY: all clean

all: data $(TARGET)

data:
	@if not exist data mkdir data

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)
	@echo.
	@echo Build successful: $(TARGET)
	@echo.
	@echo NOTE: Copy libsodium.dll from $(SODIUM_DLL_DIR) to
	@echo       the directory containing $(TARGET) before running.

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

clean:
	del /Q /F $(TARGET) $(OBJS) 2>NUL || true
