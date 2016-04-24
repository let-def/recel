%.o: %.c Makefile
	gcc -O3 -c $<

recel: main.o recel_distance.o recel_iter.o stb.o
	gcc -o $@ -lm $^
