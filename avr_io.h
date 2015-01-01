#ifndef AVR_IO_H_GUARD_7FCA6973_B2F4_479A_9F39_2DFE7DE31870
#define AVR_IO_H_GUARD_7FCA6973_B2F4_479A_9F39_2DFE7DE31870

typedef struct {
	/* 1オクテット読み書きする関数 */
	int (*io_8bits)(int out);
	/* リセット出力を設定する関数
	 * reset_outが真 : HIGHを出力する
	 * reset_outが偽 : LOWを出力する
	 */
	void (*set_reset)(int reset_out);
	/* wait_timeミリ秒ウェイトを入れる */
	void (*wait)(int wait_time);
} avrio_t;

#endif
