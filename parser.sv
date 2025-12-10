`timescale 1ns / 1ps


module parser (
 input logic clk,
 input logic rst,              
 input logic start_msg,        // Single bit that indicates the start of a new message
 input logic end_msg,          // Single bit that indicates the end of a message
 input logic [7:0] message,    // The data of the message (byte-wise serial)
 input logic valid,            // Single bit indicating whether the current message byte is valid
 
 // All message types (A, E, X, D, U, F)
 output reg valid_msg,        // Single bit which indicates whether the entire message is valid
 output reg [7:0] msg_type,  // Stores which type of market action the current message encodes
 output reg [15:0] stock_locate, // Locate code identifying the security
 output reg [15:0] tracking_no, // Nasdaq internal tracking number
 output reg [47:0] timestamp, // Nanoseconds since midnight
 output reg [63:0] order_ref_no, // The unique reference number assigned to the order at the time of receipt
 
 // Add Order (A), Order Executed Message (E), Order Cancel Message (X), Order Replace Message (U), Add Order with MPID Attribution Message (F) only
 output reg [31:0] shares, // The total number of shares associated with the order
 
// Add Order (A) only, Order Replace Message (U) only, Add Order with MPID Attribution Message (F) only
 output reg [31:0] price, // The display price of the new order

 // Add Order (A) and Add Order with MPID Attribution Message (F) only
 output reg [7:0] buy_sell, // The type of order being added. “B” = Buy Order. “S” = SellOrder
 output reg [63:0] stock, // Stock symbol, right padded with spaces

 // Order Executed Message (E) only
 output reg [63:0] match_no, // The Nasdaq generated day unique Match Number of this execution

 // Order Replace Message (U) only
 output reg[63:0] new_order_ref_no,  // The unique reference number assigned to the new order at the time of receipt

 // Add Order with MPID Attribution Message (F) only
 output reg[31:0] attribution  // Nasdaq Market participant identifier associated with the entered order
 
); 
 
 reg [5:0] byte_idx; // stores the index of the current byte
 reg message_invalid; // keeps track of whether an invalid byte has been encountered yet
 
 always_ff @(posedge clk or posedge rst) begin 
 
    if(rst) begin 
        valid_msg <= 1'd0;
        message_invalid <= 1'b0; 
        msg_type <= 8'd0; 
        stock_locate <= 16'd0;
        tracking_no <= 16'd0;
        timestamp <= 48'd0;
        order_ref_no <= 64'd0;
        shares <= 32'd0;
        buy_sell <= 8'd0;
        stock <= 64'd0;
        price <= 32'd0;
        match_no <= 64'd0;
        byte_idx <= 6'd0;
        new_order_ref_no <= 64'd0;
        attribution <= 32'd0; 
    end else begin
        
        valid_msg <= 1'b0; // Later overwritten if final byte and overall message is valid
    
        if(start_msg && valid) begin
            byte_idx <= 6'd1; // Stores 1 in the counter, which will be the index of the next byte
            msg_type <= message;
            message_invalid <= 1'b0;
            
            // Clear any lingering values from previous messages
            stock_locate <= 16'd0;
            tracking_no <= 16'd0;
            timestamp <= 48'd0;
            order_ref_no <= 64'd0;
            shares <= 32'd0;
            buy_sell <= 8'd0;
            stock <= 64'd0;
            price <= 32'd0;
            match_no <= 64'd0;
            new_order_ref_no <= 64'd0;
            attribution <= 32'd0;
        end else if (valid && !end_msg) begin // If current byte valid and not start or end, increment byte_idx
            byte_idx <= byte_idx + 1;
        end else if(!valid) begin
            message_invalid <= 1'b1;
        end
        
        if(valid && !message_invalid && !start_msg) begin
            // Assigns message to output corresponding to current byte index
            case(byte_idx)
                6'd1: stock_locate[15:8] <= message;
                6'd2: stock_locate[7:0] <= message;
                6'd3: tracking_no[15:8] <= message;
                6'd4: tracking_no[7:0] <= message; 
                6'd5: timestamp[47:40] <= message;
                6'd6: timestamp[39:32] <= message;
                6'd7: timestamp[31:24] <= message;
                6'd8: timestamp[23:16] <= message;
                6'd9: timestamp[15:8] <= message;
                6'd10: timestamp[7:0] <= message;
                6'd11: order_ref_no[63:56] <= message;
                6'd12: order_ref_no[55:48] <= message;
                6'd13: order_ref_no[47:40] <= message;
                6'd14: order_ref_no[39:32] <= message;
                6'd15: order_ref_no[31:24] <= message;
                6'd16: order_ref_no[23:16] <= message;
                6'd17: order_ref_no[15:8] <= message;
                6'd18: order_ref_no[7:0] <= message;
                default: begin
                    if(msg_type == 8'h44) begin
                        // Set sentinel values for unused output signals in D-type message
                        shares <= 32'd0;
                        buy_sell <= 8'd0;
                        stock <= 64'd0;
                        price <= 32'd0;
                        match_no <= 64'd0;
                        new_order_ref_no <= 64'd0;
                        attribution <= 32'd0;
                    end
                    if(msg_type == 8'h58) begin
                        // Set sentinel values for unused output signals in X-type message
                        buy_sell <= 8'd0;
                        stock <= 64'd0;
                        price <= 32'd0;
                        match_no <= 64'd0;
                        new_order_ref_no <= 64'd0;
                        attribution <= 32'd0;
                        // Set other signals depending on current byte index
                        case(byte_idx)
                            6'd19: shares[31:24] <= message;
                            6'd20: shares[23:16] <= message;
                            6'd21: shares[15:8] <= message;
                            6'd22: shares[7:0] <= message;
                        endcase
                    end
                    if(msg_type == 8'h45) begin
                        // Set sentinel values for unused output signals in E-type message
                        buy_sell <= 8'd0;
                        stock <= 64'd0;
                        price <= 32'd0;
                        new_order_ref_no <= 64'd0;
                        attribution <= 32'd0;
                        // Set other signals depending on current byte index
                        case(byte_idx)
                            6'd19: shares[31:24] <= message;
                            6'd20: shares[23:16] <= message;
                            6'd21: shares[15:8] <= message;
                            6'd22: shares[7:0] <= message;
                            6'd23: match_no[63:56] <= message;
                            6'd24: match_no[55:48] <= message;
                            6'd25: match_no[47:40] <= message;
                            6'd26: match_no[39:32] <= message;
                            6'd27: match_no[31:24] <= message;
                            6'd28: match_no[23:16] <= message;
                            6'd29: match_no[15:8] <= message;
                            6'd30: match_no[7:0] <= message;
                        endcase
                    end
                    if(msg_type == 8'h41) begin
                        // Set sentinel values for unused output signals in A-type message
                        match_no <= 64'd0;
                        new_order_ref_no <= 64'd0;
                        attribution <= 32'd0;
                        // Set other signals depending on current byte index
                        case(byte_idx)
                            6'd19: buy_sell <= message;
                            6'd20: shares[31:24] <= message;
                            6'd21: shares[23:16] <= message;
                            6'd22: shares[15:8] <= message;
                            6'd23: shares[7:0] <= message;
                            6'd24: stock[63:56] <= message;
                            6'd25: stock[55:48] <= message;
                            6'd26: stock[47:40] <= message;
                            6'd27: stock[39:32] <= message;
                            6'd28: stock[31:24] <= message;
                            6'd29: stock[23:16] <= message;
                            6'd30: stock[15:8] <= message;
                            6'd31: stock[7:0] <= message;
                            6'd32: price[31:24] <= message;
                            6'd33: price[23:16] <= message;
                            6'd34: price[15:8] <= message;
                            6'd35: price[7:0] <= message;
                        endcase
                    end
                    if(msg_type == 8'h55) begin
                        // Set sentinel values for unused output signals in U-type message
                        match_no <= 64'd0;
                        buy_sell <= 8'd0;
                        stock <= 64'd0;
                        attribution <= 32'd0;
                        // Set other signals depending on current byte index
                        case(byte_idx)
                            6'd19: new_order_ref_no[63:56] <= message; 
                            6'd20: new_order_ref_no[55:48] <= message; 
                            6'd21: new_order_ref_no[47:40] <= message; 
                            6'd22: new_order_ref_no[39:32] <= message; 
                            6'd23: new_order_ref_no[31:24] <= message; 
                            6'd24: new_order_ref_no[23:16] <= message; 
                            6'd25: new_order_ref_no[15:8] <= message; 
                            6'd26: new_order_ref_no[7:0] <= message; 
                            6'd27: shares[31:24] <= message;
                            6'd28: shares[23:16] <= message;
                            6'd29: shares[15:8] <= message;
                            6'd30: shares[7:0] <= message;
                            6'd31: price[31:24] <= message;
                            6'd32: price[23:16] <= message;
                            6'd33: price[15:8] <= message;
                            6'd34: price[7:0] <= message;
                        endcase
                    end
                    if(msg_type == 8'h46) begin
                        // Set sentinel values for unused output signals in F-type message
                        match_no <= 64'd0;
                        new_order_ref_no <= 64'd0;
                        // Set other signals depending on current byte index
                        case(byte_idx)
                            6'd19: buy_sell <= message;
                            6'd20: shares[31:24] <= message;
                            6'd21: shares[23:16] <= message;
                            6'd22: shares[15:8] <= message;
                            6'd23: shares[7:0] <= message;
                            6'd24: stock[63:56] <= message;
                            6'd25: stock[55:48] <= message;
                            6'd26: stock[47:40] <= message;
                            6'd27: stock[39:32] <= message;
                            6'd28: stock[31:24] <= message;
                            6'd29: stock[23:16] <= message;
                            6'd30: stock[15:8] <= message;
                            6'd31: stock[7:0] <= message;
                            6'd32: price[31:24] <= message;
                            6'd33: price[23:16] <= message;
                            6'd34: price[15:8] <= message;
                            6'd35: price[7:0] <= message;
                            6'd36: attribution[31:24] <= message; 
                            6'd37: attribution[23:16] <= message; 
                            6'd38: attribution[15:8] <= message; 
                            6'd39: attribution[7:0] <= message; 
                        endcase
                    end
                end
            endcase 
        end
        
        // If the overall message is valid and this is the last byte, raise valid_msg for one cycle
        if(valid && !message_invalid && end_msg) begin
            valid_msg <= 1'b1;
        end
    end
    
 end

endmodule
