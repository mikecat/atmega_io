#include <ctype.h>
#include "load_hex.h"

/* スタートコードを読み込む */
static int load_hex_start(FILE *fp) {
	int in;
	do {
		in = getc(fp);
		if (ferror(fp)) return LOAD_HEX_IO_ERROR;
		if (in == EOF) return LOAD_HEX_UNEXPECTED_EOF;
	} while (isspace(in));
	return (in == ':') ? LOAD_HEX_SUCCESS : LOAD_HEX_INVALID_CHAR;
}

/* 16進数の文字を数値に変換する */
static int hex_to_int(int c) {
	static const char *hex_table = "0123456789ABCDEF";
	int i;
	char target = (char)toupper(c);
	for (i = 0; hex_table[i] != '\0'; i++) {
		if (target == hex_table[i]) return i;
	}
	return -1;
}

/* 16進数2桁のバイトデータを読み込む */
static int load_hex_byte(int *in, FILE *fp) {
	int ret = 0;
	int in_left = 2;
	int in_char;
	while (in_left > 0) {
		/* 文字を読み込む */
		do {
			in_char = getc(fp);
			if (ferror(fp)) return LOAD_HEX_IO_ERROR;
			if (in == EOF) return LOAD_HEX_UNEXPECTED_EOF;
		} while (isspace(in_char));
		/* 数値に反映する */
		if (!isxdigit(in_char)) return LOAD_HEX_INVALID_CHAR;
		ret = (ret << 4) | hex_to_int(in_char);
		in_left--;
	}
	*in = ret;
	return LOAD_HEX_SUCCESS;
}

int load_hex(char *out, int out_size, FILE *fp) {
	return LOAD_HEX_SUCCESS;
}

int chars_to_words(unsigned int *out, const char *in, int in_bytes) {
	int i;
	if (out == NULL || in == NULL || in_bytes < 0 || in_bytes % 2 != 0) {
		return LOAD_HEX_INVALID_PARAMETER;
	}
	for (i = 0; i < in_bytes; i += 2) {
		out[i / 2] = ((unsigned char)in[i] & 0xff) |
			(((unsigned int)in[i + 1] & 0xff) << 8);
	}
	return LOAD_HEX_SUCCESS;
}
