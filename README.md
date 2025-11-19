# market-data-feed-handler
Task 6 of High Performance Computing C++ practice

---

## **Task 6: Complete Market Data Feed Handler**

**Difficulty: Expert**

This **realistic market data processing system** combines everything you've learned:
- Binary message parsing (Task 4)
- Order book management (Task 2)
- VWAP calculation (Task 3)
- Lock-free queues (Task 5)

### The System:

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

---

## **Specs:**

This is a multi-threaded system that:

1. **Parser Thread:**
   - Reads binary market data from stdin
   - Parses trade and quote messages (zero-copy)
   - Pushes messages to lock-free queue

2. **Processor Thread:**
   - Pops messages from queue
   - Updates order book for quotes
   - Calculates VWAP for trades
   - Tracks statistics

3. **Output:**
   - Final order book state (best 5 bid/ask levels)
   - VWAP per symbol
   - Message processing rate
   - Total latency (end-to-end)

---

## **Binary Message Format:**

### **Message Types:**

```cpp
enum class MessageType : uint8_t {
    Trade = 1,
    Quote = 2
};
```

### **Trade Message (32 bytes):**
```
[Type: 1 byte][Timestamp: 8 bytes][Symbol: 8 bytes][Price: 8 bytes][Quantity: 4 bytes][Padding: 3 bytes]
```

### **Quote Message (40 bytes):**
```
[Type: 1 byte][Timestamp: 8 bytes][Symbol: 8 bytes][Bid Price: 8 bytes][Bid Qty: 4 bytes][Ask Price: 8 bytes][Ask Qty: 4 bytes][Padding: 3 bytes]
```

**Note:** Quotes are **snapshots** (not incremental). When you receive a quote for AAPL, it replaces the previous AAPL quote entirely.

---

## **Message Structures:**

```cpp
struct MessageHeader {
    uint8_t type;
    uint64_t timestamp;  // Microseconds
    char symbol[8];
};

struct TradeMessage {
    MessageHeader header;
    uint64_t price;      // Cents
    uint32_t quantity;
    uint8_t padding[3];
};

struct QuoteMessage {
    MessageHeader header;
    uint64_t bid_price;  // Cents
    uint32_t bid_qty;
    uint64_t ask_price;  // Cents
    uint32_t ask_qty;
    uint8_t padding[3];
};
```

---

### **1. Order Book Per Symbol:**

Track best bid/ask for each symbol. For simplicity, just track **top level** (not full depth):

```cpp
struct OrderBookEntry {
    uint64_t bid_price;
    uint32_t bid_qty;
    uint64_t ask_price;
    uint32_t ask_qty;
    uint64_t last_update_time;
};

std::unordered_map<std::string, OrderBookEntry> order_books;
```

### **2. VWAP Tracker Per Symbol:**

```cpp
struct VWAPData {
    uint64_t sum_price_qty;  // Sum of (price * quantity)
    uint64_t sum_qty;        // Sum of quantity
    uint32_t trade_count;
};

std::unordered_map<std::string, VWAPData> vwap_trackers;
```

### **3. Message Union (for queue):**

Since messages have different sizes, use a union:

```cpp
union MarketDataMessage {
    MessageHeader header;
    TradeMessage trade;
    QuoteMessage quote;
};
```

---

## **Output Format:**

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

---

## **Test Data Generator:**

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
    
    # Base prices for each symbol (in cents)
    base_prices = {
        b'AAPL\x00\x00\x00\x00': 15000,
        b'GOOGL\x00\x00\x00': 280000,
        b'MSFT\x00\x00\x00\x00': 35000,
        b'AMZN\x00\x00\x00\x00': 17000,
    }
    
    with open(filename, 'wb') as f:
        # Write number of messages
        f.write(struct.pack('<Q', num_messages))
        
        timestamp = 1000000  # Start at 1 second
        
        for i in range(num_messages):
            symbol = random.choice(symbols)
            base_price = base_prices[symbol]
            
            # 70% quotes, 30% trades
            if random.random() < 0.7:
                # Quote message
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
                f.write(b'\x00\x00\x00')  # Padding
            else:
                # Trade message
                msg_type = 1
                price = base_price + random.randint(-50, 50)
                quantity = random.randint(1, 10) * 100
                
                f.write(struct.pack('<B', msg_type))
                f.write(struct.pack('<Q', timestamp))
                f.write(symbol)
                f.write(struct.pack('<Q', price))
                f.write(struct.pack('<I', quantity))
                f.write(b'\x00\x00\x00')  # Padding
            
            # Increment timestamp (random microseconds)
            timestamp += random.randint(1, 100)

if __name__ == '__main__':
    generate_market_data('market_feed.bin', 1_000_000)
    print("Generated market_feed.bin with 1M messages")
```

---

## **Constraints:**

- Process **1,000,000+ messages**
- Target throughput: **5M+ messages/sec**
- Ring buffer size: **8192 elements** (power of 2)
- Keep total code under **500 lines**
- Must compile with `-fsanitize=thread` without errors

---

## **Performance Expectations:**

| Implementation Quality | Throughput | Latency |
|------------------------|------------|---------|
| Basic (single-threaded) | 1-2M msg/s | 500ns |
| Good (multi-threaded) | 5-10M msg/s | 200ns |
| Excellent (optimized) | 20-50M msg/s | 50ns |
