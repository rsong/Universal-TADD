#include <cufft.h>
#include <cuda.h>
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#define IMUL(a, b) __mul24(a, b)

////////////////////////////////////////////////////////////////////////////////
// Padding kernels
////////////////////////////////////////////////////////////////////////////////
typedef float2 Complex;


////////////////////////////////////////////////////////////////////////////////
// Helper functions
////////////////////////////////////////////////////////////////////////////////
//Round a / b to nearest higher integer value
__host__  int iDivUp(int a, int b){
	return (a % b != 0) ? (a / b + 1) : (a / b);
}

//Align a to nearest higher multiple of b
__host__  int iAlignUp(int a, int b){
	return (a % b != 0) ?  (a - a % b + b) : a;
}

////////////////////////////////////////////////////////////////////////////////
// Data configuration
////////////////////////////////////////////////////////////////////////////////
__host__ int calculateFFTsize(int dataSize){
	//Highest non-zero bit position of dataSize
	int hiBit;
	//Neares lower and higher powers of two numbers for dataSize
	unsigned int lowPOT, hiPOT;

	//Align data size to a multiple of half-warp
	//in order to have each line starting at properly aligned addresses
	//for coalesced global memory writes in padKernel() and padData()
	dataSize = iAlignUp(dataSize, 16);

	//Find highest non-zero bit
	for(hiBit = 31; hiBit >= 0; hiBit--)
		if(dataSize & (1U << hiBit)) break;

	//No need to align, if already power of two
	lowPOT = 1U << hiBit;
	if(lowPOT == dataSize) return dataSize;

	//Align to a nearest higher power of two, if the size is small enough,
	//else align only to a nearest higher multiple of 512,
	//in order to save computation and memory bandwidth
	hiPOT = 1U << (hiBit + 1);
	//if(hiPOT <= 1024)
		return hiPOT;
	//else 
	//	return iAlignUp(dataSize, 512);
}

////////////////////////////////////////////////////////////////////////////////
// Cyclically shift convolution kernel, so that the center is at (0, 0)
////////////////////////////////////////////////////////////////////////////////
texture<float, 2, cudaReadModeElementType> texKernel;

__global__ void padKernel(
	float *d_PaddedKernel,
	int fftW,
	int fftH,
	int kernelW,
	int kernelH,
	int kernelX,
	int kernelY
){
	const int x = IMUL(blockDim.x, blockIdx.x) + threadIdx.x;
	const int y = IMUL(blockDim.y, blockIdx.y) + threadIdx.y;

	if(x < kernelW && y < kernelH){
		int kx = x - kernelX; if(kx < 0) kx += fftW;
		int ky = y - kernelY; if(ky < 0) ky += fftH;
		d_PaddedKernel[IMUL(ky, fftW) + kx] = tex2D(texKernel, (float)x + 0.5f, (float)y + 0.5f);
	}
}


////////////////////////////////////////////////////////////////////////////////
// Copy input data array to the upper left corner and pad by border values
////////////////////////////////////////////////////////////////////////////////
texture<float, 2, cudaReadModeElementType> texData;

__global__ void padData(
	float *d_PaddedData,
	int fftW,
	int fftH,
	int dataW,
	int dataH,
	int kernelW,
	int kernelH,
	int kernelX,
	int kernelY
){
	const int x = IMUL(blockDim.x, blockIdx.x) + threadIdx.x;
	const int y = IMUL(blockDim.y, blockIdx.y) + threadIdx.y;
	const int borderW = dataW + kernelX;
	const int borderH = dataH + kernelY;
	int dx;
	int dy;

	if(x < fftW && y < fftH){
		if(x < dataW) dx = x;
		if(y < dataH) dy = y;
		if(x >= dataW && x < borderW) dx = dataW - 1;
		if(y >= dataH && y < borderH) dy = dataH - 1;
		if(x >= borderW) dx = 0;
		if(y >= borderH) dy = 0;

		d_PaddedData[IMUL(y, fftW) + x] =
			tex2D(texData, (float)dx + 0.5f, (float)dy + 0.5f);
	}
}


