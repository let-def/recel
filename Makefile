%.o: %.c Makefile
	gcc -ggdb -O2 -c $<

recel: main.o recel_distance.o recel_scan.o stb.o fasttable.o
	gcc -o $@ -lm $^

clean:
	rm -rf recel *.o 
