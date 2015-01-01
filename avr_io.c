#include <stdio.h>
#include <limits.h>
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

/**
 * Poll RDY/~BSYを実行する。
 * 0が返ってくるまで処理を続ける。
 * @param func 利用する関数が格納された構造体へのポインタ
 * @return エラーコード
 */
static int wait_operation(const avrio_t *func) {
	static const int out_seq[4] = {0xF0, 0x00, 0x00, 0x00};
	int in_seq[4];
	int i;
	int ret;
	if (func == NULL) return AVRIO_INVALID_PARAMETER;
	do {
		/* 念のためin syncかを確認する */
		ret = send_programming_enable(func);
		if (ret != AVRIO_SUCCESS) return ret;
		/* ポーリングを行う */
		for (i = 0; i < 4; i++) {
			in_seq[i] = (func->io_8bits)(out_seq[i]);
			if (in_seq[i] < 0) return AVRIO_CONTROLLER_ERROR;
		}
	} while ((in_seq[3] & 1) != 0);
	return AVRIO_SUCCESS;
}

int read_signature_byte(const avrio_t *func, int *out) {
	int out_seq[4] = {0x30, 0x00, 0x00, 0x00};
	int i, j;
	int spe_ret;
	if (func == NULL || out == NULL) return AVRIO_INVALID_PARAMETER;
	spe_ret = send_programming_enable(func);
	if (spe_ret != AVRIO_SUCCESS) return spe_ret;
	for (i = 0; i < 3; i++) {
		int ret = 0;
		for (j = 0; j < 4; j++) {
			ret = (func->io_8bits)(out_seq[j]);
			if (ret < 0) return AVRIO_CONTROLLER_ERROR;
		}
		out_seq[2]++;
		out[i] = ret;
	}
	return AVRIO_SUCCESS;
}

int read_information(const avrio_t *func, int *lock_bits, int *fuse_bits,
int *fuse_high_bits, int *extended_fuse_bits, int *calibration_byte) {
	static const int out_seq[5][4] = {
		{0x58, 0x00, 0x00, 0x00},
		{0x50, 0x00, 0x00, 0x00},
		{0x58, 0x08, 0x00, 0x00},
		{0x50, 0x08, 0x00, 0x00},
		{0x38, 0x00, 0x00, 0x00}
	};
	int *ptr[5] = {
		lock_bits, fuse_bits, fuse_high_bits,
		extended_fuse_bits, calibration_byte
	};
	int i, j;
	int spe_ret;
	if (func == NULL) return AVRIO_INVALID_PARAMETER;
	spe_ret = send_programming_enable(func);
	if (spe_ret != AVRIO_SUCCESS) return spe_ret;
	for (i = 0; i < 5; i++) {
		int ret = 0;
		if (ptr[i] == NULL) continue;
		for (j = 0; j < 4; j++) {
			ret = (func->io_8bits)(out_seq[i][j]);
			if (ret < 0) return AVRIO_CONTROLLER_ERROR;
		}
		*ptr[i] = ret;
	}
	return AVRIO_SUCCESS;
}

int read_program(const avrio_t *func, unsigned int *data_out,
unsigned int start_addr, unsigned int data_size) {
	int out_seq[4];
	int spe_ret;
	unsigned int i, j;
	if (func == NULL || data_out == NULL ||
	UINT_MAX - data_size < start_addr || ((start_addr + data_size) & ~0xffff) != 0) {
		/* オーバーフローまたはアドレスがオーバーランする */
		return AVRIO_INVALID_PARAMETER;
	}
	spe_ret = send_programming_enable(func);
	if (spe_ret != AVRIO_SUCCESS) return spe_ret;
	out_seq[3] = 0x00;
	for (i = 0; i < data_size; i++) {
		int ret1, ret2;
		/* Low byteを読み込む */
		out_seq[0] = 0x20;
		out_seq[1] = ((start_addr + i) >> 8) & 0xff;
		out_seq[2] = (start_addr + i) & 0xff;
		for (j = 0; j < 4; j++) {
			ret1 = (func->io_8bits)(out_seq[j]);
			if (ret1 < 0) return AVRIO_CONTROLLER_ERROR;
		}
		/* High byteを読み込む */
		out_seq[0] = 0x28;
		for (j = 0; j < 4; j++) {
			ret2 = (func->io_8bits)(out_seq[j]);
			if (ret2 < 0) return AVRIO_CONTROLLER_ERROR;
		}
		/* 合体して格納する */
		data_out[i] = (unsigned int)ret1 | ((unsigned int)ret2 << 8);
	}
	return AVRIO_SUCCESS;
}

int read_eeprom(const avrio_t *func, int *data_out,
unsigned int start_addr, unsigned int data_size) {
	int out_seq[4] = {0xA0, 0x00, 0x00, 0x00};
	int spe_ret;
	unsigned int i, j;
	if (func == NULL || data_out == NULL ||
	UINT_MAX - data_size < start_addr || ((start_addr + data_size) & ~0x03ff) != 0) {
		/* オーバーフローまたはアドレスがオーバーランする */
		return AVRIO_INVALID_PARAMETER;
	}
	spe_ret = send_programming_enable(func);
	if (spe_ret != AVRIO_SUCCESS) return spe_ret;
	for (i = 0; i < data_size; i++) {
		int ret;
		out_seq[1] = ((start_addr + i) >> 8) & 0x03;
		out_seq[2] = (start_addr + i) & 0xff;
		for (j = 0; j < 4; j++) {
			ret = (func->io_8bits)(out_seq[j]);
			if (ret < 0) return AVRIO_CONTROLLER_ERROR;
		}
		data_out[i] = ret;
	}
	return AVRIO_SUCCESS;
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

int write_program(const avrio_t *func, const unsigned int *data,
unsigned int start_addr, unsigned int data_size, unsigned int page_size) {
	int spe_ret;
	spe_ret = send_programming_enable(func);
	if (spe_ret != AVRIO_SUCCESS) return spe_ret;
}

int write_eeprom(const avrio_t *func, const int *data,
unsigned int start_addr, unsigned int data_size, unsigned int page_size) {
	int spe_ret;
	spe_ret = send_programming_enable(func);
	if (spe_ret != AVRIO_SUCCESS) return spe_ret;
}
