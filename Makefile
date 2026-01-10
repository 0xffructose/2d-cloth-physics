all: build

build: main.c
	gcc main.c -o program.exe -lraylib -lopengl32 -lwinmm