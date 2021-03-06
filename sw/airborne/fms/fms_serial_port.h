#ifndef FMS_SERIAL_PORT_H
#define FMS_SERIAL_PORT_H

#include <termios.h>

struct FmsSerialPort {
  int fd;                         /* serial device fd          */
  struct termios  orig_termios;   /* saved tty state structure */
  struct termios  cur_termios;    /* tty state structure       */
};

extern struct FmsSerialPort* serial_port_new(void);
extern void serial_port_free(struct FmsSerialPort* me);
extern void serial_port_flush(struct FmsSerialPort* me);
extern void serial_port_flush_output(struct FmsSerialPort* me);
int  serial_port_open_raw(struct FmsSerialPort* me, const char* device, speed_t speed);
extern int  serial_port_open(struct FmsSerialPort* me, const char* device, 
			     void(*term_conf_callback)(struct termios*, speed_t*));
extern void serial_port_close(struct FmsSerialPort* me);

#endif /* FMS_SERIAL_PORT_H */
