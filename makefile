debug: clean
	mkdir bin/Debug
	g++ -Wall -g -o bin/Debug/aMAZEd SDL_main.cpp -std=c++11 -I/usr/include/SDL2 -lSDL2

clean:
	rm -rf bin/Debug
