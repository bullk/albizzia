/*
*/

#ifndef AUDIOCLIP_H
#define AUDIOCLIP_H

//#include <stdlib.h>
#include <string>
#include <stk/FileWvIn.h>
#include "widget.hpp"

class AudioClip : public Widget
{
public:
	AudioClip(int x, int y, int w, int h);
	AudioClip(int x, int y, int w, int h, std::string fileName);
	~AudioClip();
	void reinitZoom();
	void testWave();
	void loadFile(std::string);
	void cacheImage();
	void drawData(cairo_t*, int, int, int);
	void draw(cairo_t* cr);
	void press(unsigned int, unsigned int);
	void zoom(double, double);
protected:
	stk::StkFrames data_;
	std::string imgfn_;
	int left, right;
	int xpress_, ypress_;
};

#endif