////////////////////////////////////////////////////////////////////////////////
// Modulate Fourier image of padded data by Fourier image of padded kernel
// and normalize by FFT size
////////////////////////////////////////////////////////////////////////////////
__device__ void complexMulAndScale(Complex& a, Complex b, float c){
	Complex t = {c * (a.x * b.x - a.y * b.y), c * (a.y * b.x + a.x * b.y)};
	a = t;
}

__global__ void modulateAndNormalize(
	Complex *fft_PaddedData,
	Complex *fft_PaddedKernel,
	int dataN
){
	const int     tid = IMUL(blockDim.x, blockIdx.x) + threadIdx.x;
	const int threadN = IMUL(blockDim.x, gridDim.x);
	const float     q = 1.0f / (float)dataN;

	for(int i = tid; i < dataN; i += threadN)
		complexMulAndScale(fft_PaddedData[i], fft_PaddedKernel[i], q);
}

////////////////////////////////////////////////////////////////////////////////
// FFT convolution program
////////////////////////////////////////////////////////////////////////////////
__host__ void fftFunction(float* output, float *in_Data, float *in_Kernel, int DATA_H, int DATA_W, int KERNEL_H, int KERNEL_W ) {

	cudaArray *a_Kernel, *a_Data;
	cudaChannelFormatDesc float2tex = cudaCreateChannelDesc<float>();
	float *d_PaddedKernel, *d_PaddedData;
	Complex *fft_PaddedKernel, *fft_PaddedData;

	cufftHandle FFTplan_R2C;
	cufftHandle FFTplan_C2R;

	int KERNEL_X, KERNEL_Y, PADDING_W, PADDING_H, FFT_W, FFT_H, FFT_SIZE, KERNEL_SIZE, DATA_SIZE, CFFT_SIZE;

	// we expect 2 inputs: prhs[0] -- data, prhs[1] -- kernel
	// Kernel center position
	KERNEL_X = KERNEL_W/2;
	KERNEL_Y = KERNEL_H/2;

	// Width and height of padding for "clamp to border" addressing mode
	PADDING_W = KERNEL_W - 1;
	PADDING_H = KERNEL_H - 1;

	// Derive FFT size from data and kernel dimensions
	FFT_W = calculateFFTsize(DATA_W + PADDING_W);
	FFT_H = calculateFFTsize(DATA_H + PADDING_H);


	//fprintf(stderr,"Calculating byte sizes..\n");
	FFT_SIZE = FFT_W * FFT_H * sizeof(float);
	CFFT_SIZE = FFT_W * FFT_H * sizeof(Complex);
	KERNEL_SIZE = KERNEL_W * KERNEL_H * sizeof(float);
	DATA_SIZE = DATA_W * DATA_H * sizeof(float);

	cudaMallocArray(&a_Kernel, &float2tex, KERNEL_W, KERNEL_H) ;
	cudaMallocArray(&a_Data,   &float2tex,   DATA_W,   DATA_H) ;
	cudaMalloc((void **)&d_PaddedKernel, 	FFT_SIZE) ;
	cudaMalloc((void **)&d_PaddedData,   	FFT_SIZE) ;
	cudaMalloc((void **)&fft_PaddedKernel, 	CFFT_SIZE);
	cudaMalloc((void **)&fft_PaddedData, 	CFFT_SIZE);

	cufftPlan2d(&FFTplan_C2R, FFT_H, FFT_W, CUFFT_C2R) ;
	cufftPlan2d(&FFTplan_R2C, FFT_H, FFT_W, CUFFT_R2C) ;

	cudaMemset(d_PaddedKernel, 0, FFT_SIZE) ;
	cudaMemset(d_PaddedData,   0, FFT_SIZE) ;

	// copying input data and convolution kernel from host to CUDA arrays
	cudaMemcpyToArray(a_Kernel, 0, 0, in_Kernel, KERNEL_SIZE, cudaMemcpyHostToDevice) ;
	cudaMemcpyToArray(a_Data,   0, 0, in_Data,   DATA_SIZE,   cudaMemcpyHostToDevice) ;
	//binding CUDA arrays to texture references
	cudaBindTextureToArray(texKernel, a_Kernel) ;
	cudaBindTextureToArray(texData,   a_Data)   ;

	//Block width should be a multiple of maximum coalesced write size 
	//for coalesced memory writes in padKernel() and padData()
	dim3 threadBlock(16, 12);
	dim3 kernelBlockGrid(iDivUp(KERNEL_W, threadBlock.x), iDivUp(KERNEL_H, threadBlock.y));
	dim3 dataBlockGrid(iDivUp(FFT_W, threadBlock.x), iDivUp(FFT_H, threadBlock.y));

	//fprintf(stderr,"Padding convolution kernel\n");
	// padding convolution kernel
	padKernel<<<kernelBlockGrid, threadBlock>>>(
		d_PaddedKernel,
		FFT_W,
		FFT_H,
		KERNEL_W,
		KERNEL_H,
		KERNEL_X,
		KERNEL_Y
		);

	//fprintf(stderr,"Padding input data array\n");
	// padding input data array
	padData<<<dataBlockGrid, threadBlock>>>(
		d_PaddedData,
		FFT_W,
		FFT_H,
		DATA_W,
		DATA_H,
		KERNEL_W,
		KERNEL_H,
		KERNEL_X,
		KERNEL_Y
		);

	cufftExecR2C(FFTplan_R2C, (cufftReal *)d_PaddedKernel, (cufftComplex *)fft_PaddedKernel);
	cufftExecR2C(FFTplan_R2C, (cufftReal *)d_PaddedData, (cufftComplex *)fft_PaddedData);

	modulateAndNormalize<<<16, 128>>>(
		fft_PaddedData,
		fft_PaddedKernel,
		FFT_W * FFT_H
		);
	cufftExecC2R(FFTplan_C2R, (cufftComplex *)fft_PaddedData, (cufftReal *)d_PaddedData);

	//fprintf(stderr,"Fetching result from GPU\n");
	cudaMemcpy(output, d_PaddedData, FFT_SIZE, cudaMemcpyDeviceToHost) ;

	//fprintf(stderr,"Freeing memory\n");
	cudaUnbindTexture(texData);
	cudaUnbindTexture(texKernel);
	cufftDestroy(FFTplan_C2R);
	cufftDestroy(FFTplan_R2C);
	cudaFree(d_PaddedData);
	cudaFree(d_PaddedKernel);
	cudaFree(fft_PaddedData);
	cudaFree(fft_PaddedKernel);
	cudaFreeArray(a_Data);
	cudaFreeArray(a_Kernel);

}

