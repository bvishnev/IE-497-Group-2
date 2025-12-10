# IE 497 Fall 2025 Group 2: FPGA ITCH Parser

## Project Overview
The goal of this project is to create a basic FPGA parser that takes in a serial ITCH data stream and separates the different fields into individual registers. In practice, this would be connected to a local order book that utilizes this information.

## Group Members
- Ben Vishnevskiy (Electrical Engineering)
- Sam Vishnevskiy (Math & Computer Science)
- Sathya Mitiran (Computer Engineering)
- Sid Rau (Computer Engineering)

## Background
ITCH is a high-speed market data protocol used by stock exchanges like NASDAQ to broadcast real-time information about orders, trades, and quotes. It provides granular, time-stamped data on every change in the order book, allowing traders and (in this case) algorithms to see market activity and react quickly. The full protocol contains 22 different message types, but only six are implemented in this project: Add Order – No MPID Attribution (type "A" message), Add Order with MPID Attribution (type "F" message). Order Executed Message (type "E" message), Order Cancel Message (type "X" message), Order Delete Message (type "D" message), and Order Replace Message (type "U" message). Messages are variable-length, and are delimited by separate start and end message flags. Each message type has its own set of fields, though some are shared across several (such as Timestamp, Tracking Number, and Order Reference Number). The full encoding schemes of each of the four message types implemented in this project are shown below.

### Add Order – No MPID Attribution ("A")
| Name                 | Offset | Length | Value   | Notes                                                                                   |
|----------------------|--------|--------|---------|-----------------------------------------------------------------------------------------|
| Message Type         | 0      | 1      | “A”     | Add Order – No MPID Attribution Message                                                 |
| Stock Locate         | 1      | 2      | Integer | Locate code identifying the security                                                    |
| Tracking Number      | 3      | 2      | Integer | Nasdaq internal tracking number                                                         |
| Timestamp            | 5      | 6      | Integer | Nanoseconds since midnight                                                              |
| Order Reference No.  | 11     | 8      | Integer | Unique reference number assigned to the new order at the time of receipt                |
| Buy/Sell Indicator   | 19     | 1      | Alpha   | “B” = Buy Order, “S” = Sell Order                                                       |
| Shares               | 20     | 4      | Integer | Total shares for the order being added                                                  |
| Stock                | 24     | 8      | Alpha   | Stock symbol, right-padded with spaces                                                  |
| Price                | 32     | 4      | Price   | Display price of the new order                                                          |

### Add Order with MPID Attribution ("F")
| Name                 | Offset | Length | Value   | Notes                                                                                   |
|----------------------|--------|--------|---------|-----------------------------------------------------------------------------------------|
| Message Type         | 0      | 1      | “F”     | Add Order with MPID Attribution Message                                                 |
| Stock Locate         | 1      | 2      | Integer | Locate code identifying the security                                                    |
| Tracking Number      | 3      | 2      | Integer | Nasdaq internal tracking number                                                         |
| Timestamp            | 5      | 6      | Integer | Nanoseconds since midnight                                                              |
| Order Reference No.  | 11     | 8      | Integer | Unique reference number assigned to the new order at the time of receipt                |
| Buy/Sell Indicator   | 19     | 1      | Alpha   | “B” = Buy Order, “S” = Sell Order                                                       |
| Shares               | 20     | 4      | Integer | Total shares for the order being added                                                  |
| Stock                | 24     | 8      | Alpha   | Stock symbol, right-padded with spaces                                                  |
| Price                | 32     | 4      | Price (4)  | Display price of the new order                                                          |
| Attribution          | 36     | 4      | Alpha   | Nasdaq Market participant identifier associated with the entered order                  |

### Order Executed Message ("E")

| Name                 | Offset | Length | Value   | Notes                                                                                    |
|----------------------|--------|--------|---------|------------------------------------------------------------------------------------------|
| Message Type         | 0      | 1      | “E”     | Add Order – Order Executed Message                                                       |
| Stock Locate         | 1      | 2      | Integer | Locate code identifying the security                                                     |
| Tracking Number      | 3      | 2      | Integer | Nasdaq internal tracking number                                                          |
| Timestamp            | 5      | 6      | Integer | Nanoseconds since midnight                                                               |
| Order Reference No.  | 11     | 8      | Integer | Unique reference number assigned to the new order at time of receipt                     |
| Executed Shares      | 19     | 4      | Integer | Number of shares executed                                                                |
| Match Number         | 23     | 8      | Integer | Nasdaq-generated day-unique Match Number for this execution (also used in Trade Break)   |

### Order Cancel Message ("X")
| Name                 | Offset | Length | Value   | Notes                                                                      |
|----------------------|--------|--------|---------|----------------------------------------------------------------------------|
| Message Type         | 0      | 1      | “X”     | Order Cancel Message                                                       |
| Stock Locate         | 1      | 2      | Integer | Locate code identifying the security                                       |
| Tracking Number      | 3      | 2      | Integer | Nasdaq internal tracking number                                            |
| Timestamp            | 5      | 6      | Integer | Nanoseconds since midnight                                                 |
| Order Reference No.  | 11     | 8      | Integer | The reference number of the order being canceled                           |
| Cancelled Shares     | 19     | 4      | Integer | Shares being removed from the display size due to cancellation             |

