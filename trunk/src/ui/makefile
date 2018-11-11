CC = gcc
CFLAGS = -pipe -fsigned-char -DNDEBUG -O6 -mcpu=pentiumpro -march=pentium -m32 \
-fomit-frame-pointer -ffast-math -malign-loops=2 -malign-jumps=2 \
-malign-functions=2 -fno-strict-aliasing -fstrength-reduce -fPIC

SOURCES = $(shell ls ui_*.c ../game/bg_misc.c ../game/q_math.c ../game/q_shared.c)
MODULES = $(SOURCES:.c=.o)

all: gameclean ui.mp.i386.so

gameclean:
	rm -rf ../game/*.o

ui.mp.i386.so: $(MODULES)
	$(CC) -shared -ldl -lm -o $@ $?

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *.so
