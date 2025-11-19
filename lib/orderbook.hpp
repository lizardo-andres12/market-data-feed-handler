#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include <cstdint>
#include <unordered_map>

using __book_key_t = std::uint64_t;


struct OrderBookEntry {
    std::uint64_t bidPrice;
    std::uint32_t bidQuantity;
    std::uint64_t askPrice;
    std::uint32_t askQuantity;
    std::uint64_t udpatedAt;
};


class OrderBook {
    std::unordered_map<__book_key_t, OrderBookEntry> book;
public:
    OrderBook() = default;

    const OrderBookEntry* getEntry(const __book_key_t key) const;

    bool insertEntry(const __book_key_t key, OrderBookEntry& entry);

    bool updateEntry(const __book_key_t key, OrderBookEntry& entry);

    bool isSymbolInBook(const __book_key_t key) const {
	auto it = book.find(key);
	return it != book.end();
    }

    OrderBook(const OrderBook& ob) = delete;
    OrderBook(OrderBook&& ob) = delete;
    void operator=(const OrderBook& ob) = delete;
    void operator=(OrderBook&& ob) = delete;
};


#endif

