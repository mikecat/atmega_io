#include "avr_io.h"

int read_signature_byte(const avrio_t *func, int *out) {
	return 0;
}

int read_information(const avrio_t *func, int *lock_bits, int *fuse_bits,
int *fuse_high_bits, int *extended_fuse_bits, int *calibration_byte) {
	return 0;
}

int read_program(const avrio_t *func, int *data_out, int start_addr, int data_size) {
	return 0;
}

int read_eeprom(const avrio_t *func, int *data_out, int start_addr, int data_size) {
	return 0;
}
