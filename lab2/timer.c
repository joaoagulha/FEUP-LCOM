#include <minix/drivers.h>
#include "timer.h"

int hook_id_IO;
unsigned int interrupt_counter;

int timer_set_frequency(unsigned char timer, unsigned long freq) {

	unsigned char previous_timer_conf, read_back_command, timer_selector, LSB,
			MSB;

	//check if given timer is valid
	if (!(timer == 0 || timer == 1 || timer == 2)) {
		printf(
				"\n\nWARNING: an error occurred. Invalid given Timer: %d \nStack Trace: \n->timer_set_frequency()",
				timer);
		return 1;
	}

	//check if provided frequency is valid
	if (freq <= FINE) {
		printf(
				"\n\nWARNING: an error occurred. Invalid given frequency: %lu. \nFrequency cannot be negative \nStack Trace: \n->timer_set_frequency()",
				freq);
		return 1;
	}

	//Bits 0,1,2,3 are not to be changed, they are to be preserved, the same as before
	previous_timer_conf = 0;

	//returns Timer 0 configuration
	if (timer_get_conf(timer, &previous_timer_conf) != FINE) {
		printf(
				"\n\nWARNING: an error occurred. Arguments -> timer: %lu, frequency: %lu \nStack Trace: \n->timer_get_conf() \n->timer_set_frequency()",
				timer, freq);
		return 1;
	}

	read_back_command = 0;

	//Since it is not to change the 4 least significant bits, we have to ensure that they represent BCD and mode 3
	//Reserved Bit -> BIT 0 is 0 (reserved)
	//Programmed Mode -> 3 -> TIMER_SQR_WAVE -> bit 1,2,3 -> 1,1,X
	//Access Type -> LSB followed by MSB -> TIMER_LSB_MSB -> Bit 4 e 5 -> 11
	read_back_command &= 0x0F;	//To maintain the timer's BCD and Mode digits
	read_back_command = (previous_timer_conf & BIT(0)) | TIMER_SQR_WAVE
			| TIMER_LSB_MSB;

	switch (timer) {
	case 0:
		//timer0 -> bit 6,7 -> 0,0
		read_back_command |= TIMER_SEL0;
		break;
	case 1:
		//timer1 -> bit 6,7 -> 0,1
		read_back_command |= TIMER_SEL1;
		break;
	case 2:
		//timer2 -> bit 6,7 -> 1,0
		read_back_command |= TIMER_SEL2;
		break;
	}

	//need to send control word (read_back_command) to the timer controller register before writing the timer passed as argument
	if (sys_outb(TIMER_CTRL, read_back_command) != FINE) {
		printf(
				"\n\nWARNING: an error occurred. \nStack Trace: \n->sys_outb() \n->timer_set_prequency()");
		return 1;
	}

	//load timer with the value of the new frequency. it is to generate the frequency matching to the intended rate
	//the higher the freq the faster the time is counted
	timer_selector = TIMER_0 + timer;

	//setFrequency: load timer with the value of timerNewFreq. it is to generate the frequency matching to the intended rate
	//the higher the freq the faster the time is counted
	freq = TIMER_FREQ / freq;
	LSB = (char) freq & 0xFF;
	MSB = (char) (freq >> 8); //Makes the shift of the MSB to the final 8 bits

	//sends the LSB
	if (sys_outb(timer_selector, LSB) != FINE) {
		printf(
				"\n\nWARNING: an error occurred. \nStack Trace: \n->sys_outb() \n->timer_set_frequency()");
		return 1;
	}

	//sends the MSB
	if (sys_outb(timer_selector, MSB) != FINE) {
		printf(
				"\n\nWARNING: an error occurred. \nStack Trace: \n->sys_outb() \n->timer_set_frequency()");

		return 1;
	}

	return 0;
}

int timer_subscribe_int(void) {
	//Assigns and enables Timer 0 interrupts. Returns the bit order in an interrupt mask;

	hook_id_IO = TIMER0_IRQ;

	//TIMER0_IRQ -> variable belonging to i8254 -> irq line of the device driver (input)
	//IRQ_REENABLE -> inform the GIH that it can give the EOI command
	//hook_id_IO -> will act as both input/output

	//sets policy of interruption request to enable the GIH to accept it, so it can give the EOI command
	//interrupt notification subscription.
	if (sys_irqsetpolicy(TIMER0_IRQ, IRQ_REENABLE, &hook_id_IO) != FINE) { //output the EOI command to the PIC, set timer IRQ policy and latch hook_id_IO to it, used to subscribe a notification on every interrupt in the input TIMER0_IRQ
		printf(
				"\n\nWARNING: an error occurred. \nStack Trace: \n->sys_irqsetpolicy() \n->timer_subscribe_int()");
		return -1;
	}

	//if sys_irqsetpolicy does not set the IRQ_REENABLE policy in its interrupt subscription request, it means, in fact, that does
	//not enable interrupts automatically. So the DD requests the kernel to send the EOI in order to enable interrupts on the IRQ
	//line associated with the specified hook_id_IO
	if (sys_irqenable(&hook_id_IO) != FINE) {
		printf(
				"\n\nWARNING: an error occurred. \nStack Trace: \n->sys_irqenable() \n->timer_subscribe_int()");
		return -1;
	}

	return BIT(TIMER0_IRQ);
}

