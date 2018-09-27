#ifndef _UTILITIES_H_
#define _UTILITIES_H_

/** @defgroup i8254 i8254
 * @{
 *
 * Constants for programming the i8254 Timer. Needs to be completed.
 */
#define FINE 0
#define NOT_FINE 1

//Ports
//Commands
#define SET_VBE_MODE 0xF0

#define VBE_CALL_CMD 0x4F00
#define VBE_SET_GRAPHIC_MODE 0x0002

#define LINEAR_FRAMEBUFFER BIT(14)

#define VIDEO_CARD 0x10
#define PC_CONFIGURATION 0x11
#define MEMORY_CONFIGURATION 0x12

#define VBE_SET_TEXT_MODE 0x00
#define TEXT_MODE 0x03

//PS2 Commands
#define MODEINFO_SIZE	256

//VBE
#define VBE_MODE 0x105

/**@}*/

#endif /* _LCOM_I8254_H */
