
alt:
	g++ convolve.cpp -o convolve.out

base:
	gcc convolve.cpp -lstdc++ -o convolve.out

clean:
	rm -rf *.out