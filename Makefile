OBJECTS=main.o recel_distance.o recel_scan.o stb.o fasttable.o

all: build/recel

build/%.o: %.c Makefile
	gcc -ggdb -O2 -o $@ -c $<

build/recel: $(patsubst %.o,build/%.o, $(OBJECTS))
	gcc -o $@ -lm $^

clean:
	rm -rf build/*

build/:
	mkdir $@

.PHONY: all clean
