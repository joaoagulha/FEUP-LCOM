#include "keyboard.h"

int kbd_hook_id; //Global variable, accessible by all functions
unsigned int sys_in_counter; //global variable, accessible by all function that stores the number of sys_inb calls made

void delay() {
	tickdelay(micros_to_ticks(TIME_TO_WAIT));
}

int kbd_subscribe_int() {

	kbd_hook_id = KBD_IRQ; //Para nao perder o id original
	//IRQ_KBD -> variable belonging to i8042 -> irq line of the keyboard
	//IRQ_REENABLE -> inform the GIH that it can give the EOI command
	//temp_kbd_hook -> will act as both input/output

	//sets policy of interruption request to enable the GIH to accept it, so it can give the EOI command
	//interrupt notification subscription.
	if (sys_irqsetpolicy(KBD_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE,
			&kbd_hook_id) != FINE) {
		printf(
				"\n\nWARNING: an error occurred. Failed to set subscription policy \nStack Trace: \n->sys_irqsetpolicy() \n->kbd_subscribe_int()");
		return -NOT_FINE;
	}
	//if sys_irqsetpolicy does not set the IRQ_REENABLE| IRQ_EXCLUSIVE policies in its interrupt subscription request, it means, in fact, that does
	//not enable interrupts automatically. So the keyboard requests the kernel to send the EOI in order to enable interrupts on the IRQ
	//line associated with the keyboard controller
	if (sys_irqenable(&kbd_hook_id) != FINE) {
		printf(
				"\n\nWARNING: an error occurred. Failed to enable interrupts \nStack Trace: \n->sys_irqenable() \n->kbd_subscribe_int()");
		return -NOT_FINE;
	}
	return BIT(KBD_IRQ); //retorna o bit da interrupt
}

int kbd_unsubscribe_int() {
	// Unsubscribes the keyboard interrupts.

	//sys_irqdisable() disables and unsubcribes previous enabled interrupts on the IRQ line associated with the specified kbd_hook_id
	if (sys_irqdisable(&kbd_hook_id) != FINE) {
		printf(
				"\n\nWARNING: an error occurred. Failed disabling interrupts (IRQ)\n.  \nStack Trace: \n->sys_irqdisable() \n->kbd_unsubscribe_int()");
		return -NOT_FINE;
	}

	//sys_irqrmpolicy() removes interruption request policy associated with kbd_hook_id
	if (sys_irqrmpolicy(&kbd_hook_id) != FINE) {
		printf(
				"\n\nWARNING: an error occurred. Failed removing previous subscription policy. \nStack Trace: \n->sys_irqrpolicy() \n->kbd_unsubscribe_int()");
		return -NOT_FINE;
	}
	return FINE;
}

unsigned long read_keyboard(unsigned short assembly) {
	if (assembly) {
		return read_keyboard_key_c();
	} else {
		return read_keyboard_key_assembly();
	}
}

unsigned long read_keyboard_key_c() {
	int kbc_counter_tries = 0;
	unsigned long scan_code_character, state;

	while (KBC_MAX_ATTEMPTS > kbc_counter_tries) {

		sys_in_counter++;
//Read comand byte from the KBD_CONTROL_REG, in order to get configuration
		if (sys_inb(KBD_STATUS_REG, &state) != FINE) {
			printf(
					"\n\nWARNING: an error occurred. Failed reading command byte from keyboard control register \nStack Trace: \n->sys_inb(KBD_STATUS_REG, &state) \n->read_keyboard_key_c()");
			return NOT_FINE;
		}

		//state & OUF_FLAG -> checks if output buffer has data available to be read
		//state & AUX_FLAG -> check if no input has come from the mouse
		if ((state & OUF_FLAG) == FINE || ((state & AUX_FLAG) != FINE)) {
//printf erro
			delay();
			kbc_counter_tries++;
			continue;
		}

		sys_in_counter++;
		//Read the scancode from the output buffer, in order to get configuration
		if (sys_inb(OUT_BUF, &scan_code_character) != FINE) {
			printf(
					"\n\nWARNING: an error occurred. Failed reading the output buffer \nStack Trace: \n->sys_inb(OUT_BUF, &scan_code_character) \n->read_keyboard_key_c()");
			return NOT_FINE;
		}
		if ((state & (TIMEOUT_ERR | PARITY_ERR)) != FINE) {
			printf(
					"\n\nWARNING: an error occurred. Invalid data introduced \nStack Trace: \n->read_keyboard_key_c()");
			return NOT_FINE;
		}
		return scan_code_character;
	}
	//delay the keyboard for the DELAY_US number of seconds

	return NOT_FINE;
}

unsigned long read_keyboard_key_assembly() {
	read_key_code_asm();

	return scanned_key_code_asm;
}

int write_to_keyboard_controller(unsigned long port, unsigned long commandByte) {
	unsigned long state;
	int kbc_counter_tries = 0;

	while (KBC_MAX_ATTEMPTS > kbc_counter_tries) {
		//Read comand byte from the mouse control register, in order to get configuration
		if (sys_inb(KBD_STATUS_REG, &state) != FINE) {
			printf(
					"\n\nWARNING: an error occurred. Failed to read command byte from mouse control register \nStack Trace: \n->sys_inb() \n->write_commmand_byte_to_controller()");
			return NOT_FINE;
		}

		//check if input buffer is full -> not write to it
		if ((state & IBF_FLAG) != FINE) {
			printf(
					"\n\nWARNING: an error occurred. Input buffer full\nStack Trace:\n->write_commmand_byte_to_controller()");
			delay();
			kbc_counter_tries++;
			continue;
		}
		//write given command byte to the given port
		if (sys_outb(port, commandByte) != FINE) {
			printf(
					"\n\nWARNING: an error occurred. Failed to write command byte %d to port %d \nStack Trace: \n->sys_outb() \n->write_commmand_byte_to_controller()",
					commandByte, port);
			return NOT_FINE;
		}
		return FINE;
	}
//printf erro
	return NOT_FINE;
}

int enable_keyboard_interrupts() {
	unsigned long command_byte;

	if (write_to_keyboard_controller(KBD_STATUS_REG, IN_BUF) != FINE) {
		printf("\n->enable_keyboard_interrupts()");
		return NOT_FINE;
	}
	if (sys_inb(OUT_BUF, &command_byte) != FINE) {
		printf(
				"\n\nWARNING: an error occurred. Failed to read status from port output buffer \nStack Trace: \n->sys_outb() \n->enable_keyboard_interrupts()");
		return NOT_FINE;
	}
	command_byte |= BIT(0);

	if (write_to_keyboard_controller(KBD_STATUS_REG, OUT_BUF) != FINE) {
		printf("\n->enable_keyboard_interrupts()");
		return NOT_FINE;
	}
	if (write_to_keyboard_controller(OUT_BUF, command_byte) != FINE) {
		printf("\n->enable_keyboard_interrupts()");
		return NOT_FINE;
	}
	return FINE;
}

unsigned int get_sys_in_counter() {
	return sys_in_counter;
}
