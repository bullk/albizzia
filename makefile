SUFFIXES = .cpp .o
.SUFFIXES: $(SUFFIXES) .

all: my_pugl_cairo_test cairo_clip_test

my_pugl_cairo_test: my_pugl_cairo_test.c
	gcc -Wall -O1 -I/usr/local/include/pugl-0 -o my_pugl_cairo_test my_pugl_cairo_test.c -lm -lpugl-0 -lGL -lcairo

cairo_clip_test: audioclip.o cairo_clip_test.o
	g++ -std=c++0x -Wall -O1 -D__LITTLE_ENDIAN__ -I. -I/usr/local/include/pugl-0 -I/usr/include/stk -o cairo_clip_test cairo_clip_test.o audioclip.o -lm -lpugl-0 -lGL -lcairo -lstk

audioclip.o: audioclip.hpp widget.hpp
cairo_clip_test.o: audioclip.hpp

.cpp.o:
	g++ -std=c++0x -Wall -O1 -D__LITTLE_ENDIAN__ -I. -I/usr/local/include/pugl-0 -c $*.cpp

clean:
	rm *.o my_pugl_cairo_test cairo_clip_test
