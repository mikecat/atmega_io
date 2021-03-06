#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#if defined(USE_NANOSLEEP)
#include <time.h>
#include <errno.h>
#elif defined(USE_USLEEP)
#include <unistd.h>
#else
#include <windows.h>
#endif

#include "atmega_io.h"

/**
 * 指定した時間以上待つ
 * @param ms 待つ時間(ミリ秒)
 */
static void sleep_ms(int ms) {
#if defined(USE_NANOSLEEP)
	struct timespec req, ret;
	int r;
	req.tv_sec = ms / 1000;
	req.tv_nsec = (long)(ms % 1000) * 1000000L;
	for(;;) {
		r = nanosleep(&req, &ret);
		if (r == -1 && errno == EINTR) {
			req = ret;
		} else {
			break;
		}
	}
#elif defined(USE_USLEEP)
	usleep(ms * 1000);
#else
	Sleep(ms);
#endif
}

int disconnect(atmegaio_t *func) {
	if (func == NULL) return ATMEGAIO_INVALID_PARAMETER;
	if (!(func->disconnect)(func->hardware_data)) return ATMEGAIO_CONTROLLER_ERROR;
	free(func);
	return ATMEGAIO_SUCCESS;
}

int reset(const atmegaio_t *func) {
	if (func == NULL) return ATMEGAIO_INVALID_PARAMETER;
	if (!(func->reset)(func->hardware_data)) return ATMEGAIO_CONTROLLER_ERROR;
	return ATMEGAIO_SUCCESS;
}

/**
 * Programming Enableを送信する
 * @param func 利用する関数が格納された構造体へのポインタ
 * @return エラーコード
 */
static int send_programming_enable(const atmegaio_t *func) {
	static const int out_seq[4] = {0xAC, 0x53, 0x00, 0x00};
	int in_seq[4];
	int i;
	if (func == NULL) return ATMEGAIO_INVALID_PARAMETER;
	for (i = 0; i < 4; i++) {
		in_seq[i] = (func->io_8bits)(func->hardware_data, out_seq[i]);
		if (in_seq[i] < 0) return ATMEGAIO_CONTROLLER_ERROR;
	}
	return in_seq[2] == 0x53 ? ATMEGAIO_SUCCESS : ATMEGAIO_PROGRAMMING_ENABLE_ERROR;
}

/**
 * Poll RDY/~BSYを実行する。
 * 0が返ってくるまで処理を続ける。
 * @param func 利用する関数が格納された構造体へのポインタ
 * @param fixed_wait 真の場合、Poll RDY/~BSYを実行するのではなく、10ms待つ
 * @return エラーコード
 */
static int wait_operation(const atmegaio_t *func, int fixed_wait) {
	static const int out_seq[4] = {0xF0, 0x00, 0x00, 0x00};
	int in_seq[4];
	int i;
	int ret;
	if (func == NULL) return ATMEGAIO_INVALID_PARAMETER;
	if (fixed_wait) {
		sleep_ms(10);
	} else {
		do {
			/* 念のためin syncかを確認する */
			ret = send_programming_enable(func);
			if (ret != ATMEGAIO_SUCCESS) return ret;
			/* ポーリングを行う */
			for (i = 0; i < 4; i++) {
				in_seq[i] = (func->io_8bits)(func->hardware_data, out_seq[i]);
				if (in_seq[i] < 0) return ATMEGAIO_CONTROLLER_ERROR;
			}
		} while ((in_seq[3] & 1) != 0);
	}
	return ATMEGAIO_SUCCESS;
}

int read_signature_byte(const atmegaio_t *func, int *out) {
	int out_seq[4] = {0x30, 0x00, 0x00, 0x00};
	int i, j;
	int spe_ret;
	if (func == NULL || out == NULL) return ATMEGAIO_INVALID_PARAMETER;
	spe_ret = send_programming_enable(func);
	if (spe_ret != ATMEGAIO_SUCCESS) return spe_ret;
	for (i = 0; i < 3; i++) {
		int ret = 0;
		for (j = 0; j < 4; j++) {
			ret = (func->io_8bits)(func->hardware_data, out_seq[j]);
			if (ret < 0) return ATMEGAIO_CONTROLLER_ERROR;
		}
		out_seq[2]++;
		out[i] = ret;
	}
	return ATMEGAIO_SUCCESS;
}

