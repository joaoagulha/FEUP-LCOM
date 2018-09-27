#include "mouse.h"

int mouse_hook_id; //Global variable, accessible by all functions
unsigned int verify_counter;
unsigned int inclination;

void delay_time() {
	tickdelay(micros_to_ticks(TIME_TO_WAIT));
}

unsigned int clear_output_buffer() {
	unsigned long state, output_buffer_cleaner;

	if (sys_inb(MOUSE_STATUS_REG, &state) != FINE) {
		printf(
				"\n\nWARNING: an error occurred. Failed to read command byte from mouse control register \nStack Trace: \n->sys_inb() \n->clearOutputBuffer()");
		return NOT_FINE;
	}
	if (state & OUF_FLAG) {
		if (sys_inb(OUF_FLAG, &output_buffer_cleaner) != FINE) {
			printf(
					"\n\nWARNING: an error occurred. Failed to read command byte from mouse output register \nStack Trace: \n->sys_inb() \n->clearOutputBuffer()");
			return NOT_FINE;
		}
		return FINE;
	}
	return FINE;
}

int mouse_subscribe_int() {
	mouse_hook_id = MOUSE_IRQ; //Not to lose the original ID
//MOUSE_IRQ -> variable belonging to i8042 -> irq line of the mouse
//IRQ_REENABLE -> inform the GIH that it can give the EOI command
//mouse_hook_id -> will act as both input/output

//sets policy of interruption request to enable the GIH to accept it, so it can give the EOI command
//interrupt notification subscription.
	if (sys_irqsetpolicy(MOUSE_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE,
			&mouse_hook_id) != FINE) {
		printf(
				"\n\nWARNING: an error occurred. Failed to set subscription policy \nStack Trace: \n->sys_irqsetpolicy() \n->mouse_subscribe_int()");
		return -NOT_FINE;
	}
//if sys_irqsetpolicy does not set the IRQ_REENABLE| IRQ_EXCLUSIVE policies in its interrupt subscription request, it means, in fact, that does
//not enable interrupts automatically. So the keyboard requests the kernel to send the EOI in order to enable interrupts on the IRQ
//line associated with the keyboard controller

	if (sys_irqenable(&mouse_hook_id) != FINE) {
		printf(
				"\n\nWARNING: an error occurred. Failed to enable interrupts \nStack Trace: \n->sys_irqenable() \n->mouse_subscribe_int()");
		return -NOT_FINE;
	}
	return BIT(MOUSE_IRQ); //retorna o bit da interrupt
}

int mouse_unsubscribe_int() {

//sys_irqdisable() disables and unsubcribes previous enabled interrupts on the IRQ line associated with the specified kbd_hook_id
	if (sys_irqdisable(&mouse_hook_id) != FINE) {
		printf(
				"\n\nWARNING: an error occurred. Failed disabling interrupts\n \nStack Trace: \n->sys_irqdisable() \n->mouse_unsubscribe_int()");
		return -NOT_FINE;
	}

//sys_irqrmpolicy() removes interruption request policy associated with kbd_hook_id
	if (sys_irqrmpolicy(&mouse_hook_id) != FINE) {
		printf(
				"\n\nWARNING: an error occurred. Failed removing previous subscription policy \nStack Trace: \n->sys_irqrpolicy() \n->mouse_unsubscribe_int()");
		return -NOT_FINE;
	}
	return FINE;
}

int read_from_mouse_controller() {
	unsigned long scan_code_character, state;
	int kbc_counter_tries = 0;

	while (KBC_MAX_ATTEMPTS > kbc_counter_tries) {
		//Read comand byte from the mouse control register, in order to get configuration
		if (sys_inb(MOUSE_STATUS_REG, &state) != FINE) {
			printf(
					"\n\nWARNING: an error occurred. Failed to read command byte from mouse control register \nStack Trace: \n->sys_inb() \n->read_from_mouse_controller()");
			return NOT_FINE;
		}

		//state & OUF_FLAG -> checks if output buffer has data available to be read
		//state & AUX_FLAG -> check if no input has come from the mouse
		if (((state & OUF_FLAG) != FINE) && (((state & AUX_FLAG) == FINE))) {
			printf(
					"\n\nWARNING: an error occurred. Output buffer doesn't have information or input as come from the mouse\nStack Trace:\n->read_from_mouse_controller()");
			delay_time();
			kbc_counter_tries++;
			continue;
		}
		//Read the scan_code_character from the output buffer, in order to get configuration
		if (sys_inb(OUT_BUF, &scan_code_character) != FINE) {
			printf(
					"\n\nWARNING: an error occurred. Failed to read the information from the output buffer\nStack Trace: \n->sys_outb() \n->read_from_mouse_controller()");
			return NOT_FINE;
		}

		//state & (PARITY_ERR | TIMEOUT_ERR) -> checks if there is invalid data
		if ((state & (PARITY_ERR | TIMEOUT_ERR)) != FINE) {
			printf(
					"\n\nWARNING: an error occurred. Invalid given information \nStack Trace: \n->read_from_mouse_controller()");
			return NOT_FINE;
		}
		return scan_code_character;
	}

	printf(
			"\n\nWARNING: an error occurred. Exceeded max number of tries to read information from the mouse controller\nStack Trace: \n->read_from_mouse_controller()");
	return NOT_FINE;
}