float* runFFT(float* DATA, int DATA_W, int DATA_H, float* KERNEL, int KERNEL_W, int KERNEL_H){
	int FFT_W, FFT_H, PADDING_H, PADDING_W;

	// Width and height of padding for "clamp to border" addressing mode
		PADDING_W = KERNEL_W - 1;
		PADDING_H = KERNEL_H - 1;

	// Derive FFT size from data and kernel dimensions
		FFT_W = calculateFFTsize(DATA_W + PADDING_W);
		FFT_H = calculateFFTsize(DATA_H + PADDING_H);
	
   //create output
		float* output=(float*)malloc(DATA_W*DATA_H*1*sizeof(float));

	// If there's just too much data to do in a single run, we need to break it up, eh?
		// how much "too big" is it?
		int MAX_FFT_W = 1048576/FFT_H;

		if ( FFT_W > MAX_FFT_W ) { // we need to break up the data
			
			int STRIP_W = MAX_FFT_W-KERNEL_W+1;
			int STRIP_SIZE = STRIP_W * DATA_H * sizeof(float);
			int OVERLAP_SIZE = DATA_H * KERNEL_W/2 * sizeof(float);
			
			//malloc input and output strips
			float* strip_output=(float*)malloc(STRIP_SIZE);
			float* strip_input=(float*)malloc(STRIP_SIZE);
			
			int REMAIN_W = DATA_W; // counter showing how much of the data remains to be processed

			// Do the first strip
			cudaMemcpy(strip_input, DATA, STRIP_SIZE, cudaMemcpyHostToHost);
			fftFunction(strip_output, strip_input, KERNEL, DATA_H, STRIP_W, KERNEL_H, KERNEL_W);
			cudaMemcpy(output, strip_output, STRIP_SIZE, cudaMemcpyHostToHost);
			
			REMAIN_W -= STRIP_W - KERNEL_W/2; // need some overlap on the right..

			while ( REMAIN_W > STRIP_W ) {
				// read the strip
				cudaMemcpy(strip_input, DATA + DATA_H*(DATA_W-REMAIN_W-KERNEL_W/2), STRIP_SIZE,cudaMemcpyHostToHost);
				// convolve the strip
				fftFunction(strip_output, strip_input, KERNEL, DATA_H, STRIP_W, KERNEL_H, KERNEL_W);
				// copy the result into the output
				cudaMemcpy(output + DATA_H*(DATA_W-REMAIN_W), strip_output+DATA_H*KERNEL_W/2, STRIP_SIZE-OVERLAP_SIZE,cudaMemcpyHostToHost);
				
				// set the remaining number of columns
				REMAIN_W -= STRIP_W - KERNEL_W;
			}
			// Now we have to do the remaining edge strip
			int LAST_STRIP_SIZE = REMAIN_W * DATA_H * sizeof(double);
			float* last_strip_output=(float*)malloc(LAST_STRIP_SIZE+OVERLAP_SIZE);
			float* last_strip_input=(float*)malloc(LAST_STRIP_SIZE+OVERLAP_SIZE);

			cudaMemcpy(last_strip_input, DATA+DATA_H*(DATA_W-REMAIN_W-KERNEL_W/2),LAST_STRIP_SIZE+OVERLAP_SIZE,cudaMemcpyHostToHost);
			fftFunction(last_strip_output, last_strip_input, KERNEL, DATA_H, REMAIN_W+KERNEL_W/2, KERNEL_H, KERNEL_W);
			cudaMemcpy(output + DATA_H*(DATA_W-REMAIN_W), last_strip_output+DATA_H*KERNEL_W/2, LAST_STRIP_SIZE,cudaMemcpyHostToHost);
			
			free(strip_output);
			free(strip_input);
			free(last_strip_output);
			free(last_strip_input);

		}
		else {
			fftFunction(output, DATA, KERNEL, DATA_H, DATA_W, KERNEL_H, KERNEL_W);
		}

     return output;
}

