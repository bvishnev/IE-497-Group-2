// This is based on a tutorial from https://docs.amd.com/r/en-US/ug1399-vitis-hls/HLS-Stream-Library

#include <cstdint>


extern "C" {
    void deserialize(uint8_t* stream_in, uint8_t* stream_out, unsigned int stream_length){
        #pragma HLS INTERFACE m_axi  port=stream_in   offset=slave bundle=gmem0      
        #pragma HLS INTERFACE m_axi  port=stream_out  offset=slave bundle=gmem1
        #pragma HLS INTERFACE s_axilite port=stream_in
        #pragma HLS INTERFACE s_axilite port=stream_out
        #pragma HLS INTERFACE s_axilite port=stream_length
        #pragma HLS INTERFACE s_axilite port=return

        for(unsigned int i = 0; i < stream_length; i++) {
            #pragma HLS PIPELINE II=1       //this pragma pipelines the loop so one loop iteration corresponds to one clock cycle
            stream_out[i] = stream_in[i]; 
        }
    }
}


