/*
*/

//#include <stdlib.h>
#include <math.h>
#include <cairo/cairo.h>
#include "audioclip.hpp"


const float ZOOM_SPEED = 0.02f;

AudioClip::AudioClip (cairo_t* cr, int x, int y, int w, int h):
	Widget(cairo_t* cr, int x, int y, int w, int h)
{
	data_.resize(44100,1,0);
	float freq = 13;
	float twopi = 2 * M_PI;
	for (unsigned int i=0; i<data_.size(); i++) {
		float x = i*twopi*freq/data_.size();
		float y = 0;
		y += sin(x);
		y += sin(3*x)/3;
		y += sin(5*x)/5;
		y += sin(7*x)/7;
		data_[i] = y;
	}
	left = 0;
	right = data_.size();
	xpress_ = 0;
	ypress_ = 0;
}

AudioClip::AudioClip(cairo_t* cr, int x, int y, int w, int h, std::string fileName):
	Widget(cairo_t* cr, int x, int y, int w, int h)
{
	stk::FileWvIn wavfile = stk::FileWvIn(fileName);
	unsigned int s = 1;
	if (wavfile.channelsOut() > 0) {
		s = wavfile.getSize();
	}
	data_ = stk::StkFrames(s, wavfile.channelsOut());
	wavfile.tick(data_);
	left = 0;
	right = data_.size();
	xpress_ = 0;
	ypress_ = 0;
	fprintf(stderr, "loaded %s size %u\n", fileName.c_str(), data_.size());
}

AudioClip::~AudioClip ()
{
}

void
AudioClip::draw()
{
	int ww = right - left;
	const float bottom = (float)(y_ + h_);
	const float xscale = (float)ww / w_;
	const float yscale = h_/2;
	//fprintf(stderr, "xscale %f\n", xscale);
	cairo_set_source_rgb (cr_, 0.2, 0.2, 0.2);
	cairo_rectangle (cr_, x_, y_, w_, h_);
	cairo_fill_preserve (cr_);
	cairo_set_line_width (cr_, 1);
	cairo_set_source_rgb (cr_, 0.4, 0.4, 0.4);
	cairo_stroke (cr_);
	cairo_set_source_rgb (cr_, 1.0, 0.7, 0.0);
	float value = data_[left];
	cairo_move_to (cr_, x_, bottom - (value+1) * yscale);
	int width = int(xscale/2);
	if (xscale > 3)
		for (int i=1; i<w_; i++) {
			unsigned int j = (unsigned int)(i * xscale) + left;
			float min = 1.0, max = -1.0;
			for (unsigned int k=j-width; k<=j+width; k++)
				if ((k >= 0) & (k < data_.size())) {
					min = fmin(min, data_[k]);
					max = fmax(max, data_[k]);
				}
			cairo_line_to (cr_, x_ + i, bottom - (max+1) * yscale);
			cairo_line_to (cr_, x_ + i, bottom - (min+1) * yscale);
		}
	else {
		for (int i=1; i<w_; i++) {
			unsigned int j = (unsigned int)(i * xscale) + left;
			cairo_line_to (cr_, x_ + i, bottom - (data_[j]+1) * yscale);
		}
	}
	cairo_stroke (cr_);
}

void
AudioClip::press(unsigned int ox, unsigned int oy)
{
	ui_state_ = true;
	xpress_ = ox;
	ypress_ = oy;
}

void
AudioClip::zoom(double mx, double my)
{
	int limit = (int)data_.size()-1;
	int ww = right - left;
	int ydelta = (int)(ZOOM_SPEED * ww * (ypress_ - my));
	int xdelta = (int)(ZOOM_SPEED * ww * (mx - xpress_));
	//fprintf(stderr, "xpress %d mx %f ypress %d my %f\n",
		//xpress_, mx, ypress_, my);
	//fprintf(stderr, "Zoom %d -> %d, ww %d xdelta %d ydelta %d\n",
		//left, right, ww, xdelta, ydelta);
		
	left += ydelta;
	if (left < 0) left = 0;
	right -= ydelta;
	if (right > limit) right = limit;
	while (right - left < 10) {
		if (left > 0) left--;
		if (right < limit) right++;
	}	
	if (xdelta < 0)	if (-xdelta > left) xdelta = -left;
	if (xdelta > 0) if (xdelta > limit-right) xdelta = limit-right;
	left += xdelta;
	right += xdelta;
	
	xpress_ = mx;
	ypress_ = my;
}

