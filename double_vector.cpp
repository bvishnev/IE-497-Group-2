extern "C" {

void double_vector(

    const int* input,     // Read-only from global memory

    int* output,          // Write to global memory

    int size

) {

    #pragma HLS INTERFACE m_axi port=input bundle=gmem0
    #pragma HLS INTERFACE m_axi port=output bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=input
    #pragma HLS INTERFACE s_axilite port=output
    #pragma HLS INTERFACE s_axilite port=size
    #pragma HLS INTERFACE s_axilite port=return

    

    for (int i = 0; i < size; i++) {

        #pragma HLS PIPELINE II=1

        output[i] = input[i] * 2;  // Example: double the values

    }

}

}
