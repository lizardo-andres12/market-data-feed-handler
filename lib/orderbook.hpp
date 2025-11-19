#ifndef ORDERBOOK_HPP
#define ORDERBOOK_HPP

#include <cstdint>
#include <optional>
#include <unordered_map>

using __book_key_t = std::uint64_t;


struct OrderBookEntry {
    std::uint64_t udpatedAt;
    std::uint64_t bidPrice;
    std::uint64_t askPrice;
    std::uint32_t bidQuantity;
    std::uint32_t askQuantity;
};


class OrderBook {
    std::unordered_map<__book_key_t, OrderBookEntry> book;
public:
    OrderBook() = default;

    std::optional<const OrderBookEntry*> getEntry(const __book_key_t key) const;

    bool insertEntry(const __book_key_t key, OrderBookEntry& entry);

    bool updateEntry(const __book_key_t key, OrderBookEntry& entry);

    OrderBook(const OrderBook& ob) = delete;
    OrderBook(OrderBook&& ob) = delete;
    void operator=(const OrderBook& ob) = delete;
    void operator=(OrderBook&& ob) = delete;
};


#endif

