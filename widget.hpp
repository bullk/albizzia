/*
*/

#ifndef WIDGET_H
#define WIDGET_H

//#include <stdlib.h>
#include <cairo/cairo.h>

#define COLOR_BG GRAY(0.2)
#define COLOR_WBG GRAY(0.0)
#define COLOR_BOR GRAY(0.4)
#define COLOR_TEXT GRAY(0.8)
#define ORANGE cairo_set_source_rgb (cr, 1.0, 0.7, 0.0)
#define GREEN cairo_set_source_rgb (cr, 0.5, 1.0, 0.2)
#define GRAY(f) cairo_set_source_rgb (cr, f, f, f)

class Widget
{
public:
	Widget(int x, int y, int w, int h):
		x_(x), y_(y), w_(w), h_(h),
		mouse_pressed_(false), mouse_over_(false)
	{
		fprintf(stderr, "creating widget %d %d, %d %d \n", x_, y_, w_, h_);
	}
	~Widget()
	{}
	virtual void draw (cairo_t*)=0;
	bool touches (double x, double y)
	{
		return (x >= x_ && x <= x_ + w_ && y >= y_ && y <= y_ + h_);
	}
	bool pressed () { return mouse_pressed_; }
	virtual void press (unsigned int, unsigned int) { mouse_pressed_ = true; }
	virtual void release () { mouse_pressed_ = false; }
	virtual void highlight(double x, double y)
	{
		mouse_over_ = touches(x, y);
	}
protected:
	int x_, y_, w_, h_;
	bool mouse_pressed_;
	bool mouse_over_;
};


#endif
