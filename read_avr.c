#include <stdio.h>
#include <stdlib.h>
#include "usbio_windows.h"
#include "avr_io.h"
#include "progress_bar.h"

int main(int argc, char *argv[]) {
	int buffer_size = 4;
	unsigned int *data;
	int start_addr;
	int read_size;
	avrio_t *avrio;
	int signature[4];
	int lock, fuse, fuse_high, extended_fuse, calibration;
	int error_code;
	if (argc != 4 || sscanf(argv[1], "%d", &start_addr) != 1 ||
	sscanf(argv[2], "%d", &read_size) != 1) {
		fprintf(stderr, "Usage: %s start_addr read_size out_file\n\n",
			argc > 0 ? argv[0] : "read_avr");
		fputs("serial out   (MOSI) : J1-7\n", stderr);
		fputs("serial in    (MISO) : J2-0\n", stderr);
		fputs("serial clock (SCK)  : J1-6\n", stderr);
		fputs("reset               : J1-5\n", stderr);
		return 1;
	}
	if ((avrio = usbio_init(8, 7, 6, 5)) == NULL) {
		fputs("usbio_init error\n", stderr);
		return 1;
	}
	if ((error_code = reset(avrio)) != AVRIO_SUCCESS) {
		fprintf(stderr, "reset error %d\n", error_code);
	}
	if ((error_code = read_signature_byte(avrio, signature)) == AVRIO_SUCCESS) {
		printf("signature = %02X %02X %02X\n",
			signature[0], signature[1], signature[2]);
	} else {
		fprintf(stderr, "read_signature_byte error %d\n", error_code);
	}
	if ((error_code = read_information(avrio, &lock, &fuse, &fuse_high,
	&extended_fuse, &calibration)) == AVRIO_SUCCESS) {
		printf("Lock bits          = %02X\n", lock);
		printf("Fuse bits          = %02X\n", fuse);
		printf("Fuse High bits     = %02X\n", fuse_high);
		printf("Extended Fuse Bits = %02X\n", extended_fuse);
		printf("Calibration Byte   = %02X\n", calibration);
	} else {
		fprintf(stderr, "read_information error %d\n", error_code);
	}
	data = malloc(sizeof(unsigned int) * read_size);
	if (data != NULL) {
		FILE* fp;
		progress_t prog;
		int i;
		fp = fopen(argv[3], "wb");
		if (fp == NULL) {
			fputs("fopen error\n", stderr);
		} else {
			int initial_read_size = read_size;
			init_progress(&prog, initial_read_size);
			while (read_size > 0) {
				int current_read_size = read_size;
				if (current_read_size > buffer_size) current_read_size = buffer_size;
				if ((error_code = read_program(avrio, data, start_addr, current_read_size)) == AVRIO_SUCCESS) {
					for (i = 0; i < current_read_size; i++) {
						unsigned char write_data[2];
						write_data[0] = data[i] & 0xff;
						write_data[1] = (data[i] >> 8) & 0xff;
						fwrite(write_data, sizeof(write_data[0]), 2, fp);
					}
				} else {
					fprintf(stderr, "read_program error %d\n", error_code);
					break;
				}
				read_size -= current_read_size;
				start_addr += current_read_size;
				update_progress(&prog, initial_read_size - read_size);
			}
			fputc('\n', stderr);
		}
		fclose(fp);
		free(data);
	} else {
		fputs("malloc error\n", stderr);
	}
	if ((error_code = disconnect(avrio)) != AVRIO_SUCCESS) {
		fprintf(stderr, "disconnect error %d\n", error_code);
	}
	return 0;
}
