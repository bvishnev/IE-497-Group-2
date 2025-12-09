# IE 497 Fall 2025 Group 2: FPGA ITCH Parser

## Project Overview
The goal of this project is to create a basic FPGA parser that takes in a serial ITCH data stream and separates the different fields into individual registers. In practice, this would be connected to a local order book that utilizes this information.

## Group Members
- Ben Vishnevskiy (Electrical Engineering)
- Sam Vishnevskiy (Math & Computer Science)
- Sathya Mitiran (Computer Engineering)
- Sid Rau (Computer Engineering)

## Background
ITCH is a high-speed market data protocol used by stock exchanges like NASDAQ to broadcast real-time information about orders, trades, and quotes. It provides granular, time-stamped data on every change in the order book, allowing traders and (in this case) algorithms to see market activity and react quickly. The full protocol contains 22 different message types, but only four are implemented in this project: Add Order – No MPID Attribution (type "A" message), Order Executed Message (type "E" message), Order Cancel Message (type "X" message), and Order Delete Message (type "D" message). Messages are variable-length, and are delimited by separate start and end message flags. Each message type has its own set of fields, though some are shared across several (such as Timestamp, Tracking Number, and Order Reference Number). The full encoding schemes of each of the four message types implemented in this project are shown below.

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

**Source:** Nasdaq, Inc. *Nasdaq TotalView-ITCH Specification*. (PDF)  
Available at: https://www.nasdaqtrader.com/content/technicalsupport/specifications/dataproducts/NQTVITCHSpecification.pdf