int timer_unsubscribe_int() {
	// Unsubscribes the Timer 0 interrupts.

	//sys_irqdisable() disables and unsubcribes previous enabled interrupts on the IRQ line associated with the specified hook_id_IO
	if (sys_irqdisable(&hook_id_IO) != FINE) {
		printf(
				"\n\nWARNING: an error occurred. Failed disabling interrupts (IRQ)\n.  \nStack Trace: \n->sys_irqdisable()");
		return 1;
	}

	//sys_irqrmpolicy() removes interruption request policy associated with Timer0
	if (sys_irqrmpolicy(&hook_id_IO) != FINE) {
		printf(
				"\n\nWARNING: an error occurred. Failed removing previous subscription policy. \nStack Trace: \n->sys_irqrpolicy()");
		return 1;
	}
	return 0;
}

void timer_int_handler() {
	//increments global variable interrupt_counter, on every timer interrupt (Timer 0)
	interrupt_counter++;
}

int timer_get_conf(unsigned char timer, unsigned char *st) {

	unsigned char readBackCommand, timer_selector;

	//check if given timer is valid, and if not return 1
	if (!(timer == 0 || timer == 1 || timer == 2)) {
		printf(
				"\n\nWARNING: an error occurred. Invalid given Timer: %d \nStack Trace: \n->timer_get_conf()",
				timer);
		return 1;
	}

	//bit 0 -> reserved -> 0
	//bit 1/2/3 depends on the timer0,timer1,timer2 respectively
	//bit 4/5 -> status/count -> active low, occurs when its zero, so bit count=1 e status=0,
	//bit 6/7 -> 11 -> rea-back command
	//2 MSBs =1
	readBackCommand = TIMER_RB_SEL(
			timer) | TIMER_RB_COUNT_ | TIMER_RB_CMD; // command for Control Register

	//send the read-back command to the timer controller register 0x43h before accessing the timer passed as function a argument.
	if (sys_outb(TIMER_CTRL, readBackCommand) != OK) {
		printf(
				"\n\nWARNING: an error occurred. \nStack Trace: \n->sys_outb() \n->timer_get_conf()");
		return 1;
	}

	//select timer register depending on the chosen timer
	//TIMER_0 = 0x40 (+) timer = 0x40 -> Timer0
	//							 0x41 -> Timer1
	//					 		 0x42 -> Timer2
	timer_selector = TIMER_0 + timer;

	//get the timer configuration.
	//extend unsigned char *st to unsigned long to be accepted as a valid argument
	if (sys_inb(timer_selector, (unsigned long*) st) != OK) {
		printf(
				"\n\nWARNING: an error occurred. \nStack Trace: \n->sys_inb() \n->timer_get_conf()");
		return 1;
	}

	return 0;
}

int timer_display_conf(unsigned char conf) {

	printf("\n\n----------------------------");
	printf("\n\nConfiguration of the Timer: 0x%x\n", conf);
	printf("\n\n----------------------------");

	// OUTPUT -> BIT 7
	printf("\nOutPut -> %s", (conf & TIMER_SEL2) ? "YES" : "NO");

	// NULL COUNT  _> BIT 6
	printf("\nNull Count -> %s", (conf & TIMER_SEL1) ? "YES" : "NO");

	// ACCESS TYPE -> BIT 4 / BIT 5
	printf("\nAccess Type -> ");
	switch (conf & TIMER_LSB_MSB) {
	case TIMER_MSB:
		printf("MSB");
		break;
	case TIMER_LSB:
		printf("LSB");
		break;
	case TIMER_LSB_MSB:
		printf("LSB followed by MSB");
		break;
	default:
		printf("Not specified");
		break;
	}

	// OPERATION MODE -> BIT 3 / BIT 2 / BIT 1
	printf("\nProgrammed Mode ");
	switch (conf & MASK_PROGRAMMED_MODE) {
	case 0:
		printf("0 -> Interrupt on Terminal Count");
		break;
	case TIMER_HAR_RETR: /* Get a request message. */

		printf("1 -> Hardware Retriggerable One-Shot");
		break;
	case TIMER_RATE_GEN:
		printf("2 -> Rate Generator");
		break;
	case TIMER_SQR_WAVE:
		printf("3 -> Square Wave Mode");
		break;
	case TIMER_SOFT_TRIG:
		printf("4 -> Software Triggered Strobe");
		break;
	case TIMER_HARD_TRIG:
		printf("5 -> Hardware Triggered Strobe (Retriggerable)");
		break;
	default:
		printf(
				"\n\nWARNING: an error occurred. Invalid Programmed Mode. \nStack Trace: \n->timer_display_conf()");
		return 1;
	}

	// (Counting mode) -> BIT 0
	printf("\nCounting mode -> %s\n", (conf & BIT(0)) ? "BCD" : "Binary");

	return 0;
}

