#pragma once

#include <foxos/psf1_font.h>
#include <foxos/g_syscalls.h>

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <extern.h>

extern framebuffer_t global_fb;
extern void* global_fb_ptr;

#ifdef __cplusplus
extern "C" {
#endif

static inline void get_fb_data() {
	if (global_fb_ptr == NULL) {
		global_fb = fb_info();
		global_fb_ptr = malloc(global_fb.buffer_size);
	}
}

static inline void fox_set_background(uint32_t colour) {
	uint64_t base = (uint64_t) global_fb_ptr;
	uint64_t bytes_per_scanline = global_fb.width * 4;
	uint64_t fb_height = global_fb.height;

	for (int vertical_scanline = 0; vertical_scanline < fb_height; vertical_scanline ++){
		uint64_t pix_ptr_base = base + (bytes_per_scanline * vertical_scanline);
		for (uint32_t* pixPtr = (uint32_t*)pix_ptr_base; pixPtr < (uint32_t*)(pix_ptr_base + bytes_per_scanline); pixPtr ++){
			*pixPtr = colour;
		}
	}
}

static inline void fox_start_frame(bool empty_fb) {
	get_fb_data();

	if (!empty_fb) {
		copy_from_fb(global_fb_ptr);
	} else {
		fox_set_background(0);
	}
}

static inline void fox_end_frame() {
	if (global_fb_ptr == NULL) {
		return;
	}

	copy_to_fb(global_fb_ptr);
}

static inline void fox_free_framebuffer() {
	if (global_fb_ptr == NULL) {
		return;
	}

	free(global_fb_ptr);
	global_fb_ptr = NULL;
}

static inline void fox_set_px(uint32_t x, uint32_t y, uint32_t colour) {
	if (x < 0 || x >= global_fb.width || y < 0 || y >= global_fb.height) {
		return;
	}

	*(uint32_t*)((uint64_t) global_fb_ptr + (x * 4) + (y * 4 * global_fb.width)) = colour;
}

static inline void fox_set_px_unsafe(uint32_t x, uint32_t y, uint32_t colour) {
	*(uint32_t*)((uint64_t) global_fb_ptr + (x * 4) + (y * 4 * global_fb.width)) = colour;
}

static inline void fox_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t colour) {
	if (x >= global_fb.width || y >= global_fb.height) {
		return;
	}

	if (x + width > global_fb.width) {
		width = global_fb.width - x;
	}
	if (y + height > global_fb.height) {
		height = global_fb.height - y;
	}

	if (x < 0 && x + width > 0) {
		width -= x;
		x = 0;
	}
	if (y < 0 && y + height > 0) {
		height -= y;
		y = 0;
	}

	for (uint32_t j = 0; j < height; j++) {
		for (uint32_t i = 0; i < width; i++) {
			fox_set_px_unsafe(i + x, j + y, colour);
		}
	}
}

static inline void fox_draw_rect_outline(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t colour) {
	if (x >= global_fb.width || y >= global_fb.height) {
		return;
	}

	bool width_too_big = false;
	bool height_too_big = false;

	if (x + width > global_fb.width) {
		width = global_fb.width - x;
		width_too_big = true;
	}
	if (y + height > global_fb.height) {
		height = global_fb.height - y;
		height_too_big = true;
	}

	for (uint32_t i = 0; i < width; i++) {
		fox_set_px(x + i, y, colour);
		if (!height_too_big) {
			fox_set_px(x + i, y + height - 1, colour);
		}
	}

	for (uint32_t i = 0; i < height; i++) {
		fox_set_px_unsafe(x, y + i, colour);
		if (!width_too_big) {
			fox_set_px_unsafe(x + width - 1, y + i, colour);
		}
	}
}

