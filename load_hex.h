#ifndef LOAD_HEX_H_GUARD_EC96B4AC_8C4A_4575_8640_BAA6335F87FF
#define LOAD_HEX_H_GUARD_EC96B4AC_8C4A_4575_8640_BAA6335F87FF

#include <stdio.h>

/**
 * ファイルハンドルからHEXファイルを読み込む。
 * @param out ファイルのデータを書き込むバッファ
 * @param out_size outのバッファサイズ
 * @param fp 読み込みに使用するファイルハンドル
 * @return エラーコード
 */
int load_hex(char *out, int out_size, FILE *fp);

/**
 * char型のデータ配列をワード配列に変換する。
 * @param out 変換したデータを書き込むバッファ
 * @param in 変換するデータ
 * @param in_bytes inの要素数
 * @return エラーコード
 */
int chars_to_words(unsigned int *out, const char *in, int in_bytes);

#endif
