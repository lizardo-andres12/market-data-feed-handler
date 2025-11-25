#ifndef ORDERBOOK_HPP
#define ORDERBOOK_HPP

#include <cstdint>
#include <optional>
#include <unordered_map>

/// type alias for clarity
using __book_key_t = std::uint64_t;


/**
 * @brief The collection of relevant data to store in the OrderBook for a single symbol.
 */
struct OrderBookEntry {
    std::uint64_t udpatedAt;
    std::uint64_t bidPrice;
    std::uint64_t askPrice;
    std::uint32_t bidQuantity;
    std::uint32_t askQuantity;
};


/**
 * @brief A class for storing symbols mapped to their current best bid/ask prices and quantities.
 * Supports insertion of a symbol/entry pair and updating the entry for an existing symbol. This
 * data structure cannot and should not be moved or copied.
 */
class OrderBook {
    std::unordered_map<__book_key_t, OrderBookEntry> book_;
public:
    OrderBook() = default;

    std::optional<const OrderBookEntry*> getEntry(const __book_key_t key) const;

    void upsertEntry(const __book_key_t key, OrderBookEntry& entry);

    void showState() const;

    OrderBook(const OrderBook& ob) = delete;
    OrderBook(OrderBook&& ob) = delete;
    void operator=(const OrderBook& ob) = delete;
    void operator=(OrderBook&& ob) = delete;
};


#endif

