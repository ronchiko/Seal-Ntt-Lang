
CC = clang
CFLAGS = -g -fPIE -pie
LIB_FLAGS = -c -fPIC
INCLUDE_DIR = -I "include"
LIB_FILES = ntt.c ntte.c nttnum.c

all: lib clean

# Make using the libaray
demo-app:
	${CC} demo/main.c /home/ron/dev/Seal-Ntt-Lang/ntt-1.0.so ${CFLAGS} ${INCLUDE_DIR} -o ntt

# Compile the shared library
lib:
	${CC} ${LIB_FILES} ${LIB_FLAGS} ${INCLUDE_DIR}
	${CC} *.o -o ntt-1.0.so -shared -Wl

# Clean the object file
clean:
	rm *.o
