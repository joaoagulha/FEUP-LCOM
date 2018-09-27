#ifndef __mouse_H
#define __mouse_H

#include <minix/drivers.h>
#include "i8042.h"
#include "i8254.h"

/** @defgroup test3 test3
 * @{
 *
 * Functions for testing the kbd code
 */

/*
 *@brief delays time equivalent to a defined constant
 *
 **/
void delay();

/*
 * @brief clears the output buffer from anything left there
 *
 * @return Return 0 on success. 1 on failure
 */
unsigned int clear_output_buffer();

/*
 * @brief subscribes and enables mouse interrupts
 *
 *
 * @return Returns bit order in interrupt mask; negative value on failure
 */
int mouse_subscribe_int();

/*
 * @brief unsubscribes mouse interrupts
 *
 *
 * @return Return 0 upon success; negative value on failure
 */
int mouse_unsubscribe_int();

/*
 * @brief reads from the mouse controller the scanned key code
 *
 * @return Return scanned key code on success. 1 on failure
 */
int read_from_mouse_controller();

/*
 * @brief returns scanned key code
 *
 * @param port -> destination of the argument commandByte
 * @param commandByte -> command information to be sent to the argument port
 *
 * @return Return 0 on success. 1 on failure
 */
unsigned int write_to_mouse_controller(unsigned long port,
		unsigned long commandByte);

/*
 * @brief returns scanned key code
 *
 * @param commandByte -> command information to be sent to the mouse control register
 *
 * @return Return 0 on success. 1 on failure
 */
unsigned int configure_mouse(unsigned long commandByte);

/*
 * @brief enable mouse device
 *
 * @return Return 0 on success. 1 on failure
 */
unsigned int enable_mouse();

/*
 * @brief disables the mouse stream mode
 *
 *
 * @return Return 0 on success. 1 on failure
 */
unsigned int disable_mouse_stream_mode();

/*
 * @brief enables data reporting
 *
 *
 * @return Return 0 on success. 1 on failure
 */
unsigned int enable_data_reporting();

/*
 * @brief enables the mouse stream mode
 *
 *
 * @return Return 0 on success. 1 on failure
 */
unsigned int enable_mouse_stream_mode();

/*
 * @brief enables the sending of data packets
 *
 *
 * @return Return 0 on success. 1 on failure
 */
unsigned int enable_sending_data_packets();

/**
 * @brief disables mouse interrupts
 *
 *
 * @return Return 0 on success. 1 on failure
 */
unsigned int disable_mouse_interrupts();

/**
 * @brief enables sending of data on remote mode
 *
 * @return Return 0 on success. 1 on failure
 */
unsigned int enable_sending_data_remote();

/**
 * @brief disables sending of data on remote mode
 *
 * @return Return 0 on success. 1 on failure
 */
unsigned int disable_sending_data_remote();

/**
 *@brief verifies if the current packet makes part of an continuous positive slope
 *
 * @param length -> length of the countinous positive slope
 * @param packet[3] -> packets that have the current information of the mouse typed information
 *
 * @return Return 0 on success. 1 on failure
 */
unsigned int verify_positive_slope(short length, unsigned char packet[3]);

/*
 * @brief gives a packet with the mouse typed information
 *
 *
 * @return returns a packet with the mouse typed information 1 on failure
 */
int getPacket();

#endif /*  __mouse_H    */

