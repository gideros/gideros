VPATH = ../libpystring

INCLUDEPATHS = -I../libpystring

objfiles = pystring.o

  CFLAGS = -Og -g -D_REENTRANT -DPYSTRING_LIBRARY -fPIC $(INCLUDEPATHS)
CXXFLAGS = -Og -g -D_REENTRANT -DPYSTRING_LIBRARY -std=gnu++0x -fPIC $(INCLUDEPATHS)

links =

%.o : %.cpp
	g++ $(CXXFLAGS) -c $<

%.o : %.c
	gcc $(CFLAGS) -c $<

pystring.so: $(objfiles) $(links)
	g++ -o pystring.so -shared $(objfiles) $(links)

-include libpystring.dep

.PHONY : depend
depend:
	g++ $(INCLUDEPATHS) -MM ../libpystring/*.cpp > libpystring.dep
