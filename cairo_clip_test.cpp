/*
*/

//#include <stdlib.h>
//#include <stdint.h>
//#include <stdio.h>
//#include <string.h>
//#include <math.h>
//#include <cairo/cairo.h>
//#include <stk/FileWvIn.h>
#include <pugl/pugl.hpp>
#include "audioclip.hpp"

static int wWitdh = 830;
static int wHeight = 220;
static int quit = 0;
static bool entered = false;


static void textDraw(cairo_t* cr, int index, const char* label);

class Item;

#include <vector>
class Selector
{
public:
	Selector (cairo_t* cr, int x, int y, int w, int h):
		Widget(cairo_t* cr, int x, int y, int w, int h)
	{
		items_ = new vector<Item>;
		
	}
	~Selector () {}
	void draw(cairo_t* cr)
	{
		cairo_set_source_rgb (cr, 0.2, 0.2, 0.2);
		cairo_rectangle (cr, x_, y_, w_, h_);
		cairo_fill_preserve (cr);
		cairo_set_line_width (cr, 1);
		cairo_set_source_rgb (cr, 0.4, 0.4, 0.4);
		cairo_stroke (cr);
		textDraw(cr, 0, "fichier.wav");
	}
	void addItem ()
	{
	}

protected:
	int x_, y_, w_, h_;
	bool ui_state_;	
	std::vector<Item> items_;
};

static AudioClip clip = AudioClip("JoDassMix.20161105.wav", 10, 10, 600, 200);
static Selector panel = Selector(620, 10, 200, 200);

static void
textDraw(cairo_t* cr, int index, const char* label)
{
	cairo_text_extents_t te;
	cairo_set_source_rgb (cr, 0.8, 0.8, 0.8);
	cairo_select_font_face (cr, "Laksaman",
		CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
		 //CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size (cr, 14);
	cairo_text_extents (cr, label, &te);
	cairo_move_to (cr, 630, 30+index*40);
	cairo_show_text (cr, label);
}


static void
onDisplay(PuglView* view)
{
	cairo_t* cr = (cairo_t*)puglGetContext(view);

	// Draw background
	int width, height;
	puglGetSize(view, &width, &height);
	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_rectangle(cr, 0, 0, width, height);
	cairo_fill(cr);

	// Draw clip
	panel.draw(cr);
	clip.draw(cr);
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
		//if (clip.pressed() & entered) {
		if (clip.pressed()) {
			clip.zoom(event->motion.x, event->motion.y);
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
		if (clip.touches(event->button.x, event->button.y)) {
			clip.press(event->button.x, event->button.y);
		}
		break;
	case PUGL_BUTTON_RELEASE:
		clip.release();
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

	bool useGL           = false;
	bool ignoreKeyRepeat = false;
	bool resizable       = false;

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
	
	return 0;
}

