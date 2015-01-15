#include <stdio.h>
#include <string.h>
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
	int page_size = 64;
	int do_chip_erase = 1;
	static char data[DATA_BUFFER_SIZE];
	static unsigned int data_words[DATA_BUFFER_SIZE];
	const char *input_file = NULL;
	int command_line_error = 0;
	int show_help = 0;
	int i;
	FILE* fp;
	int ret;
	/* コマンドライン引数を読み込む */
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--lock-bits") == 0 || strcmp(argv[i], "-l") == 0) {
			if ((++i) < argc) {
				if (sscanf(argv[i], "%x", &lock_bits) != 1) {
					fprintf(stderr, "invalid argument for --lock-bits\n");
					command_line_error = 1;
				}
			} else {
				fprintf(stderr, "missing argument for --lock-bits\n");
				command_line_error = 1;
			}
		} else if (strcmp(argv[i], "--fuse-bits") == 0 || strcmp(argv[i], "-f") == 0) {
			if ((++i) < argc) {
				if (sscanf(argv[i], "%x", &fuse_bits) != 1) {
					fprintf(stderr, "invalid argument for --fuse-bits\n");
					command_line_error = 1;
				}
			} else {
				fprintf(stderr, "missing argument for --fuse-bits\n");
				command_line_error = 1;
			}
		} else if (strcmp(argv[i], "--fuse-high-bits") == 0 || strcmp(argv[i], "-fh") == 0) {
			if ((++i) < argc) {
				if (sscanf(argv[i], "%x", &fuse_high_bits) != 1) {
					fprintf(stderr, "invalid argument for --fuse-high-bits\n");
					command_line_error = 1;
				}
			} else {
				fprintf(stderr, "missing argument for --fuse-high-bits\n");
				command_line_error = 1;
			}
		} else if (strcmp(argv[i], "--extended-fuse-bits") == 0 || strcmp(argv[i], "-ef") == 0) {
			if ((++i) < argc) {
				if (sscanf(argv[i], "%x", &extended_fuse_bits) != 1) {
					fprintf(stderr, "invalid argument for --extended-fuse-bits\n");
					command_line_error = 1;
				}
			} else {
				fprintf(stderr, "missing argument for --extended-fuse-bits\n");
				command_line_error = 1;
			}
		} else if (strcmp(argv[i], "--page-size") == 0 || strcmp(argv[i], "-p") == 0) {
			if ((++i) < argc) {
				if (sscanf(argv[i], "%d", &page_size) != 1) {
					fprintf(stderr, "invalid argument for --page-size\n");
					command_line_error = 1;
				}
			} else {
				fprintf(stderr, "missing argument for --page-size\n");
				command_line_error = 1;
			}
		} else if (strcmp(argv[i], "--input-file") == 0 || strcmp(argv[i], "-i") == 0) {
			if ((++i) < argc) {
				input_file = argv[i];
			} else {
				fprintf(stderr, "missing argument for --input-file\n");
				command_line_error = 1;
			}
		} else if (strcmp(argv[i], "--chip-erase") == 0) {
			do_chip_erase = 1;
		} else if (strcmp(argv[i], "--no-chip-erase") == 0) {
			do_chip_erase = 0;
		} else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
			show_help = 1;
		} else {
			fprintf(stderr, "unrecognized command line option: %s\n", argv[i]);
			command_line_error = 1;
		}
	}
	/* 必要ならヘルプを表示する */
	if (show_help || command_line_error) {
		fprintf(stderr, "Usage: %s [options...]\n", argc > 0 ? argv[0] : "write_avr");
		fputs("options:\n", stderr);
		fputs("--lock-bits <byte> / -l <byte> : write Lock bits\n", stderr);
		fputs("--fuse-bits <byte> / -f <byte> : write Fuse bits\n", stderr);
		fputs("--fuse-high-bits <byte> / -fh <byte> : write Fuse High bits\n", stderr);
		fputs("--extended-fuse-bits <byte> / -ef <byte> : write Extended Fuse bits\n", stderr);
		fputs("--page-size <size> / -p <size> : set page size (default: 64)\n", stderr);
		fputs("--input-file <file> / -i <file> : set input file (default: stdin)\n", stderr);
		fputs("--chip-erase : do chip erase before writing (default)\n", stderr);
		fputs("--no-chip-erase : don't do chip erase before writing\n", stderr);
		fputs("--help / -h : show this help\n", stderr);

		fputs("\nconnection between USB-IO2.0 and AVR:\n", stderr);
		fputs("serial out   (MOSI) : J1-7\n", stderr);
		fputs("serial in    (MISO) : J2-0\n", stderr);
		fputs("serial clock (SCK)  : J1-6\n", stderr);
		fputs("reset               : J1-5\n", stderr);
		return command_line_error ? 1 : 0;
	}

	/* ファイルを読み込む */
	if (input_file == NULL || strcmp(input_file, "-") == 0) {
		fp = stdin;
	} else {
		fp = fopen(input_file, "r");
		if (fp == NULL) {
			fprintf(stderr, "file \"%s\" open error\n", input_file);
			return 1;
		}
	}
	ret = load_hex(data, sizeof(data), fp);
	if (fp != stdin) fclose(fp);
	if (ret != LOAD_HEX_SUCCESS) {
		fprintf(stderr, "error %d on load_hex\n", ret);
		return 1;
	}
	if ((ret != chars_to_words(data_words, data, sizeof(data))) != LOAD_HEX_SUCCESS) {
		fprintf(stderr, "error %d on chars_to_words\n", ret);
		return 1;
	}
	return 0;
}