int read_information(const atmegaio_t *func, int *lock_bits, int *fuse_bits,
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
	if (func == NULL) return ATMEGAIO_INVALID_PARAMETER;
	spe_ret = send_programming_enable(func);
	if (spe_ret != ATMEGAIO_SUCCESS) return spe_ret;
	for (i = 0; i < 5; i++) {
		int ret = 0;
		if (ptr[i] == NULL) continue;
		for (j = 0; j < 4; j++) {
			ret = (func->io_8bits)(func->hardware_data, out_seq[i][j]);
			if (ret < 0) return ATMEGAIO_CONTROLLER_ERROR;
		}
		*ptr[i] = ret;
	}
	return ATMEGAIO_SUCCESS;
}

int read_program(const atmegaio_t *func, unsigned int *data_out,
unsigned int start_addr, unsigned int data_size) {
	int out_seq[4];
	int spe_ret;
	unsigned int i, j;
	if (func == NULL || data_out == NULL ||
	UINT_MAX - data_size < start_addr || ((start_addr + data_size) & ~0xffff) != 0) {
		/* オーバーフローまたはアドレスがオーバーランする */
		return ATMEGAIO_INVALID_PARAMETER;
	}
	spe_ret = send_programming_enable(func);
	if (spe_ret != ATMEGAIO_SUCCESS) return spe_ret;
	out_seq[3] = 0x00;
	for (i = 0; i < data_size; i++) {
		int ret1, ret2;
		/* Low byteを読み込む */
		out_seq[0] = 0x20;
		out_seq[1] = ((start_addr + i) >> 8) & 0xff;
		out_seq[2] = (start_addr + i) & 0xff;
		for (j = 0; j < 4; j++) {
			ret1 = (func->io_8bits)(func->hardware_data, out_seq[j]);
			if (ret1 < 0) return ATMEGAIO_CONTROLLER_ERROR;
		}
		/* High byteを読み込む */
		out_seq[0] = 0x28;
		for (j = 0; j < 4; j++) {
			ret2 = (func->io_8bits)(func->hardware_data, out_seq[j]);
			if (ret2 < 0) return ATMEGAIO_CONTROLLER_ERROR;
		}
		/* 合体して格納する */
		data_out[i] = (unsigned int)ret1 | ((unsigned int)ret2 << 8);
	}
	return ATMEGAIO_SUCCESS;
}

int read_eeprom(const atmegaio_t *func, int *data_out,
unsigned int start_addr, unsigned int data_size) {
	int out_seq[4] = {0xA0, 0x00, 0x00, 0x00};
	int spe_ret;
	unsigned int i, j;
	if (func == NULL || data_out == NULL ||
	UINT_MAX - data_size < start_addr || ((start_addr + data_size) & ~0x03ff) != 0) {
		/* オーバーフローまたはアドレスがオーバーランする */
		return ATMEGAIO_INVALID_PARAMETER;
	}
	spe_ret = send_programming_enable(func);
	if (spe_ret != ATMEGAIO_SUCCESS) return spe_ret;
	for (i = 0; i < data_size; i++) {
		int ret;
		out_seq[1] = ((start_addr + i) >> 8) & 0x03;
		out_seq[2] = (start_addr + i) & 0xff;
		for (j = 0; j < 4; j++) {
			ret = (func->io_8bits)(func->hardware_data, out_seq[j]);
			if (ret < 0) return ATMEGAIO_CONTROLLER_ERROR;
		}
		data_out[i] = ret;
	}
	return ATMEGAIO_SUCCESS;
}

int chip_erase(const atmegaio_t *func, int fixed_wait) {
	static const int out_seq[4] = {0xAC, 0x80, 0x00, 0x00};
	int spe_ret;
	int i;
	spe_ret = send_programming_enable(func);
	if (spe_ret != ATMEGAIO_SUCCESS) return spe_ret;
	for (i = 0; i < 4; i++) {
		int ret = (func->io_8bits)(func->hardware_data, out_seq[i]);
		if (ret < 0) return ATMEGAIO_CONTROLLER_ERROR;
	}
	return wait_operation(func, fixed_wait);
}

int write_information(const atmegaio_t *func, int fixed_wait, int lock_bits,
int fuse_bits, int fuse_high_bits, int extended_fuse_bits) {
	int out_seq[4][4] = {
		{0xAC, 0xA0, 0x00, fuse_bits},
		{0xAC, 0xA8, 0x00, fuse_high_bits},
		{0xAC, 0xA4, 0x00, extended_fuse_bits},
		{0xAC, 0xE0, 0x00, lock_bits}
	};
	int spe_ret;
	int i, j;
	int ret;
	spe_ret = send_programming_enable(func);
	if (spe_ret != ATMEGAIO_SUCCESS) return spe_ret;
	for (i = 0; i < 4; i++) {
		if (out_seq[i][3] < 0) continue;
		/* 書き込みを行う */
		for (j = 0; j < 4; j++) {
			ret = (func->io_8bits)(func->hardware_data, out_seq[i][j]);
			if (ret < 0) return ATMEGAIO_CONTROLLER_ERROR;
		}
		/* 完了を待つ */
		ret = wait_operation(func, fixed_wait);
		if (ret != ATMEGAIO_SUCCESS) return ret;
	}
	return ATMEGAIO_SUCCESS;
}

