/* jack-clock */

#define HAVE_GL 1
#define PUGL_HAVE_GL 1
#define HAVE_CAIRO 1
#define PUGL_HAVE_CAIRO 1

#include <stdio.h>
#include <iostream>
//#include <stdlib.h>
//#include <math.h>

#include <spdlog/spdlog.h>
#include <jack/jack.h>

//#include <X11/Xlib.h>
#include <cairo/cairo.h>
#include "pugl/pugl.h"

#include "Clock.hpp"

using namespace std;

static int wWitdh = 640;
static int wHeight = 480;
static int quit = 0;
static bool entered = false;
jack_client_t* app;
Clock * myclock;
static unsigned int oldBar = 0;
static unsigned int oldBeat = 0;


typedef struct {
	int         x;
	int         y;
	int         w;
	int         h;
	bool        pressed;
	const char* label;
} Button;

static Button toggle_button = { 200, 10, 40, 40, false, "" };

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
	if ( myclock->isStarted() ) {
		cairo_set_source_rgba(cr, 0.6, 1.0, 0.0, 1);
	} else {
		cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1);
	}
	cairo_move_to (cr, but->x + 13, but->y + 10);
	cairo_line_to (cr, but->x + 30, but->y + 20);
	cairo_line_to (cr, but->x + 13, but->y + 30);
	cairo_close_path(cr);
	cairo_fill(cr);
	}

static bool
buttonTouches(const Button* but, double x, double y)
{
	return (x >= toggle_button.x && x <= toggle_button.x + toggle_button.w &&
	        y >= toggle_button.y && y <= toggle_button.y + toggle_button.h);
}

void toggleClock()
{
	if ( myclock->isStarted() ) {
		jack_transport_stop (app);
		myclock->pause();
	}
	else {
		jack_transport_start (app);
		myclock->start();
	}
}

