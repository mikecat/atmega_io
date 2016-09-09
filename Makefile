CC=gcc
CFLAGS=-Wall -Wextra -static
LDFLAGS=-s -static

.PHONY: all
all: read_atmega.exe write_atmega.exe load_hex_test.exe

read_atmega.exe: read_atmega.o atmega_io.o usbio_windows.o progress_bar.o
	$(CC) -o read_atmega.exe read_atmega.o atmega_io.o usbio_windows.o progress_bar.o -lsetupapi -lhid

write_atmega.exe: write_atmega.o atmega_io.o usbio_windows.o progress_bar.o load_hex.o
	$(CC) -o write_atmega.exe write_atmega.o atmega_io.o usbio_windows.o progress_bar.o load_hex.o -lsetupapi -lhid

load_hex_test.exe: load_hex.c
	$(CC) $(CFLAGS) -DLOAD_HEX_TEST -o load_hex_test.exe load_hex.c $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^
