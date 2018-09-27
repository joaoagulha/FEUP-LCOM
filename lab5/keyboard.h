#ifndef __KBD_H
#define __KBD_H

#include <minix/drivers.h>
#include "i8042.h"

/** @defgroup test3 test3
 * @{
 *
 * Functions for testing the kbd code
 */

/**
 * @brief Subscribes and enables Keyboard interrupts
 *
 *
 * @return Returns bit order in interrupt mask; negative value on failure
 */
int kbd_subscribe_int();

/**
 * @brief Unsubscribes Keyboard interrupts
 *
 * @return Return 0 upon success; negative value on failure
 */
int kbd_unsubscribe_int();

/**
 * @brief returns scanned key code
 *
 * @param assembly -> number to distinguish between read keyboard scanned with code written in C language or code written in assembly code
 *
 *
 * @return Return scanned key code. 1 on failure
 */
unsigned long read_keyboard(unsigned short assembly);

/**
 * @brief reads the scanned key code from the output buffer in C language
 *
 * @return Return the scanned key code. 1 on failure
 *
 */
unsigned long read_keyboard_key_c();

/**
 * @brief reads the scanned key code from the output buffer in assembly language
 *
 * @return Return the scanned key code. 1 on failure
 *
 */
unsigned long read_keyboard_key_assembly();

/**
 *
 *
 */
int write_to_keyboard_controller(unsigned long port, unsigned long commandByte);

/**
 * @brief enables keyboard interrupts
 *
 * @return Return 1 on failure
 *
 */
int enable_keyboard_interrupts();

/**
 * @brief get sys_in calls counter
 *
 * @return return sys_in calls counter
 */
unsigned int get_sys_in_counter();

#endif
