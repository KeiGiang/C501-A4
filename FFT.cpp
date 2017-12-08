#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <math.h>

using namespace std;
#define SWAP(a,b)  tempr=(a);(a)=(b);(b)=tempr
// using std::cout;
// using std::endl;

typedef struct  WAV_HEADER{
    unsigned char   ChunkID[4];     // RIFF
    unsigned int    ChunkSize;      // RIFF Chunk Size
    unsigned char   Format[4];      // FMT header

    unsigned char   Subchunk1ID[4]; // Size of the fmt chunk
    unsigned int    Subchunk1Size;  // Size of the fmt chunk
    unsigned short  AudioFormat;    // Audio format 1=PCM,6=mulaw,7=alaw, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM
    unsigned short  numChannel;     // Number of channels 1=Mono 2=Sterio
    unsigned int    sampleRate;   // Sampling Frequency in Hz
    unsigned int    byteRate;    // bytes per second
    unsigned short  blockAlign;     // 2=16-bit mono, 4=16-bit stereo
    unsigned short  bitsPerSample;  // Number of bits per sample

    unsigned char   Subchunk2ID[4]; // "data"  string
    unsigned long   Subchunk2Size;  // Sampled data length

}wav_hdr;

bool ends_with(string const & value, string const & ending)
{
    if (ending.size() > value.size())
        return false;

    return equal(ending.rbegin(), ending.rend(), value.rbegin());
}

// four1 is from Professor Manzara's handout
void four1(double data[], int nn, int isign)
{
    unsigned long n, mmax, m, j, istep, i;
    double wtemp, wr, wpr, wpi, wi, theta;
    double tempr, tempi;

    n = nn << 1;
    j = 1;

    for (i = 1; i < n; i += 2)
    {
    	if (j > i)
        {
    	    SWAP(data[j], data[i]);
    	    SWAP(data[j+1], data[i+1]);
    	}
    	m = nn;
    	while (m >= 2 && j > m)
        {
    	    j -= m;
    	    m >>= 1;
        }
        j += m;
    }

    mmax = 2;
    while (n > mmax)
    {
    	istep = mmax << 1;
    	theta = isign * (6.28318530717959 / mmax);
    	wtemp = sin(0.5 * theta);
    	wpr = -2.0 * wtemp * wtemp;
    	wpi = sin(theta);
    	wr = 1.0;
    	wi = 0.0;
    	for (m = 1; m < mmax; m += 2)
        {
    	    for (i = m; i <= n; i += istep)
            {
        		j = i + mmax;
        		tempr = wr * data[j] - wi * data[j+1];
        		tempi = wr * data[j+1] + wi * data[j];
        		data[j] = data[i] - tempr;
        		data[j+1] = data[i+1] - tempi;
        		data[i] += tempr;
        		data[i+1] += tempi;
    	    }
    	    wr = (wtemp = wr) * wpr - wi * wpi + wr;
    	    wi = wi * wpr + wtemp * wpi + wi;
    	}
    	mmax = istep;
    }
}

// Algorithm taken from Professor Manzara's handout
// x[] = signal to be convolved
// N = x.numSamples()
// h[] = impulse response signal
// M = h.numSamples()
// y[] = output signal
// P = y.numSamples(), must =  N + M - 1
void convolve(float x[], int N, float h[], int M, float y[], int P)
{
    printf("Convolving...");
	int size = 2;
	int longest;

	if (N > M)
		longest=N;
	else
		longest=M;

	while (size < longest)
		size *= 2;

    size *= 2;
	double * array1 = new double[size];
	double * array2 = new double[size];
	double * finalArray = new double[size];

    for (int i = 0; i < size; i++)
    {
        array1[i] = 0;
        array2[i] = 0;
        finalArray[i] = 0;
    }

	for (int i = 0; i < N; i++)
		array1[i*2] = x[i];

    // printf("Fuck\n");
	for (int i = 0; i < M; i++)
		array2[i*2] = h[i];

	four1(array1-1, (size / 2), 1);
	four1(array2-1, (size / 2), 1);

    for (int i = 0; i < size; i += 2)
    {
        finalArray[i] = (array1[i] * array2[i]) - (array1[i+1] * array2[i+1]);
        finalArray[i+1] = (array1[i+1] * array2[i]) + (array1[i] * array2[i+1]);
    }

    four1((finalArray - 1), (size / 2), -1);

    float max;
    for (int i = 0; i < P; i++)
    {
        y[i] = finalArray[i*2];

        if (abs(y[i]) > max){
            max = y[i];
        }
    }

    for (int i = 0; i < P; i++)
    {
        y[i] /= max;
        if(y[i] < -1.0){
            y[i] = (-1.0);
        }
    }

}

unsigned char buffer4[4];
unsigned char buffer2[2];
wav_hdr wavHeader;

