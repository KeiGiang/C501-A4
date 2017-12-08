
alt:
	g++ baseline.cpp -o baseline.out

base:
	gcc baseline.cpp -lstdc++ -o baseline.out

clean:
	rm -rf *.out
	rm -rf *.txt
	rm -rf out.wav
