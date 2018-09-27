#include "test5.h"

int stop_by_esc(int irq_set_kbd) {
	unsigned long scanned_key_code;
	int ipc_configuration, driver_receive_error;
	message msg;

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

					if ((scanned_key_code = read_keyboard(1)) == FINE) {
						printf("\n->kbd_test_scan()");
						return NOT_FINE;
					}

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
	return FINE;
}

int video_test_init(unsigned short mode, unsigned short delay) {

	void *video_gr_adress = vg_init(mode);
	sleep(1000 * delay);

	if (timer_test_int((unsigned long) delay) != FINE) {
		printf("\n->video_test_init()");
		return (void *) NOT_FINE;
	}

	// sleep function can be alternatively
	//sleep(1000 * delay);

	sleep(1000 * delay);
	if (vg_exit() != FINE) {
		printf("\n->video_test_init()");
		return (void *) NOT_FINE;
	}

	return video_gr_adress;
}

int video_test_square(unsigned short x, unsigned short y, unsigned short size,
		unsigned long color) {

	int irq_set_kbd;
	if ((irq_set_kbd = kbd_subscribe_int()) == -NOT_FINE) {
		printf("\n->video_test_square()");
		return NOT_FINE;
	}

	if (vg_init(VBE_MODE) == NULL) {
		printf("\n->video_test_square()");
		return NOT_FINE;
	}
	if (vg_paint_square(x, y, size, color) != FINE) {
		printf("\n->video_test_square()");
	}

	video_dump_fb();
	if (stop_by_esc(irq_set_kbd) != FINE) {
		printf("\n->video_test_square()");
	}
	if (kbd_unsubscribe_int() == -NOT_FINE) {
		printf("\n->video_test_square()");
		return NOT_FINE;
	}

	if (vg_exit() != FINE) {
		return NOT_FINE;
		printf("\n->video_test_square()");
	}
	return FINE;
}

int video_test_line(unsigned short xi, unsigned short yi, unsigned short xf,
		unsigned short yf, unsigned long color) {

	int irq_set_kbd;

	if ((irq_set_kbd = kbd_subscribe_int()) == -NOT_FINE) {
		printf("\n->video_test_line()");
		return NOT_FINE;
	}

	if (vg_init(VBE_MODE) == NULL) {
		printf("\n->video_test_line()");
		return NOT_FINE;
	}

	if (vg_paint_line(xi, yi, xf, yf, color) != FINE) {
		printf("\n->video_test_line()");
	}
	video_dump_fb();
	if (stop_by_esc(irq_set_kbd) != FINE) {
		printf("\n->video_test_line()");
	}
	if (kbd_unsubscribe_int() == -NOT_FINE) {
		printf("\n->video_test_line()");
		return NOT_FINE;
	}

	if (vg_exit() != FINE) {
		return NOT_FINE;
		printf("\n->video_test_line()");
	}
	return FINE;
}

int test_xpm(char *xpm[], unsigned short xi, unsigned short yi) {

	int irq_set_kbd;

	if ((irq_set_kbd = kbd_subscribe_int()) == -NOT_FINE) {
		printf("\n->test_xpm()");
		return NOT_FINE;
	}

	if (vg_init(VBE_MODE) == NULL) {
		printf("\n->test_xpm()");
		return NOT_FINE;
	}

	if (vg_paint_xpm(xpm, xi, yi) != FINE) {
		printf("\n->test_xpm()");
	}
	video_dump_fb();

	if (stop_by_esc(irq_set_kbd) != FINE) {
		printf("\n->test_xpm()");
	}
	if (kbd_unsubscribe_int() == -NOT_FINE) {
		printf("\n->test_xpm()");
		return NOT_FINE;
	}

	if (vg_exit() != FINE) {
		return NOT_FINE;
		printf("\n->test_xpm()");
	}
	return FINE;
}

int test_circle(unsigned short x, unsigned short y, unsigned short radius,
		unsigned long color) {

	int irq_set_kbd;

	if ((irq_set_kbd = kbd_subscribe_int()) == -NOT_FINE) {
		printf("\n->video_test_line()");
		return NOT_FINE;
	}

	if (vg_init(VBE_MODE) == NULL) {
		printf("\n->video_test_line()");
		return NOT_FINE;
	}

	if (vg_paint_circle(x, y, radius, color) != FINE) {
		printf("\n->video_test_line()");
	}
	video_dump_fb();
	if (stop_by_esc(irq_set_kbd) != FINE) {
		printf("\n->video_test_line()");
	}
	if (kbd_unsubscribe_int() == -NOT_FINE) {
		printf("\n->video_test_line()");
		return NOT_FINE;
	}

	if (vg_exit() != FINE) {
		return NOT_FINE;
		printf("\n->video_test_line()");
	}
	return FINE;
}

