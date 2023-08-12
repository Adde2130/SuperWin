CC := gcc
CXX := g++
CFLAGS := -Wall -Wextra -Iinclude # -mwindows for no console
CXXFLAGS := $(CFLAGS) -static-libstdc++

SRC_DIR := source
BUILD_DIR := build

C_SRCS := $(wildcard $(SRC_DIR)/*.c)
CPP_SRCS := $(wildcard $(SRC_DIR)/*.cpp)
C_OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SRCS))
CPP_OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(CPP_SRCS))
OBJS := $(C_OBJS) $(CPP_OBJS)

LIBS := -lgdi32 -lgdiplus -luser32 -lole32 -loleaut32 -luuid -lshell32 -lmsimg32

TARGET := SuperWin


.PHONY: all clean

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)