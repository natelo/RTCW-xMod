CC = gcc
CFLAGS = -pipe -fsigned-char -DNDEBUG -O6 -mcpu=pentiumpro -march=pentium \
-fomit-frame-pointer -ffast-math -malign-loops=2 -malign-jumps=2 \
-malign-functions=2 -fno-strict-aliasing -fstrength-reduce -fPIC -DGAMEDLL

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
