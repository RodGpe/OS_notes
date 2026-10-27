CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L
TARGET  = miniserv

$(TARGET): src/server.c
	$(CC) $(CFLAGS) src/server.c -o $(TARGET)

clean:
	rm -f $(TARGET)

demo-shutdown: $(TARGET)
	./demo_apagado_ordenado.sh

demo-shutdown-pause: $(TARGET)
	./demo_apagado_ordenado.sh --pause

.PHONY: clean demo-shutdown demo-shutdown-pause
