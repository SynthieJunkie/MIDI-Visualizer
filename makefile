build:
	rm -f Main
	sleep 0.5
	g++ Main.cpp -o Main -lSDL2 -lrtmidi -O3
	./Main
