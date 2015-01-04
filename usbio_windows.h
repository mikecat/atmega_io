#ifndef USBIO_WINDOWS_H_GUARD_618B5E6E_9646_432F_97B9_A7EAD7641536
#define USBIO_WINDOWS_H_GUARD_618B5E6E_9646_432F_97B9_A7EAD7641536

#include "avr_io.h"

/* USB-IO2.0を用いた通信を初期化する。
 * 成功と判定したら真、失敗を検出したら偽を返す。
 */
int usbio_init(avrio_t *avrio, int sin_port, int sout_port, int reset_port);

/* USB-IO2.0を用いた通信を終了する。
 * 成功と判定したら真、失敗を検出したら偽を返す。
 */
int usbio_stop(avrio_t *avrio);

/* USB-IO2.0を用いて8ビット送受信する。
 * 成功したら受信したデータを、失敗したら-1を返す。
 */
int usbio_io_8bits(void *hardware_data, int out);

/* USB-IO2.0を用いてリセットを行う。
 * 成功と判定したら真、失敗を検出したら偽を返す。
 */
int usbio_reset(void *hardware_data);

#endif