static void onDisplay(PuglView* view)
{
	cairo_t* cr = (cairo_t*)puglGetContext(view);

	// Draw background
	int width, height;
	puglGetSize(view, &width, &height);
	cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
	cairo_rectangle(cr, 0, 0, width, height);
	cairo_fill(cr);

	//cairo_text_extents_t te;

	// Draw hh:mm:ss
	char hms[12];
	sprintf (hms, "%02u:%02u:%02u", myclock->getHour(), myclock->getMinute(), myclock->getSecond());
	cairo_set_source_rgb (cr, 0.0, 0.8, 0.6);
	cairo_select_font_face (cr, "Lato",
		//CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
		 CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size (cr, 32);
	//cairo_text_extents (cr, hms, &te);
	cairo_move_to (cr, 10, 40);
	cairo_show_text (cr, hms);
	
	
	// Draw bar:beat:tick
	char bbt[20];
	sprintf (bbt, "%d:%d:%03d", myclock->getBar(), myclock->getBeat(), myclock->getTick());
	cairo_set_source_rgb (cr, 0.0, 0.6, 0.8);
	//cairo_select_font_face (cr, "Lato",
		 //CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	//cairo_set_font_size (cr, 32);
	cairo_move_to (cr, 10, 80);
	cairo_show_text (cr, bbt);
	
	// Draw button
	buttonDraw(cr, &toggle_button);
	
}

static void onClose(PuglView* view)
{
	quit = 1;
}

static void onEvent(PuglView* view, const PuglEvent* event)
{
	switch (event->type) {
	case PUGL_MOTION_NOTIFY:
		//puglPostRedisplay(view);
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
			toggleClock();
			//puglPostRedisplay(view);
		}
		break;
	case PUGL_BUTTON_RELEASE:
	case PUGL_ENTER_NOTIFY:
		entered = true;
		//puglPostRedisplay(view);
		break;
	case PUGL_LEAVE_NOTIFY:
		entered = false;
		//puglPostRedisplay(view);
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

static void forceDraw (PuglView* view)
{
	//XExposeEvent expose_event;
	//XSendEvent(getVisual(view)->display, puglGetNativeWindow(view), true, ExposureMask, expose_event);
	onDisplay (view);
}

int process_callback (jack_nframes_t nframes, void *arg)
{
	//PuglView* view = (PuglView*) arg;
	//puglPostRedisplay(view);
	return 0;
}

void timebase_callback (jack_transport_state_t state, jack_nframes_t nframes, jack_position_t *pos, int new_pos, void *arg)
{
	myclock->update();
	//unsigned int t = myclock->update();
	//unsigned int u = pos->usecs % 1000000;
	auto mainlog= spdlog::get("main");
	//mainlog->info("timebase_callback : {} tick(s), {} µs", t, u);
	
	PuglView* view = (PuglView*) arg;
	if ( (pos->usecs % 1000000) < 1000 ) forceDraw(view);
	pos->bar = myclock->getBar();
	if ( myclock->getBar() != oldBar ) {
		mainlog->info("bar {}", myclock->getBar());
		oldBar = myclock->getBar();
		forceDraw(view);
	}
	pos->beat = myclock->getBeat();
	if ( myclock->getBeat() != oldBeat ) {
		mainlog->info("beat {}", myclock->getBeat());
		oldBeat = myclock->getBeat();
		forceDraw(view);
	}
	pos->bar = myclock->getBar();
	pos->beat = myclock->getBeat();
	pos->tick = myclock->getTick();
	pos->beats_per_bar = myclock->getBeatsPerBar();
	pos->beat_type = 4;
	pos->ticks_per_beat = myclock->getTicksPerBeat();
	pos->beats_per_minute = *(myclock->getTempoP());
}

int main(int argc, char** argv)
{
	try
	{         
		// trace, debug, info, warn, error, critical
		//spdlog::level::level_enum LOGLEVEL = spdlog::level::debug;
		spdlog::level::level_enum LOGLEVEL = spdlog::level::info;
		
		//auto mainlog = spdlog::basic_logger_mt("main", "main.log");
		auto mainlog = spdlog::stdout_color_mt("main");
		mainlog->set_level(LOGLEVEL);
		mainlog->info("Albizzia version 0.1 pre-alpha");
	}
	catch (const spdlog::spdlog_ex& ex)
	{
		cout << "Log failed: " << ex.what() << endl;
	}

	auto mainlog= spdlog::get("main");	
	mainlog->info("----- init -----");
	
	myclock = new Clock();
	
	const char **ports;
	const char *client_name = "test";
	const char *server_name = NULL;
	jack_options_t joptions = JackSessionID;
	jack_status_t jstatus;
	jack_position_t zero;
	zero.valid = JackPositionBBT;
	zero.bar = 1;
	zero.beat = 1;
	zero.tick = 0;
	zero.beats_per_bar = 4;
	zero.beat_type = 4;
	zero.ticks_per_beat = 120;
	zero.beats_per_minute = 120;

	app = jack_client_open (client_name, joptions, &jstatus, server_name);
	
	bool ignoreKeyRepeat = false;
	bool resizable       = false;

	PuglView* view = puglInit(NULL, NULL);
	puglInitWindowSize(view, wWitdh, wHeight);
	puglInitResizable(view, resizable);
	puglInitContextType(view, PUGL_CAIRO_GL);

	puglIgnoreKeyRepeat(view, ignoreKeyRepeat);
	puglSetEventFunc(view, onEvent);

	puglCreateWindow(view, "jack-clock");
	puglShowWindow(view);

	jack_set_process_callback (app, process_callback, view);
	int jm = jack_set_timebase_callback (app, 0, timebase_callback, view);
	if ( jm ) mainlog->error("failed to register as JACK timebase master, error code {}", jm);
	jack_transport_stop (app);
	jack_transport_reposition (app, &zero); 		
	jack_activate (app);

	while (!quit) {
		puglWaitForEvent(view);
		puglProcessEvents(view);
	}

	puglDestroy(view);

	jack_transport_stop (app);
	jack_transport_reposition (app, &zero); 		
	jack_release_timebase(app);
	jack_deactivate (app);
	jack_client_close (app);
	
	return 0;
}

