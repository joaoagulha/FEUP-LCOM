#include "test3.h"

//unsigned long read_keyboard_key_polling();
int print_scan_code(unsigned long scanned_key_code, unsigned int *two_byte);

int kbd_test_scan(unsigned short assembly) {

	unsigned long scanned_key_code;
	int ipc_configuration, driver_receive_error, irq_set_kbd;
	message msg;
	unsigned int two_byte = 0;

	//check if given assembly value referring to C or Assembly is valid, and if not print message and return 1
	if (!(assembly == FINE || assembly == (FINE + 1))) {
		printf(
				"\n\nWARNING: an error occurred. Invalid given programming language respective value \nStack Trace: \n->kbd_test_scan()");
		return NOT_FINE;
	}

	if ((irq_set_kbd = kbd_subscribe_int()) == -NOT_FINE) {
		printf("\n->kbd_test_scan()");
		return NOT_FINE;
	}

	//INTERRUPTION LOOP -> until the keyboard key is not ESC key
	while (scanned_key_code != ESC_KEY_CODE) {

		/* get request message */
		//driver_receive_error will have the received message sent by the DD (kernel or some other processes), upon making a request
		//ANY -> sender message (any sender)
		//&msg -> where the message will be put. used to avoid having to make a call to the driver_receive function several times
		//&ipc_configuration -> where the ipc configuration, which contains the message configuration (standard message or a kernel notification, will be put
		if ((driver_receive_error = driver_receive(ANY, &msg,
				&ipc_configuration)) != FINE) {
			printf(
					"\n\nWARNING: an error occurred. driver_receive failed with: %d \nStack Trace: \n->driver_receive() \n->kbd_test_scan()",
					driver_receive_error);
			continue;
		}

		// receives notification of the interrupt request. returns true if the msg is received and it is a notification and false otherwise
		if (is_ipc_notify(ipc_configuration)) { //received notification

			//Endpoint has both a source and destination
			//_ENDPOINT_P -> extracts the proccess identifier from a process endpoint
			//msg.m_source -> sender of the message's endpoint -> address with the information about communication endpoints
			switch (_ENDPOINT_P(msg.m_source)) {
			case HARDWARE: //hardware interrupt notification

				// hardware interrupt notification
				// msg.NOTIFY_ARG -> bitmask with the bits of the active interrupts sub, call the corresponding interrupt handler
				// irq_set is written in .NOTIFY_ARG anytime keyboard interrupt is generated
				if (msg.NOTIFY_ARG & irq_set_kbd) { //subscribed interrupt

					if ((scanned_key_code = read_keyboard(assembly)) == FINE) {
						printf("\n->kbd_test_scan()");
						return NOT_FINE;
					}

					print_scan_code(scanned_key_code, &two_byte);
				}
				break;
			default:
				printf(
						"\n\nWARNING: an error occurred. _ENDPOINT_P(msg.m_source) \nStack Trace: \n->driver_receive() \n->kbd_test_scan()");
				break;
			}
		} else { /* received a standard message, not a notification */
			printf(
					"\n\nWARNING: an error occurred. Received Standard Notification. \nStack Trace: \n->driver_receive() \n->kbd_test_scan()");
		}
	}

	if (kbd_unsubscribe_int() == -NOT_FINE) {
		printf("\n->kbd_test_scan()");
		return NOT_FINE;
	}
	if (assembly) {
		printf("\nNumber of sys_in(): %d", get_sys_in_counter());
	}
	printf("\n\nkbd_test_scan() function was correctly executed\n\n");
	return FINE;
}

int kbd_test_poll() {
	//Before running must disable interrupts with the program provided, to prevent Minix’s keyboard IH from "stealing" the scan codes from the OUT_BUF  before the handler does it

	unsigned long scanned_key_code;
	unsigned int two_byte = 0;

	while (scanned_key_code != ESC_KEY_CODE) {

		scanned_key_code = read_keyboard_key_c();
		if (scanned_key_code != NOT_FINE) {
			print_scan_code(scanned_key_code, &two_byte);
		}
	}

	printf("\nNumber of sys_in(): %d", get_sys_in_counter());

	if (enable_keyboard_interrupts() != FINE) {
		printf("\nkbd_test_poll");
		return NOT_FINE;
	}

	printf("\nNumber of sys_in(): %d", get_sys_in_counter());
	printf("\n\nkbd_test_poll() function was correctly executed\n\n");
	return FINE;
}

