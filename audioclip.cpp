/*
*/

//#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <cairo/cairo.h>
#include "audioclip.hpp"


const float ZOOM_SPEED = 0.02f;

AudioClip::AudioClip (int x, int y, int w, int h):
	Widget(x, y, w, h)
{
	reinitZoom();
}

AudioClip::AudioClip(int x, int y, int w, int h, std::string fileName):
	Widget(x, y, w, h)
{
	loadFile(fileName);
}

AudioClip::~AudioClip ()
{
}

void AudioClip::reinitZoom()
{
	xpress_ = 0;
	ypress_ = 0;
	left = 0;
	right = data_.size();
}

void AudioClip::testWave()
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
	reinitZoom();
}

void AudioClip::loadFile(std::string fileName)
{
	stk::FileWvIn wavfile = stk::FileWvIn(fileName);
	unsigned int s = 1;
	if (wavfile.channelsOut() > 0) {
		s = wavfile.getSize();
	}
	data_ = stk::StkFrames(s, wavfile.channelsOut());
	wavfile.tick(data_);
	imgfn_ = "." + fileName + ".png";
	int fd = open(imgfn_.c_str(), O_CREAT | O_WRONLY | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd < 0) {
	  if (errno == EEXIST) {
	  }
	} else {
		cacheImage ();
	}
	fprintf(stderr, "loaded %s size %u\n", fileName.c_str(), data_.size());
	reinitZoom();
}

void AudioClip::cacheImage ()
{
    cairo_surface_t *surface;
    cairo_t *cr;
    int imw = data_.size() / 1000;
    int imh = 200;

    surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, imw, imh);
    cr = cairo_create (surface);

	drawData(cr, 0, 0, imw);

    cairo_surface_write_to_png (surface, imgfn_.c_str());
    cairo_destroy (cr);
    cairo_surface_destroy (surface);
}

void AudioClip::drawData(cairo_t* cr, int ox, int oy, int imw)
{
	const float bottom = (float)(oy + h_);
	const float xscale = (float)(right-left) / imw;
	const float yscale = h_/2;
	ORANGE;
	cairo_set_line_width (cr, 1);
	float value = data_[left];
	cairo_move_to (cr, ox, bottom - (value+1) * yscale);
	int width = int(xscale/2);
	if (xscale > 500)
		for (int i=1; i<imw; i++) {
			unsigned int j = (unsigned int)(i * xscale) + left;
			float min = 1.0, max = -1.0;
			for (unsigned int k=j-width; k<=j+width; k++)
				if ((k >= 0) & (k < data_.size())) {
					min = fmin(min, data_[k]);
					max = fmax(max, data_[k]);
				}
			cairo_line_to (cr, ox + i, bottom - (max+1) * yscale);
			cairo_line_to (cr, ox + i, bottom - (min+1) * yscale);
		}
	else {
		for (int i=1; i<imw; i++) {
			unsigned int j = (unsigned int)(i * xscale) + left;
			cairo_line_to (cr, ox + i, bottom - (data_[j]+1) * yscale);
		}
	}
	cairo_stroke (cr);
}

void AudioClip::draw(cairo_t* cr)
{
	COLOR_WBG;
	cairo_rectangle (cr, x_, y_, w_, h_);
	cairo_fill_preserve (cr);
	cairo_set_line_width (cr, 1);
	COLOR_BOR;
	cairo_stroke (cr);
	drawData(cr, x_, y_, w_);
}




void
AudioClip::press(unsigned int ox, unsigned int oy)
{
	mouse_pressed_ = true;
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

