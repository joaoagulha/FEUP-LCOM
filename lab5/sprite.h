#ifndef _SPRITE_H
#define _SPRITE_H

#include "video_gr.h"

struct Sprite {
	unsigned short xi, yi, xf, yf;
	int width, height;  // dimensions
	short speed;
	unsigned short frame_rate, negative_speed_counter, x_position;
	float dx, dy, m, b;
	char * map;          // the pixmap
	unsigned char stop;
};

struct Sprite *Sprite_Create(char *xpm[], unsigned short xi, unsigned short yi,
		unsigned short xf, unsigned short yf, short speed,
		unsigned short frame_rate);

void Sprite_Move(struct Sprite *s);

void Sprite_Delete(struct Sprite *s);

void Sprite_Draw(struct Sprite *s);

#endif
