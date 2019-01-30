// Includes
#include <iostream>
#include <stdio.h>
#include <math.h>
#include <time.h>

#include <cuda.h>
#include <cublas.h>

// Constants used by the program
#define MAX_PITCH_VALUE_IN_BYTES       262144
#define MAX_TEXTURE_WIDTH_IN_BYTES     65536
#define MAX_TEXTURE_HEIGHT_IN_BYTES    32768
#define MAX_PART_OF_FREE_MEMORY_USED   0.9
#define BLOCK_DIM                      16

void knn(float** ref_host, int ref_width, int ref_classes, float* query_host, int query_width, int height, int k, float** dist_host, int* ind_host);
void printErrorMessage(cudaError_t error, int memorySize);

__global__ void cuAddQNormAndSqrt(float *dist, int width, int pitch, float *q, int k);
__global__ void cuInsertionSort(float *dist, int dist_pitch, int *ind, int ind_pitch, int width, int height, int k);
__global__ void cuAddRNorm(float *dist, int width, int pitch, int height, float *vec);
__global__ void cuComputeNorm(float *mat, int width, int pitch, int height, float *norm);