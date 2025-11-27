#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>

#include "messages.hpp"


/**
 * @brief A class for storing symbols mapped to their current best bid/ask prices and quantities.
 * Supports insertion of a symbol/entry pair and updating the entry for an existing symbol. This
 * data structure cannot and should not be moved or copied.
 */
class OrderBook {
public:
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
     * @brief Default constructor for OrderBook that instantiates all member variables.
     */
    OrderBook() = default;

    /**
     * @brief Attempts to retrieve the order metadata object associated with the input symbol.
     *
     * @param key The symbol of the instrument stored as the key.
     * @return An optional containing a const pointer to the `OrderBookEntry` associated with `key` if it exists.
     */
    std::optional<const OrderBookEntry*> getEntry(const std::uint64_t key) const;

    /**
     * @brief Creates a new symbol-to-quote mapping using the symbol and `QuoteMessage` object provided. If the key already exits,
     * 		map the key to a new entry based on `msg`.
     *
     * @param key The symbol to update or insert.
     * @param msg The data of the incoming quote.
     */
    void upsertEntry(const std::uint64_t, const QuoteMessage& msg);

    /**
     * @brief Displays the state of the book.
     */
    void showState() const;

    OrderBook(const OrderBook& ob) = delete;
    OrderBook(OrderBook&& ob) = delete;
    void operator=(const OrderBook& ob) = delete;
    void operator=(OrderBook&& ob) = delete;

private:
    std::unordered_map<std::uint64_t, OrderBookEntry> book_;
};

