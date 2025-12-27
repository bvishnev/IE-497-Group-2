# IE 497 Fall 2025 Group 2: FPGA ITCH Parser

## Project Overview
The goal of this project is to create a basic FPGA parser that takes in a serial ITCH data stream and separates the different fields into individual registers. In practice, this would be connected to a local order book that utilizes this information.

## Group Members
- Ben Vishnevskiy (Electrical Engineering)
- Sam Vishnevskiy (Math & Computer Science)
- Sathya Mitiran (Computer Engineering)
- Sid Rau (Computer Engineering)

## Background

### FPGAs
A Field Programmable Gate Array (FPGA) is a type of chip that acts as an effective middle ground between a Central Processing Unit (CPU) and an Application Specific Integrated Circuit (ASIC). 

A CPU is the type of processing chip used widely in commercial computers because it is the most flexible option. A CPU consists of multiple hard-coded cores, each of which can process instructions that indicate what the hardware should do next. These instructions are sourced from software, which is what makes CPUs so highly flexible and able to support such a broad range of general tasks. The price of this extreme versatility is that the overhead incurred by fetching, decoding, and executing instructions slows down the performance of the CPU. For varied instruction streams without patterns, like the tasks of a commercial computer, CPUs are excellent, but they are suboptimal for applications that involve repeated, predictable, or parallel instructions or where extremely low latency (the delay between the calling and completion of a computer action) is a priority.  

At the other end of the spectrum, an ASIC is a chip that is engineered to perform one specific task only. Since ASICs don’t support any interpretation of instructions, they are extremely fast and offer predictable latency. The price of this performance is very costly and time-intensive development that requires extreme precision. Furthermore, the complete rigidity of an ASIC once it has been manufactured renders it virtually useless if specifications or objectives change even slightly. 

The middle ground between CPUs and ASICs is FPGAs, which are boards that contain numerous unordered logic units that can be reconfigured to support different functionalities. This is unlike an ASIC, which is hardwired to perform a very specific sequence of actions, or a CPU, which is hard-coded with instruction processing capabilities that allow variable software to run on fixed hardware. The FPGA offers support for physically reconfiguring the hardware of the chip itself, which offers substantially more flexibility than an ASIC, which is crucial for cutting down on the massive cost of redesigning and manufacturing new ASIC circuits for even small changes in functionality. The FPGA is not quite as general purpose as a CPU, but it makes up for this by offering significantly lower and more predictable latency, much closer to that of an ASIC, which is vital in performance-critical applications. 

### HFT & Market Making
One increasingly common use case of FPGAs is in the financial sector, particularly in a field known as high-frequency trading (HFT). HFT is an algorithmic trading strategy that involves automatically buying and selling large volumes of securities such as stocks, bonds, futures, and options at very high rates. One common HFT strategy is market making, in which a firm provides liquidity to a market by continuously buying a security at the bid price and selling at the ask price, and profiting on the difference. This process makes financial markets more efficient, as all of the market participants gain continuous access to liquid assets that can easily be traded. However, when there are multiple competing market makers, the profits go to whichever firm makes the trades first. This is because as one market maker adds more liquidity to the market, the bid-ask spread tightens, leaving less potential profit for those who are late to trade. This is because bid and ask prices reflect supply and demand, so a market maker posting lower asks and higher bids tightens the spread. This paradigm has led to an arms race for developing faster and lower latency systems in electronic markets. This is precisely where FPGAs come into play: they are a tool to process market data with very low latency, which is essential for feeding into market making algorithms, while offering dramatically more flexibility than ASICs in the case of changing strategies, new requirements for data extraction, or updated specifications for how market data is transferred.

### ITCH Protocol
ITCH is a high-speed market data protocol used by stock exchanges like NASDAQ to broadcast real-time information about orders, trades, and quotes. It provides granular, time-stamped data on every change in the order book, allowing traders and (in this case) algorithms to see market activity and react quickly. The full protocol contains 22 different message types, but only six are implemented in this project: Add Order – No MPID Attribution (type "A" message), Add Order with MPID Attribution (type "F" message), Order Executed Message (type "E" message), Order Cancel Message (type "X" message), Order Delete Message (type "D" message), and Order Replace Message (type "U" message). Messages are variable-length and are delimited by a separate start message flag. Each message type has its own set of fields, though some are shared across several (such as Timestamp, Tracking Number, and Order Reference Number). The full encoding schemes for each of the message types implemented in this project are shown below.