unsigned int write_to_mouse_controller(unsigned long port,
		unsigned long commandByte) {
	unsigned long state;
	int kbc_counter_tries = 0;

	while (KBC_MAX_ATTEMPTS > kbc_counter_tries) {
		//Read comand byte from the mouse control register, in order to get configuration
		if (sys_inb(MOUSE_STATUS_REG, &state) != FINE) {
			printf(
					"\n\nWARNING: an error occurred. Failed to read command byte from mouse control register \nStack Trace: \n->sys_inb() \n->write_to_mouse_controller()");
			return NOT_FINE;
		}

		//state & IBF_FLAG -> check if input buffer is full -> not write to it
		if ((state & IBF_FLAG) != FINE) {
			printf(
					"\n\nWARNING: an error occurred. Input buffer full\nStack Trace:\n->write_to_mouse_controller()");
			delay_time();
			kbc_counter_tries++;
			continue;
		}
		//write given command byte to the given port
		if (sys_outb(port, commandByte) != FINE) {
			printf(
					"\n\nWARNING: an error occurred. Failed to write command byte %d, to the given port %d \nStack Trace: \n->sys_outb() \n->write_to_mouse_controller()",
					commandByte, port);
			return NOT_FINE;
		}
		return FINE;
	}

	printf(
			"\n\nWARNING: an error occurred. Exceeded max number of tries to write information from the mouse controller\nStack Trace: \n->write_to_mouse_controller()");
	return NOT_FINE;
}

unsigned int configure_mouse(unsigned long commandByte) {
	unsigned long acknowledgmentByte, status_reg;
	int acknowledgmentFlag = NOT_FINE;

	while (acknowledgmentFlag != FINE) {
		//write the WRITE_TO_MOUSE command to the mouse controller
		if (write_to_mouse_controller(MOUSE_STATUS_REG, WRITE_TO_MOUSE) != FINE) {
			printf("\n->configure_mouse()");
			return NOT_FINE;
		}

		if (sys_inb(MOUSE_STATUS_REG, &status_reg) != FINE) {
			continue;
		}

		if ((status_reg & IBF_FLAG) == 0) {
			//write given command byte to the output buffer
			if (write_to_mouse_controller(OUT_BUF, commandByte) != FINE) {
				printf("\n->configure_mouse()");
				return NOT_FINE;
			}
			//Read acknowledgment byte from the mouse output buffer, in order to get configuration
			if (sys_inb(OUT_BUF, &acknowledgmentByte) != FINE) {
				printf(
						"\n\nWARNING: an error occurred. Failed to read acknowledgment byte from the output register \nStack Trace: \n->sys_inb() \n->configure_mouse()");
				return NOT_FINE;
			}

			//Test error in acknowledgment byte
			if (acknowledgmentByte == ERROR) {
				printf(
						"\n\nWARNING: an error occurred. Mouse controller reports error through acknowledgment byte \nStack Trace: \n->configure_mouse()");
				return NOT_FINE;
			}
			if (acknowledgmentByte == NACK) {
				acknowledgmentByte = RESEND;
			}
			if (acknowledgmentByte == ACK) {
				acknowledgmentFlag = FINE;
			}
		}

		//Delay to give time the OS to update information
		tickdelay(micros_to_ticks(TIME_TO_WAIT));
	}
	return FINE;
}

unsigned int enable_mouse() {

	//writes MOUSE_STATUS_REG the MOUSE_ENABLE command
	if (write_to_mouse_controller(MOUSE_STATUS_REG, MOUSE_ENABLE) != FINE) {
		printf("\n->enable_mouse()");
		return NOT_FINE;
	}
	return FINE;
}

unsigned int disable_mouse_stream_mode() {

	//configures the mouse to disable data reporting
	if (configure_mouse(DIS_DATA_REPORT) != FINE) {
		printf("\n->disable_mouse_stream_mode()");
		return NOT_FINE;
	}
	return FINE;
}

unsigned int enable_data_reporting() {

	//configures the mouse to enable data reporting
	if (configure_mouse(ENA_DATA_REPORT) != FINE) {
		printf("\n->enable_data_reporting()");
		return NOT_FINE;
	}
	return FINE;
}

unsigned int enable_mouse_stream_mode() {

	//configures the mouse to enable stream mode
	if (configure_mouse(SET_STREAM_MODE) != FINE) {
		printf("\n->enable_mouse_stream_mode()");
		return NOT_FINE;
	}
	return FINE;
}

