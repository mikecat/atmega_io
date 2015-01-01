#ifndef AVR_IO_H_GUARD_7FCA6973_B2F4_479A_9F39_2DFE7DE31870
#define AVR_IO_H_GUARD_7FCA6973_B2F4_479A_9F39_2DFE7DE31870

/* AVRの読み書きに必要な操作を行う関数の情報を持つ構造体 */
typedef struct {
	/* 1オクテット読み書きする関数
	 * 成功したら読み込んだ値(0以上255以下)、失敗したら-1を返す。
	 */
	int (*io_8bits)(int out);
	/* リセット出力を設定する関数
	 * reset_outが真 : HIGHを出力する
	 * reset_outが偽 : LOWを出力する
	 * 成功したら真を、失敗したら偽を返す。
	 */
	int (*set_reset)(int reset_out);
	/* wait_timeミリ秒ウェイトを入れる関数
	 * 成功したら真を、失敗したら偽を返す。
	 */
	int (*wait)(int wait_time);
} avrio_t;

/* エラーコード */
enum {
	/* 成功と判定された */
	AVRIO_SUCCESS = 0,
	/* 関数のパラメータが不正 */
	AVRIO_INVALID_PARAMETER,
	/* AVR操作関数が失敗を返した */
	AVRIO_CONTROLLER_ERROR,
	/* Programming Enableで接続失敗を検出した */
	AVRIO_PROGRAMMING_ENABLE_ERROR
};

/**
 * Signature Byteを読み込む。
 * outはあらかじめ3要素以上確保しておかないといけない。
 * @param func 利用する関数が格納された構造体へのポインタ
 * @param out 読み込んだSignature Byteを保存する配列
 * @return エラーコード
 */
int read_signature_byte(const avrio_t *func, int *out);

/**
 * 各種情報を読み込む。不要なパラメータはNULLで良い。
 * @param func 利用する関数が格納された構造体へのポインタ
 * @param lock_bits Lock bitsを読み込む領域へのポインタ
 * @param fuse_bits Fuse bitsを読み込む領域へのポインタ
 * @param fuse_high_bits Fuse High bitsを読み込む領域へのポインタ
 * @param extended_fuse_bits Extended Huse Bitsを読み込む領域へのポインタ
 * @param calibration_byte Calibration Byteを読み込む領域へのポインタ
 * @return エラーコード
 */
int read_information(const avrio_t *func, int *lock_bits, int *fuse_bits,
	int *fuse_high_bits, int *extended_fuse_bits, int *calibration_byte);

/**
 * プログラムデータを読み込む。
 * data_outはあらかじめ十分な領域を確保しておかないといけない。
 * @param func 利用する関数が格納された構造体へのポインタ
 * @param data_out 読み込んだプログラムデータを格納する配列
 * @param start_addr 読み込みを開始するプログラムデータのアドレス
 * @param data_size 読み込むプログラムのワード数
 * @return エラーコード
 */
int read_program(const avrio_t *func, int *data_out, int start_addr, int data_size);

/**
 * EEPROMのデータを読み込む。
 * data_outはあらかじめ十分な領域を確保しておかないといけない。
 * @param func 利用する関数が格納された構造体へのポインタ
 * @param data_out 読み込んだEEPROMデータを格納する配列
 * @param start_addr 読み込みを開始するEEPROMのアドレス
 * @param data_size 読み込むEEPROMデータのワード数
 * @return エラーコード
 */
int read_eeprom(const avrio_t *func, int *data_out, int start_addr, int data_size);

#endif
