#include "test5.h"
#include "pixmap.h"

static int proc_args(int argc, char **argv);
static unsigned long parse_ulong(char *str, int base);
static void print_usage(char **argv);

int main(int argc, char **argv) {
	/* DO NOT FORGET TO initialize service */
	sef_startup();
	if (argc == 1) { /* Prints usage of the program if no arguments are passed */
		print_usage(argv);
		return 0;
	} else
		return proc_args(argc, argv);
}

static void print_usage(char **argv) {
	printf(
			"Usage: one of the following:\n"
					"\t service run %s -args \"init <mode> <delay>\"\n"
					"\t service run %s -args \"square <x> <y> <size> <color>\"\n"
					"\t service run %s -args \"line <xi>  <yi> <xf> <yf> <color>\"\n"
					"\t service run %s -args \"xpm <xpm> <xi> <yi>\"\n"
					"\t service run %s -args \"move <xpm> <xi>  <yi> <xf> <yf> <speed> <frame_rate>\"\n"
					"\t service run %s -args \"circle <x> <y> <radius> <color>\"\n"
					"\t service run %s -args \"controller\"\n", argv[0],
			argv[0], argv[0], argv[0], argv[0], argv[0], argv[0]);
}

static int proc_args(int argc, char **argv) {
	unsigned short mode, delay, x, y, size, xi, yi, xf, yf, radius;
	unsigned long color;
	char **pix_map;

	if (strncmp(argv[1], "init", strlen("init")) == 0) {

		if (argc != 4) {
			printf("video: wrong no. of arguments for video_test_init()\n");
			return 1;
		}

		mode = parse_ulong(argv[2], 10); /* Parses string to unsigned long */
		if (mode == ULONG_MAX)
			return 1;

		delay = parse_ulong(argv[2], 10); /* Parses string to unsigned long */
		if (delay == ULONG_MAX)
			return 1;

		printf("video::video_test_init(%lu,%d)\n", mode, delay);
		return video_test_init(mode, delay);

	} else if (strncmp(argv[1], "square", strlen("square")) == 0) {
		if (argc != 6) {
			printf("video: wrong no of arguments for video_test_remote()\n");
			return 1;
		}

		x = parse_ulong(argv[2], 10); /* Parses string to unsigned long */
		if (x == ULONG_MAX)
			return 1;

		y = parse_ulong(argv[3], 10); /* Parses string to unsigned long */
		if (y == ULONG_MAX)
			return 1;

		size = parse_ulong(argv[4], 10); /* Parses string to unsigned long */
		if (size == ULONG_MAX)
			return 1;

		color = parse_ulong(argv[5], 10); /* Parses string to unsigned long */
		if (color == ULONG_MAX)
			return 1;

		printf("video::video_test_square(%lu,%d,%d,%d)\n", x, y, size, color);
		return video_test_square(x, y, size, color);

	} else if (strncmp(argv[1], "line", strlen("line")) == 0) {
		if (argc != 7) {
			printf("video: wrong no of arguments for video_test_line()\n");
			return 1;
		}

		xi = parse_ulong(argv[2], 10); /* Parses string to unsigned long */
		if (xi == ULONG_MAX)
			return 1;

		yi = parse_ulong(argv[3], 10); /* Parses string to unsigned long */
		if (yi == ULONG_MAX)
			return 1;

		xf = parse_ulong(argv[4], 10); /* Parses string to unsigned long */
		if (xf == ULONG_MAX)
			return 1;

		yf = parse_ulong(argv[5], 10); /* Parses string to unsigned long */
		if (yf == ULONG_MAX)
			return 1;

		color = parse_ulong(argv[6], 10); /* Parses string to unsigned long */
		if (color == ULONG_MAX)
			return 1;

		printf("video::video_test_line(%d,%d,%d,%d,%d)\n", xi, yi, xf, yf,
				color);
		return video_test_line(xi, yi, xf, yf, color);

	} else if (strncmp(argv[1], "xpm", strlen("xpm")) == 0) {
		if (argc != 5) {
			printf("ERROR: wrong no. of arguments for test_xpm()\n");
			return 1;
		}

		if (strncmp(argv[2], "cross", strlen("cross")) == 0) {
			pix_map = cross;
		} else if (strncmp(argv[2], "penguin", strlen("penguin")) == 0) {
			pix_map = penguin;
		} else if (strncmp(argv[2], "pic1", strlen("pic1")) == 0) {
			pix_map = pic1;
		} else if (strncmp(argv[2], "pic2", strlen("pic2")) == 0) {
			pix_map = pic2;
		} else if (strncmp(argv[2], "pic3", strlen("pic3")) == 0) {
			pix_map = pic3;
		} else {
			printf("\video::test_xpm() Invalid XPM");
			return 1;
		}
		if (**pix_map == ULONG_MAX)
			return 1;

		xi = parse_ulong(argv[3], 10); /* Parses string to unsigned long */
		if (xi == ULONG_MAX)
			return 1;

		yi = parse_ulong(argv[4], 10); /* Parses string to unsigned long */
		if (yi == ULONG_MAX) {
			return 1;
		}

		//printf("Graphic::test_xpm(%lu, %lu, %s)\n", argv[2], xi, yi);
		return test_xpm(pix_map, xi, yi);
	} else if (strncmp(argv[1], "move", strlen("move")) == 0) {
		if (argc != 9) {
			printf("ERROR: wrong no. of arguments for test_xpm()\n");
			return 1;
		}

		short s;
		unsigned short f;

		if (strncmp(argv[2], "cross", strlen("cross")) == 0) {
			pix_map = cross;
		} else if (strncmp(argv[2], "penguin", strlen("penguin")) == 0) {
			pix_map = penguin;
		} else if (strncmp(argv[2], "pic1", strlen("pic1")) == 0) {
			pix_map = pic1;
		} else if (strncmp(argv[2], "pic2", strlen("pic2")) == 0) {
			pix_map = pic2;
		} else if (strncmp(argv[2], "pic3", strlen("pic3")) == 0) {
			pix_map = pic3;
		} else {
			printf("\video::test_xpm() Invalid XPM");
			return 1;
		}

		if (**pix_map == ULONG_MAX)
			return 1;

		xi = parse_ulong(argv[3], 10); /* Parses string to unsigned long */
		if (xi == ULONG_MAX)
			return 1;
		yi = parse_ulong(argv[4], 10); /* Parses string to unsigned long */
		if (yi == ULONG_MAX)
			return 1;
		xf = parse_ulong(argv[5], 10); /* Parses string to unsigned long */
		if (xf == ULONG_MAX)
			return 1;
		yf = parse_ulong(argv[6], 10); /* Parses string to unsigned long */
		if (yf == ULONG_MAX)
			return 1;
		s = parse_ulong(argv[7], 10); /* Parses string to unsigned long */

		f = parse_ulong(argv[8], 10); /* Parses string to unsigned long */
		if (f == ULONG_MAX)
			return 1;

		//printf("Graphic::test_xpm(%lu, %lu, %s)\n", argv[2], xi, yi);
		return test_move(pix_map, xi, yi, xf, yf, s, f);
	} else if (strncmp(argv[1], "circle", strlen("circle")) == 0) {
		if (argc != 6) {
			printf("ERROR: wrong no. of arguments for test_circle()\n");
			return 1;
		}

		x = parse_ulong(argv[2], 10); /* Parses string to unsigned long */
		if (xi == ULONG_MAX)
			return 1;

		x = parse_ulong(argv[3], 10); /* Parses string to unsigned long */
		if (yi == ULONG_MAX) {
			return 1;
		}

		radius = parse_ulong(argv[4], 10); /* Parses string to unsigned long */
		if (radius == ULONG_MAX)
			return 1;

		color = parse_ulong(argv[5], 10); /* Parses string to unsigned long */
		if (color == ULONG_MAX)
			return 1;

		//printf("Graphic::test_xpm(%lu, %lu, %s)\n", argv[2], xi, yi);
		return test_circle(x, y, radius, color);
	} else if (strncmp(argv[1], "controller", strlen("controller")) == 0) {
		if (argc != 2) {
			printf("ERROR: wrong no. of arguments for test_controller()\n");
			return 1;
		}

		//printf("Graphic::test_xpm(%lu, %lu, %s)\n", argv[2], xi, yi);
		return test_controller();
	} else {
		printf("mouse: %s -> is not a valid function!\n", argv[1]);
		return 1;
	}

}

static unsigned long parse_ulong(char *str, int base) {
	char *endptr;
	unsigned long val;

	/* Convert string to unsigned long */
	val = strtoul(str, &endptr, base);

	/* Check for conversion errors */
	if ((errno == ERANGE && val == ULONG_MAX) || (errno != 0 && val == 0)) {
		perror("strtoul");
		return ULONG_MAX;
	}

	if (endptr == str) {
		printf("kbd: parse_ulong: no digits were found in %s\n", str);
		return ULONG_MAX;
	}

	/* Successful conversion */
	return val;
}
