#ifndef _CI_DRAW_RECT_H_
#define _CI_DRAW_RECT_H_

typedef struct draw_rect_params_t {
	void *reg_base;
	void *buffer_base;
	int x1;
	int y1;
	int x2;
	int y2;
	unsigned char color;
};

void vga_draw_rect(draw_rect_params_t *params);

#endif