int kbd_test_timed_scan(unsigned short n) {

	int irq_set_timer, irq_set_kbd, driver_receive_error, ipc_configuration;
	message msg;
	unsigned int enlapsed_time = 0, two_byte = 0;
	unsigned long scanned_key_code;

	if (n < (FINE + 1)) {
		printf(
				"\n\nWARNING: an error occurred. Invalid given time \nStack Trace: \n->kbd_test_timed_scan()");
		return NOT_FINE;
	}

	//irq_set _> bit mask that will have the bits matching the hook_id_IO set to TIMER0_IRQ in interrupt mask
	if ((irq_set_timer = timer_subscribe_int()) == -NOT_FINE) {
		printf("\n->kbd_test_timed_scan()");
		return NOT_FINE;
	}

	//irq_set _> bit mask that will have the bits matching the hook_id_IO set to IRQ_KBD in interrupt mask
	if ((irq_set_kbd = kbd_subscribe_int()) == -NOT_FINE) {
		printf("\n->kbd_test_timed_scan()");
		return NOT_FINE;
	}

	n = n * TIMER_DEFAULT_FREQ; // will be a tick counter

	//INTERRUPTION LOOP -> until the time has not yet passed.
	while (scanned_key_code != ESC_KEY_CODE && enlapsed_time < n) {

		/* get request message */
		//driver_receive_error will have the received message sent by the DD (kernel or some other processes), upon making a request
		//ANY -> sender message (any sender)
		//&msg -> where the message will be put. used to avoid having to make a call to the driver_receive function several times
		//&ipc_configuration -> where the ipc configuration, which contains the message configuration (standard message or a kernel notification, will be put
		if ((driver_receive_error = driver_receive(ANY, &msg,
				&ipc_configuration))) {
			printf("driver_receive failed with: %d", driver_receive_error);
			continue;
		}

		// receives notification of the interrupt request. returns true if the msg is received and it is a notification and false otherwise
		if (is_ipc_notify(ipc_configuration)) {

			//Endpoint has both a source and destination
			//_ENDPOINT_P -> extracts the proccess identifier from a process endpoint
			//msg.m_source -> sender of the message's endpoint -> address with the information about communication endpoints
			switch (_ENDPOINT_P(msg.m_source)) {
			case HARDWARE:

				// hardware interrupt notification
				// msg.NOTIFY_ARG -> bitmask with the bits of the active interrupts sub, call the corresponding interrupt handler
				// irq_set_timer is written in .NOTIFY_ARG anytime TIMER_0 interrupt is generated
				if (msg.NOTIFY_ARG & irq_set_timer) { /* subscribed interrupt */
					//calls the interrupt handler
					timed_scan_int_handler(&enlapsed_time);
					if ((enlapsed_time != FINE)
							&& (enlapsed_time % TIMER_DEFAULT_FREQ == FINE)) {
						//1 second -> 60hz default frequency, 60 ticks -> which means the message will be printed at each 60 ticks, and so it will be printed at each one second passed
						printf("\n%d seconds\n",
								(enlapsed_time / TIMER_DEFAULT_FREQ));
					}
				}

				// hardware interrupt notification
				// msg.NOTIFY_ARG -> bitmask with the bits of the active interrupts sub, call the corresponding interrupt handler
				// irq_set_kbd is written in .NOTIFY_ARG anytime keyboard interrupt is generated
				if (msg.NOTIFY_ARG & irq_set_kbd) {
					if ((scanned_key_code = read_keyboard_key_c()) == FINE) {
						printf("\n->kbd_test_scan()");
						return NOT_FINE;
					}

					if (print_scan_code(scanned_key_code, &two_byte)) {
						enlapsed_time = 0;
					}

				}

				break;
			default:
				break; /* no other notifications expected: do nothing */
			}
		} else {/*received a standard message, not a notification*/
			/*no standard messages expected, do nothing*/
		}

	}

	//Disable kbd interrupts and unsubscribe them from the line
	if (kbd_unsubscribe_int() == -NOT_FINE) {
		printf("\n->kbd_test_timed_scan()");
		return NOT_FINE;
	}

	//Disable timer interrupts and unsubscribe them from the line
	if (timer_unsubscribe_int() == -NOT_FINE) {
		printf("\n->kbd_test_timed_scan()");
		return NOT_FINE;
	}

	return FINE;
}

int print_scan_code(unsigned long scanned_key_code, unsigned int *two_byte) {
	if (scanned_key_code & KEY_RELEASED) {
		if (scanned_key_code == TWO_BYTE_SCAN_CODE) {
			(*two_byte) = 1;
		} else {
			printf("\nBreakcode: ");
			if ((*two_byte) == 1) {
				printf("0x%02x ", TWO_BYTE_SCAN_CODE);
				(*two_byte) = 0;
			}
			printf("0x%02x", scanned_key_code);
			printf("\n");
			return 1;
		}
	} else {
		if (scanned_key_code == TWO_BYTE_SCAN_CODE) {
			(*two_byte) = 1;
		} else {
			printf("\n\nMakecode: ");
			if ((*two_byte) == 1) {
				printf("0x%02x ", TWO_BYTE_SCAN_CODE);
			}
			printf("0x%02x", scanned_key_code);
			return 1;
		}
	}
	return 0;
}
