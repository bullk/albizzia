/*
*/

//#include <stdlib.h>
#include <cairo/cairo.h>

class Widget
{
public:
	Widget(cairo_t* cr, int x, int y, int w, int h):
		cr_(cr), x_(x), y_(y), w_(w), h_(h)
	{
		ui_state_ = false;
	}
	~Widget();
	virtual void draw() = 0;
	bool touches(double x, double y);
	bool pressed() { return ui_state_; }
	virtual void press(unsigned int, unsigned int) { ui_state_ = true; }
	virtual void release() { ui_state_ = false; }
protected:
	cairo_t* cr_;
	int x_, y_, w_, h_;
	bool ui_state_;
};

bool Widget::touches(double x, double y)
{
	return (x >= x_ && x <= x_ + w_ && y >= y_ && y <= y_ + h_);
}
