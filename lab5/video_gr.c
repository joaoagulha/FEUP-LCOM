#include "video_gr.h"

/* Constants for VBE 0x105 mode */

/* The physical address may vary from VM to VM.
 * At one time it was 0xD0000000
 *  #define VRAM_PHYS_ADDR    0xD0000000
 * Currently on lab B107 is 0xF0000000
 * Better run my version of lab5 as follows:
 *     service run `pwd`/lab5 -args "mode 0x105"
 */
#define VRAM_PHYS_ADDR	0xF0000000
#define H_RES             1024
#define V_RES		  768
#define BITS_PER_PIXEL	  8

/* Private global variables */

static phys_bytes phys_mem;
static char *video_mem; /* Process (virtual) address to which VRAM is mapped */

static unsigned h_res; /* Horizontal screen resolution in pixels */
static unsigned v_res; /* Vertical screen resolution in pixels */
static unsigned bits_per_pixel; /* Number of VRAM bits per pixel */

int vg_exit() {
	struct reg86u reg86;

	reg86.u.b.intno = VIDEO_CARD; /* BIOS video services */
	reg86.u.b.ah = VBE_SET_TEXT_MODE; /* Set Video Mode function */
	reg86.u.b.al = TEXT_MODE; /* 80x25 text mode*/

	if (sys_int86(&reg86) != FINE) {
		printf(
				"\n\nWARNING: an error occurred. tvg_exit(): sys_int86() failed\nStack Trace: \n->vg_exit()");
		return NOT_FINE;
	}
	return FINE;
}

void *vg_init(unsigned short mode) {
//	SET GRAPHIC MODE
	struct reg86u reg86;
	reg86.u.w.ax = VBE_CALL_CMD | VBE_SET_GRAPHIC_MODE; // VBE call, function 02 -- set VBE mode
	reg86.u.w.bx = LINEAR_FRAMEBUFFER | mode; // set bit 14: linear framebuffer
	reg86.u.b.intno = VIDEO_CARD;

	if (sys_int86(&reg86) != FINE) {
		printf(
				"\n\nWARNING: an error occurred. set_vbe_mode: sys_int86()\nStack Trace: \n->vg_init()");
		return NULL;
	}

	if (reg86.u.b.ah != FINE) {
		printf(
				"\n\nWARNING: an error occurred. set_vbe_mode failed\nStack Trace: \n->vg_init()");
		return NULL;
	}

	//GET VBE MODE INFO
	vbe_mode_info_t vbe_mode;
	if (vbe_get_mode_info(mode, &vbe_mode) != FINE) {
		printf("\nvg_init");
	}

	v_res = vbe_mode.YResolution;
	h_res = vbe_mode.XResolution;
	bits_per_pixel = vbe_mode.BitsPerPixel;

	struct mem_range mr;
	unsigned int vram_size = h_res * v_res * (bits_per_pixel / 8); //VRAM's size, but you can use the frame-buffer size, instead

	// Allow memory mapping
	phys_mem = (phys_bytes) vbe_mode.PhysBasePtr;
	mr.mr_base = (phys_bytes) vbe_mode.PhysBasePtr; //depois trocar por generico (endereco base)
	mr.mr_limit = mr.mr_base + vram_size;

	int r;
	if ((r = sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr)) != FINE) {
		panic("sys_privctl (ADD_MEM) failed: %d\n", r);
	}

	//Map memory
	if ((video_mem = vm_map_phys(SELF, (void *) mr.mr_base, vram_size))
			== MAP_FAILED) {
		panic("couldn't map video memory");
	}

	//return memory position
	return video_mem;
//	return (void*) vbe_mode.PhysBasePtr;
}

int vg_set_pixel(unsigned short x, unsigned short y, unsigned long color) {
	//check the x and y argument values
	if ((x < 0 || x >= H_RES) || y < 0 || y >= V_RES) {
		printf(
				"\n\nWARNING: an error occurred. Invalid input values\nStack Trace: \n->vg_set_pixel()");
		return NOT_FINE;
	}

	//write to the position (x,y) the argument color
	*(video_mem + (x + y * h_res)) = (char) color;
	return FINE;
}

void convertCoordinates(unsigned short *x, unsigned short *y,
		unsigned short size) {
	if (size % 2 == FINE) {
		*x = 512 + *x - (size / 2);
		*y = 384 + *y - (size / 2);
	} else {
		*x = 512 + *x - ((size - 1) / 2);
		*y = 384 + *y - ((size - 1) / 2);
	}
}

int vg_paint_square(unsigned short x, unsigned short y, unsigned short size,
		unsigned long color) {
	//check the x and y argument values
	if (x
			< 0|| x >= H_RES || y < 0 || y >= V_RES || size <= FINE || size > V_RES) {
		printf(
				"\n\nWARNING: an error occurred. Invalid input values\nStack Trace: \n->paint_square()");
		return NOT_FINE;
	}
	convertCoordinates(&x, &y, size);

	int x_position, y_position;

	for (x_position = 0; x_position < size; x_position++) {
		for (y_position = 0; y_position < size; y_position++) {
			if (vg_set_pixel(x + x_position, y + y_position, color) != FINE) {
				printf("\n->paint_square()");
				return NOT_FINE;
			}
		}
	}

	return video_dump_fb();
}

