%.o: %.c Makefile recel.h
	gcc -ggdb -O2 -c $<

recel: main.o recel_distance.o recel_trace.o recel_fill.o stb.o
	gcc -o $@ -lm $^

clean:
	rm -rf recel *.o 
