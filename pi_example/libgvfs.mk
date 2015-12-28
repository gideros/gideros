VPATH = \
../libgvfs

INCLUDEPATHS = \
-I../libgvfs \
-I../libgvfs/private \

objfiles = \
    wsetup.o \
    wbuf.o \
    vfscanf.o \
    vfprintf.o \
    ungetc.o \
    tmpfile.o \
    stdio.o \
    setvbuf.o \
    rget.o \
    refill.o \
    putc.o \
    makebuf.o \
    getc.o \
    fwrite.o \
    fwalk.o \
    fvwrite.o \
    ftell.o \
    fseek.o \
    fscanf.o \
    freopen.o \
    fread.o \
    fputs.o \
    fputc.o \
    fprintf.o \
    fopen.o \
    flockfile.o \
    flags.o \
    findfp.o \
    fileops.o \
    fgets.o \
    fgetc.o \
    fflush.o \
    ferror.o \
    feof.o \
    fclose.o \
    extra.o \
    clrerr.o \
    gfile.o \
    gpath.o

CXXFLAGS = -Og -g -D_REENTRANT -DGIDEROS_LIBRARY -DSTRICT_LINUX -std=gnu++0x -fPIC $(INCLUDEPATHS)
  CFLAGS = -Og -g -D_REENTRANT -DGIDEROS_LIBRARY -DSTRICT_LINUX -fPIC $(INCLUDEPATHS)

links = -lpthread

%.o : %.cpp
	g++ $(CXXFLAGS) -c $<

%.o : %.c
	gcc $(CFLAGS) -c $<

gvfs.so: $(objfiles) $(links)
	g++ -o gvfs.so -shared $(objfiles) $(links)

-include libgvfs.dep

.PHONY : depend
depend:
	g++ $(INCLUDEPATHS) -MM ../libgvfs/*.cpp ../libgvfs/*.c > libgvfs.dep

.PHONY : clean
clean:
	rm $(objfiles)
