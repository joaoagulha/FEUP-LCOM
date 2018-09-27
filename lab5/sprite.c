#include "sprite.h"

struct Sprite *Sprite_Create(char *xpm[], unsigned short xi, unsigned short yi,
		unsigned short xf, unsigned short yf, short speed,
		unsigned short frame_rate) {
	struct Sprite *s = (struct Sprite *) malloc(sizeof(struct Sprite));
	if (s == NULL)
		return NULL;

	s->xi = xi;
	s->yi = yi;
	s->xf = xf;
	s->yf = yf;
	s->dx = xf - xi;
	s->dy = yf - yi;

	s->m = s->dy / s->dx;
	s->b = s->yf - s->m * s->xf;

	s->x_position = xi;
	s->speed = speed;
	s->negative_speed_counter = 0;
	s->frame_rate = frame_rate;
	s->map = vg_xpmToPixmap(xpm, &s->width, &s->height);
	s->stop = 0;
	if (s->map == NULL) {
		free(s);
		return NULL;
	}

	return s;
}

void Sprite_Delete(struct Sprite *s) {
	if (s == NULL)
		return;

	if (s->map != NULL)
		free(s->map);

	free(s);
	s = NULL;
}

void Sprite_Draw(struct Sprite *s) {
	vg_drawSprite(s);
}

void Sprite_Move(struct Sprite *s) {
	if (s->stop == 0) {

		if (s->speed > 0) {
			vg_clearSprite(s);
			Sprite_Draw(s);
			video_dump_fb();

			if (s->x_position < s->xf) {

				s->x_position += s->speed;
				if (s->x_position >= s->xf) {
					s->stop = 1;
				}

			} else if (s->x_position > s->xf) {
				s->x_position -= s->speed;
				if (s->x_position <= s->xf) {
					s->stop = 1;
				}
			}

			if (s->yi < s->yf) {
//				s->yi = s->dy * (s->x_position - s->xi) / s->dx;
				s->yi += s->speed;
				if (s->yi >= s->yf) {
					s->stop = 1;
				}
			}

			if (s->yi > s->yf) {
				s->yi -= s->speed;
				if (s->yi <= s->yf) {
					s->stop = 1;
				}
			}
		} else {
			if (s->negative_speed_counter != -s->speed) {
				s->negative_speed_counter++;
			} else {
				vg_clearSprite(s);
				Sprite_Draw(s);
				video_dump_fb();

				if (s->x_position < s->xf) {

					s->x_position++;
					if (s->x_position >= s->xf) {
						s->stop = 1;
					}
				} else if (s->x_position > s->xf) {

					s->x_position--;
					if (s->x_position <= s->xf) {
						s->stop = 1;
					}
				}

				if (s->yi < s->yf) {
					//				s->yi = s->dy * (s->x_position - s->xi) / s->dx;
					s->yi++;
					if (s->yi >= s->yf) {
						s->stop = 1;
					}
				}

				if (s->yi > s->yf) {
					s->yi--;
					if (s->yi <= s->yf) {
						s->stop = 1;
					}
				}

				s->negative_speed_counter = 0;
			}
		}
//	}
	}
}
