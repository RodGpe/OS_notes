CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L
TARGET  = miniserv

$(TARGET): src/server.c
	$(CC) $(CFLAGS) src/server.c -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: clean
