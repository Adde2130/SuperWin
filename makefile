CC := gcc
CXX := g++
CFLAGS := -Wall -Wextra -Iinclude
CXXFLAGS := $(CFLAGS) -static-libstdc++

SRC_DIR := source
DLL_SRC_DIR := $(SRC_DIR)/DLL
BUILD_DIR := build
DLL_TARGET := dll

C_SRCS := $(wildcard $(SRC_DIR)/*.c)
CPP_SRCS := $(wildcard $(SRC_DIR)/*.cpp)
DLL_SRCS := $(wildcard $(DLL_SRC_DIR)/*.cpp)
C_OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SRCS))
CPP_OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(CPP_SRCS))
DLL_OBJS := $(patsubst $(DLL_SRC_DIR)/%.cpp,$(BUILD_DIR)/DLL_%.o,$(DLL_SRCS))
OBJS := $(C_OBJS) $(CPP_OBJS) $(DLL_OBJS)

DLLS := $(patsubst $(DLL_SRC_DIR)/%.cpp,$(DLL_TARGET)/%.dll,$(DLL_SRCS))

LIBS := -lgdi32 -lgdiplus -luser32 -lole32 -loleaut32 -luuid -lshell32 -lmsimg32

TARGET := SuperWin

.PHONY: all clean

all: $(BUILD_DIR) $(DLL_TARGET) $(TARGET) $(DLLS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(DLL_TARGET):
	mkdir -p $(DLL_TARGET)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/DLL_%.o: $(DLL_SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(DLL_TARGET)/%.dll: $(BUILD_DIR)/DLL_%.o
	$(CXX) -shared $< -o $@ $(LIBS)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)

clean:
	rm -rf $(BUILD_DIR) $(DLL_TARGET) $(TARGET)