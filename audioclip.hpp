/*
*/

//#include <stdlib.h>
#include <string>
#include <stk/FileWvIn.h>
#include "widget.hpp"

class AudioClip : public Widget
{
public:
	AudioClip(cairo_t* cr, int x, int y, int w, int h);
	AudioClip(cairo_t* cr, int x, int y, int w, int h, std::string fileName);
	~AudioClip();
	void draw();
	void press(unsigned int, unsigned int);
	void zoom(double, double);
protected:
	stk::StkFrames data_;
	int left, right;
	int xpress_, ypress_;
};

