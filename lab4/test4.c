#include "test4.h"

static unsigned char packet[3];

void print_packet() {
	//primeiro packet esta a dar overflow,X/Y tem de se testar e botao testar
	printf("\nB1=0x%X B2=0x%X B3=0x%X ", packet[FINE], packet[FINE + 1],
			packet[FINE + 2]);
	printf("LB=%d ", ((packet[FINE] & L_BUTTON) ? NOT_FINE : FINE));
	printf("MB=%d ", ((packet[FINE] & M_BUTTON) ? NOT_FINE : FINE));
	printf("RB=%d ", ((packet[FINE] & R_BUTTON) ? NOT_FINE : FINE));
	printf("XOV=%d ", ((packet[FINE] & X_OVFL) ? NOT_FINE : FINE));
	printf("YOV=%d ", ((packet[FINE] & Y_OVFL) ? NOT_FINE : FINE));

	short x, y;
	x = packet[FINE + 1];
	if (packet[FINE] & X_SIGN) {
		x |= 0xFF00;
	}
	y = packet[FINE + 2];
	if (packet[FINE] & Y_SIGN) {
		y |= 0xFF00;
	}
	printf("X=%d Y=%d ", x, y);
}

int mouse_test_packet(unsigned short cnt) {
//check if given counter value  is valid (positive)

	if (cnt <= FINE) {
		printf(
				"\n\nWARNING: an error occurred. Invalid given counter of packets\nStack Trace: \n->mouse_test_packet()");
		return NOT_FINE;
	}
	int irq_set_mouse;

	if ((irq_set_mouse = mouse_subscribe_int()) == -NOT_FINE) {
		printf("\n->mouse_test_packet()");
		return NOT_FINE;
	}
	if (enable_sending_data_packets() != FINE) {
		printf("\n->mouse_test_packet()");
		return NOT_FINE;
	}

	int ipc_configuration, driver_receive_error;
	message msg;
	unsigned int packetIterator = FINE, first = FINE;
	unsigned short originalCounter = cnt;

	while (cnt > 0) {
		if ((driver_receive_error = driver_receive(ANY, &msg,
				&ipc_configuration)) != 0) {
			printf(
					"\n\nWARNING: an error occurred. driver_receive failed with: %d \nStack Trace: \n->driver_receive() \n->mouse_test_packet()",
					driver_receive_error);
			continue;
		}
		if (is_ipc_notify(ipc_configuration)) {
			switch (_ENDPOINT_P(msg.m_source)) {
			case HARDWARE:
				if (msg.NOTIFY_ARG & irq_set_mouse) {
					/*
					 *
					 */
					packet[packetIterator] = getPacket();
					if ((packet[FINE] & BIT(3)) != FINE) { //Bit(3) of first packet equals one
						if ((packetIterator
								== (sizeof(packet) / sizeof(char) - 1))) { // packet array is full
							if (first == FINE) {
								first = NOT_FINE;
								packetIterator = FINE;
							} else {
								print_packet();
								cnt--;
								packetIterator = FINE;
							}
						} else
							packetIterator++;
					} else {
						packetIterator = FINE;
					}

					/*
					 *
					 */
				}
				break;
			default:
				break;
			}
		} else {
		}
	}
	printf("\n\n%d packets printed", originalCounter);

	if (disable_mouse_stream_mode() != FINE) {
		printf("\n->mouse_test_packet()");
		return NOT_FINE;
	}

	if (mouse_unsubscribe_int() == -NOT_FINE) {
		printf("\n->mouse_test_packet()");
		return NOT_FINE;
	}
	if (clear_output_buffer() != FINE) {
		printf("\n->mouse_test_remote()");
		return NOT_FINE;
	}
	printf("\n\nmouse_test_packet() function was correctly executed\n\n");
	return FINE;
}