//DATA is 2D
float* sobel(float* DATA, int DATA_W, int DATA_H, float* KERNEL, int KERNEL_W, int KERNEL_H){

    float          GX[3][3];
    float         GY[3][3];
    /* 3x3 GX Sobel mask.  Ref: www.cee.hw.ac.uk/hipr/html/sobel.html */
    GX[0][0] = -1; GX[0][1] = 0; GX[0][2] = 1;
    GX[1][0] = -2; GX[1][1] = 0; GX[1][2] = 2;
    GX[2][0] = -1; GX[2][1] = 0; GX[2][2] = 1;

    /* 3x3 GY Sobel mask.  Ref: www.cee.hw.ac.uk/hipr/html/sobel.html */
    GY[0][0] =  1; GY[0][1] =  2; GY[0][2] =  1;
    GY[1][0] =  0; GY[1][1] =  0; GY[1][2] =  0;
    GY[2][0] = -1; GY[2][1] = -2; GY[2][2] = -1;

	//allocate X and Y 
	float* outputX=(float*)malloc(DATA_W*DATA_H*1*sizeof(float));
	float* outputY=(float*)malloc(DATA_W*DATA_H*1*sizeof(float));

	outputX = runFFT(DATA,DATA_W,DATA_H,*GX,3,3);
	outputX = runFFT(DATA,DATA_W,DATA_H,*GY,3,3);


	//alocate edgeImage
	float* edgeImage=(float*)malloc(DATA_W*DATA_H*1*sizeof(float));

	return edgeImage;
}

