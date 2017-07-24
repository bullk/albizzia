/*
  Copyright 2012-2014 David Robillard <http://drobilla.net>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THIS SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

/**
   @file pugl_cairo_test.c A simple Pugl test that creates a top-level window.
*/

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
//#include <tgmath.h>
#include <cairo/cairo.h>

#include "pugl/pugl.h"

static int wWitdh = 800;
static int wHeight = 600;
static int quit = 0;
static bool entered = false;
const unsigned int AUDIO_DEPTH = 1<<16;
const unsigned int AUDIO_SCALE = 1<<15;
const float ZOOM_SPEED = 0.02f;

typedef struct {
	int         x;
	int         y;
	int         w;
	int         h;
	bool        pressed;
	const char* label;
} Button;

static Button toggle_button = { 10, 10, 40, 40, false, "" };

typedef struct {
	int         x;
	int         y;
	int			radius;
	float		angle;
	bool        pressed;
} Pie;

static Pie dapie = { 30, 80, 20, 1.0f, false };

typedef struct {
	unsigned int size;
	short* data;
	unsigned int left, right;
	int x, y, w, h;
	bool pressed;
	unsigned int xpress, ypress;
} AudioClip;

static AudioClip daclip = { 44100, NULL, 0, 0, 10, 290, 600, 300, false, 0, 0 };

static void
roundedBox(cairo_t* cr, double x, double y, double w, double h)
{
	static const double radius  = 10;
	static const double degrees = 3.14159265 / 180.0;

	cairo_new_sub_path(cr);
	cairo_arc(cr,
	          x + w - radius,
	          y + radius,
	          radius, -90 * degrees, 0 * degrees);
	cairo_arc(cr,
	          x + w - radius, y + h - radius,
	          radius, 0 * degrees, 90 * degrees);
	cairo_arc(cr,
	          x + radius, y + h - radius,
	          radius, 90 * degrees, 180 * degrees);
	cairo_arc(cr,
	          x + radius, y + radius,
	          radius, 180 * degrees, 270 * degrees);
	cairo_close_path(cr);
}

static void
buttonDraw(cairo_t* cr, const Button* but)
{
	// Draw base
	cairo_set_source_rgba(cr, 0.2, 0.5, 0.0, 1);
	roundedBox(cr, but->x, but->y, but->w, but->h);
	cairo_fill_preserve(cr);
	
	// Draw border
	cairo_set_source_rgba(cr, 0.2, 0.5, 0.0, 0.5);
	cairo_set_line_width(cr, 4.0);
	cairo_stroke(cr);
	
	// Draw icon
	if (but->pressed) {
		cairo_set_source_rgba(cr, 0.6, 1.0, 0.0, 1);
	} else {
		cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1);
	}
	cairo_move_to (cr, but->x + 13, but->y + 10);
	cairo_line_to (cr, but->x + 30, but->y + 20);
	cairo_line_to (cr, but->x + 13, but->y + 30);
	cairo_close_path(cr);
	cairo_fill(cr);
	
	// Draw label
	//cairo_text_extents_t extents;
	//cairo_set_font_size(cr, 32.0);
	//cairo_text_extents(cr, but->label, &extents);
	//cairo_move_to(cr,
	              //(but->x + but->w / 2) - extents.width / 2,
	              //(but->y + but->h / 2) + extents.height / 2);
	//cairo_set_source_rgba(cr, 0, 0, 0, 1);
	//cairo_show_text(cr, but->label);
}

static void
pieDraw(cairo_t* cr, const Pie* pie)
{
	static const double cycle = 2 * 3.14159265;
	
	// Draw base
	cairo_set_source_rgb(cr, 0.8, 0.8, 0.0);
	cairo_new_sub_path(cr);
	cairo_arc(cr, pie->x, pie->y, pie->radius,
		-0.25 * cycle, (pie->angle-0.25) * cycle);
	cairo_line_to (cr, pie->x, pie->y);
	cairo_close_path(cr);
	cairo_fill_preserve(cr);

	// Draw border
	cairo_set_source_rgb(cr, 0.4, 0.4, 0.0);
	cairo_set_line_width(cr, 2.0);
	cairo_stroke(cr);
}

