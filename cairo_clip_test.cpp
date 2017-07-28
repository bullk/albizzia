/*
*/

//#include <stdlib.h>
//#include <stdint.h>
//#include <stdio.h>
//#include <string.h>
//#include <math.h>
//#include <cairo/cairo.h>
//#include <stk/FileWvIn.h>
#include <ftw.h>
#include <fnmatch.h>
#include <pugl/pugl.hpp>
#include "audioclip.hpp"

static int wWitdh = 830;
static int wHeight = 220;
static int quit = 0;
static bool entered = false;

typedef void (*command)(std::string);

static void textDraw(cairo_t* cr, int x, int y, const char* text)
{
	cairo_text_extents_t te;
	COLOR_TEXT;
	cairo_select_font_face (cr, "Laksaman",
		CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
		 //CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size (cr, 14);
	cairo_text_extents (cr, text, &te);
	cairo_move_to (cr, x, y);
	//fprintf(stderr, "writing %s at %d %d \n", text, x, y);
	cairo_show_text (cr, text);
}


class Item : public Widget
{
public:
	Item (int x, int y, int w, int h, std::string s, command c) :
		Widget(x, y, w, h), str_(s)
	{
		fprintf(stderr, "creating Item %s\n", s.c_str());
		str_ = s;
		command_ = c;
	}
	~Item () {}
	virtual void draw(cairo_t* cr)
	{
		if (mouse_over_) {
			fprintf(stderr, "highlight %d\n", mouse_over_);
			COLOR_BOR;
		}
		else 
			COLOR_WBG;
		cairo_rectangle (cr, x_, y_, w_, h_);
		cairo_fill_preserve (cr);
		COLOR_BOR;
		cairo_set_line_width (cr, 1);
		cairo_stroke (cr);
		textDraw(cr, x_+7, y_+h_-7, str_.c_str());
	}
	virtual void highlight(double x, double y)
	{
		mouse_over_ = touches(x, y);
		if (mouse_over_)
			fprintf(stderr, "Item %s highlight %d\n", str_.c_str(), mouse_over_);
	}
protected:
	std::string str_;
	command command_;
};
	

#include <vector>
#include <algorithm>

class Selector : public Widget
{
public:
	Selector (int x, int y, int w, int h):
		Widget(x, y, w, h)
	{
	}
	~Selector () {}
	void draw(cairo_t* cr)
	{
		//fprintf(stderr, "highlight %d\n", mouse_over_);
		//if (mouse_over_) COLOR_BOR;
		//else
			COLOR_WBG;
		cairo_rectangle (cr, x_, y_, w_, h_);
		cairo_fill_preserve (cr);
		COLOR_BOR;
		cairo_set_line_width (cr, 1);
		cairo_stroke (cr);
		for (Item i : items_)
			i.draw(cr);
	}
	void addItem (std::string s, command c)
	{
		long unsigned int i = items_.size();
		std::string fname;
		std::size_t found = s.rfind("/");
		if (found!=std::string::npos)
			fname = s.substr(found+1);
		else fname = s;
		fprintf(stderr, "Adding item %s at pos %lu\n", fname.c_str(), i);
		items_.push_back(Item(x_, i*24 + y_, w_, 24, fname, c));
	}
	void highlight (double x, double y)
	{
		//fprintf(stderr, "panel highlight\n");
		mouse_over_ = touches(x, y);
		if (mouse_over_)
			for (Item i : items_)
				i.highlight(x, y);
	}

protected:
	std::vector<Item> items_;
};


static Selector panel = Selector(620, 10, 200, 200);
static AudioClip clip = AudioClip(10, 10, 600, 200, "JoDassMix.20161105.wav");
static std::vector<std::string> audiofiles_;

int scanAudioFilesCallback( const char *fpath, const struct stat *sb, int typeflag )
{
	char * localpath = strdup( fpath );
	if ( typeflag == FTW_F ) {
		if ( fnmatch( "*.wav", localpath, FNM_CASEFOLD ) == 0 ) {
			audiofiles_.push_back( localpath );
		}
	}
	return 0;
}

int scanAudioFiles ()
{
	audiofiles_.clear();
	ftw( ".", scanAudioFilesCallback, 16 );
	std::sort( audiofiles_.begin(), audiofiles_.end() );
	return 0;
}


static void
onDisplay(PuglView* view)
{
	cairo_t* cr = (cairo_t*)puglGetContext(view);

	// Draw background
	int width, height;
	puglGetSize(view, &width, &height);
	COLOR_BG;
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

static void onEvent(PuglView* view, const PuglEvent* event)
{
	switch (event->type) {
	case PUGL_MOTION_NOTIFY:
		if (clip.pressed()) {
			clip.zoom(event->motion.x, event->motion.y);
		}
		else
			panel.highlight(event->motion.x, event->motion.y);
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

void changeFile(std::string s)
{
	clip.loadFile(s);
}

int main(int argc, char** argv)
{
	scanAudioFiles();
	for (std::string s : audiofiles_)
		panel.addItem(s, changeFile);

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

