#include "knn_cublas.cuh"

#include <stdio.h>
#include <math.h>
#include "cuda.h"
#include <time.h>

texture<float, 2, cudaReadModeElementType> texA;

__global__ void cuComputeDistanceTexture(int wA, float * B, int wB, int pB, int dim, float* AB){
    unsigned int xIndex = blockIdx.x * blockDim.x + threadIdx.x;
    unsigned int yIndex = blockIdx.y * blockDim.y + threadIdx.y;
    if ( xIndex<wB && yIndex<wA ){
        float ssd = 0;
        for (int i=0; i<dim; i++){
            float tmp  = tex2D(texA, (float)yIndex, (float)i) - B[ i * pB + xIndex ];
            ssd += tmp * tmp;
        }
        AB[yIndex * pB + xIndex] = ssd;
    }
}


__global__ void cuInsertionSort(float *dist, int dist_pitch, int *ind, int ind_pitch, int width, int height, int k){

	// Variables
    int l, i, j;
    float *p_dist;
	int   *p_ind;
    float curr_dist, max_dist;
    int   curr_row,  max_row;
    unsigned int xIndex = blockIdx.x * blockDim.x + threadIdx.x;
	
    if (xIndex<width){
        
        // Pointer shift, initialization, and max value
        p_dist   = dist + xIndex;
		p_ind    = ind  + xIndex;
        max_dist = p_dist[0];
        p_ind[0] = 1;
        
        // Part 1 : sort kth firt elementZ
        for (l=1; l<k; l++){
            curr_row  = l * dist_pitch;
			curr_dist = p_dist[curr_row];
			if (curr_dist<max_dist){
                i=l-1;
				for (int a=0; a<l-1; a++){
					if (p_dist[a*dist_pitch]>curr_dist){
						i=a;
						break;
					}
				}
                for (j=l; j>i; j--){
					p_dist[j*dist_pitch] = p_dist[(j-1)*dist_pitch];
					p_ind[j*ind_pitch]   = p_ind[(j-1)*ind_pitch];
                }
				p_dist[i*dist_pitch] = curr_dist;
				p_ind[i*ind_pitch]   = l+1;
			}
			else
				p_ind[l*ind_pitch] = l+1;
			max_dist = p_dist[curr_row];
		}
        
        // Part 2 : insert element in the k-th first lines
        max_row = (k-1)*dist_pitch;
        for (l=k; l<height; l++){
			curr_dist = p_dist[l*dist_pitch];
			if (curr_dist<max_dist){
                i=k-1;
				for (int a=0; a<k-1; a++){
					if (p_dist[a*dist_pitch]>curr_dist){
						i=a;
						break;
					}
				}
                for (j=k-1; j>i; j--){
					p_dist[j*dist_pitch] = p_dist[(j-1)*dist_pitch];
					p_ind[j*ind_pitch]   = p_ind[(j-1)*ind_pitch];
                }
				p_dist[i*dist_pitch] = curr_dist;
				p_ind[i*ind_pitch]   = l+1;
                max_dist             = p_dist[max_row];
            }
        }
    }
}

__global__ void cuParallelSqrt(float *dist, int width, int pitch, int k){
    unsigned int xIndex = blockIdx.x * blockDim.x + threadIdx.x;
    unsigned int yIndex = blockIdx.y * blockDim.y + threadIdx.y;
    if (xIndex<width && yIndex<k)
        dist[yIndex*pitch + xIndex] = sqrt(dist[yIndex*pitch + xIndex]);
}


void printErrorMessage(cudaError_t error, int memorySize){
    printf("==================================================\n");
    printf("MEMORY ALLOCATION ERROR  : %s\n", cudaGetErrorString(error));
    printf("Whished allocated memory : %d\n", memorySize);
    printf("==================================================\n");
}

