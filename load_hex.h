#ifndef LOAD_HEX_H_GUARD_EC96B4AC_8C4A_4575_8640_BAA6335F87FF
#define LOAD_HEX_H_GUARD_EC96B4AC_8C4A_4575_8640_BAA6335F87FF

#include <stdio.h>

enum {
	LOAD_HEX_SUCCESS = 0, /* 成功 */
	LOAD_HEX_INVALID_PARAMETER, /* 引数が不正 */
	LOAD_HEX_SIZE_OVER, /* 確保された範囲よりデータが大きい */
	LOAD_HEX_IO_ERROR, /* ファイル操作エラー */
	LOAD_HEX_INVALID_CHAR, /* ファイルに不正な文字が含まれる */
	LOAD_HEX_CHECKSUM_ERROR, /* チェックサムが一致しない */
	LOAD_HEX_UNEXPECTED_EOF /* 予期せぬファイル終端 */
};

/**
 * ファイルハンドルからHEXファイルを読み込む。
 * out_sizeは非負でなければならない。
 * out_size番地以上にデータを書き込もうとされた場合、
 * かつ他のエラーが検出されない場合は、LOAD_HEX_SIZE_OVERが返される。
 * @param out ファイルのデータを書き込むバッファ
 * @param out_size outのバッファサイズ
 * @param fp 読み込みに使用するファイルハンドル
 * @return エラーコード
 */
int load_hex(char *out, int out_size, FILE *fp);

/**
 * char型のデータ配列をワード配列に変換する。
 * in_bytesは非負の偶数でないといけない。
 * @param out 変換したデータを書き込むバッファ
 * @param in 変換するデータ
 * @param in_bytes inの要素数
 * @return エラーコード
 */
int chars_to_words(unsigned int *out, const char *in, int in_bytes);

#endif