static void
clipDraw(cairo_t* cr, const AudioClip* clip)
{
	unsigned int ww = clip->right - clip->left;
	const float bottom = (float)(clip->y + clip->h);
	const float xscale = (float)ww / clip->w;
	const float yscale = (float)clip->h / AUDIO_DEPTH;
	cairo_set_source_rgb (cr, 0.3, 0.3, 0.3);
	cairo_rectangle (cr, clip->x, clip->y, clip->w, clip->h);
	cairo_fill_preserve (cr);
	cairo_set_line_width (cr, 1);
	cairo_set_source_rgb (cr, 0.5, 0.5, 0.5);
	cairo_stroke (cr);
	cairo_set_source_rgb (cr, 0.9, 0.9, 0.9);
	cairo_move_to (cr, clip->x, bottom - (float)(clip->data[clip->left]+AUDIO_SCALE) * yscale);
	for (unsigned int i=1; i<clip->w; i++) {
		unsigned int j = (unsigned int)(i * xscale) + clip->left;
		cairo_line_to (cr,
			clip->x + (float)i,
			bottom - (float)(clip->data[j]+AUDIO_SCALE) * yscale);
	}
	cairo_stroke (cr);
}

static bool
buttonTouches(const Button* but, double x, double y)
{
	return (x >= toggle_button.x && x <= toggle_button.x + toggle_button.w &&
	        y >= toggle_button.y && y <= toggle_button.y + toggle_button.h);
}

static bool
pieTouches(const Pie* pie, double x, double y)
{
	int dx = x - pie->x;
	int dy = y - pie->y;
	return (dx*dx + dy*dy <= pie->radius * pie->radius);
}

static bool
clipTouches(const AudioClip* clip, double x, double y)
{
	return (x >= clip->x && x <= clip->x + clip->w &&
	        y >= clip->y && y <= clip->y + clip->h);
}

