#include <stdio.h>
#include "usbio_windows.h"
#include "avr_io.h"
#include "progress_bar.h"
#include "load_hex.h"

#define DATA_BUFFER_SIZE 0x10000

int main(int argc, char *argv[]) {
	int lock_bits = -1;
	int fuse_bits = -1;
	int fuse_high_bits = -1;
	int extended_fuse_bits = -1;
	static char data[DATA_BUFFER_SIZE];
	static unsigned int data_words[DATA_BUFFER_SIZE];
	return 0;
}
