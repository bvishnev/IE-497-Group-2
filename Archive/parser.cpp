#include <stdint.h>
#include <ap_int.h>
#include <hls_stream.h>

// Byte transmission structure
struct ByteData {
    uint8_t data;
    uint8_t valid;
    uint8_t start_msg;
    uint8_t end_msg;
};

// Parser output structure
struct ParserOutput {
    uint8_t valid_msg;
    uint8_t msg_type;
    uint16_t stock_locate;
    uint16_t tracking_no;
    uint64_t timestamp;
    uint64_t order_ref_no;
    uint32_t shares;
    uint8_t buy_sell;
    uint64_t stock;
    uint32_t price;
    uint64_t match_no;
    uint64_t new_order_ref_no;
    uint32_t attribution;
};

extern "C" {
void parser(
    // Input: stream of bytes to process
    const ByteData* input_stream,
    int num_bytes,
    
    // Output: parsed messages
    ParserOutput* output_stream,
    int* num_outputs
) {
    #pragma HLS INTERFACE m_axi port=input_stream bundle=gmem0 offset=slave
    #pragma HLS INTERFACE m_axi port=output_stream bundle=gmem1 offset=slave
    #pragma HLS INTERFACE m_axi port=num_outputs bundle=gmem2 offset=slave
    #pragma HLS INTERFACE s_axilite port=num_bytes
    #pragma HLS INTERFACE s_axilite port=return
    
    // Parser state variables
    uint8_t byte_idx = 0;
    bool count_en = false;
    bool message_invalid = false;
    
    // Current message being parsed
    ParserOutput current_msg;
    current_msg.valid_msg = 0;
    current_msg.msg_type = 0;
    current_msg.stock_locate = 0;
    current_msg.tracking_no = 0;
    current_msg.timestamp = 0;
    current_msg.order_ref_no = 0;
    current_msg.shares = 0;
    current_msg.buy_sell = 0;
    current_msg.stock = 0;
    current_msg.price = 0;
    current_msg.match_no = 0;
    current_msg.new_order_ref_no = 0;
    current_msg.attribution = 0;
    
    int output_count = 0;
    
    // Process each byte sequentially
    PROCESS_BYTES: for (int i = 0; i < num_bytes; i++) {
        #pragma HLS PIPELINE II=1
        
        ByteData byte_in = input_stream[i];
        uint8_t message = byte_in.data;
        bool valid = byte_in.valid;
        bool start_msg = byte_in.start_msg;
        bool end_msg = byte_in.end_msg;
        
        // Reset valid_msg at start of each cycle
        current_msg.valid_msg = 0;
        
        if (start_msg && valid) {
            byte_idx = 1;
            count_en = true;
            current_msg.msg_type = message;
            
            // Check if valid message type
            if (message != 0x41 && message != 0x44 && message != 0x45 && 
                message != 0x46 && message != 0x55 && message != 0x58) {
                message_invalid = true;
                byte_idx = 63;
                count_en = false;
            } else {
                message_invalid = false;
            }
            
            // Clear previous message data
            current_msg.stock_locate = 0;
            current_msg.tracking_no = 0;
            current_msg.timestamp = 0;
            current_msg.order_ref_no = 0;
            current_msg.shares = 0;
            current_msg.buy_sell = 0;
            current_msg.stock = 0;
            current_msg.price = 0;
            current_msg.match_no = 0;
            current_msg.new_order_ref_no = 0;
            current_msg.attribution = 0;
        } else if (count_en) {
            byte_idx++;
        }
        
        if (!valid) {
            byte_idx = 63;
            count_en = false;
            message_invalid = true;
        }
        
        // Process valid bytes that aren't start bytes
        if (valid && !message_invalid && !start_msg) {
            uint8_t idx = byte_idx;
            
            // Common fields (bytes 1-18)
            if (idx == 1) current_msg.stock_locate = (current_msg.stock_locate & 0x00FF) | ((uint16_t)message << 8);
            else if (idx == 2) current_msg.stock_locate = (current_msg.stock_locate & 0xFF00) | message;
            else if (idx == 3) current_msg.tracking_no = (current_msg.tracking_no & 0x00FF) | ((uint16_t)message << 8);
            else if (idx == 4) current_msg.tracking_no = (current_msg.tracking_no & 0xFF00) | message;
            else if (idx == 5) current_msg.timestamp = (current_msg.timestamp & 0x0000FFFFFFFFFF) | ((uint64_t)message << 40);
            else if (idx == 6) current_msg.timestamp = (current_msg.timestamp & 0xFFFF00FFFFFFFF) | ((uint64_t)message << 32);
            else if (idx == 7) current_msg.timestamp = (current_msg.timestamp & 0xFFFFFF00FFFFFF) | ((uint64_t)message << 24);
            else if (idx == 8) current_msg.timestamp = (current_msg.timestamp & 0xFFFFFFFF00FFFF) | ((uint64_t)message << 16);
            else if (idx == 9) current_msg.timestamp = (current_msg.timestamp & 0xFFFFFFFFFF00FF) | ((uint64_t)message << 8);
            else if (idx == 10) current_msg.timestamp = (current_msg.timestamp & 0xFFFFFFFFFFFF00) | message;
            else if (idx == 11) current_msg.order_ref_no = (current_msg.order_ref_no & 0x00FFFFFFFFFFFFFF) | ((uint64_t)message << 56);
            else if (idx == 12) current_msg.order_ref_no = (current_msg.order_ref_no & 0xFF00FFFFFFFFFFFF) | ((uint64_t)message << 48);
            else if (idx == 13) current_msg.order_ref_no = (current_msg.order_ref_no & 0xFFFF00FFFFFFFFFF) | ((uint64_t)message << 40);
            else if (idx == 14) current_msg.order_ref_no = (current_msg.order_ref_no & 0xFFFFFF00FFFFFFFF) | ((uint64_t)message << 32);
            else if (idx == 15) current_msg.order_ref_no = (current_msg.order_ref_no & 0xFFFFFFFF00FFFFFF) | ((uint64_t)message << 24);
            else if (idx == 16) current_msg.order_ref_no = (current_msg.order_ref_no & 0xFFFFFFFFFF00FFFF) | ((uint64_t)message << 16);
            else if (idx == 17) current_msg.order_ref_no = (current_msg.order_ref_no & 0xFFFFFFFFFFFF00FF) | ((uint64_t)message << 8);
            else if (idx == 18) current_msg.order_ref_no = (current_msg.order_ref_no & 0xFFFFFFFFFFFFFF00) | message;
            
            // Message-type specific fields (bytes 19+)
            else if (idx >= 19) {
                uint8_t msg_type = current_msg.msg_type;
                
                if (msg_type == 0x44) { // D-type - ends at byte 18
                    count_en = false;
                }
                else if (msg_type == 0x58) { // X-type
                    if (idx == 19) current_msg.shares = (current_msg.shares & 0x00FFFFFF) | ((uint32_t)message << 24);
                    else if (idx == 20) current_msg.shares = (current_msg.shares & 0xFF00FFFF) | ((uint32_t)message << 16);
                    else if (idx == 21) current_msg.shares = (current_msg.shares & 0xFFFF00FF) | ((uint32_t)message << 8);
                    else if (idx == 22) { current_msg.shares = (current_msg.shares & 0xFFFFFF00) | message; count_en = false; }
                }
                else if (msg_type == 0x45) { // E-type
                    if (idx == 19) current_msg.shares = (current_msg.shares & 0x00FFFFFF) | ((uint32_t)message << 24);
                    else if (idx == 20) current_msg.shares = (current_msg.shares & 0xFF00FFFF) | ((uint32_t)message << 16);
                    else if (idx == 21) current_msg.shares = (current_msg.shares & 0xFFFF00FF) | ((uint32_t)message << 8);
                    else if (idx == 22) current_msg.shares = (current_msg.shares & 0xFFFFFF00) | message;
                    else if (idx == 23) current_msg.match_no = (current_msg.match_no & 0x00FFFFFFFFFFFFFF) | ((uint64_t)message << 56);
                    else if (idx == 24) current_msg.match_no = (current_msg.match_no & 0xFF00FFFFFFFFFFFF) | ((uint64_t)message << 48);
                    else if (idx == 25) current_msg.match_no = (current_msg.match_no & 0xFFFF00FFFFFFFFFF) | ((uint64_t)message << 40);
                    else if (idx == 26) current_msg.match_no = (current_msg.match_no & 0xFFFFFF00FFFFFFFF) | ((uint64_t)message << 32);
                    else if (idx == 27) current_msg.match_no = (current_msg.match_no & 0xFFFFFFFF00FFFFFF) | ((uint64_t)message << 24);
                    else if (idx == 28) current_msg.match_no = (current_msg.match_no & 0xFFFFFFFFFF00FFFF) | ((uint64_t)message << 16);
                    else if (idx == 29) current_msg.match_no = (current_msg.match_no & 0xFFFFFFFFFFFF00FF) | ((uint64_t)message << 8);
                    else if (idx == 30) { current_msg.match_no = (current_msg.match_no & 0xFFFFFFFFFFFFFF00) | message; count_en = false; }
                }
                else if (msg_type == 0x41) { // A-type
                    if (idx == 19) current_msg.buy_sell = message;
                    else if (idx == 20) current_msg.shares = (current_msg.shares & 0x00FFFFFF) | ((uint32_t)message << 24);
                    else if (idx == 21) current_msg.shares = (current_msg.shares & 0xFF00FFFF) | ((uint32_t)message << 16);
                    else if (idx == 22) current_msg.shares = (current_msg.shares & 0xFFFF00FF) | ((uint32_t)message << 8);
                    else if (idx == 23) current_msg.shares = (current_msg.shares & 0xFFFFFF00) | message;
                    else if (idx == 24) current_msg.stock = (current_msg.stock & 0x00FFFFFFFFFFFFFF) | ((uint64_t)message << 56);
                    else if (idx == 25) current_msg.stock = (current_msg.stock & 0xFF00FFFFFFFFFFFF) | ((uint64_t)message << 48);
                    else if (idx == 26) current_msg.stock = (current_msg.stock & 0xFFFF00FFFFFFFFFF) | ((uint64_t)message << 40);
                    else if (idx == 27) current_msg.stock = (current_msg.stock & 0xFFFFFF00FFFFFFFF) | ((uint64_t)message << 32);
                    else if (idx == 28) current_msg.stock = (current_msg.stock & 0xFFFFFFFF00FFFFFF) | ((uint64_t)message << 24);
                    else if (idx == 29) current_msg.stock = (current_msg.stock & 0xFFFFFFFFFF00FFFF) | ((uint64_t)message << 16);
                    else if (idx == 30) current_msg.stock = (current_msg.stock & 0xFFFFFFFFFFFF00FF) | ((uint64_t)message << 8);
                    else if (idx == 31) current_msg.stock = (current_msg.stock & 0xFFFFFFFFFFFFFF00) | message;
                    else if (idx == 32) current_msg.price = (current_msg.price & 0x00FFFFFF) | ((uint32_t)message << 24);
                    else if (idx == 33) current_msg.price = (current_msg.price & 0xFF00FFFF) | ((uint32_t)message << 16);
                    else if (idx == 34) current_msg.price = (current_msg.price & 0xFFFF00FF) | ((uint32_t)message << 8);
                    else if (idx == 35) { current_msg.price = (current_msg.price & 0xFFFFFF00) | message; count_en = false; }
                }
                else if (msg_type == 0x55) { // U-type
                    if (idx == 19) current_msg.new_order_ref_no = (current_msg.new_order_ref_no & 0x00FFFFFFFFFFFFFF) | ((uint64_t)message << 56);
                    else if (idx == 20) current_msg.new_order_ref_no = (current_msg.new_order_ref_no & 0xFF00FFFFFFFFFFFF) | ((uint64_t)message << 48);
                    else if (idx == 21) current_msg.new_order_ref_no = (current_msg.new_order_ref_no & 0xFFFF00FFFFFFFFFF) | ((uint64_t)message << 40);
                    else if (idx == 22) current_msg.new_order_ref_no = (current_msg.new_order_ref_no & 0xFFFFFF00FFFFFFFF) | ((uint64_t)message << 32);
                    else if (idx == 23) current_msg.new_order_ref_no = (current_msg.new_order_ref_no & 0xFFFFFFFF00FFFFFF) | ((uint64_t)message << 24);
                    else if (idx == 24) current_msg.new_order_ref_no = (current_msg.new_order_ref_no & 0xFFFFFFFFFF00FFFF) | ((uint64_t)message << 16);
                    else if (idx == 25) current_msg.new_order_ref_no = (current_msg.new_order_ref_no & 0xFFFFFFFFFFFF00FF) | ((uint64_t)message << 8);
                    else if (idx == 26) current_msg.new_order_ref_no = (current_msg.new_order_ref_no & 0xFFFFFFFFFFFFFF00) | message;
                    else if (idx == 27) current_msg.shares = (current_msg.shares & 0x00FFFFFF) | ((uint32_t)message << 24);
                    else if (idx == 28) current_msg.shares = (current_msg.shares & 0xFF00FFFF) | ((uint32_t)message << 16);
                    else if (idx == 29) current_msg.shares = (current_msg.shares & 0xFFFF00FF) | ((uint32_t)message << 8);
                    else if (idx == 30) current_msg.shares = (current_msg.shares & 0xFFFFFF00) | message;
                    else if (idx == 31) current_msg.price = (current_msg.price & 0x00FFFFFF) | ((uint32_t)message << 24);
                    else if (idx == 32) current_msg.price = (current_msg.price & 0xFF00FFFF) | ((uint32_t)message << 16);
                    else if (idx == 33) current_msg.price = (current_msg.price & 0xFFFF00FF) | ((uint32_t)message << 8);
                    else if (idx == 34) { current_msg.price = (current_msg.price & 0xFFFFFF00) | message; count_en = false; }
                }
                else if (msg_type == 0x46) { // F-type
                    if (idx == 19) current_msg.buy_sell = message;
                    else if (idx == 20) current_msg.shares = (current_msg.shares & 0x00FFFFFF) | ((uint32_t)message << 24);
                    else if (idx == 21) current_msg.shares = (current_msg.shares & 0xFF00FFFF) | ((uint32_t)message << 16);
                    else if (idx == 22) current_msg.shares = (current_msg.shares & 0xFFFF00FF) | ((uint32_t)message << 8);
                    else if (idx == 23) current_msg.shares = (current_msg.shares & 0xFFFFFF00) | message;
                    else if (idx == 24) current_msg.stock = (current_msg.stock & 0x00FFFFFFFFFFFFFF) | ((uint64_t)message << 56);
                    else if (idx == 25) current_msg.stock = (current_msg.stock & 0xFF00FFFFFFFFFFFF) | ((uint64_t)message << 48);
                    else if (idx == 26) current_msg.stock = (current_msg.stock & 0xFFFF00FFFFFFFFFF) | ((uint64_t)message << 40);
                    else if (idx == 27) current_msg.stock = (current_msg.stock & 0xFFFFFF00FFFFFFFF) | ((uint64_t)message << 32);
                    else if (idx == 28) current_msg.stock = (current_msg.stock & 0xFFFFFFFF00FFFFFF) | ((uint64_t)message << 24);
                    else if (idx == 29) current_msg.stock = (current_msg.stock & 0xFFFFFFFFFF00FFFF) | ((uint64_t)message << 16);
                    else if (idx == 30) current_msg.stock = (current_msg.stock & 0xFFFFFFFFFFFF00FF) | ((uint64_t)message << 8);
                    else if (idx == 31) current_msg.stock = (current_msg.stock & 0xFFFFFFFFFFFFFF00) | message;
                    else if (idx == 32) current_msg.price = (current_msg.price & 0x00FFFFFF) | ((uint32_t)message << 24);
                    else if (idx == 33) current_msg.price = (current_msg.price & 0xFF00FFFF) | ((uint32_t)message << 16);
                    else if (idx == 34) current_msg.price = (current_msg.price & 0xFFFF00FF) | ((uint32_t)message << 8);
                    else if (idx == 35) current_msg.price = (current_msg.price & 0xFFFFFF00) | message;
                    else if (idx == 36) current_msg.attribution = (current_msg.attribution & 0x00FFFFFF) | ((uint32_t)message << 24);
                    else if (idx == 37) current_msg.attribution = (current_msg.attribution & 0xFF00FFFF) | ((uint32_t)message << 16);
                    else if (idx == 38) current_msg.attribution = (current_msg.attribution & 0xFFFF00FF) | ((uint32_t)message << 8);
                    else if (idx == 39) { current_msg.attribution = (current_msg.attribution & 0xFFFFFF00) | message; count_en = false; }
                }
            }
        }
        
        // Output complete message when end_msg is signaled
        if (valid && !message_invalid && end_msg) {
            current_msg.valid_msg = 1;
            count_en = false;
            byte_idx = 63;
            
            // Write output
            output_stream[output_count] = current_msg;
            output_count++;
        }
    }
    
    // Write number of valid messages parsed
    *num_outputs = output_count;
}
}