int mouse_test_async(unsigned short idle_time) {
	if (idle_time < (FINE + 1)) {
		printf(
				"\n\nWARNING: an error occurred. Invalid given time \nStack Trace: \n->mouse_test_async()");
		return NOT_FINE;
	}

	int irq_set_timer, irq_set_mouse;

	if (timer_set_frequency(FINE, TIMER_DEFAULT_FREQ) != FINE) {
		printf("\n->mouse_test_async()");
		return NOT_FINE;
	}

//irq_set _> bit mask that will have the bits matching the hook_id_IO set to TIMER0_IRQ in interrupt mask
	if ((irq_set_timer = timer_subscribe_int()) == -NOT_FINE) {
		printf("\n->mouse_test_async()");
		return NOT_FINE;
	}

	if ((irq_set_mouse = mouse_subscribe_int()) == -NOT_FINE) {
		printf("\n->mouse_test_async()");
		return NOT_FINE;
	}
	if (enable_sending_data_packets() != FINE) {
		printf("\n->mouse_test_async()");
		return NOT_FINE;
	}

	int driver_receive_error, ipc_configuration;
	message msg;
	unsigned int enlapsed_time = 0;
	unsigned int packetIterator = FINE, first = FINE;
	idle_time = idle_time * TIMER_DEFAULT_FREQ; // will be a tick counter

//INTERRUPTION LOOP -> until the time has not yet passed.
	while (enlapsed_time < idle_time) {

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
						printf("\n%d   seconds",
								(enlapsed_time / TIMER_DEFAULT_FREQ));
					}
				}

				// hardware interrupt notification
				// msg.NOTIFY_ARG -> bitmask with the bits of the active interrupts sub, call the corresponding interrupt handler
				// irq_set_kbd is written in .NOTIFY_ARG anytime keyboard interrupt is generated
				if (msg.NOTIFY_ARG & irq_set_mouse) {
					enlapsed_time = 0;

					packet[packetIterator] = getPacket();
					if ((packet[FINE] & BIT(3)) != FINE) {//Bit(3) of first packet equals one
						if ((packetIterator
								== (sizeof(packet) / sizeof(char) - 1))) {// packet array is full
							if (first == FINE) {
								first = NOT_FINE;
								packetIterator = FINE;
							} else {
								print_packet();
								packetIterator = FINE;
							}
						} else
							packetIterator++;
					} else {
						packetIterator = FINE;
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
	if (disable_mouse_stream_mode() != FINE) {
		printf("\n->mouse_test_async()");
		return NOT_FINE;
	}

	if (mouse_unsubscribe_int() == -NOT_FINE) {
		printf("\n->mouse_test_async()");
		return NOT_FINE;
	}

	if ((irq_set_timer = timer_unsubscribe_int()) == -NOT_FINE) {
		printf("\n->mouse_test_async()");
		return NOT_FINE;
	}
	if (clear_output_buffer() != FINE) {
		printf("\n->mouse_test_remote()");
		return NOT_FINE;
	}
	printf("\n\nmouse_test_async() function was correctly executed\n\n");
	return FINE;
}

int mouse_test_remote(unsigned long period, unsigned short cnt) {

	if (cnt <= FINE || period <= FINE) {
		printf(
				"\n\nWARNING: an error occurred. Input arguments must be bigger than zero.  \nStack Trace: \n->mouse_test_remote()");
		return NOT_FINE;
	}

	if (enable_sending_data_remote() != FINE) {
		printf("\n->mouse_test_remote()");
		return NOT_FINE;
	}

	unsigned long byte;
	while (cnt > 0) {
		configure_mouse(READ_DATA);
		size_t i;
		for (i = 0; i < 3; i++) {
			if (sys_inb(OUT_BUF, &byte) != FINE) {
				printf(
						"\n\nWARNING: an error occurred. There was an error reading from the output buffer \nStack Trace: \n->sys_inb()\n->mouse_test_remote()");
				return NOT_FINE;
			}
			packet[i] = byte;
		}
		print_packet();
		cnt--;
		tickdelay(micros_to_ticks(period * 1000));
	}

	if (disable_sending_data_remote() != FINE) {
		printf("\n->mouse_test_remote()");
		return NOT_FINE;
	}

	if (enable_mouse_stream_mode() != FINE) {
		printf("\n->mouse_test_remote()");
		return NOT_FINE;
	}

	if (clear_output_buffer() != FINE) {
		printf("\n->mouse_test_remote()");
		return NOT_FINE;
	}
	printf("\n\nmouse_test_remote() function was correctly executed\n\n");
	return FINE;
}

int mouse_test_gesture(short length) { //check if given counter value  is valid (positive)
	if (length == FINE) {
		printf(
				"\n\nWARNING: an error occurred. Invalid given length\nStack Trace: \n->mouse_test_gesture()");
		return NOT_FINE;
	}

	int irq_set_mouse;

	if ((irq_set_mouse = mouse_subscribe_int()) == -NOT_FINE) {
		printf("\n->mouse_test_gesture()");
		return NOT_FINE;
	}
	if (enable_sending_data_packets() != FINE) {
		printf("\n->mouse_test_gesture()");
		return NOT_FINE;
	}

	int ipc_configuration, driver_receive_error;
	message msg;
	unsigned int packetIterator = FINE, first = FINE;
	unsigned short wrongPacket = NOT_FINE;

	while (wrongPacket) {
		if ((driver_receive_error = driver_receive(ANY, &msg,
				&ipc_configuration)) != 0) {
			printf(
					"\n\nWARNING: an error occurred. driver_receive failed with: %d \nStack Trace: \n->driver_receive() \n->mouse_test_gesture()",
					driver_receive_error);
			continue;
		}
		if (is_ipc_notify(ipc_configuration)) {
			switch (_ENDPOINT_P(msg.m_source)) {
			case HARDWARE:
				if (msg.NOTIFY_ARG & irq_set_mouse) {

					packet[packetIterator] = getPacket();
					if ((packet[FINE] & BIT(3)) != FINE) { //Bit(3) of first packet equals one
						if ((packetIterator
								== (sizeof(packet) / sizeof(char) - 1))) { // packet array is full
							if (first == FINE) {
								first = NOT_FINE;
								packetIterator = FINE;
							} else {
								print_packet();
								wrongPacket = verify_positive_slope(length,
										packet);
								packetIterator = FINE;
							}
						} else
							packetIterator++;
					} else {
						packetIterator = FINE;
					}
				}
				break;
			default:
				break;
			}
		} else {
		}
	}

	if (disable_mouse_stream_mode() != FINE) {
		printf("\n->mouse_test_gesture()");
		return NOT_FINE;
	}
	if (mouse_unsubscribe_int() == -NOT_FINE) {
		printf("\n->mouse_test_gesture()");
		return NOT_FINE;
	}
	if (clear_output_buffer() != FINE) {
		printf("\n->mouse_test_remote()");
		return NOT_FINE;
	}

	printf("\n\nmouse_test_gesture() function was correctly executed\n\n");
	return FINE;
}