static void
textDraw(cairo_t* cr)
{
	cairo_text_extents_t te;
	//float red = ((float)rand() / RAND_MAX) / 8 + 0.5;
	//float green = ((float)rand() / RAND_MAX) / 8 + 0.875;
	//float blue = ((float)rand() / RAND_MAX) / 8 + 0;
	cairo_set_source_rgb (cr, 0.8, 0.8, 0.8);
	//"Blue Highway Linocut","Butterbelly","Candy Stripe BRK",
	//"Jawbreaker OL1 BRK","Knuckle Down","Laksaman","Lato",
	cairo_select_font_face (cr, "Pants Patrol",
		//CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
		CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size (cr, 20);
	cairo_text_extents (cr, "play", &te);
	cairo_move_to (cr, 60, 35);
	cairo_show_text (cr, "play");
}

static void
onDisplay(PuglView* view)
{
	cairo_t* cr = puglGetContext(view);

	// Draw background
	int width, height;
	puglGetSize(view, &width, &height);
	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_rectangle(cr, 0, 0, width, height);
	cairo_fill(cr);

	// Draw button
	buttonDraw(cr, &toggle_button);
	// Draw text
	textDraw(cr);
	// Draw clip
	clipDraw(cr, &daclip);
	// Draw pie
	pieDraw(cr, &dapie);
}

static void
onClose(PuglView* view)
{
	quit = 1;
}

static void
onEvent(PuglView* view, const PuglEvent* event)
{
	switch (event->type) {
	case PUGL_MOTION_NOTIFY:
		//dapie.angle = (float)event->motion.y / wHeight;
		if (dapie.pressed) {
			dapie.x = (int)event->motion.x;
			dapie.y = (int)event->motion.y;
		}
		if (daclip.pressed & entered) {
			unsigned int ww = daclip.right - daclip.left;
			float ydelta = ZOOM_SPEED * (float)(ww * (daclip.ypress - event->motion.y));
			float xdelta = ZOOM_SPEED * (float)(ww * (event->motion.x - daclip.xpress));
			//fprintf(stderr, "Zoom %d -> %d, ww %d xdelta %f ydelta %f\n",
				//daclip.left, daclip.right, ww, xdelta, ydelta);
			if (daclip.left < -ydelta) daclip.left = 0;
			else daclip.left += ydelta;
			if (daclip.size-daclip.right < -ydelta) daclip.right = daclip.size;
			else if (ww > -ydelta) daclip.right -= ydelta;
				else daclip.right = daclip.left +1;
			if (abs(xdelta) < daclip.left) daclip.left -= xdelta;
			if (abs(xdelta) < daclip.size-daclip.right) daclip.right -= xdelta;
			daclip.xpress = event->motion.x;
			daclip.ypress = event->motion.y;
			
		}
		puglPostRedisplay(view);
		break;
	case PUGL_KEY_PRESS:
		if (event->key.character == 'q' ||
		    event->key.character == 'Q' ||
		    event->key.character == PUGL_CHAR_ESCAPE) {
			quit = 1;
		}
		break;
	case PUGL_BUTTON_PRESS:
		if (buttonTouches(&toggle_button, event->button.x, event->button.y)) {
			toggle_button.pressed = !toggle_button.pressed;
			puglPostRedisplay(view);
		}
		if (pieTouches(&dapie, event->button.x, event->button.y))
			dapie.pressed = true;
		if (clipTouches(&daclip, event->button.x, event->button.y)) {
			//fprintf(stderr, "Entering zoom mode\n");
			daclip.pressed = true;
			daclip.xpress = event->button.x;
			daclip.ypress = event->button.y;
		}
		break;
	case PUGL_BUTTON_RELEASE:
		dapie.pressed = false;
		daclip.pressed = false;
		break;
	case PUGL_ENTER_NOTIFY:
		entered = true;
		puglPostRedisplay(view);
		break;
	case PUGL_LEAVE_NOTIFY:
		entered = false;
		puglPostRedisplay(view);
		break;
	case PUGL_EXPOSE:
		onDisplay(view);
		break;
	case PUGL_CLOSE:
		onClose(view);
		break;
	default: break;
	}
}

int
main(int argc, char** argv)
{
	float freq = 13;
	daclip.data = malloc(daclip.size * sizeof(short));
	float twopi = 2 * M_PI;
	for (unsigned int i=0; i<daclip.size; i++) {
		float x = (float)i*twopi*freq/daclip.size;
		float y = 0;
		y += sin(x);
		y += sin(3*x)/3;
		y += sin(5*x)/5;
		y += sin(7*x)/7;
		y /= 1;
		daclip.data[i] = (short)(y * AUDIO_SCALE);
	}
	daclip.right = daclip.size;

	bool useGL           = false;
	bool ignoreKeyRepeat = false;
	bool resizable       = false;
	for (int i = 1; i < argc; ++i) {
		if (!strcmp(argv[i], "-h")) {
			printf("USAGE: %s [OPTIONS]...\n\n"
			       "  -g  Use OpenGL\n"
			       "  -h  Display this help\n"
			       "  -i  Ignore key repeat\n"
			       "  -r  Resizable window\n", argv[0]);
			return 0;
		} else if (!strcmp(argv[i], "-g")) {
			useGL = true;
		} else if (!strcmp(argv[i], "-i")) {
			ignoreKeyRepeat = true;
		} else if (!strcmp(argv[i], "-r")) {
			resizable = true;
		} else {
			fprintf(stderr, "Unknown option: %s\n", argv[i]);
		}
	}

	PuglView* view = puglInit(NULL, NULL);
	puglInitWindowSize(view, wWitdh, wHeight);
	puglInitResizable(view, resizable);
	puglInitContextType(view, useGL ? PUGL_CAIRO_GL : PUGL_CAIRO);

	puglIgnoreKeyRepeat(view, ignoreKeyRepeat);
	puglSetEventFunc(view, onEvent);

	puglCreateWindow(view, "Pugl Test");
	puglShowWindow(view);

	while (!quit) {
		puglWaitForEvent(view);
		puglProcessEvents(view);
	}

	puglDestroy(view);
	
	free(daclip.data);
	return 0;
}

