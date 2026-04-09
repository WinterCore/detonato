CC = gcc
CFLAGS = -Ivendor -Ivendor/glad/include -Ivendor/libtess2/include $(shell pkg-config --cflags glfw3)
LDFLAGS = -lm -lGL $(shell pkg-config --libs glfw3)

SRC = $(wildcard src/*.c) vendor/glad/src/gl.c $(wildcard vendor/libtess2/src/*.c)

detonato: $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS) -o detonato

clean:
	rm -f detonato
