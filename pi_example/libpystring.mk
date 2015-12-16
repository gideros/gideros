VPATH = ../libpystring

INCLUDEPATHS = -I../libpystring

objfiles = pystring.o

CXXFLAGS = -Og -g -D_REENTRANT -DPYSTRING_LIBRARY $(INCLUDEPATHS)

links =

%.o : %.cpp
	g++ $(CXXFLAGS) -c $<

%.o : %.c
	gcc $(CXXFLAGS) -c $<

pystring.so: $(objfiles) $(links)
	g++ -o pystring.so -shared $(objfiles) $(links)

-include libpystring.dep

.PHONY : depend
depend:
	g++ $(INCLUDEPATHS) -MM ../libpystring/*.cpp > libpystring.dep
