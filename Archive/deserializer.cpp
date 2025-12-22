// This is based on a tutorial from https://docs.amd.com/r/en-US/ug1399-vitis-hls/HLS-Stream-Library

#include <cstdint>


extern "C" {
    void deserialize(uint8_t* mem_in, uint8_t* mem_out, unsigned int length){
        #pragma HLS INTERFACE m_axi  port=mem_in   offset=slave bundle=gmem0      
        #pragma HLS INTERFACE m_axi  port=mem_out  offset=slave bundle=gmem1
        #pragma HLS INTERFACE s_axilite port=mem_in
        #pragma HLS INTERFACE s_axilite port=mem_out
        #pragma HLS INTERFACE s_axilite port=length
        #pragma HLS INTERFACE s_axilite port=return

        for(unsigned int i = 0; i < length; i++) {
            #pragma HLS PIPELINE II=1
            mem_out[i] = mem_in[i]; 
        }
    }
}

