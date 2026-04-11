CC = gcc
CFLAGS = -g -O0 -Wall -Wextra -Ivendor -Ivendor/glad/include -Ivendor/libtess2/include $(shell pkg-config --cflags glfw3)
LDFLAGS = -lm -lGL $(shell pkg-config --libs glfw3)

LIBTESS_SRC = $(wildcard vendor/libtess2/src/*.c)

DETONATO_SRC = src/main.c src/gl_helpers.c src/render.c src/aids.c vendor/glad/src/gl.c src/style.c
PREPROCESS_SRC = src/preprocess.c src/gl_helpers.c src/render.c src/aids.c vendor/glad/src/gl.c src/style.c $(LIBTESS_SRC)

all: detonato preprocess

detonato: $(DETONATO_SRC)
	$(CC) $(CFLAGS) $(DETONATO_SRC) $(LDFLAGS) -o detonato

preprocess: $(PREPROCESS_SRC)
	$(CC) $(CFLAGS) $(PREPROCESS_SRC) $(LDFLAGS) -o preprocess

clean:
	rm -f detonato preprocess