### Order Delete Message ("D")
| Name                 | Offset | Length | Value   | Notes                                                                      |
|----------------------|--------|--------|---------|----------------------------------------------------------------------------|
| Message Type         | 0      | 1      | “D”     | Order Delete Message                                                       |
| Stock Locate         | 1      | 2      | Integer | Locate code identifying the security                                       |
| Tracking Number      | 3      | 2      | Integer | Nasdaq internal tracking number                                            |
| Timestamp            | 5      | 6      | Integer | Nanoseconds since midnight                                                 |
| Order Reference No.  | 11     | 8      | Integer | The reference number of the order being canceled                           |

### Order Replace Message ("U")
| Name                 | Offset | Length | Value   | Notes                                                                      |
|----------------------|--------|--------|---------|----------------------------------------------------------------------------|
| Message Type         | 0      | 1      | “D”     | Order Delete Message                                                       |
| Stock Locate         | 1      | 2      | Integer | Locate code identifying the security                                       |
| Tracking Number      | 3      | 2      | Integer | Nasdaq internal tracking number                                            |
| Timestamp            | 5      | 6      | Integer | Nanoseconds since midnight                                                 |
| Original Order Reference No.  | 11     | 8      | Integer | The original order reference number of the order being replaced   |
| New Order Reference No. | 19  | 8               | Integer | The new reference number for this order at time of replacement    |
| Shares               | 27     | 4      | Integer | The new total displayed quantity                                           |
| Price                | 31     | 4      | Price (4)   | The new display price for the order                                        |



**Source:** Nasdaq, Inc. *Nasdaq TotalView-ITCH Specification*. (PDF)  
Available at: https://www.nasdaqtrader.com/content/technicalsupport/specifications/dataproducts/NQTVITCHSpecification.pdf

## Implementation Details
The design was implemented in SystemVerilog (source file included). The module takes in the data byte stream, start and end message flags, a valid byte flag, along with a clock and asynchronous reset signal. Each field of the message is assigned to its own output register; unused fields are assigned to zero. Finally, there is another output that is raised for one cycle at the end of a message to indicate that a valid message has been processed. The inputs and outputs are summarized in the tables below.

### Inputs

| Name       | Size (bits) | Description |
|------------|-------------|-------------|
| clk       | 1           | Clock signal |
| rst        | 1           | Reset signal |
| start_msg  | 1           | Indicates the start of a new message |
| end_msg    | 1           | Indicates the end of a message |
| message    | 8           | Byte-wise serial message data |
| valid      | 1           | Indicates whether the current message byte is valid |

### Outputs

| Name          | Size (bits) | Messages Used For | Description |
|---------------|-------------|-------------------|-------------|
| valid_msg     | 1           | A, F, E, X, D, U       | Indicates whether the entire message is valid |
| msg_type      | 8           | A, F, E, X, D, U       | Type of market action encoded in the message |
| stock_locate  | 16          | A, F, E, X, D, U       | Locate code identifying the security |
| tracking_no   | 16          | A, F, E, X, D, U       | Nasdaq internal tracking number |
| timestamp     | 48          | A, F, E, X, D, U       | Nanoseconds since midnight |
| order_ref_no  | 64          | A, F, E, X, D, U       | Unique reference number assigned to the order |
| shares        | 32          | A, F, E, X, U          | Total number of shares associated with the order |
| price         | 32          | A, F, U                 | Display price of the new order |
| buy_sell      | 8           | A, F                 | Type of order (“B”=Buy, “S”=Sell) |
| stock         | 64          | A, F                 | Stock symbol, right padded with spaces |
| attribution   | 32          | F                 | Nasdaq Market participant identifier associated with the entered order
| match_no      | 64          | E                 | Day-unique Match Number for this execution |
| new_order_ref_no | 64       | U                    | The unique reference number assigned to the new order at the time of receipt | 

The implementation also utilizes two internal signals, `byte_idx` and `message_invalid`. `byte_idx` is a 6-bit counter used to keep track of the index of the current byte, which is then used to determine which field it belongs to. `byte_idx` is incremented on every clock edge, and reset with the `start_msg` signal. `message_invalid` is a 1-bit signal that keeps track of whether any bytes so far have been invalid, which is used to make the ultimate determination of whether the overall message was valid.

## Next Steps
While the implementation of the parser is largely complete, it is still a draft, and has not yet been rigorously tested and validated. The next step would be to write a testbench that tests the design with several valid messages of each type, as well as a variety of invalid cases (e.g. `valid` signal equal to `0` for one or more bytes, improper message length, etc.). Longer term, future work could include implementation and testing on a physical FPGA board, or designing an order book that uses the outputs of the parser as inputs.

