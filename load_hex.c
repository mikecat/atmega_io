#include <ctype.h>
#include <limits.h>
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
static int load_hex_byte(int *out, FILE *fp) {
	int ret = 0;
	int in_left = 2;
	int in_char;
	while (in_left > 0) {
		/* 文字を読み込む */
		do {
			in_char = getc(fp);
			if (ferror(fp)) return LOAD_HEX_IO_ERROR;
			if (in_char == EOF) return LOAD_HEX_UNEXPECTED_EOF;
		} while (isspace(in_char));
		/* 数値に反映する */
		if (!isxdigit(in_char)) return LOAD_HEX_INVALID_CHAR;
		ret = (ret << 4) | hex_to_int(in_char);
		in_left--;
	}
	*out = ret;
	return LOAD_HEX_SUCCESS;
}

int load_hex(char *out, int out_size, FILE *fp) {
	int address_offset = 0;
	int size_over_flag = 0;
	if (out == NULL || out_size < 0) {
		return LOAD_HEX_INVALID_PARAMETER;
	}
	for (;;) {
		int ret;
		int checksum = 0;
		int data_size;
		int address_high, address_low, address = 0;
		int data_type;
		int i;
		/* 行のヘッダの読み込み */
		if ((ret = load_hex_start(fp)) != LOAD_HEX_SUCCESS) return ret;
		if ((ret = load_hex_byte(&data_size, fp)) != LOAD_HEX_SUCCESS) return ret;
		if ((ret = load_hex_byte(&address_high, fp)) != LOAD_HEX_SUCCESS) return ret;
		if ((ret = load_hex_byte(&address_low, fp)) != LOAD_HEX_SUCCESS) return ret;
		if ((ret = load_hex_byte(&data_type, fp)) != LOAD_HEX_SUCCESS) return ret;
		checksum = (data_size + address_high + address_low + data_type) & 0xff;
		if (data_type == 0x00 && data_size > 0) {
			/* オーバーフローチェック */
			if ((INT_MAX >> 8) < address_high) return LOAD_HEX_SIZE_OVER;
			/* アドレスを設定する */
			address = (address_high << 8) | address_low;
			/* オーバーフローチェック */
			if (address > INT_MAX - address_offset) return LOAD_HEX_SIZE_OVER;
			/* アドレスを調整する */
			address += address_offset;
		}
		/* 行の内容の読み込み */
		for (i = 0; i < data_size; i++) {
			int data;
			if ((ret = load_hex_byte(&data, fp)) != LOAD_HEX_SUCCESS) return ret;
			checksum = (checksum + data) & 0xff;
			if (data_type == 0x00) {
				/* データを設定する */
				if (0 <= address && address < out_size) {
					out[address] = data;
				} else {
					size_over_flag = 1;
				}
				/* 次のデータがある場合は、アドレスを進める */
				if (i + 1 < data_size) {
					if (address > INT_MAX - 1) return LOAD_HEX_SIZE_OVER;
					address++;
				}
			} else if (data_type == 0x02 || data_type == 0x04) {
				/* オーバーフローチェック */
				if ((INT_MAX >> 8) < address) return LOAD_HEX_SIZE_OVER;
				/* アドレスを更新する */
				address = (address << 8) | data;
			}
		}
		/* 読み込んだアドレスを反映させる */
		if (data_type == 0x02) {
			/* オーバーフローチェック */
			if ((INT_MAX >> 4) < address) return LOAD_HEX_SIZE_OVER;
			/* アドレスのオフセットを設定する */
			address_offset = address << 4;
		} else if (data_type == 0x04) {
			/* オーバーフローチェック */
			if ((INT_MAX >> 16) < address) return LOAD_HEX_SIZE_OVER;
			/* アドレスのオフセットを設定する */
			address_offset = address << 16;
		}
		/* チェックサムの読み込み */
		if ((ret = load_hex_byte(&i, fp)) != LOAD_HEX_SUCCESS) return ret;
		checksum = (checksum + i) & 0xff;
		if (checksum != 0x00) return LOAD_HEX_CHECKSUM_ERROR;
		/* End Of Fileなら終了する */
		if (data_type == 0x01) break;
	}
	return size_over_flag ? LOAD_HEX_SIZE_OVER : LOAD_HEX_SUCCESS;
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

#ifdef LOAD_HEX_TEST
/* 標準入力からHEXファイルを読み込み、標準出力に内容を出力する */
int main(void) {
	static char buffer[1024 * 1024];
	static unsigned int buffer_int[1024 * 1024 / 2];
	unsigned int i;
	int prev_printed = 0;
	int print_exists = 0;
	int words_per_line = 8;
	int ret;
	for (i = 0; i < sizeof(buffer) / sizeof(buffer[0]); i++) buffer[i] = 0xff;
	ret = load_hex(buffer, sizeof(buffer), stdin);
	if (ret != LOAD_HEX_SUCCESS) {
		fprintf(stderr, "error %d in load_hex\n", ret);
		return 1;
	}
	ret = chars_to_words(buffer_int, buffer, sizeof(buffer));
	if (ret != LOAD_HEX_SUCCESS) {
		fprintf(stderr, "error %d in  chars_to_words\n", ret);
		return 1;
	}
	for (i = 0; i < sizeof(buffer_int) / sizeof(buffer_int[0]); i++) {
		if (i % words_per_line == 0) {
			int print_it = 0;
			int j;
			for (j = 0; j < words_per_line; j++) {
				if (buffer_int[i + j] != 0xffff) {
					print_it = 1;
					break;
				}
			}
			if (!print_it) {
				i += words_per_line - 1;
				prev_printed = 0;
				continue;
			}
			if (print_exists) putchar('\n');
			if (!prev_printed) printf("address = %08X\n", i);
		} else {
			putchar(' ');
		}
		printf("%04X", buffer_int[i]);
		prev_printed = 1;
		print_exists = 1;
	}
	if (i % words_per_line == 0) putchar('\n');
	return 0;
}
#endif
