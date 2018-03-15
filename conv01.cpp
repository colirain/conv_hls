/****************************************/
//generate date: 3/13/2018
//author: litingyu
//function: convolution
/*****************************************/

//#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "conv_core.h"

#define INPUTSIZE 1024
#define OUTPUTSIZE 1024


void hls_conv_accel(AXI_VAL INPUT_STREAM[INPUTSIZE], AXI_VAL OUTPUT_STREAM[OUTPUTSIZE]){
	#pragma HLS INTERFACE ap_ctrl_none port=return
	//#pragma HLS INTERFACE s_axilite port=return     bundle=CONTROL_BUS
	#pragma HLS INTERFACE axis      port=OUTPUT_STREAM
	#pragma HLS INTERFACE axis      port=INPUT_STREAM
	
	accel_conv(INPUT_STREAM, OUTPUT_STREAM);
	return;
}
