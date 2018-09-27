#include "vbe.h"

#define LINEAR_MODEL_BIT 14

#define PB2BASE(x) (((x) >> 4) & 0x0F000)
#define PB2OFF(x) ((x) & 0x0FFFF)

//VBE Commands
#define VBE_MODE_INFO 0x4F01
#define VBE_CTRL_INFO 0x4F00

int vbe_get_mode_info(unsigned short mode, vbe_mode_info_t *vmi_p) {

	if (lm_init() == NULL) {
		printf(
				"\n\nWARNING: an error occurred. lm_init() failed\nStack Trace: \n->vbe_get_mode_info()");
		return NOT_FINE;
	}

	mmap_t m;
	if (lm_alloc(sizeof(vbe_mode_info_t), &m) == NULL) {
		printf(
				"\n\nWARNING: an error occurred. lm_alloc() failed\nStack Trace: \n->vbe_get_mode_info()");
		return NOT_FINE;
	}

	struct reg86u reg86;
	reg86.u.w.di = PB2OFF(m.phys); /* set the offset accordingly */
	reg86.u.w.es = PB2BASE(m.phys); /* set a segment base */
	reg86.u.w.ax = VBE_MODE_INFO; /* VBE get mode info */
	/* translate the buffer linear address to a far pointer */
	reg86.u.b.intno = 0x10;
	reg86.u.w.cx = mode;

	if (sys_int86(&reg86) != FINE) { /* call BIOS */
		printf(
				"\n\nWARNING: an error occurred. sys_int86() failed\nStack Trace: \n->vbe_get_mode_info()");
		lm_free(&m);
		return NOT_FINE;
	}

	*vmi_p = *((vbe_mode_info_t *) m.virtual);
	lm_free(&m);

	return FINE;
}

void* vbe_get_controller_info(vbe_controller_info_t *vbe_info) {
	mmap_t map;
	struct reg86u r;

	void* aux = lm_init();
	if (aux == NULL) {
		printf(
				"\n\nWARNING: an error occurred. lm_init() failed\nStack Trace: \n->vbe_get_controller_info()");
		return NULL;
	}

	if (lm_alloc(sizeof(vbe_controller_info_t), &map) == NULL) {
		printf(
				"\n\nWARNING: an error occurred. lm_alloc() failed\nStack Trace: \n->vbe_get_controller_info()");
		return NULL;
	}

	vbe_info->VbeSignature[3] = '2';
	vbe_info->VbeSignature[0] = 'V';
	vbe_info->VbeSignature[2] = 'E';
	vbe_info->VbeSignature[1] = 'B';

	r.u.b.intno = 0x10;

	r.u.w.di = PB2OFF(map.phys);
	r.u.w.es = PB2BASE(map.phys);
	r.u.w.ax = VBE_CTRL_INFO; /*Get Controller Info Function*/

	if (sys_int86(&r) != FINE) {
		printf(
				"\n\nWARNING: an error occurred. sys_int86() failed\nStack Trace: \n->vbe_get_mode_info()");
		lm_free(&map);
		return NULL;
	}

	*vbe_info = *(vbe_controller_info_t*) map.virtual;
	lm_free(&map);
	return aux;
}
