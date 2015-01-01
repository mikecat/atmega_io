#include <stdio.h>
#include "avr_io.h"

/**
 * Programming Enableを送信する
 * @param func 利用する関数が格納された構造体へのポインタ
 * @return エラーコード
 */
static int send_programming_enable(const avrio_t *func) {
	static const int out_seq[4] = {0xAC, 0x53, 0x00, 0x00};
	int in_seq[4];
	int i;
	if (func == NULL) return AVRIO_INVALID_PARAMETER;
	for (i = 0; i < 4; i++) {
		in_seq[i] = (func->io_8bits)(out_seq[i]);
		if (in_seq[i] < 0) return AVRIO_CONTROLLER_ERROR;
	}
	return in_seq[2] == 0x53 ? AVRIO_SUCCESS : AVRIO_PROGRAMMING_ENABLE_ERROR;
}

int read_signature_byte(const avrio_t *func, int *out) {
	int spe_ret;
	spe_ret = send_programming_enable(func);
	if (spe_ret != AVRIO_SUCCESS) return spe_ret;
	return 0;
}

int read_information(const avrio_t *func, int *lock_bits, int *fuse_bits,
int *fuse_high_bits, int *extended_fuse_bits, int *calibration_byte) {
	int spe_ret;
	spe_ret = send_programming_enable(func);
	if (spe_ret != AVRIO_SUCCESS) return spe_ret;
	return 0;
}

int read_program(const avrio_t *func, int *data_out, int start_addr, int data_size) {
	int spe_ret;
	spe_ret = send_programming_enable(func);
	if (spe_ret != AVRIO_SUCCESS) return spe_ret;
	return 0;
}

int read_eeprom(const avrio_t *func, int *data_out, int start_addr, int data_size) {
	int spe_ret;
	spe_ret = send_programming_enable(func);
	if (spe_ret != AVRIO_SUCCESS) return spe_ret;
	return 0;
}

int chip_erase(const avrio_t *func) {
	int spe_ret;
	spe_ret = send_programming_enable(func);
	if (spe_ret != AVRIO_SUCCESS) return spe_ret;
}

int write_information(const avrio_t *func, int lock_bits, int fuse_bits,
int fuse_high_bits, int extended_fuse_bits, int calibration_byte) {
	int spe_ret;
	spe_ret = send_programming_enable(func);
	if (spe_ret != AVRIO_SUCCESS) return spe_ret;
}

int write_program(const avrio_t *func, const int *data,
int start_addr, int data_size, int page_size) {
	int spe_ret;
	spe_ret = send_programming_enable(func);
	if (spe_ret != AVRIO_SUCCESS) return spe_ret;
}

int write_eeprom(const avrio_t *func, const int *data,
int start_addr, int data_size, int page_size) {
	int spe_ret;
	spe_ret = send_programming_enable(func);
	if (spe_ret != AVRIO_SUCCESS) return spe_ret;
}
