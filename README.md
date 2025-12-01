# Market Data Feed Handler

A high-performance, multi-threaded market data processing system for parsing and analyzing binary market data feeds in real-time.

## Overview

This system processes binary-encoded trade and quote messages using a lock-free architecture to achieve high throughput and low latency. It maintains order books per symbol, calculates volume-weighted average prices (VWAP), and provides comprehensive performance statistics.

### Architecture

```
[Binary Market Data File]
         ↓
   [Parser Thread] ────→ [Lock-Free Queue] ────→ [Processor Thread]
                                                         ↓
                                                  [Order Book]
                                                  [VWAP Tracker]
                                                         ↓
                                                   [Statistics]
```

### Key Features

- **Zero-copy binary message parsing** for minimal overhead
- **Lock-free ring buffer** for inter-thread communication
- **Per-symbol order book** tracking best bid/ask
- **Real-time VWAP calculation** for all trades
- **Performance metrics** including throughput and latency

## Binary Message Format

### Message Types

```cpp
enum class MessageType : uint8_t {
    Trade = 1,
    Quote = 2
};
```

### Trade Message (32 bytes)

```
[Type: 1 byte][Timestamp: 8 bytes][Symbol: 8 bytes][Price: 8 bytes][Quantity: 4 bytes][Padding: 3 bytes]
```

```cpp
struct TradeMessage {
    MessageType type;
    uint64_t timestamp;  // Microseconds since epoch
    uint64_t symbol;     // 8-byte symbol identifier
    uint64_t price;      // Price in cents
    uint32_t quantity;   // Share quantity
    uint8_t padding[3];
};
```

### Quote Message (44 bytes)

```
[Type: 1 byte][Timestamp: 8 bytes][Symbol: 8 bytes][Bid Price: 8 bytes][Bid Qty: 4 bytes][Ask Price: 8 bytes][Ask Qty: 4 bytes][Padding: 3 bytes]
```

```cpp
struct QuoteMessage {
    MessageType type;
    uint64_t timestamp;  // Microseconds since epoch
    uint64_t symbol;     // 8-byte symbol identifier
    uint64_t bid_price;  // Bid price in cents
    uint32_t bid_qty;    // Bid quantity
    uint64_t ask_price;  // Ask price in cents
    uint32_t ask_qty;    // Ask quantity
    uint8_t padding[3];
};
```

**Note:** Quote messages are snapshots that replace the previous quote for a given symbol entirely (not incremental updates).

## Data Structures

### Order Book Entry

```cpp
struct OrderBookEntry {
    uint64_t bid_price;
    uint32_t bid_qty;
    uint64_t ask_price;
    uint32_t ask_qty;
    uint64_t last_update_time;
};

std::unordered_map<uint64_t, OrderBookEntry> order_books;
```

### VWAP Tracker

```cpp
struct VWAPEntry {
    uint64_t sum_price_qty;  // Sum of (price × quantity)
    uint64_t sum_qty;        // Total quantity
    uint32_t trade_count;    // Number of trades
};

std::unordered_map<uint64_t, VWAPEntry> vwap_trackers;
```

### Message Union

```cpp
union MarketDataMessage {
    MessageHeader header;
    TradeMessage trade;
    QuoteMessage quote;
};
```

## Building

```bash
# Compile with thread sanitizer for validation
g++ -std=c++17 -O3 -pthread -fsanitize=thread -o feed_handler main.cpp

# Run with test data
./feed_handler < market_feed.bin
```

## Output Format

```
=== Order Books ===
AAPL: Bid 150.25 x 1000 | Ask 150.30 x 800
GOOGL: Bid 2800.50 x 500 | Ask 2801.00 x 600

=== VWAP ===
AAPL: $150.28 (10,000 shares, 150 trades)
GOOGL: $2800.75 (5,000 shares, 75 trades)

=== Performance ===
Total messages: 1,000,000
Processing time: 250 ms
Throughput: 4,000,000 msgs/sec
Average latency: 45 ns/msg
```

## Test Data Generation

A Python script is provided to generate binary market data files:

```python
import struct
import random

def generate_market_data(filename, num_messages=1_000_000):
    symbols = [
        b'AAPL\x00\x00\x00\x00',
        b'GOOGL\x00\x00\x00',
        b'MSFT\x00\x00\x00\x00',
        b'AMZN\x00\x00\x00\x00',
    ]
    
    base_prices = {
        b'AAPL\x00\x00\x00\x00': 15000,
        b'GOOGL\x00\x00\x00': 280000,
        b'MSFT\x00\x00\x00\x00': 35000,
        b'AMZN\x00\x00\x00\x00': 17000,
    }
    
    with open(filename, 'wb') as f:
        f.write(struct.pack('<Q', num_messages))
        timestamp = 1000000
        
        for i in range(num_messages):
            symbol = random.choice(symbols)
            base_price = base_prices[symbol]
            
            if random.random() < 0.7:  # 70% quotes
                msg_type = 2
                bid_price = base_price + random.randint(-100, 0)
                bid_qty = random.randint(1, 20) * 100
                ask_price = base_price + random.randint(0, 100)
                ask_qty = random.randint(1, 20) * 100
                
                f.write(struct.pack('<B', msg_type))
                f.write(struct.pack('<Q', timestamp))
                f.write(symbol)
                f.write(struct.pack('<Q', bid_price))
                f.write(struct.pack('<I', bid_qty))
                f.write(struct.pack('<Q', ask_price))
                f.write(struct.pack('<I', ask_qty))
                f.write(b'\x00\x00\x00')
            else:  # 30% trades
                msg_type = 1
                price = base_price + random.randint(-50, 50)
                quantity = random.randint(1, 10) * 100
                
                f.write(struct.pack('<B', msg_type))
                f.write(struct.pack('<Q', timestamp))
                f.write(symbol)
                f.write(struct.pack('<Q', price))
                f.write(struct.pack('<I', quantity))
                f.write(b'\x00\x00\x00')
            
            timestamp += random.randint(1, 100)

if __name__ == '__main__':
    generate_market_data('market_feed.bin', 1_000_000)
    print("Generated market_feed.bin with 1M messages")
```

Usage:
```bash
python generate_data.py
```

## Configuration

- **Ring buffer size:** 8192 elements (power of 2)
- **Default test size:** 1,000,000 messages
- **Thread model:** Single parser thread, single processor thread

## Performance Benchmarks

| Configuration | Throughput | Avg Latency |
|---------------|------------|-------------|
| Single-threaded | 1-2M msg/s | ~500ns |
| Multi-threaded | 5-10M msg/s | ~200ns |
| Optimized | 20-50M msg/s | ~50ns |