void knn(float** ref_host, int ref_width, int ref_classes, float* query_host, int query_width, int height, int k, float** dist_host, int* ind_host){
    
    unsigned int size_of_float = sizeof(float);
    unsigned int size_of_int   = sizeof(int);
    
    // Variables
    float        *query_dev;
    float        *dist_dev;
    int          *ind_dev;
    cudaArray    *ref_array;
    cudaError_t  result;
    size_t       query_pitch;
    size_t	     query_pitch_in_bytes;
    size_t       ind_pitch;
    size_t       ind_pitch_in_bytes;
    size_t       max_nb_query_traited;
    size_t       actual_nb_query_width;
    
    // CUDA Initialisation
    cuInit(0);
       
    // Determine maximum number of query that can be treated
    //max_nb_query_traited = ( memory_free * MAX_PART_OF_FREE_MEMORY_USED - size_of_float * ref_width*height ) / ( size_of_float * (height + ref_width) + size_of_int * k);
    max_nb_query_traited = query_width;
    
    // Allocation of global memory for query points and for distances
    result = cudaMallocPitch( (void **) &query_dev, &query_pitch_in_bytes, max_nb_query_traited * size_of_float, height + ref_width);
    if (result){
        printErrorMessage(result, max_nb_query_traited*size_of_float*(height+ref_width));
        return;
    }
    query_pitch = query_pitch_in_bytes/size_of_float;
    dist_dev    = query_dev + height * query_pitch;
	

    // Allocation of global memory for indexes	
    result = cudaMallocPitch( (void **) &ind_dev, &ind_pitch_in_bytes, max_nb_query_traited * size_of_int, k);
	if (result){
        cudaFree(query_dev);
        printErrorMessage(result, max_nb_query_traited*size_of_int*k);
        return;
    }
    ind_pitch = ind_pitch_in_bytes/size_of_int;
    
	for(int cl=0; cl < ref_classes; cl++)
	{
			// Allocation of memory (global or texture) for reference points
				// Allocation of texture memory
				cudaChannelFormatDesc channelDescA = cudaCreateChannelDesc<float>();
				result = cudaMallocArray( &ref_array, &channelDescA, ref_width, height );
				if (result){
					printErrorMessage(result, ref_width*height*size_of_float);
					cudaFree(ind_dev);
					cudaFree(query_dev);
					return;
				}
				cudaMemcpyToArray( ref_array, 0, 0, ref_host[cl], ref_width * height * size_of_float, cudaMemcpyHostToDevice );
        
				// Set texture parameters and bind texture to array
				texA.addressMode[0] = cudaAddressModeClamp;
				texA.addressMode[1] = cudaAddressModeClamp;
				texA.filterMode     = cudaFilterModePoint;
				texA.normalized     = 0;
				cudaBindTextureToArray(texA, ref_array);
		
			// Split queries to fit in GPU memory
			for (int i=0; i<query_width; i+=max_nb_query_traited){
        
				// Number of query points considered
				actual_nb_query_width = min( max_nb_query_traited, query_width-i );
        
				// Copy of part of query actually being treated
				cudaMemcpy2D(query_dev, query_pitch_in_bytes, &query_host[i], query_width*size_of_float, actual_nb_query_width*size_of_float, height, cudaMemcpyHostToDevice);
        
				// Grids ans threads
				dim3 g_16x16(actual_nb_query_width/16, ref_width/16, 1);
				dim3 t_16x16(16, 16, 1);
				if (actual_nb_query_width%16 != 0) g_16x16.x += 1;
				if (ref_width  %16 != 0) g_16x16.y += 1;
				//
				dim3 g_256x1(actual_nb_query_width/256, 1, 1);
				dim3 t_256x1(256, 1, 1);
				if (actual_nb_query_width%256 != 0) g_256x1.x += 1;
				//
				dim3 g_k_16x16(actual_nb_query_width/16, k/16, 1);
				dim3 t_k_16x16(16, 16, 1);
				if (actual_nb_query_width%16 != 0) g_k_16x16.x += 1;
				if (k  %16 != 0) g_k_16x16.y += 1;
        
				// Kernel 1: Compute all the distances
				cuComputeDistanceTexture<<<g_16x16,t_16x16>>>(ref_width, query_dev, actual_nb_query_width, query_pitch, height, dist_dev);
				    
				// Kernel 2: Sort each column
				cuInsertionSort<<<g_256x1,t_256x1>>>(dist_dev, query_pitch, ind_dev, ind_pitch, actual_nb_query_width, ref_width, k);
        
				// Kernel 3: Compute square root of k first elements
				cuParallelSqrt<<<g_k_16x16,t_k_16x16>>>(dist_dev, query_width, query_pitch, k);
        
				// Memory copy of output from device to host
				cudaMemcpy2D(&dist_host[cl][i], query_width*size_of_float, dist_dev, query_pitch_in_bytes, actual_nb_query_width*size_of_float, k, cudaMemcpyDeviceToHost);
				//cudaMemcpy2D(&ind_host[i],  query_width*size_of_int,   ind_dev,  ind_pitch_in_bytes,   actual_nb_query_width*size_of_int,   k, cudaMemcpyDeviceToHost);
			}
	}
    
    // Free memory
    cudaFreeArray(ref_array);

    cudaFree(ind_dev);
    cudaFree(query_dev);
}


