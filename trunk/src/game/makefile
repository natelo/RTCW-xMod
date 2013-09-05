CC = gcc
CFLAGS = -Wall -fno-strict-aliasing -Wimplicit -Wstrict-prototypes \
-pipe -O3 -march=i586 -fomit-frame-pointer \
-funroll-loops -ffast-math -falign-loops=2 -falign-jumps=2 \
-falign-functions=2 -fstrength-reduce -fPIC -DGAMEDLL -DNDEBUG

SOURCES = $(shell ls g_*.c q_*.c ai_*.c bg_animation.c bg_misc.c bg_pmove.c bg_slidemove.c ../botai/*.c)
MODULES = $(SOURCES:.c=.o)

all: gameclean qagame.mp.i386.so

gameclean:
	rm -rf *.o ../botai/*.o

qagame.mp.i386.so: $(MODULES)
	$(CC) -shared -ldl -lm -o $@ $?

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *.so ../botai/*.o