int test_move(char *xpm[], unsigned short xi, unsigned short yi,
		unsigned short xf, unsigned short yf, short speed,
		unsigned short frame_rate) {

	int irq_set_kbd, irq_set_timer, ipc_configuration, driver_receive_error;
	struct Sprite *spriteObj;
	message msg;
	unsigned int counter = 0;
	unsigned long scanned_key_code;
	unsigned short x_position, y_position;

	if (frame_rate > 60) {
		printf(
				"\n\nWARNING: Invalid given frame_rate: %d. The max refresh rate supported is 60 fps: \nStack Trace: \n->test_move()",
				frame_rate);
		return NOT_FINE;
	}

	if (speed == 0) {
		printf("video_test_move::Invalid speed argument! It can't be 0 \n");
		printf(
				"\n\nWARNING: Invalid given speed: %d. The Speed cannot be zero: \nStack Trace: \n->test_move()",
				speed);
		return NOT_FINE;
	}

	if ((irq_set_kbd = kbd_subscribe_int()) == -NOT_FINE) {
		printf("\n->test_move()");
		return NOT_FINE;
	}

	if ((irq_set_timer = timer_subscribe_int()) == -NOT_FINE) {
		printf("\n->test_move()");
		return NOT_FINE;
	}

	if (vg_init(VBE_MODE) == NULL) {
		printf("\n->test_move()");
		return NOT_FINE;
	}

	spriteObj = Sprite_Create(xpm, xi, yi, xf, yf, speed, frame_rate);
	timer_set_frequency(0, frame_rate);
	x_position = xi;
	y_position = yi;
	counter = 0;
	ipc_configuration = 0;
	driver_receive_error = 0;

	while (scanned_key_code != ESC_KEY_CODE) {
		if ((driver_receive_error = driver_receive(ANY, &msg,
				&ipc_configuration)) != FINE) {
			printf(
					"\n\nWARNING: an error occurred. driver_receive failed with: %d \nStack Trace: \n->driver_receive() \n->timer_test_int()",
					driver_receive_error);
			continue;
		}
		if (is_ipc_notify(ipc_configuration)) {
			switch (_ENDPOINT_P(msg.m_source)) {
			case HARDWARE:

				if (msg.NOTIFY_ARG & irq_set_kbd) { //subscribed interrupt
					if ((scanned_key_code = read_keyboard(1)) == FINE) {
						printf("\n->kbd_test_scan()");
						return NOT_FINE;
					}
				}

				if (msg.NOTIFY_ARG & irq_set_timer) {
					Sprite_Move(spriteObj);
				}

				break;
			default:
				printf(
						"\n\nWARNING: an error occurred. _ENDPOINT_P(msg.m_source) \nStack Trace: \n->driver_receive() \n->timer_test_int()");
				break;
			}
		} else { // received a standard message, not a notification
			printf(
					"\n\nWARNING: an error occurred. Received Standard Notification. \nStack Trace: \n->driver_receive() \n->timer_test_int()");
		}
	}

	Sprite_Delete(spriteObj);

	timer_set_frequency(0, TIMER_DEFAULT_FREQ);

	if (kbd_unsubscribe_int() == -NOT_FINE) {
		printf("\n->test_move()");
		return NOT_FINE;
	}

	if (timer_unsubscribe_int() == -NOT_FINE) {
		printf("\n->test_move()");
		return NOT_FINE;
	}

	if (vg_exit() != FINE) {
		return NOT_FINE;
		printf("\n->test_move()");
	}
	return FINE;

}

int test_controller() {
	vbe_controller_info_t *vbe_info = malloc(sizeof(vbe_controller_info_t));

	void* info = (void*) vbe_get_controller_info(vbe_info);
	if (info == NULL) {
		printf("\nvbe_controller_info_t()");
		return NOT_FINE;
	}

	printf("Capabilities of the VBE:\n\n");

	printf("DAC:\n");
	if (!(vbe_info->Capabilities[0] & BIT(0)))
		printf("DAC has a non variable width. 6 bits per primary color.\n");
	else
		printf("DAC width can be switched to 8 bits per primary color.\n\n");

	printf("Controller of the VBE:\n");
	if (!(vbe_info->Capabilities[1] & BIT(1)))
		printf("Controller isn't compatible with VGA.\n");
	else
		printf("Controller is compatible with VGA.\n");

	printf("RAMDAC:\ñ");
	if (!(vbe_info->Capabilities[2] & BIT(2)))
		printf("RAMDAC with large blocks.\n");
	else
		printf("RAMDAC with normal operation.\n");

	printf("Video Modes Supported by the VGA:\n");
	//Gets the address
	short *phys_address = info
			+ (((int) (vbe_info->VideoModePtr) >> 12) & 0xF0000)
			+ ((int) vbe_info->VideoModePtr & 0x0FFFF);

	while (*phys_address != -NOT_FINE) {
		printf("0x%x\t", *phys_address);
		phys_address++;
	}

	printf("\nMemory Total is:  %dKB. \n", vbe_info->TotalMemory * 64);
	free(vbe_info);

	return FINE;

}