**Add Order – No MPID Attribution ("A")**
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

**Add Order with MPID Attribution ("F")**
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
| Price                | 32     | 4      | Price (4)  | Display price of the new order                                                       |
| Attribution          | 36     | 4      | Alpha   | Nasdaq Market participant identifier associated with the entered order                  |

**Order Executed Message ("E")**

| Name                 | Offset | Length | Value   | Notes                                                                                    |
|----------------------|--------|--------|---------|------------------------------------------------------------------------------------------|
| Message Type         | 0      | 1      | “E”     | Add Order – Order Executed Message                                                       |
| Stock Locate         | 1      | 2      | Integer | Locate code identifying the security                                                     |
| Tracking Number      | 3      | 2      | Integer | Nasdaq internal tracking number                                                          |
| Timestamp            | 5      | 6      | Integer | Nanoseconds since midnight                                                               |
| Order Reference No.  | 11     | 8      | Integer | Unique reference number assigned to the new order at time of receipt                     |
| Executed Shares      | 19     | 4      | Integer | Number of shares executed                                                                |
| Match Number         | 23     | 8      | Integer | Nasdaq-generated day-unique Match Number for this execution                              |

**Order Cancel Message ("X")**
| Name                 | Offset | Length | Value   | Notes                                                                      |
|----------------------|--------|--------|---------|----------------------------------------------------------------------------|
| Message Type         | 0      | 1      | “X”     | Order Cancel Message                                                       |
| Stock Locate         | 1      | 2      | Integer | Locate code identifying the security                                       |
| Tracking Number      | 3      | 2      | Integer | Nasdaq internal tracking number                                            |
| Timestamp            | 5      | 6      | Integer | Nanoseconds since midnight                                                 |
| Order Reference No.  | 11     | 8      | Integer | The reference number of the order being canceled                           |
| Cancelled Shares     | 19     | 4      | Integer | Shares being removed from the display size due to cancellation             |

**Order Delete Message ("D")**
| Name                 | Offset | Length | Value   | Notes                                                                      |
|----------------------|--------|--------|---------|----------------------------------------------------------------------------|
| Message Type         | 0      | 1      | “D”     | Order Delete Message                                                       |
| Stock Locate         | 1      | 2      | Integer | Locate code identifying the security                                       |
| Tracking Number      | 3      | 2      | Integer | Nasdaq internal tracking number                                            |
| Timestamp            | 5      | 6      | Integer | Nanoseconds since midnight                                                 |
| Order Reference No.  | 11     | 8      | Integer | The reference number of the order being canceled                           |

**Order Replace Message ("U")**
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
The design was implemented in SystemVerilog (source files included). The module takes in the data byte stream, a start message flag, and a valid byte flag, along with a clock and asynchronous reset signal. Each field of the message is assigned to its own output register; unused fields are assigned to zero. Finally, there is another output that is raised for one cycle at the end of a message to indicate that a valid message has been processed. The inputs and outputs are summarized in the tables below.

### Inputs

| Name       | Size (bits) | Description |
|------------|-------------|-------------|
| clk       | 1           | Clock signal |
| rst        | 1           | Reset signal |
| start_msg  | 1           | Indicates the start of a new message |
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

The implementation also utilizes four internal signals: `byte_idx`, `count_en`, `end_msg`, and `message_invalid`. `byte_idx` is a 6-bit counter used to keep track of the index of the current byte, which is then used to determine which field it belongs to. `byte_idx` is incremented on every clock edge, and reset with the `start_msg` signal. `count_en` is an enable signal that indicates whether `byte_idx` should be incremented on a given clock cycle. `count_en` is `1` when the current `message` is between a start delimiter and the computed end delimiter and the encoding is valid, and `0` when the current `message` is outside of a start delimiter and the computed end delimiter or the encoding is invalid. When `count_en` is `0`, `byte_idx` is set to its maximum value of `0b111111`, which is outside of the index range of any message type and helps prevent accidentally overwriting data. `end_msg` is a flag that indicates when the last byte of a valid message has been reached, and is determined based on the length of the given message type and `byte_idx`. `message_invalid` is a 1-bit signal that keeps track of whether any bytes so far have been invalid, which is used to make the ultimate determination of whether the overall message was valid. 

