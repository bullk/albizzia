#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pugl/pugl.hpp>
#include <cairo/cairo.h>

static int wWitdh = 480;
static int wHeight = 480;
static int quit = 0;
static bool entered = false;
//static int n = 1;



static void onDisplay(PuglView* view)
{
	cairo_t* cr = (cairo_t*)puglGetContext(view);

	// Draw background
	int width, height;
	puglGetSize(view, &width, &height);
	cairo_rectangle(cr, 0, 0, width, height);
	cairo_fill(cr);

	// Draw image
	int w, h, x, y;
	cairo_surface_t *image;

	image = cairo_image_surface_create_from_png ("text3023.png");
	w = cairo_image_surface_get_width (image);
	h = cairo_image_surface_get_height (image);
	x = (width-w) /2;
	y = (height-h) /2;
	
	cairo_translate (cr, width/2, height/2);
	cairo_rotate (cr, (float)(entered * M_PI)/180);
	//if (rand()%2) cairo_scale (cr, 1.0, 1.0);
	//else cairo_scale (cr, 0.99, 0.99);
	cairo_translate (cr, -width/2, -height/2);

	//cairo_set_source_surface (cr, image, x, y);
	cairo_set_source_surface (cr, image, x, y);
	cairo_paint (cr);
	cairo_surface_destroy (image);	
}

static void onClose(PuglView* view)
{
	quit = 1;
}

static void onEvent(PuglView* view, const PuglEvent* event)
{
	switch (event->type) {
	case PUGL_MOTION_NOTIFY:
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
		break;
	case PUGL_BUTTON_RELEASE:
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

int main(int argc, char** argv)
{
	bool useGL           = false;
	bool ignoreKeyRepeat = false;
	bool resizable       = false;

	PuglView* view = puglInit(NULL, NULL);
	puglInitWindowSize(view, wWitdh, wHeight);
	puglInitResizable(view, resizable);
	puglInitContextType(view, useGL ? PUGL_CAIRO_GL : PUGL_CAIRO);

	puglIgnoreKeyRepeat(view, ignoreKeyRepeat);
	puglSetEventFunc(view, onEvent);

	puglCreateWindow(view, "splash bullk");
	puglShowWindow(view);

	while (!quit) {
		puglWaitForEvent(view);
		puglProcessEvents(view);
	}

	puglDestroy(view);
	
	return 0;
}

