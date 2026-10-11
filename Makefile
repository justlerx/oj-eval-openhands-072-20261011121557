CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2
TARGET = code
SOURCE = riscv_simulator.c

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE)

clean:
	rm -f $(TARGET)

.PHONY: all clean