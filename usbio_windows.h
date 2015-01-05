#ifndef USBIO_WINDOWS_H_GUARD_618B5E6E_9646_432F_97B9_A7EAD7641536
#define USBIO_WINDOWS_H_GUARD_618B5E6E_9646_432F_97B9_A7EAD7641536

#include "avr_io.h"

/* USB-IO2.0を用いた通信を初期化する。
 * 成功と判定したら通信用データのポインタ、失敗を検出したらNULLを返す。
 */
avrio_t *usbio_init(int sin_port, int sout_port, int clock_port, int reset_port);

#endif
