# Makefile untuk sysinfo
# Program by Brifeb & ChatGPT

CC = gcc
CFLAGS = -O2 -Wall
TARGET = sysinfo
SRC = src/sysinfo.c
PREFIX = /usr/local

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

install: $(TARGET)
	mkdir -p $(PREFIX)/bin
	cp $(TARGET) $(PREFIX)/bin/
	@echo ">> Installed $(TARGET) to $(PREFIX)/bin/$(TARGET)"

uninstall:
	rm -f $(PREFIX)/bin/$(TARGET)
	@echo ">> Uninstalled $(TARGET)"

clean:
	rm -f $(TARGET)

.PHONY: all install uninstall clean
