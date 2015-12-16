VPATH = ../lua/etc

INCLUDEPATHS = \
-I../lua/src \
-I../libgvfs

objfiles = all_lua.o

CXXFLAGS = -Og -g -D_REENTRANT $(INCLUDEPATHS)

links =

%.o : %.cpp
	g++ $(CXXFLAGS) -c $<

%.o : %.c
	gcc $(CXXFLAGS) -c $<

lua.so: $(objfiles) $(links)
	g++ -o lua.so -shared $(objfiles) $(links)

-include liblua.dep

.PHONY : depend
depend:
	g++ $(INCLUDEPATHS) -MM ../lua/etc/*.cpp > liblua.dep
