#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

#include "lib/messages.hpp"
#include "lib/orderbook.hpp"
#include "lib/spsc_queue.hpp"


int main() {
    SPSCQueue<int, 1024> queue;
    MarketDataMessage msg;
    OrderBook book;

    std::vector<std::uint8_t> buf { 0x01, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    msg.trade = *reinterpret_cast<TradeMessage*>(buf.data());
    std::cout << msg.trade.timestamp << '\n';

    OrderBookEntry entry {
	std::uint64_t { 2 },
	std::uint32_t { 1 },
	std::uint64_t { 2 },
	std::uint32_t { 1 },
	std::uint64_t { 4 }
    };
    book.insertEntry(std::uint64_t { 3 }, entry);

    const OrderBookEntry* found = book.getEntry(std::uint64_t { 3 });
    assert(found != nullptr);

    std::cout << found->udpatedAt << '\n';
    std::cout << "IT WOREKD ASDF" << '\n';
    
    return 0;
}

