%.o: %.c Makefile recel.h
	gcc -ggdb -O2 -c $<

recel: main.o recel_distance.o stb.o fasttable.o
	gcc -o $@ -lm $^

clean:
	rm -rf recel *.o 
