CC = gcc
CFLAGS = -O2 -std=c11 -Wall -Wextra -Wpedantic
LDLIBS = -pthread
TARGET = stress-test
SRC = main.c

.PHONY: all clean
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDLIBS)

clean:
	rm -f $(TARGET)