int timer_test_time_base(unsigned long freq) {
	if (timer_set_frequency(0, freq) != FINE) {
		printf("\n->timer_test_time_base()");
		return 1;
	}

	printf("\n\ntimer_test_time_base() function was correctly executed\n\n");
	return 0;
}

int timer_test_int(unsigned long time) {
	//Prints one message per second, during a time interval with duration equal to time
	int irq_set, ipc_configuration = 0, driver_receive_error = 0;
	message msg;

	if (time < (FINE + 1)) {
		printf(
				"\n\nWARNING: an error occurred. Expecified time is invalid. \nStack Trace: \n->timer_test_int()");
		return 1;
	}

	//begging to work at a normal frequency
	if (timer_test_time_base(TIMER_DEFAULT_FREQ) != FINE) {
		printf("\n->timer_test_int()");
		return 1;
	}

	irq_set = 0;

	//irq_set _> bit mask that will have the bits matching the hook_id_IO set to 1 in interrupt mask
	if ((irq_set = timer_subscribe_int()) < FINE) { //subscribe timer0
		printf("\n->timer_test_int() -> Error subscribing");
		return 1;
	}

	interrupt_counter = 0;
	time = time * TIMER_DEFAULT_FREQ; // will be a tick counter

	printf("\n");
	//INTERRUPTION LOOP -> until the time has not yet passed.
	while (interrupt_counter < time) {

		//driver_receive_error will have the received message sent by the DD (kernel or some other processes), upon making a request
		//ANY -> sender message (any sender)
		//&msg -> where the message will be put. used to avoid having to make a call to the driver_receive function several times
		//&ipc_configuration -> where the ipc configuration, which contains the message configuration (standard message or a kernel notification, will be put
		if ((driver_receive_error = driver_receive(ANY, &msg,
				&ipc_configuration)) != FINE) {
			printf(
					"\n\nWARNING: an error occurred. driver_receive failed with: %d \nStack Trace: \n->driver_receive() \n->timer_test_int()",
					driver_receive_error);
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
				// irq_set is written in .NOTIFY_ARG anytime TIMER_0 interrupt is generated
				if (msg.NOTIFY_ARG & irq_set) {

					//calls the interrupt handler
					timer_int_handler();
					if ((interrupt_counter != FINE)
							&& (interrupt_counter % TIMER_DEFAULT_FREQ == FINE)) {

						//1 second -> 60hz default frequency, 60 ticks -> which means the message will be printed at each 60 ticks, and so it will be printed at each one second passed
						printf("%d   seconds\n",
								interrupt_counter / TIMER_DEFAULT_FREQ);
					}
				}
				break;
			default:
				printf(
						"\n\nWARNING: an error occurred. _ENDPOINT_P(msg.m_source) \nStack Trace: \n->driver_receive() \n->timer_test_int()");
				break;
			}
		} else { /* received a standard message, not a notification */
			printf(
					"\n\nWARNING: an error occurred. Received Standard Notification. \nStack Trace: \n->driver_receive() \n->timer_test_int()");
		}
	}

	interrupt_counter = 0;

	//Disable interrupts and unsubscribe them from the line
	if (timer_unsubscribe_int() != FINE) {
		printf("\n->timer_test_int()");
		return 1;
	}

	if (timer_test_time_base(TIMER_DEFAULT_FREQ) != FINE) {	// reset default frequency
		printf("\n->timer_test_int()");
		return 1;
	}
	return 0;
}

int timer_test_config(unsigned char timer) {

	unsigned char config;

	//check if given timer is valid, and if not print message and return 1
	if (!(timer == 0 || timer == 1 || timer == 2)) {
		printf("\n\nInvalid given Timer: %d\n", timer);
		printf(
				"\n\nWARNING: an error occurred. \nStack Trace: \n->timer_test_config()");
		return 1;
	}
	config = 0;

	//check if there was any error, and if not print message and return 1
	if (timer_get_conf(timer, &config) != OK) {
		printf("\n->timer_test_config()");
		return 1;
	}

	//check if there was any error, and if not print message and return 1
	if (timer_display_conf(config) != OK) {
		printf("\n->timer_test_config()");
		return 1;
	}

	printf("\n\ntimer_test_config() function was correctly executed\n\n");
	return 0;
}
