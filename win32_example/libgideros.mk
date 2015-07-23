VPATH = \
..\lua\src;\
..\libpystring

INCLUDEPATHS = \
-I..\lua\src \
-I..\libpystring

objfiles = binderutil.o stringid.o eventdispatcher.o \
event.o refptr.o eventvisitor.o pluginmanager.o luautil.o

CXXFLAGS = -O2 -DGIDEROS_LIBRARY $(INCLUDEPATHS)

links = libgid.dll liblua.dll libpystring.dll

%.o : %.cpp
	g++ $(CXXFLAGS) -c $<

%.o : %.c
	g++ $(CXXFLAGS) -c $<

libgideros.dll: $(objfiles) $(links)
	g++ -o libgideros.dll -shared $(objfiles) $(links)

-include libgideros.dep

.PHONY : depend
depend:
	g++ $(INCLUDEPATHS) -MM ../libgideros/*.cpp > libgideros.dep
