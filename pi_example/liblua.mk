VPATH = ../lua/etc

INCLUDEPATHS = \
-I../lua/src \
-I../libgvfs

objfiles = all_lua.o

CXXFLAGS = -Og -g -D_REENTRANT -fPIC -std=gnu++0x $(INCLUDEPATHS)
  CFLAGS = -Og -g -D_REENTRANT -fPIC $(INCLUDEPATHS)

links = gvfs.so

%.o : %.cpp
	g++ $(CXXFLAGS) -c $<

%.o : %.c
	gcc $(CFLAGS) -c $<

lua.so: $(objfiles) $(links)
	g++ -o lua.so -shared $(objfiles) $(links)

-include liblua.dep

.PHONY : depend
depend:
	g++ $(INCLUDEPATHS) -MM ../lua/etc/*.c > liblua.dep
