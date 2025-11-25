import os
import struct
import sys
import random

DEBUG=True if os.getenv('DEBUG', False) == 'true' else False

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
        
        for _ in range(num_messages):
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
                
                if DEBUG:
                    print(f"QUOTE -> symbol: {symbol}, bid price: {bid_price}, bid qty: {bid_qty}, ask price: {ask_price}, ask qty: {ask_qty}, timestamp: {timestamp}")

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

                if DEBUG:
                    print(f"TRADE -> symbol: {symbol}, price: {price}, qty: {quantity}, timestamp: {timestamp}")
                
                f.write(struct.pack('<B', msg_type))
                f.write(struct.pack('<Q', timestamp))
                f.write(symbol)
                f.write(struct.pack('<Q', price))
                f.write(struct.pack('<I', quantity))
                f.write(b'\x00\x00\x00')  # Padding
            
            # Increment timestamp (random microseconds)
            timestamp += random.randint(1, 100)

if __name__ == '__main__':
    if not os.path.exists('./build'):
        print(f"Error: trying to place file in ./build directory, which doesn't exist")
        sys.exit(1)

    generate_market_data('./build/market_feed.bin', 5_000_000)
    print("Generated market_feed.bin")
    sys.exit(0)

