CC=gcc
CFLAGS=-Wall -Wextra -static
LDFLAGS=-s -static -lsetupapi -lhid

read_avr.exe: read_avr.o avr_io.o usbio_windows.o progress_bar.o
	$(CC) -o read_avr.exe read_avr.o avr_io.o usbio_windows.o progress_bar.o $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^
