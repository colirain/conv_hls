/***************************************/
//generate date:3/13/2018
//author: litingyu
//function: convolution core
/**************************************/

#include <stdio.h>
#include <assert.h>
#include <ap_axi_sdata.h>
#include <math.h>

#define IMAGESIZE 256
#define KERNELSIZE 5
#define OUTSIZE IMAGESIZE-KERNELSIZE

typedef ap_axiu<32,1,1,1> AXI_VAL;

template <typename T, int U, int TI, int TD>
T pop_stream(ap_axiu <sizeof(T)*8,U,TI,TD> const &e)
{
	#pragma HLS INLINE
          
	//assert(sizeof(T) == sizeof(int));
	union
	{
		int ival;
		T oval;
	} converter;
	converter.ival = e.data;
	T ret = converter.oval;

	volatile ap_uint<sizeof(T)> strb = e.strb;
	volatile ap_uint<sizeof(T)> keep = e.keep;
	volatile ap_uint<U> user = e.user;
	volatile ap_uint<1> last = e.last;
	volatile ap_uint<TI> id = e.id;
	volatile ap_uint<TD> dest = e.dest;

	return ret;
}


template <typename T, int U, int TI, int TD> ap_axiu <sizeof(T)*8,U,TI,TD> push_stream(T const &v, bool last = false)
{
	#pragma HLS INLINE
	ap_axiu<sizeof(T)*8,U,TI,TD> e;

	//assert(sizeof(T) == sizeof(int));
	union
	{
		int oval;
		T ival;
	} converter;
	converter.ival = v;
	e.data = converter.oval;

	// set it to sizeof(T) ones
	e.strb = -1;
	e.keep = 15; //e.strb;
	e.user = 0;
	e.last = last ? 1 : 0;
	e.id = 0;
	e.dest = 0;
	return e;
}

template <typename T, int in_num, int in_size, int out_num, int out_size, int k_size> 
void convcore(T input[in_num][in_size][in_size], T weights[k_size * k_size * out_num], T bias[out_num], T output[out_num][out_size][out_size]){
		int i,j,k,l;
		int k_i,k_j;

		//initialization
		for(int i=0; i< out_num; i++)
			for(int j=0; j< out_size; j++)
				for(int k=0; k<out_size; k++)
					output[i][j][k] = 0;
		//original output
		for(i=0; i<out_num; i++){
			for(j=0; j<in_size-k_size+1; j++){
				for(k=0; k<in_size-k_size+1; k++){
					for(l=0; l<in_num; l++){
						for(k_i=0; k_i<k_size; k_i++){
							for(k_j=0; k_j<k_size; k_j++){
								output[i][j][k] = output[i][j][k] + input[l][j+k_i][k+k_j]*weights[i*(k_size*k_size*in_num)+l*(k_size*k_size)+k_i*k_size+k_j];
							}
						}
						
					}
					
				}
			}
		}

		//output*bias
		for(i=0;i<out_num; i++)
			for(j=0;j<out_size;j++)
				for(k=0;k<out_size; k++)
					output[i][j][k] = output[i][j][k]*bias[i];

return;
}

void hls_convolution(float inputImage[1][IMAGESIZE][IMAGESIZE], float outputImage[1][OUTSIZE][OUTSIZE]){
	float weights[KERNELSIZE * KERNELSIZE] = {
			  2,   4,    5,    4,     2,
			  4,    9,     12,    9,     4,
			  5,    12,   15,    12,   5,
			 4,    9,     12,    9,     4,
			 4,      5,     4,     2};
	float bias[1] = {0.008695652173913044};
	convcore<float, 1, IMAGESIZE, 1, OUTSIZE, KERNELSIZE>(inputImage,weights, bias, outputImage);
	return;
}


void accel_conv(AXI_VAL inputstream[1024], AXI_VAL outputstream[1024]){
	float input[1][IMAGESIZE][IMAGESIZE];
	float output[1][OUTSIZE][OUTSIZE];

	for(int i=0; i<IMAGESIZE; i++)
		for(int j=0; j<IMAGESIZE; j++)
		{
			#pragma HLS PIPELINE II=1
			int k = i*IMAGESIZE+j;
			input[0][i][j] = pop_stream<float,1,1,1>(inputstream[k]);

		}

	hls_convolution(input, output);


	for(int i=0; i<OUTSIZE; i++)
	{
		for(int j=0; j<OUTSIZE; j++){
			#pragma HLS PIPELINE II=1
			outputstream[i*OUTSIZE+j] = push_stream<float,1,1,1>(output[0][i][j], i*j == (OUTSIZE-1)*(OUTSIZE-1));
		}

	}
	return;
}
