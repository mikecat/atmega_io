#ifndef USBIO_WINDOWS_H_GUARD_618B5E6E_9646_432F_97B9_A7EAD7641536
#define USBIO_WINDOWS_H_GUARD_618B5E6E_9646_432F_97B9_A7EAD7641536

struct avrio_t;

/* USB-IO2.0を用いた通信を初期化する */
int usbio_init(struct avrio_t *avrio, int sin_port, int sout_port, int reset_port);
/* USB-IO2.0を用いた通信を終了する */
int usbio_stop(struct avrio_t *avrio);
/* USB-IO2.0を用いて8ビット送受信する */
int usbio_io_8bits(void *hardware_data, int out);

#endif