unsigned int enable_sending_data_packets() {

	//enables the mouse to receive information
	if (enable_mouse() != FINE) {
		printf("\n->enable_sending_data_packets()");
		return NOT_FINE;
	}
	//disables the mouse stream mode
	if (disable_mouse_stream_mode() != FINE) {
		printf("\n->enable_sending_data_packets()");
		return NOT_FINE;
	}
	//enables mouse data reporting
	if (enable_data_reporting() != FINE) {
		printf("\n->enable_sending_data_packets()");
		return NOT_FINE;
	}
	//enable the mouse stream mode
	if (enable_mouse_stream_mode() != FINE) {
		printf("\n->enable_sending_data_packets()");
		return NOT_FINE;
	}
	return FINE;
}

unsigned int disable_mouse_interrupts() {
	unsigned long command_byte;

	//write to the MOUSE_STATUS_REG the IN_BUF command
	if (write_to_mouse_controller(MOUSE_STATUS_REG, IN_BUF) != FINE) {
		printf("\n->disable_mouse_interrupts()");
		return NOT_FINE;
	}

	//read from the output buffer the command byte
	if (sys_inb(OUT_BUF, &command_byte) != FINE) {
		printf(
				"\n\nWARNING: an error occurred. Failed to read status from port output buffer \nStack Trace: \n->sys_outb() \n->disable_mouse_interrupts()");
		return NOT_FINE;
	}

	//change received command byte to ensure the mouse interrupts are disabled
	command_byte ^= BIT(1);
	//command_byte = command_byte & 0xFD;

	//write to the MOUSE_STATUS_REG the OUT_BUF command
	if (write_to_mouse_controller(MOUSE_STATUS_REG, OUT_BUF) != FINE) {
		printf("\n->disable_mouse_interrupts()");
		return NOT_FINE;
	}

	//write to the OUT_BUF the command byte
	if (write_to_mouse_controller(OUT_BUF, command_byte) != FINE) {
		printf("\n->disable_mouse_interrupts()");
		return NOT_FINE;
	}
	return FINE;
}

unsigned int enable_mouse_remote_mode() {
//enables the mouse remote mode

	//configure the mouse to remote mode
	if (configure_mouse(SET_REMOTE_MODE) != FINE) {
		printf("\n->enable_mouse_remote_mode()");
		return NOT_FINE;
	}
	return FINE;
}

unsigned int enable_sending_data_remote() {

	//disables the mouse stream mode
	if (disable_mouse_stream_mode() != FINE) {
		printf("\n->enable_sending_data_remote()");
		return NOT_FINE;
	}

	//disables the mouse interrupts
	if (disable_mouse_interrupts() != FINE) {
		printf("\n->enable_sending_data_remote()");
		return NOT_FINE;
	}

	//enable mouse stream mode
	if (enable_mouse_remote_mode() != FINE) {
		printf("\n->enable_sending_data_remote()");
		return NOT_FINE;
	}
	return FINE;
}

unsigned int disable_sending_data_remote() {
	unsigned long command_byte;

	//write to the MOUSE_STATUS_REG the OUT_BUF command
	if (write_to_mouse_controller(MOUSE_STATUS_REG, OUT_BUF) != FINE) {
		printf("\n->disable_sending_data_remote()");
		return NOT_FINE;
	}

	//read from the OUT_BUF the command_byte
	if (sys_inb(OUT_BUF, &command_byte) != FINE) {
		printf(
				"\n\nWARNING: an error occurred. Failed to read the command byte from port output buffer \nStack Trace: \n->sys_outb() \n->disable_mouse_interrupts()");
		return NOT_FINE;
	}
	command_byte |= BIT(1);
//	command_byte = command_byte & 0x02;

	//write to the OUT_BUF the command_byte
	if (write_to_mouse_controller(OUT_BUF, command_byte) != FINE) {
		printf("\n->disable_mouse_interrupts()");
		return NOT_FINE;
	}
	return FINE;
}

unsigned int verify_positive_slope(short length, unsigned char packet[3]) {
	short x, y;
	x = packet[FINE + 1];
	if (packet[FINE] & X_SIGN) {
		x |= 0xFF00;
	}
	y = packet[FINE + 2];
	if (packet[FINE] & Y_SIGN) {
		y |= 0xFF00;
	}
	unsigned int right_button = packet[FINE] & R_BUTTON ? NOT_FINE : FINE;
	if (length < FINE && x < FINE && y < FINE
			&& right_button == 1 /*&& inclination == NOT_FINE*/) {
		verify_counter++;
		if (verify_counter >= (-length)) {
			return FINE;
		} else {
			return NOT_FINE;
		}
	}

	if (length > FINE && x > FINE && y > FINE
			&& right_button == 1 /*&& inclination == FINE*/) {
		verify_counter++;
		if (verify_counter >= length) {
			return FINE;
		} else {
			return NOT_FINE;
		}
	}

	verify_counter = 0;
	return NOT_FINE;
}

int getPacket() {
	return read_from_mouse_controller();
}

