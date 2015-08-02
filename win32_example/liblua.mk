VPATH = ..\lua\etc

INCLUDEPATHS = \
-I..\lua\src \
-I..\libgvfs

objfiles = all_lua.o

CXXFLAGS = -O2 -DLUA_BUILD_AS_DLL $(INCLUDEPATHS)

links = libgvfs.dll

%.o : %.cpp
	g++ $(CXXFLAGS) -c $<

%.o : %.c
	gcc $(CXXFLAGS) -c $<

liblua.dll: $(objfiles) $(links)
	g++ -o liblua.dll -shared $(objfiles) $(links)

-include liblua.dep

.PHONY : depend
depend:
	g++ $(INCLUDEPATHS) -MM ../lua/etc/*.cpp > liblua.dep
