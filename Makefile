CXX = g++
CPPFLAGS = -Wall -Wextra -Wshadow -Wformat=2 -Wfloat-equal -Wconversion -Wlogical-op \
           -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -D_FORTIFY_SOURCE=2 \
					 -DDEBUG \
           -O3 --std=c++23

SRC = src/$(TARGET).cpp
BUILD_DIR = build
BIN = $(BUILD_DIR)/$(TARGET)

all: $(BIN)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN): $(SRC) | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) -o $@ $<

clean:
	rm -rf $(BUILD_DIR)
