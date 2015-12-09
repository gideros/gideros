VPATH = \
../libgvfs

INCLUDEPATHS = \
-I../libgvfs \
-I../libgvfs/private \
#-I../libgid/external/pthreads-w32-2-9-1-release/Pre-built.2/include

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

CXXFLAGS = -O2 -DGIDEROS_LIBRARY -DSTRICT_LINUX $(INCLUDEPATHS)

links = -lpthread
#links = ..\libgid\external\pthreads-w32-2-9-1-release\Pre-built.2\lib\x86\libpthreadGC2.a

%.o : %.cpp
	g++ $(CXXFLAGS) -c $<

%.o : %.c
	gcc $(CXXFLAGS) -c $<

gvfs.so: $(objfiles) $(links)
	g++ -o gvfs.so -shared $(objfiles) $(links)

-include libgvfs.dep

.PHONY : depend
depend:
	g++ $(INCLUDEPATHS) -MM ../libgvfs/*.cpp ../libgvfs/*.c > libgvfs.dep
