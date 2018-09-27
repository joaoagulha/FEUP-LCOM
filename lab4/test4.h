#ifndef __TEST3_H
#define __TEST3_H

#include "timer.h"
#include "mouse.h"

/** @defgroup test3 test3
 * @{
 *
 * Functions for testing the mouse code
 */

/**
 * @brief prints the packet information on a user friendly way
 *
 */
void print_packet();

/**
 * @brief test if screen prints the cnt number of packets corresponding to the typed mouse information, through stream mode using interruptions
 *
 * @param cnt -> number of packets to be displayed
 *
 * @return Return 0 on success. 1 on failure
 */
int mouse_test_packet(unsigned short cnt);

/**
 * @brief test if screen prints of packets corresponding to the typed mouse information for idle_time number of seconds counting from the last time the mouse read information
 *
 * @param idle_time -> number of seconds to the program keep running counting from the last time the mouse read information
 *
 * @return Return 0 on success. 1 on failure
 */
int mouse_test_async(unsigned short idle_time);

/**
 * @brief test if screen prints the cnt number of packets corresponding to the typed mouse information, through remote mode
 *
 * @param period -> number of miliSeconds for the program to freeze
 * @param cnt -> number of packets to be displayed
 *
 * @return Return 0 on success. 1 on failure
 */
int mouse_test_remote(unsigned long period, unsigned short cnt);

/**
 * @brief test if the program recognizes a continous positive slope either upwards or downwards
 *
 * @param length -> number of mouse points that define the line (length of the line)
 *
 * @return Return 0 on success. 1 on failure
 */
int mouse_test_gesture(short length);

#endif /* __TEST4_H */
