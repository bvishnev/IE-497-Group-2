// Example of actual serial stream processing
#include <ap_axi_sdata.h>
#include <hls_stream.h>

extern "C" {
    void deserialize(
        hls::stream<ap_axis<8,0,0,0>>& stream_in,  // AXI-Stream interface
        hls::stream<ap_axis<8,0,0,0>>& stream_out,
        unsigned int stream_length)
    {
        #pragma HLS INTERFACE axis port=stream_in    // True streaming
        #pragma HLS INTERFACE axis port=stream_out   // True streaming
        #pragma HLS INTERFACE s_axilite port=stream_length
        #pragma HLS INTERFACE s_axilite port=return
        
        for(unsigned int i = 0; i < stream_length; i++) {
            #pragma HLS PIPELINE II=1
            ap_axis<8,0,0,0> temp = stream_in.read();
            stream_out.write(temp);
        }
    }
}
