CFLAGS=-std=c99 -O3 -Wno-format -march=native -funroll-loops -fomit-frame-pointer

ifdef MPAR
	CFLAGS += -DMPAR=$(MPAR)
endif
	
all:
	$(CC) $(CFLAGS) helpers.c haraka.c -o haraka

clean:
	rm haraka