int write_program(const atmegaio_t *func, int fixed_wait, const unsigned int *data,
unsigned int start_addr, unsigned int data_size, unsigned int page_size) {
	int out_seq[4];
	int spe_ret;
	unsigned int i, j;
	int ret;
	if (func == NULL || data == NULL ||
	UINT_MAX - data_size < start_addr || ((start_addr + data_size) & ~0xffff) != 0 ||
	page_size == 0 || start_addr % page_size != 0) {
		/* オーバーフローまたはアドレスがオーバーランするまたはアラインメント違反 */
		return ATMEGAIO_INVALID_PARAMETER;
	}
	spe_ret = send_programming_enable(func);
	if (spe_ret != ATMEGAIO_SUCCESS) return spe_ret;
	for (i = 0; i < data_size; i++) {
		/* Low byteをloadする */
		out_seq[0] = 0x40;
		out_seq[1] = 0;
		out_seq[2] = (start_addr + i) & 0xff;
		out_seq[3] = data[i] & 0xff;
		for (j = 0; j < 4; j++) {
			ret = (func->io_8bits)(func->hardware_data, out_seq[j]);
			if (ret < 0) return ATMEGAIO_CONTROLLER_ERROR;
		}
		/* High byteをloadする */
		out_seq[0] = 0x48;
		out_seq[3] = (data[i] >> 8) & 0xff;
		for (j = 0; j < 4; j++) {
			ret = (func->io_8bits)(func->hardware_data, out_seq[j]);
			if (ret < 0) return ATMEGAIO_CONTROLLER_ERROR;
		}
		/* データの終わりまたはページの区切り */
		if ((i + 1) % page_size == 0 || (i + 1) >= data_size) {
			/* PageをWriteする */
			out_seq[0] = 0x4C;
			out_seq[1] = ((start_addr + i) >> 8) & 0xff;
			out_seq[3] = 0x00;
			for (j = 0; j < 4; j++) {
				ret = (func->io_8bits)(func->hardware_data, out_seq[j]);
				if (ret < 0) return ATMEGAIO_CONTROLLER_ERROR;
			}
			/* 完了を待つ */
			ret = wait_operation(func, fixed_wait);
			if (ret != ATMEGAIO_SUCCESS) return ret;
		}
	}
	return ATMEGAIO_SUCCESS;
}

int write_eeprom(const atmegaio_t *func, int fixed_wait, const int *data,
unsigned int start_addr, unsigned int data_size) {
	int out_seq[4];
	int spe_ret;
	spe_ret = send_programming_enable(func);
	if (spe_ret != ATMEGAIO_SUCCESS) return spe_ret;
	unsigned int i, j;
	int ret;
	if (func == NULL || data == NULL ||
	UINT_MAX - data_size < start_addr || ((start_addr + data_size) & ~0x03ff) != 0) {
		/* オーバーフローまたはアドレスがオーバーランする */
		return ATMEGAIO_INVALID_PARAMETER;
	}
	for (i = 0; i < data_size; i++) {
		/* loadする */
		out_seq[0] = 0xC1;
		out_seq[1] = 0;
		out_seq[2] = (start_addr + i) & 0x03;
		out_seq[3] = data[i] & 0xff;
		for (j = 0; j < 4; j++) {
			ret = (func->io_8bits)(func->hardware_data, out_seq[j]);
			if (ret < 0) return ATMEGAIO_CONTROLLER_ERROR;
		}
		/* データの終わりまたはページの区切り */
		if ((i + 1) % 4 == 0 || (i + 1) >= data_size) {
			/* PageをWriteする */
			out_seq[0] = 0xC2;
			out_seq[1] = ((start_addr + i) >> 8) & 0x03;
			out_seq[2] = (start_addr + i) & 0xFC;
			out_seq[3] = 0x00;
			for (j = 0; j < 4; j++) {
				ret = (func->io_8bits)(func->hardware_data, out_seq[j]);
				if (ret < 0) return ATMEGAIO_CONTROLLER_ERROR;
			}
			/* 完了を待つ */
			ret = wait_operation(func, fixed_wait);
			if (ret != ATMEGAIO_SUCCESS) return ret;
		}
	}
	return ATMEGAIO_SUCCESS;
}
