#include "load_hex.h"

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