static inline void fox_draw_circle(uint32_t x, uint32_t y, uint32_t r, uint32_t colour) {
	// x and y are the center of the circle
	// r is the radius


	// Bresenham's circle algorithm
	// https://en.wikipedia.org/wiki/Midpoint_circle_algorithm

	int32_t f;
	int32_t ddF_x;
	int32_t ddF_y;
	int32_t x1;
	int32_t y1;

next_iter:
	f = 1 - r;
	ddF_x = 0;
	ddF_y = -2 * r;
	x1 = 0;
	y1 = r;

	fox_set_px(x, y + r, colour);
	fox_set_px(x, y - r, colour);
	fox_set_px(x + r, y, colour);
	fox_set_px(x - r, y, colour);

	while (x1 < y1) {
		if (f >= 0) {
			y1--;
			ddF_y += 2;
			f += ddF_y;
		}

		x1++;
		ddF_x += 2;
		f += ddF_x + 1;

		fox_set_px(x + x1, y + y1, colour);
		fox_set_px(x - x1, y + y1, colour);
		fox_set_px(x + x1, y - y1, colour);
		fox_set_px(x - x1, y - y1, colour);

		fox_set_px(x + y1, y + x1, colour);
		fox_set_px(x - y1, y + x1, colour);
		fox_set_px(x + y1, y - x1, colour);
		fox_set_px(x - y1, y - x1, colour);
	}

	r--;

	if (r > 0) {
		goto next_iter;
	}
}

static inline void fox_draw_circle_outline(uint32_t x, uint32_t y, uint32_t r, uint32_t colour) {
	// x and y are the center of the circle
	// r is the radius


	// Bresenham's circle algorithm
	// https://en.wikipedia.org/wiki/Midpoint_circle_algorithm

	int32_t f = 1 - r;
	int32_t ddF_x = 0;
	int32_t ddF_y = -2 * r;
	int32_t x1 = 0;
	int32_t y1 = r;

	fox_set_px(x, y + r, colour);
	fox_set_px(x, y - r, colour);
	fox_set_px(x + r, y, colour);
	fox_set_px(x - r, y, colour);

	while (x1 < y1) {
		if (f >= 0) {
			y1--;
			ddF_y += 2;
			f += ddF_y;
		}

		x1++;
		ddF_x += 2;
		f += ddF_x + 1;

		fox_set_px(x + x1, y + y1, colour);
		fox_set_px(x - x1, y + y1, colour);
		fox_set_px(x + x1, y - y1, colour);
		fox_set_px(x - x1, y - y1, colour);

		fox_set_px(x + y1, y + x1, colour);
		fox_set_px(x - y1, y + x1, colour);
		fox_set_px(x + y1, y - x1, colour);
		fox_set_px(x - y1, y - x1, colour);
	}
}

static void fox_draw_char(uint32_t x, uint32_t y, char c, uint32_t colour, psf1_font_t* font) {
	char* font_ptr = (char*) font->glyph_buffer + (c * font->psf1_Header->charsize);

	for (unsigned long i = y; i < y + 16; i++){
		for (unsigned long j = x; j < x + 8; j++){
			if ((*font_ptr & (0b10000000 >> (j - x))) > 0) {
				fox_set_px(j, i, colour);
			}
		}
		font_ptr++;
	}
}

static void fox_draw_string(uint32_t x, uint32_t y, char* str, uint32_t colour, psf1_font_t* font) {
	uint32_t i = 0;
	while (str[i] != '\0') {
		fox_draw_char(x + (i * 8), y, str[i], colour, font);
		i++;
	}
}

static inline int fox_abs(int x) {
	if (x < 0) {
		return -x;
	}
	return x;
}

static void fox_draw_line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t colour) {
	// Bresenham's line algorithm
	// https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm

	int32_t dx = fox_abs(x2 - x1);
	int32_t dy = fox_abs(y2 - y1);
	int32_t sx = x1 < x2 ? 1 : -1;
	int32_t sy = y1 < y2 ? 1 : -1;
	int32_t err = dx - dy;

	while (1) {
		fox_set_px(x1, y1, colour);

		if (x1 == x2 && y1 == y2) {
			break;
		}

		int32_t e2 = 2 * err;

		if (e2 > -dy) {
			err -= dy;
			x1 += sx;
		}

		if (e2 < dx) {
			err += dx;
			y1 += sy;
		}
	}
}

#ifdef __cplusplus
}
#endif