float* readFile(char* toRead, int *wavSize)
{

    FILE *inputFile;
    inputFile = fopen(toRead, "r" );

    if (inputFile == NULL)
    {
        exit(-1);
    }

    fread(wavHeader.ChunkID, sizeof(wavHeader.ChunkID), 1, inputFile);
    fread(buffer4, sizeof(buffer4), 1, inputFile);
    wavHeader.ChunkSize = buffer4[0] | (buffer4[1] << 8) | (buffer4[2] << 16) | (buffer4[3] << 24);
    fread(wavHeader.Format, sizeof(wavHeader.Format), 1, inputFile);
    fread(wavHeader.Subchunk1ID, sizeof(wavHeader.Subchunk1ID), 1, inputFile);
    fread(buffer4, sizeof(buffer4), 1, inputFile);
    wavHeader.Subchunk1Size = buffer4[0] | (buffer4[1] << 8) | (buffer4[2] << 16) | (buffer4[3] << 24);
    fread(buffer2, sizeof(buffer2), 1, inputFile);
    wavHeader.AudioFormat = buffer2[0] | (buffer2[1] << 8);
    char format_name[10] = "";
    if (wavHeader.AudioFormat == 1)
        strcpy(format_name, "PCM");
    fread(buffer2, sizeof(buffer2), 1, inputFile);
    wavHeader.numChannel = buffer2[0] | (buffer2[1] << 8);
    fread(buffer4, sizeof(buffer4), 1, inputFile);
    wavHeader.sampleRate = buffer4[0] | (buffer4[1] << 8) | (buffer4[2] << 16) | (buffer4[3] << 24);
    fread(buffer4, sizeof(buffer4), 1, inputFile);
    wavHeader.byteRate = buffer4[0] | (buffer4[1] << 8) | (buffer4[2] << 16) | (buffer4[3] << 24);
    fread(buffer2, sizeof(buffer2), 1, inputFile);
    wavHeader.blockAlign = buffer2[0] | (buffer2[1] << 8);
    fread(buffer2, sizeof(buffer2), 1, inputFile);
    wavHeader.bitsPerSample = buffer2[0] | (buffer2[1] << 8);

    if (wavHeader.Subchunk1Size == 18)
    {
        fread(buffer2, sizeof(buffer2), 1, inputFile);
        fread(wavHeader.Subchunk2ID, sizeof(wavHeader.Subchunk2ID), 1, inputFile);
    }
    else
        fread(wavHeader.Subchunk2ID, sizeof(wavHeader.Subchunk2ID), 1, inputFile);

    fread(buffer4, sizeof(buffer4), 1, inputFile);
    wavHeader.Subchunk2Size = buffer4[0] | (buffer4[1] << 8) | (buffer4[2] << 16) | (buffer4[3] << 24);

    // printf("ChunkID %s\n", wavHeader.ChunkID);
    // printf("ChunkSize: bytes: %u, kb:%u, \n", wavHeader.ChunkSize, wavHeader.ChunkSize/1024);
    // printf("Format: %s\n", wavHeader.Format);
    // printf("Subchunk1ID: %s\n", wavHeader.Subchunk1ID);
    // printf("Subchunk1Size: %u \n", wavHeader.Subchunk1Size);
    // printf("Format type: %u %s\n", wavHeader.AudioFormat, format_name);
    // printf("Channels: %u \n", wavHeader.numChannel);
    // printf("Sample Rate: %u\n", wavHeader.sampleRate);
    // printf("Byte Rate: %u , Bit Rate: %u\n", wavHeader.byteRate, wavHeader.byteRate*8);
    // printf("BlockAlign: %u \n", wavHeader.blockAlign);
    // printf("BitsPerSample: %u \n", wavHeader.bitsPerSample);
    // printf("Subchunk2ID: %s \n", wavHeader.Subchunk2ID );
    // printf("Subchunk2Size: %u\n", wavHeader.Subchunk2Size);
    // printf("---------------------\n");

    *wavSize = wavHeader.Subchunk2Size/2;

    short * filedata = new short[*wavSize];
    for (int i = 0; i < *wavSize; i++)
    {
        fread((char *) &filedata[i], 2, 1, inputFile);
        // printf("%d\n", filedata[i]);
    }

    short sample;
    float * wavSignal = new float[*wavSize];
    for (int i = 0; i < *wavSize; i++)
    {
        sample = filedata[i];
        wavSignal[i] = sample / 32767.0;
        if (wavSignal[i] < -1.0)
        {
            wavSignal[i] = -1.0;
        }
        // printf("%f\n", wavSignal[i]);
    }

    if(fclose(inputFile) != 0)
    {
        printf("Error closing input file\n");
        exit(-1);
    }

    return wavSignal;
}

