#ifndef __TEST3_H
#define __TEST3_H

#include "timer.h"
#include "keyboard.h"

/** @defgroup test3 test3
 * @{
 *
 * Functions for testing the kbd code
 */

/**
 * @brief Test the reading of scancodes through keyboard interrupts
 *
 * Prints the scancodes sent from the keyboard, indicating whether or not the scancodes are make or break code.
 * Exits upon release of the ESC key
 *
 * @param assembly 0-> Assembly, 1-> C
 *
 * @return Return 0 upon success; non-zero otherwise
 */
int kbd_test_scan(unsigned short assembly);

/**
 * @brief Test the reading of scancodes through keyboard polling
 *
 * Prints the scancodes sent from the keyboard, indicating whether or not the scancodes are make or break code.
 * Exits upon release of the ESC key
 *
 * @return Return 0 upon success; non-zero otherwise
 */
int kbd_test_poll();

/**
 * @brief To test handling of more than one interrupt
 *
 *  Prints the scancodes sent from the keyboard, indicating whether or not the scancodes are make or break code.
 *  Exits upon release of the ESC key or if no scancodes are received for n seconds
 *
 * @param n Number of seconds without scancodes before exiting
 *
 * @return Return 0 upon success; non-zero otherwise
 */
int kbd_test_timed_scan(unsigned short n);

#endif /* __TEST3_H */
