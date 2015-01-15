CC=gcc
CFLAGS=-Wall -Wextra -static
LDFLAGS=-s -static

.PHONY: all
all: read_avr.exe load_hex_test.exe

read_avr.exe: read_avr.o avr_io.o usbio_windows.o progress_bar.o
	$(CC) -o read_avr.exe read_avr.o avr_io.o usbio_windows.o progress_bar.o -lsetupapi -lhid

load_hex_test.exe: load_hex.c
	$(CC) $(CFLAGS) -DLOAD_HEX_TEST -o load_hex_test.exe load_hex.c $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^
