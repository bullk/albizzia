SUFFIXES = .cpp .o
.SUFFIXES: $(SUFFIXES) .

all: splash-bullk my_pugl_cairo_test cairo_clip_test jack-clock jack-client

jack-client: jack-client.cpp
	g++ -std=c++0x -Wall -O1 -o jack-client jack-client.cpp -ljack -lpthread

my_pugl_cairo_test: my_pugl_cairo_test.c
	gcc -Wall -O1 -I/usr/local/include/pugl-0 -o my_pugl_cairo_test my_pugl_cairo_test.c -lm -lpugl-0 -lGL -lcairo

cairo_clip_test: audioclip.o cairo_clip_test.o
	g++ -std=c++0x -Wall -O1 -D__LITTLE_ENDIAN__ -I. -I/usr/local/include/pugl-0 -I/usr/include/stk -o cairo_clip_test cairo_clip_test.o audioclip.o -lm -lpugl-0 -lGL -lcairo -lstk

splash-bullk: splash-bullk.cpp
	g++ -std=c++0x -Wall -O1 -D__LITTLE_ENDIAN__ -I. -I/usr/local/include/pugl-0 -o splash-bullk splash-bullk.cpp -lm -lpugl-0 -lGL -lcairo

jack-clock: jack-clock.cpp Clock.o
#	g++ -std=c++0x -Wall -O1 -D__LITTLE_ENDIAN__ -I. -o jack-clock jack-clock.cpp pugl/pugl_x11.c Clock.o -lcairo -ljack -lpthread
	g++ -std=c++0x -Wall -O1 -D__LITTLE_ENDIAN__ -I. -I/usr/local/include/pugl-0 -o jack-clock jack-clock.cpp Clock.o -lpugl-0 -lcairo -ljack -lpthread

audioclip.o: audioclip.hpp widget.hpp
cairo_clip_test.o: audioclip.hpp

.cpp.o:
	g++ -g -std=c++0x -Wall -O1 -D__LITTLE_ENDIAN__ -I. -I/usr/local/include/pugl-0 -c $*.cpp

clean:
	rm *.o my_pugl_cairo_test cairo_clip_test
