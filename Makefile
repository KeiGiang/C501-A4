
alt:
	g++ baseline.cpp -o baseline.out
	g++ FFT.cpp -o FFT.out

base:
	gcc baseline.cpp -lstdc++ -o baseline.out
	gcc FFT.cpp -lstdc++ -o FFT.out

clean:
	rm -rf *.out
	rm -rf *.txt
	rm -rf out.wav
