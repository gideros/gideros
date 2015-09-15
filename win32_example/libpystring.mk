VPATH = \
..\libpystring

INCLUDEPATHS = -I..\libpystring

objfiles = pystring.o

CXXFLAGS = -O2 -DPYSTRING_LIBRARY $(INCLUDEPATHS)

links =

%.o : %.cpp
	g++ $(CXXFLAGS) -c $<

%.o : %.c
	gcc $(CXXFLAGS) -c $<

pystring.dll: $(objfiles) $(links)
	g++ -o pystring.dll -shared $(objfiles) $(links)

-include libpystring.dep

.PHONY : depend
depend:
	g++ $(INCLUDEPATHS) -MM ../libpystring/*.cpp > libpystring.dep
