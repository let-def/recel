all: main.c
	gcc -o main -lm main.c ../build/stb.o
	./main subset.png

watch:
	while inotifywait -e close ./main; do make && ./main subset.png; done