int vg_paint_line(unsigned short xi, unsigned short yi, unsigned short xf,
		unsigned short yf, unsigned long color) {
	//algorithm based on https://en.wikipedia.org/wiki/Line_drawing_algorithm
	if (xi < FINE || xi >= H_RES || xf < FINE || xf >= H_RES || yi < FINE
			|| yf < FINE || yi >= V_RES || yf >= V_RES) {
		printf(
				"\n\nWARNING: an error occurred. Invalid input values\nStack Trace: \n->paint_line()");
		return NOT_FINE;
	}

	int dx = xf - xi, iterations, i;
	int dy = yf - yi;
	float xin, yin;
	float x, y;

	unsigned short x_position = xi, y_position;

	if (dx == 0 && dy == 0) {
		vg_set_pixel(xi, yi, color);
		return FINE;
	}

	if (dx == 0) {
		xin = 0;
		yin = iterations / dy; // == 1 || -1
		iterations = abs(dy);
	} else {

		if (abs(dy) > abs(dx)) {
			iterations = abs(dy);
			xin = dx / (float) iterations;
			yin = iterations / dy; // == 1 || -1
		} else {
			iterations = abs(dx);
			xin = iterations / dx; // == 1 || -1
			yin = dy / (float) iterations;
		}
	}

	x = xi;
	y = yi;

	for (i = 0; i < iterations; i++) {
		x += xin;
		y += yin;
		vg_set_pixel((long) x, (long) y, color);
	}

	return video_dump_fb();
}

int vg_paint_xpm(char *xpm[], unsigned short xi, unsigned short yi) {

	int image_height, image_width;
	char color;

	video_dump_fb();

	int x_position = FINE, y_position = FINE;
	char * pix_map = (char *) read_xpm(xpm, &image_width, &image_height);
	if (pix_map == NULL) {
		printf(
				"\n\nWARNING: an error occurred. Failed reading pixel map\nStack Trace: \n->paint_xpm()");
		free(pix_map);
		return NOT_FINE;
	}

	for (x_position = x_position; x_position < image_width; x_position++) {
		y_position = FINE;
		for (y_position = y_position; y_position < image_height; y_position++) {
			color = (*(pix_map + x_position + y_position * image_width));
			if (vg_set_pixel(xi + x_position, yi + y_position, color) != FINE) {
				printf("\n->paint_xpm()");
				free(pix_map);
				return NOT_FINE;
			}
		}
	}
	free(pix_map);
	return FINE;
}

int vg_drawSprite(struct Sprite *s) {

	int x;
	int y;
	for (x = 0; x < s->width; ++x) {
		for (y = 0; y < s->height; ++y) {
			char color = (*(s->map + x + y * s->width));

			if (vg_set_pixel(x + s->x_position, y + s->yi, color)) {
				return NOT_FINE;
			}
		}
	}
	return FINE;
}

int vg_clearSprite(struct Sprite *s) {
	int x;
	int y;
	for (x = -1; x < s->width + 1; x++) {
		for (y = -1; y < s->height + 1; y++) {
			char color = 0;
			if (vg_set_pixel(x + s->x_position - 1, y + s->yi - 1, color)) {
				return NOT_FINE;
			}
		}
	}
	return FINE;

}

int vg_clearScreen(unsigned short color) {
	int x = 0;
	int y = 0;

	for (x = 0; x < H_RES; ++x) {
		for (y = 0; y < V_RES; ++y) {
			if (vg_set_pixel(x, y, (char) color)) {
				return NOT_FINE;
			}
		}
	}
	return FINE;
}

char* vg_xpmToPixmap(char *xpm[], int *wd, int *ht) {
	return (char *) read_xpm(xpm, wd, ht);
}

int vg_paint_circle(unsigned short x_position, unsigned short y_position,
		unsigned short radius, unsigned long color) {
	if (x_position < FINE || x_position >= H_RES || y_position < H_RES
			|| y_position >= V_RES) {
		printf(
				"\n\nWARNING: an error occurred. Invalid input values\nStack Trace: \n->paint_line()");
		return NOT_FINE;
	}

	int x = radius - 1;
	int y = 0;
	int dx = 1;
	int dy = 1;
	int err = dx - (radius << 1);

	while (x >= y) {
		vg_set_pixel(x_position + x, y_position + y, color);
		vg_set_pixel(x_position + y, y_position + x, color);
		vg_set_pixel(x_position - y, y_position + x, color);
		vg_set_pixel(x_position - x, y_position + y, color);
		vg_set_pixel(x_position - x, y_position - y, color);
		vg_set_pixel(x_position - y, y_position - x, color);
		vg_set_pixel(x_position + y, y_position - x, color);
		vg_set_pixel(x_position + x, y_position - y, color);

		if (err <= 0) {
			y++;
			err += dy;
			dy += 2;
		}
		if (err > 0) {
			x--;
			dx += 2;
			err += dx - (radius << 1);
		}
	}
	return FINE;
}