void writeFile(char* toOpen, float data[], int length)
{

    FILE *outputFile;
    outputFile = fopen(toOpen, "wb" );

    if (outputFile == NULL)
    {
        exit(-1);
    }

    // setup header
    unsigned char   ChunkID[] = "RIFF";
    unsigned int    ChunkSize = 36 + length;
    unsigned char   Format[] = "WAVE";

    unsigned char   Subchunk1ID[] = "fmt ";
    unsigned int    Subchunk1Size = 16;
    unsigned short  AudioFormat = wavHeader.AudioFormat;
    unsigned short  numChannel = wavHeader.numChannel;
    unsigned int    sampleRate = wavHeader.sampleRate;
    unsigned int    byteRate = wavHeader.byteRate;
    unsigned short  blockAlign = wavHeader.blockAlign;
    unsigned short  bitsPerSample = wavHeader.bitsPerSample;

    unsigned char   Subchunk2ID[] = "data";
    unsigned long   Subchunk2Size = length * 2;

    // printf("ChunkID %s\n", ChunkID);
    // printf("ChunkSize: bytes: %u, kb:%u, \n", ChunkSize, ChunkSize/1024);
    // printf("Format: %s\n", Format);
    // printf("Subchunk1ID: %s\n", Subchunk1ID);
    // printf("Subchunk1Size: %u \n", Subchunk1Size);
    // printf("Format type: %u \n", AudioFormat);
    // printf("Channels: %u \n", numChannel);
    // printf("Sample Rate: %u\n", sampleRate);
    // printf("Byte Rate: %u , Bit Rate: %u\n", byteRate, byteRate*8);
    // printf("BlockAlign: %u \n", blockAlign);
    // printf("BitsPerSample: %u \n", bitsPerSample);
    // printf("Subchunk2ID: %s \n", Subchunk2ID );
    // printf("Subchunk2Size: %u\n", Subchunk2Size);

    fwrite(ChunkID, sizeof(char), 4, outputFile);
    fwrite(&ChunkSize, sizeof(int), 1, outputFile);
    fwrite(Format, sizeof(char), 4, outputFile);

    fwrite(Subchunk1ID, sizeof(char), 4, outputFile);
    fwrite(&Subchunk1Size, sizeof(int), 1, outputFile);
    fwrite(&AudioFormat, sizeof(short), 1, outputFile);
    fwrite(&numChannel, sizeof(short), 1, outputFile);
    fwrite(&sampleRate, sizeof(int), 1, outputFile);
    fwrite(&byteRate, sizeof(int), 1, outputFile);
    fwrite(&blockAlign, sizeof(short), 1, outputFile);
    fwrite(&bitsPerSample, sizeof(short), 1, outputFile);

    fwrite(Subchunk2ID, sizeof(char), 4, outputFile);
    fwrite(&Subchunk2Size, sizeof(long), 1, outputFile);

    short sample;
    for (int i = 0; i < length; i++){
        sample = (short)(data[i] * 32767.0);
        fwrite(&sample, sizeof(short), 1, outputFile);
        // printf("%d\n", sample);
    }


    if(fclose(outputFile) != 0)
    {
        printf("Error closing output file\n");
        exit(-1);
    }

}

int main(int argc, char* argv[])
{
    clock_t startTime = clock();

    // check input parameters
    if (argc != 4) {
        printf("%s\n", "Check runtime parameters");
        return -1;
    }

    for (int i = 1; i < argc; i++) {
        if (!ends_with(argv[i], ".wav")) {
            printf("%s%s%s\n", "file ", argv[i], " needs to be a .wav file");
            return -1;
        }
    }

    int wavSize;
    int file1Size;
    int file2Size;
    int outputFileSize;

    char* file1 = argv[1];
    float* file1Signal = readFile(file1, &wavSize);
    file1Size = wavSize;

    char* file2 = argv[2];
    float* file2Signal = readFile(file2, &wavSize);
    file2Size = wavSize;

    outputFileSize = file1Size + file2Size - 1;
    float *outputFileSignal = new float[outputFileSize];
    // printf("%d, %d, %d\n", file1Size, file2Size, outputFileSize);

    convolve(file1Signal, file1Size, file2Signal, file2Size, outputFileSignal, outputFileSize);
    printf("done\n");

    for(int i = 0; i < outputFileSize; i++)
    {
        outputFileSignal[i] = outputFileSignal[i] * 0.8;
    }

    char* outputFile = argv[3];
    writeFile(outputFile, outputFileSignal, outputFileSize);

    // float test[0];
    // convolve(test, 0, test, 0, test, 0);

    float time = (clock() - startTime) / (float) CLOCKS_PER_SEC;
    printf("This program took %f seconds\n", time);


    return 0;
}
