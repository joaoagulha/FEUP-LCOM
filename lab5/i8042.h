#ifndef __T8042_H
#define __I8042_H

#define BIT(n) (0x01<<(n))
#define FINE 0
#define NOT_FINE 1
#define TIMEOUT 4 //number of times kbd waits to receive message
#define TIME_TO_WAIT 20000
#define KBD_IRQ 1
#define MOUSE_IRQ	12
#define KBC_MAX_ATTEMPTS 3

//Ports
#define KBD_STATUS_REG 0x64
#define MOUSE_STATUS_REG 0x64
#define KBD_BUF 0x60
#define OUT_BUF 0x60
#define IN_BUF 0x20

//Commands
#define MOUSE_ENABLE 0xA8
#define KBD_ENABLE 0xAE
#define WRITE_TO_MOUSE 0xD4
#define DIS_DATA_REPORT 0xF5
#define ENA_DATA_REPORT 0xF4
#define SET_REMOTE_MODE 0xF0
#define SET_STREAM_MODE	0xEA
#define READ_DATA	0xEB
#define ACK	0xFA
#define NACK 0xFE
#define ERROR 0xFC
#define RESEND 0xFE

//Scan Codes
#define OUF_FLAG BIT(0)
#define IBF_FLAG BIT(1)
#define AUX_FLAG BIT(5)
#define ESC_KEY_CODE 0x81
#define TWO_BYTE_SCAN_CODE 0xE0
#define KBD_STAT_REG 0x1
#define IN_BUF_FULL 0x2
#define TIMEOUT_ERR 0x40 //BIT 6
#define PARITY_ERR 0x80 //BIT 7
#define KEY_RELEASED BIT(7)

//Mouse
#define L_BUTTON BIT(0)
#define R_BUTTON BIT(1)
#define M_BUTTON BIT(2)
#define X_SIGN BIT(4)
#define Y_SIGN BIT(5)
#define X_OVFL BIT(6)
#define Y_OVFL BIT(7)

#endif /* __TEST3_H */
