#include <stdint.h>
#include <ap_int.h>

// Message parser state structure
struct ParserState {
    uint8_t byte_idx;
    bool count_en;
    bool message_invalid;
    
    // Output fields
    bool valid_msg;
    uint8_t msg_type;
    uint16_t stock_locate;
    uint16_t tracking_no;
    uint64_t timestamp;  // Using 64-bit to hold 48-bit value
    uint64_t order_ref_no;
    uint32_t shares;
    uint32_t price;
    uint8_t buy_sell;
    uint64_t stock;
    uint64_t match_no;
    uint64_t new_order_ref_no;
    uint32_t attribution;
};

extern "C" {
void parser(
    // Input signals
    bool start_msg,
    bool end_msg,
    uint8_t message,
    bool valid,
    bool rst,
    
    // Output signals (passed by pointer to simulate registers)
    bool* valid_msg,
    uint8_t* msg_type,
    uint16_t* stock_locate,
    uint16_t* tracking_no,
    uint64_t* timestamp,
    uint64_t* order_ref_no,
    uint32_t* shares,
    uint32_t* price,
    uint8_t* buy_sell,
    uint64_t* stock,
    uint64_t* match_no,
    uint64_t* new_order_ref_no,
    uint32_t* attribution,
    
    // State (must persist between calls)
    ParserState* state
) {
    #pragma HLS INTERFACE s_axilite port=start_msg
    #pragma HLS INTERFACE s_axilite port=end_msg
    #pragma HLS INTERFACE s_axilite port=message
    #pragma HLS INTERFACE s_axilite port=valid
    #pragma HLS INTERFACE s_axilite port=rst
    #pragma HLS INTERFACE s_axilite port=valid_msg
    #pragma HLS INTERFACE s_axilite port=msg_type
    #pragma HLS INTERFACE s_axilite port=stock_locate
    #pragma HLS INTERFACE s_axilite port=tracking_no
    #pragma HLS INTERFACE s_axilite port=timestamp
    #pragma HLS INTERFACE s_axilite port=order_ref_no
    #pragma HLS INTERFACE s_axilite port=shares
    #pragma HLS INTERFACE s_axilite port=price
    #pragma HLS INTERFACE s_axilite port=buy_sell
    #pragma HLS INTERFACE s_axilite port=stock
    #pragma HLS INTERFACE s_axilite port=match_no
    #pragma HLS INTERFACE s_axilite port=new_order_ref_no
    #pragma HLS INTERFACE s_axilite port=attribution
    #pragma HLS INTERFACE s_axilite port=state
    #pragma HLS INTERFACE s_axilite port=return
    
    if (rst) {
        state->valid_msg = false;
        state->message_invalid = false;
        state->msg_type = 0;
        state->stock_locate = 0;
        state->tracking_no = 0;
        state->timestamp = 0;
        state->order_ref_no = 0;
        state->shares = 0;
        state->buy_sell = 0;
        state->stock = 0;
        state->price = 0;
        state->match_no = 0;
        state->byte_idx = 0;
        state->new_order_ref_no = 0;
        state->attribution = 0;
        state->count_en = false;
    } else {
        state->valid_msg = false;
        
        if (start_msg && valid) {
            state->byte_idx = 1;
            state->count_en = true;
            state->msg_type = message;
            
            if (message != 0x41 && message != 0x44 && message != 0x45 && 
                message != 0x46 && message != 0x55 && message != 0x58) {
                state->message_invalid = true;
                state->byte_idx = 63;
                state->count_en = false;
            } else {
                state->message_invalid = false;
            }
            
            // Clear lingering values
            state->stock_locate = 0;
            state->tracking_no = 0;
            state->timestamp = 0;
            state->order_ref_no = 0;
            state->shares = 0;
            state->buy_sell = 0;
            state->stock = 0;
            state->price = 0;
            state->match_no = 0;
            state->new_order_ref_no = 0;
            state->attribution = 0;
        } else if (state->count_en) {
            state->byte_idx++;
        }
        
        if (!valid) {
            state->byte_idx = 63;
            state->count_en = false;
            state->message_invalid = true;
        }
        
        if (valid && !state->message_invalid && !start_msg) {
            uint8_t idx = state->byte_idx;
            
            // Common fields for all message types
            switch (idx) {
                case 1: state->stock_locate = (state->stock_locate & 0x00FF) | ((uint16_t)message << 8); break;
                case 2: state->stock_locate = (state->stock_locate & 0xFF00) | message; break;
                case 3: state->tracking_no = (state->tracking_no & 0x00FF) | ((uint16_t)message << 8); break;
                case 4: state->tracking_no = (state->tracking_no & 0xFF00) | message; break;
                case 5: state->timestamp = (state->timestamp & 0x0000FFFFFFFFFF) | ((uint64_t)message << 40); break;
                case 6: state->timestamp = (state->timestamp & 0xFFFF00FFFFFFFF) | ((uint64_t)message << 32); break;
                case 7: state->timestamp = (state->timestamp & 0xFFFFFF00FFFFFF) | ((uint64_t)message << 24); break;
                case 8: state->timestamp = (state->timestamp & 0xFFFFFFFF00FFFF) | ((uint64_t)message << 16); break;
                case 9: state->timestamp = (state->timestamp & 0xFFFFFFFFFF00FF) | ((uint64_t)message << 8); break;
                case 10: state->timestamp = (state->timestamp & 0xFFFFFFFFFFFF00) | message; break;
                case 11: state->order_ref_no = (state->order_ref_no & 0x00FFFFFFFFFFFFFF) | ((uint64_t)message << 56); break;
                case 12: state->order_ref_no = (state->order_ref_no & 0xFF00FFFFFFFFFFFF) | ((uint64_t)message << 48); break;
                case 13: state->order_ref_no = (state->order_ref_no & 0xFFFF00FFFFFFFFFF) | ((uint64_t)message << 40); break;
                case 14: state->order_ref_no = (state->order_ref_no & 0xFFFFFF00FFFFFFFF) | ((uint64_t)message << 32); break;
                case 15: state->order_ref_no = (state->order_ref_no & 0xFFFFFFFF00FFFFFF) | ((uint64_t)message << 24); break;
                case 16: state->order_ref_no = (state->order_ref_no & 0xFFFFFFFFFF00FFFF) | ((uint64_t)message << 16); break;
                case 17: state->order_ref_no = (state->order_ref_no & 0xFFFFFFFFFFFF00FF) | ((uint64_t)message << 8); break;
                case 18: state->order_ref_no = (state->order_ref_no & 0xFFFFFFFFFFFFFF00) | message; break;
                default:
                    // Message-type specific fields
                    if (state->msg_type == 0x44) { // D-type
                        state->shares = 0;
                        state->buy_sell = 0;
                        state->stock = 0;
                        state->price = 0;
                        state->match_no = 0;
                        state->new_order_ref_no = 0;
                        state->attribution = 0;
                        state->count_en = false;
                    } else if (state->msg_type == 0x58) { // X-type
                        state->buy_sell = 0;
                        state->stock = 0;
                        state->price = 0;
                        state->match_no = 0;
                        state->new_order_ref_no = 0;
                        state->attribution = 0;
                        switch (idx) {
                            case 19: state->shares = (state->shares & 0x00FFFFFF) | ((uint32_t)message << 24); break;
                            case 20: state->shares = (state->shares & 0xFF00FFFF) | ((uint32_t)message << 16); break;
                            case 21: state->shares = (state->shares & 0xFFFF00FF) | ((uint32_t)message << 8); break;
                            case 22: state->shares = (state->shares & 0xFFFFFF00) | message; state->count_en = false; break;
                        }
                    } else if (state->msg_type == 0x45) { // E-type
                        state->buy_sell = 0;
                        state->stock = 0;
                        state->price = 0;
                        state->new_order_ref_no = 0;
                        state->attribution = 0;
                        switch (idx) {
                            case 19: state->shares = (state->shares & 0x00FFFFFF) | ((uint32_t)message << 24); break;
                            case 20: state->shares = (state->shares & 0xFF00FFFF) | ((uint32_t)message << 16); break;
                            case 21: state->shares = (state->shares & 0xFFFF00FF) | ((uint32_t)message << 8); break;
                            case 22: state->shares = (state->shares & 0xFFFFFF00) | message; break;
                            case 23: state->match_no = (state->match_no & 0x00FFFFFFFFFFFFFF) | ((uint64_t)message << 56); break;
                            case 24: state->match_no = (state->match_no & 0xFF00FFFFFFFFFFFF) | ((uint64_t)message << 48); break;
                            case 25: state->match_no = (state->match_no & 0xFFFF00FFFFFFFFFF) | ((uint64_t)message << 40); break;
                            case 26: state->match_no = (state->match_no & 0xFFFFFF00FFFFFFFF) | ((uint64_t)message << 32); break;
                            case 27: state->match_no = (state->match_no & 0xFFFFFFFF00FFFFFF) | ((uint64_t)message << 24); break;
                            case 28: state->match_no = (state->match_no & 0xFFFFFFFFFF00FFFF) | ((uint64_t)message << 16); break;
                            case 29: state->match_no = (state->match_no & 0xFFFFFFFFFFFF00FF) | ((uint64_t)message << 8); break;
                            case 30: state->match_no = (state->match_no & 0xFFFFFFFFFFFFFF00) | message; state->count_en = false; break;
                        }
                    } else if (state->msg_type == 0x41) { // A-type
                        state->match_no = 0;
                        state->new_order_ref_no = 0;
                        state->attribution = 0;
                        switch (idx) {
                            case 19: state->buy_sell = message; break;
                            case 20: state->shares = (state->shares & 0x00FFFFFF) | ((uint32_t)message << 24); break;
                            case 21: state->shares = (state->shares & 0xFF00FFFF) | ((uint32_t)message << 16); break;
                            case 22: state->shares = (state->shares & 0xFFFF00FF) | ((uint32_t)message << 8); break;
                            case 23: state->shares = (state->shares & 0xFFFFFF00) | message; break;
                            case 24: state->stock = (state->stock & 0x00FFFFFFFFFFFFFF) | ((uint64_t)message << 56); break;
                            case 25: state->stock = (state->stock & 0xFF00FFFFFFFFFFFF) | ((uint64_t)message << 48); break;
                            case 26: state->stock = (state->stock & 0xFFFF00FFFFFFFFFF) | ((uint64_t)message << 40); break;
                            case 27: state->stock = (state->stock & 0xFFFFFF00FFFFFFFF) | ((uint64_t)message << 32); break;
                            case 28: state->stock = (state->stock & 0xFFFFFFFF00FFFFFF) | ((uint64_t)message << 24); break;
                            case 29: state->stock = (state->stock & 0xFFFFFFFFFF00FFFF) | ((uint64_t)message << 16); break;
                            case 30: state->stock = (state->stock & 0xFFFFFFFFFFFF00FF) | ((uint64_t)message << 8); break;
                            case 31: state->stock = (state->stock & 0xFFFFFFFFFFFFFF00) | message; break;
                            case 32: state->price = (state->price & 0x00FFFFFF) | ((uint32_t)message << 24); break;
                            case 33: state->price = (state->price & 0xFF00FFFF) | ((uint32_t)message << 16); break;
                            case 34: state->price = (state->price & 0xFFFF00FF) | ((uint32_t)message << 8); break;
                            case 35: state->price = (state->price & 0xFFFFFF00) | message; state->count_en = false; break;
                        }
                    } else if (state->msg_type == 0x55) { // U-type
                        state->match_no = 0;
                        state->buy_sell = 0;
                        state->stock = 0;
                        state->attribution = 0;
                        switch (idx) {
                            case 19: state->new_order_ref_no = (state->new_order_ref_no & 0x00FFFFFFFFFFFFFF) | ((uint64_t)message << 56); break;
                            case 20: state->new_order_ref_no = (state->new_order_ref_no & 0xFF00FFFFFFFFFFFF) | ((uint64_t)message << 48); break;
                            case 21: state->new_order_ref_no = (state->new_order_ref_no & 0xFFFF00FFFFFFFFFF) | ((uint64_t)message << 40); break;
                            case 22: state->new_order_ref_no = (state->new_order_ref_no & 0xFFFFFF00FFFFFFFF) | ((uint64_t)message << 32); break;
                            case 23: state->new_order_ref_no = (state->new_order_ref_no & 0xFFFFFFFF00FFFFFF) | ((uint64_t)message << 24); break;
                            case 24: state->new_order_ref_no = (state->new_order_ref_no & 0xFFFFFFFFFF00FFFF) | ((uint64_t)message << 16); break;
                            case 25: state->new_order_ref_no = (state->new_order_ref_no & 0xFFFFFFFFFFFF00FF) | ((uint64_t)message << 8); break;
                            case 26: state->new_order_ref_no = (state->new_order_ref_no & 0xFFFFFFFFFFFFFF00) | message; break;
                            case 27: state->shares = (state->shares & 0x00FFFFFF) | ((uint32_t)message << 24); break;
                            case 28: state->shares = (state->shares & 0xFF00FFFF) | ((uint32_t)message << 16); break;
                            case 29: state->shares = (state->shares & 0xFFFF00FF) | ((uint32_t)message << 8); break;
                            case 30: state->shares = (state->shares & 0xFFFFFF00) | message; break;
                            case 31: state->price = (state->price & 0x00FFFFFF) | ((uint32_t)message << 24); break;
                            case 32: state->price = (state->price & 0xFF00FFFF) | ((uint32_t)message << 16); break;
                            case 33: state->price = (state->price & 0xFFFF00FF) | ((uint32_t)message << 8); break;
                            case 34: state->price = (state->price & 0xFFFFFF00) | message; state->count_en = false; break;
                        }
                    } else if (state->msg_type == 0x46) { // F-type
                        state->match_no = 0;
                        state->new_order_ref_no = 0;
                        switch (idx) {
                            case 19: state->buy_sell = message; break;
                            case 20: state->shares = (state->shares & 0x00FFFFFF) | ((uint32_t)message << 24); break;
                            case 21: state->shares = (state->shares & 0xFF00FFFF) | ((uint32_t)message << 16); break;
                            case 22: state->shares = (state->shares & 0xFFFF00FF) | ((uint32_t)message << 8); break;
                            case 23: state->shares = (state->shares & 0xFFFFFF00) | message; break;
                            case 24: state->stock = (state->stock & 0x00FFFFFFFFFFFFFF) | ((uint64_t)message << 56); break;
                            case 25: state->stock = (state->stock & 0xFF00FFFFFFFFFFFF) | ((uint64_t)message << 48); break;
                            case 26: state->stock = (state->stock & 0xFFFF00FFFFFFFFFF) | ((uint64_t)message << 40); break;
                            case 27: state->stock = (state->stock & 0xFFFFFF00FFFFFFFF) | ((uint64_t)message << 32); break;
                            case 28: state->stock = (state->stock & 0xFFFFFFFF00FFFFFF) | ((uint64_t)message << 24); break;
                            case 29: state->stock = (state->stock & 0xFFFFFFFFFF00FFFF) | ((uint64_t)message << 16); break;
                            case 30: state->stock = (state->stock & 0xFFFFFFFFFFFF00FF) | ((uint64_t)message << 8); break;
                            case 31: state->stock = (state->stock & 0xFFFFFFFFFFFFFF00) | message; break;
                            case 32: state->price = (state->price & 0x00FFFFFF) | ((uint32_t)message << 24); break;
                            case 33: state->price = (state->price & 0xFF00FFFF) | ((uint32_t)message << 16); break;
                            case 34: state->price = (state->price & 0xFFFF00FF) | ((uint32_t)message << 8); break;
                            case 35: state->price = (state->price & 0xFFFFFF00) | message; break;
                            case 36: state->attribution = (state->attribution & 0x00FFFFFF) | ((uint32_t)message << 24); break;
                            case 37: state->attribution = (state->attribution & 0xFF00FFFF) | ((uint32_t)message << 16); break;
                            case 38: state->attribution = (state->attribution & 0xFFFF00FF) | ((uint32_t)message << 8); break;
                            case 39: state->attribution = (state->attribution & 0xFFFFFF00) | message; state->count_en = false; break;
                        }
                    }
                    break;
            }
        }
        
        if (valid && !state->message_invalid && end_msg) {
            state->valid_msg = true;
            state->count_en = false;
            state->byte_idx = 63;
        }
    }
    
    // Write outputs
    *valid_msg = state->valid_msg;
    *msg_type = state->msg_type;
    *stock_locate = state->stock_locate;
    *tracking_no = state->tracking_no;
    *timestamp = state->timestamp;
    *order_ref_no = state->order_ref_no;
    *shares = state->shares;
    *price = state->price;
    *buy_sell = state->buy_sell;
    *stock = state->stock;
    *match_no = state->match_no;
    *new_order_ref_no = state->new_order_ref_no;
    *attribution = state->attribution;
}
}