## Testing 
To test that `parser.sv` performs the desired functions as intended, we created a testbench file called `parser_tb.sv`, which executes several test cases that represent different types of valid and invalid encodings across all supported market actions. We validated the outputs of the parser by inspecting the waveform viewer and the console printout. The parser correctly writes all of the data to the corresponding registers in valid messages, and stops writing to registers immediately upon receiving an invalid byte. The parser also correctly ignores any data sent outside of the start and end delimiters, which prevents writing garbage data to registers. 

Waveform diagrams for select test cases are shown below. The rest can be found in the `Simulation Waveforms` folder. 

### Valid "A" message
<p align="center">
  <img src="./Simulation%20Waveforms/Valid%20A%20Message.png" width="1000">
</p>

### Invalid "A" message
<p align="center">
  <img src="./Simulation%20Waveforms/Invalid%20A%20Message.png" width="1000">
</p>

### In Between Messages
<p align="center">
  <img src="./Simulation%20Waveforms/In%20Between%20Messages.png" width="1000">
</p>

## FPGA Platform Validation
The target hardware for the parser is the [AMD Alveo™ U55C High Performance Compute Card](https://www.amd.com/en/products/accelerators/alveo/u55c/a-u55c-p00g-pq-g.html), although full deployment of the final design on physical hardware was not completed due to time constraints. However, the FPGA board, toolchain, and execution workflow were successfully validated using a representative test design. 

We interacted with the FPGA through [Apache Guacamole](https://guacamole.apache.org/), a remote desktop gateway, and [Xilinx Runtime (XRT)](https://xilinx.github.io/XRT/2025.1/html/index.html), a software provided by AMD to interface between Xilinx FPGA boards and host programs running on a CPU. Both the FPGA and the remote environment were provided courtesy of Professor David Lariviere and the FinTech Lab at the University of Illinois Urbana-Champaign.

The design that we successfully compiled and ran on the FPGA was a basic vector doubling kernel. The source file `double_vector.cpp` is included. Note that this design was implemented using C++ for [Vitis High-Level Synthesis (HLS)](https://www.amd.com/en/products/software/adaptive-socs-and-fpgas/vitis/vitis-hls.html) rather than Verilog because HLS is sufficient as proof of concept for the design and integrates more easily with Vitis and XRT, though compilation of RTL kernels is also supported. This kernel takes in an array of 1024 integers, doubles each one, and outputs the new array. The kernel is tested by a C host file called `double_vector_test.c`, which uses the [OpenCL library](https://github.com/KhronosGroup/OpenCL-Headers) to send a test vector where the value of each element is just its index, and compares the results produced by the FPGA to the expected results. A screenshot of the terminal output after successful execution is shown below.

<p align="center">
  <img src="./Successful%20Execution%20of%20Double%20Vector.png" width="800">
</p>


In addition, included below is a list of terminal commands needed in the compilation and execution process.    
    
To set up Vitis:    
`cd /tools/Xilinx/Vitis_HLS/2024.2`    
`source /tools/Xilinx/Vitis_HLS/2024.2/settings64.sh`    
`cd ~`    
`PLATFORM=/opt/xilinx/platforms/xilinx_u55c_gen3x16_xdma_3_202210_1/xilinx_u55c_gen3x16_xdma_3_202210_1.xpfm`   

To compile the kernel:   
`v++ -t hw --platform xilinx_u55c_gen3x16_xdma_3_202210_1 -c -k kernel_function_header -o xo_filename.xo kernel_cpp_filename.cpp`   
`v++ -t hw --platform xilinx_u55c_gen3x16_xdma_3_202210_1 -l -o xclbin_filename.xclbin xo_filename.xo`   

To set up XRT:     
`source /opt/xilinx/xrt/setup.sh`

To compile the host file:    
`gcc -o object_filename host_filename.c -lOpenCL`    

To run the host file:   
`./object_filename xclbin_filename.xclbin`    


## Next Steps
The next step in development would be to compile the full parser and validate it on the physical U55C FPGA board. In addition, while the implementation of the parser is largely complete, it has still yet to be tested with real market data rather than the arbritary placeholder values in the testbench. Future work could include building out the parser to support the full breadth of possible market actions, and then using this complete parser on a live or historical market data stream. Finally, future work could also include designing an order book that uses the outputs of the parser as inputs to support book-building functionalities.

