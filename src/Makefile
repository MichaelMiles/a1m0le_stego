CC = gcc
CFLAGS = -Wall -g -std=c11
PROGS = a1m0le_stego

all: $(PROGS)

a1m0le_stego: a1m0le_stego.o load_image.o
	$(CC) $(CFLAGS) -o $@ $^ -lm
	mv a1m0le_stego ../build/

a1m0le_stego.o: a1m0le_stego.c image.h
	$(CC) $(CFLAGS) -c $<

load_image.o: load_image.c image.h stb_image_write.h stb_image.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm ../build/$(PROGS) *.o
