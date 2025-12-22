`timescale 1ns / 1ps

module parser_tb;

    // Clock and reset
    logic clk;
    logic rst;

    // Inputs to parser
    logic start_msg;
    logic end_msg;
    logic [7:0] message;
    logic valid;

    // Outputs from parser
    reg valid_msg;
    reg [7:0] msg_type;
    reg [15:0] stock_locate;
    reg [15:0] tracking_no;
    reg [47:0] timestamp;
    reg [63:0] order_ref_no;
    reg [31:0] shares;
    reg [7:0] buy_sell;
    reg [63:0] stock;
    reg [31:0] price;
    reg [63:0] match_no;
    reg [63:0] new_order_ref_no;
    reg [31:0] attribution;

    // Instantiate the parser
    parser dut (
        .clk(clk),
        .rst(rst),
        .start_msg(start_msg),
        // .end_msg(end_msg),
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
        .match_no(match_no),
        .new_order_ref_no(new_order_ref_no),
        .attribution(attribution)
    );

    // Clock generation
    initial clk = 0;
    always #5 clk = ~clk;  // 100MHz clock

    // Task to send a byte
    task send_byte(input [7:0] data, input is_valid, input logic is_start);
        begin
            start_msg = is_start;
            message = data;
            valid = is_valid;
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

        // Valid "A" type message
        send_byte(8'h41, 1, 1);  // msg_type (start message)
        send_byte(8'h01, 1, 0);  // stock_locate[15:8]
        send_byte(8'h02, 1, 0);  // stock_locate[7:0]
        send_byte(8'h03, 1, 0);  // tracking_no[15:8]
        send_byte(8'h04, 1, 0);  // tracking_no[7:0]
        send_byte(8'h00, 1, 0);  // timestamp[47:40]
        send_byte(8'h01, 1, 0);  // timestamp[39:32]
        send_byte(8'h02, 1, 0);  // timestamp[31:24]
        send_byte(8'h03, 1, 0);  // timestamp[23:16]
        send_byte(8'h04, 1, 0);  // timestamp[15:8]
        send_byte(8'h05, 1, 0);  // timestamp[7:0]
        send_byte(8'hAA, 1, 0);  // order_ref_no[63:56]
        send_byte(8'hBB, 1, 0);  // order_ref_no[55:48]
        send_byte(8'hCC, 1, 0);  // order_ref_no[47:40]
        send_byte(8'hDD, 1, 0);  // order_ref_no[39:32]
        send_byte(8'hEE, 1, 0);  // order_ref_no[31:24]
        send_byte(8'hFF, 1, 0);  // order_ref_no[23:16]
        send_byte(8'h11, 1, 0);  // order_ref_no[15:8]
        send_byte(8'h22, 1, 0);  // order_ref_no[7:0]
        send_byte(8'h42, 1, 0);  // buy_sell
        send_byte(8'h00, 1, 0);  // shares[31:24]
        send_byte(8'h00, 1, 0);  // shares[23:16]
        send_byte(8'h10, 1, 0);  // shares[15:8]
        send_byte(8'h00, 1, 0);  // shares[7:0]
        send_byte(8'h53, 1, 0);  // stock[63:56] ('S')
        send_byte(8'h54, 1, 0);  // stock[55:48] ('T')
        send_byte(8'h4F, 1, 0);  // stock[47:40] ('O')
        send_byte(8'h43, 1, 0);  // stock[39:32] ('C')
        send_byte(8'h4B, 1, 0);  // stock[31:24] ('K')
        send_byte(8'h20, 1, 0);  // stock[23:16] (' ')
        send_byte(8'h20, 1, 0);  // stock[15:8] (' ')
        send_byte(8'h20, 1, 0);  // stock[7:0] (' ')
        send_byte(8'h00, 1, 0);  // price[31:24]
        send_byte(8'h01, 1, 0);  // price[23:16]
        send_byte(8'h02, 1, 0);  // price[15:8]
        send_byte(8'h03, 1, 0);  // price[7:0], end message

        // In-between message garbage
        send_byte(8'hf8, 1, 0);
        send_byte(8'h32, 0, 0);

        // Invalid "A" type message
        send_byte(8'h41, 1, 1);  // msg_type (start message)
        send_byte(8'h01, 1, 0);  // stock_locate[15:8]
        send_byte(8'h02, 1, 0);  // stock_locate[7:0]
        send_byte(8'h03, 1, 0);  // tracking_no[15:8]
        send_byte(8'h04, 1, 0);  // tracking_no[7:0]
        send_byte(8'h00, 1, 0);  // timestamp[47:40]
        send_byte(8'h01, 1, 0);  // timestamp[39:32]
        send_byte(8'h02, 1, 0);  // timestamp[31:24]
        send_byte(8'h03, 1, 0);  // timestamp[23:16]
        send_byte(8'h04, 1, 0);  // timestamp[15:8]
        send_byte(8'h05, 1, 0);  // timestamp[7:0]
        send_byte(8'hAA, 1, 0);  // order_ref_no[63:56]
        send_byte(8'hBB, 1, 0);  // order_ref_no[55:48]
        send_byte(8'hCC, 1, 0);  // order_ref_no[47:40]
        send_byte(8'hDD, 1, 0);  // order_ref_no[39:32]
        send_byte(8'hEE, 0, 0);  // order_ref_no[31:24]
        send_byte(8'hFF, 1, 0);  // order_ref_no[23:16]
        send_byte(8'h11, 1, 0);  // order_ref_no[15:8]
        send_byte(8'h22, 1, 0);  // order_ref_no[7:0]
        send_byte(8'h42, 1, 0);  // buy_sell
        send_byte(8'h00, 1, 0);  // shares[31:24]
        send_byte(8'h00, 1, 0);  // shares[23:16]
        send_byte(8'h10, 1, 0);  // shares[15:8]
        send_byte(8'h00, 1, 0);  // shares[7:0]
        send_byte(8'h53, 1, 0);  // stock[63:56] ('S')
        send_byte(8'h54, 1, 0);  // stock[55:48] ('T')
        send_byte(8'h4F, 1, 0);  // stock[47:40] ('O')
        send_byte(8'h43, 1, 0);  // stock[39:32] ('C')
        send_byte(8'h4B, 1, 0);  // stock[31:24] ('K')
        send_byte(8'h20, 1, 0);  // stock[23:16] (' ')
        send_byte(8'h20, 1, 0);  // stock[15:8] (' ')
        send_byte(8'h20, 1, 0);  // stock[7:0] (' ')
        send_byte(8'h00, 1, 0);  // price[31:24]
        send_byte(8'h01, 1, 0);  // price[23:16]
        send_byte(8'h02, 1, 0);  // price[15:8]
        send_byte(8'h03, 1, 0);  // price[7:0], end message

        // Valid "F" type message
        send_byte(8'h46, 1, 1);  // msg_type (start message)
        send_byte(8'h01, 1, 0);  // stock_locate[15:8]
        send_byte(8'h02, 1, 0);  // stock_locate[7:0]
        send_byte(8'h03, 1, 0);  // tracking_no[15:8]
        send_byte(8'h04, 1, 0);  // tracking_no[7:0]
        send_byte(8'h00, 1, 0);  // timestamp[47:40]
        send_byte(8'h01, 1, 0);  // timestamp[39:32]
        send_byte(8'h02, 1, 0);  // timestamp[31:24]
        send_byte(8'h03, 1, 0);  // timestamp[23:16]
        send_byte(8'h04, 1, 0);  // timestamp[15:8]
        send_byte(8'h05, 1, 0);  // timestamp[7:0]
        send_byte(8'hAA, 1, 0);  // order_ref_no[63:56]
        send_byte(8'hBB, 1, 0);  // order_ref_no[55:48]
        send_byte(8'hCC, 1, 0);  // order_ref_no[47:40]
        send_byte(8'hDD, 1, 0);  // order_ref_no[39:32]
        send_byte(8'hEE, 1, 0);  // order_ref_no[31:24]
        send_byte(8'hFF, 1, 0);  // order_ref_no[23:16]
        send_byte(8'h11, 1, 0);  // order_ref_no[15:8]
        send_byte(8'h22, 1, 0);  // order_ref_no[7:0]
        send_byte(8'h53, 1, 0);  // buy_sell
        send_byte(8'h00, 1, 0);  // shares[31:24]
        send_byte(8'h00, 1, 0);  // shares[23:16]
        send_byte(8'h10, 1, 0);  // shares[15:8]
        send_byte(8'h00, 1, 0);  // shares[7:0]
        send_byte(8'h53, 1, 0);  // stock[63:56] ('S')
        send_byte(8'h54, 1, 0);  // stock[55:48] ('T')
        send_byte(8'h4F, 1, 0);  // stock[47:40] ('O')
        send_byte(8'h43, 1, 0);  // stock[39:32] ('C')
        send_byte(8'h4B, 1, 0);  // stock[31:24] ('K')
        send_byte(8'h20, 1, 0);  // stock[23:16] (' ')
        send_byte(8'h20, 1, 0);  // stock[15:8] (' ')
        send_byte(8'h20, 1, 0);  // stock[7:0] (' ')
        send_byte(8'h00, 1, 0);  // price[31:24]
        send_byte(8'h01, 1, 0);  // price[23:16]
        send_byte(8'h02, 1, 0);  // price[15:8]
        send_byte(8'h03, 1, 0);  // price[7:0]
        send_byte(8'h41, 1, 0);  // attribution[31:24]
        send_byte(8'h44, 1, 0);  // attribution[23:16]
        send_byte(8'h41, 1, 0);  // attribution[15:8]
        send_byte(8'h4D, 1, 0);  // attribution[7:0], end

        // Valid "E" Type Message
        send_byte(8'h45, 1, 1);  // msg_type (start message)
        send_byte(8'h01, 1, 0);  // stock_locate[15:8]
        send_byte(8'h02, 1, 0);  // stock_locate[7:0]
        send_byte(8'h03, 1, 0);  // tracking_no[15:8]
        send_byte(8'h04, 1, 0);  // tracking_no[7:0]
        send_byte(8'h00, 1, 0);  // timestamp[47:40]
        send_byte(8'h01, 1, 0);  // timestamp[39:32]
        send_byte(8'h02, 1, 0);  // timestamp[31:24]
        send_byte(8'h03, 1, 0);  // timestamp[23:16]
        send_byte(8'h04, 1, 0);  // timestamp[15:8]
        send_byte(8'h05, 1, 0);  // timestamp[7:0]
        send_byte(8'hAA, 1, 0);  // order_ref_no[63:56]
        send_byte(8'hBB, 1, 0);  // order_ref_no[55:48]
        send_byte(8'hCC, 1, 0);  // order_ref_no[47:40]
        send_byte(8'hDD, 1, 0);  // order_ref_no[39:32]
        send_byte(8'hEE, 1, 0);  // order_ref_no[31:24]
        send_byte(8'hFF, 1, 0);  // order_ref_no[23:16]
        send_byte(8'h11, 1, 0);  // order_ref_no[15:8]
        send_byte(8'h22, 1, 0);  // order_ref_no[7:0]
        send_byte(8'h00, 1, 0);  // shares[31:24]
        send_byte(8'h00, 1, 0);  // shares[23:16]
        send_byte(8'h10, 1, 0);  // shares[15:8]
        send_byte(8'h00, 1, 0);  // shares[7:0]
        send_byte(8'hAA, 1, 0);  // match_no[63:56]
        send_byte(8'hBB, 1, 0);  // match_no[55:48]
        send_byte(8'hCC, 1, 0);  // match_no[47:40]
        send_byte(8'hDD, 1, 0);  // match_no[39:32]
        send_byte(8'hEE, 1, 0);  // match_no[31:24]
        send_byte(8'hFF, 1, 0);  // match_no[23:16]
        send_byte(8'h67, 1, 0);  // match_no[15:8]
        send_byte(8'h69, 1, 0);  // match_no[7:0] (end message)

        // Valid "X" Type Message
        send_byte(8'h58, 1, 1);  // msg_type (start message)
        send_byte(8'h01, 1, 0);  // stock_locate[15:8]
        send_byte(8'h02, 1, 0);  // stock_locate[7:0]
        send_byte(8'h03, 1, 0);  // tracking_no[15:8]
        send_byte(8'h04, 1, 0);  // tracking_no[7:0]
        send_byte(8'h00, 1, 0);  // timestamp[47:40]
        send_byte(8'h01, 1, 0);  // timestamp[39:32]
        send_byte(8'h02, 1, 0);  // timestamp[31:24]
        send_byte(8'h03, 1, 0);  // timestamp[23:16]
        send_byte(8'h04, 1, 0);  // timestamp[15:8]
        send_byte(8'h05, 1, 0);  // timestamp[7:0]
        send_byte(8'hAA, 1, 0);  // order_ref_no[63:56]
        send_byte(8'hBB, 1, 0);  // order_ref_no[55:48]
        send_byte(8'hCC, 1, 0);  // order_ref_no[47:40]
        send_byte(8'hDD, 1, 0);  // order_ref_no[39:32]
        send_byte(8'hEE, 1, 0);  // order_ref_no[31:24]
        send_byte(8'hFF, 1, 0);  // order_ref_no[23:16]
        send_byte(8'h11, 1, 0);  // order_ref_no[15:8]
        send_byte(8'h22, 1, 0);  // order_ref_no[7:0]
        send_byte(8'h00, 1, 0);  // shares[31:24]
        send_byte(8'h00, 1, 0);  // shares[23:16]
        send_byte(8'h10, 1, 0);  // shares[15:8]
        send_byte(8'h00, 1, 0);  // shares[7:0] (end message)

        // Valid "D" Type Message
        send_byte(8'h44, 1, 1);  // msg_type (start message)
        send_byte(8'h01, 1, 0);  // stock_locate[15:8]
        send_byte(8'h02, 1, 0);  // stock_locate[7:0]
        send_byte(8'h03, 1, 0);  // tracking_no[15:8]
        send_byte(8'h04, 1, 0);  // tracking_no[7:0]
        send_byte(8'h00, 1, 0);  // timestamp[47:40]
        send_byte(8'h01, 1, 0);  // timestamp[39:32]
        send_byte(8'h02, 1, 0);  // timestamp[31:24]
        send_byte(8'h03, 1, 0);  // timestamp[23:16]
        send_byte(8'h04, 1, 0);  // timestamp[15:8]
        send_byte(8'h05, 1, 0);  // timestamp[7:0]
        send_byte(8'hAA, 1, 0);  // order_ref_no[63:56]
        send_byte(8'hBB, 1, 0);  // order_ref_no[55:48]
        send_byte(8'hCC, 1, 0);  // order_ref_no[47:40]
        send_byte(8'hDD, 1, 0);  // order_ref_no[39:32]
        send_byte(8'hEE, 1, 0);  // order_ref_no[31:24]
        send_byte(8'hFF, 1, 0);  // order_ref_no[23:16]
        send_byte(8'h11, 1, 0);  // order_ref_no[15:8]
        send_byte(8'h22, 1, 0);  // order_ref_no[7:0] (end message)

        // Valid "U" Type Message
        send_byte(8'h55, 1, 1);  // msg_type (start message)
        send_byte(8'h01, 1, 0);  // stock_locate[15:8]
        send_byte(8'h02, 1, 0);  // stock_locate[7:0]
        send_byte(8'h03, 1, 0);  // tracking_no[15:8]
        send_byte(8'h04, 1, 0);  // tracking_no[7:0]
        send_byte(8'h00, 1, 0);  // timestamp[47:40]
        send_byte(8'h01, 1, 0);  // timestamp[39:32]
        send_byte(8'h02, 1, 0);  // timestamp[31:24]
        send_byte(8'h03, 1, 0);  // timestamp[23:16]
        send_byte(8'h04, 1, 0);  // timestamp[15:8]
        send_byte(8'h05, 1, 0);  // timestamp[7:0]
        send_byte(8'hAA, 1, 0);  // order_ref_no[63:56]
        send_byte(8'hBB, 1, 0);  // order_ref_no[55:48]
        send_byte(8'hCC, 1, 0);  // order_ref_no[47:40]
        send_byte(8'hDD, 1, 0);  // order_ref_no[39:32]
        send_byte(8'hEE, 1, 0);  // order_ref_no[31:24]
        send_byte(8'hFF, 1, 0);  // order_ref_no[23:16]
        send_byte(8'h11, 1, 0);  // order_ref_no[15:8]
        send_byte(8'h22, 1, 0);  // order_ref_no[7:0]
        send_byte(8'h67, 1, 0);  // new_order_ref_no[63:56]
        send_byte(8'h67, 1, 0);  // new_order_ref_no[55:48]
        send_byte(8'h67, 1, 0);  // new_order_ref_no[47:40]
        send_byte(8'h67, 1, 0);  // new_order_ref_no[39:32]
        send_byte(8'h67, 1, 0);  // new_order_ref_no[31:24]
        send_byte(8'h67, 1, 0);  // new_order_ref_no[23:16]
        send_byte(8'h67, 1, 0);  // new_order_ref_no[15:8]
        send_byte(8'h67, 1, 0);  // new_order_ref_no[7:0]
        send_byte(8'h00, 1, 0);  // shares[31:24]
        send_byte(8'h00, 1, 0);  // shares[23:16]
        send_byte(8'h10, 1, 0);  // shares[15:8]
        send_byte(8'h00, 1, 0);  // shares[7:0]
        send_byte(8'h00, 1, 0);  // price[31:24]
        send_byte(8'h01, 1, 0);  // price[23:16]
        send_byte(8'h02, 1, 0);  // price[15:8]
        send_byte(8'h03, 1, 0);  // price[7:0], end message

        // Invalid message type
        send_byte(8'h5A, 1, 1);  // msg_type (start message)
        send_byte(8'h04, 1, 0);
        send_byte(8'h05, 1, 0);
        send_byte(8'hAA, 1, 0);
        send_byte(8'hBB, 1, 0);
        send_byte(8'hCC, 1, 0);
        send_byte(8'hDD, 1, 0);
        send_byte(8'hEE, 1, 0);
        send_byte(8'hFF, 1, 0);  // end message


        $finish;
    end

    // Monitor outputs
    initial begin
        $monitor(
            "Time: %0t | valid_msg=%b | msg_type=%h | stock_locate=%h | tracking_no=%h | timestamp=%h | order_ref_no=%h | shares=%h | buy_sell=%h | stock=%h | price=%h | match_no=%h",
            $time, valid_msg, msg_type, stock_locate, tracking_no, timestamp, order_ref_no, shares,
            buy_sell, stock, price, match_no);
    end

endmodule
