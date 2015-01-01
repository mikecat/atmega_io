#ifndef AVR_IO_H_GUARD_7FCA6973_B2F4_479A_9F39_2DFE7DE31870
#define AVR_IO_H_GUARD_7FCA6973_B2F4_479A_9F39_2DFE7DE31870

/* AVRの読み書きに必要な操作を行う関数の情報を持つ構造体 */
typedef struct {
	/* 1オクテット読み書きする関数
	 * 成功したら読み込んだ値(0以上255以下)、失敗したら-1を返す。
	 */
	int (*io_8bits)(int out);
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

/**
 * Chip Eraseを行う。
 * @param func 利用する関数が格納された構造体へのポインタ
 * @return エラーコード
 */
int chip_erase(const avrio_t *func);

/**
 * 各種情報を書き込む。書き込まない情報は-1を入れる。
 * @param func 利用する関数が格納された構造体へのポインタ
 * @param lock_bits Lock bitsに書き込むデータ
 * @param fuse_bits Fuse bitsをに書き込むデータ
 * @param fuse_high_bits Fuse High bitsに書き込むデータ
 * @param extended_fuse_bits Extended Huse Bitsに書き込むデータ
 * @param calibration_byte Calibration Byteに書き込むデータ
 * @return エラーコード
 */
int write_information(const avrio_t *func, int lock_bits, int fuse_bits,
	int fuse_high_bits, int extended_fuse_bits, int calibration_byte);

/**
 * プログラムデータを書き込む。
 * data_outはあらかじめ十分な領域を確保しておかないといけない。
 * @param func 利用する関数が格納された構造体へのポインタ
 * @param data 書き込むプログラムデータを格納する配列
 * @param start_addr 書き込みを開始するプログラムデータのアドレス
 * @param data_size 書き込むプログラムのワード数
 * @param page_size 書き込みに使用するページサイズ(適切に設定しないと失敗します)
 * @return エラーコード
 */
int write_program(const avrio_t *func, const int *data,
	int start_addr, int data_size, int page_size);

/**
 * EEPROMのデータを書き込む。
 * page_sizeを0以下にすると、1ワードずつ書き込みます。
 * data_outはあらかじめ十分な領域を確保しておかないといけない。
 * @param func 利用する関数が格納された構造体へのポインタ
 * @param data 書き込むEEPROMデータを格納する配列
 * @param start_addr 書き込みを開始するEEPROMのアドレス
 * @param data_size 書き込むEEPROMデータのワード数
 * @param page_size 書き込みに使用するページサイズ(適切に設定しないと失敗します)
 * @return エラーコード
 */
int write_eeprom(const avrio_t *func, const int *data,
	int start_addr, int data_size, int page_size);

#endif
