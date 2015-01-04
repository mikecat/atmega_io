#include <stdio.h>
#include <stdlib.h>
#include "usbio_windows.h"
#include "avr_io.h"

int main(int argc, char *argv[]) {
	int *data;
	int page_size;
	int read_size;
	avrio_t avrio;
	int signature[4];
	int lock, fuse, fuse_high, extended_fuse, calibration;
	if (argc != 4 || sscanf(argv[1], "%d", &page_size) != 1 ||
	sscanf(argv[2], "%d", &read_size) != 1) {
		fprintf(stderr, "Usage: %s page_size read_size out_file\n",
			argc > 0 ? argv[0] : "read_avr");
		fputs("serial out   : J1-7\n", stderr);
		fputs("serial in    : J2-0\n", stderr);
		fputs("serial clock : J1-6\n", stderr);
		fputs("reset        : J1-5\n", stderr);
		return 1;
	}
	if (!usbio_init(&avrio, 8, 7, 6, 5)) {
		fputs("usbio_init error\n", stderr);
		return 1;
	}
	if (!usbio_reset(avrio.hardware_data)) {
		fputs("usbio_reset error\n", stderr);
	}
	if (read_signature_byte(&avrio, signature) == AVRIO_SUCCESS) {
		printf("signature = %02X %02X %02X\n",
			signature[0], signature[1], signature[2]);
	} else {
		fputs("read_signature_byte error\n", stderr);
	}
	if (read_information(&avrio, &lock, &fuse, &fuse_high,
	&extended_fuse, &calibration) == AVRIO_SUCCESS) {
		printf("Lock bits          = %02X\n", lock);
		printf("Fuse bits          = %02X\n", fuse);
		printf("Fuse High bits     = %02X\n", fuse_high);
		printf("Extended Fuse Bits = %02X\n", extended_fuse);
		printf("Calibration Byte   = %02X\n", calibration);
	} else {
		fputs("read_information error\n", stderr);
	}
	data = malloc(sizeof(int) * read_size);
	if (data != NULL) {
		if (read_program(&avrio, data, 0, read_size) == AVRIO_SUCCESS) {
			FILE* fp;
			int i;
			fp = fopen(argv[3], "wb");
			if (fp == NULL) {
				fputs("fopen error\n", stderr);
			} else {
				for (i = 0; i < read_size; i++) {
					unsigned char write_data[2];
					write_data[0] = data[i] & 0xff;
					write_data[1] = (data[i] >> 8) & 0xff;
					fwrite(write_data, sizeof(write_data[0]), 2, fp);
				}
				fclose(fp);
			}
		} else {
			fputs("read_program error\n", stderr);
		}
	} else {
		fputs("malloc error\n", stderr);
	}
	if (!usbio_stop(&avrio)) {
		fputs("usbio_stop error\n", stderr);
	}
	return 0;
}
