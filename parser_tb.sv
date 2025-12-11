`timescale 1ns/1ps

module tb_parser;

    // Clock and reset
    logic clk;
    logic rst;

    // Inputs to parser
    logic start_msg;
    logic end_msg;
    logic [7:0] message;
    logic valid;

    // Outputs from parser
    logic valid_msg;
    logic [7:0] msg_type;
    logic [15:0] stock_locate;
    logic [15:0] tracking_no;
    logic [47:0] timestamp;
    logic [63:0] order_ref_no;
    logic [31:0] shares;
    logic [7:0] buy_sell;
    logic [63:0] stock;
    logic [31:0] price;
    logic [63:0] match_no;

    // Instantiate the parser
    parser uut (
        .clk(clk),
        .rst(rst),
        .start_msg(start_msg),
        .end_msg(end_msg),
        .message(message),
        .valid(valid),
        .valid_msg(valid_msg),
        .msg_type(msg_type),
        .stock_locate(stock_locate),
        .tracking_no(tracking_no),
        .timestamp(timestamp),
        .order_ref_no(order_ref_no),
        .shares(shares),
        .buy_sell(buy_sell),
        .stock(stock),
        .price(price),
        .match_no(match_no)
    );

    // Clock generation
    initial clk = 0;
    always #5 clk = ~clk;  // 100MHz clock

    // Task to send a byte
    task send_byte(input [7:0] byte, input is_valid, input logic is_start, input logic is_end);
        begin
            start_msg = is_start;
            end_msg = is_end;
            message = byte;
            valid = is_valid;
            @(posedge clk);
            valid = 1'b0;
            start_msg = 1'b0;
            end_msg = 1'b0;
            @(posedge clk);
        end
    endtask

    // Test sequence
    initial begin
        // Initialize signals
        rst = 1;
        start_msg = 0;
        end_msg = 0;
        message = 0;
        valid = 0;
        @(posedge clk);
        rst = 0;

        // Example: Send a simple A-type message (msg_type = 8'h41)
        send_byte(8'h41, 1, 1, 0); // start message
        send_byte(8'h01, 1, 0, 0); // stock_locate[15:8]
        send_byte(8'h02, 1, 0, 0); // stock_locate[7:0]
        send_byte(8'h03, 1, 0, 0); // tracking_no[15:8]
        send_byte(8'h04, 1, 0, 0); // tracking_no[7:0]
        send_byte(8'h00, 1, 0, 0); // timestamp[47:40]
        send_byte(8'h01, 1, 0, 0); // timestamp[39:32]
        send_byte(8'h02, 1, 0, 0); // timestamp[31:24]
        send_byte(8'h03, 1, 0, 0); // timestamp[23:16]
        send_byte(8'h04, 1, 0, 0); // timestamp[15:8]
        send_byte(8'h05, 1, 0, 0); // timestamp[7:0]
        send_byte(8'hAA, 1, 0, 0); // order_ref_no[63:56]
        send_byte(8'hBB, 1, 0, 0); // order_ref_no[55:48]
        send_byte(8'hCC, 1, 0, 0); // order_ref_no[47:40]
        send_byte(8'hDD, 1, 0, 0); // order_ref_no[39:32]
        send_byte(8'hEE, 1, 0, 0); // order_ref_no[31:24]
        send_byte(8'hFF, 1, 0, 0); // order_ref_no[23:16]
        send_byte(8'h11, 1, 0, 0); // order_ref_no[15:8]
        send_byte(8'h22, 1, 0, 0); // order_ref_no[7:0]
        send_byte(8'h42, 1, 0, 0); // buy_sell
        send_byte(8'h00, 1, 0, 0); // shares[31:24]
        send_byte(8'h00, 1, 0, 0); // shares[23:16]
        send_byte(8'h10, 1, 0, 0); // shares[15:8]
        send_byte(8'h00, 1, 0, 0); // shares[7:0]
        send_byte(8'h53, 1, 0, 0); // stock[63:56] ('S')
        send_byte(8'h54, 1, 0, 0); // stock[55:48] ('T')
        send_byte(8'h4F, 1, 0, 0); // stock[47:40] ('O')
        send_byte(8'h43, 1, 0, 0); // stock[39:32] ('C')
        send_byte(8'h4B, 1, 0, 0); // stock[31:24] ('K')
        send_byte(8'h20, 1, 0, 0); // stock[23:16] (' ')
        send_byte(8'h20, 1, 0, 0); // stock[15:8] (' ')
        send_byte(8'h20, 1, 0, 0); // stock[7:0] (' ')
        send_byte(8'h00, 1, 0, 0); // price[31:24]
        send_byte(8'h01, 1, 0, 0); // price[23:16]
        send_byte(8'h02, 1, 0, 0); // price[15:8]
        send_byte(8'h03, 1, 0, 1); // price[7:0], end message

        // Wait a few cycles to observe output
        //repeat(10) @(posedge clk);

        $stop;
    end

    // Monitor outputs
    initial begin
        $monitor("Time: %0t | valid_msg=%b | msg_type=%h | stock_locate=%h | tracking_no=%h | timestamp=%h | order_ref_no=%h | shares=%h | buy_sell=%h | stock=%h | price=%h | match_no=%h",
                  $time, valid_msg, msg_type, stock_locate, tracking_no, timestamp, order_ref_no, shares, buy_sell, stock, price, match_no);
    end

endmodule
