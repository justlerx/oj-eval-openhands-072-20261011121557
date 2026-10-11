CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET = code
SOURCE = riscv_simulator.cpp

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE)

clean:
	rm -f $(TARGET)

.PHONY: all clean