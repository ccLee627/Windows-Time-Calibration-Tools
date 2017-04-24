// QpcTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <math.h>
#include <Windows.h>
#include <intrin.h>  

double StdDevAsFractionOfMean(DWORD64 * Samples, size_t SampleSize)
{
	double mean = 0;
	double err = 0;
	DWORD64 previous = Samples[0];
	for (size_t i = 1; i < SampleSize; i++)
	{
		mean += Samples[i] - previous;
		previous = Samples[i];
	}
	mean /= SampleSize;
	previous = Samples[0];
	for (size_t i = 1; i < SampleSize; i++)
	{
		double delta = Samples[i] - previous;
		delta -= mean;
		err += delta * delta;
		previous = Samples[i];
	}
	err /= SampleSize;
	err = sqrt(err);
	return err / mean;
}

void ScaleAndPrintResults(LARGE_INTEGER Start, LARGE_INTEGER End, size_t SampleSize, DWORD64* Samples, const char * Name)
{
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);

	double queryTime = End.QuadPart - Start.QuadPart;
	queryTime /= freq.QuadPart;
	queryTime /= SampleSize;
	queryTime *= 1e9;
	double stdev = StdDevAsFractionOfMean(Samples, SampleSize) * queryTime;
	printf("%s latency %.1fns STDEV %.1fns\n", Name, queryTime, stdev);
}

int main(int argc, char ** argv)
{
	if (argc != 3) {
		printf("%s samples_size iterations\n", argv[0]);
		exit(-1);
	}

    // Dump the command line args
    for (int i = 0; i < argc; i++) {
        printf("%s ", argv[i]);
    }
    printf("\n");

	size_t sampleSize = atoll(argv[1]);
	size_t iterations = atol(argv[2]);
	DWORD64* samples = new DWORD64[sampleSize];
	memset(samples, 0, sizeof(DWORD64) * sampleSize);


	for (int j = 0; j < iterations; j++)
	{
		FILETIME ft;
		LARGE_INTEGER start, end;
		QueryPerformanceCounter(&start);
		for (long long i = 0; i < sampleSize; i++)
		{
			GetSystemTimePreciseAsFileTime(&ft);
			samples[i] = __rdtsc();
		}
		QueryPerformanceCounter(&end);
		ScaleAndPrintResults(start, end, sampleSize, samples, "GetSystemTimePreciseAsFileTime");
	}
	for (int j = 0; j < iterations; j++)
	{
		LARGE_INTEGER ft;
		LARGE_INTEGER start, end;
		QueryPerformanceCounter(&start);
		for (long long i = 0; i < sampleSize; i++)
		{
			QueryPerformanceCounter(&ft);
			samples[i] = __rdtsc();
		}
		QueryPerformanceCounter(&end);
		ScaleAndPrintResults(start, end, sampleSize, samples, "QueryPerformanceCounter");
	}

	for (int j = 0; j < iterations; j++)
	{
		LARGE_INTEGER start, end;
		QueryPerformanceCounter(&start);
		for (long long i = 0; i < sampleSize; i++)
		{
			samples[i] = __rdtsc();
		}
		QueryPerformanceCounter(&end);
		ScaleAndPrintResults(start, end, sampleSize, samples, "__rdtsc");
	}
    for (int j = 0; j < iterations; j++)
    {
        LARGE_INTEGER start, end;
        QueryPerformanceCounter(&start);
        for (long long i = 0; i < sampleSize; i++)
        {
            unsigned int cpuid;
            samples[i] = __rdtscp(&cpuid);
        }
        QueryPerformanceCounter(&end);
        ScaleAndPrintResults(start, end, sampleSize, samples, "__rdtscp");
    }

	return 0;
}

